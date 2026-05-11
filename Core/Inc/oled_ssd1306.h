/*
 * oled_ssd1306.h
 *
 *  Created on: May 11, 2026
 *      Author: 90546
 */

#ifndef INC_OLED_SSD1306_H_
#define INC_OLED_SSD1306_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define OLED_WIDTH   128
#define OLED_HEIGHT  64

void OLED_SSD1306_Init(I2C_HandleTypeDef *hi2c);
void OLED_SSD1306_Fill(uint8_t color);
void OLED_SSD1306_UpdateScreen(void);
void OLED_SSD1306_SetCursor(uint8_t x, uint8_t y);
void OLED_SSD1306_WriteString(const char *str);
void OLED_SSD1306_WriteChar(char ch);

#endif /* INC_OLED_SSD1306_H_ */
