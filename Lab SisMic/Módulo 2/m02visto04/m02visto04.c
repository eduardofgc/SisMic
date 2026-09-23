#include <msp430.h>

#define MOTOR BIT0

volatile int posicao = 90;
volatile int direcao = 5;
volatile int comparacao;
volatile unsigned int velocidade = 10;
volatile unsigned int contadorS1 = 0;
volatile unsigned int contadorS2 = 0;
volatile unsigned char autoS1 = 0;  
volatile unsigned char autoS2 = 0; 


#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void){

	//s1 apertado
	if (!(P2IN & BIT1)) {
		autoS1 = 0;                         
		autoS2 = 0;
		if (posicao > 0) posicao--;
		contadorS1++;
	}
	else {
		if (contadorS1 >= 30) autoS1 = 1;    
		contadorS1 = 0;
	}

	//s2 apertado
	if (!(P1IN & BIT1)) {
		autoS1 = 0;
		autoS2 = 0;
		if (posicao < 180) posicao++;
		contadorS2++;
	}
	else {
		if (contadorS2 >= 30) autoS2 = 1;
		contadorS2 = 0;
	}

	//one-touch
	if (autoS1) {
		if (posicao > 0) posicao--;
		else autoS1 = 0;
	}
	if (autoS2) {
		if (posicao < 180) posicao++;
		else autoS2 = 0;
	}

	TA1CCR1 = 500 + ((unsigned long)posicao * 2000) / 180;
}

void main(void){
	WDTCTL = WDTPW | WDTHOLD;
	
	
	P2DIR |= MOTOR; 
	P2SEL |= MOTOR; 

	//chave S1
	P2DIR &= ~BIT1;
	P2REN |= BIT1;
	P2OUT |= BIT1;

	//chave S2
	P1DIR &= ~BIT1;
	P1REN |= BIT1;
	P1OUT |= BIT1;

	// Configurar Timer A1
	TA1CTL = TASSEL__SMCLK | MC__UP; // Usar SMCLK (1 MHz) e modo UP
	TA1CCR0 = 20000 - 1; // Período 20 ms
	TA1CCTL1 = OUTMOD_7; // Colocar TA1.1 no modo Reset/Set
	TA1CCR1 = 1500-1; // Pulso de 1,5 ms (90 graus), tente outros

	//timer A0
	TA0CCR0  = 3277 - 1;
	TA0CCTL0 = CCIE;                           
	TA0CTL   = TASSEL__ACLK | MC__UP | TACLR;  

	__enable_interrupt();


	for(;;){
	}
}