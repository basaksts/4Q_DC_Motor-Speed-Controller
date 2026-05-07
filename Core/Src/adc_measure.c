/*
 * adc_measure.c
 *
 *  Created on: May 7, 2026
 *      Author: 90546
 */


#include "adc_measure.h"

static ADC_HandleTypeDef *m_hadc = NULL;
static uint16_t current_zero_raw = 0;

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

uint16_t ADCMeasure_ReadCurrentRaw(void)
{
    return ADCMeasure_ReadRawChannel(ADC_CHANNEL_3);
}

void ADCMeasure_CalibrateCurrentZero(void)
{
    uint32_t sum = 0;
    const uint16_t sample_count = 100;

    for (uint16_t i = 0; i < sample_count; i++)
    {
        sum += ADCMeasure_ReadCurrentRaw();
        HAL_Delay(2);
    }

    current_zero_raw = (uint16_t)(sum / sample_count);
}

float ADCMeasure_GetMotorCurrentA(void)
{
    uint16_t raw = ADCMeasure_ReadCurrentRaw();

    /*
     * ADC raw değerini voltaja çeviriyoruz.
     * Bu voltaj STM32 ADC pinindeki voltajdır.
     */
    float adc_voltage = ((float)raw / ADC_MAX_COUNT) * ADC_REF_VOLTAGE;

    /*
     * Sıfır akım noktasını da voltaja çeviriyoruz.
     */
    float zero_voltage = ((float)current_zero_raw / ADC_MAX_COUNT) * ADC_REF_VOLTAGE;

    /*
     * ACS712 çıkışı STM32'ye gelmeden önce direnç bölücüden geçtiyse
     * ADC tarafındaki farkı gerçek ACS712 çıkış farkına geri ölçekliyoruz.
     */
    float sensor_voltage_diff = (adc_voltage - zero_voltage) * ACS712_ADC_DIVIDER_GAIN;

    /*
     * Akım = gerilim farkı / hassasiyet
     * ACS712 5A için hassasiyet yaklaşık 0.185 V/A.
     */
    float current_A = sensor_voltage_diff / ACS712_SENSITIVITY_V_PER_A;

    return current_A;
}

uint16_t ADCMeasure_ReadVout1Raw(void)
{
    return ADCMeasure_ReadRawChannel(ADC_CHANNEL_4);
}

uint16_t ADCMeasure_ReadVout2Raw(void)
{
    return ADCMeasure_ReadRawChannel(ADC_CHANNEL_5);
}

float ADCMeasure_GetVout1Voltage(void)
{
    uint16_t raw = ADCMeasure_ReadVout1Raw();

    float adc_voltage = ((float)raw / ADC_MAX_COUNT) * ADC_REF_VOLTAGE;

    return adc_voltage * MOTOR_VOLTAGE_DIVIDER_GAIN;
}

float ADCMeasure_GetVout2Voltage(void)
{
    uint16_t raw = ADCMeasure_ReadVout2Raw();

    float adc_voltage = ((float)raw / ADC_MAX_COUNT) * ADC_REF_VOLTAGE;

    return adc_voltage * MOTOR_VOLTAGE_DIVIDER_GAIN;
}

float ADCMeasure_GetMotorVoltage(void)
{
    float vout1 = ADCMeasure_GetVout1Voltage();
    float vout2 = ADCMeasure_GetVout2Voltage();

    float vmotor = vout1 - vout2;

    /*
     * OLED’de büyüklük göstermek için mutlak değer kullanıyoruz.
     * Yön bilgisi zaten current_direction değişkeninden geliyor.
     */
    if (vmotor < 0.0f)
    {
        vmotor = -vmotor;
    }

    return vmotor;
}
