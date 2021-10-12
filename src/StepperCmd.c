/**************************************************************************
* File Name         SimpleCommand.c
* Description       Print tests with and without string argument.
*				          
* Date				          Name(s)						          Action
* September 20, 2021		Jaskaran K. & David C.			First Implementation
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
#define OE_PIN GPIO_PIN_4
#define OE_PORT GPIOC

#define RST_PIN GPIO_PIN_3
#define RST_PORT GPIOC

#define PS_PIN GPIO_PIN_8
#define PS_PORT GPIOC

#define STEP_PIN GPIO_PIN_14
#define STEP_PORT GPIOB

#define FR_PIN GPIO_PIN_5
#define FR_PORT GPIOC

#define VREF_PIN GPIO_PIN_4
#define VREF_PORT GPIOA

#define MONI_PIN GPIO_PIN_9
#define MONI_PORT GPIOC



/**************************************************************************
------------------------ OWN FUNCTION DEFINITIONS -------------------------
***************************************************************************/

/*--------------------------------------------------------------------------
*	Name:			    
*	Description:	
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t StepperInitialize()
{
  
  GPIO_InitTypeDef My_GPIO_InitStructA = {0};  
  GPIO_InitTypeDef My_GPIO_InitStructB = {0};  
  GPIO_InitTypeDef My_GPIO_InitStructC = {0};  
 
  My_GPIO_InitStructA.Pin = (uint16_t)(GPIO_PIN_4);

  My_GPIO_InitStructA.Mode = GPIO_MODE_OUTPUT_PP;
  My_GPIO_InitStructA.Pull = GPIO_NOPULL;
  My_GPIO_InitStructA.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &My_GPIO_InitStructA);

  My_GPIO_InitStructB.Pin = (uint16_t)(GPIO_PIN_14);

  My_GPIO_InitStructB.Mode = GPIO_MODE_OUTPUT_PP;
  My_GPIO_InitStructB.Pull = GPIO_NOPULL;
  My_GPIO_InitStructB.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &My_GPIO_InitStructB);

  My_GPIO_InitStructC.Pin = (uint16_t)(GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9);

  My_GPIO_InitStructC.Mode = GPIO_MODE_OUTPUT_PP;
  My_GPIO_InitStructC.Pull = GPIO_NOPULL;
  My_GPIO_InitStructC.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &My_GPIO_InitStructC);

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("stepperinit", StepperInitialize,"\t\tInitialize GPIO pins to control the stepper motor.")


/*--------------------------------------------------------------------------
*	Name:			    
*	Description:	
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t StepperEnable()
{
  uint16_t argument = 0;
  if (!fetch_uint16_arg(&argument)){
    if((argument == 1) || (argument == 0)){
      // Low = enable, High = disable (Output enable)
      HAL_GPIO_WritePin(OE_PORT, OE_PIN, !argument);
      // Low = Standby, High = Operating (Power save)
      HAL_GPIO_WritePin(PS_PORT, PS_PIN, argument);
      // Step pin inititates in LOW (Step signal)
      HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_RESET);
      // High = Normal, Low = Reset. (Reset)
      HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_SET);
      // MD1=1, MD2=1 = 2W1-2 phase excitation (Excitation mode)
      // HARD WIRED -> 3.3V
      // ATT1=0, ATT2=0 = 100% current (Current level control)
      // HARD WIRED -> 0V
      // High = Max current, Low = Min current. (VREF)
      HAL_GPIO_WritePin(VREF_PORT, VREF_PIN, GPIO_PIN_SET);
      // High = CCW, Low = CW. (Motor direction)
      HAL_GPIO_WritePin(FR_PORT, FR_PIN, GPIO_PIN_SET);
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
*	Name:			    
*	Description:	 
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