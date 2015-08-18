/* ----------------------------------------------------------------------------

   Copyright (C) 2013-2014 Intel Mobile Communications GmbH
   
        Sec Class: Intel Confidential (IC)
   
   All rights reserved.
   ----------------------------------------------------------------------------

   This document contains proprietary information belonging to IMC.
   Technologies Denmark A/S. Passing on and copying of this document, use
   and communication of its contents is not permitted without prior written
   authorisation.

   Revision Information:
  $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/v1/prggen.cpp $
  ---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#if defined(UNIX_BUILD)
#include <memory.h>
#endif

#include <string.h>

#if defined(FEAT_FTLE_PRGGEN)
  #include <ftle_def.h>
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
#if defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT)
#define MAX_NUM_BLOCKS          0x800000  /*this is only for emmc lepton 4G byte/512*/
#else
#define MAX_NUM_BLOCKS          (MMAP_MODEM_SW_FLASH_SIZE/MMAP_FLASH_BLOCK_SIZE)
#endif

#define NO_OF_BLOCKS(size)      ((size+MMAP_FLASH_BLOCK_SIZE-1)/MMAP_FLASH_BLOCK_SIZE) 
#define RESERVE(p)              (p/100)
#define RESERVE_BLOCKS(size, p) ((NO_OF_BLOCKS((size * (RESERVE(p)))) > 1) ?  NO_OF_BLOCKS(size * (RESERVE(p))) : 1)

#if defined(FEAT_FTLE_PRGGEN)
  #define CAT_NANDID(X)   FTLE_ID_##X
  #define CAT_NANDOPT(X)  FTLE_OPT_##X
#elif defined(FEAT_NAND_PRGGEN)
  #define CAT_NANDID(X)   NAND_ID_##X
  #define CAT_NANDOPT(X)  NAND_OPT_##X
#endif  /* NAND_PRESENT || NANDDFMC_PRESENT */

//These flags have been exported by the Flashtool team. This is used to indicate the options to be set for each of the combined partition.
//A feature accross which the bit is set in a memory map entry means that this feature is excluded for the corresponding entry.
//Example: For Modem SW, EXCLUDE_PAGING bit is set to indicate that this is not a Paging Image.
#if defined(FEAT_FOTA_COMBINED_PARTITION)	
#define EXCLUDE_UTA_FLASH_BYTE_ACCESS  (0x00100000)
#define EXCLUDE_VALIDATION             (0x00200000)
#define EXCLUDE_LOADTORAM              (0x00400000)
#define EXCLUDE_PAGING                 (0x00800000)
#endif

#if defined(FEAT_NAND_PRGGEN)||(defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT)) 
  #include <v1/mem_region_cfg.h>
#endif

/* Temporary hack to generate common prg for both Modem and CDS */
#undef CDS

#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)

