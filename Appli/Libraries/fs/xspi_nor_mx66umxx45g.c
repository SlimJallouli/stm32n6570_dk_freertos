/*
 * FreeRTOS STM32 Reference Integration
 *
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#include "logging_levels.h"
#define LOG_LEVEL    LOG_DEBUG
#include "logging.h"
#include "FreeRTOS.h"
#include "task.h"

#include "xspi_nor_mx66umxx45g.h"


static TaskHandle_t xTaskHandle = NULL;

static inline void xspi_HandleCallback(XSPI_HandleTypeDef *pxXSPI, HAL_XSPI_CallbackIDTypeDef xCallbackId)
{
  configASSERT(pxXSPI != NULL);
  configASSERT(xTaskHandle != NULL);
  BaseType_t xHigherPriorityTaskWoken;

  xTaskNotifyIndexedFromISR(xTaskHandle, 1, xCallbackId, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* STM32 HAL Callbacks */
static void xspi_RxCpltCallback(XSPI_HandleTypeDef *pxXSPI)
{
  xspi_HandleCallback(pxXSPI, HAL_XSPI_RX_CPLT_CB_ID);
}

static void xspi_TxCpltCallback(XSPI_HandleTypeDef *pxXSPI)
{
  xspi_HandleCallback(pxXSPI, HAL_XSPI_TX_CPLT_CB_ID);
}

static void xspi_CmdCpltCallback(XSPI_HandleTypeDef *pxXSPI)
{
  xspi_HandleCallback(pxXSPI, HAL_XSPI_CMD_CPLT_CB_ID);
}

static void xspi_StatusMatchCpltCallback(XSPI_HandleTypeDef *pxXSPI)
{
  xspi_HandleCallback(pxXSPI, HAL_XSPI_STATUS_MATCH_CB_ID);
}

static void xspi_TimeoutCallback(XSPI_HandleTypeDef *pxXSPI)
{
  xspi_HandleCallback(pxXSPI, HAL_XSPI_TIMEOUT_CB_ID);
}

static void xspi_ErrorCallback(XSPI_HandleTypeDef *pxXSPI)
{
  xspi_HandleCallback(pxXSPI, HAL_XSPI_ERROR_CB_ID);
}

static void xspi_AbortCallback(XSPI_HandleTypeDef *pxXSPI)
{
  xspi_HandleCallback(pxXSPI, HAL_XSPI_ABORT_CB_ID);
}

static BaseType_t xspi_WaitForCallback(HAL_XSPI_CallbackIDTypeDef xCallbackID, TickType_t xTicksToWait)
{
  configASSERT(xCallbackID <= HAL_XSPI_TIMEOUT_CB_ID);
  configASSERT(xCallbackID >= HAL_XSPI_ERROR_CB_ID);

  TickType_t xRemainingTicks = xTicksToWait;
  TimeOut_t xTimeOut;

  uint32_t ulNotifyValue = 0xFFFFFFFF;

  vTaskSetTimeOutState(&xTimeOut);

  while (ulNotifyValue != xCallbackID)
  {
    (void) xTaskNotifyWaitIndexed(1, 0x0, 0xFFFFFFFF, &ulNotifyValue, xRemainingTicks);

    if (xTaskCheckForTimeOut(&xTimeOut, &xRemainingTicks))
    {
      ulNotifyValue = 0xFFFFFFFF;
      break;
    }
  }

  return (ulNotifyValue == xCallbackID);
}

/* Initialize static variables for the current operation */
static inline void xspi_OpInit(XSPI_HandleTypeDef *pxXSPI)
{
  xTaskHandle = xTaskGetCurrentTaskHandle();
}

static BaseType_t xspi_InitDriver(XSPI_HandleTypeDef *pxXSPI)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;

  /* Register additional callbacks */
  xHalStatus = HAL_XSPI_RegisterCallback(pxXSPI, HAL_XSPI_RX_CPLT_CB_ID, xspi_RxCpltCallback);
  xHalStatus &= HAL_XSPI_RegisterCallback(pxXSPI, HAL_XSPI_TX_CPLT_CB_ID, xspi_TxCpltCallback);
  xHalStatus &= HAL_XSPI_RegisterCallback(pxXSPI, HAL_XSPI_CMD_CPLT_CB_ID, xspi_CmdCpltCallback);
  xHalStatus &= HAL_XSPI_RegisterCallback(pxXSPI, HAL_XSPI_STATUS_MATCH_CB_ID, xspi_StatusMatchCpltCallback);
  xHalStatus &= HAL_XSPI_RegisterCallback(pxXSPI, HAL_XSPI_TIMEOUT_CB_ID, xspi_TimeoutCallback);
  xHalStatus &= HAL_XSPI_RegisterCallback(pxXSPI, HAL_XSPI_ERROR_CB_ID, xspi_ErrorCallback);
  xHalStatus &= HAL_XSPI_RegisterCallback(pxXSPI, HAL_XSPI_ABORT_CB_ID, xspi_AbortCallback);

  if (xHalStatus != HAL_OK)
  {
    LogError("Error while register OSPI driver callbacks.");
    return pdFALSE;
  }

  return pdTRUE;
}

static void xspi_AbortTransaction(XSPI_HandleTypeDef *pxXSPI, TickType_t xTimeout)
{
  (void) HAL_XSPI_Abort_IT(pxXSPI);
  (void) xspi_WaitForCallback(HAL_XSPI_ABORT_CB_ID, xTimeout);
}

