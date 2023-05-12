#ifndef DFRobot_LCD_H
#define DFRobot_LCD_H

#include <inttypes.h>
//#include "Print.h"

/*!
 *  @brief Device I2C Arress
 */
#define LCD_ADDRESS     (0x7c>>1)
#define RGB_ADDRESS     (0xc0>>1)

/*!
 *  @brief color define
 */ 
#define WHITE           0
#define RED             1
#define GREEN           2
#define BLUE            3
#define ONLY            3

#define REG_RED         0x04        // pwm2
#define REG_GREEN       0x03        // pwm1
#define REG_BLUE        0x02        // pwm0
#define REG_ONLY        0x02

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT	0x08

/*!
 *  @brief commands
 */
#define LCD_CLEARDISPLAY 	0x01
#define LCD_RETURNHOME 		0x02
#define LCD_ENTRYMODESET 	0x04
#define LCD_DISPLAYCONTROL 	0x08
#define LCD_CURSORSHIFT 	0x10
#define LCD_FUNCTIONSET 	0x20
#define LCD_SETCGRAMADDR 	0x40
#define LCD_SETDDRAMADDR 	0x80

/*!
 *  @brief flags for display entry mode
 */
#define LCD_ENTRYRIGHT 		  0x00
#define LCD_ENTRYLEFT 		  0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/*!
 *  @brief flags for display on/off control
 */
#define LCD_DISPLAYON 	0x04
#define LCD_DISPLAYOFF 	0x00
#define LCD_CURSORON 	0x02
#define LCD_CURSOROFF 	0x00
#define LCD_BLINKON 	0x01
#define LCD_BLINKOFF 	0x00

/*!
 *  @brief flags for display/cursor shift
 */
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 	0x00
#define LCD_MOVERIGHT 	0x04
#define LCD_MOVELEFT 	0x00

/*!
 *  @brief flags for function set
 */
#define LCD_8BITMODE 	0x10
#define LCD_4BITMODE 	0x00
#define LCD_2LINE 	0x08
#define LCD_1LINE 	0x00
#define LCD_5x10DOTS 	0x04
#define LCD_5x8DOTS 	0x00

/*
 *  @brief initialize
 */ 

void init();
  
void clear();
void home();

/*
 *  @brief Turn the display on/off (quickly)
 */
  
void noDisplay();
void display();

/*
 *  @brief Turn on and off the blinking showCursor
 */
  
void stopBlink();
void blink();

/*
 *  @brief Turns the underline showCursor on/off
 */
  
void noCursor();
void cursor();

/*
 *  @brief These commands scroll the display without changing the RAM
 */
  
void scrollDisplayLeft();
void scrollDisplayRight();
 
/*
 *  @brief This is for text that flows Left to Right
 */
 
void leftToRight();
 
/*
 *  @brief This is for text that flows Right to Left
 */
  
void rightToLeft();

/*
 *  @brief This will 'left justify' text from the showCursor
 */
  
void noAutoscroll();
 
/*
 *  @brief This will 'right justify' text from the showCursor
 */
  
void autoscroll();
   
/*
 *  @brief Allows us to fill the first 8 CGRAM locations
 *		 with custom characters
 */
  
void customSymbol(uint8_t, uint8_t[]);
void setCursor(uint8_t, uint8_t);  
  
/*
 *  @brief color control
 */

void setReg(uint8_t addr, uint8_t data);
void setRGB(uint8_t r, uint8_t g, uint8_t b);               // set rgb
void setPWM(uint8_t color, uint8_t pwm);      // set pwm 
void setColor(uint8_t color);
void setColorAll();
void setColorWhite();

/*
 *  @brief blink the LED backlight
 */
  
void blinkLED(void);
void noBlinkLED(void);

/*
 *  @brief send data
 */
  
void write(uint8_t);

/*
 *  @brief send command
 */
  
void command(uint8_t);
  
/*
 *  @brief compatibility API function aliases
 */
  
void blink_on();									// alias for blink()
void blink_off();       							// alias for noBlink()
void cursor_on();      	 							// alias for cursor()
void cursor_off();      							// alias for noCursor()
void setBacklight(uint8_t new_val);						// alias for backlight() and nobacklight()
void load_custom_character(uint8_t char_num, uint8_t *rows);	// alias for createChar()
void printstr(const char[]);
  
void print(const char c[]);
  
void begin(uint8_t cols, uint8_t rows, uint8_t charsize);
void send(uint8_t *data, uint8_t len);

#endif