#if defined(FEAT_FOTA_COMBINED_PARTITION)	
#define MMAP_ADD_LOAD_REGION(_PRG_NAME_, _CLASS_, _REGION_, _BASE_, _SIZE_)  \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      if((strcmp(#_PRG_NAME_ , "VLX")==0) || (strcmp(#_PRG_NAME_ , "NVM_STATIC_CALIB")==0))\
      	p_mmap->Class  = CUST_CLASS; \
      else \
      	p_mmap->Class  = MMAP_MODEM_SW_RO_CODE_CLASS;  \
      if(strcmp(#_PRG_NAME_ , "CODE_IMAGE")==0) \
      	{  \
    	    p_mmap->Flags  = EXCLUDE_PAGING;  \
    	    strncpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_,strlen(#_PRG_NAME_)>15?15:strlen(#_PRG_NAME_)); \
    	  }  \
      else if(strcmp(#_PRG_NAME_ , "PAGING_IMAGE")==0) \
      	{  \
		      p_mmap->Flags  = EXCLUDE_LOADTORAM;  \
		      strncpy((char *)&p_mmap->FormatSpecify[0], "CODE_IMAGE",strlen("CODE_IMAGE")>15?15:strlen("CODE_IMAGE")); \
		    }  \
		  else  \
		  	{  \
          p_mmap->Flags  = 0; \
          strncpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_,strlen(#_PRG_NAME_)>15?15:strlen(#_PRG_NAME_)); \
        }  \
      p_mmap++; \
      *p_image_type = CAT_NANDID(_PRG_NAME_);  \
      p_image_type++;
#else
#define MMAP_ADD_LOAD_REGION(_PRG_NAME_, _CLASS_, _REGION_, _BASE_, _SIZE_)  \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      if((strcmp(#_PRG_NAME_ , "VLX")==0) || (strcmp(#_PRG_NAME_ , "NVM_STATIC_CALIB")==0))\
      	p_mmap->Class  = CUST_CLASS; \
      else \
      	p_mmap->Class  = MMAP_MODEM_SW_RO_CODE_CLASS;  \
      p_mmap->Flags  = 0; \
      strncpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_,strlen(#_PRG_NAME_)>15?15:strlen(#_PRG_NAME_)); \
      p_mmap++; \
      *p_image_type = CAT_NANDID(_PRG_NAME_);  \
      p_image_type++;
#endif //#if defined(FEAT_FOTA_COMBINED_PARTITION)	

#define MMAP_ADD_BINARY_SECTION_EMPTY(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_)  \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = _PRG_CLASS_;  \
      p_mmap->Flags  = 0; \
      if(strcmp(#_SECTION_ , "SYSTEM")==0) \
        strncpy((char *)&p_mmap->FormatSpecify[0], "SYSTEM",strlen("SYSTEM")>15?15:strlen("SYSTEM")); \
      else if(strcmp(#_SECTION_ , "CACHE")==0) \
        strncpy((char *)&p_mmap->FormatSpecify[0], "CACHE",strlen("CACHE")>15?15:strlen("CACHE")); \
      else if(strcmp(#_SECTION_ , "USERDATA")==0) \
        strncpy((char *)&p_mmap->FormatSpecify[0], "USERDATA",strlen("USERDATA")>15?15:strlen("USERDATA")); \
      else \
        strncpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_,strlen(#_PRG_NAME_)>15?15:strlen(#_PRG_NAME_));  \
      p_mmap++; \
      *p_image_type = CAT_NANDID(_PRG_NAME_);  \
      p_image_type++;

# if defined(FEAT_FW_CUST_BIN)
#define MMAP_ADD_BINARY_SECTION_CUST_EXTEND(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_, _EXTEND_) \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = _PRG_CLASS_;  \
      p_mmap->Flags  = 0; \
      strncpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_,strlen(#_PRG_NAME_)>15?15:strlen(#_PRG_NAME_)); \
      p_mmap++; \
      *p_image_type = CAT_NANDID(_PRG_NAME_);  \
      p_image_type++; \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = CUST_CLASS;  \
      p_mmap->Flags  = 0; \
      sprintf((char *)&p_mmap->FormatSpecify[0], "%s_%s",#_PRG_NAME_,"BIN"); \
      p_mmap++; \
      *p_image_type = CAT_NANDID(_PRG_NAME_);  \
      p_image_type++;
#else
#define MMAP_ADD_BINARY_SECTION_CUST_EXTEND(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_, _EXTEND_) \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = _PRG_CLASS_;  \
      p_mmap->Flags  = 0; \
      strncpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_,strlen(#_PRG_NAME_)>15?15:strlen(#_PRG_NAME_)); \
      p_mmap++; \
      *p_image_type = CAT_NANDID(_PRG_NAME_);  \
      p_image_type++;
#endif	

#else

#define MMAP_ADD_LOAD_REGION(_PRG_NAME_, _CLASS_, _REGION_, _BASE_, _SIZE_)  \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = MMAP_MODEM_SW_RO_CODE_CLASS;  \
      p_mmap->Flags  = 0; \
      strncpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_,strlen(#_PRG_NAME_)>15?15:strlen(#_PRG_NAME_)); \
      p_mmap++;

#define MMAP_ADD_BINARY_SECTION_EMPTY(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_)  \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = _PRG_CLASS_;  \
      p_mmap->Flags  = 0; \
      strncpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_,strlen(#_PRG_NAME_)>15?15:strlen(#_PRG_NAME_)); \
      p_mmap++;

#define MMAP_ADD_BINARY_SECTION_CUST_EXTEND(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_, _EXTEND_) \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = _PRG_CLASS_;  \
      p_mmap->Flags  = 0; \
      strncpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_,strlen(#_PRG_NAME_)>15?15:strlen(#_PRG_NAME_)); \
      p_mmap++; \
      p_mmap->Start  = _BASE_;  \
      p_mmap->Length = _SIZE_;  \
      p_mmap->Class  = CUST_CLASS;  \
      p_mmap->Flags  = 0; \
      sprintf((char *)&p_mmap->FormatSpecify[0], "%s_%s",#_PRG_NAME_,"BIN"); \
      p_mmap++; 
#endif

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

#define MMAP_ADD_DISC_SECTION_EMPTY(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_) \
  MMAP_ADD_BINARY_SECTION_EMPTY(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, 0, 0)
  
#define ALIGN_2K(ADDR) (((ADDR) + 0x7FF) & 0xFFFFF800) 

#include <mem_static_internal.h>

PRGSequencer CurrentFile;

MemoryMapEntryStructType *g_p_mmap;
U32 g_image_type[MAX_MAP_ENTRIES];
U32 *g_p_image_type = &g_image_type[0];
#if defined(FEAT_NAND_PRGGEN)||(defined(FEAT_FTLE_PRGGEN)&&(MM_CUST_SCF_SUPPORT))
U32 g_partition_id_to_region_map[CAT_NANDID(EMPTY)];
#endif
char *p_global_path;
char *p_input_path;

FILE *fp;
char *g_p_hex2fls_tool;
char *g_p_flssign_tool;
char *g_p_hex2fls_input_location;
char *g_p_sign_script;
char *g_p_scf_tools_obj;
char *p_mm_defs;
char *p_int_stage;

char g_boot_file_loc[256];
char g_scf_tools_obj[256];
char g_p_hex2fls_input_files[512];
char *g_p_hex2fls_fota_input_files;
char p_output_path[256];
char g_pre_erase_file[256];
#if defined(INI_FILE)
char g_ini_file[256] = INI_FILE;
#endif

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
  #define FEAT_OPM_MEM_PARTITION_ID2               CAT_NANDID(PARTITION_TABLE)
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
  LIST p = (&queue)->front;
  U32 prev_start_block_number;
    
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
    p = p->next;

    if (p->ID == id)
    {
      (&queue)->dynamic_done = TRUE;
    }

    if(p != NULL)
    {
      p->rev_end_block = prev_start_block_number;
      p->p_partition_entry->EndBlock   = p->rev_end_block;
      prev_start_block_number = p->rev_end_block + PARTITION_BLOCK_SIZE(p->end_block, p->start_block);
    }
    else /* if p is null, break the loop */
      break;
  }
  /******************************** debug info starts ********************************/
  /*
   p = (&queue)->front;
   printf("\nPriority Queue:\n"); 
   while( p != NULL)
   {
    printf("ID[%02X] S[%04X] E[%04X] RS[%04X] RE[%04X] PE[0x%x]\n",
               p->ID,
               p->start_block,
               p->end_block,
               p->rev_start_block,
               p->rev_end_block,
               p->p_partition_entry);
    p = p->next;
   }
   p = (&queue)->front;
   printf("\nMain Partition:\n");
   while( p != NULL)
   {   
      printf("ID[%02X] IT[%02X] S[%04X] E[%04X] MB[%04X] SR[%04X] ER[%04X] O[%08X] RES[%08X]\n",
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
  */
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
      p->p_partition_entry->StartBlock	= p->p_partition_entry->EndBlock | partition_blk_size; 
    }
    /* If partition ID is identified, don't need to traverse further */
    if (p->ID == id2)
    {
      break;
    }
    p = p->next;
  }  
  /* Step 2:ends */

  /****************************************** debug info starts ********************************/  

  for( int i = 0; i < MAX_MAP_ENTRIES; i++)
  {
    if (g_p_mmap[i].Class == NOT_USED)
      break;
          
    if (g_image_type[i] == CAT_NANDID(EMPTY))
      continue;
	
    printf("LA[%08X] ID[%02X] IT[%02X] S[%04X] E[%04X] MB[%04X] SR[%04X] ER[%04X] O[%08X] RES[%08X]\n",
               g_p_mmap[i].Start,
               p_part_entry_front->ID,
               p_part_entry_front->ImageType,
               p_part_entry_front->StartBlock,
               p_part_entry_front->EndBlock,
               p_part_entry_front->MaxBlocksInSubPartition,
               p_part_entry_front->StartReservoirBlock,
               p_part_entry_front->EndReservoirBlock,
               p_part_entry_front->Options,
               p_part_entry_front->Reserved);		   
	p_part_entry_front++;
  }

  /******************************************** debug info ends ************************************/
  return 1;
#endif
}
/*****************************************************************************************************
 * PRG partitions relative to end of physical eMMC partitions implementation end
 ****************************************************************************************************/
 
#if defined(FEAT_OPM_MEM_GPT_SUPPORT)
static U32 get_ftle_attr(U32 image_id)
{
  U32 ftle_attr = 0;
  
  switch(image_id)
  {  
    case CAT_NANDID(PSI_IMAGE):
    case CAT_NANDID(PARTITION_TABLE):
    case CAT_NANDID(SLB_IMAGE):
    case CAT_NANDID(PRIMARY_GPT): 
    case CAT_NANDID(SECONDARY_GPT):    
    case CAT_NANDID(CERTIFICATE_DATA):    
      ftle_attr = 0;
      break;      

    case CAT_NANDID(BCB):
    case CAT_NANDID(BOOT_IMG):
    case CAT_NANDID(RECOVERY_IMG):
    case CAT_NANDID(RECOVERY_IMG_MIRROR):
    case CAT_NANDID(KDUMP_IMG):
      /* 0x00000101 */
      ftle_attr = (FTLE_ATTR_GPT_PARTITION | FTLE_ATTR_GPT_TYPE_GUID_a);
      break;
      
    case CAT_NANDID(LINUX_NVM_FS):
    case CAT_NANDID(SYSTEM):
    case CAT_NANDID(USERDATA):
    case CAT_NANDID(CACHE):
    case CAT_NANDID(USER_DISC):    
      /* 0x00000201 */
      ftle_attr = (FTLE_ATTR_GPT_PARTITION | FTLE_ATTR_GPT_TYPE_GUID_b);      
      break;
      
    case CAT_NANDID(DSP_IMAGE):
    case CAT_NANDID(CODE_IMAGE):
    case CAT_NANDID(VLX):
    case CAT_NANDID(NVM_STATIC_CALIB): 
    case CAT_NANDID(NVM_STATIC_FIX): 
    case CAT_NANDID(NVM_DYNAMIC):
      /* 0x00000301 */
      ftle_attr = (FTLE_ATTR_GPT_PARTITION | FTLE_ATTR_GPT_TYPE_GUID_a | FTLE_ATTR_GPT_TYPE_GUID_b);
      break;
      
    default:
      printf("No FTLE attributes available for this partition ID %s,%d\n",__FUNCTION__,__LINE__);
    break;      
  }
  
  return ftle_attr;
}

static U32 get_options(U32 image_id)
{
  U32 opt = 0;
  
  switch(image_id)
  {
    case CAT_NANDID(PSI_IMAGE):
    case CAT_NANDID(PARTITION_TABLE):
    case CAT_NANDID(PRIMARY_GPT): 
    case CAT_NANDID(SECONDARY_GPT):
    case CAT_NANDID(BCB):
    case CAT_NANDID(LINUX_NVM_FS):
    case CAT_NANDID(CERTIFICATE_DATA):
    case CAT_NANDID(SYSTEM):
    case CAT_NANDID(USERDATA):
    case CAT_NANDID(CACHE):
    case CAT_NANDID(USER_DISC):
      opt = 0;
      break;
      
    case CAT_NANDID(VLX):
    case CAT_NANDID(BOOT_IMG):
    case CAT_NANDID(RECOVERY_IMG):
    case CAT_NANDID(RECOVERY_IMG_MIRROR):
    case CAT_NANDID(DSP_IMAGE):
    case CAT_NANDID(CODE_IMAGE):
      opt = CAT_NANDOPT(LOADMAP) | CAT_NANDOPT(COPYTORAM) | CAT_NANDOPT(VERIFICATION_REQUIRED); /* 15 */
      break;
      
    case CAT_NANDID(SLB_IMAGE):
      opt = CAT_NANDOPT(LOADMAP) | CAT_NANDOPT(VERIFICATION_REQUIRED); /* 11 */;
      break;
      
    case CAT_NANDID(NVM_STATIC_CALIB):
      opt = CAT_NANDOPT(COPYTORAM); /* 4 */;
      break;
      
    default:
      printf("Prg file Generation process error %s,%d\n",__FUNCTION__,__LINE__);
    break;
  }
  
  return opt;
}
#endif/* FEAT_OPM_MEM_GPT_SUPPORT */

static U32 get_part_type(U32 image_id)
{
  U32 image_type = 0;

#if defined(FEAT_NAND_PRGGEN)
  switch(image_id)
  {
    case CAT_NANDID(PSI_IMAGE):
    case CAT_NANDID(SLB_IMAGE):
      image_type = NAND_IT_BOOTUP_IMAGE;
    break;

    case CAT_NANDID(CUSTOMIZATION_DATA):
    case CAT_NANDID(CODE_IMAGE):
    case CAT_NANDID(DSP_IMAGE):
    case CAT_NANDID(CDS_IMAGE):
    case CAT_NANDID(UA_IMAGE):
    case CAT_NANDID(PAGING_IMAGE):
	case CAT_NANDID(VLX):
	case CAT_NANDID(LINUX_KERNEL):
	case CAT_NANDID(INITRD):
	case CAT_NANDID(INITRD_RECOVERY):
      image_type = NAND_IT_BBR_IMAGE;
    break;

    case CAT_NANDID(CERTIFICATE_DATA):
#if !defined(MM_CUST_SCF_SUPPORT)
      image_type = NAND_IT_BBR_IMAGE;
#else
      image_type = NAND_IT_RAW_IMAGE;      
#endif      
    break;    
    case CAT_NANDID(CUSTOMIZATION_DISC):
    case CAT_NANDID(USBRO):
    case CAT_NANDID(ROOT_DISC):
    case CAT_NANDID(USER_DISC):
      image_type = NAND_IT_FAT_IMAGE;
    break;

    case CAT_NANDID(FTL_STARTUP_DISC):
      image_type = NAND_IT_FTL_IMAGE;
    break;

    case CAT_NANDID(BAD_BLOCK_TABLE):
    case CAT_NANDID(RAW_NAND_IMAGE):
    case CAT_NANDID(PREFLASH):
    case CAT_NANDID(DYN_SDS_IMAGE):
    case CAT_NANDID(PARTITION_TABLE):
      image_type = NAND_IT_RAW_IMAGE;
    break;

    case CAT_NANDID(NVM_STATIC_CALIB):
    case CAT_NANDID(NVM_STATIC_FIX):
    case CAT_NANDID(NVM_DYNAMIC):
      image_type = NAND_IT_SDS_IMAGE;
    break;

    default:
      printf("Prg file Generation process error 0x%X, %s,%d\n", image_id,__FUNCTION__,__LINE__);
    break;
  }
  
  return image_type;
#elif defined(FEAT_FTLE_PRGGEN)
  switch(image_id)
  {
  #if defined(FEAT_OPM_MEM_GPT_SUPPORT)||defined(MM_CUST_SCF_SUPPORT)
    case CAT_NANDID(PARTITION_TABLE):
  #endif
    case CAT_NANDID(PSI_IMAGE):
    	image_type = 1;
    break;
    
    case CAT_NANDID(SLB_IMAGE):
	#if defined(MM_CUST_SCF_SUPPORT)
    case CAT_NANDID(CERTIFICATE_DATA):
	#endif
      image_type = 2;
    break;
    
    default:
      image_type = 0;
  }

  return image_type;
#endif
}

#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
static U32 get_opt_type(U32 image_id)
{
  U32 opt = 0;
  switch(image_id)
  {

    case CAT_NANDID(PSI_IMAGE):
      opt = 0
#if defined(FEAT_NAND_PRGGEN) || (defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT))
        | CAT_NANDOPT(READONLYIMAGE) |CAT_NANDOPT(VERIFICATION_REQUIRED) 
#if !defined(FEAT_FTLE_PRGGEN)
        | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400)
#endif
#endif
         ;
    break;
    
    case CAT_NANDID(SLB_IMAGE):
        opt = CAT_NANDOPT(LOADMAP) | CAT_NANDOPT(VERIFICATION_REQUIRED) 
#if !defined(FEAT_FTLE_PRGGEN)
        | CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400
#endif
#if defined(FEAT_NAND_PRGGEN) || (defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT))
        | CAT_NANDOPT(READONLYIMAGE)
#endif
#if !defined(MM_CUST_SCF_SUPPORT)
        | (12 << 28)
#endif		
         ;
    break;

    case CAT_NANDID(CODE_IMAGE):
#if defined(FEAT_NAND_PRGGEN) || (defined(FEAT_FTLE_PRGGEN) && (MM_CUST_SCF_SUPPORT))
#if defined(FEAT_FOTA_COMBINED_PARTITION)	
      // Combines Option settings for both Modem SW and Paging Image.
      opt = (12 << 28) | CAT_NANDOPT(RO_BIT_CORRECTION_OK)|CAT_NANDOPT(LOADMAP)|CAT_NANDOPT(COPYTORAM)|CAT_NANDOPT(READONLYIMAGE)|CAT_NANDOPT(VERIFICATION_REQUIRED) | CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400) | CAT_NANDOPT(PAGING_DATA);
#else
      opt = CAT_NANDOPT(LOADMAP)|CAT_NANDOPT(COPYTORAM)|CAT_NANDOPT(READONLYIMAGE)|CAT_NANDOPT(VERIFICATION_REQUIRED) | CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS) 
#if !defined(FEAT_FTLE_PRGGEN)
| (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400)|CAT_NANDOPT(RO_BIT_CORRECTION_OK)
#endif
    ;
#endif	  
#else
      opt = CAT_NANDOPT(LOADMAP)|CAT_NANDOPT(COPYTORAM)|CAT_NANDOPT(READONLYIMAGE)|CAT_NANDOPT(VERIFICATION_REQUIRED) | CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS);
#endif	
    break;
    case CAT_NANDID(DSP_IMAGE):
    case CAT_NANDID(CDS_IMAGE):
#if defined(FEAT_NAND_PRGGEN)
      opt = CAT_NANDOPT(RO_BIT_CORRECTION_OK)|CAT_NANDOPT(LOADMAP) | CAT_NANDOPT(READONLYIMAGE)|CAT_NANDOPT(VERIFICATION_REQUIRED) | CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400) | (12 << 28);
#if defined(MM_CUST_SCF_SUPPORT) && defined(DEMAND_PAGING_ENABLED)
      opt = opt | CAT_NANDOPT(PAGING_DATA);      
#else
      opt = opt | CAT_NANDOPT(COPYTORAM); 
#endif  
#else
      opt = CAT_NANDOPT(LOADMAP)|CAT_NANDOPT(READONLYIMAGE)|CAT_NANDOPT(VERIFICATION_REQUIRED) | CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS);
#if defined(MM_CUST_SCF_SUPPORT) && defined(DEMAND_PAGING_ENABLED)
      opt = opt | CAT_NANDOPT(PAGING_DATA);      
#else
      opt = opt | CAT_NANDOPT(COPYTORAM); 
#endif

#endif
    break;
    case CAT_NANDID(CUSTOMIZATION_DATA):
#if defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT) /*only for emmc lepton */
	  opt = CAT_NANDOPT(READONLYIMAGE) | CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS) | CAT_NANDOPT(COPYTORAM) | CAT_NANDOPT(LOADMAP);   
#else
	  opt = CAT_NANDOPT(ALLOW_UTA_BYTE_IF_ACCESS) |  CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400) | CAT_NANDOPT(RO_BIT_CORRECTION_OK) | CAT_NANDOPT(READONLYIMAGE);
#endif
    break;  

    case CAT_NANDID(USBRO):
    case CAT_NANDID(CERTIFICATE_DATA):
      opt = CAT_NANDOPT(RO_BIT_CORRECTION_OK)|CAT_NANDOPT(LOADMAP) | CAT_NANDOPT(COPYTORAM)|CAT_NANDOPT(READONLYIMAGE)|CAT_NANDOPT(VERIFICATION_REQUIRED) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400);
#if defined(MM_CUST_SCF_SUPPORT)
      #if !defined(FEAT_FTLE_PRGGEN)
      opt = (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) | CAT_NANDOPT(PRESERVE_PARTITION) | CAT_NANDOPT(READONLYIMAGE));
      #else
          opt = CAT_NANDOPT(READONLYIMAGE);
      #endif
      #endif        
    break;

    case CAT_NANDID(PAGING_IMAGE):
     #if defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT)
      opt = (12 << 28) | CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS) | CAT_NANDOPT(PAGING_DATA) | CAT_NANDOPT(LOADMAP) | CAT_NANDOPT(READONLYIMAGE) | CAT_NANDOPT(VERIFICATION_REQUIRED);
     #else

      opt = (12 << 28) | CAT_NANDOPT(RO_BIT_CORRECTION_OK)|CAT_NANDOPT(LOADMAP)|CAT_NANDOPT(READONLYIMAGE)|CAT_NANDOPT(VERIFICATION_REQUIRED) | CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400) | CAT_NANDOPT(PAGING_DATA); 
     #endif
    break;
	
	/* Check for UA_IMAGE whether opt = (12 << 28);  *//* Fall through */// is required or not */
	
    case CAT_NANDID(UA_IMAGE):
#if defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT)/*only for emmc lepton support*/
         opt = CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS) | CAT_NANDOPT(LOADMAP) | CAT_NANDOPT(READONLYIMAGE);
#else
	  opt =  CAT_NANDOPT(RO_BIT_CORRECTION_OK)|CAT_NANDOPT(LOADMAP)|CAT_NANDOPT(READONLYIMAGE)|CAT_NANDOPT(VERIFICATION_REQUIRED) | CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400);
#endif
    break;
#if defined(FEAT_NAND_PRGGEN) && defined(FEAT_VMM_SUPPORT)
	case CAT_NANDID(ROOT_DISC):
    case CAT_NANDID(USER_DISC):
	case CAT_NANDID(CUSTOMIZATION_DISC):
      opt =  CAT_NANDOPT(FS_DEVICE_NOT_SYSTEM_DISK) | CAT_NANDOPT(FS_DEVICE_PLP_RUN_CHECK_DISK) | CAT_NANDOPT(FS_DEVICE_ACCESS_BOTH) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400);    
      // As per storage team, opt should be CAT_NANDOPT(FS_DEVICE_ACCESS_INTERNAL); But currently seeing some issues with this. Hence need further analysis.
    break;
#else
    case CAT_NANDID(ROOT_DISC):
    	#if !defined(FEAT_FAT_FS) //if customer file system enabled
    	  opt = (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400) | CAT_NANDOPT(ALLOW_UTA_BYTE_IF_ACCESS);
    	#else //Default for IMC file system
          #if defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT)
          opt =  CAT_NANDOPT(FS_DEVICE_PLP_ENABLE) | CAT_NANDOPT(FS_DEVICE_PLP_RUN_CHECK_DISK) | CAT_NANDOPT(FS_DEVICE_ACCESS_INTERNAL);
          #else
          opt =  CAT_NANDOPT(FS_DEVICE_SYSTEM_DISK) | CAT_NANDOPT(FS_DEVICE_PLP_RUN_CHECK_DISK) | CAT_NANDOPT(FS_DEVICE_ACCESS_INTERNAL) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400);    
          #endif
        #endif
    break;    
    case CAT_NANDID(USER_DISC):
      #if !defined(FEAT_FAT_FS) //if customer file system enabled
    	  opt = (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400) | CAT_NANDOPT(ALLOW_UTA_BYTE_IF_ACCESS);
    	#else //Default for IMC file system
        opt =  CAT_NANDOPT(FS_DEVICE_SYSTEM_DISK) | CAT_NANDOPT(FS_DEVICE_PLP_RUN_CHECK_DISK) | CAT_NANDOPT(FS_DEVICE_ACCESS_INTERNAL) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400);    
      #endif
    break;
#if !defined(FEAT_FTLE_PRGGEN)
    case CAT_NANDID(CUSTOMIZATION_DISC):
      opt =  CAT_NANDOPT(FS_DEVICE_NOT_SYSTEM_DISK) | CAT_NANDOPT(FS_DEVICE_PLP_RUN_CHECK_DISK) | CAT_NANDOPT(FS_DEVICE_ACCESS_BOTH) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400);
    break;
#endif /*!defined(FEAT_FTLE_PRGGEN)*/
#endif
    case CAT_NANDID(FTL_STARTUP_DISC):
#if defined(MM_CUST_SCF_SUPPORT) 
      opt = (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400);
#endif      
#if defined(FEAT_NAND_PRGGEN)
    case CAT_NANDID(RAW_NAND_IMAGE):
    case CAT_NANDID(PREFLASH):
#else
    case CAT_NANDID(CLONE_IMAGE):
    case CAT_NANDID(PREFLASH_IMAGE):
#endif
#if defined(FEAT_VMM_SUPPORT)
#if defined(FEAT_FTLE_PRGGEN)
    case CAT_NANDID(CUSTOMIZATION_DISC):
#endif
#endif
    case CAT_NANDID(DYN_SDS_IMAGE):
    case CAT_NANDID(PARTITION_TABLE):
      opt = opt | 0;
    break;

#if defined(FEAT_NAND_PRGGEN)
    case CAT_NANDID(BAD_BLOCK_TABLE):
      opt = CAT_NANDOPT(READONLYIMAGE);
    break;
#endif

    case CAT_NANDID(NVM_STATIC_CALIB):
    case CAT_NANDID(NVM_DYNAMIC):
#if defined(FEAT_OPM_MEM_SECTION_NVM_MIRROR)
      opt = CAT_NANDOPT(COPYTORAM);
#else
  #if !defined(FEAT_FTLE_PRGGEN)
      opt = CAT_NANDOPT(SHORT_TIMEOUT) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400);
    #if defined(MM_CUST_SCF_SUPPORT)      
      opt = opt | CAT_NANDOPT(PRESERVE_PARTITION);
    #endif        
  #else
      opt = 0;
  #endif
