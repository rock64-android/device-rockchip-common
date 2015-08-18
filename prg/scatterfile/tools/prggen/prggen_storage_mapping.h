// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/prggen_storage_mapping.h $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

/* dummy mapping to ease filling MMAP_UPDATE_PARTITION_INFO */
#define PARTITION_IT_BOOTUP_SLB_IMAGE  PARTITION_IT_BOOTUP_IMAGE
#define PARTITION_ID_EBL          (0xFE)
#define PARTITION_ID_PSI_RAM      (0xFD)
/* local mapping to make MMAP_UPDATE_PARTITION_INFO common for both NAND and EMMC */
#define FTLE_IT_BOOTUP_IMAGE      PARTITION_PP_BOOT_PARTITION_1
#define FTLE_IT_BOOTUP_SLB_IMAGE  PARTITION_PP_BOOT_PARTITION_2
#define FTLE_IT_BBR_IMAGE         PARTITION_PP_USER_PARTITION
#define FTLE_IT_FAT_IMAGE         PARTITION_PP_USER_PARTITION
#define FTLE_IT_FTL_IMAGE         PARTITION_PP_USER_PARTITION
#define FTLE_IT_RAW_IMAGE         PARTITION_PP_USER_PARTITION
#define FTLE_IT_SDS_IMAGE         PARTITION_PP_USER_PARTITION

#if defined(FEAT_FTLE_PRGGEN)
  #define BACKUP_DATA_DEST             FS_DEVICE_PLP_RUN_CHECK_DISK
  #define BACKUP_DATA_REQUIRED         FS_DEVICE_PLP_DISABLE
#endif

typedef struct
{
  union
  {
    U32                   attribute;
    struct
    {
      U32 GPT_PARTITION               :1;
      U32 GPT_SYSTEM_PARTITION        :1;
      U32 GPT_HIDE_EFI                :1;
      U32 GPT_LEGACY_BIOS_BOOTABLE    :1;
      U32 GPT_READ_ONLY               :1;
      U32 GPT_HIDDEN                  :1;
      U32 GPT_DO_NOT_AUTOMOUNT        :1;
      U32 Reserved0                   :1;
      U32 GPT_TYPE_GUID_a             :1;
      U32 GPT_TYPE_GUID_b             :1;
      U32 GPT_TYPE_GUID_c             :1;
      U32 Reserved1                   :5;
      U32 POWER_ON_WP_NORMAL_MODE     :1;
      U32 POWER_ON_WP_UPDATE_MODE     :1;
      U32 POWER_ON_WP_PRODUCTION_MODE :1;
      U32 POWER_ON_WP_SPARE_MODE      :1;
    }                     s_attribute;
  }                       u_attribute;

  U32                     image_type;
  U32                     image_class;
  
  union
  {
    U32                   flags;
  }                       u_flags;

  CHAR                    tag[16];
  CHAR                    fls_tag[16];
  CHAR                    output[16];

  union
  {
    U32                   options;
    struct
    {
      U32 VERIFICATION_REQUIRED     :1;
      U32 READONLYIMAGE             :1;
      U32 COPYTORAM                 :1;
      U32 WIPE_OUT_DATA             :1;
      U32 LOADMAP                   :1;
      U32 SHORT_TIMEOUT             :1;
      U32 PAGING_DATA               :1;
      U32 RO_BIT_CORRECTION_OK      :1;
      U32 ACCEPTABLE_BIT_ERRORS     :3;
      U32 TEST_ACCEPTED             :1;
      U32 ALLOW_UTA_BLOCK_IF_ACCESS :1;
      U32 ALLOW_UTA_BYTE_IF_ACCESS  :1;
      U32 SECTOR_BASED_PT           :1;
      U32 PRESERVE_PARTITION        :1;
      U32 FS_DEVICE_ACCESS_INTERNAL :1;
      U32 FS_DEVICE_ACCESS_EXTERNAL :1;
      U32 FS_DEVICE_PLP_DISABLE     :1;
      U32 FS_DEVICE_PLP_RUN_CHECK_DISK :1;
      U32 FS_DEVICE_PLP_ENABLE      :1;
      U32 FS_DEVICE_SYSTEM_DISK     :1;
      U32 FS_DEVICE_NOT_SYSTEM_DISK :1;
      U32 FS_PARTITION_REGISTER     :1;
      U32 Reserved1                 :2;
      U32 FS_DEVICE_ALLOCATION_SCATTER     :1;
      U32 FS_DEVICE_ALLOCATION_NORMAL     :1;      
      U32 ALIGNMENT                 :4;
    }                     s_options;
  } u_options;
  
  CHAR h2f[1024];
} prggen_partition_description_t;

