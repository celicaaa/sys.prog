#pragma once
#include <windows.h>

class EventController {
private:
    HANDLE hStartEvent;
    HANDLE hStopEvent;
    HANDLE hConfirmEvent;
    HANDLE hCloseEvent;

public:
    EventController();
    ~EventController();

    bool Initialize();
    void WaitForCommand(int& commandIndex);
    void SendConfirmation();
    void Cleanup();
};