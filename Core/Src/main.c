/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h> //You are permitted to use this library, but currently only printf is implemented. Anything else is up to you!

#include "common.h"
#include "k_mem.h"
#include "k_task.h"

int i_test = 0;

int i_test2 = 0;

void TaskA(void *) {
    while(1){
      printf("%d, %d\r\n", i_test, i_test2);
      osPeriodYield();
    }
}

void TaskB(void *) {
    while(1){
      i_test = i_test + 1;
      osPeriodYield();
    }
}

void TaskC(void *) {
    while(1){
      i_test2 = i_test2 + 1;
      osPeriodYield();
    }
}
//#define SVC(code) asm volatile ("SVC %[immediate]"::[immediate] "I" (code)) //for svc call
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* MCU Configuration: Don't change this or the whole chip won't work!*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* MCU Configuration is now complete. Start writing your code below this line */

  printf("\r\n\n\n\n\n\n");

//  extern uint32_t _img_end;
//  printf(“End of Image: %x\r\n”,&_img_end);

  osKernelInit();

  k_mem_init();


  TCB st_mytask;
  st_mytask.stack_size = STACK_SIZE;
  st_mytask.ptask = &TaskA;
  osCreateDeadlineTask(4, STACK_SIZE, &st_mytask);

  st_mytask.ptask = &TaskB;
  osCreateDeadlineTask(4, STACK_SIZE, &st_mytask);

  st_mytask.ptask = &TaskC;
  osCreateDeadlineTask(12, STACK_SIZE, &st_mytask);

//test should print out two different addresses 
  osKernelStart();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  // printf("Hello, world!\r\n");
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
