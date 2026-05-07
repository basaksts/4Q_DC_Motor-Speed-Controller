/*
 * encoder_rpm.c
 *
 *  Created on: May 7, 2026
 *      Author: 90546
 */


#include "encoder_rpm.h"

static volatile uint32_t encoder_pulse_count = 0;
static uint32_t last_update_ms = 0;
static float motor_rpm = 0.0f;

void EncoderRPM_Init(void)
{
    encoder_pulse_count = 0;
    motor_rpm = 0.0f;
    last_update_ms = HAL_GetTick();
}

/*
 * Bu fonksiyon her encoder pulse'ında çağrılır.
 * EXTI interrupt callback içinden çağıracağız.
 */
void EncoderRPM_OnPulse(void)
{
    encoder_pulse_count++;
}

/*
 * Belirli aralıklarla pulse sayısını RPM'e çevirir.
 *
 * RPM = pulse_count * 60000 / (PPR * sample_time_ms)
 *
 * Örneğin:
 * sample_time = 100 ms
 * PPR = 20
 * pulse_count = 50
 *
 * RPM = 50 * 60000 / (20 * 100)
 * RPM = 1500
 */
void EncoderRPM_Update(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t elapsed_ms = now - last_update_ms;

    if (elapsed_ms >= RPM_SAMPLE_TIME_MS)
    {
        uint32_t pulses = encoder_pulse_count;
        encoder_pulse_count = 0;

        motor_rpm = ((float)pulses * 60000.0f) / (ENCODER_PPR * (float)elapsed_ms);

        last_update_ms = now;
    }
}

float EncoderRPM_GetRPM(void)
{
    return motor_rpm;
}

uint32_t EncoderRPM_GetPulseCount(void)
{
    return encoder_pulse_count;
}
