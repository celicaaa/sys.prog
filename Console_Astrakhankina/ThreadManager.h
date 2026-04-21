#pragma once
#include "LocalTransport.h"
#include <vector>
using namespace std;

class ThreadManager {
private:
    vector<int> threadIds;
    int nextId;

public:
    ThreadManager();
    ~ThreadManager();

    void CreateThread();
    void RemoveLastThread();
    bool HasThreads() const;
    void Cleanup();
    void SendToThread(int threadId, MessageTypes type, const wstring& data = L"");
};