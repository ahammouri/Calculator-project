/**
 * $File: calculate_answer.c
 *
 *  *******************************************************************************************
 *
 *  @file      calculate_answer.c
 *
 *  @brief     The calculation function and associated constants.
 *  *******************************************************************************************
 *
 *  $NoKeywords
 **/

/**********************************************************************************************
 * Module includes
 **********************************************************************************************/
#include "calculate_answer.h"
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**********************************************************************************************
 * Referenced external functions
 **********************************************************************************************/

/**********************************************************************************************
 * Referenced external variables
 **********************************************************************************************/

/**********************************************************************************************
 * Global variable definitions
 **********************************************************************************************/
const char error_message_line1[MAX_ERROR_MESSAGES][17] = {
    "No error",     "Unidentified",    "SOFT BUG: Empty", "No null or too",
    "Invalid char", "Number with > 1", "Invalid number",  "May not start",
    "May not end",  "Two adjacent",    "Two adjacent",    "E must be foll-",
};
const char error_message_line2[MAX_ERROR_MESSAGES][17] = {
    "No error",
    "error",
    "input string",
    "long I/P string",
    "in input string",
    "decimal point",
    "",
    "with +,x,/ or E",
    "with operator",
    "operators",
    "E operators",
    "owed by integer",
};

/**********************************************************************************************
 * Private constant definitions
 **********************************************************************************************/
#define MAX_NUMS_AND_OPS 20
#define MAX_NUMBER_STRING_LENGTH 50 // Added bounds checking constant

/**********************************************************************************************
 * Private type definitions
 **********************************************************************************************/
typedef struct {
  double number[MAX_NUMS_AND_OPS];
  int n_numbers;
  char infix_operator[MAX_NUMS_AND_OPS];
  int n_infix_operators;
  char num_and_op_used[MAX_NUMS_AND_OPS];
} ParsedExpression_t;

/**********************************************************************************************
 * Private function declarations
 **********************************************************************************************/
static bool is_operator(char character);
static void syntax_check_stage1(char *p_input_buffer, uint8_t max_buffer_size,
                                uint8_t *p_error_ref_no);
static void syntax_check_stage2(char *p_input_buffer, uint8_t *p_error_ref_no);
static double simple_atof(const char *p_string);
static void extract_number(char *p_input_buffer, uint8_t *p_ch_no,
                           uint8_t buf_len,
                           ParsedExpression_t *p_parsed_expression,
                           uint8_t *p_error_ref_no);
static void extract_operator(char *p_input_buffer, uint8_t *p_ch_no,
                             uint8_t buf_len,
                             ParsedExpression_t *p_parsed_expression,
                             uint8_t *p_error_ref_no);
static void identify_tokens(char *p_input_buffer, uint8_t *p_error_ref_no,
                            ParsedExpression_t *p_parsed_expression);
static void syntax_check_stage3(ParsedExpression_t parsed_expression,
                                uint8_t *p_error_ref_no);
static void merge_numbers(ParsedExpression_t *p_parsed_expression,
                          uint8_t current_index, uint8_t next_index,
                          char operator);
static void
evaluate_expression_one_operator(ParsedExpression_t *p_parsed_expression,
                                 char operator, uint8_t * p_error_ref_no);
static double evaluate_expression(ParsedExpression_t p_parsed_expression,
                                  uint8_t *p_error_ref_no);

/**********************************************************************************************
 * Private variable definitions
 **********************************************************************************************/

/**********************************************************************************************
 * Public function definitions
 **********************************************************************************************/

/**
 * @brief   Parse the input from keyboard and produce either the answer or an
 *error message.
 * @param [in] input_buffer A string with the characters read from keyboard.
 * @param [in] input_buffer_size The size of the input_buffer array
 * @param [out] The reference number of the error, if any.
 * @return  If there was no error, the result of the calculation is returned.
 * 		If there was an error, 0.0 is returned.
 **/
