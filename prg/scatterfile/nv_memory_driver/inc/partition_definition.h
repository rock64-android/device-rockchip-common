/**
 *       Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 *
 *            Sec Class: Intel Confidential (IC)
 *
 *
 * This document contains proprietary information belonging to IMC.
 * Denmark A/S. Passing on and copying of this document, use and communication
 * of its contents is not permitted without prior written authorisation.
 *
 * Description:
 *   Layout of spare area in NAND flash
 *
 * Revision Information:
 *  $File name:  /mhw_drv_src/nv_memory_driver/inc/partition_definition.h $
 *   Responsible: gjoerup
 *   Comment:
 *     Checkin performed by OptiCM
 */

#ifndef PARTITION_DEFINITION_H
#define PARTITION_DEFINITION_H

#define PARTITION_MARKER_NAND 0x00000000
#define PARTITION_MARKER_FTLE 0x00001234

// ------------------ PARTITION ID SPECIFICATION -------------------

#define  PARTITION_ID_PSI_IMAGE           0x00 /* partition ID for PSI code image           */
#define  PARTITION_ID_CODE_IMAGE          0x01 /* partition ID for main code image          */
#define  PARTITION_ID_ROOT_DISC           0x02 /* partition ID for root disc (RW)           */
#define  PARTITION_ID_CUSTOMIZATION_DISC  0x03 /* partition ID for customization disc (RO)  */
#define  PARTITION_ID_USER_DISC           0x04 /* partition ID for User Disc (RW - USBMS)   */
#define  PARTITION_ID_USER_DISC_EXTEND1   0x05 /* Extension to PARTITION_ID_USER_DISC        */
#define  PARTITION_ID_STA_SDS_IMAGE       0x06 /* partition ID for Static Simple Data Storage disc */
#define  PARTITION_ID_DYN_SDS_IMAGE       0x07 /* partition ID for Dynamic Simple Data Storage disc */
#define  PARTITION_ID_EXCEPTION_LOG_IMAGE 0x08 /* partition ID for exception log area */
#define  PARTITION_ID_FTL_STARTUP_DISC    0x09 /* partition ID for Flash Translation Layer Startup area */
#define  PARTITION_ID_SECURITY_IMAGE       0x0A /* partition ID for Security data and code */
#define  PARTITION_ID_DSP_IMAGE           0x0B /* partition ID for Aeneas firm ware */
#define  PARTITION_ID_DEBUG_DATA          0x0C /* partition ID for Debug data */
#define  PARTITION_ID_STARTUP_IMAGE       0x0D /* partition ID for Startup Image. */
#define  PARTITION_ID_PARTITION_TABLE     0x0E /* partition ID for the partition table */
#define  PARTITION_ID_CUSTOMIZATION_DATA  0x0F /* partition ID for RAW customization data */
#define  PARTITION_ID_SLB_IMAGE           0x10 /* partition ID for SLB code image */
#define  PARTITION_ID_UA_IMAGE            0x11 /* partition ID for UA partition */
#define  PARTITION_ID_CALIB_BACKUP_IMAGE  0x12 /* partition ID for holding calib backup. */
#define  PARTITION_ID_BAD_BLOCK_TABLE     0x13 /* partition ID for bad block table partition */
#define  PARTITION_ID_GSM_FW_IMAGE        0x14 /* partition ID for GSM firmware */
#define  PARTITION_ID_CDS_IMAGE           0x15 /* partition ID for core dump image */
#define  PARTITION_ID_NVM_STATIC_CALIB    0x16 /* partition ID for static calib data in NVM */
#define  PARTITION_ID_NVM_STATIC_FIX      0x17 /* partition ID for static fix data in NVM */
#define  PARTITION_ID_NVM_DYNAMIC         0x18 /* partition ID for dynamic data in NVM */
#define  PARTITION_ID_USBRO               0x19 /* partition ID for RO USB disc */
#define  PARTITION_ID_PAGING_IMAGE        0x1A /* partition ID for paging image */
#define  PARTITION_ID_CERTIFICATE_DATA    0x1B /* partition ID for certificate data. */
#define  PARTITION_ID_LTE                 0x1C /* partition ID for LTE */
#define  PARTITION_ID_CA5_MULTICORE       0x1D /* partition ID for Multicore system SW */
#define  PARTITION_ID_UA_BUFFER           0x1E /* partition ID for UA data */
#define  PARTITION_ID_NVM_BACKUP          0x1F /* partition ID for NVM backup*/

