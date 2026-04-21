#pragma once
#include <windows.h>

class EventController {
private:
    HANDLE hStartEvent;
    HANDLE hStopEvent;
    HANDLE hConfirmEvent;
    HANDLE hCloseEvent;
    HANDLE hMessageEvent;  // New event for message from dialog

public:
    EventController();
    ~EventController();

    bool Initialize();
    void WaitForCommand(int& commandIndex);
    void SendConfirmation();
    void Cleanup();
    
    // Signal that a message is ready to be received via MMF
    void SignalMessageReady();
};