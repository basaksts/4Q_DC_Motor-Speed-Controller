/*
 * oled_app.h
 *
 *  Created on: May 11, 2026
 *      Author: 90546
 */
#ifndef INC_OLED_APP_H_
#define INC_OLED_APP_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

/*
 * OLED ekran fiziksel olarak bağlı değilken 0 yapılabilir.
 * OLED bağlıyken 1 yap.
 */
#define OLED_APP_ENABLED  1

void OLED_App_Init(I2C_HandleTypeDef *hi2c);
void OLED_App_UpdateMainScreen(uint16_t pwm_percent,
                               int16_t rpm,
                               float current_A,
                               const char *direction,
                               const char *status);

#endif /* INC_OLED_APP_H_ */