double CalculateAnswer(char *p_input_buffer, uint8_t input_buffer_size,
                       uint8_t *p_error_ref_no) {
  double answer = 0.0;
  ParsedExpression_t parsed_expression;
  *p_error_ref_no = 0;

  // Basic syntax checks:
  syntax_check_stage1(p_input_buffer, input_buffer_size, p_error_ref_no);

  if (0u != *p_error_ref_no) {
    return 0.0; // Even if it won't be used, the result should be defined.
  }

  // No operator errors (e.g. two together):
  syntax_check_stage2(p_input_buffer, p_error_ref_no);

  if (0u != *p_error_ref_no) {
    return 0.0; // Even if it won't be used, the result should be defined.
  }

  /* Parse the input string into tokens (representing numbers
     and operators such as +, x): */
  parsed_expression.n_numbers = parsed_expression.n_infix_operators = 0;
  identify_tokens(p_input_buffer, p_error_ref_no, &parsed_expression);

  if (0u != *p_error_ref_no) {
    return 0.0; // Even if it won't be used, the result should be defined.
  }

  /* There should not be two E operators following each other
    (e.g. 12.E3E4). This is easier to test once the input has been
    parsed into tokens: */
  syntax_check_stage3(parsed_expression, p_error_ref_no);

  if (0u != *p_error_ref_no) {
    return 0.0; // Even if it won't be used, the result should be defined.
  }

  /* The input string is now known to be valid, so evaluate it:*/
  answer = evaluate_expression(parsed_expression, p_error_ref_no);

  return answer;
}

/**********************************************************************************************
 * Private function definitions
 **********************************************************************************************/

/**
 * @brief   Checks if a character is a mathematical operator.
 * @param[in]   character - The character to be evaluated.
 * @return  true if the character is an operator (+, -, x, /, E), false
 *otherwise.
 **/
static bool is_operator(char character) {
  bool b_is_operator = false;
  switch (character) {
  case '+':
  case '-':
  case 'x':
  case '/':
  case 'E':
    b_is_operator = true;
    break;
  default:
    b_is_operator = false;
    break;
  }

  return b_is_operator;
}

/**
 * @brief   Performs basic syntax checks on the input buffer.
 *
 * This function validates the input string by checking for:
 * - Empty strings
 * - Missing null-terminator or string length exceeding buffer size
 * - Invalid characters (only digits, '+', '-', 'x', '/', '.', 'E' are allowed)
 *
 * Only the first encountered error is reported via the error reference number.
 *
 * @param[in]  p_input_buffer     Pointer to the input string buffer.
 * @param[in]  max_buffer_size    Maximum allowed buffer size.
 * @param[out] p_error_ref_no     Pointer to a variable where the error code
 * will be stored:
 *                                - 0: No error
 *                                - 2: Empty string
 *                                - 3: Null terminator missing or string too
 * long
 *                                - 4: Invalid character found
 * @return     void
 */
static void syntax_check_stage1(char *p_input_buffer, uint8_t max_buffer_size,
                                uint8_t *p_error_ref_no) {
  uint8_t index;
  uint8_t actual_buffer_size;

  // Empty string (should have been handled in main()):
  if ('\0' == p_input_buffer[0]) {
    *p_error_ref_no = 2; // "SOFT BUG: Empty"
    return;              // Only report first error, so don't check for more.
  }

  // Null missing or string too long:
  *p_error_ref_no = 3;
  for (index = 0; index < max_buffer_size; index++) {
    if ('\0' == p_input_buffer[index]) {
      *p_error_ref_no = 0;
      break;
    }
  }

  if (0u != *p_error_ref_no) {
    return; // Only report first error, so don't check for more.
  }

  actual_buffer_size = strlen(p_input_buffer); /* Null found, so strlen is safe
                                                  and no need for strnlen. */

  // Invalid char:
  for (index = 0; index < actual_buffer_size; index++) {
    switch (p_input_buffer[index]) {
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
    case '+':
    case '-':
    case 'x':
    case '/':
    case '.':
    case 'E':
      break;
    default:
      *p_error_ref_no = 4;
      return;
    }
  }
}

