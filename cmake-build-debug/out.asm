global _start
_start:
    mov rax, 0
    push rax
    mov rax, 0
    push rax
    ;; if
    push QWORD [rsp + 8]
    pop rax
    test rax, rax
    jz label0
    push QWORD [rsp + 8]
    ;; exit
    mov rax, 60
    pop rdi
    syscall
    ;; /exit
    add rsp, 0
    jmp label1
label0:
    ;; elif
    push QWORD [rsp + 0]
    pop rax
    test rax, rax
    jz label2
    push QWORD [rsp + 0]
    ;; exit
    mov rax, 60
    pop rdi
    syscall
    ;; /exit
    add rsp, 0
    jmp label1
label2:
    ;; else
    mov rax, 10
    push rax
    ;; exit
    mov rax, 60
    pop rdi
    syscall
    ;; /exit
    add rsp, 0
label1:
    ;; /if
    mov rax, 1
    push rax
    ;; exit
    mov rax, 60
    pop rdi
    syscall
    ;; /exit
    mov rax, 60
    mov rdi, 0
    syscall