#define  PARTITION_ID_USER_DISC_EXTEND2            0x20
#define  PARTITION_ID_USER_DISC_EXTEND3            0x21
#define  PARTITION_ID_USER_DISC_EXTEND4            0x22
#define  PARTITION_ID_USER_DISC_EXTEND5            0x23
#define  PARTITION_ID_ROOT_DISC_EXTEND1            0x24
#define  PARTITION_ID_ROOT_DISC_EXTEND2            0x25
#define  PARTITION_ID_ROOT_DISC_EXTEND3            0x26
#define  PARTITION_ID_ROOT_DISC_EXTEND4            0x27
#define  PARTITION_ID_ROOT_DISC_EXTEND5            0x28
#define  PARTITION_ID_CUSTOMIZATION_DISC_EXTEND1   0x29
#define  PARTITION_ID_CUSTOMIZATION_DISC_EXTEND2   0x2A
#define  PARTITION_ID_CUSTOMIZATION_DISC_EXTEND3   0x2B
#define  PARTITION_ID_CUSTOMIZATION_DISC_EXTEND4   0x2C
#define  PARTITION_ID_CUSTOMIZATION_DISC_EXTEND5   0x2D

#define  PARTITION_ID_CUSTOMIZATION_DISC_TWO       0x2E
#define  PARTITION_ID_ROOT_DISC_TWO                0x2F

#define  PARTITION_ID_CODE_IMAGE_2                 0x30 /* partition ID for second code partition - for download recovery */
#define  PARTITION_ID_DSP_IMAGE_2                  0x31 /* partition ID for second AENEAS image – for download recovery */
#define  PARTITION_ID_LTE_2                        0x32 /* partition ID for second LTE image – for download recovery */
#define  PARTITION_ID_UA_IMAGE_2                   0x33 /* partition ID for second UA partition - for download recovery  */
#define  PARTITION_ID_UA_DELTA_PKG                 0x34 /* partition ID for second UA delta package - for download recovery  */
#define  PARTITION_ID_CA5_MULTICORE_2              0x35
#define  PARTITION_ID_LC_FW_2                      0x36 /* partition ID for second TDS DSP - for download recovery  */
#define  PARTITION_ID_CUSTOMIZATION_SCRIPT_2       0x37 /* partition ID for second customization script - for download recovery  */

//--------------LINUX / ANDROID -----------
#define  PARTITION_ID_VLX                 0x40
#define  PARTITION_ID_LINUX_KERNEL        0x41
#define  PARTITION_ID_INITRD              0x42
#define  PARTITION_ID_INITRD_RECOVERY     0x43
#define  PARTITION_ID_SYSTEM              0x44 /**< Android system filesystem */
#define  PARTITION_ID_USERDATA            0x45 /**< Android userdata filesystem */
#define  PARTITION_ID_CACHE               0x46 /**< Android cache filesystem */
#define  PARTITION_ID_BOOT_IMG            0x47 /**< Android packed kernel+initrd image */
#define  PARTITION_ID_RECOVERY_IMG        0x48 /**< Android packed kernel+initrd image for recovery mode */
#define  PARTITION_ID_RECOVERY_IMG_MIRROR 0x49 /**< Android packed kernel+initrd image for recovery mode - mirror */
#define  PARTITION_ID_BCB                 0x4A /**< Android bootloader control block for recovery */
#define  PARTITION_ID_KDUMP_IMG           0x4B /**< Linux packed kernel + initrd for kdump mode */
#define  PARTITION_ID_LINUX_NVM_FS        0x4C /**< Linux NVM filesystem */
#define  PARTITION_ID_PRIMARY_GPT         0x4E /**< partition ID for partition for the primary GPT and MBR */
#define  PARTITION_ID_SECONDARY_GPT       0x4F /**< partition ID for partition for the secondary GPT */

#define  PARTITION_ID_CUST_NVM_PLAIN      0x50 /* partition ID for customer NVM partition */
#define  PARTITION_ID_CUST_NVM_PROTECT    0x51 /* partition ID for customer NVM partition */
#define  PARTITION_ID_UI_DATA             0x52 /* partition ID for customer UI data */

