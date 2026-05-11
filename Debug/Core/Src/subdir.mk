################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc_measure.c \
../Core/Src/control_loop.c \
../Core/Src/encoder_rpm.c \
../Core/Src/fault_safety.c \
../Core/Src/main.c \
../Core/Src/oled_app.c \
../Core/Src/oled_ssd1306.c \
../Core/Src/pwm_control.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/uart_debug.c 

OBJS += \
./Core/Src/adc_measure.o \
./Core/Src/control_loop.o \
./Core/Src/encoder_rpm.o \
./Core/Src/fault_safety.o \
./Core/Src/main.o \
./Core/Src/oled_app.o \
./Core/Src/oled_ssd1306.o \
./Core/Src/pwm_control.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/uart_debug.o 

C_DEPS += \
./Core/Src/adc_measure.d \
./Core/Src/control_loop.d \
./Core/Src/encoder_rpm.d \
./Core/Src/fault_safety.d \
./Core/Src/main.d \
./Core/Src/oled_app.d \
./Core/Src/oled_ssd1306.d \
./Core/Src/pwm_control.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/uart_debug.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc_measure.cyclo ./Core/Src/adc_measure.d ./Core/Src/adc_measure.o ./Core/Src/adc_measure.su ./Core/Src/control_loop.cyclo ./Core/Src/control_loop.d ./Core/Src/control_loop.o ./Core/Src/control_loop.su ./Core/Src/encoder_rpm.cyclo ./Core/Src/encoder_rpm.d ./Core/Src/encoder_rpm.o ./Core/Src/encoder_rpm.su ./Core/Src/fault_safety.cyclo ./Core/Src/fault_safety.d ./Core/Src/fault_safety.o ./Core/Src/fault_safety.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/oled_app.cyclo ./Core/Src/oled_app.d ./Core/Src/oled_app.o ./Core/Src/oled_app.su ./Core/Src/oled_ssd1306.cyclo ./Core/Src/oled_ssd1306.d ./Core/Src/oled_ssd1306.o ./Core/Src/oled_ssd1306.su ./Core/Src/pwm_control.cyclo ./Core/Src/pwm_control.d ./Core/Src/pwm_control.o ./Core/Src/pwm_control.su ./Core/Src/stm32f1xx_hal_msp.cyclo ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_hal_msp.su ./Core/Src/stm32f1xx_it.cyclo ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/stm32f1xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f1xx.cyclo ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/system_stm32f1xx.su ./Core/Src/uart_debug.cyclo ./Core/Src/uart_debug.d ./Core/Src/uart_debug.o ./Core/Src/uart_debug.su

.PHONY: clean-Core-2f-Src

