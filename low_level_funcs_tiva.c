/**
 * $File: low_level_funcs_tiva.c
 *
 *  *******************************************************************************************
 *
 *  @file      low_level_funcs_tiva.c
 *
 *  @brief      Set of functions at the bottom level for the 3662 calculator mini-project.
 * 				These are the hardware drivers.
 * 				For documentation, see the documentation in the corresponding .h file.
 *  *******************************************************************************************
 *
 *  $NoKeywords
 **/

/**********************************************************************************************
 * Module includes
 **********************************************************************************************/
#include "TExaS.h"
#include "low_level_funcs_tiva.h"
#include "_tivaware/driverlib/flash.h"
/**********************************************************************************************
 * Referenced external functions
 **********************************************************************************************/

/**********************************************************************************************
 * Referenced external variables
 **********************************************************************************************/

/**********************************************************************************************
 * Global variable definitions
 **********************************************************************************************/

/**********************************************************************************************
 * Private constant definitions
 **********************************************************************************************/
#define ANSWER_FLASH_ADDRESS 0x0003F800 /*Address in flash where the previous answer is stored.*/

/*Ports*/
// Port A (bit 2 is EN, bit 3 is RS):
#define GPIO_PORTA_DATA_R  (*((volatile unsigned long *)0x400043FC))
#define GPIO_PORTA_DIR_R   (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_PUR_R   (*((volatile unsigned long *)0x0))
#define GPIO_PORTA_PDR_R   (*((volatile unsigned long *)0x0))
#define GPIO_PORTA_DEN_R   (*((volatile unsigned long *)0x4000451C))
#define GPIO_PORTA_LOCK_R  (*((volatile unsigned long *)0x40004520))
#define GPIO_PORTA_CR_R    (*((volatile unsigned long *)0x40004524))
#define GPIO_PORTA_AMSEL_R (*((volatile unsigned long *)0x40004528))
#define GPIO_PORTA_PCTL_R  (*((volatile unsigned long *)0x4000452C))

// Port B (PORTB[2:5] are LCD DB4 to DB7):
#define GPIO_PORTB_DATA_R  (*((volatile unsigned long *)0x400053FC))
#define GPIO_PORTB_DIR_R   (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_PUR_R   (*((volatile unsigned long *)0x0))
#define GPIO_PORTB_PDR_R   (*((volatile unsigned long *)0x0))
#define GPIO_PORTB_DEN_R   (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_LOCK_R  (*((volatile unsigned long *)0x40005520))
#define GPIO_PORTB_CR_R    (*((volatile unsigned long *)0x40005524))
#define GPIO_PORTB_AMSEL_R (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R  (*((volatile unsigned long *)0x4000552C))

// Port D (PORTD[0:3] are the outputs to the columns):
#define GPIO_PORTD_DATA_R  (*((volatile unsigned long *)0x400073FC))
#define GPIO_PORTD_DIR_R   (*((volatile unsigned long *)0x40007400))
#define GPIO_PORTD_AFSEL_R (*((volatile unsigned long *)0x40007420))
#define GPIO_PORTD_PUR_R   (*((volatile unsigned long *)0x0))
#define GPIO_PORTD_PDR_R   (*((volatile unsigned long *)0x0))
#define GPIO_PORTD_DEN_R   (*((volatile unsigned long *)0x4000751C))
#define GPIO_PORTD_LOCK_R  (*((volatile unsigned long *)0x40007520))
#define GPIO_PORTD_CR_R    (*((volatile unsigned long *)0x40007524))
#define GPIO_PORTD_AMSEL_R (*((volatile unsigned long *)0x40007528))
#define GPIO_PORTD_PCTL_R  (*((volatile unsigned long *)0x4000752C))

// Port E (PORTE[0:3] are the inputs from the rows):
#define GPIO_PORTE_DATA_R  (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R   (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_PUR_R   (*((volatile unsigned long *)0x0))
#define GPIO_PORTE_PDR_R   (*((volatile unsigned long *)0x40024514))
#define GPIO_PORTE_DEN_R   (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_LOCK_R  (*((volatile unsigned long *)0x40024520))
#define GPIO_PORTE_CR_R    (*((volatile unsigned long *)0x40024524))
#define GPIO_PORTE_AMSEL_R (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R  (*((volatile unsigned long *)0x4002452C))

