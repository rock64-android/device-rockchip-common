/* ----------------------------------------------------------------------------

   Copyright (C) 2011-2014 Intel Mobile Communications GmbH

        Sec Class: Intel Confidential (IC)

   All rights reserved.
   ----------------------------------------------------------------------------

   This document contains proprietary information belonging to IMC.
   Technologies Denmark A/S. Passing on and copying of this document, use
   and communication of its contents is not permitted without prior written
   authorisation.

   Revision Information:
  $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/prggen.cpp $
  ---------------------------------------------------------------------------*/

#if !defined(PRGGEN_VERSION)
  #define PRGGEN_VERSION  1
  #include <v1/prggen.cpp>
#endif

#if PRGGEN_VERSION == 2

#if 1
  #define DEBUG_PRG(X)  X
#else
  #define DEBUG_PRG(X)
#endif

#include <stdio.h>
#include <stdlib.h>
#if defined(UNIX_BUILD)
#include <memory.h>
#endif

#include <string.h>
#include <partition_definition.h>
#if defined(FEAT_FTLE_PRGGEN)
  #define FTLE_PRESENT
  #include <ftle.h>
#if defined(FEAT_OPM_MEM_GPT_SUPPORT)
  #include <ftle_cfg.h>
#endif
#else
  #include <bastypes.h>
#endif

#include "MemoryStream.h"
#include "PrgHandler.h"
#include "PrgSequencer.h"

#include "header.h"
#include <prggen_rules_tags.h>

/**< Acceptable value in partition_marker position in pt. */
#if defined(FEAT_FTLE_PRGGEN)
#define PARTITION_MARKER   PARTITION_MARKER_FTLE
#elif defined(FEAT_NAND_PRGGEN)
#define PARTITION_MARKER   PARTITION_MARKER_NAND
#else
#define PARTITION_MARKER   0x0000
#endif

#define MAX_NUM_BLOCKS          (MMAP_MODEM_SW_FLASH_SIZE/MMAP_FLASH_BLOCK_SIZE)
#define NO_OF_BLOCKS(size)      ((size+MMAP_FLASH_BLOCK_SIZE-1)/MMAP_FLASH_BLOCK_SIZE)
#define RESERVE(p)              (p/100)
#define RESERVE_BLOCKS(size, p) ((NO_OF_BLOCKS((size * (RESERVE(p)))) > 1) ?  NO_OF_BLOCKS(size * (RESERVE(p))) : 1)
#define RESERVE_P_BLOCKS(size, p) (NO_OF_BLOCKS(RESERVE(size*p)))


#if defined(FEAT_FLASH_WRITE_PROTECT_ENABLED)
/* Write protection blocks has to be aligned by 8MB i.e. 0x4000/16384d */
  #define EMMC_WP_BLOCK_ALIGN_SIZE   16384
  #define ALIGN_BLOCK_FOR_WP(block)  (block + (EMMC_WP_BLOCK_ALIGN_SIZE - (block % EMMC_WP_BLOCK_ALIGN_SIZE)))
#endif

#if defined(FEAT_NAND_PRGGEN)
  #include <mem_region_cfg.h>
#endif

/* Temporary hack to generate common prg for both Modem and CDS */
#undef CDS

#define ALIGN_2K(ADDR) (((ADDR) + 0x7FF) & 0xFFFFF800)

#include <prggen_mmap.h>
#include <mem_static_internal.h>
#include <prggen_storage_mapping.h>

PRGSequencer CurrentFile;

MemoryMapEntryStructType *g_p_mmap;
U32 g_image_type[MAX_MAP_ENTRIES];
U32 *g_p_image_type = &g_image_type[0];
#if defined(FEAT_NAND_PRGGEN)
U32 g_partition_id_to_region_map[PARTITION_ID_EMPTY];
#endif
char *p_global_path;
char *p_input_path;

FILE *fp;
char *g_p_hex2fls_tool;
char *g_p_hex2fls_input_location;
char *g_p_sign_script;
char *g_p_scf_tools_obj;
char *p_mm_defs;
char *p_int_stage;

char g_boot_file_loc[256];
char g_scf_tools_obj[256];
char g_p_hex2fls_input_files[512];
char p_output_path[256];
char p_layout_path[256];
char g_pre_erase_file[256];
#if defined(INI_FILE)
char g_ini_file[256] = INI_FILE;
#endif

prggen_partition_info_t g_partition_info[PARTITION_ID_EMPTY];

struct partition_control_advanced_s
{
  U32 level;

  U32 ImageType;
  U32 Options;
  U32 Reserved;
} g_partition_control_advanced[MAX_MAP_ENTRIES] = {0};

#include <prggen_utils.c>

#if defined(FEAT_FTLE_PRGGEN)
  #include <prggen_storage_emmc.c>
#endif  /* FEAT_FTLE_PRGGEN */

#if defined(FEAT_NAND_PRGGEN)
  #include <prggen_storage_nand.c>
#endif  /* FEAT_NAND_PRGGEN */


/*****************************************************************************************************
 * PRG partitions relative to end of physical eMMC partitions implementation starts
 ****************************************************************************************************/
 /* structure of priority-based linked list*/
struct list {
  U32 ID;          
  U32 start_block;
  U32 end_block;
  U32 rev_start_block;                            /* Signed magnitude representation of start block number if any */
  U32 rev_end_block;                              /* Signed magnitude representation of end block number if any */
  PartitionEntryStructType    *p_partition_entry; /* corresponding partition id pointer */
  struct list *next;
};

typedef struct list *LIST;


struct pqueue 
{
  LIST front;                       /* this member holds address first node of priority-based linked list */
  U32 max_allocated_block_number;   /* contains the last block number */
  BOOL dynamic_done;                /* initially false; will be true  after PRG partitions relative to end of physical eMMC partitions is done */
};

struct pqueue queue;                              /* local variables, declarition of priority queue */
PartitionEntryStructType    *p_part_entry_front;  /* local variables, holds the address of first PartitionEntry, need only for debugging */
#define NEGATIVE_ZERO_IN_SIGNED_MAGNITUDE_FORMAT 0x80000000
/* BOOT_PARTITION */
#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
  #define FEAT_OPM_MEM_PARTITION_ID2               PARTITION_ID_PARTITION_TABLE
