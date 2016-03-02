; Find primes < 10000, using erastotenes' sieve

global main

extern printf

section .text

    main:
        
        ; fill array with numbers [10000..0]
        mov ecx, 10000              ; ecx = iterator = n
        fill_loop:
            mov [array+ecx*4], ecx  ; place next number
            loop fill_loop          ; while ecx > 0
        
        mov edx, 2
        
        loop_outer:                 ; edx: i
            cmp edx, 100            ; for (i = 2; i <= sqrt n; i++)
            jge end_outer           
            mov eax, edx            ; eax = edx
                                    ; eax: j
            loop_inner:             ; zero out every edx'th number       
                add eax, edx        ; for(j = 2i; j <= n; j += i)
                cmp eax, 10000      
                jge end_inner
                mov word [array+eax*4], 0x0 ; zero out non-primes
                jmp loop_inner
            end_inner

            inc edx
            jmp loop_outer
        end_outer

        mov eax, 2
        mov ebx, 0
        mov ecx, 0
        mov edx, 0

        loop_print:                 ; print everything not zeroed
            cmp eax, 10000          ; eax - iterator = [2..10000]
            jge _end                ; done printing
            mov ebx, [array+eax*4]  ; put next num to print in ebx
            cmp ebx, 0              ; zero check
            je continue             ; neeext
            push eax                ; save eax (iterator)
            push dword ebx          ; push printf params (number)
            push format             ; (format string)
            call printf             ; much overhead wow
            add esp, 8              ; go back on stack 2 words
            pop eax                 ; restore eax
            continue:
                inc eax
            jmp loop_print          ; repeat

        _end:
        ret                         ; END


section .data
    format:    db '%ld, ', 10, 0    ; data for printf (format, '\n', '\0')
    array:      resd 10000          ; array of 10k dwords (4bit ints)
