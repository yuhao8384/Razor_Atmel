/**********************************************************************************************************************
File: user_app.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app as a template:
 1. Copy both user_app.c and user_app.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserAppInitialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserAppRunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserAppFlags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern u8 G_au8DebugScanfBuffer[];  /* From debug.c */
extern u8 G_u8DebugScanfCharCount;  /* From debug.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp_StateMachine;            /* The state machine function pointer */
static u32 UserApp_u32Timeout;                      /* Timeout counter used across states */

static u8 UserApp_au8UserInputBuffer[128];  /* Char buffer */
static u8* UserApp_au8UserInputBuffer_pointer = UserApp_au8UserInputBuffer;




/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserAppInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserAppInitialize(void)
{
  /*display my name in Line1*/
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR+4, "A3.YuHao");
  
  /*set the backlight of LCD*/
  LedOn(LCD_RED);
  LedOn(LCD_BLUE);
  LedOff(LCD_GREEN);
  
  /*clear the user_input_buffer*/
  for(u8 i = 0; i < 128; i++)
  {
    UserApp_au8UserInputBuffer[i] = 0;
  }
  
  /* Home the cursor */
  LCDCommand(LCD_ADDRESS_CMD | LINE2_START_ADDR); 
  LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR | LCD_DISPLAY_BLINK);
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp_StateMachine = UserAppSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp_StateMachine = UserAppSM_FailedInit;
  }

} /* end UserAppInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserAppRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserAppRunActiveState(void)
{
  UserApp_StateMachine();

} /* end UserAppRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void UserAppSM_Idle(void)
{
   u8 u8CharsCount = 0;/*the number of chars*/
   static u32 u32CharsNumber = 0; /*the total number of chars*/
   static u8 u8CursorPosition = 0;
   static u8 u8ScanRate = 0;
   
   u8ScanRate++;
   /*Scan the keyboard every 10ms*/
   if(u8ScanRate == 10)
   {
       u8ScanRate = 0;
       /* Read the buffer*/   
       u8CharsCount = DebugScanf(UserApp_au8UserInputBuffer_pointer);
       u32CharsNumber += u8CharsCount;
       
       /*display the chars*/
       LCDMessage(LINE2_START_ADDR + u8CursorPosition, UserApp_au8UserInputBuffer_pointer);
       
       /* if UserApp_au8UserInputBuffer is full, start from begining*/
       if( (u32CharsNumber % 127) ==0)
       {
           for(u8 i = 0; i < 128; i++)
          {
            UserApp_au8UserInputBuffer[i] = 0;
          }
         UserApp_au8UserInputBuffer_pointer = UserApp_au8UserInputBuffer;
       }
       else
       {
         UserApp_au8UserInputBuffer_pointer += u8CharsCount;
       }
       
       /*if screen is full, start from the left*/
       if(u8CursorPosition == 20 )
       {
         LCDClearChars(LINE2_START_ADDR, 20);
         u8CursorPosition = 0;
       }
       else
       {
         u8CursorPosition += u8CharsCount;
       }
   }
   
   /*Clear the chars in Line2 and start from begining*/
   if( WasButtonPressed(BUTTON0) )
   {
     ButtonAcknowledge(BUTTON0);
     LCDClearChars(LINE2_START_ADDR, 20);
     u8CursorPosition = 0;    
   }
   
} /* end UserAppSM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_Error(void)          
{
  
} /* end UserAppSM_Error() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserAppSM_FailedInit(void)          
{
    
} /* end UserAppSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
