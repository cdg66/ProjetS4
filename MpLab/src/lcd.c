/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Digilent

  @File Name
    lcd.c

  @Description
        This file groups the functions that implement the LCD library.
        The library implements control of the LCD device. 
        It is accessed in a "parallel like" approach. 
        Library provides functions for simple commands, displaying characters, handling user characters.
        Include the file together with config.h, utils.c and utils.h in the project when this library is needed.	
 
  @Author
    Cristian Fatu 
    cristian.fatu@digilent.ro
*/
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
#include <xc.h>
#include <sys/attribs.h>
#include <string.h>
#include "config.h"
#include "lcd.h"
#include <stdio.h>
/* ************************************************************************** */

/* ------------------------------------------------------------ */
/***	LCD_Init
**
**	Parameters:
**		
**
**	Return Value:
**		
**
**	Description:
**		This function initializes the hardware used in the LCD module: 
**      The following digital pins are configured as digital outputs: LCD_DISP_RS, LCD_DISP_RW, LCD_DISP_EN
**      The following digital pins are configured as digital inputs: LCD_DISP_RS.
**      The LCD initialization sequence is performed, the LCD is turned on.
**          
*/
void LCD_Init()
{
    LCD_ConfigurePins();
    LCD_InitSequence(displaySetOptionDisplayOn);
}

/* ------------------------------------------------------------ */
/***	LCD_ConfigurePins
**
**	Parameters:
**		
**
**	Return Value:
**		
**
**	Description:
**		This function configures the digital pins involved in the LCD module: 
**      The following digital pins are configured as digital outputs: LCD_DISP_RS, LCD_DISP_RW, LCD_DISP_EN
**      The following digital pins are configured as digital inputs: LCD_DISP_RS.
**      The function uses pin related definitions from config.h file.
**      This is a low-level function called by LCD_Init(), so user should avoid calling it directly.
**      
**          
*/
void LCD_ConfigurePins()
{
	//tris_LCD_DATA |= msk_LCD_DATA;
	lat_LCD_DISP_RW = 1;
	lat_LCD_DISP_EN = 1;    

    // set control pins as digital outputs.
    tris_LCD_DISP_RS = 0;
    tris_LCD_DISP_RW = 0;
    tris_LCD_DISP_EN = 0;
    
    // disable analog (set pins as digital))
    //ansel_LCD_DISP_RS = 0;
    
    // default (IO) function for remapable pins
    rp_LCD_DISP_RS = 0;
    rp_LCD_DISP_RW = 0;
    rp_LCD_DISP_EN = 0;
    
    // make data pins digital (disable analog)
    /*ansel_LCD_DB2 = 0;
    ansel_LCD_DB4 = 0;
    ansel_LCD_DB5 = 0;
    ansel_LCD_DB6 = 0;
    ansel_LCD_DB7 = 0;*/
}

/* ------------------------------------------------------------ */
/***	LCD_WriteByte
**
**	Parameters:
**		unsigned char bData - the data to be written to LCD, over the parallel interface
**
**	Return Value:
**		
**
**	Description:
**		This function writes a byte to the LCD. 
**      It implements the parallel write using LCD_DISP_RS, LCD_DISP_RW, LCD_DISP_EN, 
**      LCD_DISP_RS pins, and data pins. 
**      For a better performance, the data pins are accessed using a pointer to 
**      the register byte where they are allocated.
**      This is a low-level function called by LCD write functions, so user should avoid calling it directly.
**      The function uses pin related definitions from config.h file.
**      
**          
*/


void delay39us(unsigned t) //
{
    int16_t fpbthresh = 1872; // = SYS_CLK_BUS_PERIPHERAL_1 * 39e-6;
    T5CON = 0x8000; /* Enable TMR1, Tpb, 1:1 */
    while (t--) { /* t x 39us loop */
        TMR5 = 0;
        while (TMR5 < fpbthresh);
    }
    T5CON = 0;
}

