#include <msp430f169.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> // CHAR_BIT, UCHAR_MAX

// ***************
// ****************
// SET OpenSmart LCD!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// P3.6: TX
// P3.7: RX
// ***************
// ****************

void millisecond_of_delay(unsigned long int t) {
    t = t * 8;
    while (t--) {
        // delay for 1ms
        __delay_cycles(1000);
    }
}

void OpenSmart_write_command(unsigned char array[], unsigned int length) {
    // this function may send 0x00 on error
    int i;
    for (i=0; i< length; i++) {
        while (!(IFG2 & UTXIFG1)) {
        };                // USART1 TX buffer ready?
        TXBUF1 = array[i];
    }
}

void test_OpenSmart_write_command(unsigned char a_byte) {
    unsigned char list[1];
    list[0] = a_byte;
    OpenSmart_write_command(list, 1);
}

unsigned int reading_finished = 1;
unsigned char bytes_we_got[30];
unsigned int bytes_length = 0;
void OpenSmart_read_command() {
    unsigned int bytes_index = 0;
    long int max_attempts = 100000; // you may want to add more zero here

    while (1) {
        while (!(IFG2 & URXIFG1)) {
            // USART1 RX buffer is not ready? if so we stuck here
            max_attempts--;
            if (max_attempts < 0) {
                return;
            }
        }

        if (reading_finished == 1) {
            if (RXBUF1 == 0x7e) {
                reading_finished = 0;
            }
        }
        if (reading_finished == 0) {
            bytes_we_got[bytes_index] = RXBUF1;
            bytes_index = bytes_index + 1;
        }
        if (RXBUF1 == 0xef) {
            bytes_length = bytes_index;
            reading_finished = 1;
            break;
        }
    }
}

void OpenSmart_wait_for_command_to_be_executed() {
    OpenSmart_read_command();

    // we do nothing here since we don't care the result;
    if (reading_finished == 1) {
        //print(bytes_we_got);
        //print(length_of_bytes_we_got);

        //OpenSmart_write_command(bytes_we_got, bytes_length);
    }
}

void OpenSmart_write_command_safely(unsigned char array[], unsigned int length) {
    OpenSmart_write_command(array, length);
    OpenSmart_wait_for_command_to_be_executed();
}

void OpenSmart_fill_LCD_with_black_color() {
    unsigned char fill_screen_with_black[] = {0x7E, 0x04, 0x20, 0x00, 0x00, 0xEF};
    OpenSmart_write_command_safely(fill_screen_with_black, 6);
}

void OpenSmart_fill_LCD_with_white_color() {
    unsigned char fill_screen_with_white[] = {0x7E, 0x04, 0x20, 0xff, 0xff, 0xEF};
    OpenSmart_write_command_safely(fill_screen_with_white, 6);
}

void OpenSmart_print_string(unsigned int x, unsigned int y, unsigned char *string) {
    // set cursor
    unsigned char set_cursor[] = {0x7e, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0xef};
    OpenSmart_write_command_safely(set_cursor, 8);

    // set Text color
    unsigned char set_text_color[] = {0x7e, 0x04, 0x02, 0xf8, 0x00, 0xef};
    OpenSmart_write_command_safely(set_text_color, 6);

    // set text size
    unsigned char set_text_size[] = {0x7e, 0x03, 0x03, 0x03, 0xef};
    OpenSmart_write_command_safely(set_text_size, 5);

    // to new line
    unsigned char new_line[] = {0x7e, 0x02, 0x10, 0xef};
    int i;
    for (i=0; i < y; i++) {
        OpenSmart_write_command_safely(new_line, 4);
        OpenSmart_write_command_safely(new_line, 4);
        OpenSmart_write_command_safely(new_line, 4);
    }

    // split string to chunks
    int string_length = strlen(string);
    int number_of_chunks = (string_length/5) + 1;
    unsigned char chunks[10][5];
    int chunk_index;
    for (chunk_index=0; chunk_index < number_of_chunks; chunk_index++) {
        for (i=0; i < 5; i++) {
            if (*string > 0) {
                chunks[chunk_index][i] = *string;
                string++;
            } else {
                chunks[chunk_index][i] = 0x20;
            }
        }
    }

    // send text chunks to LCD
    unsigned char text[9];
    for (chunk_index=0; chunk_index < number_of_chunks; chunk_index++) {
        text[0] = 0x7e;
        text[1] = 0x07;
        text[2] = 0x11;
        for (i=0; i < 5; i++) {
            text[i+3] = chunks[chunk_index][i];
        }
        text[8] = 0xef;
        OpenSmart_write_command_safely(text, 9);
    }
}

