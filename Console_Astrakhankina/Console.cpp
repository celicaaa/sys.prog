#include <iostream>
#include <locale>
#include <thread>
#include "ThreadManager.h"
#include "EventController.h"
#include "LocalTransport.h"
#include "Session.h"
using namespace std;

// Event handles for communication with dialog
HANDLE hMessageReceivedEvent = NULL;
HANDLE hProcessingConfirmedEvent = NULL;

// Main thread session for receiving messages
Session* mainSession = nullptr;
mutex mainSessionMx;

// Thread function for main thread message processing
void MainThreadWorker() {
    wcout << L"Main thread started, waiting for messages..." << endl;
    
    while (true) {
        Message m;
        if (mainSession && mainSession->getMessage(m, 500)) {
            if (m.header.messageType == MT_CLOSE) {
                wcout << L"Main thread received close command." << endl;
                break;
            } else if (m.header.messageType == MT_DATA) {
                // Output received text to stdout
                wcout << L"Main thread received: " << m.data << endl;
                
                // Signal that message was received
                if (hProcessingConfirmedEvent) {
                    SetEvent(hProcessingConfirmedEvent);
                }
            }
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    // Initialize events for dialog communication
    hMessageReceivedEvent = CreateEvent(NULL, FALSE, FALSE, L"MessageReceivedEvent_Astrakhankina");
    hProcessingConfirmedEvent = CreateEvent(NULL, FALSE, FALSE, L"ProcessingConfirmedEvent_Astrakhankina");
    
    // Initialize transport DLL
    if (!LocalTransport::Initialize()) {
        wcout << L"Failed to initialize transport. Exiting." << endl;
        return 1;
    }

    ThreadManager threadMgr;
    EventController ipc;

    if (!ipc.Initialize()) {
        return 1;
    }

    // Create main session for receiving messages
    mainSession = new Session(0);
    
    // Start main thread worker
    thread mainThread(MainThreadWorker);

    wcout << L"Консоль запущена. Ожидание команд..." << endl;

    bool running = true;
    while (running) {
        int commandIndex;
        ipc.WaitForCommand(commandIndex);

        switch (commandIndex) {
        case 0:  // Start - create thread
            threadMgr.CreateThread();
            ipc.SendConfirmation();
            break;

        case 1:  // Stop - remove last thread
            if (threadMgr.HasThreads()) {
                threadMgr.RemoveLastThread();
                ipc.SendConfirmation();
            }
            break;

        case 2:  // Close
            running = false;
            break;
            
        case 3:  // Message from dialog
            {
                // Receive message via MMF
                MMFHeader header;
                wchar_t buffer[65536]; // Large buffer for arbitrary length text
                int outSize = 0;
                
                // Use the transport directly to receive
                LocalTransport lt(-1);
                Message m;
                lt.receive(m);
                
                if (m.header.size > 0) {
                    int targetAddr = m.header.to;
                    
                    if (targetAddr == -1) {
                        // Send to all threads (broadcast)
                        wcout << L"Broadcasting message to all threads: " << m.data << endl;
                        threadMgr.BroadcastMessage(MT_DATA, m.data);
                    } else if (targetAddr == 0) {
                        // Send to main thread
                        wcout << L"Sending to main thread: " << m.data << endl;
                        mainSession->addMessage(MT_DATA, m.data);
                    } else {
                        // Send to specific thread
                        wcout << L"Sending to thread " << targetAddr << L": " << m.data << endl;
                        threadMgr.SendToThread(targetAddr, MT_DATA, m.data);
                    }
                    
                    // Confirm processing
                    ipc.SendConfirmation();
                }
            }
            break;
        }
    }

    // Cleanup
    wcout << L"Shutting down..." << endl;
    
    // Send close to main thread
    if (mainSession) {
        mainSession->addMessage(MT_CLOSE);
    }
    
    mainThread.join();
    delete mainSession;
    
    threadMgr.Cleanup();
    ipc.Cleanup();
    LocalTransport::Cleanup();
    
    CloseHandle(hMessageReceivedEvent);
    CloseHandle(hProcessingConfirmedEvent);
    
    wcout << L"Консоль завершена." << endl;

    return 0;
}