/**
 * @brief   Performs advanced syntax checks on the input buffer.
 *
 * This function validates operator usage in a mathematical expression string.
 * It checks for:
 * - Operators at invalid positions (start or end, with exceptions)
 * - Invalid sequences of adjacent operators (only 'x-', '/-', 'E-' are allowed)
 * - Use of 'E' must be followed by an integer (no decimal points allowed until
 * next operator or end)
 *
 * Only the first encountered error is reported via the error reference number.
 *
 * @param[in]  p_input_buffer   Pointer to the input string buffer
 * (null-terminated).
 * @param[out] p_error_ref_no   Pointer to a variable where the error code will
 * be stored:
 *                              - 0: No error
 *                              - 7: Operator at invalid start position (except
 * leading '-')
 *                              - 8: Operator at invalid end position
 *                              - 9: Invalid pair of adjacent operators
 *                              - 11: 'E' not followed by a valid integer
 * @return     void
 */
static void syntax_check_stage2(char *p_input_buffer, uint8_t *p_error_ref_no) {
  uint8_t index;
  uint8_t buffer_size = strlen(p_input_buffer);
  char ch1 = p_input_buffer[0];

  /* First and last characters may not be operators,
   * except that the first may be a minus: */
  if ((is_operator(ch1)) && ('-' != ch1)) {
    *p_error_ref_no = 7;
    return; // Only report first error, so don't check for more.
  }
  if (is_operator(p_input_buffer[buffer_size - 1])) {
    *p_error_ref_no = 8;
    return; // Only report first error, so don't check for more.
  }

  /* There are only 3 valid cases of an operator following immediately
   * after another: x- /- and E- . */
  for (index = 0; index < buffer_size - 1; index++) {
    ch1 = p_input_buffer[index];
    char ch2 =
        p_input_buffer[index +
                       1]; /* The loop stops before buffer_size-1,
                           one before the end, so ch2 will not over-run. */
    bool b_okay = true;
    if ((is_operator(ch1)) && (is_operator(ch2))) {
      b_okay = false; // Adjacent operators are invalid by default
      if (ch2 == '-' && (ch1 == 'x' || ch1 == '/' || ch1 == 'E')) {
        b_okay = true; // Exception: x-, /-, E- are valid
      }
    }

    if (false == b_okay) { // Error: no need to check the rest of the string.
      *p_error_ref_no = 9; // "Two adjacent" "operators"
      return;              // Only report first error, so don't check for more.
    }
  }

  /* An E operator must be followed by an integer (not, e.g., 1.2E3.4).
   * Check that after an E there are no dots
   * until next operator or end of buffer: */
  for (index = 0; index < buffer_size - 1; index++) {
    if ('E' == p_input_buffer[index]) {
      for (size_t buffer_index = index + 1; buffer_index < buffer_size;
           buffer_index++) {
        if (is_operator(p_input_buffer[buffer_index])) {
          break; /* From the inner loop.
                  * We've met the next operator
                  * before a dot: we're happy.
                  * (NB - dot does not count
                  * as an operator for
                  * is_operator().) */
        }
        if ('.' == p_input_buffer[buffer_index]) {
          *p_error_ref_no = 11; /*
              "E must be foll-" "owed by integer" */
          return;               /* Only report first error,
                               so don't check for more. */
        }
      }
    }
  }
}

/**
 * @brief   Converts a simplified ASCII string to a floating-point number.
 *
 * This function parses a numeric string and returns its double-precision
 * floating-point value. It supports:
 * - Leading whitespace
 * - Optional '+' or '-' sign
 * - Integer and fractional parts (e.g., "123.456")
 *
 * It does **not** handle:
 * - Scientific notation (e.g., "1.23e4")
 * - Invalid characters or error reporting
 *
 * @param[in]  p_string   Pointer to a null-terminated string containing the
 * numeric input.
 * @return     The corresponding double-precision floating-point value.
 */
