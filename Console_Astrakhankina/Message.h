#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
using namespace std;

struct Message;

class Sender {
public:
    virtual void send(Message&) const = 0;
};

class Receiver {
public:
    virtual void receive(Message&) const = 0;
};

enum MessageTypes {
    MT_CLOSE = 0,
    MT_DATA = 1
};

struct MessageHeader {
    int messageType;
    int size;
    int to;
    int from;
};

struct Message {
    MessageHeader header = { 0 };
    wstring data;

    Message() = default;

    Message(MessageTypes messageType, const wstring& msgData = L"")
        : data(msgData) {
        header = { messageType, int(msgData.length() * sizeof(wchar_t)), -1, -1 };
    }

    Message(int to, MessageTypes messageType, const wstring& msgData = L"")
        : data(msgData) {
        header = { messageType, int(msgData.length() * sizeof(wchar_t)), to, -1 };
    }

    void send(const Sender& sender) const {
        sender.send(const_cast<Message&>(*this));
    }

    void receive(const Receiver& receiver) const {
        receiver.receive(const_cast<Message&>(*this));
    }

    static void sendMessage(const Sender& sender, MessageTypes messageType, const wstring& data = L"") {
        Message m(messageType, data);
        m.send(sender);
    }

    static void sendMessage(const Sender& sender, int to, MessageTypes messageType, const wstring& data = L"") {
        Message m(to, messageType, data);
        m.send(sender);
    }

    static Message receiveMessage(const Receiver& receiver) {
        Message m;
        m.receive(receiver);
        return m;
    }
};