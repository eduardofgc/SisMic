#include <msp430.h>

#define MOTOR BIT0


void debounce(unsigned int time){
	volatile unsigned int n;
	for (n = 0; n < time; n++);
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

	P2DIR &= ~BIT1;
	P2REN |= BIT1;
	P2OUT |= BIT1;

	P1DIR &= ~BIT1;
	P1REN |= BIT1;
	P1OUT |= BIT1;

    // Usar pino P2.0 para o PWM (Canal 1 do Timer A1)
    P2DIR |= MOTOR;             // P2.0 como saída
    P2SEL |= MOTOR;             // Selecionar a função alternativa do pino (TA1.1)

    // Configurar Timer A1
    TA1CTL = TASSEL__SMCLK | MC__UP; // Usar SMCLK (1 MHz) e modo UP
    TA1CCR0 = 20000 - 1;        // Período 20 ms
    TA1CCTL1 = OUTMOD_7;        // Colocar TA1.1 no modo Reset/Set
    TA1CCR1 = 1500-1;           // Pulso de 1,5 ms (90 graus), tente outros

	while (1){
		if (!(P2IN & BIT1)){
			debounce(3000);

			if (TA1CCR1 < 2500 - 100){
				TA1CCR1 += 100; 

				while (!(P2IN & BIT1));
				debounce(3000);

			}
		}

		if (!(P1IN & BIT1)){
			debounce(3000);

			if (TA1CCR1 > 500 - 100){
				TA1CCR1 -= 100;

				while (!(P1IN & BIT1));
				debounce(3000);
			}
		}
	}

    for(;;){
        __no_operation();
    }
}