void OpenSmart_print_number(int x, int y, long int number) {
    char text[20];
    sprintf(text, "%d", number);
    print_string(x, y, text);
}

void OpenSmart_print_float(int x, int y, float number) {
    char text[20];
    sprintf(text, "%f", number);
    print_string(x, y, text);
}

unsigned char * int_to_bytes_array(short int num) {
    unsigned char bytes[sizeof(int)];
    int i;
    for (i=0; i<sizeof(int); i++)
    {
        bytes[i] = num & UCHAR_MAX;
        num >>= CHAR_BIT;
    }
    return bytes;
}

void OpenSmart_draw_rectangle(short int x, short int y, short int width, short int height) {
    unsigned char text[14];
    
    // start
    text[0] = 0x7e;
    text[1] = 0x0c;
    text[2] = 0x26;

    // x
    text[3] = int_to_bytes_array(x)[1];
    text[4] = int_to_bytes_array(x)[0];

    // y
    text[5] = int_to_bytes_array(y)[1];
    text[6] = int_to_bytes_array(y)[0];

    // width
    text[7] = int_to_bytes_array(width)[1];
    text[8] = int_to_bytes_array(width)[0];

    // height
    text[9] = int_to_bytes_array(height)[1];
    text[10] = int_to_bytes_array(height)[0];

    // color
    text[11] = 0x00;
    text[12] = 0x00;

    // end
    text[13] = 0xef;

    OpenSmart_write_command_safely(text, 14);
}

void initialize_OpenSmart_LCD() {
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

    //IE2 |= URXIE1;                            // Enable USART1 RX interrupt
    //_BIS_SR(GIE); // just enable general interrupt

    millisecond_of_delay(1000*5); // wait for LCD to wake up

    // you must add [] if you want to define an array in C
    unsigned char set_baud[] = {0x7E, 0x03, 0x40, 0x04, 0xEF};
    unsigned char set_blacklight[] = {0x7E, 0x03, 0x06, 0x96, 0xEF};
    OpenSmart_write_command_safely(set_baud, 5);
    OpenSmart_write_command_safely(set_blacklight, 5);
    OpenSmart_fill_LCD_with_white_color();
}

void screen_clean() {
    OpenSmart_fill_LCD_with_white_color();
}

void print_string(unsigned int x, unsigned int y, unsigned char *string) {
    OpenSmart_print_string(x, y-1, string);
}

void print_number(int x, int y, long int number) {
    OpenSmart_print_number(x, y, number);
}

void print_float(int x, int y, float number) {
    OpenSmart_print_float(x, y, number);
}

void test_print_string() {
    OpenSmart_fill_LCD_with_black_color();

    print_string(0, 1, "hi, yingshaoxo");
    print_string(0, 2, "hi, yingshaoxo");
    print_float(0, 3, -2.552255);
    print_number(0, 4, 666);
}

// ***************
// ****************
// SET Serial Communication!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// TX(transmit): P3.4
// RX(receive): P3.5
//
// One thing you have to know: Tx connect to Rx, Rx connect to Tx  !!!!!!!!!
//
// VCC: 3.3V
//
// ***************
// ****************

