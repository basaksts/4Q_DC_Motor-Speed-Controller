#ifndef INC_PWM_CONTROL_H_
#define INC_PWM_CONTROL_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

/*
 * Motor kontrol modülü
 *
 * Şu an geçici/test pin planı:
 * PA8 -> PWM çıkışı
 * PB0 -> yön kontrol pini 1
 * PB1 -> yön kontrol pini 2
 *
 * Not:
 * Nihai sistemde BTS7960 / IBT-2 kullanılacak.
 * BTS7960 için RPWM, LPWM, R_EN, L_EN pinleri motor netleşince
 * yeniden eşlenecek.
 *
 * Bu dosyadaki fonksiyon isimleri genel tutulmuştur:
 * Motor_Init()
 * Motor_Set_Speed()
 * Motor_Emergency_Stop()
 */

#define MOTOR_PWM_MAX_DUTY   3599U
#define MOTOR_PWM_SAFE_MAX   3420U   // Yaklaşık %95 duty, güvenlik sınırı

typedef enum {
    MOTOR_STOP = 0,
    MOTOR_CW,
    MOTOR_CCW
} MotorDir_t;

void Motor_Init(TIM_HandleTypeDef *htim, uint32_t channel);

// Verilen duty (0-3599) ve yöne göre motoru sürer.
void Motor_Set_Speed(uint16_t duty, MotorDir_t dir);

// Acil durumlarda (Overcurrent vb.) PWM'i anında keser ve pinleri RESET'e çeker.
void Motor_Emergency_Stop(void);
void Motor_Coast(void);
void Motor_Brake(void);

#endif /* INC_PWM_CONTROL_H_ */
