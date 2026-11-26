#include <msp430.h>
#include <stdint.h>

#define LCD_ADDR 0x27                            // PCF8574T = 0x27, PCF8574AT = 0x3F

#define LCD_RS BIT0
#define LCD_RW BIT1
#define LCD_EN BIT2
#define LCD_BL BIT3

#define DELAY_MS(x) __delay_cycles((x) * 1049UL)

#define V1 64                                    // abaixo: joystick para um lado
#define V2 192                                   // acima: joystick para o outro lado

const char *frases[5] = {
    "Frase 1:\nOla, mundo!",
    "Frase 2:\nMSP430 na UnB",
    "Frase 3:\nLab de SisMic",
    "Frase 4:\nEng. Computacao",
    "Frase 5:\nFim da lista"
};

volatile uint8_t leitura;                        // último valor do ADC
volatile uint8_t novaLeitura = 0;                // avisa o main que chegou amostra

/* ===================== I2C ===================== */

void i2cInit(void) {
    UCB0CTL1 = UCSWRST;
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    UCB0CTL1 = UCSSEL__SMCLK | UCSWRST;
    UCB0BR0 = 11;
    UCB0BR1 = 0;

    P3SEL |= BIT0 | BIT1;
    P3DIR &= ~(BIT0 | BIT1);
    P3REN |= BIT0 | BIT1;
    P3OUT |= BIT0 | BIT1;

    UCB0CTL1 &= ~UCSWRST;
}

uint8_t i2cSend(uint8_t addr, uint8_t data) {
    uint8_t nack;

    UCB0I2CSA = addr;
    UCB0CTL1 |= UCTR | UCTXSTT;
    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = data;
    while (UCB0CTL1 & UCTXSTT);

    nack = (UCB0IFG & UCNACKIFG) ? 1 : 0;
    if (!nack)
        while (!(UCB0IFG & UCTXIFG));

    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP);
    UCB0IFG &= ~UCNACKIFG;
    return nack;
}

/* ===================== LCD ===================== */

void lcdWriteNibble(uint8_t nibble, uint8_t isChar) {
    uint8_t b = (nibble << 4) | LCD_BL | (isChar ? LCD_RS : 0);

    i2cSend(LCD_ADDR, b);
    i2cSend(LCD_ADDR, b | LCD_EN);
    i2cSend(LCD_ADDR, b);
}

void lcdWriteByte(uint8_t byte, uint8_t isChar) {
    lcdWriteNibble(byte >> 4, isChar);
    lcdWriteNibble(byte & 0x0F, isChar);
}

void lcdClear(void) {
    lcdWriteByte(0x01, 0);
    DELAY_MS(2);
}

void lcdInit(void) {
    DELAY_MS(50);
    lcdWriteNibble(0x3, 0); DELAY_MS(5);
    lcdWriteNibble(0x3, 0); DELAY_MS(1);
    lcdWriteNibble(0x3, 0); DELAY_MS(1);
    lcdWriteNibble(0x2, 0); DELAY_MS(1);

    lcdWriteByte(0x28, 0);
    lcdWriteByte(0x0C, 0);
    lcdWriteByte(0x06, 0);
    lcdClear();
}

void lcdWrite(char *str) {
    uint8_t col = 0, lin = 0;

    while (*str) {
        if (*str == '\n' || col == 16) {
            if (lin == 1) break;
            lcdWriteByte(0xC0, 0);
            lin = 1;
            col = 0;
            if (*str == '\n') { str++; continue; }
        }
        lcdWriteByte(*str++, 1);
        col++;
    }
}

void mostrarFrase(int i) {
    lcdClear();
    lcdWrite((char *)frases[i]);
}

/* ===================== ADC + Timer ===================== */

void adcInit(void) {
    P6SEL |= BIT0;                               // P6.0 = A0 (eixo X do joystick)

    ADC12CTL0 &= ~ADC12ENC;                      // libera a configuração
    ADC12CTL0 = ADC12SHT0_0 | ADC12ON;           // amostra 4 ciclos (≥ t_cap), liga o ADC
    ADC12CTL1 = ADC12SHP | ADC12SHS_1 |          // timer de amostragem, gatilho = TA0.1
                ADC12SSEL_3 | ADC12CONSEQ_2;     // clock = SMCLK, canal único repetido
    ADC12CTL2 = ADC12RES_0;                      // 8 bits (0 a 255)
    ADC12MCTL0 = ADC12INCH_0;                    // MEM0 ← A0
    ADC12IE = BIT0;                              // interrupção quando MEM0 for preenchido
    ADC12CTL0 |= ADC12ENC;                       // arma o ADC
}

void timerInit(void) {
    TA0CCR0 = 3277 - 1;                          // período de 100 ms com ACLK
    TA0CCR1 = 1638;                              // borda de subida no meio do período
    TA0CCTL1 = OUTMOD_3;                         // set/reset: uma borda de subida por período
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;      // ACLK, modo up
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void) {
    switch (ADC12IV) {
        case ADC12IV_ADC12IFG0:                  // nova amostra em MEM0
            leitura = ADC12MEM0;                 // guarda (e limpa a flag)
            novaLeitura = 1;
            __bic_SR_register_on_exit(LPM0_bits);// acorda o main
            break;
        default:
            break;
    }
}

/* ===================== Programa ===================== */

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    i2cInit();
    lcdInit();
    adcInit();
    timerInit();

    int atual = 0;
    int zonaAnterior = 0;                        // -1 = um lado, 0 = neutro, 1 = outro lado
    mostrarFrase(atual);

    while (1) {
        __bis_SR_register(LPM0_bits | GIE);      // dorme até chegar uma amostra

        if (novaLeitura) {
            novaLeitura = 0;

            int zona;
            if (leitura < V1)      zona = -1;    // um lado
            else if (leitura > V2) zona = 1;     // outro lado
            else                   zona = 0;     // neutro

            if (zona != 0 && zonaAnterior == 0) {             // saiu do neutro agora
                if (zona == 1) atual = (atual == 4) ? 0 : atual + 1; // próxima
                else           atual = (atual == 0) ? 4 : atual - 1; // anterior
                mostrarFrase(atual);
            }
            zonaAnterior = zona;
        }
    }
}