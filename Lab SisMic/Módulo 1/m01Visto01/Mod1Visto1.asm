	.cdecls "msp430.h"
	.global main

	.data
vetor:
	.byte 1, 2, 3, 4, 5, 6, 7, 8


	.text

main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL

	;
	; CODIGO AQUI
	;

	mov.w #vetor, R12
	mov.b #8, R13

	call #reduceSum8

	jmp $

reduceSum8:

    push R4
    mov.b #0, R4

subloop:

    tst.b R13
    jz fim

    add.b @R12, R4      
    add.w #1, R12       
    dec.b R13
    jmp subloop

fim:

    mov.b R4, R12        
    pop R4

    ret


	;
	; CODIGO AQUI
	;

	jmp $
	nop

	.end