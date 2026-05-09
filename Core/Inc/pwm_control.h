#ifndef INC_PWM_CONTROL_H_
#define INC_PWM_CONTROL_H_

#include "stm32f1xx_hal.h"

/* * Başak'ın TIM1 ayarlarında ARR değeri 3599 olarak belirlenmiş.
 * Bu yüzden Duty Cycle için maksimum değerimiz 3599'dur.
 */
#define MOTOR_PWM_SAFE_MAX    3599U

// Motor Yönleri - Başak'ın main.c içerisindeki MotorDir_t ile uyumlu
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_CW,   // Clockwise (Saat yönü)
    MOTOR_CCW   // Counter-Clockwise (Saat yönünün tersi)
} MotorDir_t;

/* * Fonksiyonlar:
 * Başak'ın main.c dosyasında bu fonksiyonların tamamı aktif olarak çağrılıyor.
 */

// Sistemi başlatır, Timer ve Kanal bilgisini kaydeder.
void Motor_Init(TIM_HandleTypeDef *htim, uint32_t channel);

// Verilen duty (0-3599) ve yöne göre motoru sürer.
void Motor_Set_Speed(uint16_t duty, MotorDir_t dir);

// Acil durumlarda (Overcurrent vb.) PWM'i anında keser ve pinleri RESET'e çeker.
void Motor_Emergency_Stop(void);

// Sürücüyü aktif/pasif hale getirmek için (opsiyonel)
void Motor_Enable(void);
void Motor_Disable(void);

#endif /* INC_PWM_CONTROL_H_ */
