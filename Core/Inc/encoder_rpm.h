/*
 * encoder_rpm.h
 *
 *  Created on: May 7, 2026
 *      Author: 90546
 */

#ifndef INC_ENCODER_RPM_H_
#define INC_ENCODER_RPM_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

/*
 * Encoder RPM ölçüm modülü
 *
 * Güncel pin planı:
 * PA1 -> Encoder A pulse input / EXTI1
 * PA2 -> Encoder B input, opsiyonel
 *
 * Şimdilik sadece Encoder A kanalındaki rising edge pulse'ları sayıyoruz.
 */

#define ENCODER_PPR             20.0f   // Motor datasheet'e göre düzeltilecek
#define RPM_SAMPLE_TIME_MS      100U

void EncoderRPM_Init(void);
void EncoderRPM_OnPulse(void);
void EncoderRPM_Update(void);

float EncoderRPM_GetRPM(void);
uint32_t EncoderRPM_GetPulseCount(void);

#endif /* INC_ENCODER_RPM_H_ */
