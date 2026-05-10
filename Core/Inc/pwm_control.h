#ifndef INC_PWM_CONTROL_H_
#define INC_PWM_CONTROL_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define MOTOR_PWM_MAX_DUTY   3599U
#define MOTOR_PWM_SAFE_MAX   3420U

typedef enum
{
    MOTOR_STOP = 0,
    MOTOR_CW,
    MOTOR_CCW
} MotorDirection_t;

/*
 * BTS7960 / IBT-2 motor driver control
 *
 * RPWM -> ileri yön PWM
 * LPWM -> geri yön PWM
 * R_EN, L_EN -> enable pinleri
 */
void Motor_Init(TIM_HandleTypeDef *htim,
                uint32_t rpwm_channel,
                uint32_t lpwm_channel);

void Motor_Set_Speed(uint16_t duty, MotorDirection_t direction);
void Motor_Emergency_Stop(void);
void Motor_Coast(void);
void Motor_Brake(void);

#endif /* INC_PWM_CONTROL_H_ */