static double simple_atof(const char *p_string) {
  int sign = 1;
  int int_part = 0;
  double frac_part = 0.0;
  double divisor = 10.0;

  // Skip leading whitespace
  while (' ' == *p_string) {
    p_string++;
  }

  // Handle optional sign
  if ('-' == *p_string) {
    sign = -1;
    p_string++;
  } else if ('+' == *p_string) {
    p_string++;
  }

  // Integer part
  while ((*p_string >= '0') && (*p_string <= '9')) {
    int_part = int_part * 10 + (*p_string - '0');
    p_string++;
  }

  // Fractional part
  if ('.' == *p_string) {
    p_string++;
    while ((*p_string >= '0') && (*p_string <= '9')) {
      frac_part += (*p_string - '0') / divisor;
      divisor *= 10;
      p_string++;
    }
  }

  return sign * (int_part + frac_part);
}

/**
 * @brief   Extracts a numeric value from the input buffer and stores it in a
 * parsed expression structure.
 *
 * This function reads characters from the input buffer starting at the
 * specified position and attempts to parse a numeric value (integer or
 * decimal). The extracted number is converted to `double` using `simple_atof()`
 * and added to the `ParsedExpression_t` structure.
 *
 * It performs basic validation, ensuring the number starts with a digit or '.'
 * and stops reading when a non-digit, non-dot character is encountered or the
 * buffer limit is reached.
 *
 * @param[in]     p_input_buffer       Pointer to the input string buffer
 * containing the expression.
 * @param[in,out] p_ch_no              Pointer to the current character index;
 * updated to the next unread position.
 * @param[in]     buf_len              Length of the input buffer.
 * @param[out]    p_parsed_expression  Pointer to the structure where the parsed
 * number will be stored.
 * @param[out]    p_error_ref_no       Pointer to a variable where error code
 * will be stored:
 *                                     - 0: No error
 *                                     - 6: Invalid starting character (not
 * digit or '.')
 *
 * @return        void
 */
static void extract_number(char *p_input_buffer, uint8_t *p_ch_no,
                           uint8_t buf_len,
                           ParsedExpression_t *p_parsed_expression,
                           uint8_t *p_error_ref_no) {
  char num_as_string[MAX_NUMBER_STRING_LENGTH] = {0};
  int next_ch_no = 0;

  // Sanity check: Must start with digit or '.'
  if ((!isdigit((unsigned char)p_input_buffer[*p_ch_no])) &&
      ('.' != p_input_buffer[*p_ch_no])) {
    *p_error_ref_no = 6;
    return;
  }

  // Extract number characters with bounds checking
  while (*p_ch_no < buf_len && next_ch_no < (MAX_NUMBER_STRING_LENGTH - 1) &&
         (isdigit((unsigned char)p_input_buffer[*p_ch_no]) ||
          p_input_buffer[*p_ch_no] == '.')) {
    num_as_string[next_ch_no++] = p_input_buffer[*p_ch_no];
    (*p_ch_no)++;
  }

  num_as_string[next_ch_no] = '\0'; // Null-terminate the number string

  if (0 == next_ch_no) {
    *p_error_ref_no = 6;
    return;
  }

  double number_read = simple_atof(num_as_string);

  if (p_parsed_expression->n_numbers < MAX_NUMS_AND_OPS) {
    p_parsed_expression->number[p_parsed_expression->n_numbers++] = number_read;
  } else {
    *p_error_ref_no = 1; // Too many numbers
  }
}

