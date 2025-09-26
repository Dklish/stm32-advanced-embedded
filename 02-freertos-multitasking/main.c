/*freeRTOS with STMF401RE
-Familarizing with freeRTOS protocol using STM32CUBEIDE 
-Just want to showcase basic understanding of semaphores, priorities, and hook functions 
Author: Diego KLish 
*/

//libraries I need to use 
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

//first well define the priority levels of our three functions 
//this is one of if not the most important concept when using FreeRTOS

#define Highest_priority osPriorityRealtime //our highest priority task
#define LED_priority osPriorityHigh //second highest 
#define Monitor_priority osPriorityLow //lowest 

/*we also then need to define our stack sizes for each
when using FreeRTOS we have to define specific memory for each */

#define Highest_stack 256 * 4 //larger for a greater safety margin given its our highest priority 
#define LED_stack     128 * 4 //standard for monitoring 
#define Monitor_stack 128 * 4 

//define our tasks handles now that weve defined priority and stack size 
osThreadId_t HighestHandle;
const osThreadAttr_t Highest_attributes = {
  .name = "HighestPriority",
  .stack_size = Highest_stack,
  .priority = Highest_priority,
};

osThreadId_t LEDHandle;
const osThreadAttr_t LED_attributes = {
  .name = "LEDTask", 
  .stack_size = LED_stack,
  .priority = LED_priority,
};

osThreadId_t MonitorHandle;
const osThreadAttr_t Monitor_attributes = {
  .name = "MonitorTask",
  .stack_size = Monitor_stack,
  .priority = Monitor_priority,
};

/*now well define our sempaphores both are bianary for this example code 
Protects LED resource from concurrent access*/
osSemaphoreId_t LEDSemaphoreHandle;
const osSemaphoreAttr_t LEDSemaphore_attributes = {
  .name = "LEDSemaphore"
};

// Binary Semaphore - Protects UART output for printf 
osSemaphoreId_t UARTSemaphoreHandle;
const osSemaphoreAttr_t UARTSemaphore_attributes = {
  .name = "UARTSemaphore"
};

//next well define our volatile variables able to be accessed by multiple tasks
//
volatile uint32_t g_system_errors = 0;
volatile bool g_highest_active = false;
volatile uint32_t g_button_presses = 0;

// System health 
volatile uint32_t g_cpu_usage_percent = 0;
volatile uint32_t g_free_heap_bytes = 0;

//now that all of this is setup we can create our tasks 

void HighestTask(void *argument)
{
  uint32_t last_button_state = GPIO_PIN_SET;
  uint32_t current_button_state;
  
  // Protected UART output using semaphore
  osSemaphoreAcquire(UARTSemaphoreHandle, osWaitForever);
  printf("Task started - Highest Priority\n");
  printf("Monitoring for critical events");
  osSemaphoreRelease(UARTSemaphoreHandle);
  
 for(;;)
  {
    // Monitor button as highest trigger (simulating fault detection)
    current_button_state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
    
    // Detect highest condition (button press)
    if(last_button_state == GPIO_PIN_SET && current_button_state == GPIO_PIN_RESET) {
      
      //  This task preempts ALL others immediately
      g_highest_active = true;
      g_system_errors++;
      
      osSemaphoreAcquire(UARTSemaphoreHandle, osWaitForever);
      printf("EMERGENCY: System entering safe mode\n");
      printf("EMERGENCY: Total system errors: %lu\n", g_system_errors);
      osSemaphoreRelease(UARTSemaphoreHandle);
      
      // Emergency LED pattern - Take LED semaphore with timeout
      if(osSemaphoreAcquire(LEDSemaphoreHandle, 100) == osOK) {
        // Rapid emergency blink pattern
        for(int i = 0; i < 10; i++) {
          HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
          osDelay(50);
          HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
          osDelay(50);
        }
        osSemaphoreRelease(LEDSemaphoreHandle);
      }
      
      // Wait for acknowledgment (button release)
      while(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
        osDelay(50);
      }
      
      // Clear emergency after acknowledgment
      osDelay(500); // Additional delay to prevent bounce
      g_highest_active = false;
      
      osSemaphoreAcquire(UARTSemaphoreHandle, osWaitForever);
      printf("Highest cleared - resuming normal operation\n\n");
      osSemaphoreRelease(UARTSemaphoreHandle);
    }
    
    last_button_state = current_button_state;
    
    // Emergency task checks every 10ms for fast response
    osDelay(10);
  }
}

