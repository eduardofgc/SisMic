	.cdecls "msp430.h"
	.global main

	.text

main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL

	;
	; CODIGO AQUI
	;


	
funcao:  ; R12 = VETOR, TAMANHO = R13, MIN = R14, MAX = R15
	push R4 ; TAMANHO
	mov.b R13, R4

subloop:
	tst.b R4
	jz fim

	cmp.b R14, 0(R12)
	jn minimo

	cmp.b 0(R12), R15
	jn maximo

	jmp proximo

minimo:
	mov.b R14, 0(R12)
	jmp proximo

maximo:
	mov.b R15, 0(R12)
	jmp proximo

proximo:
	dec.b R4
	inc.w R12

	jmp subloop

fim:
	pop R4
	ret


	;
	; CODIGO AQUI
	;

	jmp $
	nop