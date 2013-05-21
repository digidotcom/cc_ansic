/*
*********************************************************************************************************
*                                             uC/FS V4
*                                     The Embedded File System
*
*                         (c) Copyright 2013-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved. Protected by international copyright laws.
*
*               uC/FS is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      FILE SYSTEM DEVICE DRIVER
*
*                                             SD/MMC CARD
*                                              CARD MODE
*
*                                BOARD SUPPORT PACKAGE (BSP) FUNCTIONS
*
*                                        Freescale Kinetis K60
*                                               on the
*
*                                        Freescale TWR-K60N512
*                                          Evaluation Board
*
* Filename      : fs_dev_sd_card_bsp.c
* Version       : v4.05.03.00
* Programmer(s) : DC
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    FS_DEV_SD_CARD_BSP_MODULE

#include <includes.h>

#if ((APP_CFG_FS_EN         == DEF_ENABLED) && (APP_CFG_FS_SD_CARD_EN == DEF_ENABLED))
#include  <fs_os.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
                                                                /* ---------------- SIM_SOPT2 SDHC CLK ---------------- */
#define  K60N512_SIM_SOPT2_CORE_CLK                     0u
#define  K60N512_SIM_SOPT2_PLL_FLL_CLK                  1u
#define  K60N512_SIM_SOPT2_OSCERCLK_CLK                 2u
#define  K60N512_SIM_SOPT2_EXTERNAL_CLK                 3u

#define  K60N512_AIPS_ALL_ON_MASK                       0x77777777u

#define  K60N512_PRESCALER_AMOUNT                       9u
#define  K60N512_DIVIDER_AMOUNT                         16u

#define  K60N512_SDHC_INIT_CLK_FREQ                     400000u
#define  K60_TIMEOUT_MIN_VALUE                          0x3FFFFuL
#define  K60_TIMEOUT_MAX_VALUE                          0xFFFFFFFFuL


/*
*********************************************************************************************************
*                                          REGISTER DEFINES
*********************************************************************************************************
*/

#define  K60N512_REG_SDHC_BASE_ADDR                     0x400B1000u


/*
*********************************************************************************************************
*                                        REGISTER BIT DEFINES
*********************************************************************************************************
*/
                                                                /* ------------------- SDHC XFERTYP ------------------- */
#define  K60N512_BIT_XFERTYP_DATA_PRES_SEL              DEF_BIT_21
#define  K60N512_BIT_XFERTYP_CIC_EN                     DEF_BIT_20
#define  K60N512_BIT_XFERTYP_CRC_CHECK_EN               DEF_BIT_19

#define  K60N512_BIT_XFERTYP_RESPONSE_TYPE_136          DEF_BIT_16
#define  K60N512_BIT_XFERTYP_RESPONSE_TYPE_48           DEF_BIT_17
#define  K60N512_BIT_XFERTYP_RESPONSE_TYPE_48_CHK_BUSY (DEF_BIT_17 | DEF_BIT_16)

#define  K60N512_BIT_XFERTYP_MSB_SEL                    DEF_BIT_05
#define  K60N512_BIT_XFERTYP_DATA_TRANS_DIR             DEF_BIT_04
#define  K60N512_BIT_XFERTYP_AC12_EN                    DEF_BIT_02
#define  K60N512_BIT_XFERTYP_BLK_CNT_EN                 DEF_BIT_01
#define  K60N512_BIT_XFERTYP_DMA_EN                     DEF_BIT_00
                                                                
                                                                /* -------------------- SDHC PRSSTAT ------------------ */
#define  K60N512_BIT_PRSSTAT_CMD_LINE_SIG_LVL           DEF_BIT_23
#define  K60N512_BIT_PRSSTAT_CARD_INSERTED              DEF_BIT_16
#define  K60N512_BIT_PRSSTAT_BUFF_READ_EN               DEF_BIT_11
#define  K60N512_BIT_PRSSTAT_BUFF_WRITE_EN              DEF_BIT_10
#define  K60N512_BIT_PRSSTAT_READ_TRANSFER_ACTIVE       DEF_BIT_09
#define  K60N512_BIT_PRSSTAT_WRITE_TRANSFER_ACTIVE      DEF_BIT_08
#define  K60N512_BIT_PRSSTAT_SD_CLK_GATED_OFF           DEF_BIT_07
#define  K60N512_BIT_PRSSTAT_PER_CLK_GATED_OFF          DEF_BIT_06
#define  K60N512_BIT_PRSSTAT_SYS_CLK_GATED_OFF          DEF_BIT_05
#define  K60N512_BIT_PRSSTAT_BUS_CLK_GATED_OFF          DEF_BIT_04
#define  K60N512_BIT_PRSSTAT_SD_CLK_STABLE              DEF_BIT_03
#define  K60N512_BIT_PRSSTAT_DATA_LINE_ACTIVE           DEF_BIT_02
#define  K60N512_BIT_PRSSTAT_DAT_INHIBIT                DEF_BIT_01
#define  K60N512_BIT_PRSSTAT_CMD_INHIBIT                DEF_BIT_00

                                                                /* -------------------- SDHC PROCTL ------------------- */
#define  K60N512_BIT_PROCTL_WAKE_EN_CARD_REM            DEF_BIT_26
#define  K60N512_BIT_PROCTL_WAKE_EN_CARD_INSERT         DEF_BIT_25
#define  K60N512_BIT_PROCTL_WAKE_EN_CARD_INT            DEF_BIT_24
#define  K60N512_BIT_PROCTL_INT_AT_BLK_GAP              DEF_BIT_19
#define  K60N512_BIT_PROCTL_READ_WAIT_CNTRL             DEF_BIT_18
#define  K60N512_BIT_PROCTL_CONTINUE_REQUEST            DEF_BIT_17
#define  K60N512_BIT_PROCTL_STOP_AT_BLK_GAP_REQ         DEF_BIT_16

#define  K60N512_BIT_PROCTL_DMA_SEL_SIMPLE_DMA         (DEF_BIT_09 | DEF_BIT_08)
#define  K60N512_BIT_PROCTL_DMA_SEL_ADMA1               DEF_BIT_08
#define  K60N512_BIT_PROCTL_DMA_SEL_ADMA2               DEF_BIT_09

#define  K60N512_BIT_PROCTL_CARD_DETECT_SIG_SEL         DEF_BIT_07
#define  K60N512_BIT_PROCTL_CARD_DETECT_TEST_LVL        DEF_BIT_06

#define  K60N512_BIT_PROCTL_ENDIAN_MODE_BIG            (DEF_BIT_05 | DEF_BIT_04)
#define  K60N512_BIT_PROCTL_ENDIAN_MODE_HALF            DEF_BIT_04
#define  K60N512_BIT_PROCTL_ENDIAN_MODE_LITTLE          DEF_BIT_05

#define  K60N512_BIT_PROCTL_DAT3_CARD_DETECT_PIN        DEF_BIT_03

#define  K60N512_BIT_PROCTL_DATA_TRANS_WIDTH_1_BIT     (DEF_BIT_02 | DEF_BIT_01)
#define  K60N512_BIT_PROCTL_DATA_TRANS_WIDTH_4_BIT      DEF_BIT_01
#define  K60N512_BIT_PROCTL_DATA_TRANS_WIDTH_8_BIT      DEF_BIT_02

#define  K60N512_BIT_PROCTL_LED_CONTROL                 DEF_BIT_00

                                                                /* -------------------- SDHC SYSCTL ------------------- */
#define  K60N512_BIT_SYSCTL_INITIALIZATION_ACTIVE       DEF_BIT_27
#define  K60N512_BIT_SYSCTL_RESET_DAT_LINE              DEF_BIT_26
#define  K60N512_BIT_SYSCTL_RESET_CMD_LINE              DEF_BIT_25
#define  K60N512_BIT_SYSCTL_SOFTWARE_RESET_ALL          DEF_BIT_24
#define  K60N512_BIT_SYSCTL_SD_CLK_EN                   DEF_BIT_03
#define  K60N512_BIT_SYSCTL_PERIPH_CLK_EN               DEF_BIT_02
#define  K60N512_BIT_SYSCTL_SYS_CLK_EN                  DEF_BIT_01
#define  K60N512_BIT_SYSCTL_IPG_CLK_EN                  DEF_BIT_00

                                                                /* ------------------- SDHC IRQSTAT ------------------- */