/*Clocks*/
// PLL related Defines
#define SYSCTL_RIS_R       (*((volatile unsigned long *)0x400FE050))
#define SYSCTL_RCC_R       (*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RCC2_R      (*((volatile unsigned long *)0x400FE070))
#define SYSCTL_RCGC1_R     (*((volatile unsigned long *)0x400FE104))
#define SYSCTL_RCGC2_R     (*((volatile unsigned long *)0x400FE108))

// SysTick related Defines
#define NVIC_ST_CTRL_R     (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R   (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R  (*((volatile unsigned long *)0xE000E018))

/*LCD defines*/
#define LCD_RS                                                                        \
    (*((volatile unsigned long *)0x40004020)) /*                                      \
                                               * The single port bit connected to the \
                                               * RS (Register Select) pin of the LCD. \
                                               */
#define LCD_EN                                                                             \
    (*((volatile unsigned long *)0x40004010)) /*                                           \
                                               * The single port bit connected to the      \
                                               * EN (ENable data transfer) pin of the LCD. \
                                               */
#define LCD_DATA (*((volatile unsigned long *)0x400050F0))
/**********************************************************************************************
 * Private type definitions
 **********************************************************************************************/

/**********************************************************************************************
 * Private function declarations
 **********************************************************************************************/
static void systick_init(void);
static void PLL_init(void);
static void systick_wait(uint32_t delay);
static void init_keyboard_ports(void);
static void send_display_nibble(unsigned char byte, unsigned char instruction_or_data);
static void send_display_byte(unsigned char byte, unsigned char instruction_or_data);
static void init_display_port(void);
static void lcd_pulse(void);
static void init_all_other(void);
/**********************************************************************************************
 * Private variable definitions
 **********************************************************************************************/

/**********************************************************************************************
 * Public function definitions
 **********************************************************************************************/

/**
 * @brief Select which column will be examined when the rows are read by read_keyboard_row().
 *
 * This should write a nibble (4-bit quantity) to the four bits allocated to the
 * keyboard columns.
 * The quantity output should be the least significant four bits of the
 * parameter.
 * The other four bits should be ignored (and are thus harmless if set).
 * The allocation of bits of the nibble to columns is:
 * 	- leftmost (keys 1,4,7,*) is bit 3 (0x8);
 * 	- next (2,5,8,0) is bit 2 (0x4);
 * 	- next (3,6,9.#) is bit 1 (0x2);
 * 	- rightmost (A,B,C,D) is bit 0 (0x1).
 * The use of this function assumes that init_keyboard_ports() has already
 * been called.
 * @param [in] nibble The 4-bit quantity to write. Exactly one bit of this should be set.
 * @return None.
 **/
void
write_keyboard_col(unsigned char nibble)
{
    static bool b_correct_nibble = true;

    switch (nibble)
    {
        case 1:
            GPIO_PORTD_DATA_R = 0x01; // Bit 0 is selected
            break;
        case 2:
            GPIO_PORTD_DATA_R = 0x02; // Bit 1 is selected
            break;
        case 3:
            GPIO_PORTD_DATA_R = 0x04; // Bit 2 is selected
            break;
        case 4:
            GPIO_PORTD_DATA_R = 0x08; // Bit 3 is written
            break;
        default:
            b_correct_nibble = false;
            break;
    }

    if (false == b_correct_nibble)
    {
        clear_display();           // Clear display
        wait_microsec(10000); // wait
        print_char('E');
        print_char('R');
        print_char('R'); // Display an error message
        print_char('O');
        print_char('R');
        wait_microsec(10000); // wait
    }
}

/**
 * @brief Read one row to see which (if any) of its key(s) has/have been pressed.
 * This should read the four bits allocated to the input from the keyboard rows.
 * These four bits should be returned as the least four bits of the result.
 * The other four bits should be set to zero.
 * The allocation of bits of the nibble to rows is:
 *  - top (keys 1,2,3,A) is bit 3 (0x8);
 * 	- next (4,5,6,B) is bit 2 (0x4);
 * 	- next (7,8,9,C) is bit 1 (0x2);
 * 	- bottom (*,0,#,D) is bit 0 (0x1).
 *
 * The use of this function assumes that init_keyboard_ports() and
 * write_keyboard_col() have already been called.
 *
 * @param   None.
 * @return  Data from port E.
 **/
unsigned char
read_keyboard_row(void)
{
    return GPIO_PORTE_DATA_R; // Reads the Data from port E
}

