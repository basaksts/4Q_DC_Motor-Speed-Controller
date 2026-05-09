/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pwm_control.h"
#include "control_loop.h"
#include "fault_safety.h"
#include "uart_debug.h"
#include "adc_measure.h"
#include "encoder_rpm.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t test_mode_enabled = 0;
static GPIO_PinState last_button_state = GPIO_PIN_SET;
static uint32_t last_button_time = 0;
#define BUTTON_DEBOUNCE_MS 200
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  UART_Debug_Init(&huart1);
    ADCMeasure_Init(&hadc1);
    ADCMeasure_CalibrateCurrentZero(); // Akım sensörünü sıfırla
    EncoderRPM_Init();
    Motor_Init(&htim1, TIM_CHANNEL_1);
    Control_Init();
    Fault_Init();

    UART_Debug_Print("Sistem Hazir, Motor Beklemede...\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
      {
        /* 1. Mod Değiştirme Butonu (PB12) */
        GPIO_PinState current_button = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);
        if (last_button_state == GPIO_PIN_SET && current_button == GPIO_PIN_RESET) {
            if (HAL_GetTick() - last_button_time > BUTTON_DEBOUNCE_MS) {
                test_mode_enabled = !test_mode_enabled;
                last_button_time = HAL_GetTick();
                Control_Init(); // Mod değişince hızları güvenliğe al
                UART_Debug_Print(test_mode_enabled ? "Mod: 4Q Otomatik Test\r\n" : "Mod: Potansiyometre Kontrol\r\n");
            }
        }
        last_button_state = current_button;

        /* 2. Sensör Verilerini Al */
        float motor_current = ADCMeasure_GetMotorCurrentA();
        float pot_val = ADCMeasure_GetPotPercent();
        EncoderRPM_Update();

        /* 3. Akım Koruması ve Motor Kontrolü */
        if (Fault_Check(motor_current) == FAULT_OVERCURRENT) {
            Motor_Emergency_Stop(); // Akım yüksekse elektriği kes!
        }
        else {
            if (test_mode_enabled) {
                Control_Run_4Q_Test(1); // Otomatik ileri-geri modu
            } else {
                Control_Run_4Q_Test(0); // Normal mod
                // Potansiyometreye göre hedef hızı belirle
                uint16_t target_pwm = (uint16_t)((pot_val / 100.0f) * MOTOR_PWM_SAFE_MAX);
                Control_Set_Target(target_pwm, MOTOR_CW);
            }
            Control_Update(); // Rampayı hesapla ve PWM'i uygula
        }

        /* 4. UART Ekranına Veri Bas (Her 100ms'de bir) */
        static uint32_t debug_tick = 0;
        if (HAL_GetTick() - debug_tick > 100) {
            UART_Debug_PrintMotorData((uint8_t)pot_val, (uint16_t)EncoderRPM_GetRPM(), 1);
            debug_tick = HAL_GetTick();
        }

        /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_1) { // PA1 Encoder Pini
        EncoderRPM_OnPulse();
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