#endif
#define PARTITION_BLOCK_SIZE(end_block, start_block) \
  (end_block - start_block)

/***************************************************************************
 * priority queue based on endblock number.
 * End block number of a node will decrease if we traverse the queue from
 * front to last.
 **************************************************************************/
int pqinsert(struct pqueue *pq, PartitionEntryStructType *p_part_entry)
{
  LIST p, s, t = NULL;
  U32 StartBlock = p_part_entry->StartBlock;
  U32 EndBlock = p_part_entry->EndBlock;
  
  p = (LIST)malloc(sizeof(struct list));
  if (p == NULL)
  {
    printf("Memory allocation failed\n");
    exit(2);
  }
  p->ID = p_part_entry->ID;
  p->start_block = StartBlock;
  p->end_block = EndBlock;
  p->p_partition_entry = p_part_entry;

  /* initialy rev_start_block, rev_end_block will contain normal StartBlock, EndBlock numbers
   * these members will be updated in update_block_number(id, id2). id => Partition id which will 
   * have dynamic partition size.
   */
   
  p->rev_start_block = StartBlock;  
  p->rev_end_block = EndBlock;
 
  /* Update maximum (or last) block number */
  if (pq->max_allocated_block_number < EndBlock)
  {
    pq->max_allocated_block_number = EndBlock;
  }

  /* for first node of the queue */
  if ( pq->front == NULL)
  {
    pq->front = p;
    p->next = NULL;
  } 
  /* to keep maximum (or last) block number at the front */
  else if (((pq->front)->end_block < EndBlock))
  {
    p->next = pq->front;
    pq->front = p;

  }
  /* search appropriate position to keep the node, to maintain the oreder */
  else
  {
    for(s = pq->front; s != NULL && (s->end_block >= EndBlock); s = s->next)
    {
      t = s;
    }
    if(t != NULL)
    {
      p->next = t->next;
      t->next = p;
    }
  }
  return 0;
}

/*****************************************************************************
 * Initialize the priority queue
 *
 ****************************************************************************/
void pqinit(struct pqueue *pq)
{
  pq->front = NULL;
  pq->max_allocated_block_number = 0x0;
  pq->dynamic_done = FALSE;
}

/*******************************************************************************
 * Update the block number according to signed magnitude representation and PRG
 * partitions relative to end of physical eMMC partitions implementation iff
 * FEAT_BLK_NUM_REL_EMMC_END is defined.
 * id: end partition id, which will have dynamic partition size.
 * id2: partition id, which will have dynamic end block number but have fixed
 * number of blocks.
 * return type:
 * + 1 = Succesful.
 * - 1 = Not Supported.
 * Step 1: Update rev_start_block, rev_end_block of the priority queue and 
 * start_block, end_block numbers of global partition.
 * 
 * Step 2: Update BOOT_PARTITION partition block numbers
 ********************************************************************************/
