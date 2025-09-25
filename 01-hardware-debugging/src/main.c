/* Example code used to practice JTAG/SWD
-Used this generated code to practice stepping through a fucntion using JTAG/SWD
-Breakpoint setting and execution control as well as LED control 
*/
#include "stm32f4xx.h"

// Global variables for debugging observation
volatile uint32_t debug_counter = 0;
volatile uint8_t led_state = 0;
volatile uint32_t loop_iterations = 0;
volatile uint16_t button_presses = 0;

// Function prototypes
void SystemClock_Config(void);
void GPIO_Init(void);
void delay_ms(uint32_t ms);
void EXTI15_10_IRQHandler(void);

/**
 * @brief Main program entry point
 */
int main(void) {
    // System initialization
    SystemClock_Config();
    GPIO_Init();

    // Main debugging loop
    while(1) {
        loop_iterations++;
        debug_counter++;

        // LED toggle logic - good for breakpoint practice
        if(debug_counter % 2 == 0) {
            // Turn LED ON
            GPIOA->ODR |= GPIO_ODR_OD5;
            led_state = 1;
        } else {
            // Turn LED OFF
            GPIOA->ODR &= ~GPIO_ODR_OD5;
            led_state = 0;
        }

        // Delay with nested function calls for step debugging
        delay_ms(1000);

        // Reset counter periodically for observation
        if(debug_counter >= 10) {
            debug_counter = 0;
        }
    }
}

/**
 * @brief Configure system clock (basic setup)
 */
void SystemClock_Config(void) {
    // Enable HSI (16MHz internal oscillator)
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));

    // Use HSI as system clock
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_HSI;
}

/**
 * @brief Initialize GPIO for LED and button
 */
void GPIO_Init(void) {
    // Enable GPIOA and GPIOC clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

    // Configure PA5 (LED) as output
    GPIOA->MODER &= ~GPIO_MODER_MODE5;
    GPIOA->MODER |= GPIO_MODER_MODE5_0;
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT5;
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5;

    // Configure PC13 (User button) as input with interrupt
    GPIOC->MODER &= ~GPIO_MODER_MODE13;
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD13;
    GPIOC->PUPDR |= GPIO_PUPDR_PUPD13_0; // Pull-up

    // Configure EXTI for button interrupt
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13;
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;

    EXTI->IMR |= EXTI_IMR_MR13;
    EXTI->FTSR |= EXTI_FTSR_TR13; // Falling edge trigger

    // Enable EXTI15_10 interrupt in NVIC
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, 0);
}

/**
 * @brief Simple delay function - good for step debugging
 * @param ms Delay in milliseconds (approximate)
 */
void delay_ms(uint32_t ms) {
    volatile uint32_t delay_counter = 0;

    for(uint32_t i = 0; i < ms; i++) {
        // Inner loop for timing (approximate)
        for(delay_counter = 0; delay_counter < 4000; delay_counter++) {
            __NOP(); // No operation - good breakpoint location
        }
    }
}

/**
 * @brief Button interrupt handler - demonstrates interrupt debugging
 */
void EXTI15_10_IRQHandler(void) {
    if(EXTI->PR & EXTI_PR_PR13) {
        // Clear interrupt flag
        EXTI->PR |= EXTI_PR_PR13;

        // Increment button counter
        button_presses++;

        // Toggle LED immediately on button press
        if(GPIOA->ODR & GPIO_ODR_OD5) {
            GPIOA->ODR &= ~GPIO_ODR_OD5;
        } else {
            GPIOA->ODR |= GPIO_ODR_OD5;
        }
    }
}