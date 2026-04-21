#include "MMFTransport.h"
#include <iostream>
using namespace std;

// MMFTransport implementation
MMFTransport::MMFTransport(const wchar_t* mMap, const wchar_t* mMutex)
    : hFile(INVALID_HANDLE_VALUE), hFileMap(NULL), hMutex(NULL), 
      mapName(mMap), mutexName(mMutex) {
    
    // Create or open the mutex for synchronization
    hMutex = CreateMutex(NULL, FALSE, mutexName);
    
    // Create or open the backing file
    hFile = CreateFile(L"filemap.dat", 
                       GENERIC_READ | GENERIC_WRITE, 
                       FILE_SHARE_READ | FILE_SHARE_WRITE, 
                       NULL, 
                       OPEN_ALWAYS, 
                       0, 
                       NULL);
}

MMFTransport::~MMFTransport() {
    if (hFileMap) CloseHandle(hFileMap);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    if (hMutex) CloseHandle(hMutex);
}

bool MMFTransport::Send(int addr, int cmdType, const wchar_t* data, int dataSize) {
    if (!data || dataSize < 0) return false;
    
    // Calculate total size needed
    DWORD totalSize = sizeof(MMFHeader) + dataSize;
    
    // Wait for mutex ownership
    DWORD waitResult = WaitForSingleObject(hMutex, INFINITE);
    if (waitResult != WAIT_OBJECT_0) return false;
    
    bool success = false;
    do {
        // Create file mapping with required size
        hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, totalSize, mapName);
        if (!hFileMap) break;
        
        // Map view of file
        BYTE* buff = (BYTE*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, totalSize);
        if (!buff) break;
        
        // Write header
        MMFHeader header;
        header.addr = addr;
        header.cmdType = cmdType;
        header.size = dataSize;
        memcpy(buff, &header, sizeof(MMFHeader));
        
        // Write data
        if (dataSize > 0) {
            memcpy(buff + sizeof(MMFHeader), data, dataSize);
        }
        
        // Flush to ensure data is written
        FlushViewOfFile(buff, totalSize);
        
        success = true;
        
        UnmapViewOfFile(buff);
        CloseHandle(hFileMap);
        hFileMap = NULL;
        
    } while (false);
    
    ReleaseMutex(hMutex);
    return success;
}

bool MMFTransport::Receive(MMFHeader& header, wchar_t* buffer, int bufferSize, int& outSize) {
    outSize = 0;
    
    // Wait for mutex ownership
    DWORD waitResult = WaitForSingleObject(hMutex, INFINITE);
    if (waitResult != WAIT_OBJECT_0) return false;
    
    bool success = false;
    do {
        // Open existing file mapping
        hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, mapName);
        if (!hFileMap) break;
        
        // Get mapping size from file
        BY_HANDLE_FILE_INFORMATION fileInfo;
        if (!GetFileInformationByHandle(hFile, &fileInfo)) break;
        
        DWORD fileSize = fileInfo.nFileSizeLow;
        if (fileSize < sizeof(MMFHeader)) break;
        
        // Map view of file
        BYTE* buff = (BYTE*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, fileSize);
        if (!buff) break;
        
        // Read header
        memcpy(&header, buff, sizeof(MMFHeader));
        
        // Validate and read data
        if (header.size > 0 && header.size <= bufferSize * sizeof(wchar_t)) {
            memcpy(buffer, buff + sizeof(MMFHeader), header.size);
            outSize = header.size / sizeof(wchar_t);
            success = true;
        } else if (header.size == 0) {
            outSize = 0;
            success = true;
        }
        
        UnmapViewOfFile(buff);
        CloseHandle(hFileMap);
        hFileMap = NULL;
        
    } while (false);
    
    ReleaseMutex(hMutex);
    return success;
}

// DLL exported functions
extern "C" {
    __declspec(dllexport) void* CreateTransport() {
        return new MMFTransport();
    }
    
    __declspec(dllexport) void DestroyTransport(void* transport) {
        if (transport) {
            delete static_cast<MMFTransport*>(transport);
        }
    }
    
    __declspec(dllexport) bool TransportSend(void* transport, int addr, int cmdType, const wchar_t* data, int dataSize) {
        if (!transport) return false;
        return static_cast<MMFTransport*>(transport)->Send(addr, cmdType, data, dataSize);
    }
    
    __declspec(dllexport) bool TransportReceive(void* transport, MMFHeader* header, wchar_t* buffer, int bufferSize, int* outSize) {
        if (!transport || !header || !outSize) return false;
        return static_cast<MMFTransport*>(transport)->Receive(*header, buffer, bufferSize, *outSize);
    }
}
