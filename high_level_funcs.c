/**
 * $File: high_level_funcs.c
 *
 *  *******************************************************************************************
 *
 *  @file      high_level_funcs.c
 *
 *  @brief     Set of functions at high level (above mid level but below main)
 *
 *  *******************************************************************************************
 *
 *  $NoKeywords
 **/

/**********************************************************************************************
 * Module includes
 **********************************************************************************************/
#include "high_level_funcs.h"
#include "mid_level_funcs.h"
#include "low_level_funcs_tiva.h"
#include <stdio.h>

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

/**********************************************************************************************
 * Private type definitions
 **********************************************************************************************/

/**********************************************************************************************
 * Private function declarations
 **********************************************************************************************/

/**********************************************************************************************
 * Private variable definitions
 **********************************************************************************************/

/**********************************************************************************************
 * Public function definitions
 **********************************************************************************************/

/**
 * @brief Reads input from a keyboard and echoes it to a display buffer.
 *
 * This function continuously reads characters from a keyboard, processes them,
 * and displays the input on the screen. It handles digit and operator input,
 * supports shift key logic for alternate characters, allows backspacing, and
 * ends input on receiving the '*' character. The input is stored in the provided
 * buffer.
 *
 * @param[out] input_buffer Pointer to the buffer where the input will be stored.
 * @param[in] input_buffer_size Size of the input buffer (maximum characters to store).
 */
void
ReadAndEchoInput(char *input_buffer, int input_buffer_size)
{
    int  j = 0;
    bool b_shift_key_pressed = false;
    char key;
	bool b_cleared = false;

    turn_cursor_on_off(1);

    while (1)
    {
        wait_microsec(1000);
        key = get_keyboard_char();

        if (j >= input_buffer_size - 1)
        {
            j = input_buffer_size - 1;
            break; // Prevent buffer overflow
        }

		if ((key != '?') && false == b_cleared)
		{
			clear_display();
			b_cleared = true;
		}
        switch (key)
        {
            // Digits
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (j < 16)
                {
                    input_buffer[j++] = key;
                    input_buffer[j] = '\0';
                    print_string(1, 0, input_buffer);
                }

                b_shift_key_pressed = false;
                wait_microsec(1000000);
                break;

            // Operator mapping with ShiftKey
            case 'A':
                input_buffer[j++] = (b_shift_key_pressed ? 'x' : '+');
                input_buffer[j] = '\0';
                print_string(1, 0, input_buffer);
                b_shift_key_pressed = false;
                wait_microsec(1000000);
                break;

            case 'B':
                input_buffer[j++] = (b_shift_key_pressed ? '/' : '-');
                input_buffer[j] = '\0';
                print_string(1, 0, input_buffer);
                b_shift_key_pressed = false;
                wait_microsec(1000000);
                break;

            case 'C':
                input_buffer[j++] = (b_shift_key_pressed ? 'E' : '.');
                input_buffer[j] = '\0';
                print_string(1, 0, input_buffer);
                b_shift_key_pressed = false;
                wait_microsec(1000000);
                break;

            case 'D': // Shift key
                b_shift_key_pressed = true;
                break;

            case '#':
                if (b_shift_key_pressed)
                {
                    if (j > 0)
                    {
                        j--;
                        input_buffer[j] = '\0';
                        print_string(1, 0, input_buffer);
                        print_char(0x20);
                        set_print_position(1, j);
                    }
                }
                else
                {
                    j = 0;
                    clear_display();
                }
                b_shift_key_pressed = false;
                wait_microsec(1000000);
                break;

            case '*': // End input
                return;

            default:
                // ignore unsupported keys
                break;
        }
    }
}

/**
 * @brief Displays a floating-point result on the second line of the display.
 *
 * This function formats a double-precision number to two decimal places and
 * prints it on the second line of the display. It also turns off the cursor
 * before displaying the result.
 *
 * @param[in] answer The floating-point value to be displayed.
 */
void
DisplayResult(double answer)
{
    //char result[16];
	int int_part = (int)answer;
	int frac_part = (int)((answer - int_part) * 100);  // 2 decimal places
	char result_str[20];

    turn_cursor_on_off(0); // Turns cursor off
	if (frac_part < 0) frac_part = -frac_part;

	snprintf(result_str, sizeof(result_str), "%d.%02d", int_part, frac_part);

	print_string(2, 1, result_str);		   // Prints the answer in the second line
}
/**
 * @brief Displays a two-line error message on the screen.
 *
 * This function clears the display, turns off the cursor, and then prints
 * the provided error messages on the first and second lines of the display.
 *
 * @param[in] error_message_line1 The message to display on the first line.
 * @param[in] error_message_line2 The message to display on the second line.
 */
void
DisplayErrorMessage(const char *error_message_line1, const char *error_message_line2)
{
    clear_display();                         // clear display
    turn_cursor_on_off(0);                   // Turns cursor off
    print_string(1, 0, error_message_line1); // Display error message on line 1
    print_string(2, 0, error_message_line2); // Display error message on line 2
}

/**********************************************************************************************
 * Private function definitions
 **********************************************************************************************/

/**********************************************************************************************
 * End of file
 **********************************************************************************************/
