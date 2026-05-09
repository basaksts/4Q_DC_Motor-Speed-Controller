#ifndef INC_FAULT_SAFETY_H_
#define INC_FAULT_SAFETY_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef enum
{
    FAULT_NONE = 0,
    FAULT_OVERCURRENT
} FaultState_t;

#define FAULT_CURRENT_LIMIT_A     1.00f
#define FAULT_CURRENT_RECOVERY_A  0.85f

void Fault_Init(void);
FaultState_t Fault_Check(float current_A);
uint8_t Fault_Is_Active(void);

#endif /* INC_FAULT_SAFETY_H_ */