S32 update_block_number(U32 id, U32 id2)
{
#if !defined (FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END)
  /* Updatation the PRG partitions relative to end of physical eMMC partitions 
   * not required, hence returns from here.
   */
  return -1;
#else


#if defined(FEAT_FLASH_WRITE_PROTECT_ENABLED)
  static int emmc_end_align_wp = 0;
#endif
  
  LIST p = (&queue)->front, p_prev_wp;

  U32 prev_start_block_number;

  if(p == NULL)
  {
	printf("Invalid Queue\n");
	return -1;
  }
    
  /* Step 1: starts */
  
  /* Max End block number will be -0x0 (signed magnitude representation 0x8000 0000) */
  p->rev_end_block = NEGATIVE_ZERO_IN_SIGNED_MAGNITUDE_FORMAT;
  p->p_partition_entry->EndBlock = p->rev_end_block;
	
  /* generally, if start block number is a, the end block number will be "a + size of the partition - 1" 
   * But in case of signed magnitude representaion for negative number, if end block number is b,
   * the start block number will be " 0x8000 0000 + size of the partition"
   */
  prev_start_block_number = NEGATIVE_ZERO_IN_SIGNED_MAGNITUDE_FORMAT | PARTITION_BLOCK_SIZE(p->end_block, p->start_block);
  
  while (p->ID != id)
  {
    p->rev_start_block = prev_start_block_number;
    p->p_partition_entry->StartBlock = p->rev_start_block;
	
    p_prev_wp = p;
    p = p->next;

    if ((p != NULL) && (p->ID == id))
    {
      (&queue)->dynamic_done = TRUE;
    }

    if((p != NULL))
    {
#if defined(FEAT_FLASH_WRITE_PROTECT_ENABLED)
      /* Align the write protection blocks at the end of FLASH by 8MB(0x4000) 
      * i.e. Align SECONDARY_GPT if FEAT_OPM_MEM_GPT_SUPPORT is defined else align the last partition in the user partition */
      if ((emmc_end_align_wp==0) && (!(p->p_partition_entry->MaxBlocksInSubPartition & PARTITION_ATTR_WRITE_PROTECT_GROUP_MASK))) 
	  {
        emmc_end_align_wp = 1;
	    if(prev_start_block_number % EMMC_WP_BLOCK_ALIGN_SIZE) 
		{
          prev_start_block_number = ALIGN_BLOCK_FOR_WP(prev_start_block_number);
          p_prev_wp->p_partition_entry->StartBlock = prev_start_block_number;
	    }
      }
      else if ((emmc_end_align_wp==1) && (p->p_partition_entry->MaxBlocksInSubPartition & PARTITION_ATTR_WRITE_PROTECT_GROUP_MASK)) 
	  {
        emmc_end_align_wp = 0;
	    if(prev_start_block_number % EMMC_WP_BLOCK_ALIGN_SIZE) 
		{
          prev_start_block_number = ALIGN_BLOCK_FOR_WP(prev_start_block_number);
          p_prev_wp->p_partition_entry->StartBlock = prev_start_block_number;
	    }
      }
#endif
      p->rev_end_block = prev_start_block_number;
      p->p_partition_entry->EndBlock   = p->rev_end_block;
      prev_start_block_number = p->rev_end_block + PARTITION_BLOCK_SIZE(p->end_block, p->start_block);
    }
    else /* if p is null, break the loop */
      break;
  }
  /******************************** debug info starts ********************************/
  
   p = (&queue)->front;
   printf("\nPriority Queue:\n"); 
   while( p != NULL)
   {
    printf("ID[%02X] S[%08X] E[%08X] RS[%04X] RE[%04X] PE[0x%x]\n",
               p->ID,
               p->start_block,
               p->end_block,
               p->rev_start_block,
               p->rev_end_block,
               (U32)p->p_partition_entry);
    p = p->next;
   }
   p = (&queue)->front;
   printf("\nMain Partition:\n");
   while( p != NULL)
   {   
      printf("ID[%02X] IT[%02X] S[%08X] E[%08X] MB[%04X] SR[%04X] ER[%04X] O[%08X] RES[%08X]\n",
               p->p_partition_entry->ID,
               p->p_partition_entry->ImageType,
               p->p_partition_entry->StartBlock,
               p->p_partition_entry->EndBlock,
               p->p_partition_entry->MaxBlocksInSubPartition,
               p->p_partition_entry->StartReservoirBlock,
               p->p_partition_entry->EndReservoirBlock,
               p->p_partition_entry->Options,
               p->p_partition_entry->Reserved);
     p = p->next;
   }
  
  /********************************* debug info ends **************************************/
  /* Step 1: ends*/

  /* Step 2: starts */
  p = (&queue)->front;
  printf("\nUpdated dynamic partition:\n");
  
  while(p != NULL)
  {
    if (p->p_partition_entry->ID == id2)
    {
      U32 partition_blk_size = PARTITION_BLOCK_SIZE(p->p_partition_entry->EndBlock, p->p_partition_entry->StartBlock);
      p->p_partition_entry->EndBlock = NEGATIVE_ZERO_IN_SIGNED_MAGNITUDE_FORMAT;
      p->p_partition_entry->StartBlock  = p->p_partition_entry->EndBlock | partition_blk_size; 
    }
    /* If partition ID is identified, don't need to traverse further */
    if (p->ID == id2)
    {
      break;
    }
    p = p->next;
  }  
  /* Step 2:ends */

#if defined(FEAT_FLASH_WRITE_PROTECT_ENABLED)
  p = (&queue)->front;
  printf("\nUpdate  partition Primary GPT to code start (0x400) as no free space is allowed in WP blocks\n");
  while(p != NULL)
  {
#if defined(VMM_MOBILEVISOR)
    if (p->p_partition_entry->ID == PARTITION_ID_PRIMARY_GPT)
    {
      p->p_partition_entry->EndBlock = 0x4000;
    } else if (p->p_partition_entry->ID == PARTITION_ID_BCB) 
    {
      p->p_partition_entry->EndBlock = p->p_partition_entry->StartBlock + NO_OF_BLOCKS(MMAP_BCB_SIZE);
    } else if (p->p_partition_entry->ID == PARTITION_ID_SLB_IMAGE) 
    {
      p->p_partition_entry->EndBlock = p->p_partition_entry->StartBlock + NO_OF_BLOCKS(MMAP_SLB_SIZE);
    }
#else
    if (p->p_partition_entry->ID == PARTITION_ID_BCB)
    {
	p->p_partition_entry->EndBlock = 0x400;
	break;
    }
#endif
    p = p->next;
  }
#endif
  
  /****************************************** debug info starts ********************************/  
  
  for( int i = 0; i < MAX_MAP_ENTRIES; i++)
  {
    if (g_p_mmap[i].Class == NOT_USED)
      break;
          
    if (g_image_type[i] == PARTITION_ID_EMPTY ||
        g_image_type[i] == PARTITION_ID_EBL ||
        g_image_type[i] == PARTITION_ID_PSI_RAM)
      {
        continue;
      }

    printf("%cLA[%08X] ID[%02X:%-20s] IT[%02X] S[%08X] E[%08X] MB[%04X] SR[%04X] ER[%04X] O[%08X] RES[%08X]\n",
               g_p_mmap[i].Start == 0 ? '*' : ' ',
               g_p_mmap[i].Start,
               (U32)p_part_entry_front->ID,
               g_partition_info[p_part_entry_front->ID].id_string,
               (U32)p_part_entry_front->ImageType,
               (U32)p_part_entry_front->StartBlock,
               (U32)p_part_entry_front->EndBlock,
               (U32)p_part_entry_front->MaxBlocksInSubPartition,
               (U32)p_part_entry_front->StartReservoirBlock,
               (U32)p_part_entry_front->EndReservoirBlock,
               (U32)p_part_entry_front->Options,
               (U32)p_part_entry_front->Reserved); 
         
  p_part_entry_front++;
  }

  /******************************************** debug info ends ************************************/
  return 1;
#endif
}
/****************************************************************
 * prggen_sanity
 *  Peform sanity check of the assumptions.
 *
 * Rules:
 ****************************************************************/
void prggen_sanity(void)
{
  printf("Sanity test...\n");
  MMAP_UPDATE_PARTITION_INFO_IT(EMPTY, FAT_IMAGE);
#if defined(FEAT_FTLE_PRGGEN)
  if (g_partition_info[PARTITION_ID_EMPTY].d.image_type != FTLE_IT_FAT_IMAGE)
#elif defined(FEAT_NAND_PRGGEN)
  if (g_partition_info[PARTITION_ID_EMPTY].d.image_type != PARTITION_IT_FAT_IMAGE))
#endif
    printf("Incorrect MMAP_UPDATE_PARTITION_INFO_IT\n");

  prggen_ftl_sanity();
}

/****************************************************************
 * prggen_init
 *  Setup the descriptors needed for generating PRG.
 *
 * Rules:
 ****************************************************************/
#define DESCRIPTION_INCLUDE_(C,F) <category/c##C##c/mem_##C##_description_##F
#define DESCRIPTION_INCLUDE(C,F) DESCRIPTION_INCLUDE_(C,F)
// #define DESCRIPTION_INCLUDE(C,F) DESCRIPTION_INCLUDE_(C,##F##.h>)