//--------------Firmware switching -----------
#define  PARTITION_ID_CARRIER_IMAGE_1     0x61 /* partition ID for multi-carrier customized code image, used in fw switching solution*/
#define  PARTITION_ID_CARRIER_IMAGE_2     0x62
#define  PARTITION_ID_CARRIER_IMAGE_3     0x63
#define  PARTITION_ID_CARRIER_IMAGE_4     0x64
#define  PARTITION_ID_CARRIER_IMAGE_5     0x65
#define  PARTITION_ID_CARRIER_IMAGE_6     0x66
#define  PARTITION_ID_CARRIER_IMAGE_7     0x67
#define  PARTITION_ID_CARRIER_IMAGE_8     0x68
#define  PARTITION_ID_CARRIER_IMAGE_9     0x69
#define  PARTITION_ID_CARRIER_IMAGE_10    0x6A

//--------------New IDs for Mobilevisor environment -----------
#define PARTITION_ID_BOOTROM_PATCH         0x70
#define PARTITION_ID_UCODE_PATCH           0x71
#define PARTITION_ID_MODEM_IMAGE           0x72
#define PARTITION_ID_MOBILEVISOR_CONFIG    0x73
#define PARTITION_ID_MOBILEVISOR           0x74
#define PARTITION_ID_SPLASH_SCREEN         0x75
#define PARTITION_ID_SECURE_VM             0x76
#define PARTITION_ID_COMBO_RECOVERY        0x77
#define PARTITION_ID_COMBO_RECOVERY_MIRROR 0x78
#define PARTITION_ID_RECOVERY              0x79
#define PARTITION_ID_FW_UPDATE             0x7A
#define PARTITION_ID_VRL                   0x7B
#define PARTITION_ID_PSI_MIRROR            0x7C
#define PARTITION_ID_BOOTROM_PATCH_MIRROR  0x7D

#define  PARTITION_ID_CUSTOMIZATION_SCRIPT 0x80 /* partition ID for customization script */
#define  PARTITION_ID_LC_FW                0x81 /* partition ID for TDS DSP image */

#define  PARTITION_ID_RAW_NAND_IMAGE      0xE0 /* partition ID used for RAW NAND image up- and download */
#define  PARTITION_ID_CLONE_IMAGE         0xE0 /* partition ID used for CLONE image up- and download */
#define  PARTITION_ID_PREFLASH_IMAGE      0xE1 /* partition ID used if PREFLASH is required.            */

#define  PARTITION_ID_EMPTY               0xFF /* partition ID for an empty block */

//-------------------- IMAGE TYPE SPECIFICATION FOR NAND DEVICES ----------------------

#define  PARTITION_IT_FAT_IMAGE           0x01 /* Type Fatfile System  (FTuL image)                    (FTL & BBR) */
#define  PARTITION_IT_YAFFS_IMAGE         0x02 /* Type Yet Another Flash File System (Not supported)   (unused!!)  */
#define  PARTITION_IT_JFFS_IMAGE          0x03 /* Type Journaling Flash File System (Not supported)    (unused!!)  */
#define  PARTITION_IT_FFS_IMAGE           0x04 /* Type Flash File System (Not supported)               (unused!!)  */
#define  PARTITION_IT_CRAMFS_IMAGE        0x05 /* Type CRAM File System (Not supported)                (unused!!)  */
#define  PARTITION_IT_RAW_IMAGE           0x06 /* Type Code Image (Non FTL image)                      (neither)   */
#define  PARTITION_IT_SDS_IMAGE           0x07 /* Type Simple Data Storage Image (FTuL image)          (FTL & BBR) */
#define  PARTITION_IT_FTL_IMAGE           0x08 /* Type Flash Translation Layer Image (FTuL image)      (FTL & BBR) */
#define  PARTITION_IT_BBR_IMAGE           0x09 /* Type Bad Block Reallocation Layer Image (BBR image)  (BBR)       */
#define  PARTITION_IT_BOOTUP_IMAGE        0x0A /* Type SLB or PSI image (Non FTL image)                (neither)   */

//-------------------- PHYSICAL PARTITION SPECIFICATION FOR EMMC DEVICES ----------------------

#define  PARTITION_PP_USER_PARTITION      0x00
#define  PARTITION_PP_BOOT_PARTITION_1    0x01
#define  PARTITION_PP_BOOT_PARTITION_2    0x02
#define  PARTITION_PP_RPMB_PARTITION      0x03

//-------------------- OPTIONS SPECIFICATION ------------------

