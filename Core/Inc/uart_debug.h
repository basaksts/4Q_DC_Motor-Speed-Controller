#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include "main.h"
#include <stdint.h>

void UART_Debug_Init(UART_HandleTypeDef *huart);
void UART_Debug_Print(const char *message);
void UART_Debug_PrintMotorData(int duty, int encoder_count, int direction);
void UART_Debug_PrintSystemData(uint16_t pwm_percent,
                                int16_t rpm,
                                float current_A,
                                const char *direction,
                                const char *status);


#endif
