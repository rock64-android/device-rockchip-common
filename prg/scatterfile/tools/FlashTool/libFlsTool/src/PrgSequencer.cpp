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
 * Description:
 *  Helper functions for mainly filesystem related tasks
 *
 * Revision Information:
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/PrgSequencer.cpp $
 *   Resonsible:
 *   Comment:
 *
 */

//#pragma hdrstop
#include <stdlib.h>

#include "PrgSequencer.h"
#include "PrgHandler.h"

#include "Security.h"
#include "c_sha1.h"
#include "string.h"

#include <iostream>
using namespace std;

//---------------------------------------------------------------------------
#ifdef _WIN32
  #include <windows.h>
  //#pragma package(smart_init)

  BOOL WINAPI WinMain(HINSTANCE hinst, DWORD reason, LPVOID lpReserved)
  {
    return true;
  }
#endif

bool PRGSequencer::AddToGlobalStruct(char* Mem)
{
  if(ElemCount >= MAX_NUM_ELEMENTS )
    return false;

  ElementHeaderStructType *E = (ElementHeaderStructType *)Mem;
  AllocateAndCopy(Mem,PRGH_GetLongFromLong(E->Type));
  return true;
}

bool PRGSequencer::AllocateAndCopy(char* Mem, uint32_t ElemType)
{
  bool found = true;
  int StructSize = 0;
#ifdef __BORLANDC__
char *ErrorMsg;
#endif

  switch(ElemType)
  {
    case GENERIC_HEADER_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( GenericHeaderElementStructType *)malloc(sizeof(GenericHeaderElementStructType));
      StructSize = sizeof(GenericHeaderElementStructType);
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
      ElemPtrs[ElemCount] = ( ToolVersionElementStructType *)malloc(sizeof(ToolVersionElementStructType));
      StructSize = sizeof(ToolVersionElementStructType);
      break;
    case MEMORYMAP_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( MemoryMapElementStructType *)malloc(sizeof(MemoryMapElementStructType));
      StructSize = sizeof(MemoryMapElementStructType);
      break;
    case DOWNLOADDATA_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( DownloadDataElementStructType *)malloc(sizeof(DownloadDataElementStructType));
      StructSize = sizeof(DownloadDataElementStructType);
      break;
    case INDIRECT_DOWNLOADDATA_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( IndirectDownloadDataElementStructType *)malloc(sizeof(IndirectDownloadDataElementStructType));
      StructSize = sizeof(IndirectDownloadDataElementStructType);
      break;
    case HARDWARE_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( HardwareElementStructType *)malloc(sizeof(HardwareElementStructType));
      StructSize = sizeof(HardwareElementStructType);
      break;
    case NANDPARTITION_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( NandPartitionElementStructType *)malloc(sizeof(NandPartitionElementStructType));
      StructSize = sizeof(NandPartitionElementStructType);
      break;
    case SECURITY_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( SecurityElementStructType *)malloc(sizeof(SecurityElementStructType));
      StructSize = sizeof(SecurityElementStructType);
      break;
    case TOC_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( TocElementStructType *)malloc(sizeof(TocElementStructType));
      StructSize = sizeof(TocElementStructType);
      break;
    case SAFE_BOOT_CODEWORD_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( SafeBootCodewordElementStructType *)malloc(sizeof(SafeBootCodewordElementStructType));
      StructSize = sizeof(SafeBootCodewordElementStructType);
      break;
    case GENERIC_VERSION_ELEM_TYPE:
    case INJECTED_PSI_VERSION_ELEM_TYPE:
    case INJECTED_EBL_VERSION_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( VersionDataElementStructType *)malloc(sizeof(VersionDataElementStructType));
      StructSize = sizeof(VersionDataElementStructType);
      break;
    case INJECTED_PSI_ELEM_TYPE:
    case INJECTED_EBL_ELEM_TYPE:
    case INJECTED_INI_ELEM_TYPE:
    case INJECTED_PDF_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( InjectionElementStructType *)malloc(sizeof(InjectionElementStructType));
      StructSize = sizeof(InjectionElementStructType);
      break;
    case ROOT_DISK_ELEM_TYPE:
    case CUST_DISK_ELEM_TYPE:
    case USER_DISK_ELEM_TYPE:
      ElemPtrs[ElemCount] = ( StorageDataElementStructType *)malloc(sizeof(StorageDataElementStructType));
      StructSize = sizeof(StorageDataElementStructType);
      break;
    default:
      found = false;
      break;
  }

  if(found)
  {
    if(Mem !=0)
    {
      memcpy(ElemPtrs[ElemCount],Mem,StructSize);
      TamperDataAfterRead(ElemType,Mem);
      if(MachEndian == BIG_ENDIAN_MACHINE)
        PRGH_SwapElementEndian(ElemPtrs[ElemCount],ElemType);
#ifdef __BORLANDC__
   // for decrypting if the file read is dongle protected
       if(ElemType==SECURITY_ELEM_TYPE)
       {
         if(!DecryptSecPack(&((SecurityElementStructType*)ElemPtrs[ElemCount])->Data,&ErrorMsg))
         {
            printf(ErrorMsg);
            exit(-1);
         }
       }
#endif

    }
    else
    {
      InitializeElement(ElemPtrs[ElemCount],ElemType,StructSize);
      ((ElementHeaderStructType*)ElemPtrs[ElemCount])->Type = ElemType;
      ((ElementHeaderStructType*)ElemPtrs[ElemCount])->Size = StructSize;
    }

    if(NewElementFunc != 0)
      NewElementFunc(ElemPtrs[ElemCount]);
    ElemCount++;
  }

  return found;
}

