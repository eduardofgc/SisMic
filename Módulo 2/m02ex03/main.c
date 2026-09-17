#include <msp430.h> 


/**
 * main.c
 */

void debounce(unsigned int time){
	volatile int n;

	for (n = 0; n < time; time++);
}

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
			debounce(100000);
			P4OUT ^= BIT7;
		}
	}
	
	return 0;
}