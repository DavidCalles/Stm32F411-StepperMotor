/**************************************************************************
* File Name         StepperCmd.c
* Description       Initializes stepper motor and makes it step by requested
*                   number of steps and providing delay using interrupts.
*				          
* Date				          Name(s)						          Action
* October 12, 2021		Jaskaran K. & David C.			First Implementation
***************************************************************************/

/**************************************************************************
---------------------------- LIBRARY DEFINITIONS --------------------------
***************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"
#include "stm32f4xx_hal_gpio.h"

/**************************************************************************
-------------------------- PRE-COMPILER DEFINITIONS -----------------------
***************************************************************************/
/* Stepper Motor Pins */
#define OE_PIN (GPIO_PIN_4)
#define OE_PORT (GPIOC)

#define RST_PIN (GPIO_PIN_3)
#define RST_PORT (GPIOC)

#define PS_PIN (GPIO_PIN_8)
#define PS_PORT (GPIOC)

#define STEP_PIN (GPIO_PIN_14)
#define STEP_PORT (GPIOB)

#define FR_PIN (GPIO_PIN_5)
#define FR_PORT (GPIOC)

#define VREF_PIN (GPIO_PIN_4)
#define VREF_PORT (GPIOA)

#define MONI_PIN (GPIO_PIN_9)
#define MONI_PORT (GPIOC)

uint16_t pins[] = {OE_PIN, RST_PIN, PS_PIN, STEP_PIN, FR_PIN, VREF_PIN};
GPIO_TypeDef* ports[] = {OE_PORT, RST_PORT, PS_PORT, STEP_PORT, FR_PORT, VREF_PORT};

#define MAX_TIMERS 16         // maximum number of timers

/**************************************************************************
------------------------------- VARIABLE TYPES ----------------------------
***************************************************************************/
typedef struct{
  uint32_t timeout;     // virtual counter timeout
  uint32_t current;     // virtual counter current value
  uint8_t flag;         // ready flag
  uint8_t enable;       // Current status of virtual timer
  uint32_t remainingSteps; // missing toggles to perform
}V_TIMER;

/**************************************************************************
---------------------------- GLOBAL VARIABLES --------------------------
***************************************************************************/
TIM_HandleTypeDef htim3;              // Timer Handler 
volatile V_TIMER timers[MAX_TIMERS];  // Timer instances
uint8_t tIndex = 0;                   // index of current timer

/**************************************************************************
------------------------ OWN FUNCTION DEFINITIONS -------------------------
***************************************************************************/

