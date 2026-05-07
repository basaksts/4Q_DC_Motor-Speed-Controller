#ifndef INC_FAULT_SAFETY_H_
#define INC_FAULT_SAFETY_H_

#include "stm32f1xx_hal.h"
#include "pwm_control.h"

// Akım sınırı tanımları
#define CURRENT_LIMIT_THRESHOLD 1.0f    // 1 A kesme sınırı
#define CURRENT_RECOVERY_LEVEL  0.85f   // Akım normale dönünce tekrar çalışma sınırı
#define ADC_MAX_VALUE           4095.0f // 12-bit ADC
#define SENSOR_SENSITIVITY      0.185f  // ACS712-5A için yaklaşık V/A

void Safety_Init(ADC_HandleTypeDef *hadc);
void Safety_Process(void);
float Get_Motor_Current(void);

#endif /* INC_FAULT_SAFETY_H_ */
