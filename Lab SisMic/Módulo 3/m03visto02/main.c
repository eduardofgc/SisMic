#include <msp430.h>
#include <stdint.h>

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    P4MAP3 = PM_TB0CCR1A;
    P4MAP7 = PM_TB0CCR2A;
    PMAPKEYID = 0;

    P4DIR |= BIT3 | BIT7;
    P4SEL |= BIT3 | BIT7;

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    TB0CCR0 = 255;
    TB0CCTL1 = OUTMOD_7;
    TB0CCTL2 = OUTMOD_7;
    TB0CCR1 = 127;
    TB0CCR2 = 127;

    TB0CCTL0 |= CCIE;
    TB0CCTL1 |= CCIE;

    TB0EX0 = TBIDEX_4;
    TB0CTL = TBSSEL_2 | ID_3 | MC_1 | TBCLR;

    P6SEL |= BIT0 | BIT1;

    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT0_2 | ADC12MSC | ADC12ON;

    ADC12CTL1 = ADC12SHP | ADC12SHS_1 | ADC12CONSEQ_1;
    ADC12CTL2 = ADC12RES_0;

    ADC12MCTL0 = ADC12INCH_0;
    ADC12MCTL1 = ADC12INCH_1 | ADC12EOS;

    ADC12IE = BIT1;
    ADC12CTL0 |= ADC12ENC;

    TA0CCR0 = 654;
    TA0CCTL1 = OUTMOD_3;
    TA0CCR1 = 327;
    TA0CTL = TASSEL_1 | MC_1 | TACLR;

    __enable_interrupt();

    for(;;) {
        __no_operation();
    }
}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void) {
    switch(ADC12IV) {
        case ADC12IV_ADC12IFG1:
            TB0CCR1 = ADC12MEM0;
            TB0CCR2 = ADC12MEM1;

            ADC12CTL0 &= ~ADC12ENC;
            ADC12CTL0 |= ADC12ENC;
            break;
        default:
            break;
    }
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void TB0_CCR0_ISR(void) {
    if (TB0CCR1 != 0)
        P1OUT |= BIT0;
}

#pragma vector=TIMER0_B1_VECTOR
__interrupt void TB0_CCRx_ISR(void) {
    switch(TB0IV) {
        case 0x02:
            if (TB0CCR1 < TB0CCR0)
                P1OUT &= ~BIT0;
            break;
        default:
            break;
    }
}