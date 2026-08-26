	.cdecls "msp430.h"
	.global main

	.data
vetor:
	.word 2, 4, 7, 1, 5 ; VETOR DE TAMANHO 5


	.text

main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL

	calla #extremais ; CALLA PQ TA DANDO ERRO DE OVERFLOW

	jmp $

extremais:

	;
	; CODIGO AQUI
	;

	push R4
	push R5
	push R6
	push R7
	push R8

	mov.w #vetor, R4
	mov.w #5, R5 ; TAMANHO DO VETOR

	mov.w @R4+, R6 ; LE PROXIMO

	mov.w R6, R7 ; MAIOR R7
	mov.w R6, R8 ; MENOR R8

	dec.w R5

extremos:

	tst.w R5
	jz fim	

	mov.w @R4+, R6 ; LE PROXIMO

	cmp.w R8, R6
	jge eh_maior
	
	mov.w R6, R8

eh_maior:

	cmp.w R6, R7
	jge skip

	mov.w R6, R7

	dec R5
	jmp extremos

skip:
	dec R5
	jmp extremos

fim:

	mov.w R8, R12
	mov.w R7, R13

	pop R8
	pop R7
	pop R6
	pop R5
	pop R4

	ret
	;
	; CODIGO AQUI
	;

	jmp $
	nop