#define  K60N512_BIT_IRQSTAT_DMA_ERR                    DEF_BIT_28
#define  K60N512_BIT_IRQSTAT_AC12_ERR                   DEF_BIT_24
#define  K60N512_BIT_IRQSTAT_DATA_END_BIT_ERR           DEF_BIT_22
#define  K60N512_BIT_IRQSTAT_DATA_CRC_ERR               DEF_BIT_21
#define  K60N512_BIT_IRQSTAT_DATA_TIMEOUT_ERR           DEF_BIT_20
#define  K60N512_BIT_IRQSTAT_CMD_INDEX_ERR              DEF_BIT_19
#define  K60N512_BIT_IRQSTAT_CMD_END_BIT_ERR            DEF_BIT_18
#define  K60N512_BIT_IRQSTAT_CMD_CRC_ERR                DEF_BIT_17
#define  K60N512_BIT_IRQSTAT_CMD_TIMEOUT_ERR            DEF_BIT_16
#define  K60N512_BIT_IRQSTAT_CARD_INTERRUPT             DEF_BIT_08
#define  K60N512_BIT_IRQSTAT_CARD_REMOVAL               DEF_BIT_07
#define  K60N512_BIT_IRQSTAT_CARD_INSERTION             DEF_BIT_06
#define  K60N512_BIT_IRQSTAT_BUF_READ_READY             DEF_BIT_05
#define  K60N512_BIT_IRQSTAT_BUF_WRITE_READY            DEF_BIT_04
#define  K60N512_BIT_IRQSTAT_DMA_INT                    DEF_BIT_03
#define  K60N512_BIT_IRQSTAT_BLK_GAP_EVENT              DEF_BIT_02
#define  K60N512_BIT_IRQSTAT_TRANS_COMP                 DEF_BIT_01
#define  K60N512_BIT_IRQSTAT_CMD_COMP                   DEF_BIT_00

#define  K60N512_BIT_IRQSTAT_ALL_MASK                  (K60N512_BIT_IRQSTAT_DMA_ERR             | K60N512_BIT_IRQSTAT_AC12_ERR          | K60N512_BIT_IRQSTAT_DATA_END_BIT_ERR  | \
                                                        K60N512_BIT_IRQSTAT_DATA_CRC_ERR        | K60N512_BIT_IRQSTAT_DATA_TIMEOUT_ERR  | K60N512_BIT_IRQSTAT_CMD_INDEX_ERR     | \
                                                        K60N512_BIT_IRQSTAT_CMD_END_BIT_ERR     | K60N512_BIT_IRQSTAT_CMD_CRC_ERR       | K60N512_BIT_IRQSTAT_CMD_TIMEOUT_ERR   | \
                                                        K60N512_BIT_IRQSTAT_CARD_INTERRUPT      | K60N512_BIT_IRQSTAT_CARD_REMOVAL      | K60N512_BIT_IRQSTAT_CARD_INSERTION    | \
                                                        K60N512_BIT_IRQSTAT_BUF_READ_READY      | K60N512_BIT_IRQSTAT_BUF_WRITE_READY   | K60N512_BIT_IRQSTAT_DMA_INT           | \
                                                        K60N512_BIT_IRQSTAT_BLK_GAP_EVENT       | K60N512_BIT_IRQSTAT_TRANS_COMP        | K60N512_BIT_IRQSTAT_CMD_COMP          )

#define  K60N512_BIT_IRQSTAT_ERR_MASK                  (K60N512_BIT_IRQSTAT_DMA_ERR             | K60N512_BIT_IRQSTAT_AC12_ERR          | K60N512_BIT_IRQSTAT_DATA_END_BIT_ERR  | \
                                                        K60N512_BIT_IRQSTAT_DATA_CRC_ERR        | K60N512_BIT_IRQSTAT_DATA_TIMEOUT_ERR  | K60N512_BIT_IRQSTAT_CMD_INDEX_ERR     | \
                                                        K60N512_BIT_IRQSTAT_CMD_END_BIT_ERR     | K60N512_BIT_IRQSTAT_CMD_CRC_ERR       )

#define  K60N512_BIT_IRQSTAT_DATA_MASK                 (K60N512_BIT_IRQSTAT_DATA_END_BIT_ERR    | K60N512_BIT_IRQSTAT_DATA_CRC_ERR      | K60N512_BIT_IRQSTAT_DATA_TIMEOUT_ERR  )

#define  K60N512_BIT_IRQSTAT_CMD_MASK                  (K60N512_BIT_IRQSTAT_CMD_INDEX_ERR       | K60N512_BIT_IRQSTAT_CMD_END_BIT_ERR   | K60N512_BIT_IRQSTAT_CMD_CRC_ERR       | \
                                                        K60N512_BIT_IRQSTAT_CMD_TIMEOUT_ERR     )

                                                                /* ------------------ SDHC IRQSTATEN ------------------ */
#define  K60N512_BIT_IRQSTATEN_DMA_ERR_STAT_EN          DEF_BIT_28
#define  K60N512_BIT_IRQSTATEN_AC12_ERR_STAT_EN         DEF_BIT_24
#define  K60N512_BIT_IRQSTATEN_DATA_END_ERR_STAT_EN     DEF_BIT_22
#define  K60N512_BIT_IRQSTATEN_DATA_CRC_ERR_STAT_EN     DEF_BIT_21
#define  K60N512_BIT_IRQSTATEN_DATA_TIMEOUT_STAT_EN     DEF_BIT_20
#define  K60N512_BIT_IRQSTATEN_CMD_INDEX_ERR_STAT_EN    DEF_BIT_19
#define  K60N512_BIT_IRQSTATEN_CMD_END_ERR_STAT_EN      DEF_BIT_18
#define  K60N512_BIT_IRQSTATEN_CMD_CRC_ERR_STAT_EN      DEF_BIT_17
#define  K60N512_BIT_IRQSTATEN_CMD_TIMEOUT_STAT_EN      DEF_BIT_16
#define  K60N512_BIT_IRQSTATEN_CARD_INT_STAT_EN         DEF_BIT_08
#define  K60N512_BIT_IRQSTATEN_CARD_REM_STAT_EN         DEF_BIT_07
#define  K60N512_BIT_IRQSTATEN_CARD_INSERT_STAT_EN      DEF_BIT_06
#define  K60N512_BIT_IRQSTATEN_BUFF_READ_READY_STAT_EN  DEF_BIT_05
#define  K60N512_BIT_IRQSTATEN_BUFF_WRITE_READY_STAT_EN DEF_BIT_04
#define  K60N512_BIT_IRQSTATEN_DMA_INT_STAT_EN          DEF_BIT_03
#define  K60N512_BIT_IRQSTATEN_BLOCK_GAP_STAT_EN        DEF_BIT_02
#define  K60N512_BIT_IRQSTATEN_TRANS_COMP_STAT_EN       DEF_BIT_01
#define  K60N512_BIT_IRQSTATEN_CMD_COMP_STAT_EN         DEF_BIT_00

#define  K60N512_BIT_IRQSTATEN_INIT_MASK               (K60N512_BIT_IRQSTATEN_DMA_ERR_STAT_EN           | K60N512_BIT_IRQSTATEN_DATA_END_ERR_STAT_EN        | K60N512_BIT_IRQSTATEN_DATA_CRC_ERR_STAT_EN    | \
                                                        K60N512_BIT_IRQSTATEN_DATA_TIMEOUT_STAT_EN      | K60N512_BIT_IRQSTATEN_CMD_INDEX_ERR_STAT_EN       | K60N512_BIT_IRQSTATEN_CMD_END_ERR_STAT_EN     | \
                                                        K60N512_BIT_IRQSTATEN_CMD_CRC_ERR_STAT_EN       | K60N512_BIT_IRQSTATEN_CMD_TIMEOUT_STAT_EN         | K60N512_BIT_IRQSTATEN_CARD_REM_STAT_EN        | \
                                                        K60N512_BIT_IRQSTATEN_BUFF_READ_READY_STAT_EN   | K60N512_BIT_IRQSTATEN_BUFF_WRITE_READY_STAT_EN    | K60N512_BIT_IRQSTATEN_TRANS_COMP_STAT_EN      | \
                                                        K60N512_BIT_IRQSTATEN_CMD_COMP_STAT_EN          )

                                                                /* ------------------ SDHC IRQSIGEN ------------------- */
