/*
 * adc_measure.c
 *
 *  Created on: May 7, 2026
 *      Author: 90546
 */


#include "adc_measure.h"

static ADC_HandleTypeDef *m_hadc = NULL;

void ADCMeasure_Init(ADC_HandleTypeDef *hadc)
{
    m_hadc = hadc;
}

/*
 * İstenen ADC kanalını tek seferlik okur.
 *
 * Neden kanal seçiyoruz?
 * Çünkü aynı ADC1 modülüyle PA0, PA3, PA4, PA5 gibi farklı pinleri okuyacağız.
 * Her okumadan önce hangi kanalı okuyacağımızı HAL_ADC_ConfigChannel ile belirliyoruz.
 */
uint16_t ADCMeasure_ReadRawChannel(uint32_t channel)
{
    if (m_hadc == NULL) {
        return 0;
    }

    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;

    if (HAL_ADC_ConfigChannel(m_hadc, &sConfig) != HAL_OK) {
        return 0;
    }

    HAL_ADC_Start(m_hadc);

    if (HAL_ADC_PollForConversion(m_hadc, 10) != HAL_OK) {
        HAL_ADC_Stop(m_hadc);
        return 0;
    }

    uint16_t value = (uint16_t)HAL_ADC_GetValue(m_hadc);

    HAL_ADC_Stop(m_hadc);

    return value;
}

/*
 * PA0 / ADC_IN0 potansiyometre okuması.
 */
uint16_t ADCMeasure_ReadPotRaw(void)
{
    return ADCMeasure_ReadRawChannel(ADC_CHANNEL_0);
}

/*
 * Potansiyometreyi yüzde olarak verir.
 *
 * ADC 0    -> 0%
 * ADC 4095 -> 100%
 */
float ADCMeasure_GetPotPercent(void)
{
    uint16_t raw = ADCMeasure_ReadPotRaw();

    return ((float)raw / ADC_MAX_COUNT) * 100.0f;
}
