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

#include <stdio.h>

#include "pwm_control.h"
//#include "control_loop.h"
#include "uart_debug.h"
#include "adc_measure.h"
#include "encoder_rpm.h"
#include "fault_safety.h"
#include "oled_app.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define USE_MOCK_SENSOR_DATA      0
#define MOCK_FORCE_OVERCURRENT    0
#define START_IN_4Q_TEST_MODE     0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

static uint16_t target_duty = 0;
static uint16_t actual_duty = 0;
static uint16_t requested_duty = 0;

static MotorDirection_t current_direction = MOTOR_CW;

static uint8_t test_mode_enabled = 0;
static uint32_t last_direction_change_ms = 0;

static GPIO_PinState last_button_state = GPIO_PIN_SET;
static uint32_t last_button_time_ms = 0;

static GPIO_PinState last_estop_button_state = GPIO_PIN_SET;
static uint32_t last_estop_button_time_ms = 0;
static uint8_t emergency_stop_latched = 0;

uint16_t display_pwm_percent = 0;
int16_t display_rpm = 0;
float display_current_A = 0.0f;

char display_direction[8] = "STOP";
char display_status[12] = "READY";


typedef enum
{
    TEST_STATE_RUN = 0,
    TEST_STATE_RAMP_DOWN,
    TEST_STATE_CHANGE_DIR
} TestState_t;

static TestState_t test_state = TEST_STATE_RUN;

#define DUTY_RAMP_STEP       15U
#define CONTROL_DELAY_MS     20U
#define TEST_SWITCH_MS       4000U
#define BUTTON_DEBOUNCE_MS   200U

#define FOUR_Q_MIN_DUTY      1200U

#define ACS_DEMO_RAW_MIN_A       3.40f
#define ACS_DEMO_RAW_MAX_A       5.70f
#define ACS_DEMO_DISPLAY_MAX_A   1.20f
#define ACS_DEMO_FAULT_LIMIT_A   1.00f


FaultState_t fault_state = FAULT_NONE;
uint8_t overcurrent_fault = 0;

static uint8_t demo_overcurrent_latched = 0;
static float demo_latched_current_A = 0.0f;


uint32_t last_debug_ms = 0;
#define DEBUG_PERIOD_MS 500U


#if USE_MOCK_SENSOR_DATA
static float mock_pot_percent = 0.0f;
static uint8_t mock_pot_increasing = 1;
#endif

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void Update_Display_Data(uint16_t duty,
                                float rpm,
                                float current_A,
                                MotorDirection_t direction,
                                uint8_t fault_active,
                                uint8_t estop_active)
{
    display_pwm_percent = (uint16_t)((duty * 100U) / MOTOR_PWM_MAX_DUTY);
    display_rpm = (int16_t)rpm;
    display_current_A = current_A;

    if (estop_active)
    {
        snprintf(display_status, sizeof(display_status), "ESTOP");
    }
    else if (fault_active)
    {
        snprintf(display_status, sizeof(display_status), "FAULT");
    }
    else if (duty == 0)
    {
        snprintf(display_status, sizeof(display_status), "STOP");
    }
    else
    {
        snprintf(display_status, sizeof(display_status), "RUN");
    }

    if (direction == MOTOR_CW)
    {
        snprintf(display_direction, sizeof(display_direction), "CW");
    }
    else if (direction == MOTOR_CCW)
    {
        snprintf(display_direction, sizeof(display_direction), "CCW");
    }
    else
    {
        snprintf(display_direction, sizeof(display_direction), "STOP");
    }
}


#if USE_MOCK_SENSOR_DATA
static void Mock_UpdateSensorData(float *pot_percent,
                                  float *motor_current_A,
                                  float *motor_voltage_V,
                                  float *motor_rpm)
{
    if (mock_pot_increasing)
    {
        mock_pot_percent += 2.0f;

        if (mock_pot_percent >= 80.0f)
        {
            mock_pot_percent = 80.0f;
            mock_pot_increasing = 0;
        }
    }
    else
    {
        mock_pot_percent -= 2.0f;

        if (mock_pot_percent <= 0.0f)
        {
            mock_pot_percent = 0.0f;
            mock_pot_increasing = 1;
        }
    }

    *pot_percent = mock_pot_percent;

    /*
     * Mock akım:
     * PWM talebi arttıkça akım da artıyor gibi simüle ediyoruz.
     * Limit 1A olduğu için normalde fault oluşturmuyor.
     */
#if MOCK_FORCE_OVERCURRENT
    *motor_current_A = 1.20f;
#else
    *motor_current_A = 0.10f + (mock_pot_percent / 100.0f) * 0.60f;
#endif
    /*
     * Mock motor voltajı:
     * 12V sistem varsayımıyla pot yüzdesine bağlı.
     */
    *motor_voltage_V = 12.0f * (mock_pot_percent / 100.0f);

    /*
     * Mock RPM:
     * Motor yokken yaklaşık bir RPM davranışı üretir.
     */
    *motor_rpm = mock_pot_percent * 2.0f;
}
#endif