void prggen_init(void)
{
#define MMAP_INCLUDE_MEMORYMAP_INFO
#define MMAP_INCLUDE_PARTITION_INFO

  /* anything provided in mem_static_cfg.h is final! */
  #include <mem_static_cfg.h>
  
  /* decriptions based on category overrides default */
#if defined(MMAP_DESCRIPTION_CATEGORY)
  #define STR_(X) #X
  #define STR(X) STR_(X)
  printf("Description category enabled: %s\n", STR(MMAP_DESCRIPTION_CATEGORY));
  #undef STR
  #undef STR_

  #include DESCRIPTION_INCLUDE(MMAP_DESCRIPTION_CATEGORY,partition.h>)
  #include DESCRIPTION_INCLUDE(MMAP_DESCRIPTION_CATEGORY,memorymap.h>)
  #include DESCRIPTION_INCLUDE(MMAP_DESCRIPTION_CATEGORY,tags.h>)
#endif  /* MMAP_DESCRIPTION_CATEGORY */
  /* default descriptions */
  #include <mem_description_partition.h>
  #include <mem_description_memorymap.h>
  #include <mem_description_tags.h>

#undef MMAP_INCLUDE_MEMORYMAP_INFO
#undef MMAP_INCLUDE_PARTITION_INFO
}

/****************************************************************
 * prggen_add_partitions_direct
 *  Add the partitions that are directly declared without memory-map.
 *
 * Rules:
 ****************************************************************/
void prggen_add_partitions_direct(void)
{
  U32 i;

  /* add partition only entries after the used memory-map */
  for (i = 0; i < MAX_MAP_ENTRIES; ++i)
  {
    if (g_p_mmap[i].Class == NOT_USED)
      break;
  }
  /* we are at the end of the used memory-map */

  MemoryMapEntryStructType *p_mmap = &g_p_mmap[i];
  struct partition_control_advanced_s *p_partition_control_advanced = &g_partition_control_advanced[i];
  U32 *p_image_type = &g_image_type[i];

#define MMAP_ADD_MEMORYMAP_ENTRY(NAND_ID, _BASE_, _SIZE_)    MMAP_ADD_PARTITION(NAND_ID, _BASE_, _SIZE_)
                                                  
#define MMAP_INCLUDE_PARTITION
  /* anything provided in mem_static_cfg.h is final! */
  #include <mem_static_cfg.h>
  /* decriptions based on category overrides default */
#if defined(MMAP_DESCRIPTION_CATEGORY)
  #define STR_(X) #X
  #define STR(X) STR_(X)
  printf("Description category enabled: %s\n", STR(MMAP_DESCRIPTION_CATEGORY));
  #undef STR
  #undef STR_

  #include DESCRIPTION_INCLUDE(MMAP_DESCRIPTION_CATEGORY,partition.h>)
#endif  /* MMAP_DESCRIPTION_CATEGORY */
  /* default descriptions */
  #include <mem_description_partition.h>

#undef MMAP_INCLUDE_PARTITION

  #include <mem_partition_cfg.h>

#if defined(MM_CUST_SCF_SUPPORT)
  #include <modem_cust_internal.h>
#endif

#undef MMAP_ADD_MEMORYMAP_ENTRY
#undef MMAP_ADD_PARTITION
#undef MMAP_ADD_PARTITION_EX
}


/****************************************************************
 * prggen_add_memorymap_direct
 *  Add the memorymap entries that are directly declared without partition.
 *
 * Rules:
 ****************************************************************/
void prggen_add_memorymap_direct(void)
{
  U32 i;
  U32 idx=0;
  
  /* add the entries after the used memory-map */
  for (i = 0; i < MAX_MAP_ENTRIES; ++i)
  {
    if (g_p_mmap[i].Class == NOT_USED)
      break;

    // Clean up memory map section. Remove if tag does not exist       
    if((g_partition_info[g_image_type[i]].d.tag[0] == '\0') || ((char)g_p_mmap[i].FormatSpecify[0] == '\0'))
      {
        g_p_mmap[i].Start = 0;
        g_p_mmap[i].Class = NOT_USED;
        g_p_mmap[i].Length = 0;
      }
  }
  
  /* we are at the end of the used memory-map */
  /* clear out the entries that are empty(NOT_USED) but at the middle of the memory_map */
  MemoryMapEntryStructType *p_mmap = &g_p_mmap[0];
  U32 j=0;
  U32 k=0;

  for (U32 j = 0; j < MAX_MAP_ENTRIES; j++)
  {
    if (p_mmap[j].Class == NOT_USED)
    {
      /* First free entry */
      i = j;
      for(k=j+1; k<MAX_MAP_ENTRIES; k++)
      {
        if (p_mmap[k].Class != NOT_USED)
        {
          p_mmap[j].Start = p_mmap[k].Start;
          p_mmap[j].Class = p_mmap[k].Class;
          p_mmap[j].Length = p_mmap[k].Length;
          strcpy((char *)&p_mmap[j].FormatSpecify[0], (char *)&p_mmap[k].FormatSpecify[0]);
          
          p_mmap[k].Start = 0;
          p_mmap[k].Class = NOT_USED;
          p_mmap[k].Length = 0;
          strcpy((char *)&p_mmap[k].FormatSpecify[0], (char *)("\0"));
          
          /* First free entry in memory-map after all the valid memory-map entries  */
          i = k;
        }
        j++;
      }
    }
  }

  /* we are at the end of the used memory-map so that any new addition that happens to the memory-map from the mem_static_cfg.h happens at the end*/
  p_mmap = &g_p_mmap[i];
#define MMAP_INCLUDE_MEMORYMAP
#define MMAP_ADD_MEMORYMAP_ENTRY(NAND_ID, base, size)  MMAP_ADD_MEMORYMAP(NAND_ID, base, size)
  /* add the entries after the used memory-map */  
  /* anything provided in mem_static_cfg.h is final! */
  #include <mem_static_cfg.h>
#undef MMAP_ADD_MEMORYMAP_ENTRY
#undef MMAP_INCLUDE_MEMORYMAP
}

