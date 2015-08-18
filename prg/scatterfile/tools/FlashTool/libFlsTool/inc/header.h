/*   ----------------------------------------------------------------------
     Copyright (C) 2014 Intel Mobile Communications GmbH

          Sec Class: Intel Confidential (IC)

     Copyright (C) 2008-2008 Infineon Technologies Denmark A/S
     ----------------------------------------------------------------------
     Revision Information:
       $$File name:  /msw_tools/FlashTool/libFlsTool/inc/header.h $
     ----------------------------------------------------------------------
     by CCCT (v0.7)
     ---------------------------------------------------------------------- */
#ifndef __HEADER_H__
#define __HEADER_H__

#if defined(__GNUC__ ) && !defined(__ICC) //don't use this header file in ICC build
  #include <stdint.h>
  #include "tool_versions.h"
#endif

#ifndef __fastcall
  #define __fastcall
#endif

/*
 * Note:
 * This is a workaround, since the NAND_PRESENT define is apparently
 * important inside "nand_spare_area_internal.h".
 *
 * Neither of these three defines are needed in this header:
 *  BOOT_INTERFACE_NAND , PSI_TYPE_RAM , NAND_PRESENT
 * #if defined(BOOT_INTERFACE_NAND) && !defined(PSI_TYPE_RAM)
 *
 */
#if defined(NAND_PRESENT) || defined(NAND_ONLY)
  #include "nand_spare_area_internal.h"  /* This is the root-definitions of the NAND partition table */
#else
/* This is "SNATCHED" from the nand_spare_area_internal.h */
  #define MAX_PARTITIONS 28

  typedef struct
  {
    uint32_t ID;                         //ID of partition which is specified in this entry.
    uint32_t StartBlock;                 //Physical start address of region, in which the partition is located.
    uint32_t EndBlock;                   //Physical end address of region, in which the partition is located.
    uint32_t MaxBlocksInSubPartition;    //Number of blocks in partition.
    uint32_t StartReservoirBlock;        //Physical start address of the reservoir that the partition is using.
    uint32_t EndReservoirBlock;          //Physiacl end address of the reservoir that the partition is using.
    uint32_t ImageType;                  //Type specification of the partition
    uint32_t Options;                    //Specific options for this image
    uint32_t Reserved;                   //FOTA virtual address.
  } PartitionEntryStructType;

  //NOTE: if MaxBlocksInSubPartition is 0x0, the Partition entry does not specify any partition.
  typedef struct
  {
    uint32_t PartitionMarker;            /* Magic number to identify possible valid data*/
    PartitionEntryStructType Partition[MAX_PARTITIONS];
    uint32_t Reserved[3];
    //uint32_t PartitionCheckSum;
  } PartitionStructType;
#endif


#define PRG_MAGIC_NUMBER 0x693729F1
#define GENERIC_UID 0 // To start with, all the elements will have same UID as 0

#define E_GOLD_CHIPSET_V2     0
#define E_GOLD_LITE_CHIPSET   1
#define S_GOLD_CHIPSET_V1     2
#define S_GOLD_LITE_CHIPSET   3
#define E_GOLD_CHIPSET_V3     4
#define S_GOLD_CHIPSET_V2     5
#define S_GOLD_CHIPSET_V3     6
#define E_GOLD_RADIO_V1       7
#define E_GOLD_VOICE_V1       8
#define S_GOLD_RADIO_V1       9
#define X_GOLD_618            10
#define X_GOLD_102            11
#define X_GOLD_213            12
#define X_GOLD_110            13
#define X_GOLD_626            14
#define X_GOLD_223            15
#define X_GOLD_215            16
#define X_GOLD_706            17
#define X_GOLD_636            18
#define X_GOLD_225            19
#define X_GOLD_716            20
#define X_GOLD_631            21
#define X_GOLD_221            22
#define X_GOLD_726            23
#define X_GOLD_632            24
#define X_GOLD_222            25
#define X_GOLD_633            26
#define SOFIA_3G              27
#define SOFIA_LTE             28

#define NUM_CHIPSELECT_OPTIONS 29

#define NOR_FLASH    0
#define NAND_FLASH   1
#define FLASH_LESS   2


#define LOAD_MAGIC 0x01FEDABE

#define MAX_FILENAME_LENGTH 128

typedef enum
{
  PRG_FILE_TYPE = 1,
  FLS_FILE_TYPE,
  EEP_FILE_TYPE,
  DFFS_FILE_TYPE,
  DISK_FILE_TYPE,
  NUM_FILE_TYPES // This should be the last
} prg_file_type;

