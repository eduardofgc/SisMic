#include <msp430.h> 

void setRedLed(){
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
}

void setS1(){
	P2DIR &= ~BIT1;
	P2REN |= BIT1;
	P2OUT |= BIT1;
}

void setS2(){
	P1DIR &= ~BIT1;
	P1REN |= BIT1;
	P2OUT |= BIT1;
}

void debounce (unsigned int time){
	volatile int n;

	for (n = 0; n < time; n++);
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	setRedLed();
	setS1();
	setS2();

	while (1){
		if (!(P2IN & BIT1) || !(P1IN & BIT1)){
			debounce(100000);
			P1OUT ^= BIT0;
		}
	}



	return 0;
}
