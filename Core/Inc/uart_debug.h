#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include "main.h"

void UART_Debug_Init(UART_HandleTypeDef *huart);
void UART_Debug_Print(const char *message);
void UART_Debug_PrintMotorData(int duty, int encoder_count, int direction);

#endif
