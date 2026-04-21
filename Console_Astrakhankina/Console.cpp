#include <iostream>
#include <locale>
#include "ThreadManager.h"
#include "EventController.h"
using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");

    ThreadManager threadMgr;
    EventController ipc;

    if (!ipc.Initialize()) {
        return 1;
    }

    wcout << L"Консоль запущена. Ожидание команд..." << endl;

    bool running = true;
    while (running) {
        int commandIndex;
        ipc.WaitForCommand(commandIndex);

        switch (commandIndex) {
        case 0:  // Start
            threadMgr.CreateThread();
            ipc.SendConfirmation();
            break;

        case 1:  // Stop
            if (threadMgr.HasThreads()) {
                threadMgr.RemoveLastThread();
                ipc.SendConfirmation();
            }
            else {
                running = false;
            }
            break;

        case 2:  // Close
            running = false;
            break;
        }
    }

    ipc.Cleanup();
    wcout << L"Консоль завершена." << endl;

    return 0;
}