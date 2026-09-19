DEFINE_NAMESPACE_DETOUR MACRO namespace,func_name,end_type
	extern namespace&_fn&func_name : qword
	extern namespace&_Indirect&func_name : qword
	public namespace&_Detour&func_name
namespace&_Detour&func_name proc
    push rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    pushfq
    sub rsp, 40h
    movups[rsp + 30h], xmm0
    movups[rsp + 20h], xmm1
    movups[rsp + 10h], xmm2
    movups[rsp + 00h], xmm3
    lea rcx, [rsp + 0C8h]
    test rsp, 08h
    je stack_already_aligned
    mov rbx, 0FEDCBA9876543210h
    push rbx
stack_already_aligned:
    sub rsp, 20h
    call namespace&_Indirect&func_name
    add rsp, 20h
    mov rbx, 0FEDCBA9876543210h
    cmp qword ptr[rsp], rbx
    jne no_alignment_marker
    pop rbx
no_alignment_marker:
    movups xmm3, [rsp + 00h]
    movups xmm2, [rsp + 10h]
    movups xmm1, [rsp + 20h]
    movups xmm0, [rsp + 30h]
    add rsp, 40h
    popfq
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
ifidn <end_type>, <RETURN>
    pop rax
    pop rsp
     ret
elseifidn <end_type>, <JUMP>
    pop rax
    pop rsp
    push rax
    mov rax,namespace&_fn&func_name
    mov rax,[rax]
    xchg rax,[rsp]
    ret
else
    pop rax
    pop rsp
     int3
endif
namespace&_Detour&func_name endp
ENDM
