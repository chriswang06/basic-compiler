global _start
_start:
    mov rax, 10
    push rax
    mov rax, 100
    push rax
    ;; if
    push QWORD [rsp + 8]
    push QWORD [rsp + 8]
    pop rax
    pop rbx
    cmp rbx, rax
    je eq_label0
    push 0
    jmp end_label1
eq_label0:
    push 1
end_label1:
    pop rax
    test rax, rax
    jz label2
    ;;reassigning
    mov rax, 100
    push rax
    pop rax
    mov [rsp + 16], rax
    ;;reassigning
    mov rax, 200
    push rax
    pop rax
    mov [rsp + 16], rax
    push QWORD [rsp + 16]
    push QWORD [rsp + 16]
    pop rax
    pop rbx
    cmp rbx, rax
    jg greater_label3
    push 0
    jmp end_label4
greater_label3:
    push 1
end_label4:
    ;; exit
    mov rax, 60
    pop rdi
    syscall
    ;; /exit
    add rsp, 0
    jmp label5
label2:
    ;; else
    mov rax, 0
    push rax
    ;; exit
    mov rax, 60
    pop rdi
    syscall
    ;; /exit
    add rsp, 0
label5:
    ;; /if
    push QWORD [rsp + 24]
    push QWORD [rsp + 24]
    pop rax
    pop rbx
    cmp rbx, rax
    jg greater_label6
    push 0
    jmp end_label7
greater_label6:
    push 1
end_label7:
    ;; exit
    mov rax, 60
    pop rdi
    syscall
    ;; /exit
    mov rax, 60
    mov rdi, 0
    syscall