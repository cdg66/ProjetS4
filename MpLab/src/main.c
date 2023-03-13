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
#include "UDP_app.h"
#include "tcpip/tcpip.h"
#include "rgbled.h"
#include <stdio.h>
#include <stdlib.h>

 

 

MAIN_DATA mainData;

 

static int compteur1 = 1;

 

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
    
    //Vous devez coder une fonction qui utilise les valeur des moyennes calculé 
    //et faire varier la couleur de la RGB. 
    
            
        /*

 

            unsigned int MoyenneX = 0;
            unsigned int MoyenneY = 0;
            unsigned int MoyenneZ = 0;
            compteur1 = 0;
            //signed short accelX = ((signed int) accel_buffer[0]<<24)>>20  | accel_buffer[1] >> 4; //VR
            //signed short accelY = ((signed int) accel_buffer[2]<<24)>>20  | accel_buffer[3] >> 4; //VR
            //signed short accelZ = ((signed int) accel_buffer[4]<<24)>>20  | accel_buffer[5] >> 4; //VR
            
            
            signed short accelX = ((signed int) UDP_Receive_Buffer[0]<<24)>>20  | UDP_Receive_Buffer[1] >> 4; //VR
            signed short accelY = ((signed int) UDP_Receive_Buffer[2]<<24)>>20  | UDP_Receive_Buffer[3] >> 4; //VR
            signed short accelZ = ((signed int) UDP_Receive_Buffer[4]<<24)>>20  | UDP_Receive_Buffer[5] >> 4; //VR
            
            
            
            MoyenneX = accelX;
            MoyenneY = accelY;
            MoyenneZ = accelZ;

 

            
            RGBLED_SetValue(MoyenneX, MoyenneY, MoyenneZ);     
        */
    
        /***************************************************************************************************************************/
            
        
       
        if(compteur1 < 41 )
        {    
            
            unsigned char MoyenneX1 = ((UDP_Receive_Buffer[compteur1*2] >> 4) + (UDP_Receive_Buffer[compteur1*3]));
            unsigned char MoyenneY1 = ((UDP_Receive_Buffer[compteur1*2 + 40] >> 4) + (UDP_Receive_Buffer[compteur1*3 + 40]));
            unsigned char MoyenneZ1 = ((UDP_Receive_Buffer[compteur1*2 + 80] >> 4) + (UDP_Receive_Buffer[compteur1*3 + 80]));
            
            unsigned int MoyenneX = (unsigned int)MoyenneX1;
            unsigned int MoyenneY = (unsigned int)MoyenneY1;
            unsigned int MoyenneZ = (unsigned int)MoyenneZ1;
            
            
            
            RGBLED_SetValue(MoyenneX, MoyenneY, MoyenneZ); 
            
            
            compteur1 ++;
        }
        else
        {
            compteur1 = 1;  
        }
}
    
  
    
    
    
    
 
    
    

 

 

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
    RGBLED_Init();

 

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

 

            accel_tasks();
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
        SYS_Tasks();
        MAIN_Tasks();
    };

 

    return 0;
}

 


/*******************************************************************************
 End of File
 */