#define  K60N512_BIT_IRQSIGEN_DMA_ERR_INT_EN            DEF_BIT_28
#define  K60N512_BIT_IRQSIGEN_AC12_ERR_INT_EN           DEF_BIT_24
#define  K60N512_BIT_IRQSIGEN_DATA_END_ERR_INT_EN       DEF_BIT_22
#define  K60N512_BIT_IRQSIGEN_CRC_ERR_INT_EN            DEF_BIT_21
#define  K60N512_BIT_IRQSIGEN_DATA_TIME_ERR_INT_EN      DEF_BIT_20
#define  K60N512_BIT_IRQSIGEN_CMD_INDEX_ERR_INT_EN      DEF_BIT_19
#define  K60N512_BIT_IRQSIGEN_CMD_END_BIT_ERR_INT_EN    DEF_BIT_18
#define  K60N512_BIT_IRQSIGEN_CMD_CRC_ERR_INT_EN        DEF_BIT_17
#define  K60N512_BIT_IRQSIGEN_CMD_TIME_ERR_INT_EN       DEF_BIT_16
#define  K60N512_BIT_IRQSIGEN_CARD_INT_EN               DEF_BIT_08
#define  K60N512_BIT_IRQSIGEN_CARD_REM_INT_EN           DEF_BIT_07
#define  K60N512_BIT_IRQSIGEN_CARD_IN_INT_EN            DEF_BIT_06
#define  K60N512_BIT_IRQSIGEN_BUFF_READ_INT_EN          DEF_BIT_05
#define  K60N512_BIT_IRQSIGEN_BUFF_WRITE_INT_EN         DEF_BIT_04
#define  K60N512_BIT_IRQSIGEN_DMA_INT_EN                DEF_BIT_03
#define  K60N512_BIT_IRQSIGEN_BLK_GAP_EVENT_INT_EN      DEF_BIT_02
#define  K60N512_BIT_IRQSIGEN_TRANSFER_COMP_INT_EN      DEF_BIT_01
#define  K60N512_BIT_IRQSIGEN_COMMAND_COMP_INT_EN       DEF_BIT_00

#define  K60N512_BIT_IRQSIGEN_ALL_MASK                 (K60N512_BIT_IRQSIGEN_DMA_ERR_INT_EN         | K60N512_BIT_IRQSIGEN_AC12_ERR_INT_EN      | K60N512_BIT_IRQSIGEN_DATA_END_ERR_INT_EN  | \
                                                        K60N512_BIT_IRQSIGEN_CRC_ERR_INT_EN         | K60N512_BIT_IRQSIGEN_DATA_TIME_ERR_INT_EN | K60N512_BIT_IRQSIGEN_CMD_INDEX_ERR_INT_EN | \
                                                        K60N512_BIT_IRQSIGEN_CMD_END_BIT_ERR_INT_EN | K60N512_BIT_IRQSIGEN_CMD_CRC_ERR_INT_EN   | K60N512_BIT_IRQSIGEN_CMD_TIME_ERR_INT_EN  | \
                                                        K60N512_BIT_IRQSIGEN_CARD_INT_EN            | K60N512_BIT_IRQSIGEN_CARD_REM_INT_EN      | K60N512_BIT_IRQSIGEN_CARD_IN_INT_EN       | \
                                                        K60N512_BIT_IRQSIGEN_BUFF_READ_INT_EN       | K60N512_BIT_IRQSIGEN_BUFF_WRITE_INT_EN    | K60N512_BIT_IRQSIGEN_DMA_INT_EN           | \
                                                        K60N512_BIT_IRQSIGEN_BLK_GAP_EVENT_INT_EN   | K60N512_BIT_IRQSIGEN_TRANSFER_COMP_INT_EN | K60N512_BIT_IRQSIGEN_COMMAND_COMP_INT_EN  )

                                                                /* ------------------- SDHC AC12ERR ------------------- */
#define  K60N512_BIT_AC12ERR_CMD_NOT_ISSUED             DEF_BIT_07
#define  K60N512_BIT_AC12ERR_CMD12_INDEX_ERR            DEF_BIT_04                                                                
#define  K60N512_BIT_AC12ERR_CMD12_CRC_ERR              DEF_BIT_03
#define  K60N512_BIT_AC12ERR_CMD12_END_BIT_ERR          DEF_BIT_02
#define  K60N512_BIT_AC12ERR_CMD12_TIMEOUT_ERR          DEF_BIT_01
#define  K60N512_BIT_AC12ERR_CMD12_NOT_EXECUTED         DEF_BIT_00
#define  K60N512_BIT_AC12ERR_CMD_ERR_MASK              (K60N512_BIT_AC12ERR_CMD_NOT_ISSUED      | K60N512_BIT_AC12ERR_CMD12_INDEX_ERR   | K60N512_BIT_AC12ERR_CMD12_CRC_ERR         || \
                                                        K60N512_BIT_AC12ERR_CMD12_END_BIT_ERR   | K60N512_BIT_AC12ERR_CMD12_TIMEOUT_ERR | K60N512_BIT_AC12ERR_CMD12_NOT_EXECUTED    )

                                                                /* -------------------- SDHC HTCAPBLT ----------------- */
#define  K60N512_BIT_HTCAPBLT_DMA_SUPPORT               DEF_BIT_22

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

                                                                /* ------------------ SDHC MEMORY MAP ----------------- */
 typedef  struct  k60n512_struct_sdhc {
     CPU_REG32  K60_SDHC_DSADDR;                                /* DMA System Address Register                          */
     CPU_REG32  K60_SDHC_BLKATTR;                               /* Block Attributes Register                            */
     CPU_REG32  K60_SDHC_CMDARG;                                /* Command Argument Register                            */
     CPU_REG32  K60_SDHC_XFERTYP;                               /* Transfer Type Register                               */
     CPU_REG32  K60_SDHC_CMDRSP0;                               /* Command Response 0                                   */
     CPU_REG32  K60_SDHC_CMDRSP1;                               /* Command Response 1                                   */
     CPU_REG32  K60_SDHC_CMDRSP2;                               /* Command Response 2                                   */
     CPU_REG32  K60_SDHC_CMDRSP3;                               /* Command Response 3                                   */
     CPU_REG32  K60_SDHC_DATPORT;                               /* Buffer Data Port Register                            */
     CPU_REG32  K60_SDHC_PRSSTAT;                               /* Present State Register                               */
     CPU_REG32  K60_SDHC_PROCTL;                                /* Protocol Control Register                            */
     CPU_REG32  K60_SDHC_SYSCTL;                                /* System Control Register                              */
     CPU_REG32  K60_SDHC_IRQSTAT;                               /* Interrupt Status Register                            */
     CPU_REG32  K60_SDHC_IRQSTATEN;                             /* Interrupt Status Enable Register                     */
     CPU_REG32  K60_SDHC_IRQSIGEN;                              /* Interrupt Signal Enable Register                     */
     CPU_REG32  K60_SDHC_AC12ERR;                               /* Auto CMD12 Error Status Register                     */
     CPU_REG32  K60_SDHC_HTCAPBLT;                              /* Host Controller Capabilities                         */
     CPU_REG32  K60_SDHC_WML;                                   /* Watermark Level Register                             */
     CPU_REG32  K60_RESERVED1[2];                               /* Reserved Registers [x2]                              */
     CPU_REG32  K60_SDHC_FEVT;                                  /* Force Event Register                                 */
     CPU_REG32  K60_SDHC_ADMAES;                                /* ADMA Error Status Register                           */
     CPU_REG32  K60_SDHC_ADSADDR;                               /* ADMA System Address Register                         */
     CPU_REG32  K60_RESERVED2[25];                              /* Reserved Registers [x2]                              */
     CPU_REG32  K60_SDHC_VENDOR;                                /* Vendor Specific Register                             */
 } K60N512_STRUCT_SDHC;


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

FS_OS_SEM   FSDev_SD_Sem;

CPU_INT32U  K60N512_CLOCK_PRESCALER[] = {1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u, 256u};

CPU_INT32U  K60N512_CLOCK_DIVISOR[] = {1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u};

                                                                /* Setting Base address of SDHC Module                  */
K60N512_STRUCT_SDHC  *p_sdhc = (K60N512_STRUCT_SDHC *)K60N512_REG_SDHC_BASE_ADDR;

/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  FSDev_SD_Card_BSP_ISR_Handler (void);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                    FILE SYSTEM SD CARD FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      FSDev_SD_Card_BSP_Open()
*
* Description : Open (initialize) SD/MMC card interface.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
* Return(s)   : DEF_OK,   if interface was opened.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : FSDev_SD_Card_Refresh().
*
* Note(s)     : (1) This function will be called EVERY time the device is opened.
*
*               (2) The Memory Protection Unit provides hardware access control for all memory references
*                   generaged in the device. The MPU concurrently monitors all system bus transactions
*                   and evaluates their appropriateness based on their access rights. Thus to change
*                   acess rights to different regions one must follow the following steps:
*                       - Disable Control/Error Status Register
*                       - Enable Access to Desired Region Descriptor
*                       - Enable Next Descriptor's Valid Bit
*                       - Enable Control/Error Status Register
*
*                   Unfortunately, when Enabling Access to the Desire Region Descriptor, to Enable the 
*                   next Descriptor's Valid Bit won't happen since the software may adjust only the 
*                   access controls within a given region as different tasks execute, the MPU_RGDAACn 
*                   does the previous steps without affecting the next descriptor's valid pin.
*
*
*               (3) Peripheral Bridge Registers can only be acessed in supervisor mode by trusted masters,
*                   which is the reason for Note(2). This defines the privilege level as well as the access
*                   (trusted/not trusted) for Master Read/Write(s). Each Master is assigned depending on its
*                   connection to the Crossbar Switch Master Ports.
*
*               (4) The Port PCR Mux configuration is based on the K60 Signal Multiplexing and Pin
*                   Assignment, for the different alternatives between pins.
*
*********************************************************************************************************
*/

