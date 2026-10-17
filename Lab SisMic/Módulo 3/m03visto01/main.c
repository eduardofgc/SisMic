#include <msp430.h>

// Esta biblioteca permite usar o tipo uint8_t
#include <stdint.h>

// Endereço I2C do módulo LCD
#define LCD_I2C_ADDR 0x27 // talvez o seu seja 0x3F

// Definição dos pinos no PCF8574
// P7-P4: D7-D4 (dados)
// P3: BL (backlight)
// P2: EN (enable)
// P1: RW (read/write)
// P0: RS (register select)
#define LCD_RS      0x01    // 0: Comando, 1: Dados
#define LCD_RW      0x02    // 0: Escrita, 1: Leitura
#define LCD_EN      0x04    // Pulso de habilitação
#define LCD_BL      0x08    // Controle da luz de fundo
#define LCD_D4      0x10    // Bit de dados 4
#define LCD_D5      0x20    // Bit de dados 5
#define LCD_D6      0x40    // Bit de dados 6
#define LCD_D7      0x80    // Bit de dados 7

// Protótipos das funções
void initI2C_Master(void);
uint8_t i2cSend(uint8_t slaveAddr, uint8_t data);
uint8_t lcdWriteNibble (uint8_t nibble, uint8_t isChar);
uint8_t lcdWriteByte (uint8_t byte, uint8_t isChar);
void lcdInit(void);
void lcdWrite(char* str);


void main(void){
  WDTCTL = WDTPW | WDTHOLD;

  // Configura os pinos GPIO para I2C
  // UCB0: P3.0 = SDA, P3.1 = SCL
  P3SEL |= BIT0 | BIT1;                   // Atribui os pinos ao módulo I2C UCB0

  // Inicializa o módulo I2C
  initI2C_Master();

  // Esperar um pouco antes de realizar outra ação
  __delay_cycles(20000);

  // Inicializar LCD
  lcdInit();

  // Enviar string para ser escrita
  lcdWrite("algo bonitinho");

  // Loop infinito
  for(;;){
    __no_operation();
  }
}

void initI2C_Master(void) {
  UCB0CTL1 |= UCSWRST;                      // Reseta para iniciar a configuração
  UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;     // Mestre, I2C (MODE = 3), síncrono
  UCB0CTL1 = UCSSEL_2 | UCSWRST;            // Usa SMCLK (1 MHz), mantém reset
  UCB0BR0 = 10;                             // SMCLK/10 = 100 kHz
  UCB0BR1 = 0;
  UCB0CTL1 &= ~UCSWRST;                // Limpa reset para terminar a configuração
}

// Exercício 1: envia 1 byte para o escravo. Retorna 0 se ACK, 1 se NACK.
uint8_t i2cSend(uint8_t slaveAddr, uint8_t data) {
  uint8_t nack;

  UCB0I2CSA = slaveAddr;                    // 1. Endereço do escravo

  UCB0CTL1 |= UCTR | UCTXSTT;               // 2. Transmissor + START

  while (!(UCB0IFG & UCTXIFG));             // 3. Espera buffer de TX vazio
  UCB0TXBUF = data;                         //    Escreve o dado (destrava o ACK)

  while (UCB0CTL1 & UCTXSTT);               // 4. Espera o ciclo de ACK (STT -> 0)

  if (!(UCB0IFG & UCNACKIFG)) {             // 5. Se o escravo respondeu (ACK)
    while (!(UCB0IFG & UCTXIFG));           //    Espera o byte ser transmitido
  }

  UCB0CTL1 |= UCTXSTP;                      // 6. STOP
  while (UCB0CTL1 & UCTXSTP);               // 7. Espera o STOP ser enviado

  nack = (UCB0IFG & UCNACKIFG) ? 1 : 0;     // 8. ACK (0) ou NACK (1)
  UCB0IFG &= ~UCNACKIFG;                    // Limpa a flag para a próxima chamada

  return nack;
}

// Exercício 3: escreve 1 nibble (em D7-D4) com pulso de EN.
// isChar = RS (1: caractere, 0: instrução). Retorna 0 se tudo deu ACK.
uint8_t lcdWriteNibble (uint8_t nibble, uint8_t isChar){
  uint8_t data = ((nibble & 0x0F) << 4) | LCD_BL;   // D7-D4, backlight ligado, EN=0, RW=0
  uint8_t nack = 0;

  if (isChar) {
    data |= LCD_RS;
  }

  nack |= i2cSend(LCD_I2C_ADDR, data);              // Prepara RS e dados (EN=0)
  nack |= i2cSend(LCD_I2C_ADDR, data | LCD_EN);     // EN = 1
  nack |= i2cSend(LCD_I2C_ADDR, data);              // EN = 0

  return nack;
}

// Exercício 4: escreve 1 byte em dois nibbles (MSB primeiro).
uint8_t lcdWriteByte (uint8_t byte, uint8_t isChar){
  uint8_t nack = 0;

  nack |= lcdWriteNibble(byte >> 4, isChar);        // 4 bits mais significativos
  nack |= lcdWriteNibble(byte & 0x0F, isChar);      // 4 bits menos significativos

  return nack;
}

void lcdInit (void){
  // Enviar esse nibble três vezes para garantir que chegamos no modo de 8 bits
  uint8_t nibble = 3;

  lcdWriteNibble(nibble, 0);
  __delay_cycles(20000);
  lcdWriteNibble(nibble, 0);
  __delay_cycles(20000);
  lcdWriteNibble(nibble, 0);
  __delay_cycles(20000);

  // Na quarta vez, enviar enviar 2 para colocar no modo 4 bits
  nibble = 2;
  lcdWriteNibble(nibble, 0);
  __delay_cycles(20000);

  // Configurar display (display ligado, cursor piscando)
  uint8_t byte = 0x0F;
  lcdWriteByte(byte, 0);
  __delay_cycles(20000);

  // Apagar tudo e levar o cursor para o início
  byte = 0x01;
  lcdWriteByte(byte, 0);
  __delay_cycles(20000);
}

// Exercício 6: escreve uma string e quebra a linha (16 colunas x 2 linhas).
// Também aceita '\n' para pular para a segunda linha.
void lcdWrite(char* str){
  uint8_t col = 0;
  uint8_t line = 0;

  while (*str != '\0') {
    if (*str == '\n' || col == 16) {
      if (line == 1) {
        break;                              // Sem espaço: já está na segunda linha
      }
      line = 1;
      col = 0;
      lcdWriteByte(0x80 | 0x40, 0);         // Cursor para o início da linha 2 (DDRAM 0x40)
      if (*str == '\n') {
        str++;
        continue;
      }
    }
    lcdWriteByte(*str, 1);
    str++;
    col++;
  }
}