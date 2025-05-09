.global _main
_main:
    ;; variable 'x' allocated at offset 0
    mov x0, #4
    str x0, [sp, #8]
    ldr x0, [sp, #8]
    str x0, [sp, #0]
    ;; compound-mult:  
    mov x0, #2
    str x0, [sp, #8]
    ldr x1, [sp, #8]
    ldr x0, [sp, #0]
    mul x0, x0, x1
    str x0, [sp, #0]
    ;; incrementing variable 'x' at offset 0
    ldr x0, [sp, #0]
    add x0, x0, #1
    str x0, [sp, #0]
    ;; variable 'y' allocated at offset 8
    mov x0, #3
    str x0, [sp, #16]
    ldr x0, [sp, #16]
    str x0, [sp, #8]
    ;; compound-plus:  
    ;; Loading variable y (stack_loc=1) from offset 8
    ldr x0, [sp, #8]
    str x0, [sp, #16]
    ldr x1, [sp, #16]
    ldr x0, [sp, #0]
    add x0, x0, x1
    str x0, [sp, #0]
    ;; incrementing variable 'x' at offset 0
    ldr x0, [sp, #0]
    add x0, x0, #1
    str x0, [sp, #0]
    ;; decrementing variable 'x' at offset 0
    ldr x0, [sp, #0]
    sub x0, x0, #1
    str x0, [sp, #0]
    ;; incrementing variable 'y' at offset 8
    ldr x0, [sp, #8]
    add x0, x0, #1
    str x0, [sp, #8]
    ;; compound-div:  
    ;; Loading variable y (stack_loc=1) from offset 8
    ldr x0, [sp, #8]
    str x0, [sp, #16]
    ldr x1, [sp, #16]
    ldr x0, [sp, #0]
    sdiv x0, x0, x1
    str x0, [sp, #0]
    ;; Evaluating exit expression
    ;; Loading variable x (stack_loc=0) from offset 0
    ldr x0, [sp, #0]
    str x0, [sp, #16]
    ;; Exit value on stack
    ;; exit
    mov x16, #1
    ldr x0, [sp, #16]
    ;; Exit with value in x0
    svc #0
    ;; /exit
    mov x16, #1
    mov x0, #0
    svc #0