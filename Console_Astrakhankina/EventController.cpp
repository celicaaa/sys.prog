#include "EventController.h"
#include <iostream>
using namespace std;

EventController::EventController()
    : hStartEvent(NULL), hStopEvent(NULL),
    hConfirmEvent(NULL), hCloseEvent(NULL), hMessageEvent(NULL) {
}

EventController::~EventController() {
    Cleanup();
}

bool EventController::Initialize() {
    hStartEvent = CreateEvent(NULL, FALSE, FALSE, L"StartEvent_Astrakhankina");
    hStopEvent = CreateEvent(NULL, FALSE, FALSE, L"StopEvent_Astrakhankina");
    hConfirmEvent = CreateEvent(NULL, FALSE, FALSE, L"ConfirmEvent_Astrakhankina");
    hCloseEvent = CreateEvent(NULL, FALSE, FALSE, L"CloseEvent_Astrakhankina");
    hMessageEvent = CreateEvent(NULL, FALSE, FALSE, L"MessageEvent_Astrakhankina");
    return true;
}

void EventController::WaitForCommand(int& commandIndex) {
    HANDLE waitHandles[] = { hStartEvent, hStopEvent, hCloseEvent, hMessageEvent };
    DWORD result = WaitForMultipleObjects(4, waitHandles, FALSE, INFINITE);
    commandIndex = result - WAIT_OBJECT_0;
}

void EventController::SendConfirmation() {
    SetEvent(hConfirmEvent);
}

void EventController::SignalMessageReady() {
    SetEvent(hMessageEvent);
}

void EventController::Cleanup() {
    if (hStartEvent) CloseHandle(hStartEvent);
    if (hStopEvent) CloseHandle(hStopEvent);
    if (hConfirmEvent) CloseHandle(hConfirmEvent);
    if (hCloseEvent) CloseHandle(hCloseEvent);
    if (hMessageEvent) CloseHandle(hMessageEvent);
}