/**
 * @brief Clear the display.
 * @param   None.
 * @return  None.
 **/
void
clear_display(void)
{
    send_display_byte(0x01, 0); // Send an instruction to the LCD to clear the screen
}

/**
 * @brief Turn the cursor on or off.
 * @param   [in] On 0 for off, any non-zero quantity for on.
 * @return  None.
 **/
void
turn_cursor_on_off(bool b_on)
{
    if (b_on)
    {
        send_display_byte(0x0F, 0); // if any non zero value is chosen The cursor turns on and blinks
    }
    else
    {
        send_display_byte(0x0C, 0); // if zero is chosen the cursor and the blinking turn off
    }
}

/**
 * @brief Set the print position for the next character printed.
 * @param   [in] line The line number, 1 for top or 2 for bottom.
 * @param   [in] char_pos The character position, counting from 1 at the
 * 			left to 16 at the right.
 * @return  None.
 **/
void
set_print_position(uint8_t line, uint8_t char_pos)
{
    if (1u == line)
    {

        send_display_byte(0x80 + char_pos, 0); // to select line one and move the cursor to that line
    }
    else if (2u == line)
    {

        send_display_byte(0xC0 + char_pos, 0); // to select line two and move the cursor to that line
    }
    else
    {
        // Do nothing
    }
}

/**
 * @brief Print a character at the current position, then increment that position
 * ready for any following character.
 * @param   [in] cha The character to be displayed.
 * @return  None.
 **/
void
print_char(char ch)
{
    send_display_byte(ch, 1); // Sends a command to the LCD to print the given input

                              //* This autoinrement can leave the next print position beyond the end of the
    //* display, so extra marks will be given for software that checks for valid
    //* position before sending the character to the display.
}

/**
 * @brief Write a double-precision floating point number to flash memory.
 * Store it in the address ANSWER_FLASH_ADDRESS.
 * @param   [in] number The number to store.
 * @return  None.
 **/
void
WriteDoubleToFlash(double number)
{
    /* Cast the address of the double to uint32_t pointer*/
    uint32_t *p_data = (uint32_t *)&number;

    /*Erase the memory address first*/
    FlashErase(ANSWER_FLASH_ADDRESS);

    /*FlashProgram() only writes 32-bit words, so write two 32-bit parts*/
    FlashProgram(p_data, ANSWER_FLASH_ADDRESS, sizeof(double));
}

/**
 * @brief Read a double-precision floating point number from flash memory.
 * Read it from the address ANSWER_FLASH_ADDRESS.
 * @param   None.
 * @return  number_read The number read.
 **/
double
read_from_flash(void)
{
    return *((double *)ANSWER_FLASH_ADDRESS);
}

/**
 * @brief Initialise everything.
 * @param   None.
 * @return  None
 **/
void
init_all_hardware(void)
{
    init_all_other();      // Initialisation of clocks
    init_display_port();   // Initialisation of the LCD
    init_keyboard_ports(); // Initialisation of the Keypad
}

/**
 * @brief Wait a specified number of microseconds.
 * @param   [in] wait_microsecs The time (in microseconds) to delay.
 * @return  None
 **/
void
wait_microsec(uint32_t wait_microsecs)
{
    /* 
     * System clock is 50 MHz.
     * That means each clock cycle is 1 / 50,000,000 = 20 ns.
     * So, 1 microsecond = 1000 ns / 20 ns = 50 cycles.
     * So, 1 µs = 50 cycles at 50 MHz.
    */
    systick_wait(50 * wait_microsecs);
}

/**********************************************************************************************
 * Private function definitions
 **********************************************************************************************/

/**
 * @brief 	Initialise the systick
 * @param   None
 * @return  None
 **/
static void
systick_init(void)
{
    NVIC_ST_CTRL_R = 0;            // disable SysTick during setup
    NVIC_ST_RELOAD_R = 0x00FFFFFF; // maximum reload value
    NVIC_ST_CURRENT_R = 0;         // any write to current clears it
    NVIC_ST_CTRL_R = 0x00000005;   // enable SysTick with core clock
}

/**
 * @brief 	Initialise the PLL
 * @param   None
 * @return  None
 **/
