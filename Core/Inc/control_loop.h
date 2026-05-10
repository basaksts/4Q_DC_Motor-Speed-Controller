#ifndef INC_CONTROL_LOOP_H_
#define INC_CONTROL_LOOP_H_

#include "pwm_control.h"

/* * Başak'ın main.c dosyasındaki değerlerle senkronize edildi.
 */
#define RAMP_STEP            30U   // Başak'ın DUTY_RAMP_STEP değeri
#define CONTROL_INTERVAL_MS  20U   // Başak'ın CONTROL_DELAY_MS değeri
#define TEST_SWITCH_MS       500U  // Yön değiştirme periyodu

/* * 4Q Test Modu için State Machine (Durum Makinesi) yapısı.
 * Bu yapıyı buraya taşıyarak main.c'deki kalabalığı önlüyoruz.
 */
typedef enum
{
    TEST_STATE_RUN = 0,
    TEST_STATE_RAMP_DOWN,
    TEST_STATE_CHANGE_DIR
} TestState_t;

// Fonksiyon Prototipleri
void Control_Init(void);

/* * Motorun gitmesini istediğimiz hedef değerleri ayarlar.
 * Bu değerlere rampa ile yavaşça çıkılır.
 */
void Control_Set_Target(uint16_t target_duty, MotorDirection_t target_dir);

/* * Her 20ms'de bir çağrılmalı.
 * Mevcut hızı hedefe yaklaştırır ve motoru sürer.
 */
void Control_Update(void);

/* * 4Q test modunu yönetir.
 * Aktifse otomatik olarak rampayı ve yön değişimini kontrol eder.
 */
void Control_Run_4Q_Test(uint8_t enable);

#endif /* INC_CONTROL_LOOP_H_ */
