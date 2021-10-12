/**************************************************************************
* File Name         ADCcmd.c
* Description       ADC multi-channel initialization and graphical 
*                   representation of result.
*				          
* Date				          Name(s)						          Action
* September 28, 2021		Jaskaran K. & David C.			First Implementation
***************************************************************************/

/**************************************************************************
---------------------------- LIBRARY DEFINITIONS --------------------------
***************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"
#include "stm32f4xx_hal_gpio.h"

#define MAX_CHANNELS 19
#define VOLTAGE_REFERENCE 3.3 // No sufix means double
#define MAX_BARS 50

/**************************************************************************
---------------------------- GLOBAL VARIABLES --------------------------
***************************************************************************/

ADC_HandleTypeDef hadc1;

ADC_ChannelConfTypeDef sConfig = {0};

uint32_t adcChannel[MAX_CHANNELS] = {0};

uint8_t chCount = 0;

uint32_t allChannels[] = {
                          ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2,
                          ADC_CHANNEL_3, ADC_CHANNEL_4, ADC_CHANNEL_5,
                          ADC_CHANNEL_6, ADC_CHANNEL_7, ADC_CHANNEL_8,
                          ADC_CHANNEL_9, ADC_CHANNEL_10, ADC_CHANNEL_11,
                          ADC_CHANNEL_12, ADC_CHANNEL_13, ADC_CHANNEL_14,
                          ADC_CHANNEL_15, ADC_CHANNEL_16, ADC_CHANNEL_17,
                          ADC_CHANNEL_18
                        };

/**************************************************************************
------------------------ OWN FUNCTION DEFINITIONS -------------------------
***************************************************************************/


/*--------------------------------------------------------------------------
*	Name:			    ADCinit
*	Description:	Initializes the ADC channels entered through the terminal 
*	Parameters:		void
*
*	Returns:		  ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t ADCinit()
{
  // Turn globals to initial value
  chCount = 0;
  for(uint8_t k = 0; k < MAX_CHANNELS; k++)
  {
    adcChannel[k] = 0;
  }

  // Enable clocks to peripherals
  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // Set GPIO pins in analog mode
  GPIO_InitTypeDef gpioInitA = {0};
  GPIO_InitTypeDef gpioInitB = {0};
  GPIO_InitTypeDef gpioInitC = {0};

  // Get arguments from command line
  while(!fetch_uint32_arg(&adcChannel[chCount]))
  {  //loops through the values entered
    if(adcChannel[chCount] > (MAX_CHANNELS - 1))
    {                  //then ask user to enter valid values
      printf("ADC values should be between 0 and 18 :)\n");
      return (!CmdReturnOk);
    }
    chCount++;
  }

  // Initialize pins as analog in corresponding port
  for(uint8_t m = 0; m<chCount; m++)
  {
    if(adcChannel[m]<8)
      gpioInitA.Pin |= (uint16_t)(1 << adcChannel[m]);
    else if (adcChannel[m]<10) 
      gpioInitB.Pin |= (uint16_t)(1 << adcChannel[m]);
    else if (adcChannel[m]<16) 
      gpioInitC.Pin |= (uint16_t)(1 << adcChannel[m]);
    else
      printf("Channel unavailable or wrong!");
  }
  if (gpioInitA.Pin != 0){
    gpioInitA.Mode = GPIO_MODE_ANALOG;
    gpioInitA.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpioInitA);
  }

  if (gpioInitB.Pin != 0){
    gpioInitB.Mode = GPIO_MODE_ANALOG;
    gpioInitB.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &gpioInitB);
  }

  if (gpioInitC.Pin != 0){
    gpioInitC.Mode = GPIO_MODE_ANALOG;
    gpioInitC.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &gpioInitC);
  }

  // ADC Initialization parameters
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  //hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    printf("ADC initialization failed!!!\n");
  }
  
  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("adcinit", ADCinit,"\t\tInitialize given ADC channels")

/*--------------------------------------------------------------------------
*	Name:			    RawAdc2Voltage
*	Description:	Changes raw value from ADC to voltage
*	Parameters:		uint32_t: raw ADC value
*
*	Returns:		  double: voltage.
---------------------------------------------------------------------------*/
double RawAdc2Voltage(uint32_t raw)
{
  return (raw*VOLTAGE_REFERENCE/4095);
}

/*--------------------------------------------------------------------------
*	Name:			    PrintGraph
*	Description:	Presents the voltages in ASCII graph
*	Parameters:		double: voltage
*               uint_32t: channel
*
*	Returns:		  void.
---------------------------------------------------------------------------*/
void PrintGraph(double voltage, uint32_t channel)
{
  uint16_t vBars = voltage/VOLTAGE_REFERENCE*MAX_BARS;
  // Change to yellow
  printf("\e[1;33m");
  printf("%ld ", channel);
  // Change to white
  printf("\e[1;37m");
  printf("[");
  // Change to cyan
  printf("\e[1;36m");
  // Print cyan bars of voltage
  for(uint16_t i = 0; i<vBars; i++)
    printf("|");
  // Change to red
  printf("\e[0;31m");
  // Print red bars of missing voltage
  for(uint16_t i = 0; i<MAX_BARS-vBars; i++)
    printf("|");
  // Change to blue
  printf("\e[1;34m");
  printf("\t%.2lfV", voltage);
  // Change to white
  printf("\e[1;37m");
  printf("]\n");
  // Reset the colors
  printf("\e[0m");
}

/*--------------------------------------------------------------------------
*	Name:			    ADCresult
*	Description:	Samples the data from channels and prints the converted 
*               results
*	Parameters:		void
*
*	Returns:		  ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t ADCresult()
{
  uint32_t currentSamples[MAX_CHANNELS] = {0};
  // Channels configuration parameters that apply to all channels
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  
  double voltage;
  printf("\n");
  char ch = getchar();
  while((ch != '\n') && (ch != EOF) )
  {
    ch = getchar();
    for(int n = 0; n < chCount; n++)
    { 
      sConfig.Channel = allChannels[adcChannel[n]];
      // sConfig.Channel = ADC_CHANNEL_7;

      if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
      {
        printf("ADC configuration failed!!!\n");
      }

      //Turn ADC1 on
      HAL_ADC_Start(&hadc1);

      //Wait for ADC conversion to complete, the timeout is 100ms
      if(HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK)
      {
        printf("Poll for convertion failed\n");
      }

      //Judge whether ADC conversion is successful
      if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
      {
        //Read value
        currentSamples[n] =  HAL_ADC_GetValue(&hadc1);
        voltage = RawAdc2Voltage(currentSamples[n]);
        printf("\033[0;33m"); 
        // printf("raw: %ld\tV: %.2lf\n", currentSamples[n], voltage);
        PrintGraph(voltage, adcChannel[n]);
      }

      HAL_ADC_Stop(&hadc1);
    }
    for(int i = 0; i < chCount; i++)
    {
      printf("\033[A");
    }
    printf("\r");
    HAL_Delay(100);
    WDTFeed();
    // Reset the colors
    printf("\e[0m");
  }

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("adcresult", ADCresult,"\t\tRetrieve results from given ADC channels")
