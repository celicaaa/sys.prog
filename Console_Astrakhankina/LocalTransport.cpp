#include "LocalTransport.h"
#include <iostream>
using namespace std;

vector<thread> LocalTransport::threads;
map<int, Session*> LocalTransport::sessions;
mutex LocalTransport::mx;

void LocalTransport::worker(int sessionID) {
    auto session = new Session(sessionID);
    {
        lock_guard<mutex> lg(mx);
        sessions[sessionID] = session;
    }

    while (true) {
        Message m = Message::receiveMessage(LocalTransport(sessionID));
        if (m.header.messageType == MT_CLOSE) {
            {
                lock_guard<mutex> lg(mx);
                sessions.erase(sessionID);
                delete session;
            }
            return;
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

void LocalTransport::cleanup() {
    waitThreads();
    {
        lock_guard<mutex> lg(mx);
        for (auto& pair : sessions) {
            delete pair.second;
        }
        sessions.clear();
    }
}