static void
PLL_init(void)
{
    // 0) Use RCC2
    SYSCTL_RCC2_R |= 0x80000000; // USERCC2
    // 1) bypass PLL while initializing
    SYSCTL_RCC2_R |= 0x00000800; // BYPASS2, PLL bypass
    // 2) select the crystal value and oscillator source
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0) // clear XTAL field, bits 10-6
                   + 0x00000540;                // 10101, configure for 16 MHz crystal
    SYSCTL_RCC2_R &= ~0x00000070;               // configure for main oscillator source
    // 3) activate PLL by clearing PWRDN
    SYSCTL_RCC2_R &= ~0x00002000;
    // 4) set the desired system divider
    SYSCTL_RCC2_R |= 0x40000000;                  // use 400 MHz PLL
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) // clear system clock divider
                    + (7 << 22);                  // configure for 50 MHz clock
                                                  //*** YOU CAN CHANGE The 50 MHz Frequency above, by changing the Dividor "7" to
                                                  //*** any desired frequency, i.e (4<<22) makes it 80 MHz etc. ..

    // 5) wait for the PLL to lock by polling PLLLRIS
    while ((SYSCTL_RIS_R & 0x00000040) == 0)
    {
    }; // wait for PLLRIS bit
    // 6) enable use of PLL by clearing BYPASS
    SYSCTL_RCC2_R &= ~0x00000800;
}

/**
 * @brief 	Delay using the systick counter
 * @param   delay delay to wait for
 * @return  None
 **/
static void
systick_wait(uint32_t delay)
{
    NVIC_ST_RELOAD_R = delay - 1; // number of counts to wait
    NVIC_ST_CURRENT_R = 0;        // any value written to CURRENT clears
    while ((NVIC_ST_CTRL_R & 0x00010000) == 0)
    {                             // wait for count flag
    }
}

/**
 * @brief 	Initalised the keyboard ports
 * @param   None
 * @return  None
 **/
static void
init_keyboard_ports(void)
{
    // Port E
    SYSCTL_RCGC2_R |= 0x00000010;    // enables clock for port E
    GPIO_PORTE_LOCK_R = 0x4C4F434B;  // unlocking Port E
    GPIO_PORTE_CR_R = 0x0F;          // allow changes to PE 0 to 3
    GPIO_PORTE_DEN_R |= 0x0F;        // sets Port E pins to digital pins
    GPIO_PORTE_DIR_R |= 0x00;        // sets port E to be input
    GPIO_PORTE_PDR_R |= 0x0F;        // pull down resistors for the input
    GPIO_PORTE_AMSEL_R &= 0x00;      // disable analog function
    GPIO_PORTE_PCTL_R &= 0x00000000; // GPIO clear bit PCTL
    GPIO_PORTE_AFSEL_R = 0x00;       // disables alternate function

    // Port D
    SYSCTL_RCGC2_R |= 0x00000008;    // enables clock for port D
    GPIO_PORTD_LOCK_R = 0x4C4F434B;  // unlocking port D
    GPIO_PORTD_CR_R |= 0x0F;         // allow changes to PD 0 to 3
    GPIO_PORTD_DEN_R |= 0x0F;        // sets Port D pins to digital pins
    GPIO_PORTD_DIR_R |= 0x0F;        // sets port D to be output
    GPIO_PORTD_AMSEL_R &= 0x00;      // disable analog function
    GPIO_PORTD_PCTL_R &= 0x00000000; // GPIO clear bit PCTL
    GPIO_PORTD_AFSEL_R = 0x00;       // disables alternate function
}

/**
 * @brief 	Send one nibble of data or an instruction to the display.
 *  * This must
 * 	1. shift the nibble into the correct four port bits
 * 	2. send the bits
 * 	3. change EN to 1
 * 	4. wait 450 ns
 * 	5. change EN to 0.
 *
 * Note that the other delays (e.g. 37 microseconds for sending instructions
 * or data) are not performed here. They are performed by functions calling
 * this one.
 * @param   delay delay to wait for
 * @param   [in] nibble The nibble to be sent. It must be in the least
 * 		four significant bits of the char.
 * @param   [in] instruction_or_data 0 for instruction,
 * 				   1 for data (i.e. text to display).
 * @return  None
 **/
static void
send_display_nibble(unsigned char byte, unsigned char instruction_or_data)
{
    LCD_RS = instruction_or_data << 3; // sets the value for the LCD_RS and shifts it to the correct bit
    LCD_DATA = byte << 2;              // sets LCD_DATA as the byte sent and shifts it to the left by 2
    lcd_pulse();                       // Pulses the EN
}

