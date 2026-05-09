#ifndef INC_FAULT_SAFETY_H_
#define INC_FAULT_SAFETY_H_

#include "stm32f1xx_hal.h"

// Başak'ın main.c dosyasındaki limitler
#define CURRENT_LIMIT_A      1.00f  // Akım bu değeri geçerse kapat
#define CURRENT_RECOVERY_A   0.85f  // Akım bu değerin altına düşerse tekrar aç

// Hata Durumları
typedef enum {
    FAULT_NONE = 0,
    FAULT_OVERCURRENT
} FaultState_t;

// Fonksiyonlar
void Fault_Init(void);
FaultState_t Fault_Check(float current_A); // Akımı kontrol eder ve durumu döner
uint8_t Fault_Is_Active(void);             // Hata var mı yok mu?

#endif