/****************************************************************
 * prggen_svg_create_partition
 *  Creates a HTML file in the output directory which displays the
 *  memory layout of the system
*****************************************************************/
void prggen_svg_create_partition(PartitionEntryStructType *p_part_entry)
{
  FILE *fp;
  U32  x, y[3], width, height, ygap;
  U32 i;

  sprintf(p_layout_path, "%s/../partition.html",p_input_path);
  fp = fopen(p_layout_path, "w");  /* creates partition layout .html */
  if (fp == NULL)
  {
    exit(2);
  }

  /* for each memory map entry */
  for (i = 0; i < MAX_MAP_ENTRIES; ++i)
  {
    /* no more valid entries, abort */
    if (g_p_mmap[i].Class == NOT_USED)
      break;
  }

  fprintf(fp, "<svg xmlns=\"http://www.w3.org/2000/svg\">\n");

  x = 10;
  y[0] = y[1] = y[2] = 90;
  width = 150;
  height = 150;
  ygap = 2;

    
    fprintf(fp, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"10\" fill=\"red\">\n", x+25 , 10);
    fprintf(fp, "    Project Name is %s\n",PLATFORM_PROJECTNAME);
    fprintf(fp, "    </text>\n"); 
    fprintf(fp, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"10\" fill=\"red\">\n", x+25 , 30);
    fprintf(fp, "    Build Stage is %s\n",p_int_stage);
    fprintf(fp, "    </text>\n"); 
    fprintf(fp, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"10\" fill=\"blue\">\n", x+25 , 70);
    fprintf(fp, "    %s(%d)\n","USER-PART", 0);
    fprintf(fp, "    </text>\n"); 
    fprintf(fp, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"10\" fill=\"blue\">\n", x+210 , 70);
    fprintf(fp, "    %s(%d)\n","BOOT-PART", 1);
    fprintf(fp, "    </text>\n"); 
    fprintf(fp, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"10\" fill=\"blue\">\n", x+380 , 70);
    fprintf(fp, "    %s(%d)\n","BOOT-PART", 2);
    fprintf(fp, "    </text>\n"); 
  /* for each partition */
  while (p_part_entry->EndBlock != 0)
  {
    U32   size_i = ((p_part_entry->EndBlock - p_part_entry->StartBlock) * MMAP_FLASH_BLOCK_SIZE) / (1024 * 1024);
    float size = ((p_part_entry->EndBlock - p_part_entry->StartBlock) * MMAP_FLASH_BLOCK_SIZE) / (1024.0 * 1024);
    U32   IT = p_part_entry->ImageType;

    x = 10 + IT * (width + 20);
//    y = 10 + p_part_entry->StartBlock / 2;

    fprintf(fp, "  <g>\n");
    fprintf(fp, "    <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" stroke=\"black\" fill=\"yellow\"> </rect>\n", x, y[IT], width, height);

    fprintf(fp, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"10\" fill=\"black\">\n", x+5, y[IT]+15);
    fprintf(fp, "    Start blk: 0x%08X\n", (U32)p_part_entry->StartBlock);
    fprintf(fp, "    </text>\n"); 
    fprintf(fp, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"10\" fill=\"black\">\n", x+width - 115, y[IT] + height-5);
    fprintf(fp, "    End blk: 0x%08X\n", (U32)p_part_entry->EndBlock);
    fprintf(fp, "    </text>\n"); 

    fprintf(fp, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"10\" fill=\"red\">\n", x + 10, y[IT] + 70);
//    printf("%cLA[%08X] ID[%02X:%-20s] IT[%02X] S[%08X] E[%08X] MB[%04X] SR[%04X] ER[%04X] O[%08X] RES[%08X]\n",
    fprintf(fp, "    %s(%02X)\n",
            g_partition_info[p_part_entry->ID].id_string,
            (U32)p_part_entry->ID);
    fprintf(fp, "    </text>\n"); 

#if 0
    fprintf(fp, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"10\" fill=\"blue\">\n", x + 10, y[IT] + 40);
    fprintf(fp, "    %s(%d)\n",
            p_part_entry->ImageType == 0x00 ? "USER-PART" : "BOOT-PART",
            p_part_entry->ImageType);
    fprintf(fp, "    </text>\n"); 
#endif

    fprintf(fp, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"10\" fill=\"blue\">\n", x + 10, y[IT] + 80);

    if (size_i == 0)
    {
      size = ((p_part_entry->EndBlock - p_part_entry->StartBlock) * MMAP_FLASH_BLOCK_SIZE) / (1024.0);
      fprintf(fp, "    %0.2f kB\n", size);
    }
    else
    {
      fprintf(fp, "    %0.2f MB\n", size);
    }

    fprintf(fp, "    </text>\n"); 

    fprintf(fp, "  </g>\n");
#if 0
    p_part_entry->ImageType,
    p_part_entry->StartBlock,
    p_part_entry->EndBlock,
    p_part_entry->MaxBlocksInSubPartition,
    p_part_entry->StartReservoirBlock,
    p_part_entry->EndReservoirBlock,
    p_part_entry->Options,
    p_part_entry->Reserved
#endif
    y[p_part_entry->ImageType] += height + ygap;
    ++p_part_entry;
  }
  
  fprintf(fp, "</svg>\n");
  fclose(fp);

}

/****************************************************************
 * NewElementFunction
 *  Callback from PRGSequencer to add elements of PRG file.
 *
 * Rules:
 ****************************************************************/
