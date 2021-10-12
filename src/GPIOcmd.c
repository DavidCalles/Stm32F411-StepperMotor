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

#define MAX_ARGUMENTS 32

/**************************************************************************
------------------------ OWN FUNCTION DEFINITIONS -------------------------
***************************************************************************/

/*--------------------------------------------------------------------------
*	Name:			    GPIOInitializeOutput
*	Description:	Initializes the GPIO pins entered through the terminal as 
*               outputs
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t GPIOInitializeOutput()
{
  uint16_t currentPin;      //stores the pin number to be initialized

  GPIO_InitTypeDef My_GPIO_InitStruct = {0};  
  //__HAL_RCC_GPIOA_CLK_ENABLE(); gpio 4 6 7 0 0 1

  while(!fetch_uint16_arg(&currentPin)){  //loops through the values entered
    if(currentPin > 15){                  //then ask user to enter valid values
      printf("Pin values should be between 0 and 15 :)\n");
      return (!CmdReturnOk);
    }
    else{
      My_GPIO_InitStruct.Pin |= (uint16_t)(1 << currentPin);
    }   
  }
  My_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  My_GPIO_InitStruct.Pull = GPIO_NOPULL;
  My_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &My_GPIO_InitStruct);

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("gpioinitout", GPIOInitializeOutput,"\t\tInitialize given GPIO pins from GPIOA as Outputs")


/*--------------------------------------------------------------------------
*	Name:			    GPIOInitializeInput
*	Description:	Initializes the GPIO pins entered through the terminal as 
*               inputs
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t GPIOInitializeInput()
{
  uint16_t currentPin;      //stores the pin number to be initialized

  GPIO_InitTypeDef My_GPIO_InitStruct = {0};  
  //__HAL_RCC_GPIOA_CLK_ENABLE(); gpio 4 6 7 0 0 1

  while(!fetch_uint16_arg(&currentPin)){  //loops through the values entered
    if(currentPin > 15){                  //then ask user to enter valid values
      printf("Pin values should be between 0 and 15 :)\n");
      return (!CmdReturnOk);
    }
    else{
      My_GPIO_InitStruct.Pin |= (uint16_t)(1 << currentPin);
    }   
  }
  My_GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  My_GPIO_InitStruct.Pull = GPIO_NOPULL;
  My_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &My_GPIO_InitStruct);

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("gpioinitin", GPIOInitializeInput,"\t\tInitialize given GPIO pins from GPIOA as Inputs")



/*--------------------------------------------------------------------------
*	Name:			    GPIOWrite
*	Description:	Sets the GPIO pins with the values entered through terminal 
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t GPIOWrite()
{
  uint16_t arguments[MAX_ARGUMENTS];  //stores the arguesments entered at terminal

  //__HAL_RCC_GPIOA_CLK_ENABLE(); gpio 4 6 7 0 0 1
  int8_t i = 0;   //initialize the position of arguments to 0
  while(!fetch_uint16_arg(&arguments[i])){  //loops through the number of values entered
    if(arguments[i] > 15){    //then ask user to enter valid values
      printf("Pin values should be between 0 and 15 :)\n");
      return (!CmdReturnOk);
    }
    i++;  //increments to next value
  }
  if((i%2) != 0){   //if number of values entered are odd
    //prints message for missing arguments
    printf("You might be missing an argument :)\n"); 
    return (!CmdReturnOk);
  }

  uint8_t offset = i/2; 
  for(int8_t j = 0; j< offset; j++){  
    //sets the values entered to the GPIO pins entered 
    HAL_GPIO_WritePin(GPIOA, (uint16_t)(1 << arguments[j]), arguments[offset+j]); 
  }

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("gpiowrite",GPIOWrite,"\t\tWrite given GPIO pins from GPIOA")

/*--------------------------------------------------------------------------
*	Name:			    GPIORead
*	Description:	Reads the GPIO pins entered through terminal 
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t GPIORead()
{
  uint16_t arguments[MAX_ARGUMENTS];  //stores the arguesments entered at terminal
  int results[MAX_ARGUMENTS];
  //__HAL_RCC_GPIOA_CLK_ENABLE(); gpio 4 6 7 0 0 1
  int8_t i = 0;   //initialize the position of arguments to 0
  while(!fetch_uint16_arg(&arguments[i])){  //loops through the number of values entered
    if(arguments[i] > 15){    //then ask user to enter valid values
      printf("Pin values should be between 0 and 15 :)\n");
      return (!CmdReturnOk);
    }
    i++;  //increments to next value
  }

  for(int8_t j = 0; j< i; j++){  
    //sets the values entered to the GPIO pins entered 
    results[i] = HAL_GPIO_ReadPin(GPIOA, (uint16_t)(1 << arguments[j]));
    printf("%d ",results[i]);
  }
  printf("\n");

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("gpioread",GPIORead,"\t\tRead given GPIO pins from GPIOA")