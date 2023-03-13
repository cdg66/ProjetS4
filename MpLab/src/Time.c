#include <xc.h>
#include <sys/attribs.h>
#include "config.h"
#include "swt.h"
#include "lcd.h"
#include "Time.h"
#include "btn.h"



void init_time()
{

    BTN_Init();
    LCD_Init();
    LCD_WriteStringAtPos("Heure : ", 0, 0);     //affichage heure
    LCD_WriteIntAtPos(00, 3, 0, 13, 0);  // affichage des secondes
    LCD_WriteStringAtPos(":", 0, 13);
    LCD_WriteIntAtPos(00, 3, 0, 10, 0);  // affichage des minutes
    LCD_WriteStringAtPos(":", 0, 10);
    LCD_WriteIntAtPos(00, 3, 0, 7, 0);    // affichage de heures
}

int set_time()
{
    
    unsigned int seconde = 0;
   
    while (BTN_GetValue(4) != 1)        //tant que BTNC c'est pas appuyé
    {
        
        
        if(BTN_GetValue(3) == 1)
        {
            seconde++;
            while(BTN_GetValue(3) == 1)
            {}  
        }
        if (BTN_GetValue(2) == 1)
        {
        
            seconde = seconde + 60;
            while(BTN_GetValue(2) == 1)
            {}
        }
        
        if(BTN_GetValue(1) == 1)
        {
            
            seconde = seconde + 3600;
            while(BTN_GetValue(1) == 1)
            {}
        }
        
    LCD_WriteIntAtPos(seconde%60, 3, 0, 13, 0);  // affichage des secondes
    LCD_WriteStringAtPos(":", 0, 13);
    LCD_WriteIntAtPos(seconde/60%60, 3, 0, 10, 0);  // affichage des minutes
    LCD_WriteStringAtPos(":", 0, 10);
    LCD_WriteIntAtPos(seconde/3600%24, 3, 0, 7, 0);  // affichage des heures
    
    }
    
    return seconde;


}