#endif  
    break;
    case CAT_NANDID(NVM_STATIC_FIX):
#if defined(FEAT_OPM_MEM_SECTION_NVM_MIRROR)
      opt = CAT_NANDOPT(COPYTORAM);
#else
  #if !defined(FEAT_FTLE_PRGGEN)
      opt = CAT_NANDOPT(SHORT_TIMEOUT) | (CAT_NANDOPT(ACCEPTABLE_BIT_ERRORS) & 0x400);
#if defined(MM_CUST_SCF_SUPPORT)      
      opt = opt | CAT_NANDOPT(WIPE_OUT_DATA) | CAT_NANDOPT(PRESERVE_PARTITION);
#endif      
  #else
      opt = 0;
  #endif
#endif     
    break;

#if defined(FEAT_VMM_SUPPORT)
	case CAT_NANDID(VLX):
	case CAT_NANDID(LINUX_KERNEL):
	case CAT_NANDID(INITRD):
	case CAT_NANDID(INITRD_RECOVERY):
	  opt = CAT_NANDOPT(RO_BIT_CORRECTION_OK)|CAT_NANDOPT(LOADMAP)|CAT_NANDOPT(COPYTORAM)|CAT_NANDOPT(VERIFICATION_REQUIRED);
	break;

#endif


    default:
      printf("Prg file Generation process error %s,%d\n",__FUNCTION__,__LINE__);
    break;
  }
  return opt;
}
#endif  /* NAND_PRESENT || FTLE_PRESENT || NANDDFMC_PRESENT */

#if defined(FEAT_NAND_PRGGEN)
#if 0
static U32 get_num_res_blk(U32 image_id,U32 size)
{
  U32 reserve_blk = 0;
  switch(image_id)
  {

    case CAT_NANDID(PSI_IMAGE):
    case CAT_NANDID(RAW_NAND_IMAGE):
    case CAT_NANDID(PREFLASH):
    case CAT_NANDID(DYN_SDS_IMAGE):
      reserve_blk = RESERVE_BLOCKS(size)*2; /* TODO: check if this is okay */
    break;

    case CAT_NANDID(CODE_IMAGE):
      reserve_blk = RESERVE_BLOCKS(size) + 5 + 5;
    break;

    case CAT_NANDID(DSP_IMAGE):
    case CAT_NANDID(CDS_IMAGE):
    case CAT_NANDID(USBRO):
    case CAT_NANDID(UA_IMAGE):
    case CAT_NANDID(PAGING_IMAGE):
    case CAT_NANDID(FTL_STARTUP_DISC):
    case CAT_NANDID(NVM_STATIC_CALIB):
    case CAT_NANDID(NVM_STATIC_FIX):
    case CAT_NANDID(NVM_DYNAMIC):
    case CAT_NANDID(BAD_BLOCK_TABLE):
    case CAT_NANDID(SLB_IMAGE):
      reserve_blk = RESERVE_BLOCKS(size) + 5;
    break;

    case CAT_NANDID(CUSTOMIZATION_DISC):
    case CAT_NANDID(CUSTOMIZATION_DATA):
    case CAT_NANDID(ROOT_DISC):
    case CAT_NANDID(USER_DISC):
      reserve_blk = RESERVE_BLOCKS(size)*1.5;
    break;

    default:
      printf("Prg file Generation process error %s,%d\n",__FUNCTION__,__LINE__);
    break;
  }
  return reserve_blk;
}
#endif 
#endif  /* NAND_PRESENT || NANDDFMC_PRESENT */

