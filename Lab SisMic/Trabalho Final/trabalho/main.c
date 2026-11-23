#include <msp430.h>

#define LCD_ADDR 0x27
#define SDA BIT0            // P6.0
#define SCL BIT1            // P6.1
#define LCD_RS 0x01
#define LCD_EN 0x04
#define LCD_BL 0x08

#define SECO      4075      // ADC com sensor seco, no ar
#define MOLHADO   1700      // ADC com sensor na água
#define ALVO_FIXO 50        // umidade desejada ao ligar, em %

#define RELE_BIT     BIT0   // P2.0 -> LED vermelho em série -> IN do módulo
                            // (perna longa no IN, perna curta no P2.0, sem resistor)

// ---------- Relé ----------
// LOW  no P2.0: o LED conduz, o IN vai perto de 0 V   -> relé liga
// HIGH no P2.0: o LED bloqueia, o módulo sobe o IN para 5 V -> relé desliga
static void rele(unsigned char ligar) {
    if (ligar) P2OUT &= ~RELE_BIT;
    else       P2OUT |=  RELE_BIT;
    P2DIR |= RELE_BIT;
}

// ---------- I2C por software ----------
static void i2c_delay(void) { __delay_cycles(20); }
static void sda_alto(void)  { P6DIR &= ~SDA; }
static void sda_baixo(void) { P6DIR |= SDA; }
static void scl_alto(void)  { P6DIR &= ~SCL; }
static void scl_baixo(void) { P6DIR |= SCL; }

static void i2c_start(void) {
    sda_alto(); scl_alto(); i2c_delay();
    sda_baixo(); i2c_delay();
    scl_baixo();
}
static void i2c_stop(void) {
    sda_baixo(); i2c_delay();
    scl_alto(); i2c_delay();
    sda_alto(); i2c_delay();
}
static void i2c_byte(unsigned char b) {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        if (b & 0x80) sda_alto(); else sda_baixo();
        i2c_delay(); scl_alto(); i2c_delay(); scl_baixo();
        b <<= 1;
    }
    sda_alto(); i2c_delay();
    scl_alto(); i2c_delay(); scl_baixo();
}
static void pcf_write(unsigned char d) {
    i2c_start();
    i2c_byte(LCD_ADDR << 1);
    i2c_byte(d);
    i2c_stop();
}

// ---------- LCD 16x2, modo 4 bits ----------
static void lcd_nibble(unsigned char n, unsigned char rs) {
    unsigned char d = (n & 0xF0) | LCD_BL | rs;
    pcf_write(d | LCD_EN);
    __delay_cycles(100);
    pcf_write(d);
    __delay_cycles(100);
}
static void lcd_byte(unsigned char b, unsigned char rs) {
    lcd_nibble(b & 0xF0, rs);
    lcd_nibble((b << 4) & 0xF0, rs);
}
static void lcd_cmd(unsigned char c) {
    lcd_byte(c, 0);
    __delay_cycles(3000);
}
static void lcd_init(void) {
    __delay_cycles(80000);
    lcd_nibble(0x30, 0); __delay_cycles(6000);
    lcd_nibble(0x30, 0); __delay_cycles(300);
    lcd_nibble(0x30, 0); __delay_cycles(300);
    lcd_nibble(0x20, 0); __delay_cycles(300);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
}
// Ressincroniza o LCD (sequência de reset por instrução do HD44780).
// Funciona a partir de qualquer estado: LCD reiniciado por queda de tensão,
// fio desconectado/reconectado, ou nibble perdido no meio de uma escrita.
static void lcd_resync(void) {
    lcd_nibble(0x30, 0); __delay_cycles(5000);
    lcd_nibble(0x30, 0); __delay_cycles(300);
    lcd_nibble(0x30, 0); __delay_cycles(300);
    lcd_nibble(0x20, 0); __delay_cycles(300);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
}
static void lcd_clear(void) { lcd_cmd(0x01); }
static void lcd_cursor(unsigned char linha, unsigned char col) {
    lcd_cmd(0x80 | (linha ? 0x40 : 0x00) | col);
}
static void lcd_print(const char *s) {
    while (*s) lcd_byte(*s++, LCD_RS);
}
static void lcd_print_num(unsigned int n) {      // 3 caracteres, alinhado à direita
    char c[4];
    c[0] = (n >= 100) ? '0' + (n / 100) : ' ';
    c[1] = (n >= 10)  ? '0' + (n / 10) % 10 : ' ';
    c[2] = '0' + (n % 10);
    c[3] = 0;
    lcd_print(c);
}
static void lcd_print_num4(unsigned int n) {     // 4 dígitos (valor bruto do ADC)
    char c[5];
    c[0] = '0' + (n / 1000) % 10;
    c[1] = '0' + (n / 100) % 10;
    c[2] = '0' + (n / 10) % 10;
    c[3] = '0' + n % 10;
    c[4] = 0;
    lcd_print(c);
}

