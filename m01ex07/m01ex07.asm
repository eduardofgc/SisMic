	.cdecls "msp430.h"
	.global main

	.text



main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL

	;
	; CODIGO AQUI
	;

	mov.w #0x2400, R4 ; ENDERECO DA SEQUENCIA
	mov.b #20, R5 ; NUMERO DE DIGITOS

	mov.b #0, 0(R4)
	dec R5
	inc R4

	mov.b #1, 0(R4)
	dec R5
	inc R4

	mov.b #0, R10
	mov.b #1, R11

	calla #fib

	jmp $

fib:
	tst.b R5
	jz fim


	add.b R11, R10
	mov.b R10, 0(R4)
	
	mov.b R11, R10
	mov.b @R4+, R11;

	dec R5

	jmp fib

fim:
	reta

	;
	; CODIGO AQUI
	;

	jmp $
	nop