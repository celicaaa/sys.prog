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

void Session::getMessage(Message& m) {
    unique_lock<mutex> ul(mx);
    ev.wait(ul, [this] { return !messages.empty(); });
    m = messages.front();
    messages.pop();
}