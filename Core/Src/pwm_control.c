#include "pwm_control.h"
#include "main.h" // Başak'ın IN1_Pin, IN2_Pin tanımlarını kullanmak için şart!

static TIM_HandleTypeDef *m_htim = NULL;
static uint32_t m_channel = 0;

void Motor_Init(TIM_HandleTypeDef *htim, uint32_t channel)
{
    m_htim = htim;
    m_channel = channel;

    /*
     * PWM çıkışını başlatıyoruz.
     * Bu satır çalışmazsa PA8 pininden PWM çıkmaz.
     */
    HAL_TIM_PWM_Start(m_htim, m_channel);

    /*
     * Güvenli başlangıç:
     * PWM = 0
     * IN1 = 0
     * IN2 = 0
     * Motor başlangıçta dönmez.
     */
    Motor_Emergency_Stop();
}

void Motor_Set_Speed(uint16_t duty, MotorDir_t dir)
{
    if (m_htim == NULL) {
        return;
    }

    /*
     * Güvenlik için duty üst sınırı.
     * TIM1 ARR = 3599 olduğundan 3420 yaklaşık %95 duty yapar.
     */
    if (duty > MOTOR_PWM_SAFE_MAX) {
        duty = MOTOR_PWM_SAFE_MAX;
    }

    if (dir == MOTOR_CW) {
        /*
         * CW yön:
         * IN1 = 1
         * IN2 = 0
         */
        HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
    }
    else if (dir == MOTOR_CCW) {
        /*
         * CCW yön:
         * IN1 = 0
         * IN2 = 1
         */
        HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_SET);
    }
    else {
        /*
         * MOTOR_STOP veya bilinmeyen durum varsa motoru güvenli durdur.
         */
        Motor_Emergency_Stop();
        return;
    }

    /*
     * PWM duty değerini timer compare registerına yazıyoruz.
     * Şu an PA8 / TIM1_CH1 geçici PWM çıkışı olarak kullanılıyor.
     */
    __HAL_TIM_SET_COMPARE(m_htim, m_channel, duty);
}

void Motor_Emergency_Stop(void)
{
    if (m_htim != NULL) {
        __HAL_TIM_SET_COMPARE(m_htim, m_channel, 0);
    }

    /*
     * Coast / boşta duruş:
     * IN1 = 0
     * IN2 = 0
     */
    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
}

void Motor_Coast(void)
{
    /*
     * Motor uçları boşa alınır.
     * L298N için IN1=0, IN2=0 ve PWM=0 güvenli coast durumudur.
     */
    Motor_Emergency_Stop();
}

void Motor_Brake(void)
{
	/*
	 * Geçici frenleme mantığı.
	 * Nihai BTS7960 bağlantısında fren/coast davranışı tekrar düzenlenecek.
	 */
    if (m_htim != NULL) {
        __HAL_TIM_SET_COMPARE(m_htim, m_channel, 0);
    }

    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_SET);
}
