#include <msp430.h>

#define S1 BIT1                                  // S1 em P2.1

void debounce(unsigned int t) {                  // atraso para esperar os rebotes
    volatile unsigned int n;                     // volatile: impede o compilador de apagar o laço
    for (n = 0; n < t; n++);                     // só gasta tempo
}

void uart_init(void) {                           // configura a USCI_A1 como UART 9600 8N1
    UCA1CTL1 |= UCSWRST;                         // segura a USCI em reset para configurar
    UCA1CTL1 |= UCSSEL__SMCLK;                   // clock da UART = SMCLK (1.048.576 Hz)
    UCA1BR0 = 109;                               // divisor: 1.048.576 / 9600 ≈ 109,2 → parte inteira
    UCA1BR1 = 0;                                 // byte alto do divisor
    UCA1MCTL = UCBRS_2 | UCBRF_0;                // modulação que compensa a parte fracionária (0,2)
    P4SEL |= BIT4 | BIT5;                        // P4.4 = TXD e P4.5 = RXD (ligados ao USB da placa)
    UCA1CTL1 &= ~UCSWRST;                        // libera a USCI para funcionar
}

void uart_putc(char c) {                         // envia um caractere
    while (!(UCA1IFG & UCTXIFG));                // espera o buffer de transmissão ficar livre
    UCA1TXBUF = c;                               // escreve o caractere, o hardware transmite
}

void uart_puts(const char *s) {                  // envia uma string
    while (*s)                                   // até chegar no '\0'
        uart_putc(*s++);                         // manda caractere por caractere
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;                    // desliga o watchdog

    P2DIR &= ~S1;                                // S1 como entrada
    P2REN |= S1;                                 // habilita resistor interno
    P2OUT |= S1;                                 // pull-up (solto = 1, pressionado = 0)

    uart_init();                                 // configura a UART

    while (1) {
        if (!(P2IN & S1)) {                      // S1 pressionado
            debounce(3000);                      // espera os rebotes do aperto
            uart_puts("S1 foi pressionado.\r\n"); // envia a frase (\r\n = nova linha)
            while (!(P2IN & S1));                // espera soltar
            debounce(3000);                      // espera os rebotes da soltura
        }
    }
}