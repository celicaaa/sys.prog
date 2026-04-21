#pragma once
#include "Message.h"
#include "Session.h"
#include <vector>
#include <map>
#include <mutex>
#include <thread>
using namespace std;

class LocalTransport : public Sender, public Receiver {
private:
    int id;
    static vector<thread> threads;
    static map<int, Session*> sessions;
    static mutex mx;

public:
    LocalTransport(int id = -1) : id(id) {}

    virtual void send(Message& m) const override {
        if (id < 0)
            sessions[m.header.to]->addMessage(m);
        else
            sessions[id]->addMessage(m);
    }

    virtual void receive(Message& m) const override {
        sessions[id]->getMessage(m);
    }

    static void worker(int sessionID);
    static void addThread(int sessionID);
    static void waitThreads();
    static void cleanup();
};