void LEDTask(void *argument)
{
  uint32_t blink_rate = 1000; // Normal blink rate
  TickType_t last_wake_time = osKernelGetTickCount();
  
  osSemaphoreAcquire(UARTSemaphoreHandle, osWaitForever);
  printf("LED: Task started - High Priority\n");
  osSemaphoreRelease(UARTSemaphoreHandle);
  
  for(;;)
  {
    // Skip LED operation during highest 
    if(!g_Highest_active) {
      
      // Must acquire semaphore before accessing LED
      if(osSemaphoreAcquire(LEDSemaphoreHandle, 1000) == osOK) {
        
        // Adjust blink rate based on system state
        if(g_system_errors > 0) {
          blink_rate = 250; // Faster blink indicates previous errors
        } else {
          blink_rate = 1000; // Normal operation
        }
        
        // Toggle LED - Protected by semaphore
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        
        osSemaphoreAcquire(UARTSemaphoreHandle, osWaitForever);
        printf("LED: Toggled (Rate: %lu ms, Errors: %lu)\n", 
               blink_rate, g_system_errors);
        osSemaphoreRelease(UARTSemaphoreHandle);
        
        // CRITICAL: Always release semaphore
        osSemaphoreRelease(LEDSemaphoreHandle);
        
      } else {
        // Semaphore timeout - LED resource busy (probably emergency task)
        osSemaphoreAcquire(UARTSemaphoreHandle, osWaitForever);
        printf("LED: Resource busy - semaphore timeout\n");
        osSemaphoreRelease(UARTSemaphoreHandle);
      }
      
      // Use precise timing
      osDelayUntil(&last_wake_time, blink_rate);
      
    } else {
      // During emergency, yield CPU time
      osDelay(100);
    }
  }
}

void MonitorTask(void *argument)
{
  uint32_t loop_count = 0;
  uint32_t last_heap_size = 0;
  
  osSemaphoreAcquire(UARTSemaphoreHandle, osWaitForever);
  printf("MONITOR: Task started - Low Priority");
  osSemaphoreRelease(UARTSemaphoreHandle);
  
  for(;;)
  {
    loop_count++;
    
    // System health monitoring - runs when CPU is available
    g_free_heap_bytes = xPortGetFreeHeapSize();
    
    // Report system status every 50 loops (gets interrupted frequently)
    if(loop_count % 50 == 0) {
      
      osSemaphoreAcquire(UARTSemaphoreHandle, osWaitForever);
      printf("MONITOR: === System Status Report ===\n");
      printf("MONITOR: Loop count: %lu\n", loop_count);
      printf("MONITOR: Free heap: %lu bytes\n", g_free_heap_bytes);
      printf("MONITOR: System errors: %lu\n", g_system_errors);
      printf("MONITOR: Emergency active: %s\n", 
             g_Highest_active ? "YES" : "NO");
      
      // Detect memory leaks
      if(last_heap_size > 0 && g_free_heap_bytes < last_heap_size) {
        printf("MONITOR: WARNING - Possible memory leak detected!\n");
      }
      osSemaphoreRelease(UARTSemaphoreHandle);
      
      last_heap_size = g_free_heap_bytes;
    }
    //long delay allows preemption
    osDelay(200);
  }
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  __disable_irq();
  
  // Turn on LED to indicate fatal error
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  
  // Infinite loop - system must be reset
  while(1) {
    /* Optional: Toggle LED to show system is still responsive
     but cannot continue normal operation */
    HAL_Delay(100);
  }
}

void vApplicationMallocFailedHook(void)
{
  // CRITICAL ERROR - Out of heap memory 
  
  printf("HOOK: Heap allocation failed!\n");
  printf("HOOK: Free heap: %lu bytes\n", xPortGetFreeHeapSize());
  printf("HOOK: System entering minimal operation mode\n");
  
  // Enter minimal operation mode
  g_Highest_active = true;
  g_system_errors++;
}


void vApplicationIdleHook(void)
{
  // Simple CPU utilization tracking 
  static uint32_t idle_counter = 0;
  idle_counter++;
  
  // Every 10000 idle cycles, estimate CPU usage
  if(idle_counter >= 10000) {
    g_cpu_usage_percent = 100 - (idle_counter / 200);
    if(g_cpu_usage_percent > 100) g_cpu_usage_percent = 100;
    
    idle_counter = 0;
  }
  

}

int main(void)
{
  // Standard STM32 initialization 
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  
  // Initialize FreeRTOS kernel 
  osKernelInitialize();
  
  // Create semaphores BEFORE tasks that use them 
  LEDSemaphoreHandle = osSemaphoreNew(1, 1, &LEDSemaphore_attributes);
  UARTSemaphoreHandle = osSemaphoreNew(1, 1, &UARTSemaphore_attributes);
  
  if(LEDSemaphoreHandle == NULL || UARTSemaphoreHandle == NULL) {
    printf("ERROR: Failed to create semaphores!\n");
    Error_Handler();
  }
  
  // Create tasks in order of priority 
  HighestHandle = osThreadNew(HighestTask, NULL, &HighestTask_attributes);
  LEDTaskHandle = osThreadNew(LEDTask, NULL, &LEDTask_attributes);  
  MonitorTaskHandle = osThreadNew(MonitorTask, NULL, &MonitorTask_attributes);
  
  if(HighestHandle == NULL || LEDTaskHandle == NULL || MonitorTaskHandle == NULL) {
    printf("ERROR: Failed to create tasks!\n");
    Error_Handler();
  }
  
  // Start the scheduler - control transfers to FreeRTOS 
  osKernelStart();
  
  // Should never reach here 
  printf("FATAL: Scheduler failed to start!\n");
  Error_Handler();
}