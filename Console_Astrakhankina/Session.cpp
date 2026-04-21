#include "Session.h"
#include <iostream>
using namespace std;

Session::Session(int sessionID) : sessionID(sessionID) {
}

Session::~Session() {
}

void Session::addMessage(Message& m) {
    {
        lock_guard<mutex> lg(mx);
        messages.push(m);
    }
    ev.notify_one();
}

void Session::addMessage(MessageTypes messageType, const wstring& data) {
    Message m(messageType, data);
    addMessage(m);
}

bool Session::getMessage(Message& m, DWORD timeout) {
    unique_lock<mutex> ul(mx);
    if (!ev.wait_for(ul, chrono::milliseconds(timeout), [this] { return !messages.empty(); })) {
        return false;
    }
    m = messages.front();
    messages.pop();
    return true;
}

bool Session::hasMessages() const {
    return !messages.empty();
}