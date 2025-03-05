
#ifndef __XSPI_H__
#define __XSPI_H__
#include "main.h"

// https://www.mouser.com/pdfDocs/MX66UM1G45G,18V,1Gb,v11.pdf

/*
 *  1 Gbit = 128 MByte
 *  2048 Blocks of 64KByte
 *  16 4096 byte Sectors per Block
 */
#define MX66UM_BLOCK_SZ              ( 64 * 1024 )
#define MX66UM_SECTOR_SZ             ( 4  * 1024 )
#define MX66UM_NUM_BLOCKS            ( 2  * 1024 )
#define MX66UM_SECTORS_PER_BLOCK     ( 16 )
#define MX66UM_NUM_SECTORS           ( MX66UM_NUM_BLOCKS * MX66UM_SECTORS_PER_BLOCK )
#define MX66UM_MEM_SZ_BYTES          ( 1024 * MX66UM_BLOCK_SZ )

#define XSPI_START_ADDRESS           ( 10 * MX66UM_BLOCK_SZ )

#define MX66UM_NUM_SECTOR_USABLE     ( MX66UM_NUM_BLOCKS - 10 )
#define MX66UM_MEM_SZ_USABLE         ( MX66UM_NUM_SECTOR_USABLE * MX66UM_SECTOR_SZ )

#define MX66UM_DEFAULT_TIMEOUT_MS    ( 1000 )

/* Flash commands */
#define OCTAL_IO_READ_CMD           0xEC13
#define OCTAL_IO_DTR_READ_CMD       0xEE11
#define OCTAL_PAGE_PROG_CMD         0x12ED
#define OCTAL_READ_STATUS_REG_CMD   0x05FA
#define OCTAL_SECTOR_ERASE_CMD      0x21DE
#define OCTAL_WRITE_ENABLE_CMD      0x06F9
#define READ_STATUS_REG_CMD         0x05
#define WRITE_CFG_REG_2_CMD         0x72
#define WRITE_ENABLE_CMD            0x06

/* Dummy clocks cycles */
#define DUMMY_CLOCK_CYCLES_READ            6
#define DUMMY_CLOCK_CYCLES_READ_REG        4
#define DUMMY_CLOCK_CYCLES_READ_OCTAL      6

/* Auto-polling values */
#define WRITE_ENABLE_MATCH_VALUE    0x02
#define WRITE_ENABLE_MASK_VALUE     0x02

#define MEMORY_READY_MATCH_VALUE    0x00
#define MEMORY_READY_MASK_VALUE     0x01

#define AUTO_POLLING_INTERVAL       0x10

/* Memory registers address */
#define CONFIG_REG2_ADDR1           0x0000000
#define CR2_DTR_OPI_ENABLE          0x02

#define CONFIG_REG2_ADDR3           0x00000300
#define CR2_DUMMY_CYCLES_66MHZ      0x07

/* Memory delay */
#define MEMORY_REG_WRITE_DELAY      40
#define MEMORY_PAGE_PROG_DELAY      2

#define MX66UM_PROGRAM_FIFO_LEN      ( 256 )

#define MX66UM_WRITE_TIMEOUT_MS      ( 10 * 1000 )
#define MX66UM_ERASE_TIMEOUT_MS      ( 10 * 1000 )
#define MX66UM_READ_TIMEOUT_MS       ( 10 * 1000 )

/* End address of the OSPI memory */

BaseType_t xspi_Init        (XSPI_HandleTypeDef *hxspi);
BaseType_t xspi_EraseSector (XSPI_HandleTypeDef * pxXSPI,
        uint32_t ulAddr,
        TickType_t xTimeout);
BaseType_t xspi_WriteAddr   (XSPI_HandleTypeDef * pxXSPI,
        uint32_t ulAddr,
        void * pxBuffer,
        uint32_t ulBufferLen,
        TickType_t xTimeout);
BaseType_t xspi_ReadAddr    (XSPI_HandleTypeDef * pxXSPI,
        uint32_t ulAddr,
        void * pxBuffer,
        uint32_t ulBufferLen,
        TickType_t xTimeout);

#endif /* __XSPI_H__ */
