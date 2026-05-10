/*
 * Motor control abstraction layer for BTS7960 / IBT-2.
 *
 * Current hardware target:
 * - RPWM: TIM3_CH1
 * - LPWM: TIM3_CH2
 * - R_EN: GPIO output
 * - L_EN: GPIO output
 *
 * main.c motor driver detaylarını bilmez.
 * Sadece Motor_Set_Speed() gibi genel fonksiyonları kullanır.
 */

#include "pwm_control.h"
#include "main.h"

static TIM_HandleTypeDef *m_htim = NULL;
static uint32_t m_rpwm_channel = 0;
static uint32_t m_lpwm_channel = 0;

void Motor_Init(TIM_HandleTypeDef *htim,
                uint32_t rpwm_channel,
                uint32_t lpwm_channel)
{
    m_htim = htim;
    m_rpwm_channel = rpwm_channel;
    m_lpwm_channel = lpwm_channel;

    HAL_TIM_PWM_Start(m_htim, m_rpwm_channel);
    HAL_TIM_PWM_Start(m_htim, m_lpwm_channel);

    /*
     * BTS7960 enable pinleri aktif edilir.
     */
    HAL_GPIO_WritePin(R_EN_GPIO_Port, R_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(L_EN_GPIO_Port, L_EN_Pin, GPIO_PIN_SET);

    Motor_Emergency_Stop();
}

void Motor_Set_Speed(uint16_t duty, MotorDirection_t direction)
{
    if (m_htim == NULL)
    {
        return;
    }

    if (duty > MOTOR_PWM_SAFE_MAX)
    {
        duty = MOTOR_PWM_SAFE_MAX;
    }

    /*
     * Normal çalışmada sürücüyü enable durumda tutuyoruz.
     */
    HAL_GPIO_WritePin(R_EN_GPIO_Port, R_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(L_EN_GPIO_Port, L_EN_Pin, GPIO_PIN_SET);

    switch (direction)
    {
        case MOTOR_CW:
            /*
             * İleri yön:
             * RPWM aktif, LPWM kapalı.
             */
            __HAL_TIM_SET_COMPARE(m_htim, m_rpwm_channel, duty);
            __HAL_TIM_SET_COMPARE(m_htim, m_lpwm_channel, 0);
            break;

        case MOTOR_CCW:
            /*
             * Geri yön:
             * LPWM aktif, RPWM kapalı.
             */
            __HAL_TIM_SET_COMPARE(m_htim, m_rpwm_channel, 0);
            __HAL_TIM_SET_COMPARE(m_htim, m_lpwm_channel, duty);
            break;

        case MOTOR_STOP:
        default:
            Motor_Emergency_Stop();
            break;
    }
}

void Motor_Emergency_Stop(void)
{
    if (m_htim != NULL)
    {
        __HAL_TIM_SET_COMPARE(m_htim, m_rpwm_channel, 0);
        __HAL_TIM_SET_COMPARE(m_htim, m_lpwm_channel, 0);
    }

    /*
     * Acil stopta PWM sıfırlanır.
     * Enable pinlerini aktif bırakıyoruz; motor sürülmez.
     */
    HAL_GPIO_WritePin(R_EN_GPIO_Port, R_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(L_EN_GPIO_Port, L_EN_Pin, GPIO_PIN_SET);
}

void Motor_Coast(void)
{
    if (m_htim != NULL)
    {
        __HAL_TIM_SET_COMPARE(m_htim, m_rpwm_channel, 0);
        __HAL_TIM_SET_COMPARE(m_htim, m_lpwm_channel, 0);
    }

    /*
     * Coast için sürücü disable edilir.
     */
    HAL_GPIO_WritePin(R_EN_GPIO_Port, R_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(L_EN_GPIO_Port, L_EN_Pin, GPIO_PIN_RESET);
}

void Motor_Brake(void)
{
    if (m_htim != NULL)
    {
        /*
         * BTS7960 frenleme davranışı kartlara göre değişebileceğinden
         * şimdilik güvenli duruş olarak iki PWM de sıfırlanır.
         * Gerçek motor testinde gerekirse ayrıca netleştirilecektir.
         */
        __HAL_TIM_SET_COMPARE(m_htim, m_rpwm_channel, 0);
        __HAL_TIM_SET_COMPARE(m_htim, m_lpwm_channel, 0);
    }

    HAL_GPIO_WritePin(R_EN_GPIO_Port, R_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(L_EN_GPIO_Port, L_EN_Pin, GPIO_PIN_SET);
}
