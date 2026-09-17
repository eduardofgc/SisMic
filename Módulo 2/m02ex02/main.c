#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	//BOTAO 2.1
	P2DIR &= ~BIT1;
	P2REN |= BIT1;
	P2OUT |= BIT1;

	//LED 4.7
	P4DIR |= BIT7;
	P4OUT &= ~BIT7;

	while(1){
		if (!(P2IN & BIT1)){
			
			P4OUT ^= BIT7;
		}
	}
	
	return 0;
}