CPU_BOOLEAN  FSDev_SD_Card_BSP_Open (FS_QTY  unit_nbr)
{
    CPU_BOOLEAN  ok;
    
  
    ok = FS_OS_SemCreate(&FSDev_SD_Sem, 0u);    
    if (ok == DEF_FAIL) {
        FS_TRACE_INFO(("FSDev_SD_Card_BSP_Open(): Could not create sem.\r\n"));
        return (DEF_FAIL);
    }
        
    OSC_CR     |= OSC_CR_ERCLKEN_MASK;                              /* Enable OSCERCLK external reference clock             */
    
                                                                    /* Set SDHC Clock Source = OSCERCLK Clock               */
    SIM_SOPT2  |= SIM_SOPT2_SDHCSRC(K60N512_SIM_SOPT2_OSCERCLK_CLK);
    

    MPU_RGDAAC0 |= (DEF_BIT_26 | DEF_BIT_27);                       /* Enable Bus Master Read/Write access. See Note(2)     */
    
    AIPS0_MPRA = K60N512_AIPS_ALL_ON_MASK;                          /* Enable all Acess-Privilages for Module 0, See Note(3)*/
    AIPS1_MPRA = K60N512_AIPS_ALL_ON_MASK;                          /* Enable all Acess-Privilages for Module 1             */
    
    SIM_SCGC5  |= SIM_SCGC5_PORTE_MASK;                             /* Clock Gate Control: Enable the PORT E Clock          */
    SIM_SCGC3  |= SIM_SCGC3_SDHC_MASK;                              /* Clock Gate Control: Enable SDHC Module               */    
                                                              
    
                                                                    /* ----------------- PIN INITIALIZATION --------------- */
    PORTE_PCR0  |= PORT_PCR_PE_MASK                                 /* Internal Pull-Up Resistor Enabled                    */
                |  PORT_PCR_PS_MASK                                 /* Pull Selected to Internal Pull-Up Resistor           */
                |  PORT_PCR_DSE_MASK                                /* High drive strength.                                 */
                | (0|PORT_PCR_MUX(4));                              /* SDHC0_D1, Alternative 4 (SDHC), Refer to Note(4)     */
    
    PORTE_PCR1  |= PORT_PCR_PE_MASK                                 /* Internal Pull-Up Resistor Enabled                    */
                |  PORT_PCR_PS_MASK                                 /* Pull Selected to Internal Pull-Up Resistor           */
                |  PORT_PCR_DSE_MASK                                /* High drive strength.                                 */    
                | (0|PORT_PCR_MUX(4));                              /* SDHC0_D0, Alternative 4 (SDHC).                      */
    
    PORTE_PCR2  |=  PORT_PCR_DSE_MASK                               /* High drive strength.                                 */
                | (0|PORT_PCR_MUX(4));                              /* SDHC0_DCLK, Alternative 4 (SDHC).                    */
    
    PORTE_PCR3  |= PORT_PCR_PE_MASK                                 /* Internal Pull-Up Resistor Enabled                    */
                |  PORT_PCR_PS_MASK                                 /* Pull Selected to Internal Pull-Up Resistor           */
                |  PORT_PCR_DSE_MASK                                /* High drive strength.                                 */
                | (0|PORT_PCR_MUX(4));                              /* SDHC0_CMD, Alternative 4 (SDHC).                     */
    
    PORTE_PCR4  |= PORT_PCR_PE_MASK                                 /* Internal Pull-Up Resistor Enabled                    */
                |  PORT_PCR_PS_MASK                                 /* Pull Selected to Internal Pull-Up Resistor           */
                |  PORT_PCR_DSE_MASK                                /* High drive strength.                                 */
                | (0|PORT_PCR_MUX(4));                              /* SDHC0_D3, Alternative 4 (SDHC).                      */

    PORTE_PCR5  |= PORT_PCR_PE_MASK                                 /* Internal Pull-Up Resistor Enabled                    */
                |  PORT_PCR_PS_MASK                                 /* Pull Selected to Internal Pull-Up Resistor           */
                |  PORT_PCR_DSE_MASK                                /* High drive strength.                                 */
                | (0|PORT_PCR_MUX(4));                              /* SDHC0_D2, Alternative 4 (SDHC).                      */
    
                                                                    /* --------------- CARD DETECTION PINS ---------------- */
    PORTE_PCR27 |= PORT_PCR_PE_MASK                                 /* Internal Pull-Up Resistor Enabled                    */
                |  PORT_PCR_PS_MASK                                 /* Pull Selected to Internal Pull-Up Resistor           */
                | (0|PORT_PCR_MUX(1));                              /* SD_CARD_DETECT, Alternative 1 (GPIO).                */
    
    PORTE_PCR28 |= PORT_PCR_PE_MASK                                 /* Internal Pull-Up Resistor Enabled                    */
                |  PORT_PCR_PS_MASK                                 /* Pull Selected to Internal Pull-Up Resistor           */
                | (0|PORT_PCR_MUX(1));                              /* SD_CARD_WP, Alternative 1 (GPIO).                    */
 
    
    DEF_BIT_CLR(p_sdhc->K60_SDHC_VENDOR, DEF_BIT_00);               /* Disable External DMA Request                         */
    
                                                                    /* Endian Mode set to Little Endian                     */
    DEF_BIT_SET(p_sdhc->K60_SDHC_PROCTL, K60N512_BIT_PROCTL_ENDIAN_MODE_LITTLE);

                                                                    /* Enable Interrupt Flags                               */
    DEF_BIT_SET(p_sdhc->K60_SDHC_IRQSTATEN, K60N512_BIT_IRQSTATEN_INIT_MASK);
    
                                                                    /* Clear All Interrupt Flags                            */
    DEF_BIT_CLR(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_ALL_MASK);

    
                                                                    /* Set Watermark Level for Read and Write to 64 Words   */
    p_sdhc->K60_SDHC_WML = (SDHC_WML_WRWML(64u) | SDHC_WML_RDWML(64u));
    
                                                                    /* Set a Transfer Block size to 200 Bytes               */
    p_sdhc->K60_SDHC_BLKATTR = (SDHC_BLKATTR_BLKCNT(1) | SDHC_BLKATTR_BLKSIZE(200));
    
                                                                    /* Initialize Default Clock Frequency                   */
    FSDev_SD_Card_BSP_SetClkFreq(unit_nbr, K60N512_SDHC_INIT_CLK_FREQ);
    
    BSP_IntVectSet (BSP_INT_ID_SDHC,                                /* Assign ISR Handler to SHDC Interrupt                 */
                    FSDev_SD_Card_BSP_ISR_Handler);

    BSP_IntEn(BSP_INT_ID_SDHC);                                     /* Enable Interrupt                                     */

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                      FSDev_SD_Card_BSP_Close()
*
* Description : Close (unitialize) SD/MMC card interface.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_Close().
*
* Note(s)     : (1) This function will be called EVERY time the device is closed.
*
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_Close (FS_QTY  unit_nbr)
{

    (void)&unit_nbr;                                                /* Prevent compiler warning.                            */

                                                                    /* Set SDHC PortE pins to their Reset State             */
    PORTE_PCR0  = 0u;
    PORTE_PCR1  = 0u;
    PORTE_PCR2  = 0u;
    PORTE_PCR3  = 0u;
    PORTE_PCR4  = 0u;
    PORTE_PCR5  = 0u;
    PORTE_PCR27 = 0u;
    PORTE_PCR28 = 0u;
    
    SIM_SCGC3 &= ~SIM_SCGC3_SDHC_MASK;                              /* Clock Gate Control: Disable SDHC Module              */

                                                                    /* Set SDHC Clock Source to Default State               */
    SIM_SOPT2 &= ~SIM_SOPT2_SDHCSRC(K60N512_SIM_SOPT2_OSCERCLK_CLK);
}


/*
*********************************************************************************************************
*                                      FSDev_SD_Card_BSP_Lock()
*
* Description : Acquire SD/MMC card bus lock.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_Rd()
*               FSDev_SD_Card_Wr()
*               FSDev_SD_Card_IO_Ctrl()
*               FSDev_SD_Card_Refresh()
*
* Note(s)     : (1) This function will be called before the SD/MMC driver begins to access the SD/MMC
*                   card bus.  The application should NOT use the same bus to access another device until
*                   the matching call to 'FSDev_SD_Card_BSP_Unlock()' has been made.
*
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_Lock (FS_QTY  unit_nbr)
{
   (void)&unit_nbr;                                                 /* Prevent compiler warning.                            */
}


