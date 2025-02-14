/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MPU_Config(void);
static void SystemIsolation_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void mainTask(void *pvParameters)
{
    // Task code goes here
    for (;;)
    {
        // Perform main task operations
    }
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

  /* Update SystemCoreClock variable according to RCC registers values. */
  SystemCoreClockUpdate();

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Update SystemCoreClock variable */
  SystemCoreClockUpdate();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  SystemIsolation_Config();
  /* USER CODE BEGIN 2 */
  xTaskCreate(mainTask, "mainTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);          // Task handle

  // Start the scheduler
  vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_TIM;
  PeriphClkInitStruct.TIMPresSelection = RCC_TIMPRES_DIV1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RIF Initialization Function
  * @param None
  * @retval None
  */
  static void SystemIsolation_Config(void)
{

  /* USER CODE BEGIN RIF_Init 0 */

  /* USER CODE END RIF_Init 0 */

  /* set all required IPs as secure privileged */
  __HAL_RCC_RIFSC_CLK_ENABLE();
  RIMC_MasterConfig_t RIMC_master = {0};
  RIMC_master.MasterCID = RIF_CID_1;
  RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;

  /*RIMC configuration*/
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_ETH1, &RIMC_master);
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_SDMMC2, &RIMC_master);

  /* RIF-Aware IPs Config */

  /* set up GPIO configuration */
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_1,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_3,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_12,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_14,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_8,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_0,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_2,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_5,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_7,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_8,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_9,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_10,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_11,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_12,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_13,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_14,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_15,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_3,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_4,GPIO_PIN_NSEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);

  /* USER CODE BEGIN RIF_Init 1 */

  /* USER CODE END RIF_Init 1 */
  /* USER CODE BEGIN RIF_Init 2 */

  /* USER CODE END RIF_Init 2 */

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};
  MPU_Attributes_InitTypeDef MPU_AttributesInit = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region 0 and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = __NON_CACHEABLE_SECTION_BEGIN;
  MPU_InitStruct.LimitAddress = __NON_CACHEABLE_SECTION_END;
  MPU_InitStruct.AttributesIndex = MPU_ATTRIBUTES_NUMBER0;
  MPU_InitStruct.AccessPermission = MPU_REGION_ALL_RW;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  MPU_AttributesInit.Number = MPU_ATTRIBUTES_NUMBER0;
  MPU_AttributesInit.Attributes = INNER_OUTER(MPU_NOT_CACHEABLE);

  HAL_MPU_ConfigMemoryAttributes(&MPU_AttributesInit);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

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

#ifdef  USE_FULL_ASSERT
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