/**
 * @brief   Extracts a mathematical operator from the input buffer and stores it
 * in a parsed expression structure.
 *
 * This function checks whether the current character in the input buffer is a
 * valid infix operator
 * (`+`, `-`, `x`, `/`, `E`). If valid, it stores the operator in the
 * `ParsedExpression_t` structure and advances the character index. If not
 * valid, it sets an error code.
 *
 * @param[in]     p_input_buffer       Pointer to the input string buffer
 * containing the expression.
 * @param[in,out] p_ch_no              Pointer to the current character index;
 * updated if an operator is found.
 * @param[in]     buf_len              Length of the input buffer.
 * @param[out]    p_parsed_expression  Pointer to the structure where the parsed
 * operator will be stored.
 * @param[out]    p_error_ref_no       Pointer to a variable where the error
 * code will be stored:
 *                                     - 0: No error
 *                                     - 7: Invalid operator
 *
 * @return        void
 */
static void extract_operator(char *p_input_buffer, uint8_t *p_ch_no,
                             uint8_t buf_len,
                             ParsedExpression_t *p_parsed_expression,
                             uint8_t *p_error_ref_no) {
  if (*p_ch_no >= buf_len) {
    return;
  }

  char ch = p_input_buffer[*p_ch_no];

  if (('+' == ch) || ('-' == ch) || ('x' == ch) || ('/' == ch) || ('E' == ch)) {
    if (p_parsed_expression->n_infix_operators < MAX_NUMS_AND_OPS) {
      p_parsed_expression
          ->infix_operator[p_parsed_expression->n_infix_operators++] = ch;
      (*p_ch_no)++;
    } else {
      *p_error_ref_no = 1; // Too many operators
    }
  } else {
    *p_error_ref_no = 7; // Invalid operator
  }
}

/**
 * @brief   Tokenises an input mathematical expression into numbers and
 * operators.
 *
 * This function parses a string containing a simple mathematical expression,
 * extracting numeric values and valid operators (`+`, `-`, `x`, `/`, `E`) and
 * storing them in the `ParsedExpression_t` structure. It enforces strict syntax
 * rules and reports the first error encountered via an error code.
 *
 * Whitespace is ignored between tokens. Numbers must start with a digit or a
 * decimal point, and valid tokens must alternate between number → operator →
 * number, etc.
 *
 * @param[in]      p_input_buffer       Pointer to the input null-terminated
 * expression string.
 * @param[out]     p_error_ref_no       Pointer to a variable where the error
 * code will be stored:
 *                                      - 0: No error
 *                                      - 6: Expected number but found invalid
 * character
 *                                      - 7: Expected operator but found invalid
 * character
 * @param[out]     p_parsed_expression  Pointer to the structure where parsed
 * numbers and operators will be stored.
 *
 * @return         void
 */
static void identify_tokens(char *p_input_buffer, uint8_t *p_error_ref_no,
                            ParsedExpression_t *p_parsed_expression) {
  uint8_t ch_no = 0;
  uint8_t buf_len = strlen(p_input_buffer);

  while (ch_no < buf_len) {
    // Must be number
    if (isdigit((unsigned char)p_input_buffer[ch_no]) ||
        ('.' == p_input_buffer[ch_no])) {
      extract_number(p_input_buffer, &ch_no, buf_len, p_parsed_expression,
                     p_error_ref_no);
      if (*p_error_ref_no != 0) {
        return;
      }
    } else {
      *p_error_ref_no = 6;
      return;
    }

    // Skip whitespace
    while ((ch_no < buf_len) &&
           (isspace((unsigned char)(p_input_buffer[ch_no])))) {
      ch_no++;
    }

    // Must be operator or end
    if (ch_no < buf_len) {
      if (p_input_buffer[ch_no] == '+' || p_input_buffer[ch_no] == '-' ||
          p_input_buffer[ch_no] == 'x' || p_input_buffer[ch_no] == '/' ||
          p_input_buffer[ch_no] == 'E') {
        extract_operator(p_input_buffer, &ch_no, buf_len, p_parsed_expression,
                         p_error_ref_no);
        if (*p_error_ref_no != 0) {
          return;
        }
      } else {
        *p_error_ref_no = 7; // unexpected char
        return;
      }
    }
  }
}

