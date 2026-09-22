#include <msp430.h> 

#define MOTOR BIT0

volatile int posicao = 0;
volatile int direcao = 5;
volatile int comparacao;
volatile unsigned int velocidade = 10;
volatile unsigned int contador = 0;


#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void){

    // lógica dos passos aqui

	contador++;

	if (velocidade == 10) comparacao = 2;
	else if (velocidade == 20) comparacao = 1;

	if (contador % comparacao != 0) return;

	if (P2IN & BIT1) return;

	if (direcao >= 0){
		posicao++;
	}
	else{
		posicao--;
	}

	if (direcao < 0 && posicao == 0){
		direcao = 0 - direcao;
	}
	else if (direcao >= 0 && posicao == 180){
		direcao = 0 - direcao;
	}

	TA1CCR1 = 500 + ((unsigned long)posicao * 2000) / 180;

}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	//chave S1
	P2DIR &= ~BIT1;
	P2REN |= BIT1;
	P2OUT |= BIT1;

	//chave S2
	P1DIR &= ~BIT1;
	P1REN |= BIT1;
	P1OUT |= BIT1;

	//timer A0
	TA0CCR0  = 1638 - 1;                       
    TA0CCTL0 = CCIE;            
    TA0CTL   = TASSEL__ACLK | MC__UP | TACLR;  
	
	P2DIR |= MOTOR;                     // P2.0 como saída
	P2SEL |= MOTOR;                     // P2.0 controlado pelo timer (TA1.1)

	TA1CCR0  = 20000 - 1;               // período: 20000 µs = 20 ms
	TA1CCTL1 = OUTMOD_7;                // Reset/Set
	TA1CCR1  = 500;                     // largura do pulso: 500 µs = 0°
	TA1CTL   = TASSEL__SMCLK | MC__UP | TACLR;


	__enable_interrupt();



	unsigned char s2Anterior = P1IN & BIT1;   // estado inicial de S2

	for (;;){
		unsigned char s2 = P1IN & BIT1;       // 0 = fechada, != 0 = aberta

		if (s2 != s2Anterior){                // houve mudança de estado
			if (s2Anterior == 0 && s2 != 0){  // fechada -> aberta: acionou
				if (velocidade == 10) velocidade = 20;
				else velocidade = 10;
			}
			__delay_cycles(20000);            // debounce (~20 ms)
		}

		s2Anterior = s2;
	}
}