static BaseType_t xspi_cmd_OPI_WREN(XSPI_HandleTypeDef *pxXSPI, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;
  BaseType_t xSuccess = pdTRUE;

 XSPI_RegularCmdTypeDef xCmd = {0};

  xCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.IOSelect           = HAL_XSPI_SELECT_IO_3_0;
  xCmd.Instruction        = OPI_CMD_WRITE_ENABLE;
  xCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  xCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  xCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  xCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  xCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  xCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  xCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  xCmd.DataMode           = HAL_XSPI_DATA_NONE;
  xCmd.DummyCycles        = 0;
  xCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  /* Clear notification state */
  (void) xTaskNotifyStateClearIndexed(NULL, 1);

  /* Send command */
  xHalStatus = HAL_XSPI_Command_IT(pxXSPI, &xCmd);

  /* Wait for command complete callback */
  if (xHalStatus == HAL_OK)
  {
    xSuccess = xspi_WaitForCallback(HAL_XSPI_CMD_CPLT_CB_ID, xTimeout);
  }
  else
  {
    xSuccess = pdFALSE;
  }

  return (xSuccess);
}
#if 0
static BaseType_t xspi_OPI_WaitForStatus(XSPI_HandleTypeDef *pxXSPI, uint32_t ulMask, uint32_t ulMatch, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;
  BaseType_t xSuccess = pdTRUE;

  /* Setup a read of the xHalStatus register */
  XSPI_RegularCmdTypeDef xCmd;
  /* = {
  .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
  .FlashId = HAL_XSPI_FLASH_ID_1,
  .Instruction = MX66UM_OPI_RDSR,
  .InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES, // 8 line STR mode
  .InstructionSize = HAL_XSPI_INSTRUCTION_16_BITS, // 2 byte instructions
  .InstructionDtrMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
  .Address = 0x00000000, // Address = 0 for RDSR
  .AddressMode = HAL_XSPI_ADDRESS_8_LINES,
  .AddressSize = HAL_XSPI_ADDRESS_32_BITS,
  .AddressDtrMode = HAL_XSPI_DATA_DTR_DISABLE,
  .AlternateBytesMode = HAL_XSPI_ALTERNATE_BYTES_NONE,
  .DataMode = HAL_XSPI_DATA_8_LINES,
  .DataDtrMode = HAL_XSPI_DATA_DTR_DISABLE,
  .NbData = 1, // RDSR reg is 1 byte of data
  .DummyCycles = 4, // PM2357 R1.1 pg 23, Note 5 => 4 dummy cycles
  .DQSMode = HAL_XSPI_DQS_DISABLE,
  .SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD, };*/

  xCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.IOSelect           = HAL_XSPI_SELECT_IO_3_0;
  xCmd.Instruction        = OCTAL_READ_STATUS_REG_CMD;
  xCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  xCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  xCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  xCmd.Address            = HAL_XSPI_ADDRESS_NONE; /* Address = 0 for RDSR */
  xCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  xCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  xCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  xCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  xCmd.DataLength         = 1; /* RDSR reg is 1 byte of data */
  xCmd.DataMode           = HAL_XSPI_DATA_NONE;
  xCmd.DummyCycles        = 4;
  xCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  /* Send command */
  xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, xTimeout);

  XSPI_AutoPollingTypeDef xPollingCfg;

  xPollingCfg.MatchValue    = ulMatch;
  xPollingCfg.MatchMask     = ulMask;
  xPollingCfg.MatchMode     = HAL_XSPI_MATCH_MODE_AND;
  xPollingCfg.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;
  xPollingCfg.IntervalTime  = AUTO_POLLING_INTERVAL;

  /* Clear notification state */
  (void) xTaskNotifyStateClearIndexed(NULL, 1);

  if (xHalStatus == HAL_OK)
  {
    /* Start auto-polling */
    xHalStatus = HAL_XSPI_AutoPolling(pxXSPI, &xPollingCfg, MX66UM_DEFAULT_TIMEOUT_MS);

    if (xHalStatus != HAL_OK)
    {
      xSuccess = pdFALSE;
    }
  }

  configASSERT(xSuccess);

  if (xSuccess == pdTRUE)
  {
    xSuccess = xspi_WaitForCallback(HAL_XSPI_STATUS_MATCH_CB_ID, xTimeout);
  }

  configASSERT(xSuccess);

  /* Abort the ongoing transaction upon failure */
  if (xSuccess == pdFALSE)
  {
    (void) xspi_AbortTransaction(pxXSPI, xTimeout);
  }

  return xSuccess;
}
#else
static BaseType_t xspi_OPI_WaitForStatus(XSPI_HandleTypeDef *pxXSPI, uint32_t ulMask, uint32_t ulMatch, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;
  BaseType_t xSuccess = pdTRUE;

  /* Setup a read of the xHalStatus register */
  XSPI_RegularCmdTypeDef xCmd;
  /* = {
  .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
  .FlashId = HAL_XSPI_FLASH_ID_1,
  .Instruction = MX66UM_OPI_RDSR,
  .InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES, // 8 line STR mode
  .InstructionSize = HAL_XSPI_INSTRUCTION_16_BITS, // 2 byte instructions
  .InstructionDtrMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
  .Address = 0x00000000, // Address = 0 for RDSR
  .AddressMode = HAL_XSPI_ADDRESS_8_LINES,
  .AddressSize = HAL_XSPI_ADDRESS_32_BITS,
  .AddressDtrMode = HAL_XSPI_DATA_DTR_DISABLE,
  .AlternateBytesMode = HAL_XSPI_ALTERNATE_BYTES_NONE,
  .DataMode = HAL_XSPI_DATA_8_LINES,
  .DataDtrMode = HAL_XSPI_DATA_DTR_DISABLE,
  .NbData = 1, // RDSR reg is 1 byte of data
  .DummyCycles = 4, // PM2357 R1.1 pg 23, Note 5 => 4 dummy cycles
  .DQSMode = HAL_XSPI_DQS_DISABLE,
  .SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD, };*/

  xCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.IOSelect           = HAL_XSPI_SELECT_IO_3_0;
  xCmd.Instruction        = OPI_CMD_READ_STATUS_REG;
  xCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  xCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  xCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  xCmd.Address            = HAL_XSPI_ADDRESS_NONE; /* Address = 0 for RDSR */
  xCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  xCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  xCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  xCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  xCmd.DataLength         = 1; /* RDSR reg is 1 byte of data */
  xCmd.DataMode           = HAL_XSPI_DATA_NONE;
  xCmd.DummyCycles        = 4;
  xCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  /* Clear notification state */
  (void) xTaskNotifyStateClearIndexed(NULL, 1);

  /* Send command */
  xHalStatus = HAL_XSPI_Command_IT(pxXSPI, &xCmd);

  XSPI_AutoPollingTypeDef xPollingCfg;

  xPollingCfg.MatchValue    = ulMatch;
  xPollingCfg.MatchMask     = ulMask;
  xPollingCfg.MatchMode     = HAL_XSPI_MATCH_MODE_AND;
  xPollingCfg.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;
  xPollingCfg.IntervalTime  = AUTO_POLLING_INTERVAL;

  /* Clear notification state */
  (void) xTaskNotifyStateClearIndexed(NULL, 1);

  if (xHalStatus == HAL_OK)
  {
    /* Start auto-polling */
    xHalStatus = HAL_XSPI_AutoPolling(pxXSPI, &xPollingCfg, MX66UM_DEFAULT_TIMEOUT_MS);

    if (xHalStatus != HAL_OK)
    {
      xSuccess = pdFALSE;
    }
  }

  configASSERT(xSuccess);

  if (xSuccess == pdTRUE)
  {
    xSuccess = xspi_WaitForCallback(HAL_XSPI_STATUS_MATCH_CB_ID, xTimeout);
  }

  configASSERT(xSuccess);

  /* Abort the ongoing transaction upon failure */
  if (xSuccess == pdFALSE)
  {
    (void) xspi_AbortTransaction(pxXSPI, xTimeout);
  }

  return xSuccess;
}
#endif
static BaseType_t xspi_SPI_WaitForStatus(XSPI_HandleTypeDef *pxXSPI, uint32_t ulMask, uint32_t ulMatch, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;
  BaseType_t xSuccess = pdTRUE;

  /* Setup a read of the xHalStatus register */
  XSPI_RegularCmdTypeDef xCmd;

  xCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.IOSelect           = HAL_XSPI_SELECT_IO_3_0;
  xCmd.Instruction        = OPI_CMD_READ_STATUS_REG;
  xCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  xCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  xCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  xCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  xCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  xCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  xCmd.DataLength         = 1;
  xCmd.DummyCycles        = 0;
  xCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
//  xCmd.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD, };

  /* Send command */
  xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, xTimeout);

  XSPI_AutoPollingTypeDef xPollingCfg;

  xPollingCfg.MatchValue    = ulMatch;
  xPollingCfg.MatchMask     = ulMask;
  xPollingCfg.MatchMode     = HAL_XSPI_MATCH_MODE_AND;
  xPollingCfg.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;
  xPollingCfg.IntervalTime  = 0x10;

  /* Clear notification state */
  (void) xTaskNotifyStateClearIndexed(NULL, 1);

  if (xHalStatus == HAL_OK)
  {
    /* Start auto-polling */
    xHalStatus = HAL_XSPI_AutoPolling_IT(pxXSPI, &xPollingCfg);

    if (xHalStatus != HAL_OK)
    {
      xSuccess = pdFALSE;
    }
  }

  if (xSuccess == pdTRUE)
  {
    xSuccess = xspi_WaitForCallback(HAL_XSPI_STATUS_MATCH_CB_ID, xTimeout);
  }

  /* Abort the ongoing transaction upon failure */
  if (xSuccess == pdFALSE)
  {
    (void) xspi_AbortTransaction(pxXSPI, xTimeout);
  }

  return xSuccess;
}
#if 0
/* send Write enable command (WREN) in SPI mode */
static BaseType_t xspi_cmd_SPI_WREN(XSPI_HandleTypeDef *pxXSPI, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;
  XSPI_RegularCmdTypeDef xCmd;

  xCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.IOSelect           = HAL_XSPI_SELECT_IO_3_0;
  xCmd.Instruction        = OCTAL_WRITE_ENABLE_CMD;
  xCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  xCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  xCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  xCmd.Address            = 0;
  xCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  xCmd.AddressWidth       = 0;
  xCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  xCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.DataMode           = HAL_XSPI_DATA_NONE;
  xCmd.DataLength         = 0;
  xCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  xCmd.DummyCycles        = 0;
  xCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
//  xCmd.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD;

  (void) xTaskNotifyStateClearIndexed(NULL, 1);

  xHalStatus = HAL_XSPI_Command_IT(pxXSPI, &xCmd);

  if (xHalStatus == HAL_OK)
  {
    if (xspi_WaitForCallback(HAL_XSPI_CMD_CPLT_CB_ID, xTimeout) != pdTRUE)
    {
      xHalStatus = -1;
    }
  }

  return (xHalStatus == HAL_OK);
}