void initialize_serial_communication() {
    /* for 115200 baud */
    volatile unsigned int i;
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    P3SEL |= 0x30;            // P3.4 and P3.5 = USART0 TXD/RXD

    BCSCTL1 &= ~XT2OFF; // XT2on

    do {
        IFG1 &= ~OFIFG; // Clear OSCFault flag
        for (i = 0xFF; i > 0; i--)
            ;                 // Time for flag to set
    } while ((IFG1 & OFIFG)); // OSCFault flag still set?

    BCSCTL2 |= SELM_2 + SELS; // MCLK = SMCLK = XT2 (safe)
    ME1 |= UTXE0 + URXE0;     // Enable USART0 TXD/RXD
    UCTL0 |= CHAR;            // 8-bit character
    UTCTL0 |= SSEL1;          // UCLK = SMCLK
    UBR00 = 0x45;             // 8MHz 115200
    UBR10 = 0x00;             // 8MHz 115200
    UMCTL0 = 0x00;            // 8MHz 115200 modulation
    UCTL0 &= ~SWRST;          // Initialize USART state machine

    IE1 |= URXIE0; // Enable USART0 RX interrupt

    _BIS_SR(GIE); // just enable general interrupt
}

unsigned int STATE_YOU_WANT_TO_SEND = 0;
void send_state_to_serial(int state) {
    STATE_YOU_WANT_TO_SEND = state;
}

unsigned int STATE_FROM_SERIAL = 0;
unsigned int TASK_NUMBER = 0;

unsigned int data_arraving = 0;
unsigned int data_index = 0;

unsigned char an_image[257];
unsigned int image_index = 0;
unsigned int an_image_received = 0;

unsigned int KeyPad_value = 15; // 15 was not exist
unsigned int old_KeyPad_state = 0;
unsigned int new_KeyPad_state = 0;

// ***************
// ****************
//
// Display imgage at LCD!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// ***************
// ****************

void draw_picture_from_serial() {
    screen_clean();

    int box_length = 15;
    unsigned char y, x;
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            if (an_image[(y*16)+x] == 1) {
                OpenSmart_draw_rectangle(x*box_length, y*box_length, box_length, box_length);
            }
        }
    }
}

// ***************
// ****************
// SET Pin interrupt for getting base time T !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// infrared sensor pin: P1.3
//
// ***************
// ****************

#define echo_pin BIT3

void initialize_infrared_sensor() {
    P1DIR &= ~echo_pin; // set P1.3 as input

    TB0CCTL0 |= CCIE;         // Timer B Capture/Compare Control 0; CCR0 interrupt enabled
    TB0CCR0 = 50000;          // start to increase TBR microseconds
    TB0CTL = TBSSEL_2 + MC_1; // Timer B Control; SMCLK, UP mode

    P1IFG &= ~echo_pin; // interrupt flag: set to 0 to indicate No interrupt is pending
    P1IE |= echo_pin;   // interrupt enable: enable interrupt on pin1.3
    P1IES |= echo_pin;  // set P1 echo_pin to falling edge interrupt: P1IES = 1

    //_BIS_SR(GIE); // global interrupt enable
    __enable_interrupt(); // you must enable all interrupt for thie code to work
}

