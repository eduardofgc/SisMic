	.cdecls "msp430.h"
	.global main

	.text

main:
	mov.w #(WDTPW|WDTHOLD), &WDTCTL

	;
	; CODIGO AQUI
	;

high_AvgPack:
    push    R4
    push    R5
    push    R6
    push    R7
    push    R8
    push    R9

    mov.b   #0x70, R4
    mov.w   R12, R5

    mov.w   R12, R6

loop:
    mov.w   @R6, R7
    mov.w   R6, R10
    add.w   #2, R10
    mov.w   R10, R8

    call    #S8_avg

    cmp.b   R4, R9
    jz      no
    jn      no

    mov.b   R9, R4
    mov.w   R6, R5

no:
    call    #GetNext

    dec     R13
    tst.b   R13
    jz      fim
    jmp     loop

fim:
    mov.w   R5, R12

    pop     R9
    pop     R8
    pop     R7
    pop     R6
    pop     R5
    pop     R4

    ret
	;
	; CODIGO AQUI
	;

	jmp $
	nop