/*
 * Switch flash from 1 bit SPI mode to 8 bit STR mode (single bit per clock)
 */
static BaseType_t xspi_cmd_SPI_8BitSTRMode(XSPI_HandleTypeDef *pxXSPI, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;

  XSPI_RegularCmdTypeDef xCmd;


  xCmd.OperationType          = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.IOSelect               = HAL_XSPI_SELECT_IO_3_0;
  xCmd.Instruction            = WRITE_CFG_REG_2_CMD;
  xCmd.InstructionMode        = HAL_XSPI_INSTRUCTION_1_LINE;
  xCmd.InstructionWidth       = HAL_XSPI_INSTRUCTION_8_BITS;
  xCmd.InstructionDTRMode     = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  xCmd.Address                = 0;
  xCmd.AddressMode            = HAL_XSPI_ADDRESS_1_LINE;
  xCmd.AddressWidth           = HAL_XSPI_ADDRESS_32_BITS;
  xCmd.AddressDTRMode         = HAL_XSPI_ADDRESS_DTR_DISABLE;
  xCmd.AlternateBytesMode     = HAL_XSPI_ALT_BYTES_1_LINE;
  xCmd.AlternateBytesWidth    = HAL_XSPI_ALT_BYTES_8_BITS;
  xCmd.AlternateBytes         = MX66UM_REG_CR2_0_SOPI;
  xCmd.AlternateBytesDTRMode  = HAL_XSPI_ALT_BYTES_DTR_DISABLE;
  xCmd.DataMode               = HAL_XSPI_DATA_NONE;
  xCmd.DataLength             = 0;
  xCmd.DataDTRMode            = HAL_XSPI_DATA_DTR_DISABLE;
  xCmd.DummyCycles            = 0;
  xCmd.DQSMode                = HAL_XSPI_DQS_DISABLE;
//  xCmd.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD };

  (void) xTaskNotifyStateClearIndexed(NULL, 1);

  xHalStatus = HAL_XSPI_Command_IT(pxXSPI, &xCmd);

  if ((xHalStatus == HAL_OK) && (xspi_WaitForCallback(HAL_XSPI_CMD_CPLT_CB_ID, xTimeout) != pdTRUE))
  {
    xHalStatus = -1;
  }

  return (xHalStatus == HAL_OK);
}

/*
 * @Brief Initialize octospi flash controller and related peripherals
 */