/*
*********************************************************************************************************
*                                     FSDev_SD_Card_BSP_Unlock()
*
* Description : Release SD/MMC card bus lock.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_Rd()
*               FSDev_SD_Card_Wr()
*               FSDev_SD_Card_IO_Ctrl()
*               FSDev_SD_Card_Refresh()
*
* Note(s)     : (1) 'FSDev_SD_Card_BSP_Lock()' will be called before the SD/MMC driver begins to access
*                   the SD/MMC card bus.  The application should NOT use the same bus to access another
*                   device until the matching call to this function has been made.
*
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_Unlock (FS_QTY  unit_nbr)
{
   (void)&unit_nbr;                                                 /* Prevent compiler warning.                            */
}


/*
*********************************************************************************************************
*                                      FSDev_SD_Card_BSP_CmdStart()
*
* Description : Start a command.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
*               p_cmd       Pointer to command to transmit (see Note #2).
*
*               p_data      Pointer to buffer address for DMA transfer (see Note #3).
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               FS_DEV_SD_CARD_ERR_NONE       No error.
*                               FS_DEV_SD_CARD_ERR_NO_CARD    No card present.
*                               FS_DEV_SD_CARD_ERR_BUSY       Controller is busy.
*                               FS_DEV_SD_CARD_ERR_UNKNOWN    Unknown or other error.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_Cmd()
*               FSDev_SD_Card_CmdRShort()
*               FSDev_SD_Card_CmdRLong()
*               FSDev_SD_Card_RdData()
*               FSDev_SD_Card_WrData()
*
* Note(s)     : (1) The command start will be followed by zero, one or two additional BSP function calls,
*                   depending on whether data should be transferred and on whether any errors occur.
*
*                   (a) 'FSDev_SD_Card_BSP_CmdStart()' starts execution of the command.  It may also set
*                       up the DMA transfer (if necessary).
*
*                   (b) 'FSDev_SD_Card_BSP_CmdWaitEnd()' waits for the execution of the command to end,
*                       getting the command response (if any).
*
*                   (c) If data should be transferred from the card to the host, 'FSDev_SD_Card_BSP_CmdDataRd()'
*                       will read that data; if data should be transferred from the host to the card,
*                       'FSDev_SD_Card_BSP_CmdDataWr()' will write that data.
*
*                   (d) If an error is returned at any point, the sequence will be aborted.
*
*               (2) The command 'p_cmd' has the following parameters :
*
*                   (a) 'p_cmd->Cmd' is the command index.
*
*                   (b) 'p_cmd->Arg' is the 32-bit argument (or 0 if there is no argument).
*
*                   (c) 'p_cmd->Flags' is a bit-mapped variable with zero or more command flags :
*
*                           FS_DEV_SD_CARD_CMD_FLAG_INIT          Initialization sequence before command.
*                           FS_DEV_SD_CARD_CMD_FLAG_BUSY          Busy signal expected after command.
*                           FS_DEV_SD_CARD_CMD_FLAG_CRC_VALID     CRC valid after command.
*                           FS_DEV_SD_CARD_CMD_FLAG_IX_VALID      Index valid after command.
*                           FS_DEV_SD_CARD_CMD_FLAG_OPEN_DRAIN    Command line is open drain.
*                           FS_DEV_SD_CARD_CMD_FLAG_DATA_START    Data start command.
*                           FS_DEV_SD_CARD_CMD_FLAG_DATA_STOP     Data stop command.
*                           FS_DEV_SD_CARD_CMD_FLAG_RESP          Response expected.
*                           FS_DEV_SD_CARD_CMD_FLAG_RESP_LONG     Long response expected.
*
*                   (d) 'p_cmd->DataDir' indicates the direction of any data transfer that should follow
*                       this command, if any :
*
*                           FS_DEV_SD_CARD_DATA_DIR_NONE            No data transfer.
*                           FS_DEV_SD_CARD_DATA_DIR_HOST_TO_CARD    Transfer host-to-card (write).
*                           FS_DEV_SD_CARD_DATA_DIR_CARD_TO_HOST    Transfer card-to-host (read).
*
*                   (e) 'p_cmd->DataType' indicates the type of the data transfer that should follow this
*                       command, if any :
*
*                           FS_DEV_SD_CARD_DATA_TYPE_NONE            No data transfer.
*                           FS_DEV_SD_CARD_DATA_TYPE_SINGLE_BLOCK    Single data block.
*                           FS_DEV_SD_CARD_DATA_TYPE_MULTI_BLOCK     Multiple data blocks.
*                           FS_DEV_SD_CARD_DATA_TYPE_STREAM          Stream data.
*
*                   (f) 'p_cmd->RespType' indicates the type of the response that should be expected from
*                       this command :
*
*                           FS_DEV_SD_CARD_RESP_TYPE_NONE    No  response.
*                           FS_DEV_SD_CARD_RESP_TYPE_R1      R1  response: Normal Response Command.
*                           FS_DEV_SD_CARD_RESP_TYPE_R1B     R1b response.
*                           FS_DEV_SD_CARD_RESP_TYPE_R2      R2  response: CID, CSD Register.
*                           FS_DEV_SD_CARD_RESP_TYPE_R3      R3  response: OCR Register.
*                           FS_DEV_SD_CARD_RESP_TYPE_R4      R4  response: Fast I/O Response (MMC).
*                           FS_DEV_SD_CARD_RESP_TYPE_R5      R5  response: Interrupt Request Response (MMC).
*                           FS_DEV_SD_CARD_RESP_TYPE_R5B     R5B response.
*                           FS_DEV_SD_CARD_RESP_TYPE_R6      R6  response: Published RCA Response.
*                           FS_DEV_SD_CARD_RESP_TYPE_R7      R7  response: Card Interface Condition.
*
*                   (g) 'p_cmd->BlkSize' and 'p_cmd->BlkCnt' are the block size and block count of the
*                       data transfer that should follow this command, if any.
*
*               (3) The pointer to the data buffer that will receive the data transfer that should follow
*                   this command is given so that a DMA transfer can be set up.
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_CmdStart (FS_QTY               unit_nbr,
                                  FS_DEV_SD_CARD_CMD  *p_cmd,
                                  void                *p_data,
                                  FS_DEV_SD_CARD_ERR  *p_err)
{
    CPU_INT32U  xfer_dat_cont;


    xfer_dat_cont   = 0u;
    

    while ((DEF_BIT_IS_SET(p_sdhc->K60_SDHC_PRSSTAT, K60N512_BIT_PRSSTAT_DAT_INHIBIT) == DEF_YES) && 
           (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_PRSSTAT, K60N512_BIT_PRSSTAT_CMD_INHIBIT) == DEF_YES)) {
        ;                                                           /* Wait till Appropriate Information can be transferred */
    }                                                               /* through the Designated Communication Line            */

                                                                    /* Check Response Type                                  */
    if (DEF_BIT_IS_SET(p_cmd->Flags, FS_DEV_SD_CARD_CMD_FLAG_RESP) == DEF_YES) {
        if (DEF_BIT_IS_SET(p_cmd->Flags, FS_DEV_SD_CARD_CMD_FLAG_RESP_LONG) == DEF_YES) {
            xfer_dat_cont |= K60N512_BIT_XFERTYP_RESPONSE_TYPE_136;
        } else {
            if (DEF_BIT_IS_SET(p_cmd->Flags, FS_DEV_SD_CARD_CMD_FLAG_BUSY) == DEF_YES) {
                xfer_dat_cont |= K60N512_BIT_XFERTYP_RESPONSE_TYPE_48_CHK_BUSY;
            } else {
                xfer_dat_cont |= K60N512_BIT_XFERTYP_RESPONSE_TYPE_48;
            }
        }
    }

    if (p_cmd->DataDir == FS_DEV_SD_CARD_DATA_DIR_HOST_TO_CARD) {
        xfer_dat_cont &= ~K60N512_BIT_XFERTYP_DATA_TRANS_DIR;       /* Clear the Direction to Write                         */
        xfer_dat_cont |= K60N512_BIT_XFERTYP_DMA_EN                 /* Enable DMA Functionality                             */
                      |  K60N512_BIT_XFERTYP_DATA_PRES_SEL          /* Indicate Data is Present & Transfer using DAT Line   */
                      |  K60N512_BIT_XFERTYP_BLK_CNT_EN;            /* Enable Block Count                                   */

        p_sdhc->K60_SDHC_DSADDR = (CPU_INT32U)p_data;               /* Set p_data in DMA System Address Register            */
    }

    if (p_cmd->DataDir == FS_DEV_SD_CARD_DATA_DIR_CARD_TO_HOST) {
        xfer_dat_cont |= K60N512_BIT_XFERTYP_DATA_TRANS_DIR         /* Set the Direction to Read                            */
                      |  K60N512_BIT_XFERTYP_DMA_EN                 /* Enable DMA Functionality                             */
                      |  K60N512_BIT_XFERTYP_DATA_PRES_SEL          /* Indicate Data is Present & Transfer using DAT Line   */
                      |  K60N512_BIT_XFERTYP_BLK_CNT_EN;            /* Enable Block Count                                   */

        p_sdhc->K60_SDHC_DSADDR = (CPU_INT32U)p_data;               /* Set p_data in DMA System Address Register            */
    }
    
    if (DEF_BIT_IS_SET(p_cmd->DataType, FS_DEV_SD_CARD_DATA_TYPE_MULTI_BLOCK) == DEF_YES) {
        xfer_dat_cont |= (K60N512_BIT_XFERTYP_BLK_CNT_EN);          /* Enable Block Count for Multiple Block Transfers      */
    }
    
    if (DEF_BIT_IS_SET(p_cmd->Flags, FS_DEV_SD_CARD_CMD_FLAG_INIT) == DEF_YES) {
                                                                    /* Send 80 SD-Clocks                                    */
        p_sdhc->K60_SDHC_SYSCTL |= K60N512_BIT_SYSCTL_INITIALIZATION_ACTIVE;
        
        while(DEF_BIT_IS_SET(p_sdhc->K60_SDHC_SYSCTL, K60N512_BIT_SYSCTL_INITIALIZATION_ACTIVE) == DEF_YES) {
            ;                                                       /* Wait for Initialization to Complete                  */
        }
    }
    
    if (DEF_BIT_IS_SET(p_cmd->Flags, FS_DEV_SD_CARD_CMD_FLAG_START_DATA_TX) == DEF_YES) {
                                                                    /* Setting Block Count in Block Attribute Register      */
        p_sdhc->K60_SDHC_BLKATTR  = SDHC_BLKATTR_BLKCNT(p_cmd->BlkCnt); 
                                                                    /* Setting Block Size in Block Attribute Register       */
        p_sdhc->K60_SDHC_BLKATTR |= SDHC_BLKATTR_BLKSIZE(p_cmd->BlkSize);
    }
    
                                                                    /* ------------------ SETUP COMMAND ------------------- */
    p_sdhc->K60_SDHC_CMDARG      = p_cmd->Arg;                      /* Write Arg in Command Argument Register               */
    xfer_dat_cont               |= SDHC_XFERTYP_CMDINX(p_cmd->Cmd); /* Write Cmd in Command Index                           */
    p_sdhc->K60_SDHC_XFERTYP     = xfer_dat_cont;                   /* Write operation(s) to SHDC_XFERTYP Register          */
    
   *p_err = FS_DEV_SD_CARD_ERR_NONE;
}


