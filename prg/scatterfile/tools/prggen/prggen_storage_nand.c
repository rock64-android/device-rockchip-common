// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/prggen_storage_nand.c $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

// prggen_storage_nand.c

static U32 get_part_type(U32 id)
{
  return g_partition_info[id].d.image_type;
}

/****************************************************************
 * prggen_ftl_partition_init
 *  This function shall initialize the details needed for updating
 * the partition information, such as regions, etc.
 *
 * Rules:
 ****************************************************************/
void prggen_ftl_partition_init(PartitionStructType *p_partition)
{
  U32 i, r, p;
  U32 partition_length;

  /* Calculate the region size information */

  /* for each memory map entry */
  for (i = 0; i < MAX_MAP_ENTRIES; ++i)
  {
    /* no more valid entries, abort */
    if (g_p_mmap[i].Class == NOT_USED)
      break;
    /* no partition associated with this entry, skip */
    if (g_image_type[i] == PARTITION_ID_EMPTY ||
        g_image_type[i] == PARTITION_ID_PSI_RAM ||
        g_image_type[i] == PARTITION_ID_EBL)
    {
      continue;
    }

    printf("IMAGE TYPE : ID[%02X]\n", g_image_type[i]);
    g_partition_id_to_region_map[g_image_type[i]] = 0xFF;
    /* find which region contains this partition */
    for (r = 0; r < sizeof(region)/sizeof(T_REGION); ++r)
    {
      for (p = 0; p < MAX_PARTITION_PER_REGION; ++p)
      {
        partition_length = 0;
        if (g_image_type[i] == region[r].nand_id[p])
        {
          /* include this image's length to this partition */
          partition_length = g_p_mmap[i].Length;
		  
          /* LOADMAP needs extra space */
          if ((g_partition_info[g_image_type[i]].d.u_options.options) & PARTITION_OPT_LOADMAP)
          {
            printf("LOADMAP, adding %lu bytes to ID[%02X]\n", ALIGN_2K(sizeof (SecurityStructType)), g_image_type[i]);
            partition_length += ALIGN_2K(sizeof (SecurityStructType));
          }

          /* ALIGNMENT needs extra space */
          if ((g_partition_info[g_image_type[i]].d.u_options.options) & (NAND_ALIGNMENT_MASK))
          {
            printf("ALIGNMENT, adding %d bytes to ID[%02X]\n", ALIGN_2K(1), g_image_type[i]);
            partition_length += ALIGN_2K(1);
          }

          /* any special handling needed? */
          switch (get_part_type(region[r].nand_id[p]))
          {
            case NAND_IT_FAT_IMAGE:
            case NAND_IT_FTL_IMAGE:
            case NAND_IT_SDS_IMAGE:
              partition_length += MMAP_FLASH_BLOCK_SIZE;  /* +1 block for FAT/FTL/SDS image types */
              break;
            default:
              break;
          }

          /* Adding Maximum size required for each partition to region */
          region[r].Size += NO_OF_BLOCKS(partition_length) * MMAP_FLASH_BLOCK_SIZE; 
          /* make one reverse lookup; id to region */
          g_partition_id_to_region_map[g_image_type[i]] = r;
          break;
        }

        if (region[r].nand_id[p+1] == 0)
        {
          /* no more search needed in this region*/
          break;
        }
      } /* for p */

      /* check if we already found a partition for this id? */
      if (g_partition_id_to_region_map[g_image_type[i]] == r)
      {
        /* yes, move to next memory map entry */
        break;
      }
    } /* for r */
    
    /* not found yet? */
    if (g_partition_id_to_region_map[g_image_type[i]] == 0xFF)
    {
      printf("Error:Partition %02X not found in region configuration.\n", g_image_type[i]);
      exit(3);
    }
  } /* for i */
  
  printf("NAND Regions:\n");
  /* Calculate the blocks needed for a paritition */
  for (r = 0; r < sizeof (region)/ sizeof (T_REGION); ++r)
  {
    region[r].StartBlock = (r == 0) ? (0) : (region[r-1].NextFreeBlock);
    region[r].EndBlock = region[r].StartBlock + NO_OF_BLOCKS(region[r].Size) + region[r].region_extra_blk;
    if ((region[r].reservoir_p != 0) || (region[r].reservoir_c != 0))
    {
      /* For Bootup reservoir sizes are added to actuals only */
      if (get_part_type(region[r].nand_id[0]) == PARTITION_IT_BOOTUP_IMAGE)
      {
        /* just ensure all partitions in this region are having same image types */
        for (p = 1; p < MAX_PARTITION_PER_REGION; ++p)
        {
          if (get_part_type(region[r].nand_id[0]) != get_part_type(region[r].nand_id[p]))
          {
            printf("Error: Incorrect region grouping for BOOTUP_IMAGE\n");
            exit (3);
          }
          if (region[r].nand_id[p+1] == 0)
            break;
        }

        if (region[r].reservoir_p == 0)
        {
          region[r].EndBlock += region[r].reservoir_c;
        }
        else if (region[r].reservoir_c == 0)
        {
          region[r].EndBlock += RESERVE_BLOCKS(region[r].Size, region[r].reservoir_p);
        }
        region[r].StartReservoir = 0;
        region[r].EndReservoir = 0;
        region[r].NextFreeBlock = region[r].EndBlock;
      }
      else
      {
        region[r].StartReservoir = region[r].EndBlock;
        if (region[r].reservoir_p == 0)
        {
          region[r].EndReservoir = region[r].StartReservoir + region[r].reservoir_c;
        }
        else if (region[r].reservoir_c == 0)
        {
          /* change reservoir calculation code to apply Formula */
          region[r].EndReservoir = region[r].StartReservoir + 2 + RESERVE_P_BLOCKS(region[r].Size, region[r].reservoir_p) +  region[r].reservoir_extra_blk;
        }
        region[r].NextFreeBlock = region[r].EndReservoir;
      }
    }
    else
    {
      region[r].StartReservoir = 0;
      region[r].EndReservoir = 0;
      region[r].NextFreeBlock = region[r].EndBlock;
    }
    
    printf("R[%02d] Size[%08X] S[%04X] E[%04X] SR[%04X] ER[%04X] R[%2d%% (=%2d b) | %2d b] ID{ ",
      r, region[r].Size, region[r].StartBlock, region[r].EndBlock, region[r].StartReservoir, region[r].EndReservoir,
      region[r].reservoir_c == 0 ? region[r].reservoir_p : 0,
      region[r].reservoir_c == 0 ? RESERVE_BLOCKS(region[r].Size, region[r].reservoir_p) : 0,
      region[r].reservoir_p == 0 ? region[r].reservoir_c : 0);
    for (p = 0; p < MAX_PARTITION_PER_REGION; ++p)
    {
      printf("%02X ", region[r].nand_id[p]);
      if (region[r].nand_id[p+1] == 0)
        break;
    }
    printf("}\n");

    if (region[r].NextFreeBlock > MAX_NUM_BLOCKS)
    {
      printf("Error: Insufficient space in storage nfb=0x%X mb=0x%X\n", region[r].NextFreeBlock, MAX_NUM_BLOCKS);
      exit(2);
    }
  }

  // set start block and end blockn for PREFLASH partition to zero
  for (r = 0; r < sizeof (region)/ sizeof (T_REGION); ++r)
  {
    for (p = 0; p < MAX_PARTITION_PER_REGION; ++p)
    {
      if (region[r].nand_id[p] == PARTITION_ID_PREFLASH)
      {
            region[r].StartBlock = 0;        
            region[r].EndBlock = 0;
      }
    }
  }  
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

  /* LOADMAP needs extra space */
  if ((g_partition_info[p_part_entry->ID].d.u_options.options) & PARTITION_OPT_LOADMAP)
  {
    DEBUG_PRG(printf("LOADMAP, adding %d bytes to ID[%02X]\n", ALIGN_2K(sizeof (SecurityStructType)), p_part_entry->ID));    
    blk_size += ALIGN_2K(sizeof (SecurityStructType));
  }

  /* ALIGNMENT needs extra space */
  if ((g_partition_info[p_part_entry->ID].d.u_options.options) & (NAND_ALIGNMENT_MASK))
  {
    DEBUG_PRG(printf("ALIGNMENT, adding %d bytes to ID[%02X]\n", ALIGN_2K(1), p_part_entry->ID));
    blk_size += ALIGN_2K(1);
  }

  /* any special handling needed? */
  switch (get_part_type(p_part_entry->ID))
  {
    case NAND_IT_FAT_IMAGE:
    case NAND_IT_FTL_IMAGE:
    case NAND_IT_SDS_IMAGE:
      blk_size += MMAP_FLASH_BLOCK_SIZE;  /* +1 block for FAT/FTL/RAW image types */
      break;
    default:
      break;
  }

  p_part_entry->StartBlock = region[g_partition_id_to_region_map[p_part_entry->ID]].StartBlock;
  p_part_entry->EndBlock = region[g_partition_id_to_region_map[p_part_entry->ID]].EndBlock;
  p_part_entry->StartReservoirBlock = region[g_partition_id_to_region_map[p_part_entry->ID]].StartReservoir;
  p_part_entry->EndReservoirBlock = region[g_partition_id_to_region_map[p_part_entry->ID]].EndReservoir;
  p_part_entry->MaxBlocksInSubPartition = NO_OF_BLOCKS(blk_size);

  if (p_part_entry->ID == PARTITION_ID_PSI_IMAGE &&
      p_part_entry->StartBlock != 0)
  {
    printf("Warning: PSI doesn't start at block-0\n");
    printf("         Check region configuration.\n");
  }
}