typedef enum
{
  GENERIC_HEADER_ELEM_TYPE = 1000,//0x3E8
  END_MARKER_ELEM_TYPE=2,
  PRGHANDLER_TOOL_ELEM_TYPE, // All the tool version element types
  PRGSEQUENCER_TOOL_ELEM_TYPE,
  MAKEPRG_TOOL_ELEM_TYPE,
  HEXTOFLS_TOOL_ELEM_TYPE,
  FLSSIGN_TOOL_ELEM_TYPE,
  DWDTOHEX_TOOL_ELEM_TYPE,
  FLASHTOOL_TOOL_ELEM_TYPE,
  STORAGETOOL_TOOL_ELEM_TYPE,
  MEMORYMAP_ELEM_TYPE,
  DOWNLOADDATA_ELEM_TYPE,
  HARDWARE_ELEM_TYPE,
  NANDPARTITION_ELEM_TYPE,
  SECURITY_ELEM_TYPE,
  TOC_ELEM_TYPE,
  SAFE_BOOT_CODEWORD_ELEM_TYPE,
  INJECTED_PSI_ELEM_TYPE,
  INJECTED_EBL_ELEM_TYPE,
  GENERIC_VERSION_ELEM_TYPE,
  INJECTED_PSI_VERSION_ELEM_TYPE,
  INJECTED_EBL_VERSION_ELEM_TYPE,
  ROOT_DISK_ELEM_TYPE,
  CUST_DISK_ELEM_TYPE,
  USER_DISK_ELEM_TYPE,
  FLSTOHEADER_TOOL_ELEM,
  BOOT_CORE_TOOL_ELEM,
  EBL_TOOL_ELEM,
  INDIRECT_DOWNLOADDATA_ELEM_TYPE,
  FLSTONAND_TOOL_ELEM,
  INJECTED_INI_ELEM_TYPE,
  INJECTED_PDF_ELEM_TYPE,
  NUM_ELEM_TYPES // This should be the last
} prg_element_type;

// --------------------------
// Memory Class defines
//---------------------------
#define NOT_USED              0
#define BOOT_CORE_PSI_CLASS   1
#define BOOT_CORE_SLB_CLASS   2
#define BOOT_CORE_EBL_CLASS   3
#define CODE_CLASS            4
#define CUST_CLASS            5
#define DYN_EEP_CLASS         6
#define STAT_EEP_CLASS        7
#define SWAP_BLOCK_CLASS      8
#define SEC_BLOCK_CLASS       9
#define EXCEP_LOG_CLASS       10
#define STAT_FFS_CLASS        11
#define DYN_FFS_CLASS         12
#define DSP_SW_CLASS          13
#define ROOT_DISK_CLASS       14
#define CUST_DISK_CLASS       15
#define USER_DISK_CLASS       16
#define HEX_EXTRACT_CLASS     17

#define NUM_MEM_CLASSES    18

#if defined(INCLUDE_MEM_CLASS_NAME_ARRAY)
const char mem_class_name[NUM_MEM_CLASSES][20] = {
  "NOT USED",
  "BOOT CORE PSI",
  "BOOT CORE SLB",
  "BOOT CORE EBL",
  "CODE",
  "CUST",
  "DYN EEP",
  "STAT EEP",
  "SWAP BLOCK",
  "SEC BLOCK",
  "EXCEPTION LOG",
  "STATIC FFS",
  "DYNAMIC_FFS",
  "DSP SW",
  "ROOT DISK",
  "CUST DISK",
  "USER DISK",
  "HEX EXTRACT"
 };
#endif

// --------------------------
// Load Map Flags
//---------------------------
#define BOOT_CORE_HASH_MARKER   0x00000001  //1
#define ALLOW_SEC_BOOT_UPDATE   0x00000002  //2
#define DONT_TOUCH_REGION       0x00000004  //4
#define SKIP_VERSION_CHECK      0x00000008  //8

#define ERASE_TOTAL_LENGTH      0x00000010  //16
#define ERASE_ONLY              0x00000020  //32
#define COMPRESSED_IMAGE        0x00000040  //64
#define COMPRESSED_TRANSFER     0x00000080  //128

