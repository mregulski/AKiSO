; compile: nasm -f elf z6.asm
; debug: nasm -f elf -g -F dwarf z6.asm
section .text
global my_ln, my_e_to, my_sinh, my_sinhm1

    my_ln:    
    ; ln(x)
    ; log_2 (x) = log_e(x) / log_e (2)
    ; log_e(x) = log_2(x) * log_e(2) <known>
    ; result in st0
        push ebp        ; set up frame
        mov ebp, esp

        fldln2
        fld     dword [ebp+8]   ; load arg from stack
        ; st0 = arg
        ; st1 = ln2
        fyl2x               ; st0 = st1 * log_2(arg): final result

        mov esp, ebp        ; exit frame
        pop ebp
        ret                 ; return


    my_e_to: 
    ; e^x
    ; result in st0
        push ebp
        mov ebp, esp

        fld DWORD [ebp+0x8] ; load x

        fldl2e			    ; load log2(e)
        fmul			    ; st0 = xlog2(e) = V
        fld st0			    ; duplicate on stack
        frndint			    ; get closest int (A), might be above
        fsub			    ; get fraction part (F; A+F=V)
        f2xm1			    ; st0 = 2^frac(xlog2(e)) - 1 
                            ; (-1 <= exponent <= 1)
        fld1        		; load 1
        fadd			    ; get frac(e^x) (AAAA)
        ; same stuff for int part
        fld DWORD [ebp+0x8]	; load x again
        fldl2e			    ; load log2(e)
        fmul			    ; mnoże (j.w.)
        frndint			    ; get closest int
        fld1			    ; load 1
        fscale			    ; st0 = 1*2^int(log2(e)) (BBBB)
        fxch st1		    ; put AAAA and BBBB on top of stack
        fxch st2
        fmul			    ; result = AAAA * BBBB
        pop ebp
        ret
        
    my_sinh:     
    ; (e^x-e^(-x)) / 2
    ; result in st0
        push ebp            ; setup frame
        mov ebp, esp

        finit               ; initialize fpu

        mov eax, [ebp+0x8]	; get argument into eax
        mov [tmp], eax      ; and store it in tmp

        fld DWORD [tmp]     ; load it as float to stack
        fchs			    ; flip sign
        fst DWORD [tmp]	    ; store it back

        mov eax, [tmp]      ; load it to eax
        push eax            ; push as argument
        call my_e_to		; calculate e^-x
        add esp, 4
        
        fld DWORD [tmp]	    ; flip sign of argument again
        fchs			    ;
        fst DWORD [tmp]	    ;

        mov eax , [tmp]
        
        push eax
        call my_e_to  		; get e^x
        add esp, 4
                            
        fxch st1		    ; put them in order: 
        fxch st3            ; st1 = e^x, st0 = e^-x
        
        fsub	    		; get top half

        fld1    			; load 1
        fld1	    		; another 1
        faddp		    	; create 2
        fdivp			    ; divide st0 by it to get final result

        pop ebp
        ret

    my_sinhm1:    ; =ln(x + sqrt(x^2 + 1))
    ; result in st0
        push ebp
        mov ebp, esp

        finit

        mov eax, [ebp+0x8]	; load arg
        mov [tmp], eax


        fld DWORD [tmp]	    ; now load it to fpu stack
        fld DWORD [tmp]     ; twice
        fmul			    ; get it squared

        fld1			    ; insert 1
        faddp			    ; add it to x^2 and get rid of it
        fsqrt			    ; sqrt(x^2+1)
        fld DWORD [tmp]	    ; load x
        faddp			    ; add it to root
        fst DWORD [tmp]	    ; store ln's arg in memory
        mov eax, [tmp]      
        push eax
        call my_ln	    	; cal my_ln(x+sqrt(x^2+1)
        add esp, 4
        pop ebp
        ret
section .data
tmp dd 0.0
