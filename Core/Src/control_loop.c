#include "control_loop.h"
#include "pwm_control.h"

// Mevcut durum değişkenleri
static uint16_t current_duty = 0;
static MotorDir_t current_dir = MOTOR_STOP;

static uint16_t target_duty = 0;
static MotorDir_t target_dir = MOTOR_STOP;

static uint32_t last_ramp_tick = 0;
static uint32_t last_4q_tick = 0;

void Control_Init(void) {
    current_duty = 0;
    target_duty = 0;
    current_dir = MOTOR_STOP;
    target_dir = MOTOR_STOP;
}

// Yeni hedef belirleme (Rampayı başlatır)
void Control_Set_Target(uint16_t duty, MotorDir_t dir) {
    target_duty = duty;
    target_dir = dir;
}

// Hızlanma ve Yavaşlama Rampası (Acceleration/Deceleration)
// Bu fonksiyon main.c içinde her döngüde çağrılmalıdır.
void Control_Update_Ramp(void) {
    if (HAL_GetTick() - last_ramp_tick < RAMP_INTERVAL_MS) return;
    last_ramp_tick = HAL_GetTick();

    // Eğer yön değişecekse önce hızı sıfıra indir (Güvenli 4Q geçişi)
    if (current_dir != target_dir && current_duty > 0) {
        if (current_duty >= RAMP_STEP) current_duty -= RAMP_STEP;
        else current_duty = 0;
    }
    // Yön aynıysa veya motor durmuşsa hızı hedefe yaklaştır
    else {
        current_dir = target_dir; // Yönü güncelle

        if (current_duty < target_duty) {
            current_duty += RAMP_STEP;
            if (current_duty > target_duty) current_duty = target_duty;
        }
        else if (current_duty > target_duty) {
            current_duty -= RAMP_STEP;
            if (current_duty < target_duty) current_duty = target_duty;
        }
    }

    // Donanıma uygula
    Motor_Set_Speed(current_duty, current_dir);
}

// Test #5: 4Q Otomatik Yön Değiştirme (Her 500ms'de bir)
void Control_Run_4Q_Test(uint32_t current_tick) {
    if (current_tick - last_4q_tick >= TEST_4Q_INTERVAL_MS) {
        last_4q_tick = current_tick;

        // CW -> CCW veya CCW -> CW geçişi yap
        if (target_dir == MOTOR_CW) {
            Control_Set_Target(900, MOTOR_CCW); // Nominal hıza yakın bir değer
        } else {
            Control_Set_Target(900, MOTOR_CW);
        }
    }
}
