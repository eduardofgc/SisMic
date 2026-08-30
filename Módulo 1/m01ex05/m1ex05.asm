	.cdecls "msp430.h"
	.global main

	.text

main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL

	;
	; CODIGO AQUI
	;

	mov.b #5, R4
	mov.b #-6, R5
	add.b R4, R5

	jn negativo
	jz zero

positivo:
	add.b #1, R5
	jmp fim

negativo:
	sub.b #1, R5
	jmp fim

zero:
	jmp fim
	
	;
	; CODIGO AQUI
	;

fim:
	jmp $
	nop