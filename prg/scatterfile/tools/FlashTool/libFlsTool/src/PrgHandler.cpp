/*
 *       Copyright (C) 2011-2014 Intel Mobile Communications GmbH
 *
 *            Sec Class: Intel Confidential (IC)
 *
 *
 * This document contains proprietary information belonging to IMC.
 * Denmark A/S. Passing on and copying of this document, use and communication
 * of its contents is not permitted without prior written authorisation.
 *
 * Description: Class substitution for Borlands C++ TMemoryStream
 *
 *
 * Revision Information:
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/PrgHandler.cpp $
 *   Resonsible:
 *   Comment:
 *
 */
#include "FileIO.h"
#include "PrgHandler.h"

#include <memory>

/*
 * Defining the endianness at run time
 */
static int x=1;
static endiann_t mach_endian = (*(char*)&(x)==1) ? LITTLE_ENDIAN_MACHINE : BIG_ENDIAN_MACHINE;
endiann_t GetEndian() { return (*(char*)&(x)==1) ? LITTLE_ENDIAN_MACHINE : BIG_ENDIAN_MACHINE; }

bool CalledByWrite = false;
uint32_t file_offset=0;

void print_bytes(char* name, unsigned char *buf, int num)
{
  int i;

  printf("%s :",name);
  for(i=0;i<num;i++)
    printf("%02x ",buf[i]);
  printf("\n");
}

void swap_endian_long(char *input,int size)
{
  int i;
  char temp[4];
  if(size%4 != 0)
  {
    printf("swap_endian_long: Size not modulo zero. Cannot change endian\n");
    //return false;
  }

  for(i=0; i < size/4; i++)
  {
    memcpy(temp,&input[i*4],4);
    input[i*4]=temp[3];
    input[i*4 + 1]=temp[2];
    input[i*4 + 2]=temp[1];
    input[i*4 + 3]=temp[0];
  }

  //return true;
}

void swap_endian_short(char *input,int size)
{
  int i;
  char temp[2];
  if(size%2 != 0)
  {
    printf("swap_endian_short: Size not modulo zero. Cannot change endian\n");
    //return false;
  }

  for(i=0; i < size/2; i++)
  {
    memcpy(temp,&input[i*2],2);
    input[i*2]=temp[1];
    input[i*2 + 1]=temp[0];
  }

  //return true;
}

static void LongCpy(char *src,uint32_t value)
{
  char* valueptr = (char*) &value;

  if(mach_endian == BIG_ENDIAN_MACHINE)
  {
    uint32_t tempvalue = value;
    char *tempptr = (char*) &tempvalue;

    valueptr[0]=tempptr[3];
    valueptr[1]=tempptr[2];
    valueptr[2]=tempptr[1];
    valueptr[3]=tempptr[0];
  }
  memcpy(src,valueptr,sizeof(value));
}
#if 0
static uint32_t GetLongFromChar(char *src)
{
  uint32_t value=*((uint32_t*)src);
  if(mach_endian == BIG_ENDIAN_MACHINE)
  {
    uint32_t tempvalue=value;
    char *tempptr = (char*) &tempvalue;

    tempptr[0]=src[3];
    tempptr[1]=src[2];
    tempptr[2]=src[1];
    tempptr[3]=src[0];

    value = tempvalue;
  }
  return value;
}
#endif

uint32_t GetLongFromLong(uint32_t value)
{
  if(mach_endian == BIG_ENDIAN_MACHINE)
  {
    uint32_t tempvalue=value;
    char *tempptr = (char*) &tempvalue;
    char *src = (char*) &value;

    tempptr[0]=src[3];
    tempptr[1]=src[2];
    tempptr[2]=src[1];
    tempptr[3]=src[0];

    value = tempvalue;
  }
  return value;
}

