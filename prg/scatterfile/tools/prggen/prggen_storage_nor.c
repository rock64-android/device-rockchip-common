// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/prggen_storage_nor.c $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

// prggen_storage_nor.c

/****************************************************************
 * prggen_fixup_sizes
 *  This function shall fix-up the sizes, could be additional
 * demand due to specific option, or specific rules
 *
 * Rules:
 * 1. LOADMAP needs space for SecurityStructType (aligned to 2K).
 ****************************************************************/
void prggen_ftl_fixup_sizes(void)
{
  U32 i;

  /* fix-up the sizes for each memory map entry */
  for (i = 0; i < MAX_MAP_ENTRIES; ++i)
  {
    /* no more valid entries, abort */
    if (g_p_mmap[i].Class == NOT_USED)
      break;
    /* no partition associated with this entry, skip */
    if (g_image_type[i] == PARTITION_ID_EMPTY)
      continue;

    /* LOADMAP needs extra space */
    if (get_opt_type(g_image_type[i]) & PARTITION_OPT_LOADMAP)
    {
      printf("LOADMAP, adding %d bytes to ID[%02X]\n", ALIGN_2K(sizeof (SecurityStructType)), g_image_type[i]);
      g_p_mmap[i].Length += ALIGN_2K(sizeof (SecurityStructType));
    }
  }
}

/****************************************************************
 * prggen_ftl_partition_init
 *  This function shall initialize the details needed for updating
 * the partition information, such as regions, etc.
 *
 * Rules:
 ****************************************************************/
void prggen_ftl_partition_init(void)
{
}

/****************************************************************
 * prggen_ftl_partition_update
 *  This function shall initialize the details needed for updating
 * the partition information, such as regions, etc.
 *
 * Rules:
 ****************************************************************/
void prggen_ftl_partition_update(PartitionEntryStructType *p_part_entry, U32 blk_size)
{
}

/****************************************************************
 * prggen_ftl_sanity
 *  This function shall perform the sanity tests on the assumptions it
 * has on memory-map, partition information, such as attributes, options, etc.
 *
 ****************************************************************/
void prggen_ftl_sanity(void)
{
}