unsigned long int temporary_accumulated_T;
unsigned long int infrared_detection_counting = 0;
unsigned long int infrared_detection_counting2 = 0;
unsigned long int infrared_detection_counting3 = 0;
unsigned long int the_average_T = 0;                         //T
unsigned long int the_time_during_one_part_of_240_parts = 0; //T1
unsigned int enter_pin_interrupt = 0;

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {
    if (P1IFG & echo_pin) // is that interrupt request come from echo_pin? is there an rising or falling edge has been detected? Each PxIFGx bit is the interrupt flag for its corresponding I/O pin and is set when the selected input signal edge occurs at the pin.
    {
        if (P1IES & echo_pin) // is this the falling edge? (P1IES & echo_pin) == 1
        {
            enter_pin_interrupt = 1;
            if (infrared_detection_counting > 9) {
                temporary_accumulated_T += TB0R; // TBR is a us time unit at this case; may have error if change happens too fast; use array is a sulution
            }
            infrared_detection_counting++;
            infrared_detection_counting2++;
            infrared_detection_counting3++;
            if (infrared_detection_counting > 19) {
                the_average_T = temporary_accumulated_T / 10;
                the_time_during_one_part_of_240_parts = the_average_T / 240;
                infrared_detection_counting = 10;
            }
            if (infrared_detection_counting2 > 30) {
                infrared_detection_counting2 = 0;
            }
            if (infrared_detection_counting3 > 1050) {
                infrared_detection_counting3 = 0;
            }
            TB0CCR0 = 50000; // start to increase TBR microseconds
        }
        P1IFG &= ~echo_pin; // clear flag, so it can start to detect new rising or falling edge, then a new call to this interrupt function will be allowed.
    }
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B(void) {
    // don't know why this have to be exist, but without it, TBR won't work
}

// ***************
// ****************
// Control the LEDs !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// data: P5.0-P5.7
// enable: P6.0-P6.3
//
// ***************
// ****************

#define enable_upper_red_chip P6OUT |= BIT3
#define disable_upper_red_chip P6OUT &= ~BIT3

#define enable_upper_green_chip P6OUT |= BIT1
#define disable_upper_green_chip P6OUT &= ~BIT1

#define enable_lower_red_chip P6OUT |= BIT2
#define disable_lower_red_chip P6OUT &= ~BIT2

#define enable_lower_green_chip P6OUT |= BIT0
#define disable_lower_green_chip P6OUT &= ~BIT0

unsigned char int_to_led_hex(int number) {
    if (number > 8) {
        number = number - 8;
    }
    switch (number) {
    case 1:
        return 0x01;
    case 2:
        return 0x02;
    case 3:
        return 0x04;
    case 4:
        return 0x08;
    case 5:
        return 0x10;
    case 6:
        return 0x20;
    case 7:
        return 0x40;
    case 8:
        return 0x80;
    default:
        return 0x00;
    }
}

void set_first_8_red_leds(unsigned char byte_data) {
    enable_upper_red_chip;
    P5OUT = byte_data;
    disable_upper_red_chip;
}

void set_first_8_green_leds(unsigned char byte_data) {
    enable_upper_green_chip;
    P5OUT = byte_data;
    disable_upper_green_chip;
}

void set_second_8_red_leds(unsigned char byte_data) {
    enable_lower_red_chip;
    P5OUT = byte_data;
    disable_lower_red_chip;
}

void set_second_8_green_leds(unsigned char byte_data) {
    enable_lower_green_chip;
    P5OUT = byte_data;
    disable_lower_green_chip;
}

void turn_off_all_leds() {
    set_first_8_green_leds(0x00);
    set_first_8_red_leds(0x00);
    set_second_8_green_leds(0x00);
    set_second_8_red_leds(0x00);
}

void delay_for_leds(unsigned char length) {
    while (length--) {
        __delay_cycles(200);
    }
}

void initialize_16_rows_LED() {
    P5DIR |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7); // set Port 5 as output
    P6DIR |= (BIT0 | BIT1 | BIT2 | BIT3);                             // set P6.0-P6.3 as output

    P5OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7); // set Port 5 to low
    P6OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);                             // set P6.0-P6.3 to low

    set_first_8_green_leds(0xff);
    set_second_8_red_leds(0xff);
    millisecond_of_delay(625);
    turn_off_all_leds();
}

// ****************

// Task 1

// ****************

