#ifndef INC_PWM_CONTROL_H_
#define INC_PWM_CONTROL_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

/*
 * Bu modül L298N motor sürücü katını kontrol eder.
 *
 * Güncel pin planı:
 * PA8  -> L298N ENA / PWM
 * PB0  -> L298N IN1
 * PB1  -> L298N IN2
 *
 * Not:
 * PA8 PWM pini TIM1_CH1 tarafından sürülür.
 * PB0 ve PB1 GPIO output olarak yön kontrolünde kullanılır.
 */

#define MOTOR_PWM_MAX_DUTY   3599U
#define MOTOR_PWM_SAFE_MAX   3420U   // Yaklaşık %95 duty, güvenlik sınırı

typedef enum {
    MOTOR_STOP = 0,
    MOTOR_CW,
    MOTOR_CCW
} MotorDir_t;

void Motor_Init(TIM_HandleTypeDef *htim, uint32_t channel);
void Motor_Set_Speed(uint16_t duty, MotorDir_t dir);
void Motor_Emergency_Stop(void);
void Motor_Coast(void);
void Motor_Brake(void);

#endif /* INC_PWM_CONTROL_H_ */
