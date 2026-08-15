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
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
  LED_STATE_IDLE,
  LED_STATE_RUNNING,
  LED_STATE_WARNING,
  LED_STATE_ERROR,
  LED_STATE_COUNT
} LED_State;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define BUTTON_DEBOUNCE_MS 50U
#define UART_RX_BUFFER_SIZE 32U
#define UART_TX_BUFFER_SIZE 128U
#define CDS_BRIGHT_THRESHOLD 1200U
#define CDS_NORMAL_THRESHOLD 2100U
#define CDS_DIM_THRESHOLD 3000U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim2;

/* USART2 설정과 상태를 HAL 드라이버에 전달하는 핸들 */
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */

static LED_State currentState = LED_STATE_IDLE;
static uint8_t uartDmaRxBuffer[UART_RX_BUFFER_SIZE];
static char uartRxBuffer[UART_RX_BUFFER_SIZE];
static volatile uint8_t uartRxIndex = 0U;
static volatile uint8_t uartCommandReady = 0U;
static volatile uint8_t uartRxOverflow = 0U;
static uint8_t uartDmaTxBuffer[UART_TX_BUFFER_SIZE];
static volatile uint8_t uartDmaTxComplete = 1U;
static volatile uint8_t timerSampleRequested = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

static void LED_AllOff(void);
static void LED_On(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
static void LED_ShowState(LED_State state);
static void UART_PrintState(LED_State state);
static void UART_HandleCommand(const char *command);
static void UART_StartDmaReceive(void);
static void UART_StoreReceivedData(const uint8_t *data, uint16_t size);
static HAL_StatusTypeDef ADC_ReadCdS(uint32_t *rawValue);
static void CdS_ProcessSample(void);
static LED_State CdS_GetBrightnessState(uint32_t rawValue);
static const char *CdS_GetBrightnessName(LED_State state);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int _write(int file, char *ptr, int len)
{
  int sent = 0;

  (void)file;

  while (sent < len)
  {
    uint16_t chunkSize = (uint16_t)(len - sent);

    if (chunkSize > UART_TX_BUFFER_SIZE)
    {
      chunkSize = UART_TX_BUFFER_SIZE;
    }

    memcpy(uartDmaTxBuffer, &ptr[sent], chunkSize);
    uartDmaTxComplete = 0U;

    if (HAL_UART_Transmit_DMA(&huart2, uartDmaTxBuffer, chunkSize) != HAL_OK)
    {
      return -1;
    }

    while (uartDmaTxComplete == 0U)
    {
      __WFI();
    }

    sent += chunkSize;
  }

  return len;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  GPIO_PinState lastButtonReading = GPIO_PIN_RESET;
  GPIO_PinState stableButtonState = GPIO_PIN_RESET;
  uint32_t lastDebounceTime = 0U;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* HAL 라이브러리, Flash 인터페이스, 1 ms 주기의 SysTick 타이머를 초기화한다. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* 내부 HSI 오실레이터와 PLL을 이용해 시스템 클럭을 84 MHz로 설정한다. */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  MX_ADC1_Init();
  MX_TIM2_Init();

  /* LED와 사용자 버튼이 연결된 GPIO를 초기화한다. */
  MX_GPIO_Init();

  MX_DMA_Init();

  /* ST-LINK 가상 COM 포트에 연결된 USART2를 115200 bps로 초기화한다. */
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  LED_ShowState(currentState);

  printf("Hello STM32!\r\n");
  UART_PrintState(currentState);

  UART_StartDmaReceive();

  if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE END 2 */

  /* 초기화가 끝난 뒤 프로그램이 계속 실행되는 메인 반복 구간 */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (timerSampleRequested != 0U)
    {
      timerSampleRequested = 0U;
      CdS_ProcessSample();
    }

    GPIO_PinState buttonReading = HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin);

    if (buttonReading != lastButtonReading)
    {
      lastDebounceTime = HAL_GetTick();
    }

    if ((HAL_GetTick() - lastDebounceTime) >= BUTTON_DEBOUNCE_MS)
    {
      if (buttonReading != stableButtonState)
      {
        stableButtonState = buttonReading;

        if (stableButtonState == GPIO_PIN_SET)
        {
          currentState = (LED_State)((currentState + 1) % LED_STATE_COUNT);
          LED_ShowState(currentState);
          UART_PrintState(currentState);
        }
      }
    }

    lastButtonReading = buttonReading;

    if (uartCommandReady != 0U)
    {
      if (uartRxOverflow != 0U)
      {
        printf("Command too long\r\n");
      }
      else
      {
        UART_HandleCommand(uartRxBuffer);
      }

      uartRxIndex = 0U;
      uartRxOverflow = 0U;
      uartCommandReady = 0U;
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
  /* 오실레이터와 버스 클럭 설정 구조체를 0으로 초기화한다. */
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** 전원 제어 장치의 클럭을 활성화하고 84 MHz 동작에 맞는 전압 스케일을 선택한다.
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** 16 MHz 내부 HSI를 PLL 입력으로 사용해 84 MHz 시스템 클럭을 만든다.
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  /* 오실레이터 설정에 실패하면 안전하게 오류 처리 루틴으로 이동한다. */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** SYSCLK와 AHB는 84 MHz, APB1은 42 MHz, APB2는 84 MHz로 설정한다.
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  ADC_ChannelConfTypeDef channelConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  channelConfig.Channel = ADC_CHANNEL_0;
  channelConfig.Rank = 1;
  channelConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;

  if (HAL_ADC_ConfigChannel(&hadc1, &channelConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8399;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DMA Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  /* USART2: 115200 bps, 데이터 8비트, 패리티 없음, 정지 비트 1개(8-N-1) */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* 한 개 이상의 GPIO 핀에 공통으로 적용할 설정 구조체 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO 레지스터에 접근할 수 있도록 사용하는 포트의 클럭을 활성화한다. */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* 초기화 중 LED가 의도치 않게 켜지지 않도록 모든 LED 출력을 LOW로 만든다. */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|RED_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, YELLOW_LED_Pin|BLUE_LED_Pin|GREEN_LED_Pin, GPIO_PIN_RESET);

  /* PC13 사용자 버튼을 폴링 입력으로 사용한다. */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* PA5(LD2), PA8(빨간 LED): 푸시풀 출력, 내부 풀업/풀다운 없음 */
  GPIO_InitStruct.Pin = LD2_Pin|RED_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* PB10(노랑), PB4(파랑), PB5(초록 LED): 푸시풀 출력 */
  GPIO_InitStruct.Pin = YELLOW_LED_Pin|BLUE_LED_Pin|GREEN_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static HAL_StatusTypeDef ADC_ReadCdS(uint32_t *rawValue)
{
  HAL_StatusTypeDef status;

  status = HAL_ADC_Start(&hadc1);
  if (status != HAL_OK)
  {
    return status;
  }

  status = HAL_ADC_PollForConversion(&hadc1, 10U);
  if (status == HAL_OK)
  {
    *rawValue = HAL_ADC_GetValue(&hadc1);
  }

  if (HAL_ADC_Stop(&hadc1) != HAL_OK && status == HAL_OK)
  {
    status = HAL_ERROR;
  }

  return status;
}

static void CdS_ProcessSample(void)
{
  uint32_t rawValue;

  if (ADC_ReadCdS(&rawValue) == HAL_OK)
  {
    uint32_t millivolts = (rawValue * 3300U) / 4095U;

    currentState = CdS_GetBrightnessState(rawValue);
    LED_ShowState(currentState);

    printf("CdS raw=%lu, voltage=%lu mV, brightness=%s\r\n",
           (unsigned long)rawValue, (unsigned long)millivolts,
           CdS_GetBrightnessName(currentState));
  }
  else
  {
    printf("CdS ADC read error\r\n");
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    timerSampleRequested = 1U;
  }
}

static LED_State CdS_GetBrightnessState(uint32_t rawValue)
{
  if (rawValue <= CDS_BRIGHT_THRESHOLD)
  {
    return LED_STATE_IDLE;
  }

  if (rawValue <= CDS_NORMAL_THRESHOLD)
  {
    return LED_STATE_RUNNING;
  }

  if (rawValue <= CDS_DIM_THRESHOLD)
  {
    return LED_STATE_WARNING;
  }

  return LED_STATE_ERROR;
}

static const char *CdS_GetBrightnessName(LED_State state)
{
  switch (state)
  {
    case LED_STATE_IDLE:
      return "BRIGHT";

    case LED_STATE_RUNNING:
      return "NORMAL";

    case LED_STATE_WARNING:
      return "DIM";

    case LED_STATE_ERROR:
      return "DARK";

    default:
      return "UNKNOWN";
  }
}

static void LED_AllOff(void)
{
  HAL_GPIO_WritePin(GPIOA, RED_LED_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, YELLOW_LED_Pin|BLUE_LED_Pin|GREEN_LED_Pin, GPIO_PIN_RESET);
}

static void LED_On(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  LED_AllOff();
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

static void LED_ShowState(LED_State state)
{
  switch (state)
  {
    case LED_STATE_IDLE:
      LED_On(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
      break;

    case LED_STATE_RUNNING:
      LED_On(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
      break;

    case LED_STATE_WARNING:
      LED_On(YELLOW_LED_GPIO_Port, YELLOW_LED_Pin);
      break;

    case LED_STATE_ERROR:
      LED_On(RED_LED_GPIO_Port, RED_LED_Pin);
      break;

    default:
      LED_AllOff();
      break;
  }
}

static void UART_PrintState(LED_State state)
{
  const char *stateName;

  switch (state)
  {
    case LED_STATE_IDLE:
      stateName = "IDLE";
      break;

    case LED_STATE_RUNNING:
      stateName = "RUNNING";
      break;

    case LED_STATE_WARNING:
      stateName = "WARNING";
      break;

    case LED_STATE_ERROR:
      stateName = "ERROR";
      break;

    default:
      stateName = "UNKNOWN";
      break;
  }

  printf("State: %s\r\n", stateName);
}

static void UART_HandleCommand(const char *command)
{
  printf("Command: %s\r\n", command);

  if ((strcmp(command, "g") == 0) || (strcmp(command, "G") == 0)
      || (strcmp(command, "led green") == 0))
  {
    currentState = LED_STATE_IDLE;
  }
  else if ((strcmp(command, "b") == 0) || (strcmp(command, "B") == 0)
           || (strcmp(command, "led blue") == 0))
  {
    currentState = LED_STATE_RUNNING;
  }
  else if ((strcmp(command, "y") == 0) || (strcmp(command, "Y") == 0)
           || (strcmp(command, "led yellow") == 0))
  {
    currentState = LED_STATE_WARNING;
  }
  else if ((strcmp(command, "r") == 0) || (strcmp(command, "R") == 0)
           || (strcmp(command, "led red") == 0))
  {
    currentState = LED_STATE_ERROR;
  }
  else if (strcmp(command, "status") == 0)
  {
    UART_PrintState(currentState);
    return;
  }
  else if (strcmp(command, "help") == 0)
  {
    printf("Commands: led green, led blue, led yellow, led red, status, help\r\n");
    return;
  }
  else
  {
    printf("Unknown command. Type help.\r\n");
    return;
  }

  LED_ShowState(currentState);
  UART_PrintState(currentState);
}

static void UART_StartDmaReceive(void)
{
  if (HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uartDmaRxBuffer,
                                   UART_RX_BUFFER_SIZE) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
}

static void UART_StoreReceivedData(const uint8_t *data, uint16_t size)
{
  for (uint16_t i = 0U; (i < size) && (uartCommandReady == 0U); i++)
  {
    uint8_t receivedByte = data[i];

    if ((receivedByte == '\r') || (receivedByte == '\n'))
    {
      if (uartRxIndex > 0U)
      {
        uartRxBuffer[uartRxIndex] = '\0';
        uartCommandReady = 1U;
      }
    }
    else if ((receivedByte == '\b') || (receivedByte == 0x7FU))
    {
      if (uartRxIndex > 0U)
      {
        uartRxIndex--;
      }
    }
    else if (uartRxIndex < (UART_RX_BUFFER_SIZE - 1U))
    {
      uartRxBuffer[uartRxIndex] = (char)receivedByte;
      uartRxIndex++;
    }
    else
    {
      uartRxOverflow = 1U;
      uartCommandReady = 1U;
    }
  }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
  if (huart->Instance == USART2)
  {
    UART_StoreReceivedData(uartDmaRxBuffer, size);
    UART_StartDmaReceive();
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    uartDmaTxComplete = 1U;
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
  /* 치명적인 초기화 오류가 발생하면 인터럽트를 막고 이 위치에서 정지한다. */
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
