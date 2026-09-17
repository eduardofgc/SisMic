#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	//botao s1 é o pino 2.1
	P2DIR &= ~BIT1; //INPUT
	P2REN |= BIT1; //ADD RESISTOR
	P2OUT |= BIT1; //FAZ RESISTOR PULL-UP

	//led verde é o pino 4.7
	P4DIR |= BIT7;
	P4OUT &= ~BIT7;

	while(1){
		if (!(P2IN & BIT1)){
			P4OUT |= BIT7;
		}
		 else {
			P4OUT &= ~BIT7;
		}
	}


	return 0;
}
