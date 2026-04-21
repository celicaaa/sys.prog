#include "ThreadManager.h"
#include <iostream>
using namespace std;

ThreadManager::ThreadManager() : nextId(1) {}

ThreadManager::~ThreadManager() {
    Cleanup();
}

void ThreadManager::CreateThread() {
    LocalTransport::addThread(nextId);
    threadIds.push_back(nextId);
    wcout << L"Создан поток  " << nextId << endl;
    nextId++;
}

void ThreadManager::RemoveLastThread() {
    if (threadIds.empty()) return;

    int lastId = threadIds.back();
    Message::sendMessage(LocalTransport(), lastId, MT_CLOSE);
    threadIds.pop_back();

    wcout << L"Последний поток удален." << endl;
}

bool ThreadManager::HasThreads() const {
    return !threadIds.empty();
}

void ThreadManager::Cleanup() {
    while (HasThreads()) {
        RemoveLastThread();
    }
    LocalTransport::waitThreads();
}

void ThreadManager::SendToThread(int threadId, MessageTypes type, const wstring& data) {
    // Send to specific thread's session
    lock_guard<mutex> lg(LocalTransport::mx);
    auto it = LocalTransport::sessions.find(threadId);
    if (it != LocalTransport::sessions.end()) {
        it->second->addMessage(type, data);
        wcout << L"Sent to thread " << threadId << L": " << data << endl;
    } else {
        wcout << L"Thread " << threadId << L" not found." << endl;
    }
}

void ThreadManager::BroadcastMessage(MessageTypes type, const wstring& data) {
    // Send to all threads with single call using MMF transport
    LocalTransport::sendToAll(type, data);
    
    // Also add to each session's queue for independent processing
    lock_guard<mutex> lg(LocalTransport::mx);
    for (auto& pair : LocalTransport::sessions) {
        pair.second->addMessage(type, data);
    }
    wcout << L"Broadcasted to all threads: " << data << endl;
}