void delay1us(unsigned t)
{
    int16_t fpbthresh = 24; // = SYS_CLK_BUS_PERIPHERAL_1 * 1e-6;
    T5CON = 0x8000; /* Enable TMR1, Tpb, 1:1 */
    while (t--) { /* t x 39us loop */
        TMR5 = 0;
        while (TMR5 < fpbthresh);
    }
    T5CON = 0;
}



void LCD_WriteByte(unsigned char bData)
{
	// Configure IO Port data pins as output.
	lat_LCD_DISP_RW = 0;
    unsigned char *pLCDData = (unsigned char *)(0xBF886430);
    *pLCDData = bData;
    tris_LCD_DATA &= ~msk_LCD_DATA;
    delay1us(1);
	// clear RW

    // access data as contiguous 8 bits, using pointer to the LSB byte of LATE register
    
    //delay1us(1);

	// Set En
	lat_LCD_DISP_EN = 1;    

    delay1us(1);
	// Clear En
	lat_LCD_DISP_EN = 0;

    
    delay1us(1);
	tris_LCD_DATA |= msk_LCD_DATA;
	lat_LCD_DISP_RW = 1;
// Set RW
 	delay1us(1);
    //lat_LCD_DISP_EN = 1;
    /*delay1us(1);
    lat_LCD_DISP_EN = 0;*/
    delay1us(1);
    while(LCD_ReadStatus() & mskBStatus );

}

/* ------------------------------------------------------------ */
/***	LCD_ReadByte
**
**	Parameters:
**		
**
**	Return Value:
**		unsigned char - the data read from LCD, over the parallel interface
**
**	Description:
**		This function reads a byte from the LCD. 
**      It implements the parallel read using LCD_DISP_RS, LCD_DISP_RW, LCD_DISP_EN, 
**      LCD_DISP_RS pins, and data pins. 
**      This is a low-level function called by LCD_ReadStatus function, so user should avoid calling it directly.
**      The function uses pin related definitions from config.h file.
**      
**          
*/
unsigned char LCD_ReadByte()
{
    unsigned char bData;
	// Configure IO Port data pins as input.
    tris_LCD_DATA |= msk_LCD_DATA;
	// Set RW
	lat_LCD_DISP_RW = 1;
    delay1us(1);

    // Clear En
	lat_LCD_DISP_EN = 1;
    delay1us(1);
  	bData = (unsigned char)(prt_LCD_DATA & (unsigned int)msk_LCD_DATA);
	lat_LCD_DISP_EN = 0;
	return bData;
}

/* ------------------------------------------------------------ */
/***	LCD_ReadStatus
**
**	Parameters:
**		
**
**	Return Value:
**		unsigned char - the status byte that was read.
**
**	Description:
**		Reads the status of the LCD.  
**      It clears the RS and calls LCD_ReadByte() function. 
**      The function uses pin related definitions from config.h file.
**          
*/
unsigned char LCD_ReadStatus()
{
	// Clear RS
	lat_LCD_DISP_RS = 0;
    
	unsigned char bStatus = LCD_ReadByte();
	return bStatus;
}

/* ------------------------------------------------------------ */
/***	LCD_WriteCommand
**
**	Parameters:
**		unsigned char bCmd -  the command code byte to be written to LCD
**
**	Return Value:
**		
**
**	Description:
**		Writes the specified byte as command. 
**      It clears the RS and writes the byte to LCD. 
**      The function uses pin related definitions from config.h file.
**      
**          
*/
void LCD_WriteCommand(unsigned char bCmd)
{ 
	// Clear RS
	lat_LCD_DISP_RS = 0;

	// Write command byte
	LCD_WriteByte(bCmd);
}