static void string_lower(char *dst, const char *src)
{
  while (1)
  {
    if (*src >= 'A' && *src <= 'Z')
      *dst = *src - 'A' + 'a';
    else
      *dst = *src;
    
    if (*src == 0)
      break;
      
    ++dst; ++src;    
  }
}

static void get_hex_details(char *filename,  U32 *s_addr,  U32 *length)
{
  char line[50];
  char line_prev[50];
  int no_sect=0;
  char g_hex_file[300];
  U32 offset=0;
  FILE *fp_hex;
 
  snprintf(g_hex_file, 300, "%s/%s", g_boot_file_loc, filename);
  fp_hex = fopen(g_hex_file,"r");
  if (fp_hex != NULL)
  {
    while (fgets(line, sizeof(line), fp_hex) != NULL )
    { 
      if((line[7] == '0') && (line[8] == '5'))
      {
        if(no_sect == 0)
        {
          sscanf(&line[9], "%8x", s_addr);
        }
        no_sect++;
      }
      else if((line[7] == '0') && (line[8] == '1'))
      {
        sscanf(&line_prev[3], "%4x", &offset);
        sscanf(&line_prev[1], "%2x", length);
      }   
      strncpy(line_prev,line,sizeof(line));  
    }
    *length += offset + ((no_sect-1)*0x10000);
    *length = ALIGN_2K(*length);
    fclose(fp_hex);
  }
  else
  {
    printf("Could not extract details from the hex file %s\n", filename);
    exit(3);
  }
}

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

    case MEMORYMAP_ELEM_TYPE:
    {
      MemoryMapStructType *p_struct = &((MemoryMapElementStructType *)struct_ptr)->Data;
      MemoryMapEntryStructType mmap[MAX_MAP_ENTRIES] = {0};
      MemoryMapEntryStructType *p_mmap = &mmap[0];
      U32 image_type[MAX_MAP_ENTRIES];
      U32 *p_image_type = &image_type[0];
      U32 i, j;	
      //U32 s_addr, len;

#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
      /* NAND or EMMC */
      //get_hex_details("psi_flash.hex", &s_addr, &len);
      p_mmap->Start  = MMAP_PSI_FLASH_BASE;
      p_mmap->Length = MMAP_PSI_FLASH_SIZE;
      p_mmap->Class  = BOOT_CORE_PSI_CLASS;
      p_mmap->Flags  = 0;      
      strncpy((char *)&p_mmap->FormatSpecify[0], "PSI_FLASH",strlen("PSI_FLASH")>15?15:strlen("PSI_FLASH"));
      p_mmap++;
      *p_image_type = CAT_NANDID(PSI_IMAGE);
      p_image_type++;
#else
#if !defined(FLASHLESS)
  #if defined(FEAT_SLB_IN_PCM)
      /* pcm - psi_flash and slb together */
      p_mmap->Start  = MMAP_SLB_BASE;
      p_mmap->Length = MMAP_SLB_SIZE;
      p_mmap->Class  = BOOT_CORE_PSI_CLASS;
      p_mmap->Flags  = 0;     
      strncpy((char *)&p_mmap->FormatSpecify[0], "CODE_IMAGE",strlen("CODE_IMAGE")>15?15:strlen("CODE_IMAGE"));
      p_mmap++;  
      p_mmap->Start  = MMAP_SLB_BASE + (MMAP_SLB_SIZE >> 1);
      p_mmap->Length = MMAP_SLB_SIZE >> 1;
      p_mmap->Class  = BOOT_CORE_SLB_CLASS;
      p_mmap->Flags  = 0;
      strncpy((char *)&p_mmap->FormatSpecify[0], "CODE_IMAGE",strlen("CODE_IMAGE")>15?15:strlen("CODE_IMAGE"));
      p_mmap++;

  #else
      /* pcm - psi_flash */
      p_mmap->Start  = MMAP_SLB_BASE;
      p_mmap->Length = MMAP_SLB_SIZE;
      p_mmap->Class  = BOOT_CORE_SLB_CLASS;
      p_mmap->Flags  = 0;      
      strncpy((char *)&p_mmap->FormatSpecify[0], "CODE_IMAGE",strlen("CODE_IMAGE")>15?15:strlen("CODE_IMAGE"));
      p_mmap++;
  #endif
  #endif //!defined(FLASHLESS)
#endif //defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)

#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
      //get_hex_details("slb.hex", &s_addr, &len);
      p_mmap->Start  = MMAP_SLB_BASE;
      p_mmap->Length = MMAP_SLB_SIZE; /*0x000F2000*/
      p_mmap->Class  = BOOT_CORE_SLB_CLASS;
      p_mmap->Flags  = 0;	
      strncpy((char *)&p_mmap->FormatSpecify[0], "SLB",strlen("SLB")>15?15:strlen("SLB"));
      p_mmap++;
      *p_image_type = CAT_NANDID(SLB_IMAGE);
      p_image_type++;
#endif

      /* below are just RAM copies */
      //get_hex_details("ebl.hex", &s_addr, &len);
      p_mmap->Start  = MMAP_EBL_BASE;
      p_mmap->Length = MMAP_EBL_SIZE;
      p_mmap->Class  = BOOT_CORE_EBL_CLASS;
      p_mmap->Flags  = 0;    
      strncpy((char *)&p_mmap->FormatSpecify[0], "EBL",strlen("EBL")>15?15:strlen("EBL"));
      p_mmap++;
#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
      *p_image_type = CAT_NANDID(EMPTY);  /* not important */
      p_image_type++;
#endif
      
      //get_hex_details("psi_ram.hex", &s_addr, &len);
      p_mmap->Start  = MMAP_PSI_RAM_BASE;
      p_mmap->Length = MMAP_PSI_RAM_SIZE;
      p_mmap->Class  = BOOT_CORE_PSI_CLASS;
      p_mmap->Flags  = 0;      
      strncpy((char *)&p_mmap->FormatSpecify[0], "PSI_RAM",strlen("PSI_RAM")>15?15:strlen("PSI_RAM"));
      p_mmap++;
#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
      *p_image_type = CAT_NANDID(EMPTY);  /* not important */
      p_image_type++;
#endif

      #define INCLUDED_IN_C_FILE
      #include <modem_sw.scf.in>

      p_struct->FlashStartAddr        = MMAP_MODEM_SW_FLASH_BASE;

      /* TODO: check how of the below are still relevent from XMM6180 projects... */
      p_struct->EepRevAddrInSw        = MMAP_MODEM_SW_STATIC_CFG_BASE;
      p_struct->EepRevAddrInEep       = MMAP_EEP_SECTION_BASE + 0x3FFA; /* TODO: remove this hardcoded value */
      p_struct->EepStartModeAddr      = MMAP_EEP_SECTION_BASE + 0x3FE0; /* TODO: remove this hardcoded value */
      p_struct->NormalModeValue       = 2;
      p_struct->TestModeValue         = 0;
      p_struct->SwVersionStringLocation = MMAP_MODEM_SW_SYS_VER_BASE;

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
#if defined(FEAT_FOTA_COMBINED_PARTITION)
          // Do not Merge if the Flags in memory map are not the same inspite of the FormatSpecify being the same.
          if ((strcmp((char *)&mmap[j].FormatSpecify[0], (char *)&mmap[i].FormatSpecify[0]) == 0)
		    && (mmap[i].Class == mmap[j].Class) && (mmap[i].Flags == mmap[j].Flags))
#else
          if ((strcmp((char *)&mmap[j].FormatSpecify[0], (char *)&mmap[i].FormatSpecify[0]) == 0)
		    && (mmap[i].Class == mmap[j].Class))
#endif		    
          {
            S32 delta = mmap[j].Start - (mmap[i].Start + mmap[i].Length);

            if (delta < 0) delta = -delta;

            mmap[i].Start  = mmap[i].Start < mmap[j].Start ? mmap[i].Start : mmap[j].Start;
            mmap[i].Length += delta + mmap[j].Length;

            printf("\tA[%08X]S[%08X] => [%08X][%08X]\n", mmap[j].Start, mmap[j].Length,
                                                         mmap[i].Start, mmap[i].Length);
            
            mmap[j].Class = NOT_USED;
            mmap[j].FormatSpecify[0] = 0;

#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
            image_type[j] = CAT_NANDID(EMPTY);
#endif
          }
        }
        printf("\n");
      }
      
      for (i = 0; i < MAX_MAP_ENTRIES; ++i)
      {
        if (mmap[i].Start == 0 || mmap[i].Class == NOT_USED)
          continue;
        mmap[i].Length = ((mmap[i].Length + MMAP_FLASH_BLOCK_SIZE - 1)/MMAP_FLASH_BLOCK_SIZE) * MMAP_FLASH_BLOCK_SIZE;
        for (j = i + 1; j < MAX_MAP_ENTRIES; j++)
        {
          if (mmap[j].Start == 0 || mmap[i].Class == NOT_USED)
            continue;
          if (((mmap[i].Class == BOOT_CORE_SLB_CLASS) && (mmap[j].Class == CODE_CLASS)) && (mmap[i].Start == mmap[j].Start))
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

      /* finally write the PRG entries */
      g_p_mmap = p_mmap = p_struct->Entry;
      memset(p_mmap, 0, MAX_MAP_ENTRIES * sizeof(mmap[0]));

      for (i = 0; i < MAX_MAP_ENTRIES; ++i)
      {
        if (mmap[i].Class != NOT_USED)
        {
          memcpy(p_mmap, &mmap[i], sizeof(mmap[i]));

          *g_p_image_type = image_type[i];
          
          printf("%-30s\t\tS[%08X] L[%08X] C[%X] F[%X]\n", p_mmap->FormatSpecify, p_mmap->Start, p_mmap->Length, p_mmap->Class, p_mmap->Flags);
          if (p_mmap->Class == CUST_CLASS)
          {
#if defined (FEAT_OPM_MEM_SECTION_CUST)
            if (strstr(p_mm_defs,"FEAT_OPM_MEM_SECTION_CUST") != NULL)
            {
              fprintf(fp, "%s --prg %s/modem_sw.prg "
                          "--output %s/%s --ebl %s/ebl.fls --psi %s/psi_ram.fls --tag %s %s/%s\n",
                          g_p_hex2fls_tool,
                          p_input_path,
                          p_output_path,
                          CUST_PACK_IMAGE_OUTPUT,
                          p_input_path, p_input_path,
                          p_mmap->FormatSpecify,						/* --tag */
                          p_global_path,                   
                          CUST_PACK_BIN_INPUT_PATH
                          );
            }
#endif
          }
#if defined(FEAT_OPM_MEM_SECTION_DISK) && !defined (FEAT_OPM_MEM_SECTION_NO_DFAT)
            else if (p_mmap->Class == ROOT_DISK_CLASS)
            {
              if (strstr(p_mm_defs,"FEAT_OPM_MEM_SECTION_DISK") != NULL)
              {
                fprintf(fp, "%s --prg %s/modem_sw.prg "
                            "--output %s/%s --ebl %s/ebl.fls --psi %s/psi_ram.fls --tag %s %s/%s\n",
                            g_p_hex2fls_tool,
                            p_input_path,
                            p_output_path,
                            ROOT_DISK_IMAGE_OUTPUT,                       
                            p_input_path, p_input_path,
                            p_mmap->FormatSpecify,                   /* --tag */
                            p_global_path,                   
                            ROOT_DISK_BIN_INPUT_PATH
                            );
              }
            }
#if !defined(MM_CUST_SCF_SUPPORT)
            else if (p_mmap->Class == USER_DISK_CLASS)
            {
              if (strstr(p_mm_defs,"FEAT_OPM_MEM_SECTION_DISK") != NULL)
              {
                fprintf(fp, "%s --prg %s/modem_sw.prg "
                            "--output %s/%s --ebl %s/ebl.fls --psi %s/psi_ram.fls --tag %s %s/%s\n",
                            g_p_hex2fls_tool,
                            p_input_path,
                            p_output_path,
                            USER_DISK_IMAGE_OUTPUT,                        
                            p_input_path, p_input_path,
                            p_mmap->FormatSpecify,                   /* --tag */
                            p_global_path,                   
                            USER_DISK_BIN_INPUT_PATH                        
                            );
              }
            }
            else if (p_mmap->Class == CUST_DISK_CLASS)
            {
              if (strstr(p_mm_defs,"FEAT_OPM_MEM_SECTION_DISK") != NULL)
              {
                fprintf(fp, "%s --prg %s/modem_sw.prg "
                            "--output %s/%s --ebl %s/ebl.fls --psi %s/psi_ram.fls --tag %s %s/%s\n",
                            g_p_hex2fls_tool,
                            p_input_path,
                            p_output_path,
                            CUST_DISK_IMAGE_OUTPUT,                        
                            p_input_path, p_input_path,
                            p_mmap->FormatSpecify,                   /* --tag */
                            p_global_path,          
                            CUST_DISK_BIN_INPUT_PATH
                            );
              }
            }
#endif			
#endif
            else
            {
              /* put an entry in hex2fls tool script */
              if (p_mmap->Class != BOOT_CORE_PSI_CLASS && p_mmap->Class != BOOT_CORE_SLB_CLASS && p_mmap->Class != BOOT_CORE_EBL_CLASS)
              {
                if (strcmp(p_int_stage,"CDS") == 0)
                {
                  if (p_mmap->FormatSpecify[0] == 'C' && p_mmap->FormatSpecify[1] == 'D' && p_mmap->FormatSpecify[2] == 'S')
                  {
                    fprintf(fp,
                                "%s --prg %s/modem_sw.prg "
                                "--output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --tag %s %s\n",
                                g_p_hex2fls_tool,
                                p_input_path,
                                p_output_path, p_mmap->FormatSpecify  /* --output */,
                                p_input_path, p_input_path,
                                p_mmap->FormatSpecify                   /* --tag */,
                                g_p_hex2fls_input_files
                                );
                  }
                }
                else
                {
                  if (p_mmap->FormatSpecify[0] == 'F' && p_mmap->FormatSpecify[1] == 'O' && p_mmap->FormatSpecify[2] == 'T' && p_mmap->FormatSpecify[3] == 'A')
                  {
                    if (strstr(p_mm_defs,"FEAT_OPM_MEM_SECTION_DISK") != NULL)
                    {
                      fprintf(fp,
                                  "%s --prg %s/modem_sw.prg "
                                  "--output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --tag %s %s\n",
                                  g_p_hex2fls_tool,
                                  p_input_path,
                                  p_output_path, p_mmap->FormatSpecify  /* --output */,
                                  p_input_path, p_input_path,
                                  p_mmap->FormatSpecify                   /* --tag */,
                                  g_p_hex2fls_input_files);
                    }
                  }
                  else
                  {
#if defined(SIGNING_FLS)
                    char converted_string[30];
#endif
                    if (!(p_mmap->FormatSpecify[0] == 'C' && p_mmap->FormatSpecify[1] == 'D' && p_mmap->FormatSpecify[2] == 'S')
                        && !(p_mmap->FormatSpecify[0] == 'E' && p_mmap->FormatSpecify[1] == 'M' && p_mmap->FormatSpecify[2] == 'P'
                        && p_mmap->FormatSpecify[3] == 'T' && p_mmap->FormatSpecify[4] == 'Y'))
                    {
/* psi_flash.hex is included unconditionally, except for XMM6360 */
#if !defined(FEAT_PSI_FLASH_EXCLUDED)
#if !defined(FEAT_NAND_PRGGEN) && !defined(FEAT_FTLE_PRGGEN)
                      if (p_mmap->FormatSpecify[0] == 'C' && p_mmap->FormatSpecify[1] == 'O' && p_mmap->FormatSpecify[2] == 'D' && p_mmap->FormatSpecify[3] == 'E' 
                          && p_mmap->FormatSpecify[4] == '_' && p_mmap->FormatSpecify[5] == 'I' && p_mmap->FormatSpecify[6] == 'M' && p_mmap->FormatSpecify[7] == 'A'
                          && p_mmap->FormatSpecify[8] == 'G' && p_mmap->FormatSpecify[9] == 'E')
                      {
                        fprintf(fp,
                                  "%s --prg %s/modem_sw.prg "
                                  "-s %s/psi_flash.xor_script.txt "
#if !defined(INI_FILE)
                                  "--output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --tag %s %s "
#else                             
                                  "--output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --zip %s/%s --tag %s %s "
#endif                            
#if defined(FEAT_SLB_IN_PCM)
                                  "%s/slb.hex "
#endif
                                  "%s/psi_flash.hex \n",
                                  g_p_hex2fls_tool,
                                  p_input_path,
                                  g_p_sign_script,
                                  p_output_path, p_mmap->FormatSpecify  /* --output */,
                                  p_input_path, p_input_path,
#if defined(INI_FILE)
                                  g_p_sign_script,
                                  g_ini_file,
#endif                            
                                  p_mmap->FormatSpecify                   /* --tag */,
                                  g_p_hex2fls_input_files,
#if defined(FEAT_SLB_IN_PCM)
                                  g_p_sign_script,
#endif
                                  g_p_sign_script);


#if defined(SIGNING_FLS)
                        string_lower(converted_string, (const char *)p_mmap->FormatSpecify);
                        fprintf(fp, "%s -s %s/scripts/%s_signing.txt "
                                    "%s/%s.fls "		  
                                    "-o %s/%s_signed.fls "
#if defined(INI_FILE)	      
                                    "--zip %s/%s \n"
#endif	      
                                    ,
                                    g_p_flssign_tool, g_p_sign_script, converted_string,
                                    p_output_path, p_mmap->FormatSpecify,
                                    p_output_path, p_mmap->FormatSpecify
#if defined(INI_FILE)	      
                                    ,g_p_sign_script, g_ini_file
#endif	      
                               );
#endif    
                      }
                      else
#endif // !defined(FEAT_NAND_PRGGEN) && !defined(FEAT_FTLE_PRGGEN)
#endif // !defined(FEAT_PSI_FLASH_EXCLUDED)
                      {
                      if (p_mmap->FormatSpecify[0] == 'C' && p_mmap->FormatSpecify[1] == 'O' && p_mmap->FormatSpecify[2] == 'D' && p_mmap->FormatSpecify[3] == 'E' 
                          && p_mmap->FormatSpecify[4] == '_' && p_mmap->FormatSpecify[5] == 'I' && p_mmap->FormatSpecify[6] == 'M' && p_mmap->FormatSpecify[7] == 'A'
                          && p_mmap->FormatSpecify[8] == 'G' && p_mmap->FormatSpecify[9] == 'E')
					  {
                        fprintf(fp,
                                  "%s --prg %s/modem_sw.prg "
#if !defined(INI_FILE)
                                  "--output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --tag %s %s\n",
#else                             
                                  "--output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --zip %s/%s --tag %s %s\n",
#endif                            
                                  g_p_hex2fls_tool,
                                  p_input_path,
                                  p_output_path, p_mmap->FormatSpecify  /* --output */,
                                  p_input_path, p_input_path,
#if defined(INI_FILE)
                                  g_p_sign_script,
                                  g_ini_file,
#endif                            
#if !defined(LEPTON_MSY)
				  p_mmap->FormatSpecify                   /* --tag */,
#else
				  "UPDAPP_V"                   /* --tag */,				  
#endif
                                  g_p_hex2fls_input_files);

#if defined(SIGNING_FLS)
                        string_lower(converted_string, (const char *)p_mmap->FormatSpecify);			
                        fprintf(fp, "%s -s %s/scripts/%s_signing.txt "
                                    "%s/%s.fls "		  
                                    "-o %s/%s_signed.fls "
#if defined(INI_FILE)	      
                                    "--zip %s/%s \n"
#endif	      
                                    ,
                                    g_p_flssign_tool, g_p_sign_script, converted_string,
                                    p_output_path, p_mmap->FormatSpecify,
                                    p_output_path, p_mmap->FormatSpecify
#if defined(INI_FILE)	      
                                    ,g_p_sign_script, g_ini_file
#endif	      
                               );
#endif

#if defined(FOTA_EMBEDDED_IN_FLS)
                        fprintf(fp,
                                  "%s --prg %s/modem_sw.prg "
#if !defined(INI_FILE)
                                  "--output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --tag %s %s\n",
#else                             
                                  "--output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --zip %s/%s --tag %s %s\n",
#endif                            
                                  g_p_hex2fls_tool,
                                  p_input_path,
                                  p_output_path, "FOTA_IMAGE"  /* --output */,
                                  p_input_path, p_input_path,
#if defined(INI_FILE)
                                  g_p_sign_script,
                                  g_ini_file,
#endif                           
				  "UPDAPP_V"                   /* --tag */,				  
                                  g_p_hex2fls_fota_input_files);

#if defined(SIGNING_FLS)
                        string_lower(converted_string, (const char *)p_mmap->FormatSpecify);			
                        fprintf(fp, "%s -s %s/scripts/%s_signing.txt "
                                    "%s/%s.fls "		  
                                    "-o %s/%s_signed.fls "
#if defined(INI_FILE)	      
                                    "--zip %s/%s \n"
#endif	      
                                    ,
                                    g_p_flssign_tool, g_p_sign_script, converted_string,
                                    p_output_path, "FOTA_IMAGE",
                                    p_output_path, "FOTA_IMAGE"
#if defined(INI_FILE)	      
                                    ,g_p_sign_script, g_ini_file
#endif	      
                               );
#endif			
#endif //FOTA_EMBEDDED_IN_FLS
 }
		      else
		      {		      
                        fprintf(fp,
                                  "%s --prg %s/modem_sw.prg "
#if !defined(INI_FILE)
                                  "--output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --tag %s %s\n",
#else                             
                                  "--output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --zip %s/%s --tag %s %s\n",
#endif                            
                                  g_p_hex2fls_tool,
                                  p_input_path,
                                  p_output_path, p_mmap->FormatSpecify  /* --output */,
                                  p_input_path, p_input_path,
#if defined(INI_FILE)
                                  g_p_sign_script,
                                  g_ini_file,
#endif                            
				  p_mmap->FormatSpecify                   /* --tag */,
                                  g_p_hex2fls_input_files);

#if defined(SIGNING_FLS)
                        string_lower(converted_string, (const char *)p_mmap->FormatSpecify);			
                        fprintf(fp, "%s -s %s/scripts/%s_signing.txt "
                                    "%s/%s.fls "		  
                                    "-o %s/%s_signed.fls "
#if defined(INI_FILE)	      
                                    "--zip %s/%s \n"
#endif	      
                                    ,
                                    g_p_flssign_tool, g_p_sign_script, converted_string,
                                    p_output_path, p_mmap->FormatSpecify,
                                    p_output_path, p_mmap->FormatSpecify
#if defined(INI_FILE)	      
                                    ,g_p_sign_script, g_ini_file
#endif	      
                               );
#endif
		      }    			
                      }
                    }
                  }
                }
              }
            }
/* TODO: Add the following in the above command once the xor script is corrected "-s %s/psi_flash.xor_script.txt  g_p_sign_script" in 439 */
          ++p_mmap;
          ++g_p_image_type;
        }
      }
    }
    break;

#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
    case NANDPARTITION_ELEM_TYPE:
    {
      NandPartitionDataStructType *p_struct = &((NandPartitionElementStructType *)struct_ptr)->Data;
      PartitionStructType         *p_partition = (PartitionStructType *)&(p_struct->Partition);
      PartitionEntryStructType    *p_part_entry = (PartitionEntryStructType *)&(p_partition->Partition);
      p_part_entry_front = p_part_entry;
      LoadAddrToPartitionTableStruct *p_load_t = (LoadAddrToPartitionTableStruct *)&(p_struct->LoadToPartition);
      struct partition_control_advanced_s
      {
	 U32 level;

	 U32 ImageType;
	 U32 Options;
	 U32 Reserved;
      } g_partition_control_advanced[MAX_MAP_ENTRIES] = {0};
#if defined(FEAT_FOTA_COMBINED_PARTITION)        
      U32 code_blk_size = 0;
#endif	  
      U32 i, j, r, p;
#if defined(FEAT_NAND_PRGGEN)
      U32 last_block = 0;
#elif defined(FEAT_FTLE_PRGGEN)
 #if !defined(MM_CUST_SCF_SUPPORT)
      U32 last_block[3 /* IT */] = {0x401 /* Image Type = 0, user partition */, /*If this is 400, PSI flashing fails as the partition validation fails bootcode - 
	  /mhw_drv_src/nv_memory_driver/ftle/src/ftle_startup.c at ftle_validate_partition_table(),line no 150 due to overlapping with the Partition table's end_block.*/
	                                  0     /* Image Type = 1, boot partition 1 */,
                                    0     /* Image Type = 2, boot partition 2 */};
 #else /*only for lepton emmc*/
       U32 last_block[3 /* IT */] = {0x0 /* Image Type = 0, user partition */, /*If this is 400, PSI flashing fails as the partition validation fails bootcode - 
	  /mhw_drv_src/nv_memory_driver/ftle/src/ftle_startup.c at ftle_validate_partition_table(),line no 150 due to overlapping with the Partition table's end_block.*/
	                                  0     /* Image Type = 1, boot partition 1 */,
                                    0     /* Image Type = 2, boot partition 2 */};
 #endif
 
#endif
      
      memset(p_partition, 0, sizeof(PartitionStructType));
      memset(p_partition->Partition, 0, sizeof(PartitionEntryStructType) * MAX_PARTITIONS);
      memset(p_load_t, 0, sizeof(LoadAddrToPartitionTableStruct));

#if defined(FEAT_FTLE_PRGGEN)
      p_partition->PartitionMarker = FTLE_PARTITION_MARKER;
#endif

      for (i = 0; i < MAX_MAP_ENTRIES; ++i)
      {
        if (g_p_mmap[i].Class == NOT_USED)
          break;
      }

      {
        MemoryMapEntryStructType *p_mmap = &g_p_mmap[i];
	struct partition_control_advanced_s *p_partition_control_advanced = &g_partition_control_advanced[i];
        U32 *p_image_type = &g_image_type[i];
        
#define MMAP_INCLUDE_PARTITION_INFO

      /* for ease, it is done here */
#define MMAP_ADD_PARTITION_EX(NAND_ID, BASE, SIZE, IMAGE_TYPE, OPTIONS, RESERVED, TAG, CLASS, FLAGS)	\
      p_mmap->Class = CLASS;			\
      p_mmap->Start = BASE;  			\
      p_mmap->Length = SIZE;  			\
      p_mmap->Flags = FLAGS;	        	\
      p_mmap++;					\
      *p_image_type = NAND_ID;  		\
      p_image_type++;				\
      p_partition_control_advanced->level = 1;	\
      p_partition_control_advanced->ImageType = IMAGE_TYPE; \
      p_partition_control_advanced->Options = OPTIONS;      \
      p_partition_control_advanced->Reserved = RESERVED;    \
      p_partition_control_advanced++;	
      		

#define MMAP_ADD_PARTITION(NAND_ID, base, size) \
      p_mmap->Class = CUST_CLASS; \
      p_mmap->Start = base;  \
      p_mmap->Length = (U32)size;  \
      p_mmap++; \
      p_partition_control_advanced++; \
      *p_image_type = CAT_NANDID(NAND_ID);  \
      p_image_type++;
      
        #include <mem_partition_cfg.h>
#if defined(MM_CUST_SCF_SUPPORT)	
	#include <modem_cust_internal.h>
#endif
      }

#undef MMAP_ADD_PARTITION
#undef MMAP_ADD_PARTITION_EX

#define MMAP_ADD_PARTITION(NAND_ID, base, size)
#define MMAP_ADD_PARTITION_EX(NAND_ID, BASE, SIZE, IMAGE_TYPE, OPTIONS, RESERVED, TAG, CLASS, FLAGS)



#if defined(FEAT_NAND_PRGGEN)||(defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT))
      /* Calculate the region size information */
      for (i = 0; i < MAX_MAP_ENTRIES; ++i)
      {      
        for (r = 0; r < sizeof(region)/sizeof(T_REGION); ++r)
        {
          for (p = 0; p < MAX_PARTITION_PER_REGION; ++p)
          {
            if (g_image_type[i] == region[r].nand_id[p])
            {
              region[r].Size += g_p_mmap[i].Length;
     
              g_partition_id_to_region_map[g_image_type[i]] = r;
            }
            if (((p+1) < MAX_PARTITION_PER_REGION) && (region[r].nand_id[p+1] == 0))
			{
              break;
            }	    
          }
        }
      }
#endif

#if defined(FEAT_FOTA_COMBINED_PARTITION)
      for (i = 0; i < MAX_MAP_ENTRIES; i++)
      {
        if(g_image_type[i] == CAT_NANDID(PAGING_IMAGE) || g_image_type[i] == CAT_NANDID(CODE_IMAGE))
          code_blk_size += g_p_mmap[i].Length; // Merge the size of both Code and Paging Image.
       
	    if(g_image_type[i] == CAT_NANDID(PAGING_IMAGE))
          g_image_type[i] = CAT_NANDID(EMPTY); // Making it empty removes the partition from the partition table.
      }
#endif

      for (i = 0, j = 0; i < MAX_MAP_ENTRIES; ++i)
      {
        U32 blk_size;

        if (g_p_mmap[i].Class == NOT_USED)
          break;

        if (g_image_type[i] == CAT_NANDID(EMPTY))
          continue;

        p_part_entry->ID = g_image_type[i];

	if (g_partition_control_advanced[i].level == 0 /* automatic */ )
	{
          p_part_entry->ImageType = get_part_type(p_part_entry->ID);
          
#if defined(FEAT_OPM_MEM_GPT_SUPPORT)
          p_part_entry->Options = get_options(p_part_entry->ID);
#else
          p_part_entry->Options = get_opt_type(p_part_entry->ID);
#endif

          /* If Byte or Block access is set then update the resrerved field with Load address */
	  if((p_part_entry->Options & CAT_NANDOPT(ALLOW_UTA_BLOCK_IF_ACCESS)) || 
             (p_part_entry->Options & CAT_NANDOPT(ALLOW_UTA_BYTE_IF_ACCESS)))
          {
	    if (g_image_type[i] != CAT_NANDID(UA_IMAGE))
            {
#if defined(MM_CUST_SCF_SUPPORT)
              if(g_image_type[i] == CAT_NANDID(CODE_IMAGE))
			    p_part_entry->Reserved = MCUSW_RESERVED_ADDRESS;
                          #if defined(DEMAND_PAGING_ENABLED)
			  else if(g_image_type[i] == CAT_NANDID(PAGING_IMAGE))
			    p_part_entry->Reserved = MCUSW_SWAP_RESERVED_ADDRESS;
                          #endif
			  else if(g_image_type[i] == CAT_NANDID(DSP_IMAGE))
			    p_part_entry->Reserved = AENEAS_RESERVED_ADDRESS;
			  else
			    p_part_entry->Reserved = g_p_mmap[i].Start; 
#else
              p_part_entry->Reserved = g_p_mmap[i].Start; 
#endif
            }
	    else
            {
              p_part_entry->Reserved = 0; /* TODO: FOTA needs other than Load address */
            }
          }
          else
          {
              p_part_entry->Reserved = 0;
          }
	}
	else if (g_partition_control_advanced[i].level == 1 /* slight manual control */ )
	{
          p_part_entry->ImageType = g_partition_control_advanced[i].ImageType;
          p_part_entry->Options = g_partition_control_advanced[i].Options;
          p_part_entry->Reserved = g_partition_control_advanced[i].Reserved;
	}

#if defined(FEAT_NAND_PRGGEN)
        /* Calculate the blocks needed for a paritition */
        for (r = 0; r < sizeof (region)/ sizeof (T_REGION); ++r)
        {
          region[r].StartBlock = (r == 0) ? (0) : (region[r-1].NextFreeBlock);
          region[r].EndBlock = region[r].StartBlock + NO_OF_BLOCKS(region[r].Size);

	  /* For Bootup reservoir sizes are added to actuals only */

          if((p_part_entry->ImageType == NAND_IT_BOOTUP_IMAGE) 
#if defined(MM_CUST_SCF_SUPPORT)
             || (p_part_entry->ID == CAT_NANDID(CERTIFICATE_DATA))
#endif
            )  
          {
            if(region[r].reservoir_p == 0)
            {
              region[r].EndBlock += region[r].reservoir_c;
            }
            else if(region[r].reservoir_c == 0)
            {
              region[r].EndBlock += RESERVE_BLOCKS(region[r].Size, region[r].reservoir_p);
            }
            region[r].StartReservoir = 0;
            region[r].EndReservoir = 0;	
            region[r].NextFreeBlock = region[r].EndBlock;	    
          }
	  else if(p_part_entry->ID == CAT_NANDID(BAD_BLOCK_TABLE))
	  {
            region[r].EndBlock += region[r].reservoir_c;
            region[r].StartReservoir = region[r].EndBlock;
            region[r].EndReservoir = region[r].EndBlock;	
            region[r].NextFreeBlock = region[r].EndBlock;
	  }
	  else if((p_part_entry->ID == CAT_NANDID(DYN_SDS_IMAGE)) || (p_part_entry->ID == CAT_NANDID(RAW_NAND_IMAGE)))
	  {
            region[r].StartBlock = 0;            
            region[r].EndBlock = 0;
            region[r].StartReservoir = 0;
            region[r].EndReservoir = 0;	    
	  }
          else 
          {
            region[r].StartReservoir = region[r].EndBlock;
            if(region[r].reservoir_p == 0)
            { 
              region[r].EndReservoir = region[r].StartReservoir + region[r].reservoir_c;
            }
            else if(region[r].reservoir_c == 0)
            {
              region[r].EndReservoir = region[r].StartReservoir + RESERVE_BLOCKS(region[r].Size, region[r].reservoir_p);		  
            }
            region[r].NextFreeBlock = region[r].EndReservoir;	  
          }
          if (region[r].NextFreeBlock > MAX_NUM_BLOCKS)
          {
            printf("Error: Insufficient space in storage nfb=0x%X mb=0x%X\n", region[r].NextFreeBlock, (U32)MAX_NUM_BLOCKS);
            exit(2);
          }
        }	
#endif

#if defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT)
        /* Calculate the blocks needed for a paritition */
        for (r = 0; r < sizeof (region)/ sizeof (T_REGION); ++r)
        {
          region[r].StartBlock = (r == 0) ? (0) : (region[r-1].NextFreeBlock);
          region[r].EndBlock = region[r].StartBlock + NO_OF_BLOCKS(region[r].Size);	
          
          if(p_part_entry->ID == CAT_NANDID(SLB_IMAGE) || p_part_entry->ID == CAT_NANDID(CODE_IMAGE))
          {
          	region[r].NextFreeBlock = 0;
          }
          else if(p_part_entry->ID == CAT_NANDID(PARTITION_TABLE))
          {
          	region[r].NextFreeBlock = 0x7FC;
          }	
          else if(p_part_entry->ID == CAT_NANDID(CERTIFICATE_DATA))
          {
          	region[r].NextFreeBlock = 0x700;
          }
          else
          {
          	region[r].NextFreeBlock = region[r].EndBlock;
          }
        }
#endif
        #if defined(FEAT_OPM_MEM_GPT_SUPPORT)
          /* PartitionEntryStructType.MaxBlocksInSubPartition == T_FTLE_PARTITION_ENTRY_STRUCT_TYPE.ftle_attribut */
          p_part_entry->MaxBlocksInSubPartition = get_ftle_attr(p_part_entry->ID);
        #else
          p_part_entry->MaxBlocksInSubPartition= 0;
        #endif
        

#if 0
        if (p_part_entry->Options & CAT_NANDOPT(LOADMAP))
        {
          g_p_mmap[i].Length += ALIGN_2K(sizeof(SecurityStructType)); 
        }
#endif
        blk_size = g_p_mmap[i].Length;

        #if defined(FEAT_FTLE_PRGGEN)
        switch (p_part_entry->ID)
        {
          case CAT_NANDID(PARTITION_TABLE):        
          #if defined(FEAT_OPM_MEM_GPT_SUPPORT)        
            /* IMC partition table needs to at the end of boot partition-0 */
            p_part_entry->StartBlock  = BOOT_PARTITION_SIZE_IN_BLOCKS;
            p_part_entry->StartBlock  -= (FTLE_PARTITION_TABLE_SIZE/ FTLE_DATA_SIZE_IN_BYTES);
	      #else
		  #if defined(MM_CUST_SCF_SUPPORT)
            p_part_entry->StartBlock  = 0x7fc;  /* only for emmc lepton*/
		  #else
		    p_part_entry->StartBlock  = 0;
		  #endif
          #endif
          break;
          
          #if defined(FEAT_OPM_MEM_GPT_SUPPORT)
          case CAT_NANDID(PRIMARY_GPT):      
            /* Primary GPT needs to be at beginning of user partition */
            p_part_entry->StartBlock  = 0;
          break;
          case CAT_NANDID(SECONDARY_GPT):      
            /* Secondary GPT needs to be at the end of the FLASH */
            p_part_entry->StartBlock  = MEM_PRG_USABLE_FLASH_SIZE;/* defined in mem_static_cfg.h */
            p_part_entry->StartBlock  -= NO_OF_BLOCKS(blk_size);
          break;          
          #endif

            
          default:
            #if defined(MM_CUST_SCF_SUPPORT)
              p_part_entry->StartBlock = region[g_partition_id_to_region_map[p_part_entry->ID]].StartBlock;
            #else
              p_part_entry->StartBlock  = last_block[p_part_entry->ImageType];
            #endif
          break;
        }
          #if defined(MM_CUST_SCF_SUPPORT)
            p_part_entry->EndBlock = region[g_partition_id_to_region_map[p_part_entry->ID]].EndBlock;
          #else
            p_part_entry->EndBlock = p_part_entry->StartBlock + NO_OF_BLOCKS(blk_size);
          #endif
        #if 0
        if(p_part_entry->EndBlock > MAX_NUM_BLOCKS)
        {
           printf("Error: Insufficient space in storage nfb=0x%X mb=0x%X\n", p_part_entry->EndBlock, MAX_NUM_BLOCKS);
           p_part_entry->EndBlock = MAX_NUM_BLOCKS;
        }  
        #endif
        #else
#if 0
        p_part_entry->StartBlock = last_block;
        switch (p_part_entry->ImageType)
        {
          case NAND_IT_BOOTUP_IMAGE:
          {
            if (p_part_entry->ID == CAT_NANDID(PSI_IMAGE))
            {
              p_part_entry->StartBlock = 0;
              p_part_entry->EndBlock = p_part_entry->StartBlock + NO_OF_BLOCKS(blk_size);
            }
            else
            {
              p_part_entry->EndBlock = p_part_entry->StartBlock + NO_OF_BLOCKS(blk_size)+ get_num_res_blk(p_part_entry->ID, blk_size);
            }
            p_part_entry->MaxBlocksInSubPartition = NO_OF_BLOCKS(blk_size);
            p_part_entry->EndReservoirBlock       = 0;
            p_part_entry->StartReservoirBlock     = 0;
          }
          break;

          case NAND_IT_FTL_IMAGE:
          case NAND_IT_FAT_IMAGE:
          case NAND_IT_RAW_IMAGE:
          {
            p_part_entry->EndBlock                = p_part_entry->StartBlock + NO_OF_BLOCKS(blk_size)+1;
            p_part_entry->MaxBlocksInSubPartition = NO_OF_BLOCKS(blk_size);
            p_part_entry->StartReservoirBlock     = p_part_entry->EndBlock;
            p_part_entry->EndReservoirBlock       = p_part_entry->EndBlock + get_num_res_blk(p_part_entry->ID, blk_size);
          }
          break;

          case NAND_IT_BBR_IMAGE:
          case NAND_IT_SDS_IMAGE:
          {
            p_part_entry->EndBlock                = p_part_entry->StartBlock + NO_OF_BLOCKS(blk_size);
            p_part_entry->MaxBlocksInSubPartition = NO_OF_BLOCKS(blk_size);
            p_part_entry->StartReservoirBlock     = p_part_entry->EndBlock;
            p_part_entry->EndReservoirBlock       = p_part_entry->EndBlock + get_num_res_blk(p_part_entry->ID, blk_size);
          }
          break;
          
          default:
            printf("*** Ignoring IT [%X] [%08X][%08X]\n", p_part_entry->ImageType, g_p_mmap[i].Start, g_p_mmap[i].Length);
          break;
        }
#else
        p_part_entry->StartBlock = region[g_partition_id_to_region_map[p_part_entry->ID]].StartBlock;
        p_part_entry->EndBlock = region[g_partition_id_to_region_map[p_part_entry->ID]].EndBlock;	
        p_part_entry->StartReservoirBlock = region[g_partition_id_to_region_map[p_part_entry->ID]].StartReservoir;
        p_part_entry->EndReservoirBlock = region[g_partition_id_to_region_map[p_part_entry->ID]].EndReservoir; 

#if defined(FEAT_FOTA_COMBINED_PARTITION)
	if(p_part_entry->ID == CAT_NANDID(CODE_IMAGE))
        {
           p_part_entry->MaxBlocksInSubPartition = NO_OF_BLOCKS(code_blk_size);
        }
	else
#endif		
          p_part_entry->MaxBlocksInSubPartition = NO_OF_BLOCKS(blk_size); 

//Work around start	
#if !defined(MM_CUST_SCF_SUPPORT)
        //For lepton, this workaround is not required. Please never ever enable this again.	
        {
           p_part_entry->MaxBlocksInSubPartition += 1;
        }
#endif		
//workaround end	

	

#endif 
#endif
#if defined (FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END)
        /* construct the priority queue based on End block number */
        pqinsert(&queue,p_part_entry); 
#endif
        printf("LA[%08X] ID[%02X] IT[%02X] S[%04X] E[%04X] MB[%04X] SR[%04X] ER[%04X] O[%08X] RES[%08X]\n",
               g_p_mmap[i].Start,
               p_part_entry->ID,
               p_part_entry->ImageType,
               p_part_entry->StartBlock,
               p_part_entry->EndBlock,
               p_part_entry->MaxBlocksInSubPartition,
               p_part_entry->StartReservoirBlock,
               p_part_entry->EndReservoirBlock,
               p_part_entry->Options,
               p_part_entry->Reserved);

        if (p_part_entry->EndReservoirBlock > MAX_NUM_BLOCKS)
        {
          printf("Error: Insufficient space in storage erb=0x%X, mb=0x%X\n", p_part_entry->EndReservoirBlock, (U32)MAX_NUM_BLOCKS);
          exit(2);
        }

        p_load_t->Table[j] = g_p_mmap[i].Start;

#if defined(FEAT_NAND_PRGGEN)
        if (p_part_entry->EndReservoirBlock > 0)
          last_block = p_part_entry->EndReservoirBlock;
        else
          last_block = p_part_entry->EndBlock;
#elif defined(FEAT_FTLE_PRGGEN)
          #if defined(FEAT_OPM_MEM_GPT_SUPPORT)
          if((p_part_entry->ID != CAT_NANDID(PRIMARY_GPT)) && 
            (p_part_entry->ID != CAT_NANDID(SECONDARY_GPT)))
          {
            last_block[p_part_entry->ImageType] = p_part_entry->EndBlock;
          }
          #else
            last_block[p_part_entry->ImageType] = p_part_entry->EndBlock;
          #endif
#endif

        ++p_part_entry;
        ++j;
        if (j == MAX_PARTITIONS )
        {
          printf("Error: reached Max partitions=0x%X", MAX_PARTITIONS);
          exit(2);  
        }		
      }
    }
	

    /* priority queue has been constructed. Now update it according to
     * PRG partitions relative to end of physical eMMC partitions
     * id: partition id, which will have dynamic partition size.
     * It will update iff FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END is defined.
     */
	 
#if defined (FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END)
    if (0 < update_block_number(FEAT_OPM_MEM_DYNAMIC_PARTITION_ID, FEAT_OPM_MEM_PARTITION_ID2))
    {
      printf("\nPRG partitions are relative to end of physical eMMC partitions in signed magnitude representation \n");
    }
#endif


#define MMAP_INCLUDE_MEMORY_MAP_INFO    
    /* memory map entries required for flash file generation but do not need corresponding entry in partitions */
    { 
      int i;
      for (i = 0; i < MAX_MAP_ENTRIES; ++i)
      {
        if(g_p_mmap[i].FormatSpecify[0] == 'E' && g_p_mmap[i].FormatSpecify[1] == 'M' && g_p_mmap[i].FormatSpecify[2] == 'P' && g_p_mmap[i].FormatSpecify[3] == 'T' && g_p_mmap[i].FormatSpecify[4] == 'Y')		
          break;
      }
      if(i < MAX_MAP_ENTRIES)
      {
        MemoryMapEntryStructType *p_mmap = &g_p_mmap[i+1];

#define MMAP_MEMORY_MAP_DUMMY_ENTRY(_PRG_NAME_, _PRG_CLASS_, _BASE_, _SIZE_)  \
        p_mmap->Start  = _BASE_;  \
        p_mmap->Length = _SIZE_;  \
        p_mmap->Class  = _PRG_CLASS_;  \
        p_mmap->Flags  = 0; \
        strncpy((char *)&p_mmap->FormatSpecify[0], #_PRG_NAME_,strlen(#_PRG_NAME_)>15?15:strlen(#_PRG_NAME_)); \
        p_mmap++;

#if defined(MM_CUST_SCF_SUPPORT)
	#include <modem_cust_internal.h>
#endif
      }
      else
      {
	 printf("PRGGEN ERROR: No room for new entry in emmory map of PRG!!!\n");
      }
    }
    break;
#endif  /* NAND_ONLY */
  }
}