BaseType_t xspi_Init(XSPI_HandleTypeDef *pxXSPI)
{
  BaseType_t xSuccess = pdTRUE;

  xspi_OpInit(pxXSPI);

  xSuccess = xspi_InitDriver(pxXSPI);
#if 1
  return XSPI_NOR_OctalDTRModeCfg(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);
#else
  if (xSuccess != pdTRUE)
  {
    LogError("Failed to initialize ospi driver.");
  }
  else
  {
    /* Set Write enable bit */
    xSuccess = xspi_cmd_SPI_WREN(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);
  }

  if (xSuccess != pdTRUE)
  {
    LogError("Failed to send WREN command.");
  }
  else
  {
    xSuccess = xspi_SPI_WaitForStatus(pxXSPI, MX66UM_REG_SR_WIP | MX66UM_REG_SR_WEL, MX66UM_REG_SR_WEL, MX66UM_DEFAULT_TIMEOUT_MS);
  }

  if (xSuccess != pdTRUE)  {
    LogError("Timed out while waiting for write enable.");
  }
  else
  {
    /* Enter 8 bit data mode */
    xSuccess = xspi_cmd_SPI_8BitSTRMode(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);
  }

  if (xSuccess != pdTRUE)
  {
    LogError("Failed to set data mode to 8Bit STR.");
  }
  else
  {
    /* Wait for WEL and WIP bits to clear */
//    xSuccess = xspi_OPI_WaitForStatus(pxXSPI, MX66UM_REG_SR_WIP | MX66UM_REG_SR_WEL, 0x0, MX66UM_DEFAULT_TIMEOUT_MS);
  }

  return xSuccess;
#endif
}
#if 1
BaseType_t xspi_ReadAddr(XSPI_HandleTypeDef * pxXSPI, uint32_t ulAddr, void * pxBuffer, uint32_t ulBufferLen, TickType_t xTimeout )
{
  HAL_StatusTypeDef xHalStatus;
  BaseType_t xSuccess = pdTRUE;

  XSPI_RegularCmdTypeDef xCmd = { 0 };

  xspi_OpInit(pxXSPI);

  if (pxXSPI == NULL)
    {
      xSuccess = pdFALSE;
      LogError("pxXSPI is NULL.");
    }

    if (ulAddr >= MX66UM_MEM_SZ_BYTES)
    {
      xSuccess = pdFALSE;
      LogError("Address is out of range.");
    }

    if (pxBuffer == NULL)
    {
      xSuccess = pdFALSE;
      LogError("pxBuffer is NULL.");
    }

    if (ulBufferLen == 0)
    {
      xSuccess = pdFALSE;
      LogError("ulBufferLen is 0.");
    }

  xCmd.OperationType         = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.IOSelect              = HAL_XSPI_SELECT_IO_7_0;

  xCmd.Instruction           = OCTAL_IO_DTR_READ_CMD;
  xCmd.InstructionMode       = HAL_XSPI_INSTRUCTION_8_LINES;
  xCmd.InstructionWidth      = HAL_XSPI_INSTRUCTION_16_BITS;
  xCmd.InstructionDTRMode    = HAL_XSPI_INSTRUCTION_DTR_ENABLE;

  xCmd.Address               = ulAddr;
  xCmd.AddressMode           = HAL_XSPI_ADDRESS_8_LINES;
  xCmd.AddressWidth          = HAL_XSPI_ADDRESS_32_BITS;
  xCmd.AddressDTRMode        = HAL_XSPI_ADDRESS_DTR_ENABLE;

  xCmd.AlternateBytes        = 0;
  xCmd.AlternateBytesMode    = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.AlternateBytesWidth   = 0;
  xCmd.AlternateBytesDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE;

  xCmd.DataMode              = HAL_XSPI_DATA_8_LINES;
  xCmd.DataLength            = ulBufferLen;
  xCmd.DataDTRMode           = HAL_XSPI_DATA_DTR_ENABLE;

  xCmd.DummyCycles           = DUMMY_CLOCK_CYCLES_READ_OCTAL;
  xCmd.DQSMode               = HAL_XSPI_DQS_ENABLE;

  /* Clear notification state */
  (void) xTaskNotifyStateClearIndexed(NULL, 1);

  xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, xTimeout);

  if (xHalStatus != HAL_OK)
  {
    xSuccess = pdFALSE;
    xspi_AbortTransaction(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);
    LogError("Failed to send 8READ command.");
  }
#if 1
  if (xSuccess == pdTRUE)
  {
    /* Clear notification state */
    (void) xTaskNotifyStateClearIndexed(NULL, 1);

    xHalStatus = HAL_XSPI_Receive_IT(pxXSPI, pxBuffer);

    /* Wait for receive op to complete */
    if (xHalStatus == HAL_OK)
    {
      xSuccess = xspi_WaitForCallback(HAL_XSPI_RX_CPLT_CB_ID, xTimeout);
    }
  }

  return (xSuccess);
#else
  if(xHalStatus == HAL_OK)
  {
    xHalStatus = HAL_XSPI_Receive(pxXSPI, pxBuffer, xTimeout);
  }

  return xHalStatus == HAL_OK;
#endif
}

