/*
 * oled_app.c
 *
 *  Created on: May 11, 2026
 *      Author: 90546
 */


#include "oled_app.h"
#include "oled_ssd1306.h"
#include <stdio.h>

void OLED_App_Init(I2C_HandleTypeDef *hi2c)
{
#if OLED_APP_ENABLED
    OLED_SSD1306_Init(hi2c);

    OLED_SSD1306_Fill(0);

    OLED_SSD1306_SetCursor(0, 0);
    OLED_SSD1306_WriteString("DC MOTOR CTRL");

    OLED_SSD1306_SetCursor(0, 12);
    OLED_SSD1306_WriteString("STM32 READY");

    OLED_SSD1306_SetCursor(0, 24);
    OLED_SSD1306_WriteString("BTS7960 MODE");

    OLED_SSD1306_UpdateScreen();

    HAL_Delay(1000);
#else
    (void)hi2c;
#endif
}

void OLED_App_UpdateMainScreen(uint16_t pwm_percent,
                               int16_t rpm,
                               float current_A,
                               const char *direction,
                               const char *status)
{
#if OLED_APP_ENABLED
    char line[24];
    int16_t current_mA = (int16_t)(current_A * 1000.0f);

    OLED_SSD1306_Fill(0);

    OLED_SSD1306_SetCursor(0, 0);
    snprintf(line, sizeof(line), "STATUS:%s", status);
    OLED_SSD1306_WriteString(line);

    OLED_SSD1306_SetCursor(0, 12);
    snprintf(line, sizeof(line), "PWM:%u%%", pwm_percent);
    OLED_SSD1306_WriteString(line);

    OLED_SSD1306_SetCursor(0, 24);
    snprintf(line, sizeof(line), "RPM:%d", rpm);
    OLED_SSD1306_WriteString(line);

    OLED_SSD1306_SetCursor(0, 36);
    snprintf(line, sizeof(line), "I:%dmA", current_mA);
    OLED_SSD1306_WriteString(line);

    OLED_SSD1306_SetCursor(0, 48);
    snprintf(line, sizeof(line), "DIR:%s", direction);
    OLED_SSD1306_WriteString(line);

    OLED_SSD1306_UpdateScreen();
#else
    (void)pwm_percent;
    (void)rpm;
    (void)current_A;
    (void)direction;
    (void)status;
#endif
}