void NewElementFunction(void *struct_ptr)
{
  ElementHeaderStructType *E = (ElementHeaderStructType *)struct_ptr;

  switch(E->Type)
  {
    case GENERIC_HEADER_ELEM_TYPE:
    {
      GenericHeaderElementStructType *p_struct = (GenericHeaderElementStructType *)struct_ptr;

      if(p_struct->Data.FileType == 0)
      {
        p_struct->Data.FileType = PRG_FILE_TYPE;
        p_struct->Data.Marker   = PRG_MAGIC_NUMBER;
      }
    }
    break;

    case HARDWARE_ELEM_TYPE:
    {
      HardwareElementStructType *p_struct = (HardwareElementStructType *)struct_ptr;

      p_struct->Data.Platform   = PLATFORM_CHIPSET;
      p_struct->Data.SystemSize = MMAP_MODEM_SW_FLASH_SIZE;
      p_struct->Data.BootSpeed  = PLATFORM_BOOTSPEED;
      p_struct->Data.HandshakeMode    = PLATFORM_HANDSHAKE_MODE;
      p_struct->Data.UsbCapable       = PLATFORM_USBCAPABLE;
      p_struct->Data.FlashTechnology  = PLATFORM_FLASHTECHNOLOGY;

      strcpy((char *)&p_struct->Data.ProjectName[0], PLATFORM_PROJECTNAME);
      strcpy((char *)&p_struct->Data.DepTemplate[0], "xxxx_delta%05d.%03d.dep");
      strcpy((char *)&p_struct->Data.CfgTemplate[0], "xxxx_eep%03d.cfg");

#if !defined(FEAT_NAND_PRGGEN)&& !defined(FLASHLESS) && !defined(FEAT_FTLE_PRGGEN)
      p_struct->Data.ChipSelect.CS3[0].ChipSelectNumber = 0; /* TODO: check this assumption always holds good, BOOT on CS0 */
      p_struct->Data.ChipSelect.CS3[0].BUSRCON = 0x04002001; /* TODO: take these values from somewhere */
      p_struct->Data.ChipSelect.CS3[0].BUSWCON = 0x04002001;
      p_struct->Data.ChipSelect.CS3[0].BUSRP   = 0x44500412;
      p_struct->Data.ChipSelect.CS3[0].BUSWP   = 0x44500412;
#else
      p_struct->Data.ChipSelect.CS3[0].ChipSelectNumber = 0xFF;
#endif  /* NOR */

      p_struct->Data.ChipSelect.CS3[1].ChipSelectNumber = 0xFF; /* TODO: check this assumption always holds good, BOOT on CS0 */
      p_struct->Data.ChipSelect.CS3[2].ChipSelectNumber = 0xFF; /* TODO: check this assumption always holds good, BOOT on CS0 */
    }
    break;

    /* Memory Map section */
    case MEMORYMAP_ELEM_TYPE:
    {
      MemoryMapStructType *p_struct = &((MemoryMapElementStructType *)struct_ptr)->Data;
      MemoryMapEntryStructType mmap[MAX_MAP_ENTRIES] = {0};
      MemoryMapEntryStructType *p_mmap = &mmap[0];
      U32 image_type[MAX_MAP_ENTRIES] = {0};
      U32 *p_image_type = &image_type[0];
      U32 i, j;

      #define MMAP_ADD_MEMORYMAP_ENTRY(ID, _BASE_, _SIZE_)  MMAP_ADD_MEMORYMAP(ID, _BASE_, _SIZE_) \
                                                            MMAP_ADD_MEMORYMAP_OPT(ID, _BASE_, _SIZE_)
                                                            
      /* include boot loader sections */
      #include <mem_description_bootldr.h>

      /* include modem sections */
      #define INCLUDED_IN_C_FILE
      #include <modem_sw.scf.in>
      
      #define MMAP_INCLUDE_MEMORYMAP
      #define MMAP_INCLUDE_PARTITION

      /* anything provided in mem_static_cfg.h is final! */
      #include <mem_static_cfg.h>

      /* decriptions based on category overrides default */
#if defined(MMAP_DESCRIPTION_CATEGORY)
      #define STR_(X) #X
      #define STR(X) STR_(X)
      printf("Description category enabled: %s\n", STR(MMAP_DESCRIPTION_CATEGORY));
      #undef STR
      #undef STR_
      #include DESCRIPTION_INCLUDE(MMAP_DESCRIPTION_CATEGORY,memorymap.h>)
#endif  /* MMAP_DESCRIPTION_CATEGORY */

      /* default descriptions */
      #include <mem_description_memorymap.h>

      #undef MMAP_INCLUDE_MEMORYMAP
      #undef MMAP_INCLUDE_PARTITION
      #undef INCLUDED_IN_C_FILE
      #undef MMAP_ADD_MEMORYMAP_ENTRY
      
      /* all sections are included now, start the processing */

      /* merge the entries that contains same class and tag with overlapping address space */
      for (i = 0; i < MAX_MAP_ENTRIES; ++i)
      {
        if (mmap[i].Start == 0 || mmap[i].Class == NOT_USED)
          continue;

        printf("Trying to Merge %s\n", mmap[i].FormatSpecify);
        printf("\tA[%08X]S[%08X]\n", mmap[i].Start, mmap[i].Length);
        for (j = i + 1; j < MAX_MAP_ENTRIES; j++)
        {
          if (mmap[j].Start == 0 || mmap[i].Class == NOT_USED)
            continue;

          if ((strcmp((char *)&mmap[j].FormatSpecify[0], (char *)&mmap[i].FormatSpecify[0]) == 0)
              && (mmap[i].Class == mmap[j].Class))
          {
            S32 delta = mmap[j].Start - (mmap[i].Start + mmap[i].Length);

            if (delta < 0) delta = -delta;

            mmap[i].Start  = mmap[i].Start < mmap[j].Start ? mmap[i].Start : mmap[j].Start;
            mmap[i].Length += delta + mmap[j].Length;

            printf("\tA[%08X]S[%08X] => [%08X][%08X]\n", mmap[j].Start, mmap[j].Length,
                                                         mmap[i].Start, mmap[i].Length);

            mmap[j].Class = NOT_USED;
            mmap[j].FormatSpecify[0] = 0;

            image_type[j] = PARTITION_ID_EMPTY;
          }
        }
        printf("\n");
      }

      for (i = 0; i < MAX_MAP_ENTRIES; ++i)
      {
        if (mmap[i].Start == 0 || mmap[i].Class == NOT_USED)
          continue;

        for (j = i + 1; j < MAX_MAP_ENTRIES; j++)
        {
          if (mmap[j].Start == 0 || mmap[i].Class == NOT_USED)
            continue;

            if (((mmap[i].Class == BOOT_CORE_SLB_CLASS) &&
                (mmap[j].Class == CODE_CLASS)) && (mmap[i].Start == mmap[j].Start))
            {
              if (mmap[i].Start == mmap[j].Start)
              {
                printf("Found overlapping sections of start address and length:\n");
                printf("\tA[%08X]S[%08X] : [%08X][%08X]\n", mmap[i].Start, mmap[i].Length,
                                                            mmap[j].Start, mmap[j].Length);
                mmap[j].Start += MMAP_MODEM_SW_BOOT_VECT_SLB_SIZE; //mmap[i].Length; Force code to start from the start of the second flash block
                printf("Correcting overlapping sections:\n");
                printf("\tA[%08X]S[%08X] : [%08X][%08X]\n", mmap[i].Start, mmap[i].Length,
                                                            mmap[j].Start, mmap[j].Length);
              }
            }
        }
        printf("\n");
      }

      /* setup basics */
      p_struct->FlashStartAddr        = MMAP_MODEM_SW_FLASH_BASE;

      /* TODO: check how of the below are still relevent from XMM6180 projects... */
      p_struct->EepRevAddrInSw        = MMAP_MODEM_SW_STATIC_CFG_BASE;
      p_struct->EepRevAddrInEep       = 0; //MMAP_EEP_SECTION_BASE + 0x3FFA; /* TODO: remove this hardcoded value */
      p_struct->EepStartModeAddr      = 0; //MMAP_EEP_SECTION_BASE + 0x3FE0; /* TODO: remove this hardcoded value */
      p_struct->NormalModeValue       = 2;
      p_struct->TestModeValue         = 0;
      p_struct->SwVersionStringLocation = MMAP_MODEM_SW_SYS_VER_BASE;

      g_p_mmap = p_mmap = p_struct->Entry;
      memset(p_mmap, 0, MAX_MAP_ENTRIES * sizeof(mmap[0]));

      /* Dump the memory map entries for visualization purpose */
      for (i = 0; i < MAX_MAP_ENTRIES; ++i)
      {
        if (mmap[i].Class != NOT_USED)
        {
          memcpy(p_mmap, &mmap[i], sizeof(mmap[i]));
          *g_p_image_type = image_type[i];

          printf("%c%-30s\t\tS[%08X] L[%08X] C[%X] F[%X]\n",
            (g_partition_info[image_type[i]].upd.h2f[0] == 1) ? '*' : ' ',
            p_mmap->FormatSpecify, p_mmap->Start, p_mmap->Length, p_mmap->Class, p_mmap->Flags);

          if (g_partition_info[image_type[i]].upd.h2f[0] == 1)
          {
            fprintf(fp, "%s\n", g_partition_info[image_type[i]].d.h2f);
          }
          ++p_mmap;
          ++g_p_image_type;
        }
      }
    }
    break;

    /* Partition Table */
    case NANDPARTITION_ELEM_TYPE:
    {
      NandPartitionDataStructType *p_struct = &((NandPartitionElementStructType *)struct_ptr)->Data;
      PartitionStructType         *p_partition = (PartitionStructType *)&(p_struct->Partition);
      PartitionEntryStructType    *p_part_entry = (PartitionEntryStructType *)&(p_partition->Partition);
      p_part_entry_front = p_part_entry;
      LoadAddrToPartitionTableStruct *p_load_t = (LoadAddrToPartitionTableStruct *)&(p_struct->LoadToPartition);

      U32 i, j;

      /* wipe */
      memset(p_partition, 0, sizeof(PartitionStructType));
      memset(p_partition->Partition, 0, sizeof(PartitionEntryStructType) * MAX_PARTITIONS);
      memset(p_load_t, 0, sizeof(LoadAddrToPartitionTableStruct));
      p_partition->PartitionMarker = PARTITION_MARKER;
      
      /* include the partitions that do-not contain memory map entries */
      prggen_add_partitions_direct();

      /* prepare the partition information (e.g. region) before looping in each partition */
      prggen_ftl_partition_init(p_partition);

      /* fill the partition details for each memory map entry */
      for (i = 0, j = 0; i < MAX_MAP_ENTRIES; ++i)
      {
        U32 blk_size;

        if (g_p_mmap[i].Class == NOT_USED)
          break;

        if (g_image_type[i] == PARTITION_ID_EMPTY ||
            g_image_type[i] == PARTITION_ID_EBL ||
            g_image_type[i] == PARTITION_ID_PSI_RAM)
        {
          continue;
        }

        p_part_entry->ID = g_image_type[i];

        /* update the partition details that are generic */
        if (g_partition_control_advanced[i].level == 0 /* automatic */ )
        {
          p_part_entry->ImageType = g_partition_info[p_part_entry->ID].d.image_type;
          p_part_entry->Options = g_partition_info[p_part_entry->ID].d.u_options.options;

          /* If Byte or Block access is set then update the resrerved field with Load address */
          if ((p_part_entry->Options & PARTITION_OPT_ALLOW_UTA_BLOCK_IF_ACCESS) ||
              (p_part_entry->Options & PARTITION_OPT_ALLOW_UTA_BYTE_IF_ACCESS))
          {
            if (g_image_type[i] != PARTITION_ID_UA_IMAGE)
            {
              p_part_entry->Reserved = g_p_mmap[i].Start;
            }
          }

          /* nandhini to fix */
          if (g_image_type[i] == PARTITION_ID_UA_IMAGE)
          {
#if defined(MMAP_UA_IMAGE_VIRTUAL_BASE)
            p_part_entry->Reserved = MMAP_UA_IMAGE_VIRTUAL_BASE;
#else
            printf("MMAP_UA_IMAGE_VIRTUAL_BASE not set. Loading default legacy value, 0xA0000000\n");
            p_part_entry->Reserved = 0xA0000000;
#endif
          }
        }
        else if (g_partition_control_advanced[i].level == 1 /* slight manual control */ )
        {
          p_part_entry->ImageType = g_partition_control_advanced[i].ImageType;
          p_part_entry->Options = g_partition_control_advanced[i].Options;
          p_part_entry->Reserved = g_partition_control_advanced[i].Reserved;
        }

        /* update the partition details that are specific to storage type */
        prggen_ftl_partition_update(p_part_entry, g_p_mmap[i].Length);

#if defined (FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END)
        /* construct the priority queue based on End block number */
        pqinsert(&queue,p_part_entry); 
#endif
        /* Dump the partition table for visualization */
        printf("%cLA[%08X] ID[%02X:%-20s] IT[%02X] S[%08X] E[%08X] MB[%04X] SR[%04X] ER[%04X] O[%08X] RES[%08X]\n",
               g_p_mmap[i].Start == 0 ? '*' : ' ',
               g_p_mmap[i].Start,
               (U32)p_part_entry->ID,
               g_partition_info[p_part_entry->ID].id_string,
               (U32)p_part_entry->ImageType,
               (U32)p_part_entry->StartBlock,
               (U32)p_part_entry->EndBlock,
               (U32)p_part_entry->MaxBlocksInSubPartition,
               (U32)p_part_entry->StartReservoirBlock,
               (U32)p_part_entry->EndReservoirBlock,
               (U32)p_part_entry->Options,
               (U32)p_part_entry->Reserved);

        if (p_part_entry->EndReservoirBlock > MAX_NUM_BLOCKS)
        {
          printf("Error: Insufficient space in storage erb=0x%X, mb=0x%X\n", (U32)p_part_entry->EndReservoirBlock, MAX_NUM_BLOCKS);
          exit(2);
        }

        p_load_t->Table[j] = g_p_mmap[i].Start;

        ++p_part_entry;
        ++j;
      }
      /* Generate the graphical view of the partitions */
      prggen_svg_create_partition((PartitionEntryStructType *)&(p_partition->Partition));
    }

    /* priority queue has been constructed. Now update it according to
     * PRG partitions relative to end of physical eMMC partitions
     * id: partition id, which will have dynamic partition size.
     * It will update iff FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END is defined.
     */
 
#if defined (FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END)
    if (0 < update_block_number(MEM_DYNAMIC_PARTITION_ID, FEAT_OPM_MEM_PARTITION_ID2))
    {
      printf("\nPRG partitions are relative to end of physical eMMC partitions in signed magnitude representation \n");
    }
#endif
    /* include the memory-map entries that do-not have a corresponding partition */
    prggen_add_memorymap_direct();
    break;
  }
}

