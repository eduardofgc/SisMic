	.cdecls "msp430.h"
	.global main
	.text
main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL
	mov #matriz, R12 ; Ponteiro para a matriz de entrada
	mov #2, R13 ; Número de linhas da matriz
	mov #3, R14 ; Número de colunas da matriz

	calla #MAT_TRANSP ; Chamar sub-rotina

	jmp $ ; Loop infinito
	nop

MAT_TRANSP:

	push R4
	push R5
	push R6
	push R7
	push R8
	push R9
	push R15

	mov R12, R6
	mov R12, R15 ; OUTBASE


; -------------------------- CALCULA ONDE COMECA A MATRIZ TRANSP

	mov R13, R4 ; LINHAS (inicializado UMA vez, fora do laço)

	mov R13, R4        ; LINHAS

linhas:
	mov R14, R5        ; reseta contador de colunas — só 1x por linha
colunas:
	incd R15

	dec R5
	jnz colunas

	dec R4
	jnz linhas

; ------------------------------- R15 COMECO MAT TRANSP

	mov R13, R9
	add R9, R9 ; PRA PULAR 1 WORD

	mov R15, R7
	mov R13, R4

linhas_transp:
	mov R7, R8
	mov R14, R5

colunas_transp:

	mov @R6+, 0(R8)
	add R9, R8

	dec R5
	jnz colunas_transp

	incd R7
	dec R4
	jnz linhas_transp

	pop     R15
    pop     R9
    pop     R8
    pop     R7
    pop     R6
    pop     R5
    pop     R4

    reta
	; Seu código aqui
	; Especificar a matriz de entrada na seção de dados

	.data

matriz:
	.word 1, 2, 3, 4, 5, 6