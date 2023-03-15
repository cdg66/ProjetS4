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
#include "btn.h"
#include "adc.h"
#include "aic.h"
#include <math.h>
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

unsigned int set_time();
void init_analog();
int get_temp();

MAIN_DATA mainData;
int compteur_temps = 0;
int compteur_flag = 0;
int Flag_sec = 0;
int val_test = 0;

static volatile int Flag_1m = 0;
static volatile int Flag_btn = 0;
void __ISR(_TIMER_2_VECTOR, IPL2AUTO) Timer2ISR(void)
{
   Flag_1m = 1;           //    Indique à la boucle principale qu'on doit traiter
   Flag_btn = 0;
   IFS0bits.T2IF = 0;     //    clear interrupt flag
}

#define TMR_TIME    0.001             // x us for each tick

void initialize_timer_interrupt(void) {
  T2CONbits.TCKPS = 3;                //    256 prescaler value
  T2CONbits.TGATE = 0;                //    not gated input (the default)
  T2CONbits.TCS = 0;                  //    PCBLK input (the default)
  PR2 = (int)(((float)(TMR_TIME * PB_FRQ) / 256) + 0.5);   //set period register, generates one interrupt every 1 ms
                                      //    48 kHz * 1 ms / 256 = 188
  TMR2 = 0;                           //    initialize count to 0
  IPC2bits.T2IP = 2;                  //    INT step 4: priority
  IPC2bits.T2IS = 0;                  //    subpriority
  IFS0bits.T2IF = 0;                  //    clear interrupt flag
  IEC0bits.T2IE = 1;                  //    enable interrupt
  T2CONbits.ON = 1;                   //    turn on Timer5
}

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
int test = 250;
/*static unsigned long int counter=0;
static void LedTask(void) {
    if(counter++ == 20000){
        LED_ToggleValue(1);
        counter = 0;
    }  
}*/


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
   /* static int data = 0;
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
    */
}

uint32_t packetnumber = 0; 
void Packetize_Task()
{
     
   /* if (accel_packet_ready == false)
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
//    UDP_Send_Packet = true;*/
}

void init_analog()
{
    tris_PMODS_JB9 = 1;  //  Set up jb9 sur pmod analog
    ansel_PMODS_JB9 = 1; //enable analog
}

int get_temp()
{    
    return ADC_AnalogRead(24);
}

void SSD_Task(int temp)
{
    SSD_WriteDigitsGrouped(temp, 0x00);    
}

unsigned int set_time()
{
    int sec = 0;
    char btn= 0;
    int position = 1;
    while(!BTN_GetValue('C'))
    {
        btn = BTN_GetGroupValue();
        switch(btn)
        {
            case 0b00000001:  //bouton up
                switch(position)
                {                
                    case 1:
                      sec = sec+1;
                      while(0b00000001 == BTN_GetGroupValue());
                      break;
                    case 2:
                      sec = sec+60;
                      while(0b00000001 == BTN_GetGroupValue());
                      break;
                    case 3:
                      sec = sec+3600;
                      while(0b00000001 == BTN_GetGroupValue());
                      break;
                }
                break;
            case 0b00000010:  //bouton left
                position++;
                if(position > 3){position = 1;}
                while(0b00000010 == BTN_GetGroupValue());
                break;
        }
        LCD_seconde(sec);
    }
    return sec;
}

void MAIN_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    mainData.state = MAIN_STATE_INIT;
    LCD_CLEAR();
    LCD_WriteStringAtPos("Heure : ", 0, 0);
    mainData.handleUSART0 = DRV_HANDLE_INVALID;
    initialize_timer_interrupt();
    UDP_Initialize(); // Initialisation de du serveur et client UDP
    LCD_Init(); // Initialisation de l'écran LCD
    ACL_Init(); // Initialisation de l'accéléromètre
    ADC_Init();
    SSD_Init(); // Initialisation du Timer4 et de l'accéléromètre
    RGBLED_Init(); // Initialisation de la LED RGB
    init_analog();
    compteur_temps = set_time();
    
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
            LCD_WriteStringAtPos("Heure : ", 0, 0);
            if(Flag_sec)
            {
                Flag_sec = 0;
                val_test = get_temp();
                float rohm = 10000*(1023.0/(float)val_test-1.0);
                float temp_c = (1.0 /(0.001129148+(0.000234125*log(rohm))+0.0000000876741*log(rohm)*log(rohm)*log(rohm)))-273.15;
                LCD_Task(temp_c,test,test,compteur_temps);
                SSD_Task(val_test);

            }
            //LedTask(); //toggle LED1 à tout les 500000 cycles
            //Packetize_Task();
            //SSD_Task(val_test);
            
            UDP_Tasks();
            //ManageSwitches();
        	JB1Toggle();
            LED0Toggle();
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
        if(Flag_1m == 1)
        {       
            Flag_1m = 0;
            if(++compteur_flag >= 200)
            {
                compteur_temps++;
                compteur_flag = 0;
                Flag_sec = 1;
            }  
        }
        SYS_Tasks();
        MAIN_Tasks();   
    }
    return 0;
}


/*******************************************************************************
 End of File
 */