void task1(row_1, row_2) {
    // the number is between 1 and 16
    unsigned char row_num1 = 0;
    unsigned char row_num2 = 0;

    switch (row_1) {
    case 1:
        row_num1 |= 0x80;
        break;
    case 2:
        row_num1 |= 0x40;
        break;
    case 3:
        row_num1 |= 0x20;
        break;
    case 4:
        row_num1 |= 0x10;
        break;
    case 5:
        row_num1 |= 0x08;
        break;
    case 6:
        row_num1 |= 0x04;
        break;
    case 7:
        row_num1 |= 0x02;
        break;
    case 8:
        row_num1 |= 0x01;
        break;
    case 9:
        row_num2 |= 0x80;
        break;
    case 10:
        row_num2 |= 0x40;
        break;
    case 11:
        row_num2 |= 0x20;
        break;
    case 12:
        row_num2 |= 0x10;
        break;
    case 13:
        row_num2 |= 0x08;
        break;
    case 14:
        row_num2 |= 0x04;
        break;
    case 15:
        row_num2 |= 0x02;
        break;
    case 16:
        row_num2 |= 0x01;
        break;
    default:
    }

    switch (row_2) {
    case 1:
        row_num1 |= 0x80;
        break;
    case 2:
        row_num1 |= 0x40;
        break;
    case 3:
        row_num1 |= 0x20;
        break;
    case 4:
        row_num1 |= 0x10;
        break;
    case 5:
        row_num1 |= 0x08;
        break;
    case 6:
        row_num1 |= 0x04;
        break;
    case 7:
        row_num1 |= 0x02;
        break;
    case 8:
        row_num1 |= 0x01;
        break;
    case 9:
        row_num2 |= 0x80;
        break;
    case 10:
        row_num2 |= 0x40;
        break;
    case 11:
        row_num2 |= 0x20;
        break;
    case 12:
        row_num2 |= 0x10;
        break;
    case 13:
        row_num2 |= 0x08;
        break;
    case 14:
        row_num2 |= 0x04;
        break;
    case 15:
        row_num2 |= 0x02;
        break;
    case 16:
        row_num2 |= 0x01;
        break;
    default:
    }

    set_first_8_red_leds(row_num1);
    set_second_8_red_leds(row_num2);
}

// ****************

// Task 2

// ****************

void task2() {
    int i;
    for (i = 1; i < 17; i++) {
        task1(i, 17 - i);
        millisecond_of_delay(125);
    }
}

// ****************

// Task 3

// ****************

void task3() {
    unsigned char i = 0;
    for (i = 0; i < 16; i++) {
        set_first_8_red_leds(0xff);
        set_second_8_red_leds(0xff);
        delay_for_leds(3);
        set_first_8_green_leds(0x00);
        set_second_8_green_leds(0x00);
        delay_for_leds(2);

        delay_for_leds(3);
        turn_off_all_leds();
        delay_for_leds(3);
    }

    delay_for_leds(44);

    for (i = 0; i < 16; i++) {
        set_first_8_red_leds(0xff);
        set_second_8_red_leds(0xff);
        delay_for_leds(3);
        set_first_8_green_leds(0x00);
        set_second_8_green_leds(0x00);
        delay_for_leds(2);

        delay_for_leds(3);
        turn_off_all_leds();
        delay_for_leds(3);
    }
}

// ****************

// Task 4

// ****************

unsigned char inverse_task4 = 0;
void task4() {
    unsigned char i = 0;
    unsigned char value = 5;

    if (inverse_task4) {
        value = (30 - infrared_detection_counting2) / 5;
    } else {
        value = infrared_detection_counting2 / 5;
    }
    value = infrared_detection_counting2 / 5;
    if (value == 0) {
        if (inverse_task4) {
            inverse_task4 = 0;
        } else {
            inverse_task4 = 1;
        }
    }

    for (i = 0; i < 16; i++) {
        set_first_8_red_leds(0xff);
        set_second_8_red_leds(0xff);
        delay_for_leds(value);
        set_first_8_green_leds(0x00);
        set_second_8_green_leds(0x00);
        delay_for_leds(value);

        delay_for_leds(3);
        turn_off_all_leds();
        delay_for_leds(3);
    }

    delay_for_leds(44);

    for (i = 0; i < 16; i++) {
        set_first_8_red_leds(0xff);
        set_second_8_red_leds(0xff);
        delay_for_leds(value);
        set_first_8_green_leds(0x00);
        set_second_8_green_leds(0x00);
        delay_for_leds(value);

        delay_for_leds(3);
        turn_off_all_leds();
        delay_for_leds(3);
    }
}

