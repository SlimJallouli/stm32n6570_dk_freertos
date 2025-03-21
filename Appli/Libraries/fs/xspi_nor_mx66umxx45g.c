
#include "logging_levels.h"
#define LOG_LEVEL    LOG_NONE
#include "logging.h"
#include "FreeRTOS.h"
//#include "task.h"

#include "xspi_nor_mx66umxx45g.h"

static BaseType_t XSPI_WriteEnable        (XSPI_HandleTypeDef *pxXSPI);
static BaseType_t XSPI_AutoPollingMemReady(XSPI_HandleTypeDef *pxXSPI);
static BaseType_t XSPI_NOR_OctalDTRModeCfg(XSPI_HandleTypeDef *pxXSPI);


#include <stdio.h>
#include <stdint.h>
#include <string.h>

BaseType_t xspi_Init        (XSPI_HandleTypeDef *pxXSPI)
{
  return XSPI_NOR_OctalDTRModeCfg(pxXSPI);
}

BaseType_t xspi_EraseSector (XSPI_HandleTypeDef *pxXSPI, uint32_t ulAddr, TickType_t xTimeout)
{
  HAL_StatusTypeDef status;
  XSPI_RegularCmdTypeDef sCommand = { 0 };

  LogDebug("Erasing address 0x%08X ",ulAddr);

  XSPI_WriteEnable(pxXSPI);

  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction        = OPI_CMD_PROG_PAGE;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataMode           = HAL_XSPI_DATA_NONE;
  sCommand.Address            = ulAddr;
  sCommand.DummyCycles        = 0;
  sCommand.DQSMode            = HAL_XSPI_DQS_ENABLE;

  status = HAL_XSPI_Command(pxXSPI, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  configASSERT(status == HAL_OK);

  /* Configure automatic polling mode to wait for end of erase ---------------- */
  XSPI_AutoPollingMemReady(pxXSPI);

  return status == HAL_OK;
}

BaseType_t xspi_WriteAddr   (XSPI_HandleTypeDef *pxXSPI, uint32_t ulAddr, const void *pxBuffer, uint32_t ulBufferLen, TickType_t xTimeout)
{
  HAL_StatusTypeDef status;
  XSPI_RegularCmdTypeDef sCommand = { 0 };

  LogDebug("Writing address 0x%08X, size  0x%08X",ulAddr, ulBufferLen);

  configASSERT((ulBufferLen%MX66UM_READ_SIZE)==0);

  XSPI_WriteEnable(pxXSPI);

  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction        = OPI_CMD_PROG_PAGE;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCommand.DataLength         = ulBufferLen;
  sCommand.Address            = ulAddr;
  sCommand.DummyCycles        = 0;
  sCommand.DQSMode            = HAL_XSPI_DQS_ENABLE;

  status = HAL_XSPI_Command(pxXSPI, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  configASSERT(status == HAL_OK);

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Transmit(pxXSPI, pxBuffer, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    configASSERT(status == HAL_OK);
  }

  /* Configure automatic polling mode to wait for end of program -------------- */
  XSPI_AutoPollingMemReady(pxXSPI);

  return status == HAL_OK;
}

BaseType_t xspi_ReadAddr    (XSPI_HandleTypeDef *pxXSPI, uint32_t ulAddr, void *pxBuffer, uint32_t ulBufferLen, TickType_t xTimeout)
{
  HAL_StatusTypeDef status;
  XSPI_RegularCmdTypeDef sCommand = { 0 };

  LogDebug("Reading address 0x%08X, size  0x%08X",ulAddr, ulBufferLen);
  configASSERT((ulBufferLen%MX66UM_READ_SIZE)==0);

  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction        = OPI_CMD_IO_DTR_READ;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCommand.DataLength         = ulBufferLen;
  sCommand.Address            = ulAddr;
  sCommand.DummyCycles        = DUMMY_CLOCK_CYCLES_READ_OCTAL;
  sCommand.DQSMode            = HAL_XSPI_DQS_ENABLE;

  status = HAL_XSPI_Command(pxXSPI, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  configASSERT(status == HAL_OK);

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Receive(pxXSPI, pxBuffer, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    configASSERT(status == HAL_OK);
  }

  return status == HAL_OK;
}

/**
 * @brief  This function send a Write Enable and wait it is effective.
 * @param  pxXSPI: XSPI handle
 * @retval None
 */
static BaseType_t XSPI_WriteEnable(XSPI_HandleTypeDef *pxXSPI)
{
  HAL_StatusTypeDef status;
  XSPI_RegularCmdTypeDef sCommand = { 0 };
  uint8_t reg[2];

  /* Enable write operations ------------------------------------------ */
  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction        = OPI_CMD_WRITE_ENABLE;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode           = HAL_XSPI_DATA_NONE;
  sCommand.DummyCycles        = 0;
  sCommand.DQSMode            = HAL_XSPI_DQS_DISABLE;

  status = HAL_XSPI_Command(pxXSPI, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  configASSERT(status == HAL_OK);

  /* Configure automatic polling mode to wait for write enabling ---- */
  sCommand.Instruction    = OPI_CMD_READ_STATUS_REG;
  sCommand.Address        = 0x0;
  sCommand.AddressMode    = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth   = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.DataMode       = HAL_XSPI_DATA_8_LINES;
  sCommand.DataDTRMode    = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength     = 2;
  sCommand.DummyCycles    = DUMMY_CLOCK_CYCLES_READ_OCTAL;
  sCommand.DQSMode        = HAL_XSPI_DQS_ENABLE;

  do
  {
    if(status == HAL_OK)
    {
      status = HAL_XSPI_Command(pxXSPI, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
      configASSERT(status == HAL_OK);
    }

    if(status == HAL_OK)
    {
      status = HAL_XSPI_Receive(pxXSPI, reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
      configASSERT(status == HAL_OK);
    }

  } while (((reg[0] & WRITE_ENABLE_MASK_VALUE) != WRITE_ENABLE_MATCH_VALUE) && (status == HAL_OK));

  return status == HAL_OK;
}

/**
 * @brief  This function read the SR of the memory and wait the EOP.
 * @param  pxXSPI: XSPI handle
 * @retval None
 */
static BaseType_t XSPI_AutoPollingMemReady(XSPI_HandleTypeDef *pxXSPI)
{
  HAL_StatusTypeDef status;
  XSPI_RegularCmdTypeDef sCommand = { 0 };
  uint8_t reg[2];

  /* Configure automatic polling mode to wait for memory ready ------ */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction = OPI_CMD_READ_STATUS_REG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_16_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCommand.Address = 0x0;
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode = HAL_XSPI_DATA_8_LINES;
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength = 2;
  sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_READ_OCTAL;
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;

  do
  {
    status = HAL_XSPI_Command(pxXSPI, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    configASSERT(status == HAL_OK);

    if(status == HAL_OK)
    {
      status = HAL_XSPI_Receive(pxXSPI, reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
      configASSERT(status == HAL_OK);
    }

  } while (((reg[0] & MEMORY_READY_MASK_VALUE) != MEMORY_READY_MATCH_VALUE) && (status == HAL_OK));

  return status == HAL_OK;
}

/**
 * @brief  This function configure the memory in Octal DTR mode.
 * @param  pxXSPI: XSPI handle
 * @retval None
 */
static BaseType_t XSPI_NOR_OctalDTRModeCfg(XSPI_HandleTypeDef *pxXSPI)
{
  uint8_t reg = 0;
  XSPI_RegularCmdTypeDef sCommand = { 0 };
  XSPI_AutoPollingTypeDef sConfig = { 0 };
  HAL_StatusTypeDef status;

  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCommand.DummyCycles        = 0;
  sCommand.DQSMode            = HAL_XSPI_DQS_DISABLE;
  sConfig.MatchMode           = HAL_XSPI_MATCH_MODE_AND;
  sConfig.AutomaticStop       = HAL_XSPI_AUTOMATIC_STOP_ENABLE;
  sConfig.IntervalTime        = 0x10;

  /* Enable write operations */
  sCommand.Instruction        = SPI_CMD_WRITE_ENABLE;
  sCommand.DataMode           = HAL_XSPI_DATA_NONE;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_NONE;

  status = HAL_XSPI_Command(pxXSPI, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  configASSERT(status == HAL_OK);

  /* Reconfigure XSPI to automatic polling mode to wait for write enabling */
  sConfig.MatchMask  = 0x02;
  sConfig.MatchValue = 0x02;

  sCommand.Instruction = SPI_CMD_READ_STATUS_REG;
  sCommand.DataMode    = HAL_XSPI_DATA_1_LINE;
  sCommand.DataLength  = 1;

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Command(pxXSPI, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    configASSERT(status == HAL_OK);
  }

  if(status == HAL_OK)
  {
    status = HAL_XSPI_AutoPolling(pxXSPI, &sConfig, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    configASSERT(status == HAL_OK);
  }

  /* Write Configuration register 2 (with Octal I/O SPI protocol) */
  sCommand.Instruction  = SPI_CMD_WRITE_CFG_REG_2;
  sCommand.AddressMode  = HAL_XSPI_ADDRESS_1_LINE;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;

  sCommand.Address = 0;
  reg = 0x2;

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Command(pxXSPI, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    configASSERT(status == HAL_OK);
  }

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Transmit(pxXSPI, &reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    configASSERT(status == HAL_OK);
  }

  sCommand.Instruction = SPI_CMD_READ_STATUS_REG;
  sCommand.DataMode    = HAL_XSPI_DATA_1_LINE;
  sCommand.DataLength  = 1;

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Command(pxXSPI, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    configASSERT(status == HAL_OK);
  }

  if(status == HAL_OK)
  {
    status = HAL_XSPI_AutoPolling(pxXSPI, &sConfig, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    configASSERT(status == HAL_OK);
  }

  return status == HAL_OK;
}

