
#include "logging_levels.h"
#define LOG_LEVEL    LOG_DEBUG
#include "logging.h"
#include "FreeRTOS.h"
#include "task.h"

#include "xspi_nor_mx66umxx45g.h"


static BaseType_t XSPI_WriteEnable        (XSPI_HandleTypeDef *hxspi);
static HAL_StatusTypeDef XSPI_AutoPollingMemReady(XSPI_HandleTypeDef *hxspi);
static BaseType_t XSPI_NOR_OctalDTRModeCfg(XSPI_HandleTypeDef *hxspi);

BaseType_t xspi_Init  (XSPI_HandleTypeDef *hxspi)
{
  return XSPI_NOR_OctalDTRModeCfg(hxspi);
}

BaseType_t xspi_EraseSector(XSPI_HandleTypeDef * pxXSPI,  uint32_t ulAddr, TickType_t xTimeout)
{
  HAL_StatusTypeDef status;
  XSPI_RegularCmdTypeDef sCommand = { 0 };

  XSPI_WriteEnable(pxXSPI);

  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction        = OCTAL_SECTOR_ERASE_CMD;
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

  status = HAL_XSPI_Command(pxXSPI, &sCommand, xTimeout);

  /* Configure automatic polling mode to wait for end of erase ---------------- */
  if(status == HAL_OK)
  {
    status = XSPI_AutoPollingMemReady(pxXSPI);
  }

  return status == HAL_OK;
}

BaseType_t xspi_WriteAddr(XSPI_HandleTypeDef * pxXSPI,
        uint32_t ulAddr,
        void * pxBuffer,
        uint32_t ulBufferLen,
        TickType_t xTimeout)
{
  HAL_StatusTypeDef status;
  XSPI_RegularCmdTypeDef sCommand = { 0 };

  XSPI_WriteEnable(pxXSPI);

  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction        = OCTAL_PAGE_PROG_CMD;
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

  status = HAL_XSPI_Command(pxXSPI, &sCommand, xTimeout);

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Transmit(pxXSPI, pxBuffer, xTimeout);
  }

  /* Configure automatic polling mode to wait for end of program -------------- */
  if(status == HAL_OK)
  {
    status = XSPI_AutoPollingMemReady(pxXSPI);
  }

  return status == HAL_OK;
}

BaseType_t xspi_ReadAddr(XSPI_HandleTypeDef * pxXSPI,
        uint32_t ulAddr,
        void * pxBuffer,
        uint32_t ulBufferLen,
        TickType_t xTimeout )
{
  HAL_StatusTypeDef status;
  XSPI_RegularCmdTypeDef sCommand = { 0 };

  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction        = OCTAL_IO_DTR_READ_CMD;
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

  status = HAL_XSPI_Command(pxXSPI, &sCommand, xTimeout);

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Receive(pxXSPI, pxBuffer, xTimeout);
  }

  return status == HAL_OK;
}

/**
 * @brief  This function send a Write Enable and wait it is effective.
 * @param  hxspi: XSPI handle
 * @retval None
 */
static BaseType_t XSPI_WriteEnable(XSPI_HandleTypeDef *hxspi)
{
  HAL_StatusTypeDef status;
  XSPI_RegularCmdTypeDef sCommand = { 0 };
  uint8_t reg[2];

  /* Enable write operations ------------------------------------------ */
  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction        = OCTAL_WRITE_ENABLE_CMD;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode           = HAL_XSPI_DATA_NONE;
  sCommand.DummyCycles        = 0;
  sCommand.DQSMode            = HAL_XSPI_DQS_DISABLE;

  status = HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);

  /* Configure automatic polling mode to wait for write enabling ---- */
  sCommand.Instruction    = OCTAL_READ_STATUS_REG_CMD;
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
      status = HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    }

    if(status == HAL_OK)
    {
      status = HAL_XSPI_Receive(hxspi, reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    }

  } while (((reg[0] & WRITE_ENABLE_MASK_VALUE) != WRITE_ENABLE_MATCH_VALUE) && (status == HAL_OK));

  return status == HAL_OK;
}

/**
 * @brief  This function read the SR of the memory and wait the EOP.
 * @param  hxspi: XSPI handle
 * @retval None
 */
static HAL_StatusTypeDef XSPI_AutoPollingMemReady(XSPI_HandleTypeDef *hxspi)
{
  HAL_StatusTypeDef status;
  XSPI_RegularCmdTypeDef sCommand = { 0 };
  uint8_t reg[2];

  /* Configure automatic polling mode to wait for memory ready ------ */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction = OCTAL_READ_STATUS_REG_CMD;
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
    status = HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);

    if(status == HAL_OK)
    {
      status = HAL_XSPI_Receive(hxspi, reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    }

  } while (((reg[0] & MEMORY_READY_MASK_VALUE) != MEMORY_READY_MATCH_VALUE) && (status == HAL_OK));

  return status;
}

/**
 * @brief  This function configure the memory in Octal DTR mode.
 * @param  hxspi: XSPI handle
 * @retval None
 */
static BaseType_t XSPI_NOR_OctalDTRModeCfg(XSPI_HandleTypeDef *hxspi)
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
  sCommand.Instruction        = WRITE_ENABLE_CMD;
  sCommand.DataMode           = HAL_XSPI_DATA_NONE;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_NONE;

  status = HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);

  /* Reconfigure XSPI to automatic polling mode to wait for write enabling */
  sConfig.MatchMask  = 0x02;
  sConfig.MatchValue = 0x02;

  sCommand.Instruction = READ_STATUS_REG_CMD;
  sCommand.DataMode    = HAL_XSPI_DATA_1_LINE;
  sCommand.DataLength  = 1;

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  if(status == HAL_OK)
  {
    status = HAL_XSPI_AutoPolling(hxspi, &sConfig, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  /* Write Configuration register 2 (with Octal I/O SPI protocol) */
  sCommand.Instruction  = WRITE_CFG_REG_2_CMD;
  sCommand.AddressMode  = HAL_XSPI_ADDRESS_1_LINE;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;

  sCommand.Address = 0;
  reg = 0x2;

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Transmit(hxspi, &reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  sCommand.Instruction = READ_STATUS_REG_CMD;
  sCommand.DataMode    = HAL_XSPI_DATA_1_LINE;
  sCommand.DataLength  = 1;

  if(status == HAL_OK)
  {
    status = HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  if(status == HAL_OK)
  {
    status = HAL_XSPI_AutoPolling(hxspi, &sConfig, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }

  return status == HAL_OK;
}