void PRGSequencer::InitializeElement(void *Mem, uint32_t ElemType,int size)
{
  memset(Mem,0x00,size);
  switch(ElemType)
  {
    case MEMORYMAP_ELEM_TYPE:
      for(int i=0;i<MAX_MAP_ENTRIES;i++)
        (( MemoryMapElementStructType *)Mem)->Data.Entry[i].Start = 0xFFFFFFFF;
      break;
    /* In case of Tool Elements, when a new element is created, the Required and Used versions */
    /* should be updated to what is given in ToolVersions.h. This is because, when a new element */
    /* is created, user should know the versions which are going to be written to the file, when he */
    /* saves the file, ( rather then setting initial values are Zeros */
    case PRGHANDLER_TOOL_ELEM_TYPE:
      ((ToolVersionElementStructType*)Mem)->Data.Required = PRGHANDLER_TOOL_REQ_VER;
      ((ToolVersionElementStructType*)Mem)->Data.Used = PRG_HANDLER_VERSION;
      break;
    case PRGSEQUENCER_TOOL_ELEM_TYPE:
      ((ToolVersionElementStructType*)Mem)->Data.Required = PRGSEQUENCER_TOOL_REQ_VER;
      ((ToolVersionElementStructType*)Mem)->Data.Used = PRG_SEQUENCER_VERSION;
      break;
    case MAKEPRG_TOOL_ELEM_TYPE:
      ((ToolVersionElementStructType*)Mem)->Data.Required = MAKEPRG_TOOL_REQ_VER;
      ((ToolVersionElementStructType*)Mem)->Data.Used = MAKEPRG_TOOL_REQ_VER;;
      break;
    case HEXTOFLS_TOOL_ELEM_TYPE:
      ((ToolVersionElementStructType*)Mem)->Data.Required = HEXTOFLS_TOOL_REQ_VER;
      (( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
      break;
    case FLSSIGN_TOOL_ELEM_TYPE:
      ((ToolVersionElementStructType*)Mem)->Data.Required = FLSSIGN_TOOL_REQ_VER;
      (( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
      break;
    case DWDTOHEX_TOOL_ELEM_TYPE:
      ((ToolVersionElementStructType*)Mem)->Data.Required = DWDTOHEX_TOOL_REQ_VER;
      (( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
      break;
    case FLASHTOOL_TOOL_ELEM_TYPE:
      ((ToolVersionElementStructType*)Mem)->Data.Required = FLASHTOOL_TOOL_REQ_VER;
      (( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
      break;
    case STORAGETOOL_TOOL_ELEM_TYPE:
      ((ToolVersionElementStructType*)Mem)->Data.Required = STORAGETOOL_TOOL_REQ_VER;
      (( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
      break;
		case FLSTOHEADER_TOOL_ELEM:
			((ToolVersionElementStructType*)Mem)->Data.Required = FLSTOHEADER_TOOL_REQ_VER;
			(( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
			break;
		case BOOT_CORE_TOOL_ELEM:
			((ToolVersionElementStructType*)Mem)->Data.Required = BOOT_CORE_TOOL_REQ_VER;
			(( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
			break;
		case EBL_TOOL_ELEM:
			((ToolVersionElementStructType*)Mem)->Data.Required = EBL_TOOL_REQ_VER;
			(( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
			break;
		case FLSTONAND_TOOL_ELEM:
			((ToolVersionElementStructType*)Mem)->Data.Required = FLSTONAND_TOOL_REQ_VER;
			(( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
			break;
		/* End of Tool element updation */
	}
}

void PRGSequencer::TamperDataAfterRead(uint32_t ElemType,char*Mem)
{
	// Data pointer has to be adjusted in case of special elements, eg: DownloadData

  if(ElemCount >= MAX_NUM_ELEMENTS) ElemCount = MAX_NUM_ELEMENTS-1; /* avoid lint warning 661 */
	switch(ElemType)
	{
		case DOWNLOADDATA_ELEM_TYPE:
			(( DownloadDataElementStructType *)ElemPtrs[ElemCount])->Data.Data = (unsigned char*) (Mem+sizeof(DownloadDataElementStructType));
      break;
    case TOC_ELEM_TYPE:
      (( TocElementStructType *)ElemPtrs[ElemCount])->Data.Data = (TocEntryStructType*)(Mem+sizeof(TocElementStructType));
      break;
    case GENERIC_VERSION_ELEM_TYPE:
    case INJECTED_PSI_VERSION_ELEM_TYPE:
    case INJECTED_EBL_VERSION_ELEM_TYPE:
      (( VersionDataElementStructType *)ElemPtrs[ElemCount])->Data.Data = (unsigned char*) (Mem+sizeof(VersionDataElementStructType));
      break;
    case INJECTED_PSI_ELEM_TYPE:
    case INJECTED_EBL_ELEM_TYPE:
    case INJECTED_INI_ELEM_TYPE:
    case INJECTED_PDF_ELEM_TYPE:
      (( InjectionElementStructType *)ElemPtrs[ElemCount])->Data.Data = (unsigned char*)(Mem+sizeof(InjectionElementStructType));
      break;
    case ROOT_DISK_ELEM_TYPE:
    case CUST_DISK_ELEM_TYPE:
    case USER_DISK_ELEM_TYPE:
      (( StorageDataElementStructType *)ElemPtrs[ElemCount])->Data.Data = (unsigned char*)(Mem+sizeof(StorageDataElementStructType));
      break;
  }
}


bool PRGSequencer::IsDynamicElement(uint32_t ElemType)
{
  if(ElemType == DOWNLOADDATA_ELEM_TYPE ||
    ElemType == INDIRECT_DOWNLOADDATA_ELEM_TYPE ||
    ElemType == SECURITY_ELEM_TYPE ||
    ElemType == TOC_ELEM_TYPE ||
    ElemType == GENERIC_VERSION_ELEM_TYPE ||
    ElemType == INJECTED_PSI_VERSION_ELEM_TYPE ||
    ElemType == INJECTED_EBL_VERSION_ELEM_TYPE ||
    ElemType == INJECTED_PSI_ELEM_TYPE ||
    ElemType == INJECTED_EBL_ELEM_TYPE ||
    ElemType == INJECTED_INI_ELEM_TYPE ||
    ElemType == INJECTED_PDF_ELEM_TYPE ||
    ElemType == ROOT_DISK_ELEM_TYPE ||
    ElemType == CUST_DISK_ELEM_TYPE ||
    ElemType == USER_DISK_ELEM_TYPE
  )
    return true;
  else
    return false;
}

char* PRGSequencer::FindElementStructure(uint32_t element_type,uint32_t UID)
{

	for(int i=0; i<ElemCount && i < MAX_NUM_ELEMENTS;i++)
	{
		if(ElemPtrs[i] != 0)
		{
			ElementHeaderStructType *E = (ElementHeaderStructType *)ElemPtrs[i];
			if(E->Type == element_type && E->UID == UID)
			{
				return (char*)ElemPtrs[i];
			}
		}
	}
	return 0;
}

bool PRGSequencer::WriteElementToFile(IFWD_MemoryStream* Output, uint32_t element_type,uint32_t uid,char* Mem)
{
  bool valid_elem=true;
  int payload_size;

  switch(element_type)
  {
    case GENERIC_HEADER_ELEM_TYPE:
      payload_size = sizeof(GenericHeaderDataStructType);
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
			payload_size = sizeof(ToolVersionStructType);
      break;
    case MEMORYMAP_ELEM_TYPE:
      payload_size = sizeof(MemoryMapStructType);
      break;
    case DOWNLOADDATA_ELEM_TYPE:
      payload_size = sizeof(DownloadDataStructType);
      break;
    case INDIRECT_DOWNLOADDATA_ELEM_TYPE:
      payload_size = sizeof(IndirectDownloadDataStructType);
      break;
    case HARDWARE_ELEM_TYPE:
      payload_size = sizeof(HardwareStructType);
      break;
    case NANDPARTITION_ELEM_TYPE:
      payload_size = sizeof(NandPartitionDataStructType);
      break;
    case SECURITY_ELEM_TYPE:
      payload_size = sizeof(SecurityStructType);
      break;
    case TOC_ELEM_TYPE:
      payload_size = sizeof(TocStructType);
      break;
    case SAFE_BOOT_CODEWORD_ELEM_TYPE:
      payload_size = sizeof(CodewordStructType);
      break;
    case GENERIC_VERSION_ELEM_TYPE:
    case INJECTED_PSI_VERSION_ELEM_TYPE:
    case INJECTED_EBL_VERSION_ELEM_TYPE:
      payload_size = sizeof(VersionDataStructType);
      break;
    case INJECTED_PSI_ELEM_TYPE:
    case INJECTED_EBL_ELEM_TYPE:
    case INJECTED_INI_ELEM_TYPE:
    case INJECTED_PDF_ELEM_TYPE:
      payload_size = sizeof(InjectionDataStructType);
      break;
    case ROOT_DISK_ELEM_TYPE:
    case CUST_DISK_ELEM_TYPE:
    case USER_DISK_ELEM_TYPE:
      payload_size = sizeof(StorageDataStructType);
      break;
    default:
      valid_elem = false;
      payload_size=0;
    break;
  }

  if(valid_elem)
  {
    //TamperDataBeforeWrite(element_type,Mem);
    PRGH_FileWriteElement(fout,
                      Output,
                      element_type,
                      uid,
                      Mem+sizeof(ElementHeaderStructType),
                      payload_size);
  }

  return valid_elem;
}

uint32_t PRGSequencer::FindElementSize(uint32_t element_type,char* Mem)
{
  bool valid_elem=true;
  uint32_t payload_size;
  uint32_t elem_size = 0;

  switch(element_type)
  {
    case GENERIC_HEADER_ELEM_TYPE:
      payload_size = sizeof(GenericHeaderDataStructType);
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
			payload_size = sizeof(ToolVersionStructType);
      break;
    case MEMORYMAP_ELEM_TYPE:
      payload_size = sizeof(MemoryMapStructType);
      break;
    case DOWNLOADDATA_ELEM_TYPE:
      payload_size = sizeof(DownloadDataStructType);
      break;
    case INDIRECT_DOWNLOADDATA_ELEM_TYPE:
      payload_size = sizeof(IndirectDownloadDataStructType);
      break;
    case HARDWARE_ELEM_TYPE:
      payload_size = sizeof(HardwareStructType);
      break;
    case NANDPARTITION_ELEM_TYPE:
      payload_size = sizeof(NandPartitionDataStructType);
      break;
    case SECURITY_ELEM_TYPE:
      payload_size = sizeof(SecurityStructType);
      break;
    case TOC_ELEM_TYPE:
      payload_size = sizeof(TocStructType);
      break;
    case SAFE_BOOT_CODEWORD_ELEM_TYPE:
      payload_size = sizeof(CodewordStructType);
      break;
    case GENERIC_VERSION_ELEM_TYPE:
    case INJECTED_PSI_VERSION_ELEM_TYPE:
    case INJECTED_EBL_VERSION_ELEM_TYPE:
      payload_size = sizeof(VersionDataStructType);
      break;
    case INJECTED_PSI_ELEM_TYPE:
    case INJECTED_EBL_ELEM_TYPE:
    case INJECTED_INI_ELEM_TYPE:
    case INJECTED_PDF_ELEM_TYPE:
      payload_size = sizeof(InjectionDataStructType);
      break;
    case ROOT_DISK_ELEM_TYPE:
    case CUST_DISK_ELEM_TYPE:
    case USER_DISK_ELEM_TYPE:
      payload_size = sizeof(StorageDataStructType);
      break;
    default:
      valid_elem = false;
      payload_size=0;
    break;
  }

  if(valid_elem)
  {
    //TamperDataBeforeWrite(element_type,Mem);
    elem_size = payload_size+
                PRGH_FindElementDataSize(element_type,Mem+sizeof(ElementHeaderStructType))+
                sizeof(ElementHeaderStructType);
  }

  return elem_size;
}

uint32_t PRGSequencer::WriteToolElementToFile(IFWD_MemoryStream* Output, uint32_t ToolElemType, uint32_t ReqVer, uint32_t UsedVer)
{
  ToolVersionStructType T;
  uint32_t result;

  //Write Tool Element
  T.Required = ReqVer;
  T.Used = UsedVer;
  result = PRGH_FileWriteElement(fout, Output, ToolElemType, GENERIC_UID, (char*)&T, sizeof(T));

  return result;
}


//#################### Interface functions ################################
PRGSequencer::PRGSequencer()
{
  Input = new IFWD_MemoryStream;
  NewElementFunc = 0;
  memset((uint8_t*)FileHash,0x00,sizeof(FileHash));
  HeaderOffset= 0;
  MemPtr = 0;
  fout=0;
  m_filename = NULL;
  memset((uint8_t*)ElemPtrs,0x00,sizeof(ElemPtrs));
  ElemCount=0;
  MachEndian = 0;
  CallingTool = 0;
  AddingNewElement = false;
  CurrentFileType = 0;
}

/*
 * KlocWorks wants these...
 *
 *  // TODO: Verify
 */
PRGSequencer::PRGSequencer(const PRGSequencer& __copy)
{
  // Private variables
  if (!__copy.Input)
    Input = new IFWD_MemoryStream();
  else
    Input = new IFWD_MemoryStream( *__copy.Input );

  NewElementFunc = __copy.NewElementFunc;
  memset((uint8_t*)ElemPtrs,0x00,sizeof(ElemPtrs));
  memcpy((uint8_t*)FileHash, (uint8_t*)__copy.FileHash,sizeof(FileHash));
  HeaderOffset = __copy.HeaderOffset;
  MemPtr=(char*)Input->Memory;

  if (__copy.m_filename)
    OpenFile(__copy.m_filename);
  else
    fout=0;

  // Public variables
  m_filename = __copy.m_filename;
  ElemCount = __copy.ElemCount;

  for(int i=0; i<ElemCount; ++i)
  {
    if(!__copy.ElemPtrs[i])
      continue;

    ElementHeaderStructType *E = (ElementHeaderStructType *)__copy.ElemPtrs[i];
    uint32_t size = FindElementSize(E->Type,(char*)__copy.ElemPtrs[i]);
    memcpy(__copy.ElemPtrs[i], ElemPtrs[i], size);
  }

  MachEndian = __copy.MachEndian;
  CallingTool = __copy.CallingTool;
  AddingNewElement = __copy.AddingNewElement;
  CurrentFileType = __copy.CurrentFileType;
}

PRGSequencer& PRGSequencer::operator=(const PRGSequencer& __copy)
{
  if(this != &__copy)
  {
    if (!__copy.Input)
      Input = new IFWD_MemoryStream();
    else
      Input = new IFWD_MemoryStream( *__copy.Input );

    // NewElementFunc = __copy.NewElementFunc;
    memset((uint8_t*)ElemPtrs,0x00,sizeof(ElemPtrs));
    memcpy((uint8_t*)FileHash, (uint8_t*)__copy.FileHash,sizeof(FileHash));
    HeaderOffset = __copy.HeaderOffset;
    MemPtr=(char*)Input->Memory;

    if (__copy.m_filename)
      OpenFile(__copy.m_filename);
    else
      fout=0;

    ElemCount = __copy.ElemCount;

    for(int i=0; i<ElemCount; ++i)
    {
      if(!__copy.ElemPtrs[i])
        continue;

      ElementHeaderStructType *E = (ElementHeaderStructType *)__copy.ElemPtrs[i];
      uint32_t size = FindElementSize(E->Type,(char*)__copy.ElemPtrs[i]);
      memcpy(__copy.ElemPtrs[i], ElemPtrs[i], size);
    }

    MachEndian = __copy.MachEndian;
    CallingTool = __copy.CallingTool;
    AddingNewElement = __copy.AddingNewElement;
    CurrentFileType = __copy.CurrentFileType;
  }
  return *this;
}

PRGSequencer::~PRGSequencer()
{
  if(Input)
  {
    delete Input;
    Input = 0;
  }

  if (fout)
    fclose(fout);

  if (m_filename)
    free(m_filename);
}

void PRGSequencer::CleanUp()
{
  Input->Clear();
  for(int i=0; i < ElemCount; i++)
  {
    if(ElemPtrs[i] != 0)
      free(ElemPtrs[i]);
    ElemPtrs[i] = 0;
  }
  ElemCount=0;
  memset(FileHash,0x00,sizeof(FileHash));
}

void PRGSequencer::SetCallBackFunc(void (*NewElementF)(void* ElementPtr))
{
  NewElementFunc = NewElementF;
}

static uint32_t GetDebugHeaderOffset(char *Filename)
{
  IFWD_MemoryStream Input;
  unsigned char offset[4];
  char *MemPtr;
  uint32_t marker,i;
  Input.LoadFromFile(Filename);
  if (!Input.Memory)
    return 0;

  marker=*((uint32_t*)Input.Memory);
  if (marker != 0xED1D6491)
    return 0;

  MemPtr=(char*)Input.Memory;
  for(i=0;i<4;i++)
  {
     offset[i]=MemPtr[0x26+i];
  }
  marker  = offset[0] << 24;
  marker |= offset[1] << 16;
  marker |= offset[2] << 8;
  marker |= offset[3];
  return marker;
}


uint32_t PRGSequencer::ReadFile(char* Filename, uint32_t ElementType, uint32_t ElementVersion,void (*NewElementF)(void* ElementPtr))
{
  uint32_t FileType,ReqVer;
  uint32_t element_index;
  uint32_t uid = GENERIC_UID;
  char *Mem;
  uint32_t retvalue=PRG_OK;

  HeaderOffset= GetDebugHeaderOffset(Filename);

  Input->Clear();
  Input->LoadFromFile(Filename);
  MemPtr=(char*)Input->Memory;

  MachEndian = PRGH_GetEndian(&MemPtr[HeaderOffset]);

  FileType=PRGH_FileCheck(&MemPtr[HeaderOffset]);
  if(FileType == INVALID_FILE_TYPE)
    return FileType;

  CallingTool = ElementType;
  CurrentFileType = FileType;

  if(FileType == DISK_FILE_TYPE)
  {
    retvalue = 0;
  }
  else
  {
    ReqVer = PRGH_VersionCheck(&MemPtr[HeaderOffset],ElementType,ElementVersion);
    if(ReqVer != 0 ) // zero means okay.
    {
      if( CallingTool == MAKEPRG_TOOL_ELEM_TYPE &&
          ( ((ElementVersion >>16) & 0x0000FFFF) > ((ReqVer >>16) & 0x0000FFFF)) &&
          CurrentFileType == PRG_FILE_TYPE
        )
        retvalue=ReqVer;
      else
       return ReqVer;
    }
  }

  NewElementFunc = NewElementF;

  Mem = &MemPtr[HeaderOffset];
  if(PRGH_GetElementIndex(Mem,GENERIC_HEADER_ELEM_TYPE,uid,&element_index))
  {
    memcpy(FileHash,((GenericHeaderElementStructType*)(Mem+element_index))->Data.Sha1Digest,sizeof(FileHash));
    AddToGlobalStruct(&Mem[element_index]);
    while(PRGH_GetNextElementIndex(Mem,&element_index))
    {
      AddToGlobalStruct(&Mem[element_index]);
    }
  }

  return retvalue;
}

uint32_t PRGSequencer::FindFileSizeNeeded()
{
  uint32_t TotalSizeNeeded = 0;

  for(int i=0; i<ElemCount && i < MAX_NUM_ELEMENTS;i++)
  {
    if(ElemPtrs[i] != 0)
    {
      ElementHeaderStructType *E = (ElementHeaderStructType *)ElemPtrs[i];
      if(!IsDynamicElement(E->Type))
    	  TotalSizeNeeded += FindElementSize(E->Type,(char*)ElemPtrs[i]);
    }
  }

  // Calculate only static elements incase of MakePrg
  if(CallingTool != MAKEPRG_TOOL_ELEM_TYPE)
  {
    // Now calculate for the dynamic elements
    for(int i=0; i<ElemCount && i < MAX_NUM_ELEMENTS;i++)
    {
      if(ElemPtrs[i] != 0)
      {
        ElementHeaderStructType *E = (ElementHeaderStructType *)ElemPtrs[i];
        if(IsDynamicElement(E->Type))
          TotalSizeNeeded += FindElementSize(E->Type,(char*)ElemPtrs[i]);
      }
    }
  }

  // At the end add element size of "End Element"
  TotalSizeNeeded += sizeof(EndElementStructType);

  return TotalSizeNeeded;
}
void PRGSequencer::OpenFile(char* Filename)
{
   fout = fopen(Filename,"wb");
   if(fout==NULL)
   {
      printf("ERROR: cannot open output file....");
      exit(1);
   }
}

void PRGSequencer::CloseFile(void)
{
   if(fout!=NULL)
   {
      fclose(fout);
      fout=0;
   }
}

uint32_t PRGSequencer::WriteFile(char* Filename)
{
  uint32_t uid = GENERIC_UID;
  uint32_t ret;
  char *Mem;
  IFWD_MemoryStream Output;

  // Find the total size needed for output file and allocate the space for file here itself
  if(fout==NULL)
     Output.SetSize(FindFileSizeNeeded());

  // first write the static elements
  for(int i=0; i<ElemCount && i < MAX_NUM_ELEMENTS;i++)
  {
    if(ElemPtrs[i] != 0)
    {
      ElementHeaderStructType *E = (ElementHeaderStructType *)ElemPtrs[i];
      if(!IsDynamicElement(E->Type))
    	  WriteElementToFile(&Output, E->Type, E->UID, (char*)ElemPtrs[i]);
    }
  }

  // Write only static elements incase of MakePrg
  if(CallingTool != MAKEPRG_TOOL_ELEM_TYPE)
  {
    // Now write the dynamic elements
    for(int i=0; i<ElemCount && i < MAX_NUM_ELEMENTS;i++)
    {
      if(ElemPtrs[i] != 0)
      {
        ElementHeaderStructType *E = (ElementHeaderStructType *)ElemPtrs[i];
        if(IsDynamicElement(E->Type))
          WriteElementToFile(&Output, E->Type, E->UID, (char*)ElemPtrs[i]);
      }
    }
  }

  PRGH_FileWriteElement(fout, &Output, END_MARKER_ELEM_TYPE, uid, 0, 0);

  // If the calling tool is Make PRG, the file hash has to be taken
  if(CallingTool == MAKEPRG_TOOL_ELEM_TYPE)
  {
    SHA1Context filehash;
    Mem = (char*)(Output.Memory);
    int len_to_hash = Output.Size - sizeof(GenericHeaderElementStructType);

    SHA1Reset(&filehash);

    SHA1Input(&filehash, (unsigned char*)(Mem+sizeof(GenericHeaderElementStructType)), len_to_hash);
    if(!SHA1Result(&filehash))
      return PRG_HASHING_ERROR; // Error in Hashing

    GenericHeaderElementStructType G;
    uint32_t element_index;
    if(PRGH_GetElementIndex(Mem, GENERIC_HEADER_ELEM_TYPE, uid, &element_index))
    {
      // Get the element
      memcpy(&G, &Mem[element_index], sizeof(GenericHeaderElementStructType));
      // Copy the hash
      for(int i=0; i<5;i++)
        G.Data.Sha1Digest[i] = filehash.Message_Digest[i];

      // Mark as derived if needed
      if(G.Data.FileType != PRG_FILE_TYPE)
      {
        // Make it PRG type
        G.Data.FileType = PRG_FILE_TYPE;
        // Then mark it as derived
        G.Data.FileType = G.Data.FileType | 0x01000000;
      }

      ret=PRGH_ReplaceElement(&Output, G.Header.Type, G.Header.UID, (char*)&(G.Data), sizeof(G.Data));
      if(ret != PRG_OK)
        return ret;
    }

    // Copy the file hash to current file hash
    memcpy(FileHash, filehash.Message_Digest, sizeof(FileHash));
  }
  if(fout==NULL)
  {
     if(!Output.SaveToFile(Filename))
       ret = PRG_FILE_WRITE_ERROR;
     else
       ret = PRG_OK;
  }
  else
    ret = PRG_OK;

  return ret;
}

void* PRGSequencer::CreateElement(uint32_t ElementType)
{
  if(AllocateAndCopy(0,ElementType))
    return ElemPtrs[ElemCount - 1];
  else
    return 0;
}

bool PRGSequencer::RemoveElement(void *element_ptr)
{
  bool found = false;

  for(int i=0; i<ElemCount && i < MAX_NUM_ELEMENTS;i++)
  {
    if((char*)ElemPtrs[i] == (char*)element_ptr)
    {
      found = true;
      if(ElemPtrs[i] != 0)
        free(ElemPtrs[i]);
      ElemPtrs[i] = 0;
      break;
    }
  }
  return found;
}

void* PRGSequencer::CreateStorageElement(uint32_t ElementType, char*data_ptr, uint32_t data_len)
{
  StorageDataElementStructType *S;

  if(ElementType != ROOT_DISK_ELEM_TYPE &&
    ElementType != CUST_DISK_ELEM_TYPE &&
    ElementType != USER_DISK_ELEM_TYPE
    )
    return 0;

  S = (StorageDataElementStructType*) CreateElement(ElementType);
  if( S == 0) return 0;

  S->Header.UID = GENERIC_UID;

  S->Data.Data = (unsigned char*)data_ptr;
  S->Data.DataLength = data_len;
  S->Data.DataOffset = 0x00;

  return S;
}

uint32_t PRGSequencer::WriteStorageFile(char* Filename,uint32_t version)
{
  uint32_t uid = GENERIC_UID;
  uint32_t result;
  GenericHeaderDataStructType Gen;
  IFWD_MemoryStream Output;

  //Write Generic Header
  Gen.FileType = DISK_FILE_TYPE;
  Gen.Marker = PRG_MAGIC_NUMBER;
  memset((char*)Gen.Sha1Digest,0xDD,sizeof(Gen.Sha1Digest));
  result=PRGH_FileWriteElement(fout, &Output, GENERIC_HEADER_ELEM_TYPE, GENERIC_UID, (char*)&Gen, sizeof(Gen));
  if(result != PRG_OK)
    return result;

  WriteToolElementToFile(&Output, PRGHANDLER_TOOL_ELEM_TYPE,PRGHANDLER_TOOL_REQ_VER,PRG_HANDLER_VERSION);
  WriteToolElementToFile(&Output, PRGSEQUENCER_TOOL_ELEM_TYPE,PRGSEQUENCER_TOOL_REQ_VER,PRG_SEQUENCER_VERSION);
  WriteToolElementToFile(&Output, MAKEPRG_TOOL_ELEM_TYPE,MAKEPRG_TOOL_REQ_VER,0x00);
  WriteToolElementToFile(&Output, HEXTOFLS_TOOL_ELEM_TYPE,HEXTOFLS_TOOL_REQ_VER,0x00);
  WriteToolElementToFile(&Output, FLSSIGN_TOOL_ELEM_TYPE,FLSSIGN_TOOL_REQ_VER,0x00);
  WriteToolElementToFile(&Output, DWDTOHEX_TOOL_ELEM_TYPE,DWDTOHEX_TOOL_REQ_VER,0x00);
  WriteToolElementToFile(&Output, FLASHTOOL_TOOL_ELEM_TYPE,FLASHTOOL_TOOL_REQ_VER,0x00);
  WriteToolElementToFile(&Output, STORAGETOOL_TOOL_ELEM_TYPE,STORAGETOOL_TOOL_REQ_VER,version);
  WriteToolElementToFile(&Output, FLSTOHEADER_TOOL_ELEM,FLSTOHEADER_TOOL_REQ_VER,0x00);
  WriteToolElementToFile(&Output, BOOT_CORE_TOOL_ELEM,BOOT_CORE_TOOL_REQ_VER,0x00);
  WriteToolElementToFile(&Output, EBL_TOOL_ELEM,EBL_TOOL_REQ_VER,0x00);
  WriteToolElementToFile(&Output, FLSTONAND_TOOL_ELEM,FLSTONAND_TOOL_REQ_VER,0x00);

  // Write the storage elements
  for(int i=0; i<ElemCount && i < MAX_NUM_ELEMENTS;i++)
  {
    if(ElemPtrs[i] != 0)
    {
      ElementHeaderStructType *E = (ElementHeaderStructType *)ElemPtrs[i];
      WriteElementToFile(&Output, E->Type, E->UID, (char*)ElemPtrs[i]);
    }
  }
  PRGH_FileWriteElement(fout, &Output, END_MARKER_ELEM_TYPE, uid, 0, 0);

  if(!Output.SaveToFile(Filename))
    result = PRG_FILE_WRITE_ERROR;
  else
    result = PRG_OK;

  return result;
}

// This is used only by MakePrg
uint32_t PRGSequencer::CompareHash()
{
  uint32_t uid = GENERIC_UID;
  char *Mem;
  prg_handler_ret_type ret;
  IFWD_MemoryStream Output;

   // Find the total size needed for output file and allocate the space for file here itself
  Output.SetSize(FindFileSizeNeeded());

  //write the static elements
  for(int i=0; i<ElemCount && i < MAX_NUM_ELEMENTS;i++)
  {
    if(ElemPtrs[i] != 0)
    {
      ElementHeaderStructType *E = (ElementHeaderStructType *)ElemPtrs[i];
      if(!IsDynamicElement(E->Type))
        WriteElementToFile(&Output, E->Type, E->UID, (char*)ElemPtrs[i]);
    }
  }
  PRGH_FileWriteElement(fout, &Output, END_MARKER_ELEM_TYPE, uid, 0, 0);

  SHA1Context filehash;
  Mem = (char*)(Output.Memory);
  int len_to_hash = Output.Size - sizeof(GenericHeaderElementStructType);

  SHA1Reset(&filehash);
  SHA1Input(&filehash, (unsigned char*)(Mem+sizeof(GenericHeaderElementStructType)), len_to_hash);

  if(!SHA1Result(&filehash))
    return PRG_HASHING_ERROR; // Error in Hashing

  if(0 == memcmp(filehash.Message_Digest, FileHash,sizeof(FileHash)))
    ret = PRG_OK;
  else
    ret = PRG_FAIL;

  return ret;
}

// This is used only by MakePrg
void PRGSequencer::TamperToolDataBeforeWrite(uint32_t MakePrgVersion)
{
  // For all the tool elements, except for PRG Handler, PRG Sequencer and MakePrg,
  // the Used value has to be set to 0x00.
  for(int i=0; i<ElemCount && i < MAX_NUM_ELEMENTS;i++)
  {
    if(ElemPtrs[i] != 0)
    {
      ElementHeaderStructType *E = (ElementHeaderStructType *)ElemPtrs[i];
      char *Mem = (char*)ElemPtrs[i];
      uint32_t ElemType = E->Type;
      switch(ElemType)
      {
        case PRGHANDLER_TOOL_ELEM_TYPE:
          ((ToolVersionElementStructType*)Mem)->Data.Required = PRGHANDLER_TOOL_REQ_VER;
          ((ToolVersionElementStructType*)Mem)->Data.Used = PRG_HANDLER_VERSION;
          break;
        case PRGSEQUENCER_TOOL_ELEM_TYPE:
          ((ToolVersionElementStructType*)Mem)->Data.Required = PRGSEQUENCER_TOOL_REQ_VER;
          ((ToolVersionElementStructType*)Mem)->Data.Used = PRG_SEQUENCER_VERSION;
          break;
        case MAKEPRG_TOOL_ELEM_TYPE:
          ((ToolVersionElementStructType*)Mem)->Data.Required = MAKEPRG_TOOL_REQ_VER;
          ((ToolVersionElementStructType*)Mem)->Data.Used = MakePrgVersion;
          break;
        case HEXTOFLS_TOOL_ELEM_TYPE:
          ((ToolVersionElementStructType*)Mem)->Data.Required = HEXTOFLS_TOOL_REQ_VER;
          (( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
          break;
        case FLSSIGN_TOOL_ELEM_TYPE:
          ((ToolVersionElementStructType*)Mem)->Data.Required = FLSSIGN_TOOL_REQ_VER;
          (( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
          break;
        case DWDTOHEX_TOOL_ELEM_TYPE:
          ((ToolVersionElementStructType*)Mem)->Data.Required = DWDTOHEX_TOOL_REQ_VER;
          (( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
          break;
        case FLASHTOOL_TOOL_ELEM_TYPE:
          ((ToolVersionElementStructType*)Mem)->Data.Required = FLASHTOOL_TOOL_REQ_VER;
          (( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
          break;
        case STORAGETOOL_TOOL_ELEM_TYPE:
          ((ToolVersionElementStructType*)Mem)->Data.Required = STORAGETOOL_TOOL_REQ_VER;
          (( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
          break;
				case FLSTOHEADER_TOOL_ELEM:
					((ToolVersionElementStructType*)Mem)->Data.Required = FLSTOHEADER_TOOL_REQ_VER;
					(( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
					break;
				case BOOT_CORE_TOOL_ELEM:
					((ToolVersionElementStructType*)Mem)->Data.Required = BOOT_CORE_TOOL_REQ_VER;
					(( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
					break;
				case EBL_TOOL_ELEM:
					((ToolVersionElementStructType*)Mem)->Data.Required = EBL_TOOL_REQ_VER;
					(( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
					break;
				case FLSTONAND_TOOL_ELEM:
					((ToolVersionElementStructType*)Mem)->Data.Required = FLSTONAND_TOOL_REQ_VER;
					(( ToolVersionElementStructType *)Mem)->Data.Used = 0x00;
					break;
			}
		}
	}
}

uint32_t PRGSequencer::GetFileLength()
{
  uint32_t start_index,end_index;
  uint32_t uid = GENERIC_UID;
  char *Mem;

  if(MemPtr == 0)
     return 0;

  Mem = &MemPtr[HeaderOffset];
  if(Mem == 0)
    return 0;

  if( PRGH_GetElementIndex(Mem,GENERIC_HEADER_ELEM_TYPE,uid,&start_index) &&
      PRGH_GetElementIndex(Mem,END_MARKER_ELEM_TYPE,uid,&end_index))
  {
    return (end_index - start_index + sizeof(EndElementStructType));
  }
  else
    return 0;
}

