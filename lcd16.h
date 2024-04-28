#include <stdio.h>
#include <stdlib.h>

//Initialize LCD
void LCD_INIT();

//A7-A0 / D7-D0 in Decimal to binary
void LCD_DEC_TO_BIN(int dec);

//Clear LCD
void LCD_CLEAR();

//Enable command mode
void RS_CMD();

//Enable character mode
void RS_CHAR();

//Trigger enable
void EN_T();

//Print char on LCD
void LCD_CHAR_TO_BIN(char ch);

//String to binary / Print string to LCD
void LCD_STRING_TO_BIN(char chA[]);

//Print Float value
void LCD_FLOAT_TO_STRING(float decch);

//Line control, Print on Upper = 1, lower = 2
void LCD_LINE(int line);

//Print decimal value
void LCD_DEC_TO_STRING(int itst);

//Decimal to hexadecimal
void LCD_DEC_TO_HEX(int dec);