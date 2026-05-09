/*
 * adc_measure.h
 *
 *  Created on: May 7, 2026
 *      Author: 90546
 */

#ifndef INC_ADC_MEASURE_H_
#define INC_ADC_MEASURE_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

/*
 * ADC ölçüm modülü
 *
 * Güncel pin planı:
 * PA0 -> ADC_IN0 -> Potansiyometre
 * PA3 -> ADC_IN3 -> ACS712 akım ölçümü
 * PA4 -> ADC_IN4 -> Motor OUT1 gerilim ölçümü
 * PA5 -> ADC_IN5 -> Motor OUT2 gerilim ölçümü
 */

#define ADC_MAX_COUNT       4095.0f
#define ADC_REF_VOLTAGE     3.3f
#define MOTOR_VOLTAGE_DIVIDER_GAIN  5.7f

#define ACS712_SENSITIVITY_V_PER_A   0.185f
#define ACS712_ADC_DIVIDER_GAIN      1.5f

void ADCMeasure_Init(ADC_HandleTypeDef *hadc);

uint16_t ADCMeasure_ReadRawChannel(uint32_t channel);

uint16_t ADCMeasure_ReadPotRaw(void);
float ADCMeasure_GetPotPercent(void);

uint16_t ADCMeasure_ReadCurrentRaw(void);
void ADCMeasure_CalibrateCurrentZero(void);
float ADCMeasure_GetMotorCurrentA(void);

uint16_t ADCMeasure_ReadVout1Raw(void);
uint16_t ADCMeasure_ReadVout2Raw(void);

float ADCMeasure_GetVout1Voltage(void);
float ADCMeasure_GetVout2Voltage(void);
float ADCMeasure_GetMotorVoltage(void);

#endif /* INC_ADC_MEASURE_H_ */
