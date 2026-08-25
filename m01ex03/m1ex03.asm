	.cdecls "msp430.h"
	.global main

	.text

main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL

	;
	; CODIGO AQUI
	;

	mov.w #0xFFF3, R4
	mov.w #0xFFF1, R5

	add.w R4, R5
	jc ehmaior
	jmp $

ehmaior:
	mov #0xFFFF, R5
	jmp $
	

	;
	; CODIGO AQUI
	;

	jmp $
	nop