int main(int argc, char **argv)
{
  U32 ret;
  char platform_name_lwr[128];
  char modem_sw_prg[256];
  char *p_board_id;
  FILE *fp_chk = NULL;

#if defined(SIGNING_FLS)
  #if defined(FOTA_EMBEDDED_IN_FLS)  
    #define NO_OF_ARGS 9
  #else
    #define NO_OF_ARGS 8
  #endif  
#else
#if defined(FOTA_EMBEDDED_IN_FLS)  
    #define NO_OF_ARGS 8
  #else
    #define NO_OF_ARGS 7
  #endif 
#endif  

#if defined (FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END)
  /* Initialize the priority queue */
  pqinit(&queue);
#endif
  if (argc < (NO_OF_ARGS+1))
  {
    printf("Insufficient arguments\n");
    for(int i=1; i<=NO_OF_ARGS; i++)
      printf("arg[%d] = %s\n",i,argv[i]);
    exit(2);
  }

  g_p_hex2fls_tool = argv[1];
  p_global_path = argv[2];
  p_board_id = argv[3]; 
  p_input_path = argv[4];
  p_int_stage = argv[5];
  p_mm_defs = argv[6];
#if defined(SIGNING_FLS)
  g_p_flssign_tool = argv[7];
#endif  
  

  printf("\nPrggen-Start\n");

  printf("%s\n", p_mm_defs);

#if defined(FOTA_EMBEDDED_IN_FLS)  
  g_p_hex2fls_fota_input_files = argv[8];
  printf("g_p_hex2fls_fota_input_files = %s\n",g_p_hex2fls_fota_input_files);
#endif

  for(int i=NO_OF_ARGS; argv[i] != NULL; i++)
  {
    snprintf(g_p_hex2fls_input_files, sizeof(g_p_hex2fls_input_files), "%s %s",g_p_hex2fls_input_files,argv[i]);
    printf("i = %d, g_p_hex2fls_input_files = %s\n",i,g_p_hex2fls_input_files);
  }
  
  g_p_sign_script = &g_boot_file_loc[0];
  g_p_scf_tools_obj = &g_scf_tools_obj[0];
  
  string_lower(platform_name_lwr, PLATFORM_PROJECTNAME);
  
  #if defined(BOOTLDR_DIR)
    sprintf(g_boot_file_loc, "%s", BOOTLDR_DIR);
  #else
    sprintf(g_boot_file_loc, "%s/dwdtools/boot/%s/%s/%s", p_global_path, platform_name_lwr, BOOT_TYPE, p_board_id);
  #endif
  
  snprintf(g_scf_tools_obj, 256, "%s/mhw_drv_src/memory_management/mem/scatterfile/tools/obj", p_global_path);
  snprintf(p_output_path, 256, "%s/output",p_input_path);
  
  printf("prggen v TODO:\n");
  printf("Memory Management Team\n");
  printf("Hex2Fls     : %s\n", g_p_hex2fls_tool);
  printf("Boot Loader : %s\n", g_boot_file_loc);
  
  snprintf(modem_sw_prg, 256, "%s/createfls", p_input_path);
  fp = fopen(modem_sw_prg, "w");  /* creates, createfls shell script */
  if (fp == NULL)
  {
    exit(2);
  }    

  snprintf(modem_sw_prg, 256, "%s/modem_sw.prg", p_input_path);
    
  /* psi_ram.fls */
  fprintf(fp, "%s --prg %s/modem_sw.prg "
#if defined(BOOT_USE_VERSIONINFO)
              "--version %s/psi_ram.version.txt "
#endif
              "-s %s/psi_ram.xor_script.txt --output %s/%s --tag %s %s/psi_ram.hex\n",
              g_p_hex2fls_tool,
              p_input_path,
#if defined(BOOT_USE_VERSIONINFO)
              g_p_sign_script,					/* check if rename of this variable is needed, this is boot loader path */
#endif
              g_p_sign_script,                  /* -s signing script */
              p_input_path, "psi_ram.fls"    /* --output */,
              "PSI_RAM"                         /* --tag */,
              g_p_sign_script);
           
#if defined(SIGNING_FLS)
  /* psi_ram_signed.fls */
  fprintf(fp, "%s -s %s/scripts/psi_ram.signing_script.txt "
              "%s/%s "		  
              "-o %s/%s \n",
              g_p_flssign_tool, g_p_sign_script,
	      p_input_path, "psi_ram.fls",
	      p_input_path, "psi_ram_signed.fls");
#endif  

  /* ebl.fls */
  fprintf(fp, "%s --prg %s/modem_sw.prg "
#if defined(BOOT_USE_VERSIONINFO)
              "--version %s/ebl.version.txt "
#endif
              "--output %s/%s --psi %s/psi_ram.fls --tag %s %s/ebl.hex\n",
              g_p_hex2fls_tool,
              p_input_path,
#if defined(BOOT_USE_VERSIONINFO)
              g_p_sign_script,					/* check if rename of this variable is needed, this is boot loader path */
#endif
              p_input_path, "ebl.fls"        /* --output */,
              p_input_path,
              "EBL"                             /* --tag */,
              g_p_sign_script);

#if defined(SIGNING_FLS)
  /* ebl_signed.fls */
  fprintf(fp, "%s -s %s/scripts/ebl.signing_script.txt "
              "%s/%s "		  
              "-o %s/%s \n",
              g_p_flssign_tool, g_p_sign_script,
	      p_input_path, "ebl.fls",
	      p_input_path, "ebl_signed.fls");
#endif    

#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
  /* psi_flash.fls */
  fprintf(fp, "%s --prg %s/modem_sw.prg "
#if defined(BOOT_USE_VERSIONINFO)
              "--version %s/psi_flash.version.txt "
#endif
              "-s %s/psi_flash.xor_script.txt --output %s/%s --psi %s/psi_ram.fls --ebl %s/ebl.fls --tag %s %s/psi_flash.hex\n",
              g_p_hex2fls_tool,
              p_input_path,
#if defined(BOOT_USE_VERSIONINFO)
              g_p_sign_script,					/* check if rename of this variable is needed, this is boot loader path */
#endif
              g_p_sign_script,                  /* -s signing script */
              p_output_path, "psi_flash.fls"    /* --output */,
              p_input_path, p_input_path,
              "PSI_FLASH"                       /* --tag */,
              g_p_sign_script);
#endif

#if defined(SIGNING_FLS)
  /* psi_flash_signed.fls */
  fprintf(fp, "%s -s %s/scripts/psi_flash.signing_script.txt "
              "%s/%s "		  
              "-o %s/%s \n",
              g_p_flssign_tool, g_p_sign_script,
	      p_output_path, "psi_flash.fls",
	      p_output_path, "psi_flash_signed.fls");
#endif  

#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN) 
  /* slb.fls */
  fprintf(fp, "%s --prg %s/modem_sw.prg "
#if defined(BOOT_USE_VERSIONINFO)
              "--version %s/slb.version.txt "
#endif
              "--output %s/%s --psi %s/psi_ram.fls --ebl %s/ebl.fls --tag %s %s/slb.hex\n",
              g_p_hex2fls_tool,
              p_input_path,
#if defined(BOOT_USE_VERSIONINFO)
              g_p_sign_script,					/* check if rename of this variable is needed, this is boot loader path */
#endif
              p_output_path, "slb.fls"        /* --output */,
              p_input_path, p_input_path, 
              "SLB"                       /* --tag */,
              g_p_sign_script);
#endif

#if defined(SIGNING_FLS)
  /* slb_signed.fls */
  fprintf(fp, "%s -s %s/scripts/slb.signing_script.txt "
              "%s/%s "		  
              "-o %s/%s \n",
              g_p_flssign_tool, g_p_sign_script,
	      p_output_path, "slb.fls",
	      p_output_path, "slb_signed.fls");
#endif    

  CurrentFile.CleanUp();
  CurrentFile.CallingTool = MAKEPRG_TOOL_ELEM_TYPE;
  CurrentFile.SetCallBackFunc(NewElementFunction);
  CurrentFile.AddingNewElement = true;
  CurrentFile.CreateElement(GENERIC_HEADER_ELEM_TYPE);
  for (int i = PRGHANDLER_TOOL_ELEM_TYPE; i < NUM_ELEM_TYPES; i++)
  {
    if (!CurrentFile.IsDynamicElement(i))
      CurrentFile.CreateElement(i);
  }

  CurrentFile.TamperToolDataBeforeWrite(MAKEPRG_TOOL_REQ_VER);

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

}

