.code
PUBLIC SysCall1
SysCall1 PROC
    mov   r10, rdx
    mov   eax, ecx
    syscall
    ret
SysCall1 ENDP
PUBLIC SysCall2
SysCall2 PROC
    mov   r10, rdx
    mov   rax, rcx
    mov   rdx, r8
    syscall
    ret
SysCall2 ENDP
END