// ****************

// Task 5

// ****************

void task5() {
}

// ****************

// Task 6

// ****************

unsigned char point_square[96];
unsigned char Idex = 0;
unsigned int dnum;
int show_task6 = 0;
void task6() {
    if (show_task6 == 1) {
        unsigned char i, index = 0;

        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(0x00);
            set_second_8_green_leds(0x00);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 32;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(0x00);
            set_second_8_green_leds(0x00);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 64;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(0x00);
            set_second_8_green_leds(0x00);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
    }
}

// ****************

// Task 7

// ****************

void task7() {
    unsigned char i, index = 0;

    if (infrared_detection_counting3 < 150) {
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(0x00);
            delay_for_leds(3);
            set_first_8_green_leds(0x00);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 32;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(0x00);
            delay_for_leds(3);
            set_first_8_green_leds(0x00);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 64;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(0x00);
            delay_for_leds(3);
            set_first_8_green_leds(0x00);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
    } else if ((infrared_detection_counting3 > 149) && (infrared_detection_counting3 < 300)) {
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(0x00);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 32;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(0x00);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 64;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(0x00);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
    } else if ((infrared_detection_counting3 > 299) && (infrared_detection_counting3 < 450)) {
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(0x00);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 32;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(0x00);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 64;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(0x00);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
    } else if ((infrared_detection_counting3 > 449) && (infrared_detection_counting3 < 600)) {
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(0x00);
            set_second_8_green_leds(0x00);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 32;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(0x00);
            set_second_8_red_leds(0x00);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 64;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
    } else if ((infrared_detection_counting3 > 599) && (infrared_detection_counting3 < 750)) {
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(0x00);
            set_second_8_red_leds(0x00);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 32;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(0x00);
            set_second_8_red_leds(0x00);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 64;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(0x00);
            set_second_8_red_leds(0x00);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
    } else if ((infrared_detection_counting3 > 749) && (infrared_detection_counting3 < 900)) {
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(0x00);
            set_second_8_green_leds(0x00);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 32;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(0x00);
            set_second_8_green_leds(0x00);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 64;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(0x00);
            set_second_8_green_leds(0x00);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
    } else if ((infrared_detection_counting3 > 899) && (infrared_detection_counting3 < 1050)) {
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 32;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
        delay_for_leds(44);

        index = 64;
        for (i = 0; i < 16; i++) {
            set_first_8_red_leds(point_square[index]);
            set_second_8_red_leds(point_square[index + 16]);
            delay_for_leds(3);
            set_first_8_green_leds(point_square[index]);
            set_second_8_green_leds(point_square[index + 16]);
            delay_for_leds(2);

            delay_for_leds(3);
            turn_off_all_leds();
            delay_for_leds(3);

            index++;
        }
    }
}

// ****************

// Other Task

// ****************

void show_camera() {
}

// ***************
// ****************
//
// Handle keypad value !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// ***************
// ****************

/* 
1	2	3	Return(or back)
4	5	6	Menu
7	8	9	Cancel
    0	.	Enter

.: 10
Return: 11
Menu: 12
Cancel: 13
Enter: 14
*/

void clean_LCD_menu() {
    screen_clean();
    millisecond_of_delay(60);
}

char input_string[50];

int task_number_from_keypad = 0;