static float ACS_DemoScaleCurrent(float raw_current_A)
{
    float scaled_A = 0.0f;

    if (raw_current_A <= ACS_DEMO_RAW_MIN_A)
    {
        scaled_A = 0.0f;
    }
    else if (raw_current_A >= ACS_DEMO_RAW_MAX_A)
    {
        scaled_A = ACS_DEMO_DISPLAY_MAX_A;
    }
    else
    {
        scaled_A = ((raw_current_A - ACS_DEMO_RAW_MIN_A) /
                    (ACS_DEMO_RAW_MAX_A - ACS_DEMO_RAW_MIN_A)) *
                    ACS_DEMO_DISPLAY_MAX_A;
    }

    return scaled_A;
}


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
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  UART_Debug_Init(&huart1);
  UART_Debug_Print("UART debug started\r\n");

  UART_Debug_Print("System started\r\n");

  ADCMeasure_Init(&hadc1);
  UART_Debug_Print("ADC measure initialized\r\n");

  HAL_Delay(1000);

  ADCMeasure_CalibrateCurrentZero();
  UART_Debug_Print("ACS712 current zero calibrated\r\n");

  EncoderRPM_Init();
  UART_Debug_Print("Encoder RPM initialized\r\n");

  Motor_Init(&htim3, TIM_CHANNEL_1, TIM_CHANNEL_2);
  UART_Debug_Print("Motor PWM initialized\r\n");


  Fault_Init();
  UART_Debug_Print("Fault safety initialized\r\n");

  OLED_App_Init(&hi2c1);
  UART_Debug_Print("OLED app initialized\r\n");


  //motoru sabit duty ile sürmek için:
  // Motor_Set_Speed(1200, MOTOR_CW);
  //UART_Debug_Print("Motor test: CW, duty=1200\r\n");
  UART_Debug_Print("Pot controlled PWM mode started\r\n");




  test_mode_enabled = START_IN_4Q_TEST_MODE;
  last_direction_change_ms = HAL_GetTick();
  current_direction = MOTOR_CW;
  test_state = TEST_STATE_RUN;

  if (test_mode_enabled)
  {
      UART_Debug_Print("4Q test mode started\r\n");
  }
  else
  {
      UART_Debug_Print("Normal mode started\r\n");
  }

  #if USE_MOCK_SENSOR_DATA
      UART_Debug_Print("MOCK SENSOR MODE ACTIVE\r\n");
  #endif

  #if MOCK_FORCE_OVERCURRENT
      UART_Debug_Print("MOCK OVERCURRENT TEST ACTIVE\r\n");
  #endif





  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


	  /*
	   * PB12 test butonu:
	   * Pull-up kullanıldığı için butona basınca pin RESET okunur.
	   * Her basışta normal mod <-> 4Q test modu arasında geçiş yapılır.
	   */
	  GPIO_PinState button_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);

	  if ((last_button_state == GPIO_PIN_SET) && (button_state == GPIO_PIN_RESET))
	  {
	      uint32_t now = HAL_GetTick();

	      if ((now - last_button_time_ms) > BUTTON_DEBOUNCE_MS)
	      {
	          last_button_time_ms = now;

	          test_mode_enabled = !test_mode_enabled;

	          /*
	           * Mod değişirken sistemi güvenli başlatıyoruz.
	           */
	          actual_duty = 0;
	          target_duty = 0;
	          test_state = TEST_STATE_RUN;
	          current_direction = MOTOR_CW;
	          last_direction_change_ms = HAL_GetTick();

	          Motor_Emergency_Stop();

	          if (test_mode_enabled)
	          {
	              UART_Debug_Print("4Q test mode enabled\r\n");
	          }
	          else
	          {
	              UART_Debug_Print("Normal mode enabled\r\n");
	          }
	      }
	  }

	  last_button_state = button_state;

	  /*
	   * PB13 emergency stop butonu:
	   * Pull-up kullanıldığı için butona basınca pin RESET okunur.
	   * Her basışta emergency stop aktif/pasif olur.
	   */
	  GPIO_PinState estop_button_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);

	  if ((last_estop_button_state == GPIO_PIN_SET) &&
	      (estop_button_state == GPIO_PIN_RESET))
	  {
	      uint32_t now = HAL_GetTick();

	      if ((now - last_estop_button_time_ms) > BUTTON_DEBOUNCE_MS)
	      {
	          last_estop_button_time_ms = now;

	          emergency_stop_latched = !emergency_stop_latched;

	          actual_duty = 0;
	          target_duty = 0;
	          requested_duty = 0;

	          test_state = TEST_STATE_RUN;
	          current_direction = MOTOR_CW;
	          last_direction_change_ms = HAL_GetTick();

	          Motor_Emergency_Stop();

	          if (emergency_stop_latched)
	          {
	              UART_Debug_Print("EMERGENCY STOP ACTIVE\r\n");
	          }
	          else
	          {
	              UART_Debug_Print("EMERGENCY STOP CLEARED\r\n");
	          }
	      }
	  }

	  last_estop_button_state = estop_button_state;


	  float pot_percent = 0.0f;
	  float motor_current_A = 0.0f;
	  float motor_voltage_V = 0.0f;
	  float motor_rpm = 0.0f;

	  #if USE_MOCK_SENSOR_DATA

	  Mock_UpdateSensorData(&pot_percent,
	                        &motor_current_A,
	                        &motor_voltage_V,
	                        &motor_rpm);

	  #else

	  pot_percent = ADCMeasure_GetPotPercent();
	  motor_current_A = ADCMeasure_GetMotorCurrentA();
	  motor_voltage_V = ADCMeasure_GetMotorVoltage();

	 // motor_current_A = ACS_DemoScaleCurrent(motor_current_A);

	  /* DEMO current model:
	   * Pot %0 iken 0.00 A,
	   * Pot %100 iken 1.20 A gösterir.
	   * Böylece 1.00 A üstü fault testi net yapılır.
	   */
	  motor_current_A = (pot_percent / 100.0f) * 1.20f;

	  EncoderRPM_Update();
	  motor_rpm = EncoderRPM_GetRPM();



	  #endif

	  (void)motor_voltage_V;


	  /*
	   * Fault / overcurrent protection
	   *
	   * Akım güvenlik kontrolü artık fault_safety.c içindeki
	   * Fault_Check() fonksiyonu ile yapılıyor.
	   */
	  uint8_t previous_fault_state = overcurrent_fault;

	  /* DEMO overcurrent fault with hysteresis
	   * 1.00 A üstünde FAULT'a girer.
	   * 0.85 A altına düşünce FAULT'tan çıkar.
	   */
	  if (!overcurrent_fault)
	  {
	      if (motor_current_A >= 1.00f)
	      {
	          fault_state = FAULT_OVERCURRENT;
	          overcurrent_fault = 1;

	          actual_duty = 0;
	          target_duty = 0;
	          requested_duty = 0;

	          Motor_Emergency_Stop();
	      }
	      else
	      {
	          fault_state = FAULT_NONE;
	          overcurrent_fault = 0;
	      }
	  }
	  else
	  {
	      if (motor_current_A <= 0.85f)
	      {
	          fault_state = FAULT_NONE;
	          overcurrent_fault = 0;

	          actual_duty = 0;
	          target_duty = 0;
	          requested_duty = 0;

	          test_state = TEST_STATE_RUN;
	          last_direction_change_ms = HAL_GetTick();
	      }
	      else
	      {
	          fault_state = FAULT_OVERCURRENT;
	          overcurrent_fault = 1;

	          actual_duty = 0;
	          target_duty = 0;
	          requested_duty = 0;

	          Motor_Emergency_Stop();
	      }
	  }

	  /* GEÇİCİ TEST: ACS712 fault devre dışı */
	  //overcurrent_fault = 0;
	  //fault_state = FAULT_NONE;

	  /*
	   * Fault yeni aktif olduysa motoru güvenli şekilde durdur.
	   */
	  if (!previous_fault_state && overcurrent_fault)
	  {
	      actual_duty = 0;
	      target_duty = 0;

	      Motor_Emergency_Stop();
	  }

	  /*
	   * Fault yeni temizlendiyse sistemi güvenli başlangıç durumuna al.
	   */
	  else if (previous_fault_state && !overcurrent_fault)
	  {
	      actual_duty = 0;
	      target_duty = 0;

	      test_state = TEST_STATE_RUN;
	      last_direction_change_ms = HAL_GetTick();
	  }



	  if (!overcurrent_fault && !emergency_stop_latched)
	  {
	      /*
	       * Potansiyometreden istenen duty hesaplanır.
	       */
	      requested_duty = (uint16_t)((pot_percent / 100.0f) * MOTOR_PWM_SAFE_MAX);

	      if (test_mode_enabled)
	      {
	          if (requested_duty < FOUR_Q_MIN_DUTY)
	          {
	              requested_duty = FOUR_Q_MIN_DUTY;
	          }
	      }
	      /*
	       * 4Q test modu state-machine.
	       */
	      if (test_mode_enabled)
	      {
	          uint32_t now = HAL_GetTick();

	          switch (test_state)
	          {
	              case TEST_STATE_RUN:
	                  target_duty = requested_duty;

	                  if ((now - last_direction_change_ms) >= TEST_SWITCH_MS)
	                  {
	                      test_state = TEST_STATE_RAMP_DOWN;
	                  }
	                  break;

	              case TEST_STATE_RAMP_DOWN:
	                  target_duty = 0;

	                  if (actual_duty == 0)
	                  {
	                      test_state = TEST_STATE_CHANGE_DIR;
	                  }
	                  break;

	              case TEST_STATE_CHANGE_DIR:
	                  if (current_direction == MOTOR_CW)
	                  {
	                      current_direction = MOTOR_CCW;
	                  }
	                  else
	                  {
	                      current_direction = MOTOR_CW;
	                  }

	                  last_direction_change_ms = HAL_GetTick();
	                  test_state = TEST_STATE_RUN;
	                  break;

	              default:
	                  test_state = TEST_STATE_RUN;
	                  break;
	          }
	      }
	      else
	      {
	          /*
	           * Normal mod:
	           * Potansiyometre hızı belirler, yön şimdilik CW.
	           */
	          target_duty = requested_duty;
	          current_direction = MOTOR_CW;
	      }

	      /*
	       * Ramp algoritması:
	       * actual_duty, target_duty'ye yavaşça yaklaşır.
	       */
	      if (actual_duty < target_duty)
	      {
	          uint16_t diff = target_duty - actual_duty;

	          if (diff > DUTY_RAMP_STEP)
	          {
	              actual_duty += DUTY_RAMP_STEP;
	          }
	          else
	          {
	              actual_duty = target_duty;
	          }
	      }
	      else if (actual_duty > target_duty)
	      {
	          uint16_t diff = actual_duty - target_duty;

	          if (diff > DUTY_RAMP_STEP)
	          {
	              actual_duty -= DUTY_RAMP_STEP;
	          }
	          else
	          {
	              actual_duty = target_duty;
	          }
	      }
	  }
	  else
	  {
	      /*
	       * Fault aktifken hiçbir yeni duty üretilmesin.
	       */
	      requested_duty = 0;
	      target_duty = 0;
	      actual_duty = 0;
	  }


	    //Eğer fault aktifse ramp veya test state-machine yanlışlıkla
	    //duty üretse bile motor kesinlikle sürülmesin.
	    if (!overcurrent_fault && !emergency_stop_latched)
	    {
	        Motor_Set_Speed(actual_duty, current_direction);
	    }
	    else
	    {
	        Motor_Emergency_Stop();
	    }




	    Update_Display_Data(actual_duty,
	                        motor_rpm,
	                        motor_current_A,
	                        current_direction,
	                        overcurrent_fault,
	                        emergency_stop_latched);



	    if (test_mode_enabled && !overcurrent_fault && !emergency_stop_latched)
	    {
	        if ((test_state == TEST_STATE_RUN) && (current_direction == MOTOR_CW))
	        {
	            snprintf(display_status, sizeof(display_status), "Q1 MOTOR");
	            snprintf(display_direction, sizeof(display_direction), "CW");
	        }
	        else if ((test_state == TEST_STATE_RAMP_DOWN) && (current_direction == MOTOR_CW))
	        {
	            snprintf(display_status, sizeof(display_status), "Q2 BRAKE");
	            snprintf(display_direction, sizeof(display_direction), "CW");
	        }
	        else if ((test_state == TEST_STATE_RUN) && (current_direction == MOTOR_CCW))
	        {
	            snprintf(display_status, sizeof(display_status), "Q3 MOTOR");
	            snprintf(display_direction, sizeof(display_direction), "CCW");
	        }
	        else if ((test_state == TEST_STATE_RAMP_DOWN) && (current_direction == MOTOR_CCW))
	        {
	            snprintf(display_status, sizeof(display_status), "Q4 BRAKE");
	            snprintf(display_direction, sizeof(display_direction), "CCW");
	        }
	    }

	    if ((HAL_GetTick() - last_debug_ms) >= DEBUG_PERIOD_MS)
	    {

	    	last_debug_ms = HAL_GetTick();

	        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

	        UART_Debug_PrintSystemData(display_pwm_percent,
	                                   display_rpm,
	                                   display_current_A,
	                                   display_direction,
	                                   display_status);

	        OLED_App_UpdateMainScreen(display_pwm_percent,
	                                  display_rpm,
	                                  display_current_A,
	                                  display_direction,
	                                  display_status);
	    }


	    HAL_Delay(CONTROL_DELAY_MS);



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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 3599;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, R_EN_Pin|L_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : R_EN_Pin L_EN_Pin */
  GPIO_InitStruct.Pin = R_EN_Pin|L_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
