;; calculate 128 bit factorial of given 2digitnumber
;; additional input is discarded: only first two digits are read (+newline)
;; if input > 34, output = lower 128bits of result
SECTION .text
global main
extern fscanf
extern printf
extern exit

    main:
        mov dword [_result],     0x1
        mov dword [_result+4],   0x0
        mov dword [_result+8],   0x0
        mov dword [_result+12],  0x0
    get_input:              ; get input from stdin (pipe or prompt)
        mov eax, 3          ; sys_read
        mov ebx, 0          ; fd: 0 = stdin
        mov ecx, input      ; buffer
        mov edx, 3          ; number of bytes (2 digits + newline)
        int 0x80            ; call kernel

    check_stdin:            ; check for excess input characters
        cmp eax, 3                  ; did we read more than 3 chars?
        jl convert_input            ; if less, good
        cmp byte [ecx + edx -1], 10 ; is last byte newline?
        je convert_input            ; if yes, good
        mov byte [ecx+edx-1], 10    ; if not, now it is
        call clear_terminal
        jmp convert_input

    ; read&discard chars left on stdin (until EOL),
    ; so that they're not read by shell
    clear_terminal:
        mov edx, 1
        mov ecx, buf
        mov ebx, 0  ; stdin
        mov eax, 3  ; sys_read
        int 0x80
        cmp byte [ecx+edx - 1], 10
        jne clear_terminal
        ret
        
    convert_input:
        cmp eax, 2          ; if we read 2 characters and we're here
                            ; then it's 1 digit number
        mov eax, 0          ; accumulator for str->dec conversion
        mov ebx, 0          ; zero ebx, as we're only loading 1B into it

        je one_digit        ; if we read one digit
        ; number = input + [input+1] *10
        digit_tens:     ; 2-digit input
            mov bl, byte [input]    ; load second digit
            sub ebx, 0x30           ; get digit value (0x30 = '0'
            imul ebx, 10
            add eax, ebx            ; add to accumulator

        digit_ones:
            mov bl, byte [input+1]  ; load first digit
            sub ebx, 0x30           ; get digit value
            add eax, ebx            ; add it to acumulator

        jmp store_result
        
        one_digit:      ; 1-digit input
            mov bl, byte[input]
            sub ebx, 0x30
            add eax, ebx
            ;jmp store_result

        store_result:
            mov dword [number], eax
        
    factorial:
        mov ecx, dword [number] ; ecx = input 
    _factor:                    ; _result = input * (input-1) * .. * 1
        call _128_mul           ; multiply _result by ecx
        loop _factor            ; decrements ecx

    print_number:
        ; printf the result (4 space separated octets)
        push eax                ; push printf args on stack
        push dword [_result]
        push dword [_result+4]
        push dword [_result+8]
        push dword [_result+12]
        push formatx
        call printf
        add esp, 36 ; remove stuff from stack
        ; add new line at the end
        push newline
        call printf        
        add esp, 2
    done:
        ;push 0
        ;call exit   ; exit(0)
        mov eax, 1
        mov ebx, 0
        int 0x80

    ; multiplication with 128 bit result
    ; multiplies 8word (_result) by ecx 
    _128_mul:       
        mov eax, [_result]
        mul ecx                 ; EDX:EAX = EAX*EBX
        mov [_result], eax      ; save first part of result
        mov ebx, edx            ; EDX is carry, save it in ECX
        
        mov eax, [_result+4]
        mul ecx
        add eax, ebx            ; add carry from previous mul
        mov [_result+4], eax
        mov ebx, edx
        ; same for next part
        mov eax, [_result+8]
        mul ecx
        add eax, ebx
        mov [_result+8], eax
        mov ebx, edx
        mov eax, [_result+12]
        mul ecx
        add eax, ebx
        mov [_result+12], eax
        ret

        
SECTION .data
    formatd:        db "%d", 0  ; decimal format for scanf
    formatx:        db "%08x %08x %08x %08x", 0
    newline:        db 10, 0
    number:         dd 0
SECTION .bss
    _result:        resd 4
    input:          resb 3      ; 2digits + EOL
    buf:            resb 1      ; buffer for cleaning