int state = -1;
int parameter1 = -1;
int parameter2 = -1;
int parameter3 = -1;
void handle_keypad_key(int number) {
    if (number != 10) {
        if (number < 10) {
            clean_LCD_menu();

            char text[1];
            sprintf(text, "%d", number);
            strcat(input_string, text);
            print_string(0, 1, input_string);

            //return; // I don't know if I should add this; problem 1
        } else if (number == 14 || number == 15) {
            if (strcmp(input_string, "") == 0) {
                state = -1;
                strcpy(input_string, "");
                parameter1 = -1;
                parameter2 = -1;
                parameter3 = -1;

                clean_LCD_menu();
                print_string(0, 1, "Which Task?");
                return;
            }
            state += 1;
            if (state == 0) {
                int target_number = atoi(input_string);
                task_number_from_keypad = target_number;
                strcpy(input_string, "");

                clean_LCD_menu();
                print_string(0, 1, "Parameter1?");
            } else if (state == 1) {
                int target_number = atoi(input_string);
                parameter1 = target_number;
                strcpy(input_string, "");

                clean_LCD_menu();
                print_string(0, 1, "Parameter2?");
            } else if (state == 2) {
                int target_number = atoi(input_string);
                parameter2 = target_number;
                strcpy(input_string, "");

                clean_LCD_menu();
                print_string(0, 1, "Parameter3?");
            } else if (state == 3) {
                int target_number = atoi(input_string);
                parameter3 = target_number;
                strcpy(input_string, "");

                clean_LCD_menu();
                print_string(0, 1, "Back to menu?");
            } else {
                state = -1;
                strcpy(input_string, "");
                parameter1 = -1;
                parameter2 = -1;
                parameter3 = -1;

                clean_LCD_menu();
                print_string(0, 1, "Which Task?");
                return;
            }
        } else if (number == 11 || number == 12 || number == 13) {
            state = -1;
            strcpy(input_string, "");
            parameter1 = -1;
            parameter2 = -1;
            parameter3 = -1;

            clean_LCD_menu();
            print_string(0, 1, "Which Task?");
            return;
        }
    }

    // handle all tasks here
    print_number(0, 2, task_number_from_keypad);
    if (parameter1 != -1) {
        print_number(0, 3, parameter1);
    }
    if (parameter2 != -1) {
        print_number(0, 4, parameter2);

        if (parameter3 != -1) {
            char text[20];
            sprintf(text, "%d       %d", parameter2, parameter3);
            print_string(0, 4, text);
        }
    }

    if (task_number_from_keypad == 5) {
        if (parameter3 != -1) {
            if (an_image_received == 1) {
                clean_LCD_menu();
                draw_picture_from_serial();
                an_image_received = 0;
            }
        } else if (parameter2 != -1) {
            if (an_image_received == 1) {
                clean_LCD_menu();
                draw_picture_from_serial();
                an_image_received = 0;
            }
        } else if (parameter1 != -1) {
            if (an_image_received == 1) {
                clean_LCD_menu();
                draw_picture_from_serial();
                an_image_received = 0;
            }
        }
    }

    if (task_number_from_keypad == 6) {
        if (parameter3 != -1) {
            if (an_image_received == 1) {
                for (dnum = 0; dnum < 256; dnum++) {
                    if (dnum < 128) {
                        Idex = 0 + 32 * 2;
                    } else {
                        Idex = 16 + 32 * 2;
                    }
                    if (an_image[dnum] == 1) {
                        point_square[Idex + dnum % 16] |= (0x80 >> ((dnum / 16) % 8));
                    } else {
                        point_square[Idex + dnum % 16] &= ~(0x80 >> ((dnum / 16) % 8));
                    }
                }
                show_task6 = 1;
                an_image_received = 0;
            }
        } else if (parameter2 != -1) {
            if (an_image_received == 1) {
                for (dnum = 0; dnum < 256; dnum++) {
                    if (dnum < 128) {
                        Idex = 0 + 32 * 1;
                    } else {
                        Idex = 16 + 32 * 1;
                    }
                    if (an_image[dnum] == 1) {
                        point_square[Idex + dnum % 16] |= (0x80 >> ((dnum / 16) % 8));
                    } else {
                        point_square[Idex + dnum % 16] &= ~(0x80 >> ((dnum / 16) % 8));
                    }
                }
                an_image_received = 0;
            }
        } else if (parameter1 != -1) {
            if (an_image_received == 1) {
                for (dnum = 0; dnum < 256; dnum++) {
                    if (dnum < 128) {
                        Idex = 0 + 32 * 0;
                    } else {
                        Idex = 16 + 32 * 0;
                    }
                    if (an_image[dnum] == 1) {
                        point_square[Idex + dnum % 16] |= (0x80 >> ((dnum / 16) % 8));
                    } else {
                        point_square[Idex + dnum % 16] &= ~(0x80 >> ((dnum / 16) % 8));
                    }
                }
                an_image_received = 0;
            }
        }
    }

    if (task_number_from_keypad == 8) {
        if (an_image_received == 1) {
            clean_LCD_menu();
            draw_picture_from_serial();
            an_image_received = 0;
        }
    }
}

