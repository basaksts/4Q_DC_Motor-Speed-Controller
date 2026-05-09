#include "control_loop.h"
#include "pwm_control.h"
#include "stm32f1xx_hal.h"

// Mevcut durum değişkenleri (Başak'ın değişken isimleriyle uyumlu hale getirildi)
static uint16_t actual_duty = 0;
static uint16_t target_duty = 0;
static MotorDir_t current_direction = MOTOR_CW;

// Zamanlama ve State-Machine değişkenleri
static uint32_t last_control_tick = 0;
static uint32_t last_direction_change_ms = 0;
static TestState_t test_state = TEST_STATE_RUN;

void Control_Init(void) {
    actual_duty = 0;
    target_duty = 0;
    current_direction = MOTOR_CW;
    test_state = TEST_STATE_RUN;
    last_direction_change_ms = HAL_GetTick();
    last_control_tick = HAL_GetTick();
}

void Control_Set_Target(uint16_t duty, MotorDir_t dir) {
    // Normal moddayken hedefi doğrudan set eder
    target_duty = duty;
    current_direction = dir;
}

void Control_Run_4Q_Test(uint8_t enable) {
    if (!enable) {
        test_state = TEST_STATE_RUN; // Test kapalıysa başlangıç durumuna dön
        return;
    }

    uint32_t now = HAL_GetTick();

    switch (test_state) {
        case TEST_STATE_RUN:
            // Hedef duty (main.c'den gelen pot değeri olabilir) korunur
            if ((now - last_direction_change_ms) >= TEST_SWITCH_MS) {
                test_state = TEST_STATE_RAMP_DOWN;
            }
            break;

        case TEST_STATE_RAMP_DOWN:
            // Yön değiştirmeden önce hızı sıfıra zorla
            target_duty = 0;
            if (actual_duty == 0) {
                test_state = TEST_STATE_CHANGE_DIR;
            }
            break;

        case TEST_STATE_CHANGE_DIR:
            // Tam duruş sağlandığında yönü değiştir
            if (current_direction == MOTOR_CW) {
                current_direction = MOTOR_CCW;
            } else {
                current_direction = MOTOR_CW;
            }
            last_direction_change_ms = now;
            test_state = TEST_STATE_RUN;
            break;

        default:
            test_state = TEST_STATE_RUN;
            break;
    }
}

void Control_Update(void) {
    // Zamanlama kontrolü (Her 20ms'de bir çalışır)
    uint32_t now = HAL_GetTick();
    if (now - last_control_tick < CONTROL_INTERVAL_MS) return;
    last_control_tick = now;

    /*
     * Ramp Algoritması:
     * actual_duty, target_duty'ye RAMP_STEP (30) adımlarla yaklaşır.
     */
    if (actual_duty < target_duty) {
        if ((target_duty - actual_duty) > RAMP_STEP) {
            actual_duty += RAMP_STEP;
        } else {
            actual_duty = target_duty;
        }
    }
    else if (actual_duty > target_duty) {
        if ((actual_duty - target_duty) > RAMP_STEP) {
            actual_duty -= RAMP_STEP;
        } else {
            actual_duty = target_duty;
        }
    }

    // Hesaplanan hızı ve yönü fiziksel motora uygula
    Motor_Set_Speed(actual_duty, current_direction);
}