/*
*********************************************************************************************************
*                                   FSDev_SD_Card_BSP_CmdWaitEnd()
*
* Description : Wait for command to end & get command response.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
*               p_cmd       Pointer to command that is ending.
*
*               p_resp      Pointer to buffer that will receive command response, if any.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               FS_DEV_SD_CARD_ERR_NONE            No error.
*                               FS_DEV_SD_CARD_ERR_NO_CARD         No card present.
*                               FS_DEV_SD_CARD_ERR_UNKNOWN         Unknown or other error.
*                               FS_DEV_SD_CARD_ERR_WAIT_TIMEOUT    Timeout in waiting for command response.
*                               FS_DEV_SD_CARD_ERR_RESP_TIMEOUT    Timeout in receiving command response.
*                               FS_DEV_SD_CARD_ERR_RESP_CHKSUM     Error in response checksum.
*                               FS_DEV_SD_CARD_ERR_RESP_CMD_IX     Response command index error.
*                               FS_DEV_SD_CARD_ERR_RESP_END_BIT    Response end bit error.
*                               FS_DEV_SD_CARD_ERR_RESP            Other response error.
*                               FS_DEV_SD_CARD_ERR_DATA            Other data err.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_Cmd()
*               FSDev_SD_Card_CmdRShort()
*               FSDev_SD_Card_CmdRLong()
*               FSDev_SD_Card_RdData()
*               FSDev_SD_Card_WrData()
*
* Note(s)     : (1) This function will be called even if no response is expected from the command.
*
*               (2) This function will NOT be called if 'FSDev_SD_Card_BSP_CmdStart()' returned an error.
*
*               (3) (a) For a command with a normal response, a  4-byte response should be stored in
*                       'p_resp'.
*
*                   (b) For a command with a long   response, a 16-byte response should be stored in
*                       'p_resp'.  The first  4-byte word should hold bits 127..96 of the response.
*                                  The second 4-byte word should hold bits  95..64 of the response.
*                                  The third  4-byte word should hold bits  63..32 of the response.
*                                  The fourth 4-byte word should hold bits  31.. 0 of the reponse.
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_CmdWaitEnd (FS_QTY               unit_nbr,
                                    FS_DEV_SD_CARD_CMD  *p_cmd,
                                    CPU_INT32U          *p_resp,
                                    FS_DEV_SD_CARD_ERR  *p_err)
{
                                                                    /* Enable Interrupt Signals                             */
    p_sdhc->K60_SDHC_IRQSIGEN = K60N512_BIT_IRQSIGEN_COMMAND_COMP_INT_EN 
                              | K60N512_BIT_IRQSIGEN_CMD_TIME_ERR_INT_EN;
    
    FS_OS_SemPend(&FSDev_SD_Sem, 0u);                               /* Pend till Command Complete or CMD Timeout Error      */
    
    if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_CMD_TIMEOUT_ERR) == DEF_YES) {
        if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_CMD_CRC_ERR) == DEF_YES) {
            *p_err = FS_DEV_SD_CARD_ERR_RESP_CHKSUM;                /* Command CRC Error                                    */
        } else {
            *p_err = FS_DEV_SD_CARD_ERR_RESP_TIMEOUT;               /* Timeout Error                                        */
        }
    }
    
    
    if (DEF_BIT_IS_SET_ANY(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_ERR_MASK) == DEF_YES) {
        *p_err = FS_DEV_SD_CARD_ERR_UNKNOWN;                        /* SDHC IRQSTAT Error Mask, Generic FS_DEV_IO Error     */
    }
    
    
    if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_CMD_COMP) == DEF_YES) {
        if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_CMD_TIMEOUT_ERR) == DEF_YES) {
            *p_err = FS_DEV_SD_CARD_ERR_RESP_TIMEOUT;               /* Last Check to Ensure no Timeout Error                */
        } else {
            ;                                                       /* Response was Recieved                                */
        }
    }     
                                                                    /* Long Response                                        */
    if (DEF_BIT_IS_SET(p_cmd->Flags, FS_DEV_SD_CARD_CMD_FLAG_RESP_LONG) == DEF_YES) {
       *p_resp     = (p_sdhc->K60_SDHC_CMDRSP3 << 8u); 
       *p_resp    |= (p_sdhc->K60_SDHC_CMDRSP2 >> 24u);
        p_resp++;
        
       *p_resp     = (p_sdhc->K60_SDHC_CMDRSP2 << 8u);
       *p_resp    |= (p_sdhc->K60_SDHC_CMDRSP1 >> 24u);
        p_resp++;
        
       *p_resp     = (p_sdhc->K60_SDHC_CMDRSP1 << 8u);
       *p_resp    |= (p_sdhc->K60_SDHC_CMDRSP0 >> 24u);
        p_resp++;
        
       *p_resp     = (p_sdhc->K60_SDHC_CMDRSP0 << 8u);
        p_resp++;
                                                                    /* Normal Response                                      */
    } else if (DEF_BIT_IS_SET(p_cmd->Flags, FS_DEV_SD_CARD_CMD_FLAG_RESP) == DEF_YES) {
        *p_resp     = p_sdhc->K60_SDHC_CMDRSP0;
    } else {
        ;
    }
                                                                    /* Reset DAT Line on Data Error                         */
    if (DEF_BIT_IS_SET_ANY(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_MASK) == DEF_YES) {
        p_sdhc->K60_SDHC_SYSCTL |= K60N512_BIT_SYSCTL_RESET_DAT_LINE;
        
        while (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_SYSCTL, K60N512_BIT_SYSCTL_RESET_DAT_LINE) == DEF_YES) {
          ;                                                         /* Wait till reset is complete                          */
        }
                                                                    /* Clear Corresponding errors after Reset is Complete   */
        DEF_BIT_CLR(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_MASK);
    }
                                                                    /* Reset CMD Line on Command Error                      */
    if (DEF_BIT_IS_SET_ANY(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_CMD_MASK) == DEF_YES) {
        p_sdhc->K60_SDHC_SYSCTL |= K60N512_BIT_SYSCTL_RESET_CMD_LINE;
        
        while (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_SYSCTL, K60N512_BIT_SYSCTL_RESET_CMD_LINE) == DEF_YES) {
          ;                                                         /* Wait till reset is complete                          */
        }
                                                                    /* Clear Corresponding errors after Reset is Complete   */
        DEF_BIT_CLR(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_CMD_MASK);
    }
    
    p_sdhc->K60_SDHC_IRQSTAT = K60N512_BIT_IRQSTAT_CMD_COMP;        /* Clear Command Complete Flag                          */
    
   *p_err = FS_DEV_SD_CARD_ERR_NONE;
    return;
    
}


