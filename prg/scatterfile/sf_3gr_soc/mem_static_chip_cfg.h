
/*  -------------------------------------------------------------------------
    Copyright (C) 2013-2014 Intel Mobile Communications GmbH
    
         Sec Class: Intel Confidential (IC)
    
     ----------------------------------------------------------------------
     Revision Information:
       $$File name:  /mhw_drv_src/memory_management/mem/scatterfile/xgold632/mem_static_chip_cfg.h $
     ---------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------------------------------------------
   ITCM configuration
   -------------------------------------------------------------------------------------------------------------------------------- */
#define MMAP_MODEM_SW_ITCM_BASE                  0
#define MMAP_MODEM_SW_ITCM_SIZE                  0

/* --------------------------------------------------------------------------------------------------------------------------------
   DTCM configuration
   -------------------------------------------------------------------------------------------------------------------------------- */
#define MMAP_MODEM_SW_DTCM_BASE                  0
#define MMAP_MODEM_SW_DTCM_SIZE                  0

/* --------------------------------------------------------------------------------------------------------------------------------
   IRAM configuration
   -------------------------------------------------------------------------------------------------------------------------------- */
#if defined (STANDALONE_VMODEM) || (VMM_MOBILEVISOR)
#define MMAP_MODEM_SW_IRAM_BASE                  0xFFF11000
#define MMAP_MODEM_SW_IRAM_SIZE                  0xF000
#else // NATIVE MODEM
#define MMAP_MODEM_SW_IRAM_BASE                  0xFFF10000
#define MMAP_MODEM_SW_IRAM_SIZE                  0x10000
#endif

/* --------------------------------------------------------------------------------------------------------------------------------
   IROM configuration
   -------------------------------------------------------------------------------------------------------------------------------- */
#define MMAP_MODEM_SW_IROM_BASE                  0xFFFC0000
#define MMAP_MODEM_SW_IROM_SIZE                  0x00040000

/* --------------------------------------------------------------------------------------------------------------------------------
   EBU address space configuration
   -------------------------------------------------------------------------------------------------------------------------------- */
#define MMAP_EBU_PORT_CS0_BASE                   0x00000000
#define MMAP_EBU_PORT_CS1_BASE                   0x40000000
#define MMAP_EBU_PORT_CS2_BASE                   0
#define MMAP_EBU_PORT_CS_NA_BASE                 0


/* --------------------------------------------------------------------------------------------------------------------------------
   On-Chip Trace buffer configuration
   -------------------------------------------------------------------------------------------------------------------------------- */
#define MMAP_MODEM_SW_TRACE_BASE                 0x80000000
#define MMAP_MODEM_SW_TRACE_SIZE                 0x01000000

/* --------------------------------------------------------------------------------------------------------------------------------
   Peripheral space configuration
   -------------------------------------------------------------------------------------------------------------------------------- */
#define MMAP_MODEM_SW_PERIP_BASE                 0xE0000000
#define MMAP_MODEM_SW_PERIP_SIZE                 0x10000000

/* --------------------------------------------------------------------------------------------------------------------------------
   IRAM WAKEUP DATA configuration (defined by bootloader)
   -------------------------------------------------------------------------------------------------------------------------------- */
#define MMAP_MODEM_SW_IRAM_WAKEUP_DATA_ADDRESS   0xFFF10100

/* --------------------------------------------------------------------------------------------------------------------------------
   IRAM BOOTSTACK DATA configuration (defined by bootrom)
   -------------------------------------------------------------------------------------------------------------------------------- */
#define MMAP_MODEM_SW_IRAM_BOOTSTACK_BASE        (MMAP_MODEM_SW_IRAM_BASE + MMAP_MODEM_SW_IRAM_SIZE - 0x900)

/* --------------------------------------------------------------------------------------------------------------------------------
   IRAM NOCLEAR DATA configuration (defined by bootrom)
   -------------------------------------------------------------------------------------------------------------------------------- */
#if defined(FEAT_VMM_SUPPORT) || defined(VMM_MOBILEVISOR)
#define MMAP_MODEM_SW_IRAM_NEXT_BOOTMODE_REQ_SIZE     (64)
#define MMAP_MODEM_SW_IRAM_NEXT_BOOTMODE_REQ_BASE    (MMAP_MODEM_SW_IRAM_BASE + MMAP_MODEM_SW_IRAM_SIZE - MMAP_MODEM_SW_IRAM_NEXT_BOOTMODE_REQ_SIZE)

#define MMAP_MODEM_SW_IRAM_NOCLEARDATA_SIZE    (0x800 - MMAP_MODEM_SW_IRAM_NEXT_BOOTMODE_REQ_SIZE)
#define MMAP_MODEM_SW_IRAM_NOCLEARDATA_BASE    (MMAP_MODEM_SW_IRAM_BASE + MMAP_MODEM_SW_IRAM_SIZE - (MMAP_MODEM_SW_IRAM_NOCLEARDATA_SIZE + MMAP_MODEM_SW_IRAM_NEXT_BOOTMODE_REQ_SIZE))
#else   
#define MMAP_MODEM_SW_IRAM_NOCLEARDATA_SIZE      0x800
#define MMAP_MODEM_SW_IRAM_NOCLEARDATA_BASE      (MMAP_MODEM_SW_IRAM_BASE + MMAP_MODEM_SW_IRAM_SIZE - MMAP_MODEM_SW_IRAM_NOCLEARDATA_SIZE)
#endif

/* --------------------------------------------------------------------------------------------------------------------------------
   IRAM VER HWID sync configuration (defined by bootloader)	
   --------------------------------------------------------------------------------------------------------------------------------*/
#if defined(GPIO_SHM_VER_SUPPORT) || defined(ADC_SHM_VER_SUPPORT)
   #define MMAP_MODEM_SW_IRAM_HWID_SYNC_SIZE      32
#else
   #define MMAP_MODEM_SW_IRAM_HWID_SYNC_SIZE      0
#endif

#define MMAP_MODEM_SW_IRAM_HWID_SYNC_BASE      MMAP_MODEM_SW_IRAM_NOCLEARDATA_BASE - MMAP_MODEM_SW_IRAM_HWID_SYNC_SIZE

/* --------------------------------------------------------------------------------------------------------------------------------
   IRAM EMIC Shared settings configuration (defined by bootrom)
   -------------------------------------------------------------------------------------------------------------------------------- */
#if defined(EMIC_SHARED_MEM_SETTINGS)
  #define MODEM_SW_EMIC_SHARED_SETTINGS_SIZE               0x00000800
  #define MODEM_SW_EMIC_SHARED_SETTINGS_BASE               0xFFF1D800
  #define MMAP_MODEM_SW_IRAM_EMIC_SHARED_SETTINGS_SIZE     MODEM_SW_EMIC_SHARED_SETTINGS_SIZE
  #define MMAP_MODEM_SW_IRAM_EMIC_SHARED_SETTINGS_BASE     MODEM_SW_EMIC_SHARED_SETTINGS_BASE
#endif


/* --------------------------------------------------------------------------------------------------------------------------------
   FlashTool related stuffs
   -------------------------------------------------------------------------------------------------------------------------------- */
#define PLATFORM_CHIPSET                         SOFIA_3G
#define PLATFORM_BOOTSPEED                       0x0001C200
#define PLATFORM_HANDSHAKE_MODE                  0
#define PLATFORM_USBCAPABLE                      0

#if defined(NAND_ONLY) || defined(FEAT_FTLE_PRGGEN)
  #define PLATFORM_FLASHTECHNOLOGY               1
#else
  #define PLATFORM_FLASHTECHNOLOGY               0
#endif
