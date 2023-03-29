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
#include "pmods.h"
#include <math.h>
#include "swt.h"

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

#define TMR_TIME    0.001             // x us for each tick



unsigned int set_time(void);
void init_analog(void);
void init_pmod(void);
float get_temp(void);
void set_fan(int val_fan);
void set_pump(int val_pump);
void set_heat(int val_heat);
void controle (void);
int RGB_Task(void);
void  Uart_Task (void);
void ManageSwitches(void);
void set_utilisateur_value(void);



MAIN_DATA mainData;
int compteur_temps = 0;
int compteur_flag = 0;
int compteur_pompe = 0;
int attente_pompe = 0;
int Flag_sec = 0;
int Flag_pompe = 0;
int Flag_attente = 1;
float temp_c = 0;
int val_test = 1023;
int compteu = 0;
float moy_temp = 0;
float moy_hum = 0;
float Max_temp = 22.0;
float Max_hum = 512;
float Min_temp = 19.0;
float Min_hum = 300;
int compteur_led = 0;
int pourcent = 0;
int compteurBufferUDP = 4;
int compt = 1;
int compt2 = 0;
signed int tab_xyz[121];

int compteur_3200;
int Flag_request_UDP = 0;
uint32_t indexPaquet = 1;

int intensite = 0;
float prct = 0;
float temp_uti = 0;
int hum_uti = 0;
    
static volatile int Flag_1m = 0;
static volatile int Flag_btn = 0;

void __ISR(_TIMER_2_VECTOR, IPL2AUTO) Timer2ISR(void)
{
   Flag_1m = 1;           //    Indique à la boucle principale qu'on doit traiter
   Flag_btn = 0;
   IFS0bits.T2IF = 0;     //    clear interrupt flag
}

