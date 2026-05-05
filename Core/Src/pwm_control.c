#include "pwm_control.h"

static TIM_HandleTypeDef *m_htim;
static uint32_t m_channel;

void Motor_Init(TIM_HandleTypeDef *htim, uint32_t channel) {
    m_htim = htim;
    m_channel = channel;
    HAL_TIM_PWM_Start(m_htim, m_channel);
}

void Motor_Set_Speed(uint16_t duty, MotorDir_t dir) {
    // %95 doluluk sınırı (Güvenlik için) [cite: 102]
    if (duty > 950) duty = 950;

    if (dir == MOTOR_CW) {
        // IN1: High, IN2: Low (L298 mantığı) [cite: 98]
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
    } else if (dir == MOTOR_CCW) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
    } else {
        Motor_Emergency_Stop();
    }
    __HAL_TIM_SET_COMPARE(m_htim, m_channel, duty);
}

void Motor_Emergency_Stop(void) {
    __HAL_TIM_SET_COMPARE(m_htim, m_channel, 0);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
}
// Motoru Donanımsal Olarak Aktif Et
void Motor_Enable(void) {
    // Başak'ın belirleyeceği EN pinini HIGH yap
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
}

// Motoru Donanımsal Olarak Devre Dışı Bırak
void Motor_Disable(void) {
    // EN pinini LOW yap
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
}