/**
 * @brief   Performs a specific syntax check for adjacent 'E' operators in the
 * parsed expression.
 *
 * Unlike earlier syntax checks that operate on the raw input string, this
 * function inspects the parsed list of infix operators within a
 * `ParsedExpression_t` structure. It looks for two consecutive `'E'`
 * characters, which would indicate a malformed scientific notation or invalid
 * usage.
 *
 * If such a pattern is found, an error code is set.
 *
 * @param[in]   p_parsed_expression  Parsed expression structure containing
 * extracted operators.
 * @param[out]  p_error_ref_no       Pointer to a variable where the error code
 * will be stored:
 *                                   - 0: No error
 *                                   - 10: Two adjacent 'E' characters found
 *
 * @return      void
 */
static void syntax_check_stage3(ParsedExpression_t p_parsed_expression,
                                uint8_t *p_error_ref_no) {
  int i;
  for (i = 0; i < p_parsed_expression.n_infix_operators - 1; i++) {
    if (('E' == p_parsed_expression.infix_operator[i]) &&
        ('E' == p_parsed_expression.infix_operator[i + 1])) {
      *p_error_ref_no = 10;
      return;
    }
  }
}

/**
 * @brief   Merges two numbers in the parsed expression using a specified
 * operator.
 *
 * This function performs an arithmetic operation between two numbers in a
 * `ParsedExpression_t` structure using the given operator. The result of the
 * operation replaces the number at `next_index`. The number at `current_index`
 * is marked as "used" and is ignored in future evaluations.
 *
 * The order of operand selection is determined externally (i.e., the caller
 * decides the correct order), and the indices provided reflect that decision.
 * This function does not sort or validate the operand order.
 *
 * Supported operators:
 * - `+`: Addition
 * - `-`: Subtraction
 * - `x`: Multiplication
 * - `/`: Division
 * - `E`: Scientific notation (num1 * 10^num2)
 *
 * @param[in,out]  p_parsed_expression  Pointer to the parsed expression
 * structure containing numbers and usage flags.
 * @param[in]      current_index        Index of the first number (to be marked
 * as used after the operation).
 * @param[in]      next_index           Index of the second number (to be
 * replaced with the result).
 * @param[in]      operator             Arithmetic operator to apply between the
 * two numbers.
 *
 * @return         void
 */
static void merge_numbers(ParsedExpression_t *p_parsed_expression,
                          uint8_t current_index, uint8_t next_index,
                          char operator) {
  double num1 = p_parsed_expression->number[current_index];
  double num2 = p_parsed_expression->number[next_index];
  switch (operator) {
  case '+':
    p_parsed_expression->number[next_index] = num1 + num2;
    break;
  case '-':
    p_parsed_expression->number[next_index] = num1 - num2;
    break;
  case 'x':
    p_parsed_expression->number[next_index] = num1 * num2;
    break;
  case '/':
    p_parsed_expression->number[next_index] = num1 / num2;
    break;
  case 'E':
    p_parsed_expression->number[next_index] = num1 * pow(10.0, num2);
    break;
  }

  p_parsed_expression->num_and_op_used[current_index] = 1; // Record as used.
}

/**
 * @brief   Evaluates all occurrences of a specific operator in a parsed
 * expression.
 *
 * This function processes the parsed mathematical expression and evaluates
 * every instance of the specified infix operator (e.g., `+`, `-`, `x`, `/`,
 * `E`). For each occurrence, it finds the next unused number, performs the
 * arithmetic operation via `merge_numbers()`, and stores the result. The
 * left-hand operand is marked as used, and the result replaces the right-hand
 * operand.
 *
 * The function assumes operator precedence is handled externally — it only
 * processes one operator type at a time.
 *
 * @param[in,out]  p_parsed_expression  Pointer to the parsed expression
 * structure containing numbers, operators, and usage tracking.
 * @param[in]      operator             The operator character to evaluate
 * (e.g., '+', '-', 'x', '/', 'E').
 * @param[out]     p_error_ref_no       Pointer to a variable where an error
 * code is stored if evaluation fails:
 *                                      - 0: No error
 *                                      - 1: Evaluation error (e.g., missing
 * operand)
 *
 * @return         void
 */
