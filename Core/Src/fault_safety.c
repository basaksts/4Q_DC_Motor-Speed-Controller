#include "fault_safety.h"

static ADC_HandleTypeDef *m_hadc;
static uint8_t is_fault_active = 0;

void Safety_Init(ADC_HandleTypeDef *hadc) {
    m_hadc = hadc;
}

// Akımı Okuma ve Amper'e Dönüştürme
float Get_Motor_Current(void) {
    uint32_t adc_val = 0;
    HAL_ADC_Start(m_hadc);
    if (HAL_ADC_PollForConversion(m_hadc, 10) == HAL_OK) {
        adc_val = HAL_ADC_GetValue(m_hadc);
    }
    HAL_ADC_Stop(m_hadc);

    // ADC değerini gerilime, gerilimi akıma dönüştürme (Hall Effect mantığı)
    float voltage = (adc_val * 3.3f) / ADC_MAX_VALUE;
    float current = (voltage - 2.5f) / SENSOR_SENSITIVITY; // 2.5V ofset ACS712 için örnektir

    return (current < 0) ? -current : current; // Mutlak değer alıyoruz
}

// Ana Güvenlik Döngüsü (Her döngüde çağrılmalı)
void Safety_Process(void) {
    float current_mA = Get_Motor_Current();

    if (current_mA > CURRENT_LIMIT_THRESHOLD) {
        // HATA: Akım 1A'i geçti! [cite: 17]
        Motor_Emergency_Stop();
        is_fault_active = 1;
    }
    else if (is_fault_active && (current_mA < CURRENT_LIMIT_THRESHOLD)) {
        // NORMAL: Akım tekrar güvenli seviyede, otomatik dön
        is_fault_active = 0;
        // Motor son kaldığı hızda veya güvenli düşük hızda başlatılabilir
    }
}
