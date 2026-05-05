#ifndef INC_PWM_CONTROL_H_
#define INC_PWM_CONTROL_H_

#include "stm32f1xx_hal.h"

// Motor Yönleri
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_CW,   // Saat yönü
    MOTOR_CCW   // Saat yönünün tersi
} MotorDir_t;

// Fonksiyonlar
void Motor_Init(TIM_HandleTypeDef *htim, uint32_t channel);
void Motor_Set_Speed(uint16_t duty, MotorDir_t dir);
void Motor_Emergency_Stop(void);

void Motor_Enable(void);
void Motor_Disable(void);

#endif
