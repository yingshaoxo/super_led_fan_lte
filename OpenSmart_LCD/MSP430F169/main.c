#include <msp430f169.h> 

int main(void)
{
    volatile unsigned int i;
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    P3SEL |= 0xC0;                            // P3.6 and P3.7 = USART1 option select; 3.6 TX, 3.7 RX

    BCSCTL1 &= ~XT2OFF;                       // XT2on

    do
    {
        IFG1 &= ~OFIFG;                           // Clear OSCFault flag
        for (i = 0xFF; i > 0; i--);               // Time for flag to set
    }
    while ((IFG1 & OFIFG));                   // OSCFault flag still set?

    BCSCTL2 |= SELM_2 + SELS;                 // MCLK= SMCLK= XT2 (safe)
    ME2 |= UTXE1 + URXE1;                     // Enable USART1 TXD/RXD
    UCTL1 |= CHAR;                            // 8-bit character
    UTCTL1 |= SSEL1;                          // UCLK = SMCLK
    UBR01 = 0x45;                             // 8Mhz/115200 - 69.44
    UBR11 = 0x00;                             //
    UMCTL1 = 0x2C;                            // modulation
    UCTL1 &= ~SWRST;                          // Initialize USART state machine

    IE2 |= URXIE1;                            // Enable USART1 RX interrupt

    _BIS_SR(GIE); // just enable general interrupt
}

unsigned char BYTE_YOU_WANT_TO_SEND;
unsigned char BYTE_FROM_SERIAL;
unsigned int a_byte_has_sent = 0;
#pragma vector=USART1RX_VECTOR
__interrupt void usart1_rx (void)
{
    while (!(IFG2 & UTXIFG1));                // USART1 TX buffer ready?

    //TXBUF1 = RXBUF1;                          // RXBUF1 to TXBUF1
    BYTE_FROM_SERIAL = RXBUF1;
    TXBUF1 = BYTE_YOU_WANT_TO_SEND;
    a_byte_has_sent = 1;
}

/*
unsigned char* hex_string_to_char(const char* hexstr)
{
    size_t len = strlen(hexstr);
    IF_ASSERT(len % 2 != 0)
        return NULL;
    size_t final_len = len / 2;
    unsigned char* chrs = (unsigned char*)malloc((final_len+1) * sizeof(*chrs));
    for (size_t i=0, j=0; j<final_len; i+=2, j++)
        chrs[j] = (hexstr[i] % 32 + 9) % 25 * 16 + (hexstr[i+1] % 32 + 9) % 25;
    chrs[final_len] = '\0';
    return chrs;
}
*/

//unsigned char a_command_array = {0x7E, 0x04, 0x20, 0xFF, 0xFF, 0xEF};
void OpenSmart_write_command(unsigned char array[], unsigned int length) {
    int i;
    for (i=0; i< length; i++) {
        a_byte_has_sent = 0;
        BYTE_YOU_WANT_TO_SEND = array[i];
        while (a_byte_has_sent == 0);
    }
    BYTE_YOU_WANT_TO_SEND = 0xEF;
}