/****************************************************************
 * prggen_ftl_sanity
 *  This function shall perform the sanity tests on the assumptions it
 * has on memory-map, partition information, such as attributes, options, etc.
 *
 ****************************************************************/
void prggen_ftl_sanity(void)
{
  PERFORM_SANITY_OPT(VERIFICATION_REQUIRED, TRUE, PARTITION_OPT_VERIFICATION_REQUIRED);
  PERFORM_SANITY_OPT(READONLYIMAGE, TRUE, PARTITION_OPT_READONLYIMAGE);
  PERFORM_SANITY_OPT(COPYTORAM, TRUE, PARTITION_OPT_COPYTORAM);
  PERFORM_SANITY_OPT(WIPE_OUT_DATA, TRUE, PARTITION_OPT_WIPE_OUT_DATA);
  PERFORM_SANITY_OPT(LOADMAP, TRUE, PARTITION_OPT_LOADMAP);
  PERFORM_SANITY_OPT(SHORT_TIMEOUT, TRUE, PARTITION_OPT_SHORT_TIMEOUT);
  PERFORM_SANITY_OPT(PAGING_DATA, TRUE, PARTITION_OPT_PAGING_DATA);
  PERFORM_SANITY_OPT(RO_BIT_CORRECTION_OK, TRUE, PARTITION_OPT_RO_BIT_CORRECTION_OK);
  PERFORM_SANITY_OPT(ACCEPTABLE_BIT_ERRORS, 0x7, PARTITION_OPT_ACCEPTABLE_BIT_ERRORS);
  PERFORM_SANITY_OPT(TEST_ACCEPTED, TRUE, PARTITION_OPT_TEST_ACCEPTED);
  PERFORM_SANITY_OPT(ALLOW_UTA_BLOCK_IF_ACCESS, TRUE, PARTITION_OPT_ALLOW_UTA_BLOCK_IF_ACCESS);
  PERFORM_SANITY_OPT(ALLOW_UTA_BYTE_IF_ACCESS, TRUE, PARTITION_OPT_ALLOW_UTA_BYTE_IF_ACCESS);
  PERFORM_SANITY_OPT(SECTOR_BASED_PT, TRUE, PARTITION_OPT_SECTOR_BASED_PT);
  PERFORM_SANITY_OPT(FS_DEVICE_ACCESS_INTERNAL, TRUE, PARTITION_OPT_FS_DEVICE_ACCESS_INTERNAL);
  PERFORM_SANITY_OPT(FS_DEVICE_ACCESS_EXTERNAL, TRUE, PARTITION_OPT_FS_DEVICE_ACCESS_EXTERNAL);
  PERFORM_SANITY_OPT(FS_DEVICE_PLP_DISABLE, TRUE, PARTITION_OPT_FS_DEVICE_PLP_DISABLE);
  PERFORM_SANITY_OPT(FS_DEVICE_PLP_RUN_CHECK_DISK, TRUE, PARTITION_OPT_FS_DEVICE_PLP_RUN_CHECK_DISK);
  PERFORM_SANITY_OPT(FS_DEVICE_PLP_ENABLE, TRUE, PARTITION_OPT_FS_DEVICE_PLP_ENABLE);
  PERFORM_SANITY_OPT(FS_DEVICE_SYSTEM_DISK, TRUE, PARTITION_OPT_FS_DEVICE_SYSTEM_DISK);
  PERFORM_SANITY_OPT(FS_DEVICE_NOT_SYSTEM_DISK, TRUE, PARTITION_OPT_FS_DEVICE_NOT_SYSTEM_DISK);
  PERFORM_SANITY_OPT(FS_PARTITION_REGISTER, TRUE, PARTITION_OPT_FS_PARTITION_REGISTER);
  PERFORM_SANITY_OPT(ALIGNMENT, 0xF, PARTITION_OPT_ALIGNMENT_MASK);
}