/* ------------------------------------------------------------ */
/***	LCD_WriteDataByte
**
**	Parameters:
**		unsigned char bData -  the data byte to be written to LCD
**
**	Return Value:
**		
**
**	Description:
**      Writes the specified byte as data. 
**      It sets the RS and writes the byte to LCD. 
**      The function uses pin related definitions from config.h file.
**      This is a low-level function called by LCD write functions, so user should avoid calling it directly.
**      
**          
*/
void LCD_WriteDataByte(unsigned char bData)
{
	// Set RS 
	lat_LCD_DISP_RS = 1;

	// Write data byte
	LCD_WriteByte(bData);
}


/* ------------------------------------------------------------ */
/***	LCD_InitSequence
**
**  Synopsis:
**              LCD_InitSequence(displaySetOptionDisplayOn);//set the display on
**
**	Parameters:
**		unsigned char bDisplaySetOptions -  display options
**					Possible options (to be OR-ed)
**						displaySetOptionDisplayOn - display ON
**						displaySetOptionCursorOn - cursor ON
**						displaySetBlinkOn - cursor blink ON
**
**	Return Value:
**		
**
**	Description:
**		This function performs the initializing (startup) sequence. 
**      The LCD is initialized according to the parameter bDisplaySetOptions. 
**      
**          
*/
void LCD_InitSequence(unsigned char bDisplaySetOptions)
{

    IFS0bits.T5IF = 0;                  //    clear interrupt flag
    IEC0bits.T5IE = 0;                  //    enable interrupt 

	//	wait 40 ms

	delay39us(1025);
	// Function Set
	LCD_WriteCommand(cmdLcdFcnInit);
	// Wait ~100 us
	//delay39us(10);
	// Function Set
	LCD_WriteCommand(cmdLcdCtlInit);
	// Wait ~100 us
	//delay39us(10);
//	LCD_DisplaySet(bDisplaySetOptions);
	// Wait ~100 us
	//delay39us(2);
	// Display Clear
	LCD_DisplayClear();
	// Wait 1.52 ms
	//delay39us(40);
    // Entry mode set
	LCD_WriteCommand(cmdLcdEntryMode);
    	// Wait 1.52 ms
	//delay39us(40);
}

/* ------------------------------------------------------------ */
/***	LCD_DisplaySet
**
**  Synopsis:
**				LCD_DisplaySet(displaySetOptionDisplayOn | displaySetOptionCursorOn);
**
**	Parameters:
**		unsigned char bDisplaySetOptions -  display options
**					Possible options (to be OR-ed)
**						displaySetOptionDisplayOn - display ON
**						displaySetOptionCursorOn - cursor ON
**						displaySetBlinkOn - cursor blink ON
**
**	Return Value:
**		
**
**	Description:
**      The LCD is initialized according to the parameter bDisplaySetOptions. 
**      If one of the above mentioned optios is not OR-ed, 
**      it means that the OFF action is performed for it.
**      
**          
*/
void LCD_DisplaySet(unsigned char bDisplaySetOptions)
{
	LCD_WriteCommand(cmdLcdCtlInit | bDisplaySetOptions);
}

/* ------------------------------------------------------------ */
/***	LCD_DisplayClear
**
**	Parameters:
**
**	Return Value:
**		
**	Description:
**      Clears the display and returns the cursor home (upper left corner, position 0 on row 0). 
**      
**          
*/
void LCD_DisplayClear()
{
	LCD_WriteCommand(cmdLcdClear);
    delay39us(40);
}

/* ------------------------------------------------------------ */
/***	LCD_ReturnHome
**
**
**	Parameters:
**
**	Return Value:
**		
**	Description:
**      Returns the cursor home (upper left corner, position 0 on row 0). 
**      
**          
*/
void LCD_ReturnHome()
{
	LCD_WriteCommand(cmdLcdRetHome);
    //delay39us(40);
}

