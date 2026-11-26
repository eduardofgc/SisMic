#include <msp430.h>
#include <stdint.h>

#define LCD_ADDR 0x27                            // PCF8574T = 0x27, PCF8574AT = 0x3F

#define LCD_RS BIT0                              // bits do PCF8574 ligados ao LCD
#define LCD_RW BIT1
#define LCD_EN BIT2
#define LCD_BL BIT3

#define DELAY_MS(x) __delay_cycles((x) * 1049UL) // MCLK = 1.048.576 Hz

#define S1 BIT1                                  // P2.1
#define S2 BIT1                                  // P1.1

const char *frases[5] = {                        // \n quebra para a 2ª linha
    "Frase 1:\nfrase 1",
    "Frase 2:\nfrase 2",
    "Frase 3:\nfrase 3",
    "Frase 4:\nfrase 4",
    "Frase 5:\nultima frase :)"
};

/* ===================== I2C ===================== */

void i2cInit(void) {
    UCB0CTL1 = UCSWRST;                          // (1) reseta a interface
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;        // (2) mestre, I2C, síncrono
    UCB0CTL1 = UCSSEL__SMCLK | UCSWRST;          //     clock = SMCLK, mantém reset
    UCB0BR0 = 11;                                //     1.048.576 / 11 ≈ 95 kHz (< 100 kHz)
    UCB0BR1 = 0;

    P3SEL |= BIT0 | BIT1;                        // (3) P3.0 = SDA, P3.1 = SCL
    P3DIR &= ~(BIT0 | BIT1);                     //     entradas
    P3REN |= BIT0 | BIT1;                        //     resistores internos
    P3OUT |= BIT0 | BIT1;                        //     pull-up

    UCB0CTL1 &= ~UCSWRST;                        // (4) libera a interface
}

uint8_t i2cSend(uint8_t addr, uint8_t data) {    // retorna 0 = ACK, 1 = NACK
    uint8_t nack;

    UCB0I2CSA = addr;                            // 1. endereço do escravo
    UCB0CTL1 |= UCTR | UCTXSTT;                  // 2. transmissor + START
    while (!(UCB0IFG & UCTXIFG));                // 3. espera o buffer esvaziar...
    UCB0TXBUF = data;                            //    ...e escreve o byte
    while (UCB0CTL1 & UCTXSTT);                  // 4. espera o ciclo de ACK do endereço

    nack = (UCB0IFG & UCNACKIFG) ? 1 : 0;
    if (!nack)                                   // 5. escravo respondeu:
        while (!(UCB0IFG & UCTXIFG));            //    espera o byte ser transmitido

    UCB0CTL1 |= UCTXSTP;                         // 6. STOP
    while (UCB0CTL1 & UCTXSTP);                  // 7. espera o STOP sair
    UCB0IFG &= ~UCNACKIFG;                       //    limpa a flag para a próxima vez
    return nack;                                 // 8. ACK ou NACK
}

/* ===================== LCD ===================== */

void lcdWriteNibble(uint8_t nibble, uint8_t isChar) {
    uint8_t b = (nibble << 4) | LCD_BL | (isChar ? LCD_RS : 0); // D7-D4, backlight, RS, RW=0

    i2cSend(LCD_ADDR, b);                        // 1. dados e RS com EN = 0
    i2cSend(LCD_ADDR, b | LCD_EN);               // 2. EN = 1
    i2cSend(LCD_ADDR, b);                        // 3. EN = 0 → LCD lê o nibble
}

void lcdWriteByte(uint8_t byte, uint8_t isChar) {
    lcdWriteNibble(byte >> 4, isChar);           // MSNibble primeiro
    lcdWriteNibble(byte & 0x0F, isChar);         // depois o LSNibble
}

void lcdClear(void) {
    lcdWriteByte(0x01, 0);                       // limpa o display
    DELAY_MS(2);                                 // leva 1,53 ms
}

void lcdInit(void) {
    DELAY_MS(50);                                // espera o LCD energizar
    lcdWriteNibble(0x3, 0); DELAY_MS(5);         // 0x3 três vezes: garante modo 8 bits
    lcdWriteNibble(0x3, 0); DELAY_MS(1);
    lcdWriteNibble(0x3, 0); DELAY_MS(1);
    lcdWriteNibble(0x2, 0); DELAY_MS(1);         // passa para modo 4 bits

    lcdWriteByte(0x28, 0);                       // 4 bits, 2 linhas, fonte 5x8
    lcdWriteByte(0x0C, 0);                       // display ligado, sem cursor
    lcdWriteByte(0x06, 0);                       // cursor anda para a direita
    lcdClear();                                  // limpa e espera 1,53 ms
}

void lcdWrite(char *str) {                       // escreve string e gerencia a quebra de linha
    uint8_t col = 0, lin = 0;

    while (*str) {
        if (*str == '\n' || col == 16) {         // '\n' ou fim da linha
            if (lin == 1) break;                 // não há 3ª linha
            lcdWriteByte(0xC0, 0);               // DDRAM 0x40 = início da 2ª linha
            lin = 1;
            col = 0;
            if (*str == '\n') { str++; continue; }
        }
        lcdWriteByte(*str++, 1);                 // caractere (RS = 1)
        col++;
    }
}

void mostrarFrase(int i) {
    lcdClear();
    lcdWrite((char *)frases[i]);
}

/* ===================== Programa ===================== */

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1; P2REN |= S1; P2OUT |= S1;      // S1 com pull-up
    P1DIR &= ~S2; P1REN |= S2; P1OUT |= S2;      // S2 com pull-up

    i2cInit();
    lcdInit();

    int atual = 0;
    mostrarFrase(atual);                         // sempre exibe uma frase

    while (1) {
        if (!(P2IN & S1)) {                      // S1: aberta → fechada
            DELAY_MS(20);                        // debounce do aperto
            atual = (atual == 0) ? 4 : atual - 1;// anterior, da frase 1 volta para a 5
            mostrarFrase(atual);
            while (!(P2IN & S1));                // não muda enquanto segurar
            DELAY_MS(20);                        // debounce da soltura
        }

        if (!(P1IN & S2)) {                      // S2: aberta → fechada
            DELAY_MS(20);
            atual = (atual == 4) ? 0 : atual + 1;// próxima, da frase 5 volta para a 1
            mostrarFrase(atual);
            while (!(P1IN & S2));
            DELAY_MS(20);
        }
    }
}