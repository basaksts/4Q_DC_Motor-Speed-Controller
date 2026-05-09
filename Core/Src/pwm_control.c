#include "pwm_control.h"
#include "main.h" // Başak'ın IN1_Pin, IN2_Pin tanımlarını kullanmak için şart!

static TIM_HandleTypeDef *m_htim;
static uint32_t m_channel;

void Motor_Init(TIM_HandleTypeDef *htim, uint32_t channel) {
    m_htim = htim;
    m_channel = channel;

    // PWM sinyalini başlat
    HAL_TIM_PWM_Start(m_htim, m_channel);

    // Başlangıçta motoru durdur
    Motor_Emergency_Stop();
}

void Motor_Set_Speed(uint16_t duty, MotorDir_t dir) {
    // Güvenlik: Duty değerini Başak'ın ARR değerine (3599) göre sınırla
    if (duty > MOTOR_PWM_SAFE_MAX) {
        duty = MOTOR_PWM_SAFE_MAX;
    }

    if (dir == MOTOR_CW) {
        // IN1: High, IN2: Low (Başak'ın GPIOB tanımları ile)
        HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
    }
    else if (dir == MOTOR_CCW) {
        // IN1: Low, IN2: High
        HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_SET);
    }
    else {
        // MOTOR_STOP veya geçersiz yön durumunda durdur
        Motor_Emergency_Stop();
        return;
    }

    // PWM doluluk oranını güncelle
    __HAL_TIM_SET_COMPARE(m_htim, m_channel, duty);
}

void Motor_Emergency_Stop(void) {
    // PWM'i sıfırla
    __HAL_TIM_SET_COMPARE(m_htim, m_channel, 0);

    // Yön pinlerini RESET'e çekerek motoru "serbest duruşa" (Coast) al
    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
}

// Motoru Donanımsal Olarak Aktif Et
void Motor_Enable(void) {
    /* * NOT: Başak'ın main.c dosyasında PA0 potansiyometre için ayrılmış!
     * Eğer motor sürücünün (L298 vb.) bir EN bacağı varsa,
     * boşta kalan başka bir pin tanımlanmalı. Şimdilik bu fonksiyonu
     * PWM zaten PA8'den gittiği için boş bırakabiliriz veya
     * Başak ile ortak bir pin belirlemelisin.
     */
}

void Motor_Disable(void) {
    Motor_Emergency_Stop();
}
