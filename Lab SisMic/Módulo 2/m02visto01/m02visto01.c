#include <msp430.h>

volatile void debouce(volatile unsigned int tempo){
    volatile int i;
    for (i = 0; i <= tempo; i++);
    return;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    
    P2DIR &= ~BIT1; 
    P2REN |= BIT1;    
    P2OUT |= BIT1;   //pull up

    
    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;

    // vermelho
    P1DIR |= BIT0;  
    P1OUT &= ~BIT0;   

   // verde 
    P4DIR |= BIT7;    
    P4OUT &= ~BIT7;  

    while(1)
    {
        // vermelho
        if ((P2IN & BIT1) == 0) {
            P1OUT ^= BIT0;              
            debouce(10000);
            while ((P2IN & BIT1) == 0); 
            debouce(10000);
        }

        // verde
        if ((P1IN & BIT1) == 0) {
            P4OUT ^=BIT7;  
            debouce(10000);
            while ((P1IN & BIT1) == 0); 
            debouce(10000);
        }
    }
}