// ---------- ADC ----------
static unsigned int ler_adc(void) {
    ADC12CTL0 |= ADC12SC;
    while (ADC12CTL1 & ADC12BUSY);
    return ADC12MEM0;
}
static unsigned int ler_adc_media(void) {
    unsigned long soma = 0;
    unsigned char i;
    for (i = 0; i < 8; i++) soma += ler_adc();
    return (unsigned int)(soma / 8);
}
static unsigned int adc_para_pct(unsigned int adc) {
    long pct = ((long)SECO - adc) * 100 / (SECO - MOLHADO);
    if (pct < 0)   pct = 0;
    if (pct > 100) pct = 100;
    return (unsigned int)pct;
}

int main(void) {
    unsigned int alvo, umid, adc;
    unsigned char bomba = 0, s1, s2, s1_ant = 0, s2_ant = 0;

    WDTCTL = WDTPW | WDTHOLD;

    rele(0);                            // começa com o relé desligado

    // Botões S1 (P2.1) e S2 (P1.1) com pull-up
    P2DIR &= ~BIT1;  P2REN |= BIT1;  P2OUT |= BIT1;
    P1DIR &= ~BIT1;  P1REN |= BIT1;  P1OUT |= BIT1;

    // I2C
    P6OUT &= ~(SDA | SCL);
    P6DIR &= ~(SDA | SCL);

    // ADC12: A2 (P6.2)
    P6SEL |= BIT2;
    ADC12CTL0 = ADC12SHT0_8 | ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12MCTL0 = ADC12INCH_2;
    ADC12CTL0 |= ADC12ENC;

    lcd_init();
    lcd_cursor(0, 0); lcd_print("Medindo...");

    adc  = ler_adc_media();
    umid = adc_para_pct(adc);
    alvo = ALVO_FIXO;                   // troque por "umid" para copiar a leitura atual ao ligar
    lcd_clear();

    while (1) {
        // Botões (ativos em LOW)
        s1 = !(P2IN & BIT1);
        s2 = !(P1IN & BIT1);

        if (s1 && s2) {
            if (!(s1_ant && s2_ant)) alvo = umid;   // os dois juntos: alvo = umidade atual
        } else if (s1 && !s1_ant && alvo < 100) {
            alvo += 5;
        } else if (s2 && !s2_ant && alvo > 0) {
            alvo = (alvo >= 5) ? alvo - 5 : 0;
        }
        s1_ant = s1;
        s2_ant = s2;

        // Leitura
        adc  = ler_adc_media();
        umid = adc_para_pct(adc);

        // Controle: abaixo do alvo rega, no alvo ou acima não rega
        bomba = (umid < alvo);

        rele(bomba);

        // Display: ressincroniza a cada ciclo e reescreve as 16 colunas das
        // duas linhas, assim qualquer lixo some sozinho em menos de 1 s
        lcd_resync();
        lcd_cursor(0, 0);  lcd_print("Umid:"); lcd_print_num(umid); lcd_print("%  ");
        lcd_print_num4(adc); lcd_print(" ");
        lcd_cursor(1, 0);  lcd_print("Alvo:"); lcd_print_num(alvo); lcd_print("%   ");
        lcd_print(bomba ? "ON  " : "OK  ");

        __delay_cycles(100000);
    }
}