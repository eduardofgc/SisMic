	.cdecls "msp430.h"
	.global main

	.data

string: 
	.byte 'E', 'd', 'u', 'a', 'r', 'd', 'o'

sigla:
	.space 8


	.text

main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL

	;
	; CODIGO AQUI
	;

	calla #geraSigla

	jmp $

geraSigla:
	push R4
	push R5
	push R10
	
	mov.w #string, R4
	mov.w #sigla, R5
	mov.b #7, R10


subloop:
	tst.b R10
	jz fim

	mov.b @R4+, R6

	cmp.b #'A', R6
	jn fim

	cmp.b #('Z'+1), R6
	jn upperCaseOkay
	
	dec R10
	jmp subloop

upperCaseOkay:
	mov.b R6, 0(R5)
	inc R5

	dec R10
	jmp subloop
fim:

	pop R10
	pop R5
	pop R4

	reta
	;
	; CODIGO AQUI
	;

	jmp $
	nop