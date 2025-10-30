// make a function that prints a line whenever called
#include "main.h"
#include "usart.h"  
#include <stdio.h>
#include "cmsis_os.h"
#include "gpio.h"
#include "fdcan.h"

/* Private includes ----------------------------------------------------------*/


// C++ functions that are called by C code need to have C linkage as the following:
extern "C" void myPrintLine()
{
    // HAL_UART_Transmit(&huart1, (uint8_t*)"Hello Chemmy from C++\r\n", 25, 100); 
    printf("Hello Chemmy from C++ using printf()\r\n");
}


extern "C" void myMainCPPInit()
{
    HAL_UART_Transmit(&huart1, (uint8_t*)"myMainCPPInit():\r\n", 25, 100); 
    // You can add more complex C++ code here
    
}

