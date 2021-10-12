/**************************************************************************
* File Name         TimerCmd.c
* Description       
*				          
* Date				          Name(s)						          Action
* October 5, 2021		    Jaskaran K. & David C.			First Implementation
***************************************************************************/

/**************************************************************************
---------------------------- LIBRARY DEFINITIONS --------------------------
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_pwr.h"
#include "stm32f4xx_hal_rcc_ex.h"
#include "stm32f4xx_hal_pwr_ex.h"
#include "stm32f4xx_hal_cortex.h"

/**************************************************************************
--------------------------- PRECOMPILER DEFINITIONS -----------------------
***************************************************************************/
#define MICROSECONDS 0
#define MILISECONDS 1

/**************************************************************************
------------------------------- VARIABLE TYPES ----------------------------
***************************************************************************/
typedef struct{
  uint32_t timeout;     // virtual counter timeout
  uint32_t current;     // virtual counter current value
  uint32_t gpioPin;     // Pin to be toggled
  uint8_t flag;         // ready flag
  uint8_t enable;       // Current status of virtual timer
  uint32_t repetitive;  // count 1 single time or undefinetely
  uint8_t nodeID;
}V_TIMER;

// Linked list node structure
typedef struct node{
  V_TIMER *vTimerPtr;
  struct node *nextNode;
}V_NODE;

/**************************************************************************
---------------------------- GLOBAL VARIABLES --------------------------
***************************************************************************/
TIM_HandleTypeDef htim3;  // Timer Handler 
V_NODE *head = NULL;      // head of linked list
uint8_t countNodes = 0;

/**************************************************************************
------------------------ OWN FUNCTION DEFINITIONS -------------------------
***************************************************************************/

/*--------------------------------------------------------------------------
*	Name:			    TimerInit
*	Description:	
*	Parameters:		void
*
*	Returns:		  ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t TimerInit()
{ 
  uint16_t timebase = 0; //input in microseconds
  uint16_t tim3Prescaler = 1; // 0xFFFF 84
  uint16_t tim3Period = 1; //0xFFFF 65535

  if (fetch_uint16_arg(&timebase))
    printf("Please enter (1) for ms or (0) for us (default).\n");
  else{
    if(timebase == MILISECONDS){
      tim3Prescaler = 1000;
      tim3Period = HAL_RCC_GetPCLK2Freq() / 1000000 - 1;
    }
    else{
      tim3Prescaler = 1;
      tim3Period = HAL_RCC_GetPCLK2Freq() / 1000000 - 1;
    }
    
    // Set up the timer
    __HAL_RCC_TIM3_CLK_ENABLE();
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = tim3Prescaler;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = tim3Period;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
      printf("Error 1 initializing the timer\n");
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
      printf("Error 2 initializing the timer\n");
    }
    HAL_NVIC_SetPriority((IRQn_Type) TIM3_IRQn, (uint32_t) 0, (uint32_t) 1);
    HAL_NVIC_EnableIRQ((IRQn_Type) TIM3_IRQn);
    // Start timer
    HAL_TIM_Base_Start_IT(&htim3);

  }
  return CmdReturnOk;
}
// MACRO: Add new command to help menu
ADD_CMD("timerinit", TimerInit,"\t\tInitializes hardware timer.")

/*--------------------------------------------------------------------------
*	Name:			    TimerInit
*	Description:	
*	Parameters:		void
*
*	Returns:		  ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t TimerInstance(int action)
{ 
  // Help messages
  if(action==CMD_SHORT_HELP) return CmdReturnOk;
  if(action==CMD_LONG_HELP) {
    printf("timer <timeout> <out_pin> <recurrent>\n\n"
	   "This command Initialize a virtual timer instance.\n"
     "timeout: number of counts of the already initialized hardware timer"
     "timebase.\n"
     "out_pin: a digital pin can be toggled with virtual timer overflow.\n"
     "recurrent: if recurrent 0, virtual timer will be disabled after first" 
     "overflow event, if recurrent 1, timer will continue indefinitely.\n" 
	  );
    return CmdReturnOk;
  }
  // Get arguments from command line
  uint32_t arguments[3] = {0};
  for(int i=0; i<3; i++){
    if(fetch_uint32_arg(&arguments[i])){
      printf("Insuficient number of arguments. \n "
      "Type <help timer> to get more information.\n");
    }
  }
  
  /* Add new node */
  // Allocate memory for the node
	V_NODE *newTimerPtr = NULL;
	newTimerPtr = (V_NODE*)malloc(sizeof(V_NODE));
	if (newTimerPtr == NULL) {
		printf("Memory Allocation Error!\n");
	}
  /* Fill in new node */
  newTimerPtr->vTimerPtr->timeout = arguments[0];
  newTimerPtr->vTimerPtr->gpioPin = arguments[1]; 
  newTimerPtr->vTimerPtr->repetitive = arguments[2];
  newTimerPtr->vTimerPtr->flag = 0;
  newTimerPtr->vTimerPtr->current = 0;
  newTimerPtr->vTimerPtr->enable = 1;
  newTimerPtr->vTimerPtr->nodeID = countNodes;

  /* Position new node */
  // Empty linked list
  if(head == NULL){
    printf("All good:%d\n", sizeof(newTimerPtr));
    //newTimerPtr->nextNode = NULL;
    //head = newTimerPtr;
  }/*
  else{
    // Traverse list and put at the end
    V_NODE *currentTimerPtr = head;
    while(currentTimerPtr->nextNode != NULL){
      currentTimerPtr = currentTimerPtr->nextNode;
    }
    currentTimerPtr->nextNode = newTimerPtr;
    newTimerPtr->nextNode = NULL;
  }
  printf("Virtual Timer ID: %d\n", countNodes);
  countNodes++;
*/
  return CmdReturnOk;
}
// MACRO: Add new command to help menu
ADD_CMD("timer", TimerInstance,"\t\tInitialize a virtual timer instance.")

/*--------------------------------------------------------------------------
*	Name:			    TimerInit
*	Description:	
*	Parameters:		void
*
*	Returns:		  ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
void VirtualTimers()
{ 
  // Traverse all virtual timer instances
  V_NODE *currentPtr = head;

  while(currentPtr != NULL){

    if(currentPtr->vTimerPtr->enable){
      // Timer has reached its desired value
      if(currentPtr->vTimerPtr->current >= currentPtr->vTimerPtr->timeout){
        currentPtr->vTimerPtr->flag = 1;
        HAL_GPIO_TogglePin(GPIOA, (uint32_t) 1 << currentPtr->vTimerPtr->gpioPin);
        // Check if timer is repetitive
        if (currentPtr->vTimerPtr->repetitive){
          currentPtr->vTimerPtr->current = 0;
        }
        else{
          // Disable it if not repetitive
          currentPtr->vTimerPtr->enable = 0;
        }  
      }
      else{
        // Normal increment
        currentPtr->vTimerPtr->current++;
        currentPtr->vTimerPtr->flag = 0;
      }
    }
    // Get next node
    currentPtr = currentPtr->nextNode;
  }
}

/*--------------------------------------------------------------------------
*	Name:			    TimerInit
*	Description:	
*	Parameters:		void
*
*	Returns:		  ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
void TIM3_IRQHandler(void)
{
  // This will call for "HAL_TIM_PeriodElapsedCallback()" on timer update 
  HAL_TIM_IRQHandler(&htim3);
}

/*--------------------------------------------------------------------------
*	Name:			    TimerInit
*	Description:	
*	Parameters:		void
*
*	Returns:		  ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  //HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  //VirtualTimers();
}