#define  PARTITION_OPT_VERIFICATION_REQUIRED           0x0000000000000001ULL /* Verification of the image contents and signature required at startup */
#define  PARTITION_OPT_READONLYIMAGE                   0x0000000000000002ULL /* This image has to be protected with FCDP at startup time             */
#define  PARTITION_OPT_COPYTORAM                       0x0000000000000004ULL /* This image has to be copied in to RAM at startup time by SLB         */
#define  PARTITION_OPT_WIPE_OUT_DATA                   0x0000000000000008ULL /* Data in this image has to be wiped-out when deleting                 */
#define  PARTITION_OPT_LOADMAP                         0x0000000000000010ULL /* Loadmap has to be located as the first in this image                 */
#define  PARTITION_OPT_SHORT_TIMEOUT                   0x0000000000000020ULL /* Specify that only short timeout for POW for this partition           */
#define  PARTITION_OPT_PAGING_DATA                     0x0000000000000040ULL /* This partition contains data object for paging                       */
#define  PARTITION_OPT_RO_BIT_CORRECTION_OK            0x0000000000000080ULL /* This partitions access right can be changed temporary to correct bit error */
#define  PARTITION_OPT_ACCEPTABLE_BIT_ERRORS           0x0000000000000700ULL /* Use 3 bits to determine number of acceptable bit errors. 0 = 0 bit, 1 = 1/8 * ECC_level, 2 = 2/8 * ECC_level, etc */
#define  PARTITION_OPT_TEST_ACCEPTED                   0x0000000000000800ULL /* Specify that testsuite may modify contents on this partition. */
#define  PARTITION_OPT_ALLOW_UTA_BLOCK_IF_ACCESS       0x0000000000001000ULL /* Specify that this partition may be accessed from UTA flash block interface (FOTA access)*/
#define  PARTITION_OPT_ALLOW_UTA_BYTE_IF_ACCESS        0x0000000000002000ULL /* Specify that this partition may be accessed from UTA flash byte interface (Normal access)*/
#define  PARTITION_OPT_SECTOR_BASED_PT                 0x0000000000004000ULL /* The block numbers in the partition entry is based on 512B sectors, not the actual block size*/
#define  PARTITION_OPT_PRESERVE_PARTITION              0x0000000000008000ULL /* On subsequent downloads or erases this partition will be preserved. */

#define  PARTITION_OPT_NO_SEC_PACK_VALIDATION          0x0000000004000000ULL /* Indicates if legacy validation to be skipped */

#define  PARTITION_OPT_LOAD_GROUP_NONE                 0x0000000000000000ULL
#define  PARTITION_OPT_LOAD_GROUP_1                    0x0000000000010000ULL /* LOAD_GROUP can be used to */
#define  PARTITION_OPT_LOAD_GROUP_2                    0x0000000000020000ULL /* specify a set of partitions */
#define  PARTITION_OPT_LOAD_GROUP_3                    0x0000000000040000ULL /* which belongs to same boot sequence. */
#define  PARTITION_OPT_BACKUP_DATA_REQUIRED            0x0000000000040000ULL /* Use this bit to decide if partition need to be back during pt change */
#define  PARTITION_OPT_LOAD_GROUP_4                    0x0000000000080000ULL /* E.g CODE_IMAGE, DSP_IMAGE and LTE has LOAD_GROUP_1 set */
#define  PARTITION_OPT_BACKUP_DATA_DEST                0x0000000000080000ULL /* Use this bit to decide which is the backup partition */
#define  PARTITION_OPT_LOAD_GROUP_5                    0x0000000000100000ULL /* while CODE_IMAGE_2, DSP_IMAGE_2 and LTE_2 has LOAD_GROUP_2 set */
#define  PARTITION_OPT_LOAD_GROUP_6                    0x0000000000200000ULL /* Then it is possible in e.g. a FOTA update to load all */
#define  PARTITION_OPT_LOAD_GROUP_7                    0x0000000000400000ULL /* images from GROUP_1 or GROUP_2 dependent on which GROUP is being updated */
#define  PARTITION_OPT_LOAD_GROUP_MASK                 0x00000000007F0000ULL