#else
BaseType_t xspi_ReadAddr(XSPI_HandleTypeDef *pxXSPI, uint32_t ulAddr, void *pxBuffer, uint32_t ulBufferLen, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;
  BaseType_t xSuccess = pdTRUE;

  xspi_OpInit(pxXSPI);

  if (pxXSPI == NULL)
  {
    xSuccess = pdFALSE;
    LogError("pxXSPI is NULL.");
  }

  if (ulAddr >= MX66UM_MEM_SZ_BYTES)
  {
    xSuccess = pdFALSE;
    LogError("Address is out of range.");
  }

  if (pxBuffer == NULL)
  {
    xSuccess = pdFALSE;
    LogError("pxBuffer is NULL.");
  }

  if (ulBufferLen == 0)
  {
    xSuccess = pdFALSE;
    LogError("ulBufferLen is 0.");
  }

  /*TODO is there a limit to the number of bytes read? */

  /* Wait for idle condition (WIP bit should be 0) */
  xSuccess = xspi_OPI_WaitForStatus(pxXSPI, MX66UM_REG_SR_WIP, 0x0, MX66UM_DEFAULT_TIMEOUT_MS);

  if (xSuccess != pdTRUE)
  {
    xspi_AbortTransaction(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);
    LogError("Timed out while waiting for OSPI IDLE condition.");
  }
  else
  {
    /* Setup an 8READ transaction */
    XSPI_RegularCmdTypeDef xCmd;

    xCmd.OperationType          = HAL_XSPI_OPTYPE_COMMON_CFG;
    xCmd.IOSelect               = HAL_XSPI_SELECT_IO_3_0;
    xCmd.Instruction            = MX66UM_OPI_8READ;
    xCmd.InstructionMode        = HAL_XSPI_INSTRUCTION_8_LINES; /* 8 line STR mode */
    xCmd.InstructionWidth       = HAL_XSPI_INSTRUCTION_16_BITS; /* 2 byte instructions */
    xCmd.InstructionDTRMode     = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    xCmd.Address                = ulAddr;
    xCmd.AddressMode            = HAL_XSPI_ADDRESS_8_LINES;
    xCmd.AddressWidth           = HAL_XSPI_ADDRESS_32_BITS;
    xCmd.AddressDTRMode         = HAL_XSPI_DATA_DTR_DISABLE;
    xCmd.AlternateBytesMode     = HAL_XSPI_ALT_BYTES_NONE;
    xCmd.DataMode               = HAL_XSPI_DATA_8_LINES;
    xCmd.DataDTRMode            = HAL_XSPI_DATA_DTR_DISABLE;
    xCmd.DataLength             = ulBufferLen;
    xCmd.DummyCycles            = MX66UM_8READ_DUMMY_CYCLES;
    xCmd.DQSMode                = HAL_XSPI_DQS_DISABLE;
//    xCmd.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD, };

    /* Clear notification state */
    (void) xTaskNotifyStateClearIndexed(NULL, 1);

    /* Send command */
    xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, MX66UM_DEFAULT_TIMEOUT_MS);

    if (xHalStatus != HAL_OK)
    {
      xSuccess = pdFALSE;
      xspi_AbortTransaction(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);
      LogError("Failed to send 8READ command.");
    }
  }

  if (xSuccess == pdTRUE)
  {
    /* Clear notification state */
    (void) xTaskNotifyStateClearIndexed(NULL, 1);

    xHalStatus = HAL_XSPI_Receive_IT(pxXSPI, pxBuffer);

    /* Wait for receive op to complete */
    if (xHalStatus == HAL_OK)
    {
      xSuccess = xspi_WaitForCallback(HAL_XSPI_RX_CPLT_CB_ID, xTimeout);
    }
  }

  return (xSuccess);
}
#endif
/*
 * @Brief write up to 256 bytes to the given address.
 */
#if 1
BaseType_t xspi_WriteAddr(XSPI_HandleTypeDef *pxXSPI, uint32_t ulAddr, const void *pxBuffer, uint32_t ulBufferLen, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;
  BaseType_t xSuccess = pdTRUE;

  xspi_OpInit(pxXSPI);

  if (pxXSPI == NULL)
  {
    xSuccess = pdFALSE;
  }

  if ((ulBufferLen > 256) || (ulBufferLen == 0))
  {
    xSuccess = pdFALSE;
  }

  if (pxBuffer == NULL)
  {
    xSuccess = pdFALSE;
  }

  if (xSuccess == pdTRUE)
  {
    /* Wait for idle condition (WIP bit should be 0) */
    xSuccess = xspi_OPI_WaitForStatus(pxXSPI, MX66UM_REG_SR_WIP, 0x0, xTimeout);
  }

  if (xSuccess == pdTRUE)
  {
    /* Enable write */
    xSuccess = xspi_cmd_OPI_WREN(pxXSPI, xTimeout);
  }

  /* Wait for Write Enable Latch */
  if (xSuccess == pdTRUE)
  {
    xSuccess = xspi_OPI_WaitForStatus(pxXSPI, MX66UM_REG_SR_WEL | MX66UM_REG_SR_WIP, MX66UM_REG_SR_WEL, xTimeout);
  }

  if (xSuccess == pdTRUE)
  {
    /* Setup a Program operation */
    XSPI_RegularCmdTypeDef xCmd;
    xCmd.OperationType          = HAL_XSPI_OPTYPE_COMMON_CFG;
    xCmd.IOSelect               = HAL_XSPI_SELECT_IO_3_0;
    xCmd.Instruction            = MX66UM_OPI_PP;
    xCmd.InstructionMode        = HAL_XSPI_INSTRUCTION_8_LINES; /* 8 line STR mode */
    xCmd.InstructionWidth       = HAL_XSPI_INSTRUCTION_16_BITS; /* 2 byte instructions */
    xCmd.InstructionDTRMode     = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    xCmd.Address                = ulAddr;
    xCmd.AddressMode            = HAL_XSPI_ADDRESS_8_LINES;
    xCmd.AddressWidth           = HAL_XSPI_ADDRESS_32_BITS;
    xCmd.AddressDTRMode         = HAL_XSPI_DATA_DTR_DISABLE;
    xCmd.AlternateBytesMode     = HAL_XSPI_ALT_BYTES_NONE;
    xCmd.DataMode               = HAL_XSPI_DATA_8_LINES;
    xCmd.DataDTRMode            = HAL_XSPI_DATA_DTR_DISABLE;
    xCmd.DataLength             = ulBufferLen;
    xCmd.DummyCycles            = 0;
    xCmd.DQSMode                = HAL_XSPI_DQS_DISABLE;
//    xCmd.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD, };

    /* Send command */
    xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, xTimeout);
  }

  /* Clear notification state */
  (void) xTaskNotifyStateClearIndexed(NULL, 1);

  if (xHalStatus != HAL_OK)
  {
    xSuccess = pdFALSE;
  }
  else
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    xHalStatus = HAL_XSPI_Transmit_IT(pxXSPI, pxBuffer);
#pragma GCC diagnostic pop
  }

  if (xHalStatus != HAL_OK)
  {
    xSuccess = pdFALSE;
  }
  else
  {
    xSuccess = xspi_WaitForCallback(HAL_XSPI_TX_CPLT_CB_ID, xTimeout);
  }

  if (xSuccess == pdTRUE)
  {
    vTaskDelay(1);

    /* Wait for idle condition (WIP bit should be 0) */
    xSuccess = xspi_OPI_WaitForStatus(pxXSPI, MX66UM_REG_SR_WIP | MX66UM_REG_SR_WEL, 0x0, xTimeout);
  }

  return xSuccess;
}
#endif
BaseType_t xspi_EraseSector(XSPI_HandleTypeDef *pxXSPI, uint32_t ulAddr, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;
  BaseType_t xSuccess = pdTRUE;

  xspi_OpInit(pxXSPI);

  if (pxXSPI == NULL)
  {
    xSuccess = pdFALSE;
  }

  /* Validate Address */
  if (ulAddr >= MX66UM_MEM_SZ_BYTES)
  {
    xSuccess = pdFALSE;
  }

  if (xSuccess == pdTRUE)
  {
    /* Wait for idle condition (WIP bit should be 0) */
    xSuccess = xspi_OPI_WaitForStatus(pxXSPI, MX66UM_REG_SR_WIP, 0x0, xTimeout);
  }

  if (xSuccess == pdTRUE)
  {
    /* Enable write */
    xSuccess = xspi_cmd_OPI_WREN(pxXSPI, xTimeout);
  }

  /* Wait for Write Enable Latch */
  if (xSuccess == pdTRUE)
  {
    xSuccess = xspi_OPI_WaitForStatus(pxXSPI, MX66UM_REG_SR_WEL, MX66UM_REG_SR_WEL, xTimeout);
  }

  if (xSuccess == pdTRUE)
  {
    /* Setup a Sector Erase operation */
    XSPI_RegularCmdTypeDef xCmd;

    xCmd.OperationType          = HAL_XSPI_OPTYPE_COMMON_CFG;
    xCmd.IOSelect               = HAL_XSPI_SELECT_IO_3_0;
    xCmd.Instruction            = MX66UM_OPI_SE;
    xCmd.InstructionMode        = HAL_XSPI_INSTRUCTION_8_LINES; /* 8 line STR mode */
    xCmd.InstructionWidth       = HAL_XSPI_INSTRUCTION_16_BITS; /* 2 byte instructions */
    xCmd.InstructionDTRMode     = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    xCmd.Address                = ulAddr;
    xCmd.AddressMode            = HAL_XSPI_ADDRESS_8_LINES;
    xCmd.AddressWidth           = HAL_XSPI_ADDRESS_32_BITS;
    xCmd.AddressDTRMode         = HAL_XSPI_DATA_DTR_DISABLE;
    xCmd.AlternateBytesMode     = HAL_XSPI_ALT_BYTES_NONE;
    xCmd.DataMode               = HAL_XSPI_DATA_NONE;
    xCmd.DataDTRMode            = HAL_XSPI_DATA_DTR_DISABLE;
    xCmd.DataLength             = 0;
    xCmd.DummyCycles            = 0;
    xCmd.DQSMode = HAL_XSPI_DQS_DISABLE;
//    xCmd.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD, };

    /* Clear notification state */
    (void) xTaskNotifyStateClearIndexed(NULL, 1);

    /* Send command */
    xHalStatus = HAL_XSPI_Command_IT(pxXSPI, &xCmd);
  }

  if (xHalStatus != HAL_OK)
  {
    xSuccess = pdFALSE;
  }
  else
  {
    xSuccess = xspi_WaitForCallback(HAL_XSPI_CMD_CPLT_CB_ID, xTimeout);
  }

  if (xSuccess == pdTRUE)
  {
    vTaskDelay(1);
    /* Wait for idle condition (WIP bit should be 0) */
    xSuccess = xspi_OPI_WaitForStatus(pxXSPI, MX66UM_REG_SR_WEL | MX66UM_REG_SR_WIP, 0x0, xTimeout);
  }

  return (xSuccess);
}