#define USE_DRIVER_FROM_FILE    0x00000100  //256
#define NAND_SKIP_PT_CHECK      0x00000200  //512  /* Written by download DLL in case of binary upload from NAND */
#define IMAGE_HAS_PUBLIC_KEYS   0x00000400  //1024 /* The key copy function will attempt to copy the keys to this image - If not set then loadmap[0] of the latest CODE image will be used */
#define IMAGE_IS_BOOT_IMAGE     0x00000800  //2048 /* The SLB will attemt to boot this image - If not set then loadmap[0] of the latest CODE image will be used */

#define IMAGE_IS_ENO_SW         0x00001000  /* This is an ENO SW */
#define LOADMAP_OPT_RESERVED1   0x00002000  //RESERVED
#define LOADMAP_OPT_RESERVED2   0x00004000  //RESERVED
#define LOADMAP_OPT_RESERVED3   0x00008000  //RESERVED

#define LOAD_TO_RAM             0x00010000
#define LOAD_TO_SD              0x00020000
#define NO_LOAD_TO_RAM          0x00040000
#define LOADMAP_OPT_RESERVED4   0x00080000  //RESERVED

#define EXCLUDE_UTA_FLASH_BYTE  0x00100000
#define EXCLUDE_VALIDATION      0x00200000
#define EXCLUDE_LOAD_TO_RAM     0x00400000
#define EXCLUDE_PAGING          0x00800000

#define LOADMAP_OPT_RESERVED5   0x01000000  //RESERVED
#define LOADMAP_OPT_RESERVED6   0x02000000  //RESERVED
#define LOADMAP_OPT_RESERVED7   0x04000000  //RESERVED
#define LOADMAP_OPT_RESERVED8   0x08000000  //RESERVED

#define LOADMAP_OPT_RESERVED9   0x10000000  //RESERVED
#define ERASE_TOTAL             0x20000000
#define NVM_REGION              0x40000000
#define ERASE_REGION            0x80000000


// --------------------------
// Element Header Structure
//---------------------------
typedef struct {
  uint32_t Type;
  uint32_t Size;
  uint32_t UID;
}ElementHeaderStructType;

// --------------------------
// Generic Header Structures
//---------------------------
typedef struct {
  uint32_t FileType; // prg,fls,eep,dffs ......
  uint32_t Marker; // magic number
  uint32_t Sha1Digest[5];
}GenericHeaderDataStructType;

typedef struct {
 ElementHeaderStructType Header;
 GenericHeaderDataStructType Data;
}GenericHeaderElementStructType;

// --------------------------
// Safe boot codeword Structure
//---------------------------
typedef struct {
  uint32_t Enabled;
  uint32_t Addr;
  uint32_t Value1;
  uint32_t Value2;
}CodewordStructType;

typedef struct {
  ElementHeaderStructType Header;
  CodewordStructType Data;
}SafeBootCodewordElementStructType;


// --------------------------
// Tool Structures
//---------------------------
typedef struct {
  uint32_t Required;
  uint32_t Used;
}ToolVersionStructType;

typedef struct {
  ElementHeaderStructType Header;
  ToolVersionStructType Data;
}ToolVersionElementStructType;

// --------------------------
// End Marker Structure
//---------------------------
typedef struct {
  ElementHeaderStructType Header;
}EndElementStructType;

// --------------------------
// Version data Structure
//---------------------------
typedef struct {
  uint32_t DataLength;
  uint8_t *Data;
  uint32_t DataOffset;
}VersionDataStructType;

typedef struct {
  ElementHeaderStructType Header;
  VersionDataStructType Data;
}VersionDataElementStructType;

// --------------------------
// Storage Structure
//---------------------------
typedef struct {
  uint32_t DataLength;
  uint8_t *Data;
  uint32_t DataOffset;
}StorageDataStructType;

typedef struct {
  ElementHeaderStructType Header;
  VersionDataStructType Data;
}StorageDataElementStructType;

// --------------------------
// Injection data Structure
//---------------------------
typedef struct {
  uint32_t CRC_16;
  uint32_t CRC_8;
  uint32_t DataLength;
  uint8_t *Data;
  uint32_t DataOffset;
}InjectionDataStructType;

typedef struct {
  ElementHeaderStructType Header;
  InjectionDataStructType Data;
}InjectionElementStructType;

// --------------------------
// Download data Structure
//---------------------------
typedef struct {
  uint32_t LoadMapIndex;
  uint32_t CompressionAlgorithm;
  uint32_t CompressedLength;
  uint32_t CRC;
  uint32_t DataLength;
  uint8_t *Data;
  uint32_t DataOffset;
}DownloadDataStructType;