/* ------------------------------------------------------------ */
/***	LCD_DisplayShift
**	Parameters:
**		unsigned char fRight - specifies display shift direction:
**				- 1 in order to shift right
**				- 0 in order to shift left
**
**	Return Value:
**		
**	Description:
**     Shifts the display one position right or left, depending on the fRight parameter.
**      
**          
*/
void LCD_DisplayShift(unsigned char fRight)
{
	unsigned char bCmd = cmdLcdDisplayShift | (fRight ? mskShiftRL: 0);
	LCD_WriteCommand(bCmd);
}

/* ------------------------------------------------------------ */
/***	LCD_CursorShift
**
**	Parameters:
**		unsigned char fRight
**				- 1 in order to shift right
**				- 0 in order to shift left
**
**	Return Value:
**		
**	Description:
**     Shifts the cursor one position right or left, depending on the fRight parameter.
**      
**          
*/
void LCD_CursorShift(unsigned char fRight)
{
	unsigned char bCmd = cmdLcdCursorShift | (fRight ? mskShiftRL: 0);
	LCD_WriteCommand(bCmd);
}

void LCD_CLEAR() {
    char vide[] = "                 ";
    LCD_WriteStringAtPos(vide, 0, 0);  // effacer LCD ligne du haut
    LCD_WriteStringAtPos(vide, 1, 0);  // effacer LCD ligne du bas
}

/* ------------------------------------------------------------ */
/***	LCD_WriteStringAtPos
**
**  Synopsis:
**      LCD_WriteStringAtPos("Demo", 0, 0);
**
**	Parameters:
**      char *szLn	- string to be written to LCD
**		int idxLine	- line where the string will be displayed
**          0 - first line of LCD
**          1 - second line of LCD
**		unsigned char idxPos - the starting position of the string within the line. 
**                                  The value must be between:
**                                      0 - first position from left
**                                      39 - last position for DDRAM for one line
**                                  
**
**	Return Value:
**		
**	Description:
**		Displays the specified string at the specified position on the specified line. 
**		It sets the corresponding write position and then writes data bytes when the device is ready.
**      Strings longer than 40 characters are trimmed. 
**      It is possible that not all the characters will be visualized, as the display only visualizes 16 characters for one line.
**      
**          
*/
void LCD_WriteStringAtPos(char *szLn, unsigned char idxLine, unsigned char idxPos)
{
	// crop string to 0x27 chars
	int len = strlen(szLn);
	if(len > 16)
	{
        //szLn[16] = 0; // trim the string so it contains 32 characters 
		len = 16;
	}

	// Set write position
	unsigned char bAddrOffset = (idxLine == 0 ? 0: 0x40) + idxPos;
	LCD_SetWriteDdramPosition(bAddrOffset);
    delay39us(4);

	unsigned char bIdx = 0;
	while(bIdx < len)
	{
		LCD_WriteDataByte(szLn[bIdx]);
        delay39us(2);
        //DelayAprox10Us(10 );
		bIdx++;
	}
}

void LCD_seconde(unsigned int seconde) {
    LCD_WriteIntAtPos(seconde%60, 3, 0, 13, 0);  // affichage des secondes
    LCD_WriteStringAtPos(":", 0, 13);
    LCD_WriteIntAtPos(seconde/60%60, 3, 0, 10, 0);  // affichage des minutes
    LCD_WriteStringAtPos(":", 0, 10);
    LCD_WriteIntAtPos(seconde/3600%24, 3, 0, 7, 0);  // affichage des heures
}


void int2char(int value, char * text, int nbDigit, int erase) {
    if (value + abs(value) == 0 && value != 0) {
        text[0] = '-';
    }
    else {
        text[0] = ' ';
    }
    value = abs(value);
    
    int i = 0;
    int div = 10;
    
    // pour les unit�es
    text[nbDigit-i-1] = (char) (value % div) + '0'; // extrait le digit
    value = value / 10;   // divise par 10 et arrondie a la baisse.
    
    // pour les autres digits
    for (i=1; i<nbDigit-1; i++) {
        if (value == 0 && erase == 1){
                text[nbDigit-i-1] = ' ';
        }
        else {
            text[nbDigit-i-1] = (char) (value % div) + '0'; // extrait le digit
        }
        value = value / 10;   // divise par 10 et arrondie a la baisse.
    }
}