/*
*********************************************************************************************************
*                                    FSDev_SD_Card_BSP_CmdDataRd()
*
* Description : Read data following command.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
*               p_cmd       Pointer to command that was started.
*
*               p_dest      Pointer to destination buffer.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               FS_DEV_SD_CARD_ERR_NONE              No error.
*                               FS_DEV_SD_CARD_ERR_NO_CARD           No card present.
*                               FS_DEV_SD_CARD_ERR_UNKNOWN           Unknown or other error.
*                               FS_DEV_SD_CARD_ERR_WAIT_TIMEOUT      Timeout in waiting for data.
*                               FS_DEV_SD_CARD_ERR_DATA_OVERRUN      Data overrun.
*                               FS_DEV_SD_CARD_ERR_DATA_TIMEOUT      Timeout in receiving data.
*                               FS_DEV_SD_CARD_ERR_DATA_CHKSUM       Error in data checksum.
*                               FS_DEV_SD_CARD_ERR_DATA_START_BIT    Data start bit error.
*                               FS_DEV_SD_CARD_ERR_DATA              Other data error.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_RdData().
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_CmdDataRd (FS_QTY               unit_nbr,
                                   FS_DEV_SD_CARD_CMD  *p_cmd,
                                   void                *p_dest,
                                   FS_DEV_SD_CARD_ERR  *p_err)
{   
                                                                    /* Enable Interrupt Signals                             */
    p_sdhc->K60_SDHC_IRQSIGEN = K60N512_BIT_IRQSIGEN_TRANSFER_COMP_INT_EN
                              | K60N512_BIT_IRQSIGEN_DMA_ERR_INT_EN
                              | K60N512_BIT_IRQSIGEN_DATA_TIME_ERR_INT_EN;
    
    FS_OS_SemPend(&FSDev_SD_Sem, 0u);                               /* Pend till Interrupt Signal(s) are set                */

    
    if (DEF_BIT_IS_SET_ANY(p_sdhc->K60_SDHC_AC12ERR, K60N512_BIT_AC12ERR_CMD_ERR_MASK) == DEF_YES) {
        *p_err = FS_DEV_SD_CARD_ERR_DATA;                           /* Generic Auto CMD12 Error                             */             
    } else if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_END_BIT_ERR) == DEF_YES) {
        *p_err = FS_DEV_SD_CARD_ERR_DATA;                           /* DATA End Bit Error                                   */
    } else if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_CRC_ERR) == DEF_YES) {
        *p_err = FS_DEV_SD_CARD_ERR_DATA_CHKSUM;                    /* DATA CRC Error                                       */
    } else if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DMA_ERR) == DEF_YES) {
        *p_err = FS_DEV_SD_CARD_ERR_DATA;                           /* DMA Error                                            */
    } else if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_TRANS_COMP) == DEF_YES) {
        if(DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_TIMEOUT_ERR) == DEF_YES) {
            *p_err = FS_DEV_SD_CARD_ERR_DATA_TIMEOUT;               /* After Transfer Complete, Check if Data Timeout       */
        } else {
            *p_err = FS_DEV_SD_CARD_ERR_NONE;                       /* Transfer Complete, no errors                         */
        }
    }
                                                                    /* Reset DAT Line on Data Error                         */
    if (DEF_BIT_IS_SET_ANY(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_MASK) == DEF_YES) {
        p_sdhc->K60_SDHC_SYSCTL |= K60N512_BIT_SYSCTL_RESET_DAT_LINE;
        
        while (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_SYSCTL, K60N512_BIT_SYSCTL_RESET_DAT_LINE) == DEF_YES) {
          ;                                                         /* Wait till reset is complete                          */
        }
                                                                    /* Clear Corresponding errors after Reset is Complete   */
        DEF_BIT_CLR(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_MASK);
    }
    
    p_sdhc->K60_SDHC_IRQSTAT = DEF_INT_32_MASK;                     /* Clear all Interrupt Flags                            */
}


/*
*********************************************************************************************************
*                                    FSDev_SD_Card_BSP_CmdDataWr()
*
* Description : Write data following command.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
*               p_cmd       Pointer to command that was started.
*
*               p_src       Pointer to source buffer.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                               FS_DEV_SD_CARD_ERR_NONE              No error.
*                               FS_DEV_SD_CARD_ERR_NO_CARD           No card present.
*                               FS_DEV_SD_CARD_ERR_UNKNOWN           Unknown or other error.
*                               FS_DEV_SD_CARD_ERR_WAIT_TIMEOUT      Timeout in waiting for data.
*                               FS_DEV_SD_CARD_ERR_DATA_UNDERRUN     Data underrun.
*                               FS_DEV_SD_CARD_ERR_DATA_CHKSUM       Err in data checksum.
*                               FS_DEV_SD_CARD_ERR_DATA_START_BIT    Data start bit error.
*                               FS_DEV_SD_CARD_ERR_DATA              Other data error.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_WrData().
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_CmdDataWr (FS_QTY               unit_nbr,
                                   FS_DEV_SD_CARD_CMD  *p_cmd,
                                   void                *p_src,
                                   FS_DEV_SD_CARD_ERR  *p_err)
{    
                                                                    /* Enable Interrupt Signals                             */
    p_sdhc->K60_SDHC_IRQSIGEN = K60N512_BIT_IRQSIGEN_TRANSFER_COMP_INT_EN
                              | K60N512_BIT_IRQSIGEN_DMA_ERR_INT_EN
                              | K60N512_BIT_IRQSIGEN_DATA_TIME_ERR_INT_EN;
    
    FS_OS_SemPend(&FSDev_SD_Sem, 0u);                               /* Pend till Interrupt Signal(s) are set                */
 

    if (DEF_BIT_IS_SET_ANY(p_sdhc->K60_SDHC_AC12ERR, K60N512_BIT_AC12ERR_CMD_ERR_MASK) == DEF_YES) {
        *p_err = FS_DEV_SD_CARD_ERR_DATA;                           /* Generic Auto CMD12 Error                             */
    } else if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_END_BIT_ERR) == DEF_YES) {
        *p_err = FS_DEV_SD_CARD_ERR_DATA;                           /* DATA End Bit Error                                   */
    } else if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_CRC_ERR) == DEF_YES) {
        *p_err = FS_DEV_SD_CARD_ERR_DATA_START_BIT;                 /* DATA CRC Error                                       */
    } else if (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_TRANS_COMP) == DEF_YES) {
        if(DEF_BIT_IS_SET(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_TIMEOUT_ERR) == DEF_YES) {
            *p_err = FS_DEV_SD_CARD_ERR_DATA_TIMEOUT;               /* After Transfer Complete, Check if Data Timeout       */
        } else {
            *p_err = FS_DEV_SD_CARD_ERR_NONE;                       /* Transfer Complete, no errors                         */
        }
                                                                    /* Clear Corresponding errors after Reset is Complete   */
        DEF_BIT_CLR(p_sdhc->K60_SDHC_IRQSTAT, K60N512_BIT_IRQSTAT_DATA_MASK);
    }
    
    p_sdhc->K60_SDHC_IRQSTAT = DEF_INT_32_MASK;                     /* Clear all Interrupt Flags                            */
}


/*
*********************************************************************************************************
*                                  FSDev_SD_Card_BSP_GetBlkCntMax()
*
* Description : Get maximum number of blocks that can be transferred with a multiple read or multiple
*               write command.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
*               blk_size    Block size, in octets.
*
* Return(s)   : Maximum number of blocks.
*
* Caller(s)   : FSDev_SD_Card_Refresh().
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

CPU_INT32U  FSDev_SD_Card_BSP_GetBlkCntMax (FS_QTY      unit_nbr,
                                            CPU_INT32U  blk_size)
{
    return (DEF_INT_32U_MAX_VAL); //HB: Evaluate to limit to 1
}


/*
*********************************************************************************************************
*                                 FSDev_SD_Card_BSP_GetBusWidthMax()
*
* Description : Get maximum bus width, in bits.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
* Return(s)   : Maximum bus width.
*
* Caller(s)   : FSDev_SD_Card_Refresh().
*
* Note(s)     : (1) The SDHC interface is capable of 1-, & 4-bit operation.
*
*               (2) This function SHOULD always return the same value.  If hardware constraints change
*                   at run-time, the device MUST be closed & re-opened for any changes to be effective.
*
*********************************************************************************************************
*/

CPU_INT08U  FSDev_SD_Card_BSP_GetBusWidthMax (FS_QTY  unit_nbr)
{
    return (4u);
}