void initialize_timer_interrupt(void) {
  T2CONbits.TCKPS = 7;                //    256 prescaler value   page 174
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
/*static unsigned long int counter=0;
static void LedTask(void) {
    if(counter++ == 20000){
        LED_ToggleValue(1);
        counter = 0;
    }  
}*/


static bool sw0_old; 
void ManageSwitches(void)
{
    bool sw0_new = SWITCH0StateGet();
    /*if((sw0_new != sw0_old) && sw0_new)
    {
        PMODS_SetValue(1, 4, 1);
    }*/
    if (SWITCH0StateGet() == true) PMODS_SetValue(1, 4, 1);
    else PMODS_SetValue(1, 4, 0);
    sw0_old = sw0_new; 
}

int RGB_Task(void)
{    
    RGBLED_SetValue(intensite, 0, intensite );
    compteur_led++;
    
    if(compteur_led <= 127) intensite = intensite + 2;
    else if((127<compteur_led) & (compteur_led < 255)) intensite = intensite-2;
    if(compteur_led >= 255) compteur_led = 0;

    prct = (intensite*100);
    prct = (prct/255);
    return (int)prct;
}

void Uart_Task (void)
{
    char valeur_utilisateur[100];

    strcpy(valeur_utilisateur, "Test des valeurs");
    SYS_CONSOLE_PRINT("\r Valeur utilisateur:  Lumiere = %d Temperature = %3.1f Humitdite = %i\n\r", intensite*100/255, temp_uti, hum_uti);
    SYS_CONSOLE_PRINT("\r Valeur de lecture:  Lumiere = %d Temperature = %i Humitdite = %i\n\r", intensite*100/255, (int)moy_temp, (int)moy_hum);

}

void set_utilisateur_value(void)
{
    char btn= 0;
    int choix = 1;
    
    while(!BTN_GetValue('R'))
    {
        btn = BTN_GetGroupValue();
        switch(btn)
        {
            case 0b00000001:  //bouton up
                switch(choix)
                {                
                    case 1:
                      temp_uti = temp_uti+1;
                      while(0b00000001 == BTN_GetGroupValue());
                      break;
                    case 2:
                      hum_uti = hum_uti+5;
                      while(0b00000001 == BTN_GetGroupValue());
                      break;
                }
                break;
            case 0b00010000:  //bouton down
                switch(choix)
                {                
                    case 1:
                      temp_uti = temp_uti-1;
                      while(0b00010000 == BTN_GetGroupValue());
                      break;
                    case 2:
                      hum_uti = hum_uti-5;
                      while(0b00010000 == BTN_GetGroupValue());
                      break;
                }
                break;
            case 0b00000010:  //bouton left
                choix++;
                if(choix > 2){choix = 1;}
                while(0b00000010 == BTN_GetGroupValue());
                break;
        }
        LCD_utilisateur(hum_uti, temp_uti);
    }
}

uint32_t packetnumber = 0; 
void Packetize_Task()
{
    if(compt <= 121)
    {
        tab_xyz[compt] = compt;
    }
    if(compt >= 121)
    {
        memcpy(UDP_Send_Buffer, tab_xyz, 121*sizeof(signed int));

    }
    UDP_bytes_to_send = 484;
    if(Flag_request_UDP)
   {
        compt++;
        Flag_request_UDP = false;  
        UDP_Send_Packet = true;
    }
    if(compt >= 120)
    {
        compt = 0; 
    }
}
void init_pmod(void)
{
    PMODS_InitPin(1, 7, 0, 0, 0);  // output vent
    PMODS_InitPin(1, 2, 0, 0, 0);  // output water pump
    PMODS_InitPin(1, 3, 0, 0, 0);  // output heat
    PMODS_InitPin(1, 4, 0, 0, 0);  // output relay
}

void init_analog(void)
{
    tris_PMODS_JB9 = 1;  //  Set up jb9 sur pmod analog
    ansel_PMODS_JB9 = 1; //enable analog
}
void set_pump(int val_pump)
{
    PMODS_SetValue(1, 2, val_pump);
}
void set_heat(int val_heat)
{
    PMODS_SetValue(1, 3, val_heat);
}
void set_fan(int val_fan)
{
    PMODS_SetValue(1, 7, val_fan);
}
float get_temp(void)
{    
    float temp = 0;
    temp = ADC_AnalogRead(24)*3.22/1024.0;
    temp = temp - 0.5;
    temp = temp / 0.01;
    
    return temp;
}

void controle (void)
{
    /*
    if (SWITCH1StateGet() == true)set_pump(1);
    else set_pump(0);
    if (SWITCH2StateGet() == true)set_fan(1);
    else set_fan(0);
    if (SWITCH3StateGet() == true)set_heat(1);
    else set_heat(0);
    */
    
    if(compteu <= 39)
    {
        //2 moy_temp? Je sais que 1 de MX3 et 1 de Zybo, mais on a juste une variable. Il faudrait une variable par capteur et faire un fct qui fait la moyenne des deux capteurs    -Justin
        //D�paqu�tise les donn�es recues du MX3 et les mets dans le bon ordre -Justin
        moy_temp =((UDP_Receive_Buffer[compteurBufferUDP]) | ((UDP_Receive_Buffer[compteurBufferUDP+1]) << 8) | ((UDP_Receive_Buffer[compteurBufferUDP+2]) << 16) | ((UDP_Receive_Buffer[compteurBufferUDP+3]) << 24));
        moy_hum = ((UDP_Receive_Buffer[compteurBufferUDP+160]) | ((UDP_Receive_Buffer[compteurBufferUDP+161]) << 8) | ((UDP_Receive_Buffer[compteurBufferUDP+162]) << 16) | ((UDP_Receive_Buffer[compteurBufferUDP+163]) << 24)); // Receive UDP
        compteu++;
    }
    if (compteurBufferUDP >= 156)
    {
        compteurBufferUDP = 4;
    }
        compteurBufferUDP += 4;
    if(compteu >= 40)
    {
        compteu = 0;
        //moy_temp = moy_temp/40.0;
        //moy_hum = moy_hum/40.0;
    }
    
    //SWITCH0 = FAN; SWITCH1 = HEAT; SWITCH2 = POMPE
    if (SWITCH1StateGet() == true)
    {
        set_heat(1);
    }
    else if(moy_temp <= temp_uti && SWITCH1StateGet() == false)
    {
        set_heat(0);    
    }
        
    if (SWITCH2StateGet() == true)
    {
        set_fan(1);
    }
    else if(moy_temp <= temp_uti && SWITCH1StateGet() == false)
    {
        set_fan(0);    
    }
    
    if(moy_temp > temp_uti )
    {
        set_fan(1);
         
    }
    else if(moy_temp < temp_uti - 3)
    {   
        set_heat(1);
    }  
        
        
   // if((temp_uti <= moy_temp) && (moy_temp <= temp_uti) && SWITCH2StateGet() == false && SWITCH1StateGet() == false)
   // {
      //  set_fan(0);
     //   set_heat(0); 
   // }
    
    if(((moy_hum < hum_uti-3 && Flag_pompe == 0) || SWITCH3StateGet() == true) && Flag_attente == 1)
    {
        set_pump(1);
        Flag_pompe = 1;
        Flag_attente = 0;
    }
    if(Flag_pompe == 0)
    {
        set_pump(0);
    }
}


void SSD_Task(int temp)
{
    int valeur = 0;
    switch (temp)
    {
        case 100:
        {
           valeur = 256;
           break;
        }
        
         case 90 ... 99:
        {
           valeur = temp + 54;
           break;
        }
         
         case 80 ... 89:
        {
           valeur = temp + 48;
           break;
        }
         
         case 70 ... 79:
        {
           valeur = temp + 42;
           break;
        }
         
         case 60 ... 69:
        {
           valeur = temp + 36;
           break;
        }
         
         case 50 ... 59:
        {
           valeur = temp + 30;
           break;
        }
         
         case 40 ... 49:
        {
           valeur = temp + 24;
           break;
        }
         
         case 30 ... 39:
        {
           valeur = temp + 18;
           break;
        }
         
         case 20 ... 29:
        {
           valeur = temp + 12;
           break;
        }
         
         case 10 ... 19:
        {
           valeur = temp + 6;
           break;
        }
         
         case 0 ... 9:
        {
           valeur = temp;
           break;
        } 
        break;
    }
    
    SSD_WriteDigitsGrouped(valeur, 0x00);    
}

unsigned int set_time(void)
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
    mainData.handleUSART0 = DRV_HANDLE_INVALID;
    initialize_timer_interrupt();
    UDP_Initialize(); // Initialisation de du serveur et client UDP
    LCD_Init(); // Initialisation de l'écran LCD
    ACL_Init(); // Initialisation de l'accéléromètre
    ADC_Init();
    //UART_InitPoll(115200);
    //AIC_Init();
    SSD_Init(); // Initialisation du Timer4 et de l'accéléromètre
    RGBLED_Init(); // Initialisation de la LED RGB
    init_analog();
    init_pmod();
    compteur_temps = set_time();
    LCD_CLEAR();
    set_utilisateur_value();
    LCD_CLEAR();
    LCD_WriteStringAtPos("Heure : ", 0, 0);

    
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
            if(Flag_sec)
            {
                Flag_sec = 0;
                temp_c = get_temp();
                LCD_Task(temp_c,moy_hum,compteur_temps);
                pourcent = RGB_Task();
                SSD_Task(pourcent);
                Uart_Task();
                LED2Toggle();
            }
            //LedTask(); //toggle LED1 à tout les 500000 cycles
            Packetize_Task();
            controle();
            UDP_Tasks();
            ManageSwitches();
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
    
    while (1) {
        if(Flag_1m == 1)
        {       
            Flag_1m = 0;
            if(++compteur_flag >= 1000)
            {
                compteur_temps++;
                compteur_flag = 0;
                Flag_sec = 1;
            }
            if(++compteur_3200 >= 3200)     //Flag lev� au 3,2sec pour envoyer request UDP au MX3 -Justin
            {
                compteur_3200 = 0;
                Flag_request_UDP = true;
            }  
            if(Flag_pompe == 1)
            {
                attente_pompe = 0;
                if(++compteur_pompe >= 5000 )
                {
                    compteur_pompe = 0;
                    Flag_pompe = 0;
                } 
            }else
            {
                if(++attente_pompe >= 10000 )
                {
                    attente_pompe = 0;
                    Flag_attente = 1;
                } 
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
