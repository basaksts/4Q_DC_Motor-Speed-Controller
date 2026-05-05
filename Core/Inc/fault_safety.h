#ifndef INC_FAULT_SAFETY_H_
#define INC_FAULT_SAFETY_H_

#include "stm32f1xx_hal.h"
#include "pwm_control.h"

// Akım Sınırı Tanımları
#define CURRENT_LIMIT_THRESHOLD 1.0f  // 1 Amper sınırı [cite: 16]
#define ADC_MAX_VALUE           4095  // 12-bit ADC için
#define SENSOR_SENSITIVITY      0.185f // ACS712-5A sensörü için tipik değer (V/A)

// Fonksiyon Prototipleri
void Safety_Init(ADC_HandleTypeDef *hadc);
void Safety_Process(void);
float Get_Motor_Current(void);

#endif
