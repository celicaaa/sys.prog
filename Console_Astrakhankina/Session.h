#pragma once
#include "Message.h"
#include <queue>
#include <mutex>
#include <condition_variable>
using namespace std;

class Session {
private:
    queue<Message> messages;
    mutex mx;
    condition_variable ev;

public:
    int sessionID;

    Session(int sessionID);
    ~Session();

    void addMessage(Message& m);
    void addMessage(MessageTypes messageType, const wstring& data = L"");
    void getMessage(Message& m);
};