void LCD_WriteIntAtPos(int value, int nbDigit, unsigned char idxLine, unsigned char idxPos, int erase) {
    // valeur  => ce qui sera affich�
    // nbDigit => nombre de chiffre � afficher
    // erase   => permet d'effacer les z�ros devant le nombre
    
    char text[nbDigit];
    int2char(value, text, nbDigit, erase);
    
	if(nbDigit > 16) {
        text[16] = 0; // trim the string so it contains 32 characters 
		nbDigit = 16;
	}

	// Set write position
	unsigned char bAddrOffset = (idxLine == 0 ? 0: 0x40) + idxPos;
	LCD_SetWriteDdramPosition(bAddrOffset);
    delay39us(4);

	unsigned char bIdx = 0;
	while(bIdx < nbDigit) {
		LCD_WriteDataByte(text[bIdx]);
        delay39us(2);
        //DelayAprox10Us(10);
		bIdx++;
	}
}

void LCD_Task(int Temperature, int humidite, int lumiere, int compteur)
{
   /* char temperature2[4] = (char)Temperature;
    char humidite2[4] = (char)humidite;
    char lumiere2[4] = (char)lumiere;
    */
    
        char temp1[30];
        
        LCD_seconde(compteur);
        
        sprintf(temp1,"Temp:%i",Temperature);
        LCD_WriteStringAtPos(temp1,1,0);

        sprintf(temp1,"Hum:%i",humidite);
        LCD_WriteStringAtPos(temp1,1,9);

      //  sprintf(temp1,"Luminosite:%i",lumiere);
       // LCD_WriteStringAtPos(temp1,1,0);
    
        LCD_seconde(compteur);
    
    
        
}
/* ------------------------------------------------------------ */
/***	LCD_SetWriteCgramPosition
**
**
**	Parameters:
**      unsigned char bAdr	- the write location. The position in CGRAM where the next data write operations will put bytes.
**
**	Return Value:
**		
**	Description:
**		Sets the DDRAM write position. This is the location where the next data write operation will be performed.
**		Writing to a location auto-increments the write location.
**      This is a low-level function called by LCD_WriteBytesAtPosCgram(), so user should avoid calling it directly.
 **      
**          
*/
void LCD_SetWriteCgramPosition(unsigned char bAdr)
{
	unsigned char bCmd = cmdLcdSetCgramPos | bAdr;
	LCD_WriteCommand(bCmd);
}


void LCD_SetWriteDdramPosition(unsigned char bAddr)
{
	LCD_WriteCommand(cmdLcdSetDdramPos | bAddr);
}




/* ------------------------------------------------------------ */
/***	LCD_WriteBytesAtPosCgram
**
**  Synopsis:
**      LCD_WriteBytesAtPosCgram(userDefArrow, 8, posCgramChar0);
**
**	Parameters:
**		unsigned char *pBytes	- pointer to the string of bytes
**		unsigned char len		- the number of bytes to be written
**		unsigned char bAdr		- the position in CGRAM where bytes will be written
**
**	Return Value:
**		
**	Description:
**		Writes the specified number of bytes to CGRAM starting at the specified position. 
**      This allows user characters to be defined.
**		It sets the corresponding write position and then writes data bytes when the device is ready.
**      
**          
*/
void LCD_WriteBytesAtPosCgram(unsigned char *pBytes, unsigned char len, unsigned char bAdr)
{
	// Set write position
	LCD_SetWriteCgramPosition(bAdr);

	// Write the string of bytes that define the character to CGRAM
	unsigned char idx = 0;
	while(idx < len)
	{
		LCD_WriteDataByte(pBytes[idx]);
		idx++;
	}
}

/* *****************************************************************************
 End of File
 */

