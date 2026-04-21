#include "LocalTransport.h"
#include <iostream>
using namespace std;

// Static member initialization
vector<thread> LocalTransport::threads;
map<int, Session*> LocalTransport::sessions;
mutex LocalTransport::mx;

HMODULE LocalTransport::hDll = NULL;
CreateTransportFunc LocalTransport::pCreateTransport = nullptr;
DestroyTransportFunc LocalTransport::pDestroyTransport = nullptr;
TransportSendFunc LocalTransport::pTransportSend = nullptr;
TransportReceiveFunc LocalTransport::pTransportReceive = nullptr;
void* LocalTransport::transport = nullptr;
bool LocalTransport::dllLoaded = false;

bool LocalTransport::LoadDll() {
    if (dllLoaded) return true;
    
    hDll = LoadLibrary(L"TransportDLL_Astrakhankina.dll");
    if (!hDll) {
        wcout << L"Failed to load DLL. Error: " << GetLastError() << endl;
        return false;
    }
    
    pCreateTransport = (CreateTransportFunc)GetProcAddress(hDll, "CreateTransport");
    pDestroyTransport = (DestroyTransportFunc)GetProcAddress(hDll, "DestroyTransport");
    pTransportSend = (TransportSendFunc)GetProcAddress(hDll, "TransportSend");
    pTransportReceive = (TransportReceiveFunc)GetProcAddress(hDll, "TransportReceive");
    
    if (!pCreateTransport || !pDestroyTransport || !pTransportSend || !pTransportReceive) {
        wcout << L"Failed to get DLL function addresses." << endl;
        FreeLibrary(hDll);
        hDll = NULL;
        return false;
    }
    
    transport = pCreateTransport();
    if (!transport) {
        wcout << L"Failed to create transport." << endl;
        FreeLibrary(hDll);
        hDll = NULL;
        return false;
    }
    
    dllLoaded = true;
    wcout << L"Transport DLL loaded successfully." << endl;
    return true;
}

void LocalTransport::UnloadDll() {
    if (transport && pDestroyTransport) {
        pDestroyTransport(transport);
        transport = nullptr;
    }
    if (hDll) {
        FreeLibrary(hDll);
        hDll = NULL;
    }
    dllLoaded = false;
}

bool LocalTransport::Initialize() {
    return LoadDll();
}

void LocalTransport::Cleanup() {
    UnloadDll();
}

void LocalTransport::send(Message& m) const {
    if (!dllLoaded || !pTransportSend || !transport) return;
    
    int dataSize = m.header.size;
    pTransportSend(transport, m.header.to, m.header.messageType, m.data.c_str(), dataSize);
}

void LocalTransport::receive(Message& m) const {
    if (!dllLoaded || !pTransportReceive || !transport) return;
    
    MMFHeader header;
    wchar_t buffer[4096];
    int outSize = 0;
    
    if (pTransportReceive(transport, &header, buffer, 4096, &outSize)) {
        m.header.to = header.addr;
        m.header.messageType = header.cmdType;
        m.header.size = header.size;
        m.data = wstring(buffer, outSize);
    }
}

void LocalTransport::sendToAll(MessageTypes messageType, const wstring& data) {
    if (!dllLoaded || !pTransportSend || !transport) return;
    
    int dataSize = int(data.length() * sizeof(wchar_t));
    // addr = -1 means "all threads"
    pTransportSend(transport, -1, messageType, data.c_str(), dataSize);
}

void LocalTransport::worker(int sessionID) {
    auto session = new Session(sessionID);
    {
        lock_guard<mutex> lg(mx);
        sessions[sessionID] = session;
    }
    
    wcout << L"Thread " << sessionID << L" started, waiting for messages..." << endl;

    while (true) {
        Message m;
        // Use timeout to allow periodic checks
        if (session->getMessage(m, 500)) {
            if (m.header.messageType == MT_CLOSE) {
                wcout << L"Thread " << sessionID << L" received close command." << endl;
                {
                    lock_guard<mutex> lg(mx);
                    sessions.erase(sessionID);
                    delete session;
                }
                return;
            } else if (m.header.messageType == MT_DATA) {
                // Write received text to file <threadID>.txt
                wstring filename = to_wstring(sessionID) + L".txt";
                HANDLE hFile = CreateFile(filename.c_str(), 
                                          GENERIC_WRITE, 
                                          FILE_SHARE_READ, 
                                          NULL, 
                                          OPEN_ALWAYS, 
                                          FILE_ATTRIBUTE_NORMAL, 
                                          NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    SetFilePointer(hFile, 0, NULL, FILE_END);
                    DWORD written;
                    WriteFile(hFile, m.data.c_str(), (DWORD)m.data.length() * sizeof(wchar_t), &written, NULL);
                    WriteFile(hFile, L"\r\n", 2 * sizeof(wchar_t), &written, NULL);
                    CloseHandle(hFile);
                    wcout << L"Thread " << sessionID << L" wrote to " << filename << L": " << m.data << endl;
                }
            }
        }
    }
}

void LocalTransport::addThread(int sessionID) {
    threads.emplace_back(worker, sessionID);
}

void LocalTransport::waitThreads() {
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    threads.clear();
}