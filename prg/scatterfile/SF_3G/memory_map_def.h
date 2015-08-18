/*----------------------------------------------------------------------------*/
/*BOOTLOADER*/
/*----------------------------------------------------------------------------*/
#define  MMAP_SLB_BASE    0x100
#define  MMAP_SLB_SIZE    0x100000
#define  MMAP_EBL_BASE    0x100
#define  MMAP_EBL_SIZE    0x100000
#define  MMAP_BOOTROM_PATCH_BASE    PARTITION_ID_BOOTROM_PATCH
#define  MMAP_BOOTROM_PATCH_SIZE    0x20000
#define  MMAP_UCODE_PATCH_BASE    PARTITION_ID_UCODE_PATCH
#define  MMAP_UCODE_PATCH_SIZE    0x3000
#define  MMAP_SPLASH_SCREEN_BASE    PARTITION_ID_SPLASH_SCREEN
#define  MMAP_SPLASH_SCREEN_SIZE    0xa10000
#define  MMAP_PSI_RAM_BASE    0xFFF00000
#define  MMAP_PSI_RAM_SIZE    0x20000
#define  MMAP_PSI_FLASH_BASE    0xFFF00000
#define  MMAP_PSI_FLASH_SIZE    0x20000

#define MMAP_VRL_BASE   0x79
#define MMAP_VRL_SIZE   0x40000

#define MMAP_PSI_MIRROR_BASE  0x7A
#define MMAP_PSI_MIRROR_SIZE  0x20000

#define MMAP_BOOTROM_PATCH_MIRROR_BASE  0x7B
#define MMAP_BOOTROM_PATCH_MIRROR_SIZE  0x20000

/*----------------------------------------------------------------------------*/
/*MODEM*/
/*----------------------------------------------------------------------------*/
#define  MMAP_CODE_IMAGE_BASE    0x1D000000

/*----------------------------------------------------------------------------*/
/*VMM*/
/*----------------------------------------------------------------------------*/
#if defined(VMM_MOBILEVISOR)
#define  MMAP_MOBILEVISOR_CONFIG_BASE    0x1CC00000
#define  MMAP_MOBILEVISOR_CONFIG_SIZE    0x40000
#endif
#if defined(VMM_MOBILEVISOR)
#define  MMAP_MOBILEVISOR_BASE    0x1CC40000
#define  MMAP_MOBILEVISOR_SIZE    0xC0000
#endif

/*----------------------------------------------------------------------------*/
/*SECURE_VM*/
/*----------------------------------------------------------------------------*/
#if defined(VMM_MOBILEVISOR)
#define  MMAP_SECURE_VM_BASE    0x1C000000
#define  MMAP_SECURE_VM_SIZE    0x00C00000
#endif

/*----------------------------------------------------------------------------*/
/*LINUX*/
/*----------------------------------------------------------------------------*/
#if defined(VMM_MOBILEVISOR) && !defined(STANDALONE_VMODEM)
#define  MMAP_BOOT_IMG_BASE    0x1B000000
#define  MMAP_BOOT_IMG_SIZE    0x1000000
#endif

#if defined(VMM_MOBILEVISOR) && defined(STANDALONE_VMODEM)
#define  MMAP_BOOT_IMG_BASE    0x00800000
#define  MMAP_BOOT_IMG_SIZE    0x1000000
#endif

#if defined(VMM_MOBILEVISOR)
#define  MMAP_RECOVERY_BASE    0x1B000000
#define  MMAP_RECOVERY_SIZE    0x1000000
#endif
#if defined(VMM_MOBILEVISOR)
#define  MMAP_FW_UPDATE_BASE    PARTITION_ID_FW_UPDATE
#define  MMAP_FW_UPDATE_SIZE    0x1000000
#endif

#if defined(VMM_MOBILEVISOR) 
#define MMAP_COMBO_RECOVERY_BASE PARTITION_ID_COMBO_RECOVERY
#define MMAP_COMBO_RECOVERY_SIZE 0x1100000
#endif
#if defined(VMM_MOBILEVISOR)
#define MMAP_COMBO_RECOVERY_MIRROR_BASE PARTITION_ID_COMBO_RECOVERY_MIRROR
#define MMAP_COMBO_RECOVERY_MIRROR_SIZE 0x1100000
#endif

#define MMAP_SECURITY_DATA_BASE  0x0
#define MMAP_SECURITY_DATA_SIZE  (0x100000)

/*----------------------------------------------------------------------------*/
/*ANDROID*/
/*----------------------------------------------------------------------------*/
#if defined(VMM_MOBILEVISOR)
#define  MMAP_LINUX_NVM_FS_BASE    0x00000000
#define  MMAP_LINUX_NVM_FS_SIZE    0x400000
#endif
#if defined(VMM_MOBILEVISOR)
#define  MMAP_CACHE_BASE    PARTITION_ID_CACHE
#define  MMAP_CACHE_SIZE    0x1a900000
#endif
#if defined(VMM_MOBILEVISOR)
#define  MMAP_SYSTEM_BASE    PARTITION_ID_SYSTEM
#define  MMAP_SYSTEM_SIZE    0x80000000
#endif
#if defined(VMM_MOBILEVISOR)
#define  MMAP_USERDATA_BASE    PARTITION_ID_USERDATA
#endif
