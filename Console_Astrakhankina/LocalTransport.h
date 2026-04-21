#pragma once
#include "Message.h"
#include "Session.h"
#include <vector>
#include <map>
#include <mutex>
#include <thread>
using namespace std;

// Function pointer types for DLL functions
typedef void* (*CreateTransportFunc)();
typedef void (*DestroyTransportFunc)(void*);
typedef bool (*TransportSendFunc)(void*, int, int, const wchar_t*, int);
typedef bool (*TransportReceiveFunc)(void*, MMFHeader*, wchar_t*, int, int*);

class LocalTransport : public Sender, public Receiver {
private:
    int id;
    static vector<thread> threads;
    static map<int, Session*> sessions;
    static mutex mx;
    
    // DLL handling
    static HMODULE hDll;
    static CreateTransportFunc pCreateTransport;
    static DestroyTransportFunc pDestroyTransport;
    static TransportSendFunc pTransportSend;
    static TransportReceiveFunc pTransportReceive;
    static void* transport;
    static bool dllLoaded;
    
    static bool LoadDll();
    static void UnloadDll();

public:
    LocalTransport(int id = -1) : id(id) {}
    
    static bool Initialize();
    static void Cleanup();

    virtual void send(Message& m) const override;
    virtual void receive(Message& m) const override;

    static void worker(int sessionID);
    static void addThread(int sessionID);
    static void waitThreads();
    
    // Send to all threads with single call
    static void sendToAll(MessageTypes messageType, const wstring& data);
};