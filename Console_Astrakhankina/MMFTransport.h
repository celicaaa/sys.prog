#pragma once
#include <windows.h>
#include <string>
using namespace std;

// Message header structure for MMF
struct MMFHeader {
    int addr;       // Target address (-1 = all threads, 0 = main thread, >0 = thread ID)
    int cmdType;    // Command type (0 = close, 1 = data)
    int size;       // Data size in bytes
};

// Transport strategy interface
class ITransportStrategy {
public:
    virtual ~ITransportStrategy() = default;
    virtual bool Send(int addr, int cmdType, const wchar_t* data, int dataSize) = 0;
    virtual bool Receive(MMFHeader& header, wchar_t* buffer, int bufferSize, int& outSize) = 0;
};

// Memory-mapped file transport implementation
class MMFTransport : public ITransportStrategy {
private:
    HANDLE hFile;
    HANDLE hFileMap;
    HANDLE hMutex;
    const wchar_t* mapName;
    const wchar_t* mutexName;

public:
    MMFTransport(const wchar_t* mMap = L"MyMapFile_Astrakhankina", 
                 const wchar_t* mMutex = L"MyMapMutex_Astrakhankina");
    ~MMFTransport();

    virtual bool Send(int addr, int cmdType, const wchar_t* data, int dataSize) override;
    virtual bool Receive(MMFHeader& header, wchar_t* buffer, int bufferSize, int& outSize) override;
};

// Exported functions for DLL
extern "C" {
    __declspec(dllexport) void* CreateTransport();
    __declspec(dllexport) void DestroyTransport(void* transport);
    __declspec(dllexport) bool TransportSend(void* transport, int addr, int cmdType, const wchar_t* data, int dataSize);
    __declspec(dllexport) bool TransportReceive(void* transport, MMFHeader* header, wchar_t* buffer, int bufferSize, int* outSize);
}
