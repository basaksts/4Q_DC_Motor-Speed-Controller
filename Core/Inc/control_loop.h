#ifndef INC_CONTROL_LOOP_H_
#define INC_CONTROL_LOOP_H_

#include "pwm_control.h"

// Ramp Ayarları (Hızlanma ve Yavaşlama İçin)
#define RAMP_STEP 5         // Her güncellemede duty miktarındaki değişim
#define RAMP_INTERVAL_MS 10  // Rampanın güncellenme periyodu (ms)

// Test #5 Sabitleri (4Q Otomatik Yön Değişimi)
#define TEST_4Q_INTERVAL_MS 500 // 500ms yön değiştirme süresi

// Fonksiyon Prototipleri
void Control_Init(void);
void Control_Set_Target(uint16_t target_duty, MotorDir_t target_dir);
void Control_Update_Ramp(void);
void Control_Run_4Q_Test(uint32_t current_tick);

#endif /* INC_CONTROL_LOOP_H_ */