#else
static BaseType_t XSPI_WriteEnable        (XSPI_HandleTypeDef *pxXSPI, TickType_t xTimeout);
static HAL_StatusTypeDef XSPI_AutoPollingMemReady(XSPI_HandleTypeDef *pxXSPI);
static BaseType_t XSPI_NOR_OctalDTRModeCfg(XSPI_HandleTypeDef *pxXSPI);

BaseType_t xspi_Init  (XSPI_HandleTypeDef *pxXSPI)
{
  BaseType_t xSuccess = pdTRUE;

  xspi_OpInit(pxXSPI);

  xSuccess = xspi_InitDriver(pxXSPI);

  if(xSuccess == pdTRUE)
  {
    xSuccess =  XSPI_NOR_OctalDTRModeCfg(pxXSPI);
  }

  return xSuccess;
}

BaseType_t xspi_EraseSector(XSPI_HandleTypeDef * pxXSPI,  uint32_t ulAddr, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus;
  XSPI_RegularCmdTypeDef xCmd = { 0 };

  LogDebug("Erasing address 0x%08X sector %d.", ulAddr/MX66UM_SECTOR_SZ);

  XSPI_WriteEnable(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);

  xCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.Instruction        = OPI_CMD_ERASE_SECTOR;
  xCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  xCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  xCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  xCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  xCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  xCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  xCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  xCmd.DataMode           = HAL_XSPI_DATA_NONE;
  xCmd.Address            = ulAddr;
  xCmd.DummyCycles        = 0;
  xCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, xTimeout);

  /* Configure automatic polling mode to wait for end of erase ---------------- */
  if(xHalStatus == HAL_OK)
  {
    xHalStatus = XSPI_AutoPollingMemReady(pxXSPI);
  }

  return xHalStatus == HAL_OK;
}