//############################## PRG HANDLER INTERFACE FUNCTIONS ######################
#ifdef __cplusplus
extern "C" {
#endif

uint32_t PRGH_GetLongFromLong(uint32_t value)
{
  return GetLongFromLong(value);
}

endiann_t PRGH_GetEndian(char *Memory)
{
  GenericHeaderElementStructType *G;
  uint32_t temp_type;

  G = (GenericHeaderElementStructType*)Memory;
  temp_type = G->Header.Type;
  swap_endian_long((char *) &temp_type, sizeof(temp_type));

  // First determine the endianess of the machine
  if(G->Header.Type == GENERIC_HEADER_ELEM_TYPE) // check if little endian
    return LITTLE_ENDIAN_MACHINE;
  else  if(temp_type == GENERIC_HEADER_ELEM_TYPE)
    return BIG_ENDIAN_MACHINE;
  else
    return UNKNOWN_ENDIAN_MACHINE;
}

uint32_t PRGH_FileCheck(char *Memory)
{
  GenericHeaderElementStructType *G;

  //Store the endian
  mach_endian=PRGH_GetEndian(Memory);

  G = (GenericHeaderElementStructType*)Memory;
  if( GetLongFromLong(G->Header.Type) != GENERIC_HEADER_ELEM_TYPE ||
    GetLongFromLong(G->Header.Size) != sizeof(GenericHeaderElementStructType) ||
    GetLongFromLong(G->Header.UID) != GENERIC_UID
    )
    return INVALID_FILE_TYPE;

  if( (GetLongFromLong(G->Data.FileType) & FILE_TYPE_MASK) < PRG_FILE_TYPE ||
    (GetLongFromLong(G->Data.FileType) & FILE_TYPE_MASK) >= NUM_FILE_TYPES ||
    GetLongFromLong(G->Data.Marker) != PRG_MAGIC_NUMBER
    )
    return INVALID_FILE_TYPE;
  else
    return GetLongFromLong(G->Data.FileType);
}

uint32_t PRGH_VersionCheck(char *Memory,uint32_t ToolElementType, uint32_t CurrentVersion)
{
  uint32_t elem_index;
  uint32_t RequiredVersion  = INVALID_TOOL_VERSION;
  ToolVersionElementStructType *T;

  if(!PRGH_GetElementIndex(Memory,ToolElementType,GENERIC_UID,&elem_index)) // Search on Generic UID
    return RequiredVersion;

  T = (ToolVersionElementStructType *)(Memory+elem_index);

  if(GetLongFromLong(T->Header.Size) != sizeof(ToolVersionElementStructType))
    return RequiredVersion;

#if 0
  // Removed to allow a tool to open the file if Tool Version and Required Version have same major version, else fail.
  if(CurrentVersion < GetLongFromLong(T->Data.Required) ||
    ((CurrentVersion >> 16) & 0x0000FFFF) > ((GetLongFromLong(T->Data.Required) >> 16) & 0x0000FFFF)
    )
#endif
    // So now a M.x Tool can open a M.y file for all values to x and y
    if(((CurrentVersion >> 16) & 0x0000FFFF) != ((GetLongFromLong(T->Data.Required) >> 16) & 0x0000FFFF))
      RequiredVersion = GetLongFromLong(T->Data.Required);
    else
      RequiredVersion = 0;

  return RequiredVersion;
}

uint8_t PRGH_GetElementIndex(char *Memory,uint32_t ElementType, uint32_t UID, uint32_t* element_index)
{
  int index = 0;
  bool found=false;
  char *temp_mem;
  bool end_marker=false;
  ElementHeaderStructType *E;

  *element_index = INVALID_ELEMENT_INDEX;
  temp_mem = Memory;
  while(!end_marker && temp_mem)
  {
    E = (ElementHeaderStructType*)temp_mem;
    // find if we have reached the last element
    if(GetLongFromLong(E->Type) == END_MARKER_ELEM_TYPE)
      end_marker=true;
    // Check the element type and Element UID
    if( (GetLongFromLong(E->Type) == ElementType) && (GetLongFromLong(E->UID) == UID)  )
    {
      *element_index=index;
      found = true;
      break;
    }
    else
    {
      if(E->Size == 0) /* Is contents bad? endless loops */
        break;
      else
      {
        index += GetLongFromLong(E->Size); // Add the size of the element to get next element index
        temp_mem = Memory+index;
      }
    }
  }

  return found;
}

uint8_t PRGH_GetNextElementIndex(char *Memory,uint32_t* current_element_index)
{
  int index = *current_element_index;
  ElementHeaderStructType *E;

  *current_element_index = INVALID_ELEMENT_INDEX;
  E = (ElementHeaderStructType *)(Memory+index);
  // find if we are at the last element
  if(GetLongFromLong(E->Type) == END_MARKER_ELEM_TYPE)
    return false;
  index += GetLongFromLong(E->Size); // Add the size of the element to get next element index
  *current_element_index=index;

  return true;
}

uint32_t PRGH_GetPrgHandlerVersion()
{
  return PRG_HANDLER_VERSION;
}


uint32_t PRGH_FileWriteElement(FILE *fout,IFWD_MemoryStream *Output,uint32_t Type,uint32_t UID,char* PayloadPtr,uint32_t PayloadSize)
{
  uint32_t elem_size = sizeof(ElementHeaderStructType) + PayloadSize;
  //uint32_t new_size = Output->Size + elem_size;
  uint32_t new_size = Output->OccupiedSize + elem_size;
  int index =0;
  char* ElemStartMem;
  char* data_ptr=NULL;
  uint32_t data_length=0;
  bool dynamic_elem=true;
  //int orig_size = Output->Size;
  int orig_size = Output->OccupiedSize;

  Output->OccupiedSize = new_size;
  ElemStartMem = (char*)Output->Memory+orig_size;

  if(mach_endian == BIG_ENDIAN_MACHINE)
  {
    CalledByWrite = true;
    PRGH_SwapElementEndian(PayloadPtr -sizeof(ElementHeaderStructType), Type);
    CalledByWrite = false;
  }

  new_size = GetLongFromLong(new_size);
  // Special handling for dynamic data elements
  switch(Type)
  {
    case DOWNLOADDATA_ELEM_TYPE:
      data_ptr = (char*)((DownloadDataStructType *)PayloadPtr)->Data ;
      ((DownloadDataStructType *)PayloadPtr)->Data=0;
      data_length = ((DownloadDataStructType *)PayloadPtr)->DataLength;
      ((DownloadDataStructType *)PayloadPtr)->DataOffset = new_size;
      break;
    case TOC_ELEM_TYPE:
      data_ptr =(char*) ((TocStructType *)PayloadPtr)->Data ;
      ((TocStructType *)PayloadPtr)->Data = 0;
      data_length = GetLongFromLong(((TocStructType *)PayloadPtr)->NoOfEntries) * sizeof(TocEntryStructType);
      ((TocStructType *)PayloadPtr)->DataOffset = new_size;
      break;
    case GENERIC_VERSION_ELEM_TYPE:
    case INJECTED_PSI_VERSION_ELEM_TYPE:
    case INJECTED_EBL_VERSION_ELEM_TYPE:
      data_ptr = (char*)((VersionDataStructType *)PayloadPtr)->Data ;
      ((VersionDataStructType *)PayloadPtr)->Data=0;
      data_length = ((VersionDataStructType *)PayloadPtr)->DataLength;
      ((VersionDataStructType *)PayloadPtr)->DataOffset = new_size;
      break;
    case INJECTED_PSI_ELEM_TYPE:
    case INJECTED_EBL_ELEM_TYPE:
    case INJECTED_INI_ELEM_TYPE:
    case INJECTED_PDF_ELEM_TYPE:
      data_ptr = (char*)((InjectionDataStructType *)PayloadPtr)->Data ;
      ((InjectionDataStructType *)PayloadPtr)->Data=0;
      data_length = ((InjectionDataStructType *)PayloadPtr)->DataLength;
      ((InjectionDataStructType *)PayloadPtr)->DataOffset = new_size;
      break;
    case ROOT_DISK_ELEM_TYPE:
    case CUST_DISK_ELEM_TYPE:
    case USER_DISK_ELEM_TYPE:
      data_ptr = (char*)((StorageDataStructType *)PayloadPtr)->Data ;
      ((StorageDataStructType *)PayloadPtr)->Data=0;
      data_length = ((StorageDataStructType *)PayloadPtr)->DataLength;
      ((StorageDataStructType *)PayloadPtr)->DataOffset = new_size;
      break;
    default:
      dynamic_elem = false;
      break;
  }

  if(Type != TOC_ELEM_TYPE) // Incase of TOC element, this already converted from BIG ENDIAN in above switch statement
    data_length = GetLongFromLong(data_length);

   if(fout!=NULL)
   {
     LongCpy((char*)&Type,Type);
     fwrite(&Type,1,4,fout);
     if(dynamic_elem)
     {
       elem_size+=data_length;
       LongCpy((char*)&elem_size,elem_size);
       fwrite(&elem_size,1,4,fout);
     }
     else
     {
       LongCpy((char*)&elem_size,elem_size);
       fwrite(&elem_size,1,4,fout);
     }
     LongCpy((char*)&UID,UID);
     fwrite(&UID,1,4,fout);
     fwrite(PayloadPtr,1,PayloadSize,fout);
     if(dynamic_elem)
     {
       Output->OccupiedSize += data_length;
       fwrite(data_ptr,1,data_length,fout);
     }
  }
  else
  {
     LongCpy(ElemStartMem + index,Type);
     index += sizeof(Type);

     if(dynamic_elem)
       LongCpy(ElemStartMem + index,elem_size+data_length);
     else
       LongCpy(ElemStartMem + index,elem_size);
     index += sizeof(elem_size);

     LongCpy(ElemStartMem + index,UID);
     index += sizeof(UID);

     memcpy(ElemStartMem + index,PayloadPtr,PayloadSize);
     index += PayloadSize;

     if(dynamic_elem)
     {
       Output->OccupiedSize += data_length;
       memcpy( (char*)Output->Memory+orig_size+index, data_ptr, data_length);
     }
  }

  return PRG_OK;
}

uint32_t PRGH_FindElementDataSize(uint32_t Type,char* PayloadPtr)
{
  uint32_t data_length=0;

  switch(Type)
  {
    case DOWNLOADDATA_ELEM_TYPE:
      data_length = ((DownloadDataStructType *)PayloadPtr)->DataLength;
      break;
    case TOC_ELEM_TYPE:
      data_length = ((TocStructType *)PayloadPtr)->NoOfEntries * sizeof(TocEntryStructType);
      break;
    case GENERIC_VERSION_ELEM_TYPE:
    case INJECTED_PSI_VERSION_ELEM_TYPE:
    case INJECTED_EBL_VERSION_ELEM_TYPE:
      data_length = ((VersionDataStructType *)PayloadPtr)->DataLength;
      break;
    case INJECTED_PSI_ELEM_TYPE:
    case INJECTED_EBL_ELEM_TYPE:
    case INJECTED_INI_ELEM_TYPE:
    case INJECTED_PDF_ELEM_TYPE:
      data_length = ((InjectionDataStructType *)PayloadPtr)->DataLength;
      break;
    case ROOT_DISK_ELEM_TYPE:
    case CUST_DISK_ELEM_TYPE:
    case USER_DISK_ELEM_TYPE:
      data_length = ((StorageDataStructType *)PayloadPtr)->DataLength;
      break;
    default:
      break;
  }

  return data_length;
}


uint32_t PRGH_ReplaceElement(IFWD_MemoryStream *Output,uint32_t Type,uint32_t UID,char* PayloadPtr,uint32_t PayloadSize)
{
  uint32_t elem_index;
  ElementHeaderStructType *E;

  if(PRGH_GetElementIndex((char*)Output->Memory,Type,UID,&elem_index))
  {
    E = (ElementHeaderStructType *)((char*)Output->Memory+elem_index);
    if( (GetLongFromLong(E->Size) - sizeof(ElementHeaderStructType)) == PayloadSize) // Replace is done only if the payload size is same as existing size
    {
      memcpy((char*)Output->Memory+elem_index+sizeof(ElementHeaderStructType),PayloadPtr,PayloadSize);
      return PRG_OK;
    }
    else
      return PRG_ELEMENT_SIZE_DIFFER;
  }
  else
    return PRG_ELEMENT_NOT_FOUND;
}

//############################## PRG HANDLER INTERFACE FUNCTIONS with FILE handles ######################

void PRGH_SetFileOffset(uint32_t value)
{
  file_offset = value;
}

uint32_t PRGH_FileCheck_File(FILE *fp)
{
  GenericHeaderElementStructType G;
  if(fp)
  {
    // Store the original position
    long int pos=FILE_pos(fp);
    // rewind the file to position 0
    FILE_seek(fp,file_offset);

    // Read the generic header
    if(FILE_read(fp, (uint8_t*)&G, sizeof(GenericHeaderElementStructType)) !=  FILE_status_ok)
      return INVALID_FILE_TYPE;

    // set the position back to original position
    FILE_seek(fp,pos);

    // Do the file check and return
    return PRGH_FileCheck((char*)&G);
  }
  return INVALID_FILE_TYPE;
}

uint32_t PRGH_VersionCheck_File(FILE *fp,uint32_t ToolElementType, uint32_t CurrentVersion)
{
  uint32_t elem_index;
  uint32_t RequiredVersion  = INVALID_TOOL_VERSION;
  ToolVersionElementStructType T;

  // Store the original position
  long int pos=FILE_pos(fp);

  // rewind the file to position 0
  FILE_seek(fp,file_offset);

  if(!PRGH_GetElementIndex_File(fp,ToolElementType,GENERIC_UID,&elem_index)) // Search on Generic UID
    return RequiredVersion;

  // set the postion to current element index
  FILE_seek(fp,elem_index);

  // Read the Tool Version element at current position
  FILE_read(fp, (uint8_t*)&T, sizeof(ToolVersionElementStructType));


  // set the position back to original position
  FILE_seek(fp,pos);

  if(GetLongFromLong(T.Header.Size) != sizeof(ToolVersionElementStructType))
    return RequiredVersion;

  if( CurrentVersion < GetLongFromLong(T.Data.Required) ||
    ((CurrentVersion >> 16) & 0x0000FFFF) > ((GetLongFromLong(T.Data.Required) >> 16) & 0x0000FFFF)
    )
    RequiredVersion = GetLongFromLong(T.Data.Required);
  else
    RequiredVersion = 0;

  return RequiredVersion;
}


uint8_t PRGH_GetElementIndex_File(FILE *fp,uint32_t ElementType, uint32_t UID, uint32_t *element_index)
{
  int index = 0;
  bool found=false;
  bool end_marker=false;
  bool endoffile = false;
  ElementHeaderStructType E;

  // Store the original position
  long int pos=FILE_pos(fp);
  // rewind the file to position 0
  FILE_seek(fp,file_offset);

  *element_index = INVALID_ELEMENT_INDEX;
  while(!end_marker)
  {
    if(FILE_read(fp, (uint8_t*)&E, sizeof(ElementHeaderStructType)) == FILE_status_EOF)
      endoffile = true;
    // find if we have reached the last element
    if(GetLongFromLong(E.Type) == END_MARKER_ELEM_TYPE)
      end_marker = true;
    // Check the element type and Element UID
    if( (GetLongFromLong(E.Type) == ElementType) && (GetLongFromLong(E.UID) == UID)  )
    {
      *element_index=index;
      found = true;
      break; /* exit while() loop */
    }
    else
    {
      if(E.Size == 0 || endoffile) /* error in size that would cause endless loop, or EOF reached? */
      {
        break; /* exit while() loop */
      }
      else
      {
        index += GetLongFromLong(E.Size); // Add the size of the element to get next element index
        FILE_seek(fp,index+file_offset);
      }
    }
  }
  *element_index += file_offset;
  // set the position back to original position
  FILE_seek(fp,pos);

  return found;
}

uint8_t PRGH_GetNextElementIndex_File(FILE *fp,uint32_t* current_element_index)
{
  int index = *current_element_index;
  ElementHeaderStructType E;

  *current_element_index = INVALID_ELEMENT_INDEX;
  // Store the original position
  long int pos=FILE_pos(fp);
  // set the postion to current element index
  FILE_seek(fp,index);

  // Read the element header at current position
  FILE_read(fp, (uint8_t*)&E, sizeof(ElementHeaderStructType));

  // set the position back to original position
  FILE_seek(fp,pos);

  // find if we are at the last element
  if(GetLongFromLong(E.Type) == END_MARKER_ELEM_TYPE)
    return false;
  index += GetLongFromLong(E.Size); // Add the size of the element to get next element index
  *current_element_index=index;

  return true;
}

void swap_GenericHeaderElementStructType_endian(GenericHeaderElementStructType *I)
{
  swap_endian_long((char*)I,sizeof(GenericHeaderElementStructType));
}

void swap_SafeBootCodewordElementStructType_endian(SafeBootCodewordElementStructType *I)
{
  swap_endian_long((char*)I,sizeof(SafeBootCodewordElementStructType));
}

void swap_ToolVersionElementStructType_endian(ToolVersionElementStructType *I)
{
  swap_endian_long((char*)I,sizeof(ToolVersionElementStructType));
}

void swap_EndElementStructType_endian(EndElementStructType *I)
{
  swap_endian_long((char*)I,sizeof(EndElementStructType));
}

void swap_VersionDataElementStructType_endian(VersionDataElementStructType *I)
{
  swap_endian_long((char*)&I->Header,sizeof(I->Header));
  swap_endian_long((char*)&I->Data.DataLength,sizeof(I->Data.DataLength));
  swap_endian_long((char*)&I->Data.DataOffset,sizeof(I->Data.DataOffset));
}

void swap_StorageDataElementStructType_endian(StorageDataElementStructType *I)
{
  swap_endian_long((char*)&I->Header,sizeof(I->Header));
  swap_endian_long((char*)&I->Data.DataLength,sizeof(I->Data.DataLength));
  swap_endian_long((char*)&I->Data.DataOffset,sizeof(I->Data.DataOffset));
}

void swap_InjectionElementStructType_endian(InjectionElementStructType *I)
{
  swap_endian_long((char*)&I->Header,sizeof(I->Header));
  if(CalledByWrite)
  {
    I->Data.CRC_16 = I->Data.CRC_16 << 16;
    I->Data.CRC_8 = I->Data.CRC_8 << 24;
  }
  else
  {
    I->Data.CRC_16 = I->Data.CRC_16 >> 16;
    I->Data.CRC_8 = I->Data.CRC_8 >> 24;
  }
  //swap_endian_long((char*)&I->Data.CRC_16,sizeof(I->Data.CRC_16));
  //swap_endian_long((char*)&I->Data.CRC_8,sizeof(I->Data.CRC_8));
  swap_endian_long((char*)&I->Data.DataLength,sizeof(I->Data.DataLength));
  swap_endian_long((char*)&I->Data.DataOffset,sizeof(I->Data.DataOffset));
}

void swap_DownloadDataElementStructType_endian(DownloadDataElementStructType *I)
{
  swap_endian_long((char*)&I->Header,sizeof(I->Header));
  swap_endian_long((char*)&I->Data.LoadMapIndex,sizeof(I->Data.LoadMapIndex));
  swap_endian_long((char*)&I->Data.CompressionAlgorithm,sizeof(I->Data.CompressionAlgorithm));
  swap_endian_long((char*)&I->Data.CompressedLength,sizeof(I->Data.CompressedLength));
  if(CalledByWrite)
    I->Data.CRC = I->Data.CRC << 16;
  else
    I->Data.CRC = I->Data.CRC >> 16;
  //swap_endian_long((char*)&I->Data.CRC,sizeof(I->Data.CRC));
  swap_endian_long((char*)&I->Data.DataLength,sizeof(I->Data.DataLength));
  swap_endian_long((char*)&I->Data.DataOffset,sizeof(I->Data.DataOffset));
}

void swap_IndirectDownloadDataElementStructType_endian(IndirectDownloadDataElementStructType *I)
{
  swap_endian_long((char*)&I->Header,sizeof(I->Header));
  swap_endian_long((char*)&I->Data.LoadMapIndex,sizeof(I->Data.LoadMapIndex));
  swap_endian_long((char*)&I->Data.CompressionAlgorithm,sizeof(I->Data.CompressionAlgorithm));
  swap_endian_long((char*)&I->Data.CompressedLength,sizeof(I->Data.CompressedLength));
  I->Data.CRC = I->Data.CRC << 16;
  //swap_endian_long((char*)&I->Data.CRC,sizeof(I->Data.CRC));
  swap_endian_long((char*)&I->Data.DataLength,sizeof(I->Data.DataLength));
  swap_endian_long((char*)&I->Data.DataOffset,sizeof(I->Data.DataOffset));
}

void swap_HardwareElementStructType_endian(HardwareElementStructType *I)
{
  swap_endian_long((char*)&I->Header,sizeof(I->Header));
  swap_endian_long((char*)&I->Data.Platform,sizeof(I->Data.Platform));
  swap_endian_long((char*)&I->Data.SystemSize,sizeof(I->Data.SystemSize));
  swap_endian_long((char*)&I->Data.BootSpeed,sizeof(I->Data.BootSpeed));
  swap_endian_long((char*)&I->Data.HandshakeMode,sizeof(I->Data.HandshakeMode));
  swap_endian_long((char*)&I->Data.UsbCapable,sizeof(I->Data.UsbCapable));
  swap_endian_long((char*)&I->Data.FlashTechnology,sizeof(I->Data.FlashTechnology));

  switch(I->Data.Platform)
  {
    case E_GOLD_CHIPSET_V2:
    case E_GOLD_LITE_CHIPSET:
    case E_GOLD_CHIPSET_V3:
    case E_GOLD_RADIO_V1:
    case E_GOLD_VOICE_V1:
    case X_GOLD_102:
    case X_GOLD_110:
      // EGold structures ( all are unsigned short)
      swap_endian_short((char*)&I->Data.ChipSelect,sizeof(I->Data.ChipSelect));
      break;
    default:
      // SGold or SGold3 struction ( all are unsigned longs )
      swap_endian_long((char*)&I->Data.ChipSelect,sizeof(I->Data.ChipSelect));
  }
}

void swap_NandPartitionElementStructType_endian(NandPartitionElementStructType *I)
{
  swap_endian_long((char*)I,sizeof(NandPartitionElementStructType));
}

void swap_SecurityElementStructType_endian(SecurityElementStructType *I)
{
  swap_endian_long((char*)&I->Header,sizeof(I->Header));
  swap_endian_long((char*)&I->Data.Type,sizeof(I->Data.Type));

  swap_endian_long((char*)&I->Data.BootCoreVersion,sizeof(I->Data.BootCoreVersion));
  swap_endian_long((char*)&I->Data.EblVersion,sizeof(I->Data.EblVersion));
  swap_endian_long((char*)&I->Data.SecExtend.Old.BootStartAddr,sizeof(I->Data.SecExtend.Old.BootStartAddr));
  swap_endian_long((char*)&I->Data.SecExtend.Old.BootLength,sizeof(I->Data.SecExtend.Old.BootLength));
  swap_endian_long((char*)&I->Data.SecExtend.Old.BootSignature,sizeof(I->Data.SecExtend.Old.BootSignature));

  swap_endian_long((char*)&I->Data.FileHash,sizeof(I->Data.FileHash));
  swap_endian_long((char*)&I->Data.Partition,sizeof(I->Data.Partition));
  swap_endian_long((char*)&I->Data.LoadAddrToPartition,sizeof(I->Data.LoadAddrToPartition));
  swap_endian_long((char*)&I->Data.LoadMagic,sizeof(I->Data.LoadMagic));
  swap_endian_long((char*)&I->Data.LoadMap,sizeof(I->Data.LoadMap));
}

void swap_MemoryMapElementStructType_endian(MemoryMapElementStructType *I)
{
  int i;

  swap_endian_long((char*)&I->Header,sizeof(I->Header));
  swap_endian_long((char*)&I->Data.EepRevAddrInSw,sizeof(I->Data.EepRevAddrInSw));
  swap_endian_long((char*)&I->Data.EepRevAddrInEep,sizeof(I->Data.EepRevAddrInEep));
  swap_endian_long((char*)&I->Data.EepStartModeAddr,sizeof(I->Data.EepStartModeAddr));
  swap_endian_long((char*)&I->Data.NormalModeValue,sizeof(I->Data.NormalModeValue));
  swap_endian_long((char*)&I->Data.TestModeValue,sizeof(I->Data.TestModeValue));
  swap_endian_long((char*)&I->Data.SwVersionStringLocation,sizeof(I->Data.SwVersionStringLocation));
  swap_endian_long((char*)&I->Data.FlashStartAddr,sizeof(I->Data.FlashStartAddr));

  for(i=0;i<MAX_MAP_ENTRIES;i++)
  {
    swap_endian_long((char*)&I->Data.Entry[i].Start,sizeof(I->Data.Entry[i].Start));
    swap_endian_long((char*)&I->Data.Entry[i].Length,sizeof(I->Data.Entry[i].Length));
    swap_endian_long((char*)&I->Data.Entry[i].Class,sizeof(I->Data.Entry[i].Class));
    swap_endian_long((char*)&I->Data.Entry[i].Flags,sizeof(I->Data.Entry[i].Flags));
  }
}


void swap_TocElementStructType_endian(TocElementStructType *I)
{
  unsigned int i,num_iterations;
  swap_endian_long((char*)&I->Header,sizeof(I->Header));
  swap_endian_long((char*)&I->Data.NoOfEntries,sizeof(I->Data.NoOfEntries));
  if(CalledByWrite)
    num_iterations = GetLongFromLong(I->Data.NoOfEntries);
  else
    num_iterations = I->Data.NoOfEntries;

  for(i=0;i<num_iterations;i++)
  {
    TocEntryStructType *T = (TocEntryStructType *)((char*)I->Data.Data + (i * sizeof(TocEntryStructType)));
    swap_endian_long((char *)&T->UID, sizeof(T->UID));
    swap_endian_long((char *)&T->MemoryClass, sizeof(T->MemoryClass));
    swap_endian_long((char *)&T->Reserved, sizeof(T->Reserved));
  }
  swap_endian_long((char*)&I->Data.DataOffset,sizeof(I->Data.DataOffset));
}

void PRGH_SwapElementEndian(void *Memory,uint32_t ElemType)
{
  //prg_handler_ret_type found = PRG_OK;
  switch(ElemType)
  {
    case GENERIC_HEADER_ELEM_TYPE:
      swap_GenericHeaderElementStructType_endian((GenericHeaderElementStructType *) Memory);
      break;
    case PRGHANDLER_TOOL_ELEM_TYPE:
    case PRGSEQUENCER_TOOL_ELEM_TYPE:
    case MAKEPRG_TOOL_ELEM_TYPE:
    case HEXTOFLS_TOOL_ELEM_TYPE:
    case FLSSIGN_TOOL_ELEM_TYPE:
    case DWDTOHEX_TOOL_ELEM_TYPE:
    case FLASHTOOL_TOOL_ELEM_TYPE:
    case STORAGETOOL_TOOL_ELEM_TYPE:
    case FLSTOHEADER_TOOL_ELEM:
    case BOOT_CORE_TOOL_ELEM:
    case EBL_TOOL_ELEM:
    case FLSTONAND_TOOL_ELEM:
      swap_ToolVersionElementStructType_endian((ToolVersionElementStructType *) Memory);
      break;
    case MEMORYMAP_ELEM_TYPE:
      swap_MemoryMapElementStructType_endian((MemoryMapElementStructType *) Memory);
      break;
    case DOWNLOADDATA_ELEM_TYPE:
      swap_DownloadDataElementStructType_endian((DownloadDataElementStructType *) Memory);
      break;
    case INDIRECT_DOWNLOADDATA_ELEM_TYPE:
      swap_IndirectDownloadDataElementStructType_endian((IndirectDownloadDataElementStructType *) Memory);
      break;
    case HARDWARE_ELEM_TYPE:
      swap_HardwareElementStructType_endian((HardwareElementStructType *) Memory);
      break;
    case NANDPARTITION_ELEM_TYPE:
      swap_NandPartitionElementStructType_endian((NandPartitionElementStructType *) Memory);
      break;
    case SECURITY_ELEM_TYPE:
      swap_SecurityElementStructType_endian((SecurityElementStructType *) Memory);
      break;
    case TOC_ELEM_TYPE:
      swap_TocElementStructType_endian((TocElementStructType *) Memory);
      break;
    case SAFE_BOOT_CODEWORD_ELEM_TYPE:
      swap_SafeBootCodewordElementStructType_endian((SafeBootCodewordElementStructType*) Memory);
      break;
    case GENERIC_VERSION_ELEM_TYPE:
    case INJECTED_PSI_VERSION_ELEM_TYPE:
    case INJECTED_EBL_VERSION_ELEM_TYPE:
      swap_VersionDataElementStructType_endian((VersionDataElementStructType*) Memory);
      break;
    case INJECTED_PSI_ELEM_TYPE:
    case INJECTED_EBL_ELEM_TYPE:
    case INJECTED_INI_ELEM_TYPE:
    case INJECTED_PDF_ELEM_TYPE:
      swap_InjectionElementStructType_endian((InjectionElementStructType*) Memory);
      break;
    case ROOT_DISK_ELEM_TYPE:
    case CUST_DISK_ELEM_TYPE:
    case USER_DISK_ELEM_TYPE:
      swap_StorageDataElementStructType_endian((StorageDataElementStructType *) Memory);
      break;
    default:
      //found = PRG_FAIL;
      break;
  }
  //return found;
}
/*lint +e545 Warning 545: Suspicious use of & */

#ifdef __cplusplus
}
#endif

