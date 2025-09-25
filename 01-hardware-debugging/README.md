# Hardware Debugging with JTAG/SWD

## Project Overview

This project demonstrates hardware debugging techniques using JTAG/SWD on the STM32F401RE Nucleo board. It covers essential debugging skills needed for embedded systems development in automotive and defense industries.

## Hardware Requirements

- STM32F401RE Nucleo Board
- USB Cable (for ST-Link debugger connection)
- Built-in ST-Link V2-1 debugger

## Debugging Techniques Demonstrated

### 1. Breakpoint Management
- Setting breakpoints in main loop
- Conditional breakpoints
- Breakpoints in interrupt handlers
- Function breakpoints

### 2. Variable Inspection
- Watch windows for global variables
- Real-time variable monitoring
- Memory view for arrays and structures
- Live variable modification during debugging

### 3. Code Stepping
- Step over function calls
- Step into functions
- Step out of functions
- Continue execution to next breakpoint

### 4. Register and Peripheral Debugging
- GPIO register inspection
- RCC (Reset and Clock Control) registers
- Interrupt controller (NVIC) registers
- Real-time peripheral state monitoring

### 5. Interrupt Debugging
- Breakpoints in interrupt service routines
- Interrupt priority and nesting

## Program Features

The test program includes:
- **LED Control**: PA5 LED toggles every second
- **Button Interrupt**: PC13 button triggers interrupt handler

## Debugging Session Exercises

### Exercise 1: Basic Breakpoints
1. Set breakpoint at line 35 (start of main loop)
2. Run program and observe execution stopping
3. Examine `debug_counter` and `loop_iterations` variables
4. Continue execution and observe variable changes

### Exercise 2: Variable Watching
1. Add `debug_counter`, `led_state`, and `button_presses` to watch window
2. Set breakpoint in LED toggle logic
3. Step through code and watch variables change in real-time
4. Modify variable values during execution

### Exercise 3: Function Stepping
1. Set breakpoint at `delay_ms()` function call
2. Step into function to examine inner workings
3. Set breakpoint inside delay loop
4. Step out of function and continue

### Exercise 4: Interrupt Debugging
1. Set breakpoint in `EXTI15_10_IRQHandler()`
2. Press user button to trigger interrupt
3. Examine stack frame and register states
4. Step through interrupt handler execution

### Exercise 5: Register Inspection
1. Set breakpoint at GPIO configuration
2. Examine GPIOA registers before and after configuration
3. Watch GPIOA->ODR register during LED toggle
4. Inspect RCC registers during clock configuration

## Development Environment Setup

### STM32CubeIDE
1. Create new STM32 project for STM32F401RE
2. Replace generated main.c with provided code
3. Configure debugger for ST-Link
4. Start debug session

### Command Line with GDB
```bash
# Connect to ST-Link via OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

# In separate terminal, connect GDB
arm-none-eabi-gdb main.elf
(gdb) target remote localhost:3333
(gdb) load
(gdb) break main
(gdb) continue
```

## Key Learning Outcomes
After completing this project, I now understand:
- How to set up JTAG/SWD debugging connections
- Essential GDB commands for embedded debugging
- Real-time variable and register inspection
- Interrupt debugging techniques