typedef struct {
  ElementHeaderStructType Header;
  DownloadDataStructType Data;
}DownloadDataElementStructType;

// --------------------------
// Indirect Download data Structure
//---------------------------
typedef struct {
  uint32_t LoadMapIndex;
  uint32_t CompressionAlgorithm;
  uint32_t CompressedLength;
  uint32_t CRC;
  uint32_t DataLength;
  uint8_t *Data;
  uint32_t DataOffset;
}IndirectDownloadDataStructType;

typedef struct {
  ElementHeaderStructType Header;
  IndirectDownloadDataStructType Data;
}IndirectDownloadDataElementStructType;

// --------------------------
// Hardware Structure
//---------------------------
//SG1,SGL,SG2
typedef struct {
  uint32_t ChipSelectNumber;
  uint32_t Addrsel;
  uint32_t Buscon;
  uint32_t Busap;
}ChipSelectStructType;
//SG3
typedef struct {
  uint32_t ChipSelectNumber;
  uint32_t BUSRCON;
  uint32_t BUSWCON;
  uint32_t BUSRP;
  uint32_t BUSWP;
}SG3ChipSelectStructType;
// all E-gold's
typedef struct
{
   uint16_t AreaStartAddress;  // High word, no area is less than 64Kbyte anyway
   uint16_t A21_Setup;      // Setup A21 for EGold ver 2 eller free extra GPIO for Ver 3
   uint16_t A22_Setup;      // Setup A22 as Addr or Gpio constant H or L
   uint16_t A23_Setup;      // Setup A23 as Addr or Gpio constant H or L
   uint16_t Ext_Gpio_Setup;    // Setup extra Gpio constant H or L
   uint16_t CS_Number;      // CS number to use in this area
   uint16_t CS_AddrSel;      // ADDRSEL for CS
   uint16_t CS_BusCon;      // BUSCON for CS
} AddrSetupStruct;    // Size is 4 dword.

//EGV_V2
typedef struct {
  uint16_t AreaStartAddress;  // I think we still need this
  uint16_t BUSRCONH;
  uint16_t BUSRCONL;
  uint16_t BUSWCONH;
  uint16_t BUSWCONL;
  uint16_t BUSRPH;
  uint16_t BUSRPL;
  uint16_t BUSWPH;
  uint16_t BUSWPL;
  uint16_t CS_AddrSel;
}EGV2SingleChipSelectStructType;  // Total 20 bytes

typedef struct {
  uint8_t ChipSelectNumber[4]; // contains the uint8_t for CS array
  EGV2SingleChipSelectStructType CS[3];
}EGV2ChipSelectStructType;  // Total 64 bytes

#ifndef FLSSIGN

typedef union {
  ChipSelectStructType     CS[4];
  SG3ChipSelectStructType  CS3[3];
  AddrSetupStruct          CE[4];
  EGV2ChipSelectStructType EGV;
}ChipSelectUnion;

typedef struct {
  uint32_t Platform;
  uint32_t SystemSize;
  uint32_t BootSpeed;
  uint32_t HandshakeMode;
  uint32_t UsbCapable;
  uint32_t FlashTechnology;

  ChipSelectUnion ChipSelect;   // LINT

  uint8_t ProjectName[20];
  uint8_t DepTemplate[32];
  uint8_t CfgTemplate[32];
}HardwareStructType;

typedef struct {
  ElementHeaderStructType Header;
  HardwareStructType Data;
}HardwareElementStructType;
#endif

// --------------------------
// NAND Partition Structure
//---------------------------
typedef struct
{
  uint32_t Table[MAX_PARTITIONS];
}LoadAddrToPartitionTableStruct;

typedef struct
{
  PartitionStructType Partition;
  LoadAddrToPartitionTableStruct LoadToPartition;
}NandPartitionDataStructType;

typedef struct {
  ElementHeaderStructType Header;
  NandPartitionDataStructType Data;
}NandPartitionElementStructType;


// --------------------------
// Secure Structure
//---------------------------
#define MAX_MODULUS_LEN (2048/8)
#define NUMBER_OF_SIGNATURES 2
#define SECURITY_BLOCK_SIZE 2048
#define SHA1_HASH_SIZE                                  (20)    // Size in bytes
#define SHA256_HASH_SIZE                                (32)    // Size in bytes
#define NUMBER_OF_LOAD_MAPS 8

