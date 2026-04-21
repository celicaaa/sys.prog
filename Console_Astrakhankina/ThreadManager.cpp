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
    LocalTransport::cleanup();
}

void ThreadManager::SendToThread(int threadId, MessageTypes type, const wstring& data) {
    Message::sendMessage(LocalTransport(), threadId, type, data);
}