	.cdecls "msp430.h"
	.global main

	.text

main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL

	;
	; CODIGO AQUI
	;

	push R4
	push R5
	push R6

	mov.b #4, R4
	mov.b #8, R5
	mov.b #0, R6

	calla #multiplicar

	pop R6
	pop R5
	pop R4
	
	jmp $

multiplicar:
	tst.b R4
	jz fim

	add.b R5, R6
	dec R4

	jmp multiplicar

fim:
	reta





	;
	; CODIGO AQUI
	;

	jmp $
	nop