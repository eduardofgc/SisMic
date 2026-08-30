	.cdecls "msp430.h"
	.global main

	.text

main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL

	;
	; CODIGO AQUI
	;

	mov.b #3, R4
	mov.b #5, R5
	mov.b R5, R6

mult:
	dec R4
	jz fim
	jn fim
	
	add R6, R5
	jmp mult

fim:

	
	;
	; CODIGO AQUI
	;

	jmp $
	nop