typedef struct
{
  char                          *id_string;
  prggen_partition_description_t d; /* description */
  prggen_partition_description_t upd; /* updated */
  U32 length; /* Length of partition = Length form memory map + 1 block length for FTuL image type + 2K for Loadmap(if set) + 2K for Alignment(if set) */
} prggen_partition_info_t;

#define MMAP_UPDATE_PARTITION_INFO_ATTRIBUTE(ID, ATTR, VALUE) \
  if (g_partition_info[PARTITION_ID_##ID].upd.u_attribute.s_attribute.ATTR != TRUE) { \
    g_partition_info[PARTITION_ID_##ID].d.u_attribute.s_attribute.ATTR = VALUE; \
    g_partition_info[PARTITION_ID_##ID].upd.u_attribute.s_attribute.ATTR = TRUE; }
  
#define MMAP_UPDATE_PARTITION_INFO_ATTRIBUTE_COPY(DST_ID, SRC_ID) \
  if (g_partition_info[PARTITION_ID_##DST_ID].upd.u_attribute.attribute == 0) { \
    g_partition_info[PARTITION_ID_##DST_ID].d.u_attribute.attribute = g_partition_info[PARTITION_ID_##SRC_ID].d.u_attribute.attribute; \
    g_partition_info[PARTITION_ID_##DST_ID].upd.u_attribute.s_attribute = g_partition_info[PARTITION_ID_##SRC_ID].upd.u_attribute.s_attribute; \
    g_partition_info[PARTITION_ID_##DST_ID].upd.u_attribute.attribute = TRUE; }

#if defined(FEAT_FTLE_PRGGEN)
#define MMAP_UPDATE_PARTITION_INFO_IT(ID, IT) \
  g_partition_info[PARTITION_ID_##ID].id_string = (char *)#ID; \
  printf("Sanity test %s\n", #ID);                   \
  if (g_partition_info[PARTITION_ID_##ID].upd.image_type != TRUE) { \
    g_partition_info[PARTITION_ID_##ID].d.image_type = FTLE_IT_##IT; \
    g_partition_info[PARTITION_ID_##ID].upd.image_type = TRUE; }
#elif defined(FEAT_NAND_PRGGEN)
#define MMAP_UPDATE_PARTITION_INFO_IT(ID, IT) \
  g_partition_info[PARTITION_ID_##ID].id_string = (char *)#ID; \
  printf("Sanity test %s\n", #ID);                   \
  if (g_partition_info[PARTITION_ID_##ID].upd.image_type != TRUE) { \
    g_partition_info[PARTITION_ID_##ID].d.image_type = PARTITION_IT_##IT; \
    g_partition_info[PARTITION_ID_##ID].upd.image_type = TRUE; }
#endif

#define MMAP_UPDATE_PARTITION_INFO_OPT(ID, OPT, VALUE)  \
  if (g_partition_info[PARTITION_ID_##ID].upd.u_options.s_options.OPT != TRUE) { \
    g_partition_info[PARTITION_ID_##ID].d.u_options.s_options.OPT = VALUE; \
    g_partition_info[PARTITION_ID_##ID].upd.u_options.s_options.OPT = TRUE; }

#define MMAP_UPDATE_PARTITION_INFO_OPT_COPY(DST_ID, SRC_ID)  \
  if (g_partition_info[PARTITION_ID_##DST_ID].upd.u_options.options == 0) { \
    DEBUG_PRG(printf("Info: OPT_COPY for %s[%X]<-%s[%X] taken from %s\n", #DST_ID, PARTITION_ID_##DST_ID, #SRC_ID, PARTITION_ID_##SRC_ID, __FILE__);) \
    g_partition_info[PARTITION_ID_##DST_ID].d.u_options.options = g_partition_info[PARTITION_ID_##SRC_ID].d.u_options.options; \
    g_partition_info[PARTITION_ID_##DST_ID].upd.u_options.s_options = g_partition_info[PARTITION_ID_##SRC_ID].upd.u_options.s_options; \
    g_partition_info[PARTITION_ID_##DST_ID].upd.u_options.options = TRUE;  } \
  else { \
    DEBUG_PRG(printf("Info: OPT_COPY for %s[%X]<-%s[%X] not taken from %s\n", #DST_ID, PARTITION_ID_##DST_ID, #SRC_ID, PARTITION_ID_##SRC_ID, __FILE__);) \
  }

#define PERFORM_SANITY_OPT(OPT, VALUE, EXP_VALUE)  \
  g_partition_info[PARTITION_ID_EMPTY].d.u_options.options = 0;  \
  MMAP_UPDATE_PARTITION_INFO_OPT(EMPTY, OPT, VALUE); \
  printf("MMAP_UPDATE_PARTITION_INFO_OPT(%s) Act[%08X]\n", #OPT, (unsigned int)g_partition_info[PARTITION_ID_EMPTY].d.u_options.options); \
  if (g_partition_info[PARTITION_ID_EMPTY].d.u_options.options != (U32)EXP_VALUE) {  \
    printf("Incorrect MMAP_UPDATE_PARTITION_INFO_OPT(%s). Exp[%08X]Act[%08X]\n",  \
      #OPT, (U32)EXP_VALUE, (unsigned int)g_partition_info[PARTITION_ID_EMPTY].d.u_options.options); \
    exit (3); }

#define PERFORM_SANITY_ATTRIBUTE(OPT, VALUE, EXP_VALUE)  \
  g_partition_info[PARTITION_ID_EMPTY].d.u_attribute.attribute = 0;  \
  MMAP_UPDATE_PARTITION_INFO_ATTRIBUTE(EMPTY, OPT, VALUE); \
  printf("MMAP_UPDATE_PARTITION_INFO_ATTRIBUTE(%s) Act[%08X]\n", #OPT, g_partition_info[PARTITION_ID_EMPTY].d.u_attribute.attribute); \
  if (g_partition_info[PARTITION_ID_EMPTY].d.u_attribute.attribute != EXP_VALUE) {  \
    printf("Incorrect MMAP_UPDATE_PARTITION_INFO_ATTRIBUTE(%s). Exp[%08X]Act[%08X]\n",  \
      #OPT, EXP_VALUE, g_partition_info[PARTITION_ID_EMPTY].d.u_attribute.attribute); \
    exit (3); }

#define MMAP_UPDATE_MEMORYMAP_INFO_TAG(ID, TAG) \
  if (g_partition_info[PARTITION_ID_##ID].upd.tag[0] == 0) { \
    strncpy(g_partition_info[PARTITION_ID_##ID].d.tag, TAG, sizeof(g_partition_info[PARTITION_ID_##ID].d.tag) - 1); \
	strncpy(g_partition_info[PARTITION_ID_##ID].d.fls_tag, TAG, sizeof(g_partition_info[PARTITION_ID_##ID].d.tag) - 1); \
	strtok(g_partition_info[PARTITION_ID_##ID].d.fls_tag, ":");  \
    string_lower(g_partition_info[PARTITION_ID_##ID].d.output, g_partition_info[PARTITION_ID_##ID].d.tag); \
    DEBUG_PRG(printf("Info: T[%s] (%s) for %s[%X] taken from %s\n", TAG, g_partition_info[PARTITION_ID_##ID].d.tag, #ID, PARTITION_ID_##ID, __FILE__);) \
    g_partition_info[PARTITION_ID_##ID].upd.tag[0] = 1;  } \
  else { \
    DEBUG_PRG(printf("Info: T[%s] (%s) for %s[%X] not taken from %s\n", TAG, g_partition_info[PARTITION_ID_##ID].d.tag, #ID, PARTITION_ID_##ID, __FILE__);) \
  }

#define MMAP_UPDATE_MEMORYMAP_INFO_TAG_COPY(DST_ID, SRC_ID) \
  if (g_partition_info[PARTITION_ID_##ID].upd.tag[0] == 0) { \
    strcpy(g_partition_info[PARTITION_ID_##DST_ID].d.tag, g_partition_info[PARTITION_ID_##SRC_ID].d.tag); \
    strcpy(g_partition_info[PARTITION_ID_##DST_ID].d.output, g_partition_info[PARTITION_ID_##SRC_ID].d.output); \
    g_partition_info[PARTITION_ID_##ID].upd.tag[0] = 1;  }

#define MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(ID, FMT, ...)  \
  if (g_partition_info[PARTITION_ID_##ID].upd.h2f[0] == 0) { \
    snprintf(g_partition_info[PARTITION_ID_##ID].d.h2f,sizeof(g_partition_info[PARTITION_ID_##ID].d.h2f), FMT, ## __VA_ARGS__); \
    g_partition_info[PARTITION_ID_##ID].upd.h2f[0] = 1;  }

#define MMAP_UPDATE_MEMORYMAP_INFO_CLASS(ID, CLASS) \
  if (g_partition_info[PARTITION_ID_##ID].upd.image_class != TRUE) { \
    g_partition_info[PARTITION_ID_##ID].d.image_class = CLASS; \
    g_partition_info[PARTITION_ID_##ID].upd.image_class = TRUE; }