#pragma vector = USART0RX_VECTOR
__interrupt void usart0_rx(void) {
    while (!(IFG1 & UTXIFG0)) {
        // USART0 TX buffer ready?
    }
    STATE_FROM_SERIAL = (int)RXBUF0; // receive state
    //TXBUF0 = (unsigned char)STATE_YOU_WANT_TO_SEND; // send state

    if (STATE_FROM_SERIAL != 0 || data_arraving == 1) {
        if (data_arraving == 0) {
            TASK_NUMBER = STATE_FROM_SERIAL;
            if (TASK_NUMBER == 8) {
                an_image_received = 0;
            }
            data_arraving = 1;

        } else {
            if (TASK_NUMBER == 8) {     // handle image data
                if (data_index < 256) { // data_index = 255, actually the 256 element
                    if (STATE_FROM_SERIAL == 9) {
                        an_image[data_index] = 1;
                    } else {
                        an_image[data_index] = 0;
                    }
                } else { // data_index == 256, actually the 257 element
                    image_index = STATE_FROM_SERIAL;
                }
            }
            if (TASK_NUMBER == 11) { // handle keypad data
                if (data_index == 0) {
                    KeyPad_value = STATE_FROM_SERIAL;
                }
                if (data_index == 1) {
                    new_KeyPad_state = STATE_FROM_SERIAL;
                }
            }

            data_index += 1;

            if (data_index > 256) {
                data_arraving = 0;
                data_index = 0;
                if (TASK_NUMBER == 8) { // handle image data after data transmission finished
                    an_image_received = 1;
                }
                if (TASK_NUMBER == 11) { // handle keypad data after data transmission finished
                    if (old_KeyPad_state != new_KeyPad_state) {
                        // do something
                        handle_keypad_key(KeyPad_value);
                    }
                    old_KeyPad_state = new_KeyPad_state;
                }
            }
        }
    } else {
        TASK_NUMBER = 0;
    }
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    initialize_OpenSmart_LCD();
    initialize_serial_communication();
    initialize_infrared_sensor();
    initialize_16_rows_LED();

    print_string(0, 1, "Which Task?");

    while (1) {
        if (enter_pin_interrupt == 1) {
            switch (task_number_from_keypad) {
            case 0:
                break;
            case 1:
                if (parameter1 != -1 && parameter2 != -1) {
                    task1(parameter1, parameter2);
                }
                break;
            case 2:
                task2();
                break;
            case 3:
                task3();
                break;
            case 4:
                task4();
                break;
            case 5:
                task5();
                break;
            case 6:
                task6();
                break;
            case 7:
                task7();
                break;
            case 8:
                show_camera();
                break;
            default:
                break;
            }
            enter_pin_interrupt = 0;
        }
    }
}
