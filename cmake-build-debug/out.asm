.global _main
_main:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    ;; Variable 'x' allocated at offset 0
    mov x0, #100
    str x0, [sp, #8]
    ldr x0, [sp, #8]
    str x0, [sp, #0]
    ;; Variable 'y' allocated at offset 8
    mov x0, #10
    str x0, [sp, #16]
    ldr x0, [sp, #16]
    str x0, [sp, #8]
    ;; if
    ;; Loading variable x (stack_loc=0) from offset 0
    ldr x0, [sp, #0]
    str x0, [sp, #16]
    ;; Loading variable y (stack_loc=1) from offset 8
    ldr x0, [sp, #8]
    str x0, [sp, #24]
    ldr x1, [sp, #24]
    ldr x0, [sp, #16]
    cmp x0, x1
    b.gt greater_label0
    mov x0, #0
    b end_label1
greater_label0:
    mov x0, #1
end_label1:
    str x0, [sp, #16]
    ldr x0, [sp, #16]
    cbz x0, label2
    ;; Reassigning variable 'x' at offset 0
    mov x0, #11
    str x0, [sp, #16]
    ldr x0, [sp, #16]
    str x0, [sp, #0]
    ;; Reassigning variable 'y' at offset 8
    ;; Loading variable x (stack_loc=0) from offset 0
    ldr x0, [sp, #0]
    str x0, [sp, #16]
    ;; Loading variable y (stack_loc=1) from offset 8
    ldr x0, [sp, #8]
    str x0, [sp, #24]
    ldr x1, [sp, #24]
    ldr x0, [sp, #16]
    cmp x0, x1
    b.gt greater_label3
    mov x0, #0
    b end_label4
greater_label3:
    mov x0, #1
end_label4:
    str x0, [sp, #16]
    ldr x0, [sp, #16]
    str x0, [sp, #8]
label2:
    ;; /if
    ;; Evaluating exit expression
    ;; Loading variable y (stack_loc=1) from offset 8
    ldr x0, [sp, #8]
    str x0, [sp, #16]
    ;; Exit value on stack
    ;; exit
    mov x16, #1
    ldr x0, [sp, #16]
    ;; Exit with value in x0
    svc #0
    ;; /exit
    mov sp, x29
    ldp x29, x30, [sp], #16
    mov x16, #1
    mov x0, #0
    svc #0