BaseType_t xspi_WriteAddr(XSPI_HandleTypeDef * pxXSPI, uint32_t ulAddr, const void * pxBuffer, uint32_t ulBufferLen, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus;
  XSPI_RegularCmdTypeDef xCmd = { 0 };

  LogDebug("Writing to address 0x%08X sector %d.", ulAddr, ulAddr/MX66UM_SECTOR_SZ);

  XSPI_WriteEnable(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);

  xCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.Instruction        = OPI_CMD_PROG_PAGE;
  xCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  xCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  xCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  xCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  xCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  xCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  xCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  xCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  xCmd.DataLength         = ulBufferLen;
  xCmd.Address            = ulAddr;
  xCmd.DummyCycles        = 0;
  xCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, xTimeout);

  if(xHalStatus == HAL_OK)
  {
    xHalStatus = HAL_XSPI_Transmit(pxXSPI, pxBuffer, xTimeout);
  }

  /* Configure automatic polling mode to wait for end of program -------------- */
  if(xHalStatus == HAL_OK)
  {
    xHalStatus = XSPI_AutoPollingMemReady(pxXSPI);
  }

  return xHalStatus == HAL_OK;
}
#if 1
BaseType_t xspi_ReadAddr(XSPI_HandleTypeDef * pxXSPI, uint32_t ulAddr, void * pxBuffer, uint32_t ulBufferLen, TickType_t xTimeout )
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;
  BaseType_t xSuccess = pdTRUE;
  XSPI_RegularCmdTypeDef xCmd = { 0 };

  LogDebug("Reading address 0x%08X sector %d.", ulAddr, ulAddr/MX66UM_SECTOR_SZ);

  if (pxXSPI == NULL)
  {
    xSuccess = pdFALSE;
    LogError("pxXSPI is NULL.");
  }

  if (ulAddr >= MX66UM_MEM_SZ_BYTES)
  {
    xSuccess = pdFALSE;
    LogError("Address is out of range.");
  }

  if (pxBuffer == NULL)
  {
    xSuccess = pdFALSE;
    LogError("pxBuffer is NULL.");
  }

  if (ulBufferLen == 0)
  {
    xSuccess = pdFALSE;
    LogError("ulBufferLen is 0.");
  }

  xCmd.OperationType         = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.IOSelect              = HAL_XSPI_SELECT_IO_7_0;

  xCmd.Instruction           = OPI_CMD_IO_DTR_READ;
  xCmd.InstructionMode       = HAL_XSPI_INSTRUCTION_8_LINES;
  xCmd.InstructionWidth      = HAL_XSPI_INSTRUCTION_16_BITS;
  xCmd.InstructionDTRMode    = HAL_XSPI_INSTRUCTION_DTR_ENABLE;

  xCmd.Address               = ulAddr;
  xCmd.AddressMode           = HAL_XSPI_ADDRESS_8_LINES;
  xCmd.AddressWidth          = HAL_XSPI_ADDRESS_32_BITS;
  xCmd.AddressDTRMode        = HAL_XSPI_ADDRESS_DTR_ENABLE;

  xCmd.AlternateBytes        = 0;
  xCmd.AlternateBytesMode    = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.AlternateBytesWidth   = 0;
  xCmd.AlternateBytesDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE;

  xCmd.DataMode              = HAL_XSPI_DATA_8_LINES;
  xCmd.DataLength            = ulBufferLen;
  xCmd.DataDTRMode           = HAL_XSPI_DATA_DTR_ENABLE;

  xCmd.DummyCycles           = DUMMY_CLOCK_CYCLES_READ_OCTAL;
  xCmd.DQSMode               = HAL_XSPI_DQS_ENABLE;

  xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, xTimeout);

  configASSERT(xHalStatus == HAL_OK);

  if (xHalStatus != HAL_OK)
  {
    xSuccess = pdFALSE;
    xspi_AbortTransaction(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);
    LogError("Failed to send OCTAL_IO_DTR_READ_CMD command.");
  }

  if(xHalStatus == HAL_OK)
  {
    xHalStatus = HAL_XSPI_Receive(pxXSPI, pxBuffer, xTimeout);
  }

  configASSERT(xHalStatus == HAL_OK);

  return xHalStatus == HAL_OK;
}
#else
BaseType_t xspi_ReadAddr(XSPI_HandleTypeDef *pxXSPI, uint32_t ulAddr, void *pxBuffer, uint32_t ulBufferLen, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;
  BaseType_t xSuccess = pdTRUE;

  xspi_OpInit(pxXSPI);

  if (pxXSPI == NULL)
  {
    xSuccess = pdFALSE;
    LogError("pxXSPI is NULL.");
  }

  if (ulAddr >= MX66UM_MEM_SZ_BYTES)
  {
    xSuccess = pdFALSE;
    LogError("Address is out of range.");
  }

  if (pxBuffer == NULL)
  {
    xSuccess = pdFALSE;
    LogError("pxBuffer is NULL.");
  }

  if (ulBufferLen == 0)
  {
    xSuccess = pdFALSE;
    LogError("ulBufferLen is 0.");
  }

  /*TODO is there a limit to the number of bytes read? */

  /* Wait for idle condition (WIP bit should be 0) */
  xSuccess = xspi_OPI_WaitForStatus(pxXSPI, MX66UM_REG_SR_WIP, 0x0, MX66UM_DEFAULT_TIMEOUT_MS);

  if (xSuccess != pdTRUE)
  {
    xspi_AbortTransaction(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);
    LogError("Timed out while waiting for OSPI IDLE condition.");
  }
  else
  {
    /* Setup an 8READ transaction */
    XSPI_RegularCmdTypeDef xCmd;

    xCmd.OperationType         = HAL_XSPI_OPTYPE_COMMON_CFG;
    xCmd.IOSelect              = HAL_XSPI_SELECT_IO_7_0;

    xCmd.Instruction           = OCTAL_IO_DTR_READ_CMD;
    xCmd.InstructionMode       = HAL_XSPI_INSTRUCTION_8_LINES;
    xCmd.InstructionWidth      = HAL_XSPI_INSTRUCTION_16_BITS;
    xCmd.InstructionDTRMode    = HAL_XSPI_INSTRUCTION_DTR_ENABLE;

    xCmd.Address               = ulAddr;
    xCmd.AddressMode           = HAL_XSPI_ADDRESS_8_LINES;
    xCmd.AddressWidth          = HAL_XSPI_ADDRESS_32_BITS;
    xCmd.AddressDTRMode        = HAL_XSPI_ADDRESS_DTR_ENABLE;

    xCmd.AlternateBytes        = 0;
    xCmd.AlternateBytesMode    = HAL_XSPI_ALT_BYTES_NONE;
    xCmd.AlternateBytesWidth   = 0;
    xCmd.AlternateBytesDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE;

    xCmd.DataMode              = HAL_XSPI_DATA_8_LINES;
    xCmd.DataLength            = ulBufferLen;
    xCmd.DataDTRMode           = HAL_XSPI_DATA_DTR_ENABLE;

    xCmd.DummyCycles           = DUMMY_CLOCK_CYCLES_READ_OCTAL;
    xCmd.DQSMode               = HAL_XSPI_DQS_ENABLE;

    /* Clear notification state */
    (void) xTaskNotifyStateClearIndexed(NULL, 1);

    /* Send command */
    xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, MX66UM_DEFAULT_TIMEOUT_MS);

    if (xHalStatus != HAL_OK)
    {
      xSuccess = pdFALSE;
      xspi_AbortTransaction(pxXSPI, MX66UM_DEFAULT_TIMEOUT_MS);
      LogError("Failed to send 8READ command.");
    }
  }

  if (xSuccess == pdTRUE)
  {
    /* Clear notification state */
    (void) xTaskNotifyStateClearIndexed(NULL, 1);

    xHalStatus = HAL_XSPI_Receive_IT(pxXSPI, pxBuffer);

    /* Wait for receive op to complete */
    if (xHalStatus == HAL_OK)
    {
      xSuccess = xspi_WaitForCallback(HAL_XSPI_RX_CPLT_CB_ID, xTimeout);
    }
  }

  return (xSuccess);
}
#endif
/**
 * @brief  This function send a Write Enable and wait it is effective.
 * @param  pxXSPI: XSPI handle
 * @retval None
 */
