/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    main.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "main.h"
#include "system_config.h"
#include "system/common/sys_module.h"   // SYS function prototypes
#include "driver/spi/src/dynamic/drv_spi_internal.h"
#include "UDP_app.h"
#include "led.h"
#include "ssd.h"
#include "accel.h"
#include "lcd.h"
#include "app_commands.h"
#include "../S4e_Depart_etud.X/rgbled.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the TCPIP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
 */

MAIN_DATA mainData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* Application's LED Task Function 
 Fonction qui fait clignoter une LED la LED1 à chaque 20000 execution du code
 */
static unsigned long int counter=0;
static void LedTask(void) {
    if(counter++ == 20000){
        LED_ToggleValue(1);
        counter = 0;
    }  
}


static bool sw0_old; 
void ManageSwitches()
{
    bool sw0_new = SWITCH0StateGet();
    if((sw0_new != sw0_old) && sw0_new)
    {
        //strcpy(UDP_Send_Buffer, "Bonjour S4\n\r");
        //UDP_bytes_to_send = strlen(UDP_Send_Buffer);
        UDP_Send_Packet = true;       
    }

    sw0_old = sw0_new; 
}

void RGB_Task()
{
    static int data = 0;
    //ACL_ReadGValues(float *rgGVals);
    //static unsigned char tempTestRGB[3] = {5,20,50};
    if (UDP_Receive_Packet == true)
    {
        data = 0;
        UDP_Receive_Packet = false;
        return;
    }
    UDP_Receive_Packet = false;
    

    // busted all the data we hold 0 until new data
    //if (data >= 40)
    //{
    //    RGBLED_SetValue(0, 0, 0);
    //    return;
    //}
    if (accel_RGB_ready == false)
    {
        return;
    }
    RGBLED_SetValue(UDP_Receive_Buffer[data+4], UDP_Receive_Buffer[data+44], UDP_Receive_Buffer[data+84]);
    data++;
    if (data >= 40)
    {
        data = 0;
    }
    //RGBLED_SetValue(tempTestRGB[0], tempTestRGB[1], tempTestRGB[2]);
    //tempTestRGB[0]++;
    //tempTestRGB[1]++;
    //tempTestRGB[2]++;
    
    //bits 23-16 correspond to color R, bits 15-8 correspond to color G, bits 7-0 (LSB byte) correspond to color B.
    //Vous devez coder une fonction qui utilise les valeur des moyennes calculé 
    //et faire varier la couleur de la RGB. 
    
}

uint32_t packetnumber = 0; 
void Packetize_Task()
{
     
    if (accel_packet_ready == false)
    {
        return;
    }
    accel_packet_ready = false;
    // clear old data 
    memset(UDP_Send_Buffer, '\0', sizeof(UDP_Send_Buffer) );
    // add packet number to buffer
    //memcpy(UDP_Send_Buffer,(char *)packetnumber, 4);
    UDP_Send_Buffer[3] = (uint8_t) (packetnumber >> 24);
    UDP_Send_Buffer[2] = (uint8_t) (packetnumber >> 16);
    UDP_Send_Buffer[1] = (uint8_t) (packetnumber >> 8);
    UDP_Send_Buffer[0] = (uint8_t) (packetnumber);
    packetnumber++;
    // concat acc buffer
    //strcpy(UDP_Send_Buffer+4, (char*)packet_accel_buffer);
    memcpy(UDP_Send_Buffer+4, packet_accel_buffer, sizeof(packet_accel_buffer));
    memset(UDP_Send_Buffer+485 , '\0', 1 );
    // 
    UDP_bytes_to_send = 485;
    UDP_Send_Packet = true;
//    accel_packet_ready = false;
//    strcpy(UDP_Send_Buffer, "test");
//    UDP_bytes_to_send = sizeof(UDP_Send_Buffer);
//    UDP_Send_Packet = true;
}






//int moyenne()
//{
//    UDP_Receive_Buffer[1]
//
//    return;
//}
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void MAIN_Initialize ( void )

  Remarks:
    See prototype in main.h.
 */

void MAIN_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    mainData.state = MAIN_STATE_INIT;

    mainData.handleUSART0 = DRV_HANDLE_INVALID;

    UDP_Initialize(); // Initialisation de du serveur et client UDP
    LCD_Init(); // Initialisation de l'écran LCD
    ACL_Init(); // Initialisation de l'accéléromètre
    SSD_Init(); // Initialisation du Timer4 et de l'accéléromètre
    RGBLED_Init(); // Initialisation de la LED RGB
}


/******************************************************************************
  Function:
    void MAIN_Tasks ( void )
 * Fonction qui execute les tâches de l'application. Cette fonction est une
 * machien d'état :
 * 1. MAIN_STATE_INIT; Initialise les périphérique de communication USART et 
 *    passe à l'état 2 quand l'initialisation est terminée.
 * 2. MAIN_STATE_SERVICE_TASKS; Execute les tâches de l'application. Ne change 
 * jamais d'état.

  Remarks:
    See prototype in main.h.
 */

void MAIN_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( mainData.state )
    {
            /* Application's initial state. */
        case MAIN_STATE_INIT:
        {
            bool appInitialized = true;
            SYS_CONSOLE_MESSAGE("Init\r\n");

            if (mainData.handleUSART0 == DRV_HANDLE_INVALID)
            {
                mainData.handleUSART0 = DRV_USART_Open(MAIN_DRV_USART, DRV_IO_INTENT_READWRITE|DRV_IO_INTENT_NONBLOCKING);
                appInitialized &= (DRV_HANDLE_INVALID != mainData.handleUSART0);
            }


            if (appInitialized)
            {
                mainData.state = MAIN_STATE_SERVICE_TASKS;
            }
            break;
        }

        case MAIN_STATE_SERVICE_TASKS:
        {
            LedTask(); //toggle LED1 à tout les 500000 cycles
            accel_tasks(); // 
            Packetize_Task();
            //accel_packet_ready = false;
            UDP_Tasks();
            ManageSwitches();
        	JB1Toggle();
            LED0Toggle();
            RGB_Task();
            break;
        }

            /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

int main(void) {
    
    SYS_Initialize(NULL);
    MAIN_Initialize();
    SYS_INT_Enable();
    SSD_WriteDigitsGrouped(0xFA9B,0x1);
    
    while (1) {
        //RGB_Task(); ////////
        SYS_Tasks();
        MAIN_Tasks();
        
    };

    return 0;
}


/*******************************************************************************
 End of File
 */