// --------------------------
// Supported Compression Algorithms
//---------------------------
enum COMPRESS_TYPE
{
  COMPRESS_TYPE_NONE,
  COMPRESS_TYPE_LZ4,
};
typedef uint32_t T_COMPRESS_TYPE; // Make sure it is 32bit wide

typedef struct
{
  T_COMPRESS_TYPE CompressionAlgorithm;
  uint32_t UncompressedLength[NUMBER_OF_LOAD_MAPS];
}T_COMPRESSLOADMAP;

typedef struct {
  uint32_t StartAddr;
  uint32_t TotalLength;
  uint32_t UsedLength;
  uint32_t ImageFlags;
}RegionStructType;

typedef struct {
  uint32_t BootVersion;
  uint32_t InvBootVersion;
  uint32_t EblVersion;
  uint32_t InvEblVersion;
}ExtendedSecStructType;

typedef struct {
  uint32_t BootStartAddr;
  uint32_t BootLength;
  uint32_t BootSignature[5];
}OldStructType;

typedef union {
   OldStructType Old;
   ExtendedSecStructType Data;
}ExtendSecUnionType;

typedef struct {
  uint8_t SecureBlock[NUMBER_OF_SIGNATURES][MAX_MODULUS_LEN];
  uint32_t Type;
   /* Create a datablock, such that the total size of SecurityStructType is SECURITY_BLOCK_SIZE */
  uint8_t DataBlock[SECURITY_BLOCK_SIZE-
               NUMBER_OF_SIGNATURES*MAX_MODULUS_LEN-            /* Sizeof SecureBlock */
               4*sizeof(uint32_t)-                              /* Type,BootCoreVersion,EblVersion,LoadMagic */
               sizeof(ExtendSecUnionType)-                      /* ExtendSecUnionType */
               SHA1_HASH_SIZE-                                  /* FileHash when using legacy SHA1 */
               SHA256_HASH_SIZE-                                /* FileHash when using SHA2 */
               sizeof(PartitionStructType)-                     /* PartitionStructType */
               sizeof(uint32_t)*MAX_PARTITIONS-                 /* LoadAddrToPartition */
               sizeof(RegionStructType)*NUMBER_OF_LOAD_MAPS -   /* LoadMap */
               sizeof(T_COMPRESSLOADMAP)                        /* Compressed LoadMap */
               ];
   T_COMPRESSLOADMAP CompressLoadmap;
   uint8_t FileHashSha2[SHA256_HASH_SIZE];
   //***********************
   uint32_t BootCoreVersion;
   uint32_t EblVersion;
   ExtendSecUnionType SecExtend;
   //***********************
   uint32_t FileHash[SHA1_HASH_SIZE/4];
   PartitionStructType Partition;
   uint32_t LoadAddrToPartition[MAX_PARTITIONS];
   uint32_t LoadMagic;
   RegionStructType LoadMap[NUMBER_OF_LOAD_MAPS];
}SecurityStructType;
// end of new structure

typedef struct {
  ElementHeaderStructType Header;
  SecurityStructType Data;
}SecurityElementStructType;
// --------------------------
// Memory map Structure
//---------------------------
#define MAX_MAP_ENTRIES 50

typedef struct {
  uint32_t Start;
  uint32_t Length;
  uint32_t Class;
  uint32_t Flags;
  uint8_t FormatSpecify[16];
}MemoryMapEntryStructType;

typedef struct {
  MemoryMapEntryStructType Entry[MAX_MAP_ENTRIES];
  uint32_t EepRevAddrInSw;
  uint32_t EepRevAddrInEep;
  uint32_t EepStartModeAddr;
  uint32_t NormalModeValue;
  uint32_t TestModeValue;
  uint32_t SwVersionStringLocation;
  uint32_t FlashStartAddr;
}MemoryMapStructType;

typedef struct {
  ElementHeaderStructType Header;
  MemoryMapStructType Data;
}MemoryMapElementStructType;

// --------------------------
// TOC Structure
//---------------------------
typedef struct {
  uint32_t UID;
  uint32_t MemoryClass;
  uint32_t Reserved[2];
  uint8_t  FileName[MAX_FILENAME_LENGTH];
}TocEntryStructType;

typedef struct {
  uint32_t NoOfEntries;
  TocEntryStructType* Data;
  uint32_t DataOffset;
} TocStructType;

typedef struct {
  ElementHeaderStructType Header;
  TocStructType Data;
} TocElementStructType;

#endif