/*
*********************************************************************************************************
*                                   FSDev_SD_Card_BSP_SetBusWidth()
*
* Description : Set bus width.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
*               width       Bus width, in bits.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_Refresh(),
*               FSDev_SD_Card_SetBusWidth().
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_SetBusWidth (FS_QTY      unit_nbr,
                                     CPU_INT08U  width)
{
    (void)&unit_nbr;                                                /* Prevent compiler warning.                            */
  
    switch (width) {
        case 1u:                                                    /* Set Data Transfer Width to 1-Bit Mode                */
                DEF_BIT_CLR(p_sdhc->K60_SDHC_PROCTL, K60N512_BIT_PROCTL_DATA_TRANS_WIDTH_1_BIT);
                break;        
                
                
        case 4u:                                                    /* Set Data Transfer Width to 4-Bit Mode                */
                DEF_BIT_SET(p_sdhc->K60_SDHC_PROCTL, K60N512_BIT_PROCTL_DATA_TRANS_WIDTH_4_BIT);
                break;
          
                
        default:                                                    /* Set Default Value to Transfer 1-Bit Mode             */
                DEF_BIT_CLR(p_sdhc->K60_SDHC_PROCTL, K60N512_BIT_PROCTL_DATA_TRANS_WIDTH_1_BIT);
                break;
    }
}


/*
*********************************************************************************************************
*                                   FSDev_SD_Card_BSP_SetClkFreq()
*
* Description : Set clock frequency.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
*               freq        Clock frequency, in Hz.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_Refresh().
*
* Note(s)     : (1) The effective clock frequency MUST be no more than 'freq'.  If the frequency cannot be
*                   configured equal to 'freq', it should be configured less than 'freq'.
*
*               (2) SDCLK Clock Formula is used here to find the best Clock Divisor and Clock Prescaler
*                   for the required frequency. The SDCLK Clock Formula is the only equation given and
*                   thus the nested 'for' loops will sort through all possibilities until the best/closest
*                   frequency to 'freq' appears, then it will break and continue to setup the SDHC Clock.
*                       The equation is the following:
*                                                          Base Clock
*                               Clock Frequency =    ----------------------
*                                                    (Prescaler * Divisor)
*
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_SetClkFreq (FS_QTY      unit_nbr,
                                    CPU_INT32U  freq)
{
    CPU_INT32U   clk_freq;
    CPU_INT32U   base_freq;
    CPU_INT08U   clk_div;
    CPU_INT08U   clk_prescaler;
    CPU_INT08U   cnt_prescaler;
    CPU_INT08U   cnt_div;
    CPU_BOOLEAN  found_val;
    
    
    (void)&unit_nbr;                                                /* Prevent compiler warning.                            */

    
    found_val = DEF_NO;                                             /* Set Inital found_val                                 */
    base_freq = BSP_CPU_ClkFreq();                                  /* Using Core/System Clock as Base Clock Freq           */
    
                                                                    /* Disable SD Clock                                     */
    DEF_BIT_CLR(p_sdhc->K60_SDHC_SYSCTL, K60N512_BIT_SYSCTL_SD_CLK_EN);
    
                                                                    /* Using SDCLK Clock Formula, Refer to Note(2)          */
        for (cnt_prescaler = 0; cnt_prescaler < K60N512_PRESCALER_AMOUNT; cnt_prescaler++) {
            for (cnt_div = 0; cnt_div < K60N512_DIVIDER_AMOUNT; cnt_div++) {
            
                clk_freq = (base_freq / (K60N512_CLOCK_PRESCALER[cnt_prescaler] * K60N512_CLOCK_DIVISOR[cnt_div]));
                
                if (clk_freq <= freq) {                             /* If Clk_freq <= Freq, set found_val to break from     */
                    found_val = DEF_YES;                            /* outer loop, and break from inner loop                */
                    break;                                      
                }
            }
            if (found_val == DEF_YES) {
                break;
            }
        } 
    
    clk_prescaler = K60N512_CLOCK_PRESCALER[cnt_prescaler];         /* Obtains best Prescaler Value for 'freq'              */
    clk_div       = K60N512_CLOCK_DIVISOR[cnt_div];                 /* Obtains best Divisor Value for 'freq'                */
    
    p_sdhc->K60_SDHC_SYSCTL  = SDHC_SYSCTL_SDCLKFS(clk_prescaler);  /* Sets Prescale Value                                  */
    p_sdhc->K60_SDHC_SYSCTL |= SDHC_SYSCTL_DVS(clk_div);            /* Sets Divisor Value                                   */
    
    
    while (DEF_BIT_IS_CLR(p_sdhc->K60_SDHC_PRSSTAT, K60N512_BIT_PRSSTAT_SD_CLK_STABLE) == DEF_YES) {
        ;                                                           /* Wait till SD Clock is Stable before Enabling Clock   */
    }
    
                                                                    /* System Clock is Active                               */
    DEF_BIT_CLR(p_sdhc->K60_SDHC_PRSSTAT, K60N512_BIT_PRSSTAT_SYS_CLK_GATED_OFF);
        
    p_sdhc->K60_SDHC_SYSCTL |= K60N512_BIT_SYSCTL_SD_CLK_EN         /* Enable SD Clock                                      */
                            |  K60N512_BIT_SYSCTL_PERIPH_CLK_EN     /* Enable Peripheral Clock                              */
                            |  K60N512_BIT_SYSCTL_SYS_CLK_EN        /* Enable System Clock                                  */
                            |  K60N512_BIT_SYSCTL_IPG_CLK_EN;       /* Enable IPG Clock                                     */
    
    while (DEF_BIT_IS_SET(p_sdhc->K60_SDHC_PRSSTAT, (K60N512_BIT_PRSSTAT_DAT_INHIBIT | K60N512_BIT_PRSSTAT_CMD_INHIBIT)) == DEF_YES) {
        ;                                                           /* Wait until able to issue next DAT & CMD Command      */
    }
    
    FSDev_SD_Card_BSP_SetTimeoutData(unit_nbr, 0xFFFFFFFFuL);       /* Reset Data Timeout Value to Max Value                */
}


/*
*********************************************************************************************************
*                                 FSDev_SD_Card_BSP_SetTimeoutData()
*
* Description : Set data timeout.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
*               to_clks     Timeout value from 0 - 14 bits.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_Refresh().
*
* Note(s)     : (1) uC/FS always asks for the maximum timeout value, therefore it has been hard-coded to 
*                   the maximum value for timeout counter.
*
*               (2) This value determines the interval by which DAT line timeouts are detected. Timeout
*                   clock frequency will be generated by dividing the base clock SDCLK value.
*                       The Timeout value is a factor of the SDCLK by the following table:
*
*                                       Counter     |    Timeout Value
*                                       ------------|------------------
*                                       0] 0000b    |   SDCLK x 2^(13)
*                                       1] 0001b    |   SDCLK x 2^(14)
*                                       2] 0010b    |   SDCLK x 2^(15)
*                                       3] 0011b    |   SDCLK x 2^(16)
*                                           ...     |        ...
*                                      12] 1100b    |   SDCLK x 2^(25)
*                                      13] 1101b    |   SDCLK x 2^(26)
*                                      14] 1110b    |   SDCLK x 2^(27)
*                                      15] 1111b    |      RESERVED
*    
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_SetTimeoutData (FS_QTY      unit_nbr,
                                        CPU_INT32U  to_clks)
{
    (void)&unit_nbr;                                                /* Prevent compiler warning.                            */
        
    p_sdhc->K60_SDHC_SYSCTL |= SDHC_SYSCTL_DTOCV(14u);              /* Data Timeout Counter Value. Refer to Note(2)         */
}


/*
*********************************************************************************************************
*                                 FSDev_SD_Card_BSP_SetTimeoutResp()
*
* Description : Set response timeout.
*
* Argument(s) : unit_nbr    Unit number of SD/MMC card.
*
*               to_ms       Timeout, in milliseconds.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_Refresh().
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void  FSDev_SD_Card_BSP_SetTimeoutResp (FS_QTY      unit_nbr,
                                        CPU_INT32U  to_ms)
{
    (void)&unit_nbr;                                                /* Prevent compiler warning.                            */
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                    FSDev_SD_Card_BSP_ISR_Handler()
*
* Description : Interrupt handler for the SDIO.
*
* Argument(s) : source  Interrupt source undefined/ignored for peripheral interrupts.
*
* Return(s)   : none.
*
* Caller(s)   : FSDev_SD_Card_BSP_Open().
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

static  void  FSDev_SD_Card_BSP_ISR_Handler (void)
{    
                                                                    /* Clear Interrupt Signals                              */
    DEF_BIT_CLR(p_sdhc->K60_SDHC_IRQSIGEN, K60N512_BIT_IRQSIGEN_ALL_MASK);

    FS_OS_SemPost(&FSDev_SD_Sem);
}

#endif // (APP_CFG_FS_SD_CARD_EN == DEF_ENABLED)