static void
evaluate_expression_one_operator(ParsedExpression_t *p_parsed_expression,
                                 char operator, uint8_t * p_error_ref_no) {
  uint8_t this_index;
  uint8_t next_index;
  bool b_found;

  for (this_index = 0; this_index < p_parsed_expression->n_infix_operators;
       this_index++) {
    // Is it the right operator?
    if (p_parsed_expression->infix_operator[this_index] != operator) {
      continue;
    }

    // Find the next unused number to merge into it:
    b_found = false;
    for (next_index = this_index + 1;
         next_index < p_parsed_expression->n_numbers; next_index++) {
      if (!p_parsed_expression->num_and_op_used[next_index]) {
        b_found = true;
        break;
      }
    }

    // Error check:
    if (false == b_found) {
      *p_error_ref_no = 1; // Unidentified error.
      return;
    }

    /* We have now found the next two numbers to merge by
     * performing the arithmetic operation: */
    merge_numbers(p_parsed_expression, this_index, next_index, operator);
  }
}

/**
 * @brief   Evaluates a parsed mathematical expression according to standard
 * operator precedence.
 *
 * This function processes a `ParsedExpression_t` structure, which contains
 * numbers and infix operators, and evaluates the entire expression in-place. It
 * uses a left-to-right strategy within each precedence level and respects
 * standard operator precedence in the following order:
 * - `'E'` (scientific notation exponent)
 * - `'/'` (division)
 * - `'x'` (multiplication)
 * - `'+'` (addition)
 * - `'-'` (subtraction)
 *
 * Intermediate results replace operands in the expression, and used numbers are
 * marked in a dedicated `num_and_op_used[]` array to ensure correct
 * left-to-right association.
 *
 * @note The function assumes the parsed expression is valid and properly
 * structured. It does not handle parentheses or nested expressions.
 *
 * @param[in]   p_parsed_expression   Parsed expression structure containing
 * numbers, operators, and state.
 * @param[out]  p_error_ref_no        Pointer to a variable where error code
 * will be stored:
 *                                    - 0: No error
 *                                    - 1: Evaluation error (e.g., missing
 * operands)
 *
 * @return      The final computed value as a `double`. If an error occurs, the
 * return value may be undefined.
 */
static double evaluate_expression(ParsedExpression_t p_parsed_expression,
                                  uint8_t *p_error_ref_no) {
  // Init the number_used array to show no numbers have been used.
  for (size_t index = 0; index < MAX_NUMS_AND_OPS; index++) {
    p_parsed_expression.num_and_op_used[index] = 0;
  }

  // Evaluate in order:
  evaluate_expression_one_operator(&p_parsed_expression, 'E', p_error_ref_no);
  if (0u != *p_error_ref_no)
    return 0.0;

  evaluate_expression_one_operator(&p_parsed_expression, '/', p_error_ref_no);
  if (0u != *p_error_ref_no)
    return 0.0;

  evaluate_expression_one_operator(&p_parsed_expression, 'x', p_error_ref_no);
  if (0u != *p_error_ref_no)
    return 0.0;

  evaluate_expression_one_operator(&p_parsed_expression, '+', p_error_ref_no);
  if (0u != *p_error_ref_no)
    return 0.0;

  evaluate_expression_one_operator(&p_parsed_expression, '-', p_error_ref_no);
  if (0u != *p_error_ref_no)
    return 0.0;

  /* There should now be nothing left except the number in the last
   * element of parsed_expression.number, which is the answer.
   */
  return p_parsed_expression.number[p_parsed_expression.n_numbers - 1];
}

/**********************************************************************************************
 * End of file
 **********************************************************************************************/
