.global _main
_main:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    mov x0, #0
    str x0, [sp, #0]
    ;; if
    mov x0, #1
    str x0, [sp, #8]
    ldr x0, [sp, #8]
    cbz x0, label0
    ;; var reassignment
    mov x0, #1
    str x0, [sp, #8]
    ldr x0, [sp, #8]
    str x0, [sp, #0]
    b label1
label0:
    ;; elif
    mov x0, #1
    str x0, [sp, #8]
    ldr x0, [sp, #8]
    cbz x0, label2
    ;; var reassignment
    mov x0, #2
    str x0, [sp, #8]
    ldr x0, [sp, #8]
    str x0, [sp, #0]
    b label1
label2:
    ;; else
    ;; var reassignment
    mov x0, #3
    str x0, [sp, #8]
    ldr x0, [sp, #8]
    str x0, [sp, #0]
label1:
    ;; /if
    ldr x0, [sp, #0]
    str x0, [sp, #8]
    ;; exit
    mov x16, #1
    ldr x0, [sp, #8]
    svc #0
    ;; /exit
    mov sp, x29
    ldp x29, x30, [sp], #16
    mov x16, #1
    mov x0, #0
    svc #0