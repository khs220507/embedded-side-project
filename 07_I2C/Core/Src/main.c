#include "main.h"
#include <stdio.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

static void SystemClock_Config(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void I2C_Scan(void);

int _write(int file, char *ptr, int len)
{
  (void)file;
  HAL_UART_Transmit(&huart2, (uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY);
  return len;
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  __HAL_RCC_DMA1_CLK_ENABLE();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  printf("I2C scanner ready\r\n");
  I2C_Scan();

  while (1)
  {
    HAL_Delay(1000U);
  }
}

static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef oscillator = {0};
  RCC_ClkInitTypeDef clock = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  oscillator.HSIState = RCC_HSI_ON;
  oscillator.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  oscillator.PLL.PLLState = RCC_PLL_ON;
  oscillator.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  oscillator.PLL.PLLM = 16;
  oscillator.PLL.PLLN = 336;
  oscillator.PLL.PLLP = RCC_PLLP_DIV4;
  oscillator.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&oscillator) != HAL_OK) Error_Handler();

  clock.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clock.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clock.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clock.APB1CLKDivider = RCC_HCLK_DIV2;
  clock.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&clock, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

static void I2C_Scan(void)
{
  uint16_t address;
  for (address = 1U; address < 128U; address++)
  {
    if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(address << 1U), 2U, 10U) == HAL_OK)
      printf("I2C device: 0x%02X\r\n", address);
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}
