extern printf, scanf

section .text
    global main

main:
    ;scanf
    ; make new frame
    push ebp
    mov ebp, esp
    ; push args
    push y
    push op
    push x
    push in_format
    ; call function
    call scanf
    ; remove args from frame
    add esp, 16
load_args:
    fld qword [x]
    fld qword [y]

_calculate:
    mov ebx, 0          ; clear ebx for loading op sign
    mov bl, byte [op]   ; move op sign into ebx for cmp
    cmp bl, '+'
        je _add         
    cmp bl, '-'
        je _sub         
    cmp bl, '*'
        je _mul
    cmp bl, '/'
        je _div
    jmp _error
    
_add:
    fadd
    jmp _store
_sub:
    fsub
    jmp _store
_mul:
    fmul
    jmp _store
_div:
    fdiv
    jmp _store
_error:
    jmp $exit
_store:
    fstp qword [wynik]

print_wynik: 
    push ebp
    mov ebp, esp

    push dword [wynik+4]    ; can't push all at once :(
    push dword [wynik]
    push out_format
    call printf
    add esp, 12
    
    mov esp,ebp
    pop ebp
$exit:
    mov eax,1
    mov ebx,0
    int 0x80

section .bss
    x:          resq 1
    y:          resq 1
    op:         resb 1
    wynik:      resq 1

section .data
    in_format:  db "%lf %c %lf", 0
    out_format: db "%lf", 10, 0
    