#define  PARTITION_OPT_FS_DEVICE_ACCESS_INTERNAL       0x0000000000010000ULL /* */
#define  PARTITION_OPT_FS_DEVICE_ACCESS_EXTERNAL       0x0000000000020000ULL /* */
#define  PARTITION_OPT_FS_DEVICE_ACCESS_BOTH           0x0000000000030000ULL /* */
#define  PARTITION_OPT_FS_DEVICE_PLP_DISABLE           0x0000000000040000ULL /* */
#define  PARTITION_OPT_FS_DEVICE_PLP_RUN_CHECK_DISK    0x0000000000080000ULL /* */
#define  PARTITION_OPT_FS_DEVICE_PLP_ENABLE            0x0000000000100000ULL /* */
#define  PARTITION_OPT_FS_DEVICE_SYSTEM_DISK           0x0000000000200000ULL /* */
#define  PARTITION_OPT_FS_DEVICE_NOT_SYSTEM_DISK       0x0000000000400000ULL /* Explicit 'NOT a System-desk' option to maintain backwards-compatibility with older PT's. */
#define  PARTITION_OPT_FS_PARTITION_REGISTER           0x0000000000800000ULL /* Specify Partition has to be registred as filesystem. */
#define  PARTITION_OPT_FS_DEVICE_ALLOCATION_SCATTER    0x0000000004000000ULL /* */
#define  PARTITION_OPT_FS_DEVICE_ALLOCATION_NORMAL     0x0000000008000000ULL /* */

#define  PARTITION_OPT_SECURE_ERASE                    0x0000000001000000ULL /**< (FTLE) This partition must be erased using secure erase if supported by driver */

#define  PARTITION_OPT_ALIGNMENT_MASK                  0x00000000F0000000ULL /* Alignment bit mask - 2^"BIT MASK" e.g. 4K alignment => BIT MASK == 12 */


/* -----------------   OPTION SPECIFICATION for PSI -------------------------------------------*/

#define  PARTITION_OPT_PSI_ALTERNATE_BOOT              0x0000000000010000ULL /* Alternate boot for PSI. If this PT option is set, PSI is downloaded as 1024 main + 128 spare */

/* ------------------- PARTITION ATTRIBUTES FOR EMMC DEVICES ---------------------------------*/

#define  PARTITION_ATTR_GPT_PARTITION                       0x00000001  /**< Enable if IMC partitions should be added to GPT  */
#define  PARTITION_ATTR_GPT_SYSTEM_PARTITION                0x00000002  /**< GPT partition attribute: System partition (disk partitioning utilities must preserve the partition as is) */
#define  PARTITION_ATTR_GPT_HIDE_EFI                        0x00000004  /**< GPT partition attribute: Hide from EFI */
#define  PARTITION_ATTR_GPT_LEGACY_BIOS_BOOTABLE            0x00000008  /**< GPT partition attribute: Legacy BIOS bootable */
#define  PARTITION_ATTR_GPT_READ_ONLY                       0x00000010  /**< GPT partition attribute: Read-only*/
#define  PARTITION_ATTR_GPT_HIDDEN                          0x00000020  /**< GPT partition attribute: Hidden*/
#define  PARTITION_ATTR_GPT_DO_NOT_AUTOMOUNT                0x00000040  /**< GPT partition attribute: Do not automount (i.e., do not assign drive letter)*/
#define  PARTITION_ATTR_GPT_TYPE_GUID_a                     0x00000100  /**< GPT partition attribute: selection of type GUID for the partition BIT0*/
#define  PARTITION_ATTR_GPT_TYPE_GUID_b                     0x00000200  /**< GPT partition attribute: selection of type GUID for the partition BIT1 */
#define  PARTITION_ATTR_GPT_TYPE_GUID_c                     0x00000400  /**< GPT partition attribute: selection of type GUID for the partition BIT2*/
#define  PARTITION_ATTR_GPT_TYPE_MASK                       0x00000700  /**< GPT partition attribute: selection of type GUID for the partition MASK for BIT0-2*/

#define  PARTITION_ATTR_POWER_ON_WP_NORMAL_MODE             0x00010000  /**< Group a number of partitions to be HW write protected in normal boot*/
#define  PARTITION_ATTR_POWER_ON_WP_UPDATE_MODE             0x00020000  /**< Group a number of partitions to be HW write protected in update boot*/
#define  PARTITION_ATTR_POWER_ON_WP_PRODUCTION_MODE         0x00040000  /**< Group a number of partitions to be HW write protected in production mode*/
#define  PARTITION_ATTR_POWER_ON_WP_SPARE_MODE              0x00080000  /**< Extra mode, not in use*/
#define  PARTITION_ATTR_WRITE_PROTECT_GROUP_MASK            0x000F0000  /**< Mask out all HW WP groups */

// The type guid are located at bit 8-10, hence the ftle_attribut need to be bitshifted 8 times and masked by "FTLE_ATTR_GPT_TYPE_MASK".
#define PARTITION_ATTR_GPT_TYPE_GUID_POS 8

#endif /* PARTITION_DEFINITION_H */
