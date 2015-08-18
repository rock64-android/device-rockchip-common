// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/prggen_mmap.h $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

#define MMAP_ADD_MEMORYMAP_OPT(ID, _BASE_, _SIZE_)  \
      *p_image_type = PARTITION_ID_##ID; \
      p_image_type++;

#define MMAP_ADD_MEMORYMAP(ID, _BASE_, _SIZE_) \
      DEBUG_PRG(printf("Adding %s @ %X T[%s] from %s\n", #ID,(unsigned int)p_mmap, g_partition_info[PARTITION_ID_##ID].d.tag, __FILE__)); \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = g_partition_info[PARTITION_ID_##ID].d.image_class;  \
      p_mmap->Flags  = g_partition_info[PARTITION_ID_##ID].d.u_flags.flags;  \
      strcpy((char *)&p_mmap->FormatSpecify[0], g_partition_info[PARTITION_ID_##ID].d.tag);  \
      p_mmap++;

#define MMAP_ADD_PARTITION_EX(NAND_ID, BASE, SIZE, IMAGE_TYPE, OPTIONS, RESERVED, TAG, CLASS, FLAGS)	\
      p_mmap->Class = CLASS;      \
      p_mmap->Start = BASE;       \
      p_mmap->Length = SIZE;      \
      p_mmap->Flags = FLAGS;      \
      strncpy((char *)&p_mmap->FormatSpecify[0], #TAG,strlen(#TAG)>15?15:strlen(#TAG)); \
      p_mmap++;                   \
      *p_image_type = NAND_ID;    \
      p_image_type++;             \
      p_partition_control_advanced->level = 1;  \
      p_partition_control_advanced->ImageType = IMAGE_TYPE; \
      p_partition_control_advanced->Options = OPTIONS;      \
      p_partition_control_advanced->Reserved = RESERVED;    \
      p_partition_control_advanced++;

  /* We do not fill in the format specifier here as these entries in the Memory-map is later removed for these */
#define MMAP_ADD_PARTITION(NAND_ID, base, size) \
      p_mmap->Class = CUST_CLASS; \
      p_mmap->Start = base;  \
      p_mmap->Length = (U32)size;  \
      p_mmap++; \
      p_partition_control_advanced++; \
      *p_image_type = PARTITION_ID_##NAND_ID;  \
      p_image_type++;
      
#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)

#define MMAP_ADD_BINARY_SECTION_CUST_EXTEND(ID, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_, _EXTEND_) \
      MMAP_ADD_MEMORYMAP_ENTRY(ID, _BASE_, _SIZE_)

#define MMAP_ADD_DISC_SECTION_EMPTY(ID, _BASE_, _SIZE_) \
  MMAP_ADD_MEMORYMAP_ENTRY(ID, _BASE_, _SIZE_)

#else

#define MMAP_ADD_BINARY_SECTION_CUST_EXTEND(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_, _EXTEND_) \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = _PRG_CLASS_;  \
      p_mmap->Flags  = 0; \
      strcpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_); \
      p_mmap++; \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = CUST_CLASS;  \
      p_mmap->Flags  = 0; \
      sprintf((char *)&p_mmap->FormatSpecify[0], "%s_%s",#_PRG_NAME_,"BIN"); \
      p_mmap++; 

/* TODO: _BASE_ base has to be calculated automatically */
#define MMAP_ADD_DISC_SECTION_EMPTY(ID, _BASE_, _SIZE_) \
  MMAP_ADD_BINARY_SECTION_EMPTY(ID, DUMMY, ID, _BASE_, _SIZE_, 0, 0)

#endif

#define MMAP_ADD_LOAD_REGION(ID, _CLASS_, _REGION_, _BASE_, _SIZE_)  \
      MMAP_ADD_MEMORYMAP_ENTRY(ID, _BASE_, _SIZE_)

#define MMAP_ADD_BINARY_SECTION_EMPTY(ID, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_)  \
      MMAP_ADD_MEMORYMAP_ENTRY(ID, _BASE_, _SIZE_)

#define MMAP_ADD_EXEC_SECTION(_SECTION_, _BASE_, _SIZE_)

#define MMAP_ADD_EXEC_SECTION_EXTEND(_SECTION_, _SCF_ATTR_, _BASE_, _SIZE_)

#define MMAP_ADD_EXEC_SECTION_BASE(_SECTION_, _BASE_)
  
#define MMAP_ADD_EXEC_SECTION_BASE_EXTEND(_SECTION_, _BASE_, _SCF_ATTR_)
  
#define MMAP_ADD_EXEC_SECTION_ZERO_OFFSET(_SECTION_)
  
#define MMAP_ADD_EXEC_SECTION_ZERO_OFFSET_EXTEND(_SECTION_, _SCF_ATTR_)

#define MMAP_ADD_EXEC_SECTION_ALIGN_OFFSET(_SECTION_)

#define MMAP_ADD_EXEC_SECTION_ALIGN_OFFSET_EXTEND(_SECTION_, _SCF_ATTR_)

#define MMAP_ADD_BINARY_SECTION(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_)  \
  MMAP_ADD_BINARY_SECTION_EMPTY(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_)

#define MMAP_ADD_BINARY_SECTION_EXTEND(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_, _EXTEND_) \
  MMAP_ADD_BINARY_SECTION_EMPTY(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_)

#define MMAP_ADD_BINARY_SECTION_EMPTY_ALIGN_NAME(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_)  \
  MMAP_ADD_BINARY_SECTION_EMPTY(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_)

#define MMAP_ADD_LOAD_REGION_EXTEND(_PRG_NAME_, _CLASS_, _REGION_, _SCF_ATTR_, _BASE_, _SIZE_)  \
  MMAP_ADD_LOAD_REGION(_PRG_NAME_, _CLASS_, _REGION_, _BASE_, _SIZE_)
