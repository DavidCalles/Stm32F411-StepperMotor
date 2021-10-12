/*************************************************************************
* File Name         SimpleCommand.c
* Description       Print tests with and without string argument.
*				          
* Date				          Name(s)						          Action
* September 26, 2021		Jaskaran K. & David C.			First Implementation
***************************************************************************/

/**************************************************************************
---------------------------- LIBRARY DEFINITIONS --------------------------
***************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"

/**************************************************************************
------------------------ OWN FUNCTION DEFINITIONS -------------------------
***************************************************************************/

/*--------------------------------------------------------------------------
*	Name:			SimplePrint
*	Description:	Print a fixed message through the terminal
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t SimplePrint()
{
  // Random print to terminal
  printf("Works Fine! \n");

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("simpleprint",SimplePrint,"\t\tSimple print test")


/*--------------------------------------------------------------------------
*	Name:			SimplePrint2
*	Description:	Print a string taken from the argument parser
*	Parameters:		void
*
*	Returns:		ret: CmdReturnOk = 0 if Okay.
---------------------------------------------------------------------------*/
ParserReturnVal_t SimplePrint2()
{
  char *my_string; // Char pointer to string variable
  uint8_t retVal = 0;

  retVal = fetch_string_arg(&my_string); // Get string argument from parser

  // Check for string type argument, FATAL ERROR if not
  (!retVal) ? printf("Printing: %s \n", my_string) : printf("Ooops!! \n"); 

  return CmdReturnOk;
}

// MACRO: Add new command to help menu
ADD_CMD("simpleprint2",SimplePrint2,"\t\tNot so simple print test")