/****************************************************************
 * main
 *  Entry point for PRGGEN
 *
 * Rules:
 ****************************************************************/
int main(int argc, char **argv)
{
  U32 ret;
  char platform_name_lwr[128];
  char modem_sw_prg[256];
  char *p_board_id;
  FILE *fp_chk = NULL;

#if defined(SIGNING_FLS)
#define NO_OF_ARGS 8  
#else
#define NO_OF_ARGS 7
#endif  

#if defined (FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END)
  /* Initialize the priority queue */
  pqinit(&queue);
#endif

  /* arguments check, and provide help string */
  if (argc < NO_OF_ARGS)
  {
    printf("Insufficient arguments\n");

    printf("%s <Hex2FlsToolPath> <VOBGlobalPath> <BoardId> <ScatterObjPath> <IntStage> <MM_DEFS> HexInput [HexInput...]\n",
      argv[0]);

    exit(2);
  }

  /* assign the arguments to meaning-ful names */
  g_p_hex2fls_tool = argv[1];
  p_global_path = argv[2];
  p_board_id = argv[3];
  p_input_path = argv[4];
  p_int_stage = argv[5];
  p_mm_defs = argv[6];

  printf("\nPrggen-Start\n");

  printf("%s\n", p_mm_defs);

  /* the rest are the input files for fls generation */
  for(int i=NO_OF_ARGS; argv[i] != NULL; i++)
  {
    snprintf(g_p_hex2fls_input_files, sizeof(g_p_hex2fls_input_files), "%s %s",g_p_hex2fls_input_files,argv[i]);
    printf("g_p_hex2fls_input_files = %s\n", g_p_hex2fls_input_files);
  }

  g_p_sign_script = &g_boot_file_loc[0];
  g_p_scf_tools_obj = &g_scf_tools_obj[0];

  string_lower(platform_name_lwr, PLATFORM_PROJECTNAME);

  #if defined(BOOTLDR_DIR)
    sprintf(g_boot_file_loc, "%s", BOOTLDR_DIR);
  #else
    sprintf(g_boot_file_loc, "%s/dwdtools/boot/%s/%s/%s", p_global_path, platform_name_lwr, BOOT_TYPE, p_board_id);
  #endif

  sprintf(g_scf_tools_obj, "%s/mhw_drv_src/memory_management/mem/scatterfile/tools/obj", p_global_path);
  sprintf(p_output_path, "%s/output",p_input_path);

  printf("prggen v2.0:\n");
  printf("Memory Management Team\n");
  printf("Hex2Fls     : %s\n", g_p_hex2fls_tool);
  printf("Boot Loader : %s\n", g_boot_file_loc);

  /* setup the descriptions */
  prggen_init();
  /* verify the descriptions */
  prggen_sanity();

  sprintf(modem_sw_prg, "%s/createfls", p_input_path);
  fp = fopen(modem_sw_prg, "w");  /* creates, createfls shell script */
  if (fp == NULL)
  {
    exit(2);
  }

  sprintf(modem_sw_prg, "%s/modem_sw.prg", p_input_path);

  /* Create a fresh PRG */
  CurrentFile.CleanUp();
  CurrentFile.CallingTool = MAKEPRG_TOOL_ELEM_TYPE; /* mimic as MAKEPRG */
  CurrentFile.SetCallBackFunc(NewElementFunction);
  CurrentFile.AddingNewElement = true;
  /* Add header */
  CurrentFile.CreateElement(GENERIC_HEADER_ELEM_TYPE);

  /* create each elements of PRG */
  for (int i = PRGHANDLER_TOOL_ELEM_TYPE; i < NUM_ELEM_TYPES; i++)
  {
    if (!CurrentFile.IsDynamicElement(i))
      CurrentFile.CreateElement(i);
  }

  /* all set */
  CurrentFile.TamperToolDataBeforeWrite(MAKEPRG_TOOL_REQ_VER);

  /* write the PRG file */
  if (PRG_OK != (ret=CurrentFile.WriteFile(modem_sw_prg)))
  {
    if (ret == PRG_FILE_WRITE_ERROR)
      printf("Save Failed !! (File is Read Only)\n");
    else if (ret == PRG_HASHING_ERROR)
      printf("Save Failed !! (Hasing Error)\n");
    else
      printf("Save Failed !!\n");

    exit(2);
  }

  fclose(fp);
  /* done */
  
  return 0;
}
#endif
