# Calculator Project

A robust embedded calculator implementation designed for TivaWare-based microcontrollers with LCD display and keypad input.

## Overview

This project implements a scientific calculator with a layered software architecture, featuring comprehensive error handling, mathematical expression parsing, and hardware abstraction for embedded systems.

## Features

- **Basic Arithmetic Operations**: Addition (+), Subtraction (-), Multiplication (x), Division (/)
- **Scientific Notation**: Support for exponential notation (E)
- **Comprehensive Error Handling**: Multi-stage syntax validation with user-friendly error messages
- **Persistent Memory**: Flash storage for calculator results
- **Hardware Abstraction**: Clean separation between hardware drivers and application logic
- **Robust Input Parsing**: Multi-stage tokenization and validation

## Architecture

The project follows a layered architecture pattern:

```
main.c                    - Top-level program flow
├── high_level_funcs      - User interface functions
├── mid_level_funcs       - Hardware abstraction layer  
├── low_level_funcs_tiva  - TivaWare hardware drivers
└── calculate_answer      - Mathematical computation engine
```

### Key Components

- **Main Controller** (`main.c`): Program entry point and main execution loop
- **UI Layer** (`high_level_funcs`): Input handling, display management, error presentation
- **Calculation Engine** (`calculate_answer`): Expression parsing, syntax validation, mathematical evaluation
- **Hardware Drivers** (`low_level_funcs_tiva`): TivaWare-specific hardware interfaces

## Hardware Requirements

- **Microcontroller**: TI Tiva C Series (TM4C123x or compatible)
- **Display**: 16x2 character LCD
- **Input**: 4x4 matrix keypad
- **Development Environment**: Code Composer Studio or compatible ARM toolchain

## Keypad Layout

```
[1] [2] [3] [A]  →  [1] [2] [3] [+ or x*]
[4] [5] [6] [B]  →  [4] [5] [6] [- or /*] 
[7] [8] [9] [C]  →  [7] [8] [9] [. or E*]
[*] [0] [#] [D]  →  [=] [0] [CLR] [SHIFT]
```
*\* When SHIFT (D) is pressed first*

## Usage

1. **Power On**: Calculator displays previous result from flash memory
2. **Input Expression**: Use keypad to enter mathematical expressions
3. **Operations**: 
   - Press `A` for addition (+) or `SHIFT+A` for multiplication (x)
   - Press `B` for subtraction (-) or `SHIFT+B` for division (/)
   - Press `C` for decimal point (.) or `SHIFT+C` for scientific notation (E)
4. **Execute**: Press `*` to calculate result
5. **Clear**: Press `#` to clear display or `SHIFT+#` for backspace
6. **Error Handling**: Invalid expressions display descriptive error messages

## Build Instructions

### Prerequisites
- Code Composer Studio v12.0 or later
- TivaWare for C Series v2.2.0 or later
- ARM GCC toolchain

### Compilation
1. Clone the repository
2. Open project in Code Composer Studio
3. Configure TivaWare path in project settings
4. Build project (Ctrl+B)
5. Flash to target device

### Manual Build
```bash
# Compile all source files
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
  -I./inc -I./_tivaware/inc -I./_tivaware/driverlib \
  -c *.c

# Link executable
arm-none-eabi-gcc -T tm4c123gh6pm.lds -o calculator.elf *.o \
  -L./_tivaware/driverlib -ldriver

# Generate binary
arm-none-eabi-objcopy -O binary calculator.elf calculator.bin
```

## Error Codes

| Code | Error Message | Description |
|------|---------------|-------------|
| 0 | No error | Successful calculation |
| 1 | Unidentified error | General calculation error |
| 2 | SOFT BUG: Empty input string | Empty input detected |
| 3 | No null or too long I/P string | String validation failed |
| 4 | Invalid char in input string | Unsupported character found |
| 5 | Number with > 1 decimal point | Multiple decimal points in number |
| 6 | Invalid number | Number parsing failed |
| 7 | May not start with +,x,/ or E | Invalid expression start |
| 8 | May not end with operator | Invalid expression end |
| 9 | Two adjacent operators | Invalid operator sequence |
| 10 | Two adjacent E operators | Invalid scientific notation |
| 11 | E must be followed by integer | Invalid exponent format |

## Code Quality Features

- **Memory Safety**: Comprehensive bounds checking and buffer overflow protection
- **Error Handling**: Multi-stage validation with graceful error recovery
- **Documentation**: Extensive Doxygen-style comments throughout
- **Modularity**: Clean separation of concerns with well-defined interfaces
- **Portability**: Hardware abstraction enables easy platform migration


## Contributing

1. Follow existing code style and documentation standards
2. Add comprehensive unit tests for new features
3. Ensure all functions include proper Doxygen documentation
4. Test on target hardware before submitting changes

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Authors

- Original implementation: [Ahmad]

## Version History

- **v1.1**: Enhanced error handling, fixed critical bugs, improved documentation
- **v1.0**: Initial implementation with basic calculator functionality
