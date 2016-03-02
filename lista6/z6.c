/*
COMPILE: nasm -f elf z6.asm && gcc z6.c z6.o -o z6
DEBUG: nasm -f elf -g -F dwarf z6.asm && gcc -g z6.c z6.o -o z6
*/
#include <stdio.h>

extern float my_ln(float);
extern float my_e_to(float);
extern float my_sinh(float);
extern float my_sinhm1(float);

int main()
{
	printf("%f\n", my_ln(3.14));
	printf("%f\n", my_e_to(3.14));
	printf("%f\n", my_sinh(3.14));
	printf("%f\n", my_sinhm1(3.14));
}