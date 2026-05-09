/*
 * uart_debug.c
 *
 *  Created on: May 5, 2026
 *      Author: 90546
 */


#include "uart_debug.h"
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef *debug_uart = NULL;

void UART_Debug_Init(UART_HandleTypeDef *huart)
{
    debug_uart = huart;
}

void UART_Debug_Print(const char *message)
{
    if (debug_uart == NULL || message == NULL)
    {
        return;
    }

    HAL_UART_Transmit(debug_uart, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
}

void UART_Debug_PrintMotorData(int duty, int encoder_count, int direction)
{
    char buffer[100];

    snprintf(buffer, sizeof(buffer),
             "Duty: %d%% | Encoder: %d | Direction: %d\r\n",
             duty,
             encoder_count,
             direction);

    UART_Debug_Print(buffer);
}