/*--------------------------------------------------------------------------
*	Name:			    StepperInitialize
*	Description:	Initializes the stepper motor and timers
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t StepperInitialize()
{
  
  GPIO_InitTypeDef My_GPIO_InitStructA = {0};
  GPIO_InitTypeDef My_GPIO_InitStructB = {0};
  GPIO_InitTypeDef My_GPIO_InitStructC = {0};  

  for(int i = 0; i< 6; i++){
    if(ports[i] == GPIOA){
      My_GPIO_InitStructA.Pin |= (pins[i]);
    }
    if(ports[i] == GPIOB){
      My_GPIO_InitStructB.Pin |= (pins[i]);
    }
    if(ports[i] == GPIOC){
      My_GPIO_InitStructC.Pin |= (pins[i]);
    }

  }

  if(My_GPIO_InitStructA.Pin != 0){
    My_GPIO_InitStructA.Mode = GPIO_MODE_OUTPUT_PP;
    My_GPIO_InitStructA.Pull = GPIO_NOPULL;
    My_GPIO_InitStructA.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &My_GPIO_InitStructA);
  }
  

  if(My_GPIO_InitStructB.Pin != 0){
    My_GPIO_InitStructB.Mode = GPIO_MODE_OUTPUT_PP;
    My_GPIO_InitStructB.Pull = GPIO_NOPULL;
    My_GPIO_InitStructB.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &My_GPIO_InitStructB);
  }

  if(My_GPIO_InitStructC.Pin != 0){
    My_GPIO_InitStructC.Mode = GPIO_MODE_OUTPUT_PP;
    My_GPIO_InitStructC.Pull = GPIO_NOPULL;
    My_GPIO_InitStructC.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &My_GPIO_InitStructC);
  }

  // Set up interrupt with timer3
  __HAL_RCC_TIM3_CLK_ENABLE();
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1000;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = HAL_RCC_GetPCLK2Freq() / 1000000 - 1;
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

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("stepperinit", StepperInitialize,"\t\tInitialize GPIO pins to control the stepper motor.")


/*--------------------------------------------------------------------------
*	Name:			    StepperEnable
*	Description:	Enables or Disables stepper motor based on argument passed
*	Parameters:		void
*
*	Returns:		  ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t StepperEnable()
{
  uint16_t argument = 0;
  if (!fetch_uint16_arg(&argument)){
    if((argument == 1) || (argument == 0)){
      // Step pin inititates in LOW (Step signal)
      HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_SET);
      HAL_Delay(5);
      HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_RESET);
      // Low = enable, High = disable (Output enable)
      HAL_GPIO_WritePin(OE_PORT, OE_PIN, !argument);
      // Low = Standby, High = Operating (Power save)
      HAL_GPIO_WritePin(PS_PORT, PS_PIN, argument);
      // High = Normal, Low = Reset. (Reset)
      HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_SET);
      // MD1=1, MD2=1 = 2W1-2 phase excitation (Excitation mode)
      // HARD WIRED -> 3.3V
      // ATT1=0, ATT2=0 = 100% current (Current level control)
      // HARD WIRED -> 0V
      // High = Max current, Low = Min current. (VREF)
      HAL_GPIO_WritePin(VREF_PORT, VREF_PIN, GPIO_PIN_SET);
      // High = CCW, Low = CW. (Motor direction)
      HAL_GPIO_WritePin(FR_PORT, FR_PIN, GPIO_PIN_RESET);
    }
    else{
      printf("Invalid argument(s)\n"
        "Please enter (1) to enable the motor or (0) to disable it.\n");
      return CmdReturnOk;
    }  
  }
  else{
    printf("No argument received.\n"
        "Please enter (1) to enable the motor or (0) to disable it.\n");
    return CmdReturnOk;
  }

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("stepperenable", StepperEnable,"\t\tEnables or disables the stepper motor controller")

/*--------------------------------------------------------------------------
*	Name:			    Step
*	Description:	Makes stepper step for given delay and number of steps
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t Step()
{ 
  uint32_t numSteps = 10;
  uint32_t delayms = 50;
  if (!fetch_uint32_arg(&numSteps)){
    if (!fetch_uint32_arg(&delayms)){
      
      // Both arguments received
      for(int32_t j = 0; j< numSteps*2; j++){  
        //sets the values entered to the GPIO pins entered 
        HAL_GPIO_TogglePin(STEP_PORT, STEP_PIN); 
        WDTFeed();
        HAL_Delay(delayms);
      }
    }
    else{
      printf("Not enough arguments were detected.");
      return CmdReturnOk;
    }
  }
  else{
    printf("No arguments were detected.");
    return CmdReturnOk;
  }

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("step", Step, "\t\tOutput a given number of steps")

/*--------------------------------------------------------------------------
*	Name:			    Step2
*	Description:	Makes stepper step for given delay and number of steps
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t Step2(int action)
{ 

  // Help messages
  if(action==CMD_SHORT_HELP) return CmdReturnOk;
  if(action==CMD_LONG_HELP) {
    printf("step <step_count> <delay_ms>\n\n"
	   "This command Initialize a virtual timer instance.\n"
     "step_count: number of steps.\n"
     "delay_ms: delay in miliseconds.\n"
	  );
    return CmdReturnOk;
  }
  // Get arguments from command line
  int32_t arguments[2] = {0};
  for(int i=0; i<2; i++){
    if(fetch_int32_arg(&arguments[i])){
      printf("Insuficient number of arguments. \n "
      "Type <help step> to get more information.\n");
    }
  }
  
  // Set direction according to number of steps
  if (arguments[0] > 0)
    HAL_GPIO_WritePin(FR_PORT, FR_PIN, GPIO_PIN_RESET);
  else
    HAL_GPIO_WritePin(FR_PORT, FR_PIN, GPIO_PIN_SET);

  /* Fill in new timer */
  timers[tIndex].timeout = (uint32_t)arguments[1];
  timers[tIndex].remainingSteps = 2 * (uint32_t)arguments[0];
  timers[tIndex].flag = 0;
  timers[tIndex].current = 0;
  timers[tIndex].enable = 1;

  /* Position of next node */
  tIndex++;

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("step2", Step2, "\t\tOutput a given number of steps")

/*--------------------------------------------------------------------------
*	Name:			    VirtualTimers
*	Description:	interrupt routine to update all virtual timers as well as 
*               their corresponding GPIO pins
*	Parameters:		void
*
*	Returns:		  void
---------------------------------------------------------------------------*/
void VirtualTimers()
{ 
  // Traverse all virtual timer instances

  for(uint8_t i=0; i<tIndex; i++){

    if(timers[i].enable)
    {
      // Timer has reached its desired value
      if(timers[i].current >= timers[i].timeout)
      {
        timers[i].flag = 1;
        HAL_GPIO_TogglePin(STEP_PORT, STEP_PIN); 
        // Check if timer is repetitive
        if (timers[i].remainingSteps > 0)
        {
          timers[i].current = 0;
          timers[i].remainingSteps--;
        }
        else
        {
          // Disable it if not repetitive
          timers[i].enable = 0;
        }  
      }
      else
      {
        // Normal increment
        timers[i].current++;
        timers[i].flag = 0;
      }
    }
  }
}

/*--------------------------------------------------------------------------
*	Name:			    TIM3_IRQHandler
*	Description:  Timer 3 Interrupt handler
*	Parameters:		void
*
*	Returns:		  void
---------------------------------------------------------------------------*/
void TIM3_IRQHandler(void)
{
  // This will call for "HAL_TIM_PeriodElapsedCallback()" on timer update 
  HAL_TIM_IRQHandler(&htim3);
}

/*--------------------------------------------------------------------------
*	Name:			    HAL_TIM_PeriodElapsedCallback
*	Description:	Callbacks for timer overflow/update event
*	Parameters:		TIM_HandleTypeDef *htim : timer peripheral handler
*
*	Returns:		  void
---------------------------------------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  //HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  VirtualTimers();
}

/*--------------------------------------------------------------------------
*	Name:			    StepReset
*	Description:	Reset the queue of timers
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t StepReset(int action)
{ 
  
  tIndex = 0;
  HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_RESET);

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("stepreset", StepReset, "\t\tReset the queue of timers")