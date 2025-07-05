/**
 * $File: mid_level_funcs.c
 *
 *  *******************************************************************************************
 *
 *  @file      mid_level_funcs.c
 *
 *  @brief     Set of functions at middle level (above low-level hardware drivers but below the
 * 			   high level).
 *  *******************************************************************************************
 *
 *  $NoKeywords
 **/

/**********************************************************************************************
 * Module includes
 **********************************************************************************************/
#include "mid_level_funcs.h"
#include "low_level_funcs_tiva.h"
#include <stddef.h>
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
#define ROW_ONE   0x01
#define ROW_TWO   0x02
#define ROW_THREE 0x04
#define ROW_FOUR  0x08
/**********************************************************************************************
 * Private type definitions
 **********************************************************************************************/

/**********************************************************************************************
 * Private function declarations
 **********************************************************************************************/
static void keyboard_read_row_col(uint8_t *p_row, uint8_t *p_col);
static char keyboard_row_col_to_char(uint8_t row, uint8_t col);
/**********************************************************************************************
 * Private variable definitions
 **********************************************************************************************/

/**********************************************************************************************
 * Public function definitions
 **********************************************************************************************/

/**
 * @brief   Get the next character from keyboard.
 * This function reads from the 16-key keypad. It waits until the user has
 * pressed a key and then returns it as an ASCII character.
 * @param   None.
 * @return  KeyPressed The character read.
 **/
char
get_keyboard_char(void)
{
    uint8_t row = 0;
    uint8_t col = 0;
    char    key_pressed = '\0';

    keyboard_read_row_col(&row, &col);                // gets what row or coloumn is pressed
    key_pressed = keyboard_row_col_to_char(row, col); // passes	what column or row is pressed and gets the coressponding key

    return key_pressed;
}

/**
 * @brief Print a string at a specified location on the LCD display.
 * This function will call functions in \a low_level_funcs_tiva to set up
 * the print position and write individual characters.
 * @param [in] line The line number, 1 for top or 2 for bottom.
 * @param [in] char_pos The character position, counting from 1 at the left to 16 at the right.
 * @param [in] string A C-format string to be displayed.
 *
 * @return None
 **/
void
print_string(const uint8_t line, const uint8_t char_pos, const char *p_string)
{
    if (NULL != p_string)
    {
        set_print_position(line, char_pos); // sets line to write in and what position to start writing

        while (*p_string)
        {
            print_char(*p_string); // Print each character from the string.
            p_string++;
        }
    }
}

/**********************************************************************************************
 * Private function definitions
 **********************************************************************************************/
/**
 * @brief   Scans the keypad to detect a keypress and identifies the corresponding row and column.
 * @param [in]  p_row - Pointer to a variable where the detected row number (1 to 4) will be stored.
 * @param [in]  p_col - Pointer to a variable where the detected column number (1 to 4) will be stored.
 * @return  None.
 */
static void
keyboard_read_row_col(uint8_t *p_row, uint8_t *p_col)
{
    unsigned char row_mask = 0;

    for (size_t column_index = 1; column_index <= 4; column_index++)
    {
        write_keyboard_col(column_index);
        row_mask = read_keyboard_row();

        if (0x00 != row_mask)
        {
            *p_col = column_index;

            switch (row_mask)
            {
                case ROW_ONE:
                    *p_row = 1;
                    break;

                case ROW_TWO:
                    *p_row = 2;
                    break;

                case ROW_THREE:
                    *p_row = 3;
                    break;

                case ROW_FOUR:
                    *p_row = 4;
                    break;
                default:
                    break;
            }
            break;
        }
    }
}

/**
 * @brief   Converts a keypad row and column number to the corresponding character.
 * @param [in]   row - Row number of the key (1 to 4).
 * @param [in]   col - Column number of the key (1 to 4).
 * @return  The character corresponding to the specified row and column, or '?' if inputs are invalid.
 */
static char
keyboard_row_col_to_char(const uint8_t row, const uint8_t col)
{

    /* Validate input first */
    if (row < 1 || row > 4 || col < 1 || col > 4)
    {
        return '?'; // Invalid row or column
    }
    /*Define 4x4 keypad layout*/
    const char keymap[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    return keymap[row - 1][col - 1];
}
/**********************************************************************************************
 * End of file
 **********************************************************************************************/