static BaseType_t XSPI_WriteEnable(XSPI_HandleTypeDef *pxXSPI, TickType_t xTimeout)
{
  HAL_StatusTypeDef xHalStatus;
  XSPI_RegularCmdTypeDef xCmd = { 0 };
  uint8_t reg[2];

  /* Enable write operations ------------------------------------------ */
  xCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.Instruction        = OPI_CMD_WRITE_ENABLE;
  xCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  xCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  xCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  xCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  xCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.DataMode           = HAL_XSPI_DATA_NONE;
  xCmd.DummyCycles        = 0;
  xCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);

  /* Configure automatic polling mode to wait for write enabling ---- */
  xCmd.Instruction    = OPI_CMD_READ_STATUS_REG;
  xCmd.Address        = 0x0;
  xCmd.AddressMode    = HAL_XSPI_ADDRESS_8_LINES;
  xCmd.AddressWidth   = HAL_XSPI_ADDRESS_32_BITS;
  xCmd.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  xCmd.DataMode       = HAL_XSPI_DATA_NONE;//HAL_XSPI_DATA_8_LINES;
  xCmd.DataDTRMode    = HAL_XSPI_DATA_DTR_ENABLE;
  xCmd.DataLength     = 2;
  xCmd.DummyCycles    = DUMMY_CLOCK_CYCLES_READ_OCTAL;
  xCmd.DQSMode        = HAL_XSPI_DQS_ENABLE;
#if 1
  do
  {
    if(xHalStatus == HAL_OK)
    {
      xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    }

    if(xHalStatus == HAL_OK)
    {
      xHalStatus = HAL_XSPI_Receive(pxXSPI, reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    }

  } while (((reg[0] & WRITE_ENABLE_MASK_VALUE) != WRITE_ENABLE_MATCH_VALUE) && (xHalStatus == HAL_OK));

  return xHalStatus == HAL_OK;
#else
  (void) xTaskNotifyStateClearIndexed(NULL, 1);

  xHalStatus = HAL_XSPI_Command_IT(pxXSPI, &xCmd);

  configASSERT(xHalStatus == HAL_OK);

  if (xHalStatus == HAL_OK)
  {
    if (xspi_WaitForCallback(HAL_XSPI_CMD_CPLT_CB_ID, xTimeout) != pdTRUE)
    {
      xHalStatus = HAL_TIMEOUT;
    }
  }

  return (xHalStatus == HAL_OK);
#endif
}

/**
 * @brief  This function read the SR of the memory and wait the EOP.
 * @param  pxXSPI: XSPI handle
 * @retval None
 */
static HAL_StatusTypeDef XSPI_AutoPollingMemReady(XSPI_HandleTypeDef *pxXSPI)
{
  HAL_StatusTypeDef xHalStatus = HAL_OK;

  XSPI_RegularCmdTypeDef xCmd = { 0 };
  uint8_t reg[2];

  /* Configure automatic polling mode to wait for memory ready ------ */
  xCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.Instruction        = OPI_CMD_READ_STATUS_REG;
  xCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  xCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  xCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  xCmd.Address            = HAL_XSPI_ADDRESS_NONE;
  xCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  xCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  xCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  xCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  xCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  xCmd.DataLength         = 2;
  xCmd.DummyCycles        = DUMMY_CLOCK_CYCLES_READ_OCTAL;
  xCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  do
  {
    xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);

    configASSERT(xHalStatus == HAL_OK);

    if(xHalStatus == HAL_OK)
    {
      xHalStatus = HAL_XSPI_Receive(pxXSPI, reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    }

  } while (((reg[0] & MEMORY_READY_MASK_VALUE) != MEMORY_READY_MATCH_VALUE) && (xHalStatus == HAL_OK));

  configASSERT(xHalStatus == HAL_OK);

  return xHalStatus;
}

/**
 * @brief  This function configure the memory in Octal DTR mode.
 * @param  pxXSPI: XSPI handle
 * @retval None
 */
static BaseType_t XSPI_NOR_OctalDTRModeCfg(XSPI_HandleTypeDef *pxXSPI)
{
  uint8_t reg = 0;
  XSPI_RegularCmdTypeDef xCmd = { 0 };
  XSPI_AutoPollingTypeDef sConfig = { 0 };
  HAL_StatusTypeDef xHalStatus;

  xCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  xCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  xCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  xCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  xCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  xCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  xCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  xCmd.DummyCycles        = 0;
  xCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  sConfig.MatchMode       = HAL_XSPI_MATCH_MODE_AND;
  sConfig.AutomaticStop   = HAL_XSPI_AUTOMATIC_STOP_ENABLE;
  sConfig.IntervalTime    = AUTO_POLLING_INTERVAL;

  /* Enable write operations */
  xCmd.Instruction        = SPI_CMD_WRITE_ENABLE;
  xCmd.DataMode           = HAL_XSPI_DATA_NONE;
  xCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;

  xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);

  /* Reconfigure XSPI to automatic polling mode to wait for write enabling */
  sConfig.MatchMask  = 0x02;
  sConfig.MatchValue = 0x02;

  xCmd.Instruction = SPI_CMD_READ_STATUS_REG;
  xCmd.DataMode    = HAL_XSPI_DATA_1_LINE;
  xCmd.DataLength  = 1;

  if(xHalStatus == HAL_OK)
  {
    xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  if(xHalStatus == HAL_OK)
  {
    xHalStatus = HAL_XSPI_AutoPolling(pxXSPI, &sConfig, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  /* Write Configuration register 2 (with Octal I/O SPI protocol) */
  xCmd.Instruction  = SPI_CMD_WRITE_CFG_REG_2;
  xCmd.AddressMode  = HAL_XSPI_ADDRESS_1_LINE;
  xCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;

  xCmd.Address = 0;
  reg = 0x2;

  if(xHalStatus == HAL_OK)
  {
    xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  if(xHalStatus == HAL_OK)
  {
    xHalStatus = HAL_XSPI_Transmit(pxXSPI, &reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  xCmd.Instruction = SPI_CMD_READ_STATUS_REG;
  xCmd.DataMode    = HAL_XSPI_DATA_1_LINE;
  xCmd.DataLength  = 1;

  if(xHalStatus == HAL_OK)
  {
    xHalStatus = HAL_XSPI_Command(pxXSPI, &xCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  if(xHalStatus == HAL_OK)
  {
    xHalStatus = HAL_XSPI_AutoPolling(pxXSPI, &sConfig, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  return xHalStatus == HAL_OK;
}
#endif
