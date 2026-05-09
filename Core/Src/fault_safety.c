#include "fault_safety.h"
#include "uart_debug.h"

static FaultState_t m_fault_state = FAULT_NONE;

void Fault_Init(void) {
    m_fault_state = FAULT_NONE;
}

FaultState_t Fault_Check(float current_A) {
    // Negatif akım (rejeneratif frenleme) durumunu da kapsamak için mutlak değer alıyoruz
    float abs_current = (current_A < 0) ? -current_A : current_A;

    if (m_fault_state == FAULT_NONE && (abs_current > CURRENT_LIMIT_A)) {
        m_fault_state = FAULT_OVERCURRENT;
        UART_Debug_Print("!!! FAULT: Overcurrent Detected !!!\r\n");
    }
    else if (m_fault_state == FAULT_OVERCURRENT && (abs_current < CURRENT_RECOVERY_A)) {
        m_fault_state = FAULT_NONE;
        UART_Debug_Print("Fault Cleared: Current is back to normal.\r\n");
    }

    return m_fault_state;
}

uint8_t Fault_Is_Active(void) {
    return (m_fault_state != FAULT_NONE);
}