/**
 * @brief 	Send one byte of data or an instruction to the display.
 * Send the byte to the port (as two nibbles), waiting the
 * appropriate time after each send.
 *
 * A wait of at least 550 ns is needed between the two nibbles.
 * This is because the second EN pulse must not start until at least
 * 1000 ns after the first one starts.
 *
 * A delay of 37 microseconds is needed after the second nibble. This is
 * to allow the display to process the byte.
 *
 * @param   [in] byte The byte to be sent.
 * @param   [in] instruction_or_data 0 for instruction,
 * 				   1 for data (i.e. text to display).
 * @return  None
 **/
static void
send_display_byte(unsigned char byte, unsigned char instruction_or_data)
{
    send_display_nibble((byte & 0xf0) >> 4, instruction_or_data); // sends the higher nibble and shifts it to the right
    wait_microsec(100);                                           // Delay of 100 microsecond between the 2 nibbles
    send_display_nibble((byte & 0x0f), instruction_or_data);      // sends the lower nibble
    wait_microsec(37000);                                         // Delay of 37 ms
}

/**
 * @brief 	This should perform all the initialisation specific to the port
 * used for output to the LCD display.
 * @param   None
 * @return  None
 **/
static void
init_display_port(void)
{
    wait_microsec(3000); // Delay of 3 ms

    // Port A
    SYSCTL_RCGC2_R |= 0x00000001; // Enables clock for port A
    GPIO_PORTA_CR_R |= 0x0C;      // Allow changes to PA 2 to 3
    GPIO_PORTA_DIR_R |= 0x0C;     // Sets PA 2 to 3 as output
    GPIO_PORTA_DEN_R |= 0x0C;     // Enable digital pins on PA 2 to 3
    GPIO_PORTA_AFSEL_R = 0x00;    // Disables alternate function
    GPIO_PORTA_AMSEL_R = 0x00;    // Disable analog function

    wait_microsec(3000);          // Delay of 3 ms

    // Port B
    SYSCTL_RCGC2_R |= 0x00000002; // Enables clock for port B
    GPIO_PORTB_CR_R |= 0x3C;      // Allow changes to PB 2 to 5
    GPIO_PORTB_DIR_R |= 0x3C;     // Sets PB 2 to 5 as output
    GPIO_PORTB_DEN_R |= 0x3C;     // Enable digital pins on PB 2 to 5
    GPIO_PORTB_AMSEL_R = 0x00;    // Disable analog function
    GPIO_PORTB_AFSEL_R = 0x00;    // Disables alternate function

    wait_microsec(15000);         // Delay of 15 ms after powering on

    send_display_nibble(0x3, 0);
    wait_microsec(4100);         // Delay of 4.1 ms
    send_display_nibble(0x3, 0);
    wait_microsec(100);          // Delay of 100 microsec
    send_display_nibble(0x3, 0);
    wait_microsec(37);           // Delay of 37 Microsec

    send_display_nibble(0x2, 0); // Sets the LCD to 4 bit mode
    wait_microsec(37);           // Delay of 37 Microsec
    send_display_byte(0x28, 0);  // Specifies the number of display lines and fonts
    send_display_byte(0x06, 0);  // Display Off
    send_display_byte(0x01, 0);  // Display clear
    send_display_byte(0x0F, 0);  // Entry Mode Set

#if LCD_TESTING    
    send_display_byte('t', 1);
    send_display_byte('e', 1);
    send_display_byte('s', 1);
    send_display_byte('t', 1);
#endif /* LCD_TESTING */
}

/**
 * @brief 	Pulses the LCD EN line
 * @param   None
 * @return  None
 **/
static void
lcd_pulse(void)
{
    LCD_EN = 1 << 2;  // this sets the LCD_EN to 1 and shifts it to the correct bit
    wait_microsec(1); // Delay of 1 microsecond
    LCD_EN = 0;       // Sets the LCD_EN to 0
    wait_microsec(1); // Delay of 1 microsecond
}

/**
 * @brief 	Initialise everything other than keyboard, display and flash.
 * @param   None
 * @return  None
 **/
static void
init_all_other(void)
{
    PLL_init();     // Initialisation of phase locked loop
    systick_init(); // Initialisation of SysTick
}

/**********************************************************************************************
 * End of file
 **********************************************************************************************/
