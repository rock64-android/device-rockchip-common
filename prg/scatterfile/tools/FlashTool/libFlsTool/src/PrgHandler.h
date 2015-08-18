/*
 *       Copyright (C) 2012-2014 Intel Mobile Communications GmbH
 *
 *            Sec Class: Intel Confidential (IC)
 *
 *
 * This document contains proprietary information belonging to IMC.
 * Denmark A/S. Passing on and copying of this document, use and communication
 * of its contents is not permitted without prior written authorisation.
 *
 * Description: Interface file
 *
 *
 * Revision Information:
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/PrgHandler.h $
 *   Resonsible:
 *   Comment:
 *
 */
#ifndef __PRGHANDLER_H__
#define __PRGHANDLER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
  #include "MemoryStream.h"
#else
  typedef void* IFWD_MemoryStream;         // When compiled with a C compiler
#endif

#include "header.h"
#include "tool_versions.h"

#define PRG_HANDLER_VERSION PRGHANDLER_TOOL_REQ_VER

#define INVALID_TOOL_VERSION 0xFFFFFFFF
#define INVALID_ELEMENT_INDEX 0xFFFFFFFF
#define INVALID_FILE_TYPE 0xFFFFFFFF

#define FILE_TYPE_MASK 0x000000FF

typedef enum {
  UNKNOWN_ENDIAN_MACHINE,
  LITTLE_ENDIAN_MACHINE,
  BIG_ENDIAN_MACHINE
} endiann_t;

#ifdef __cplusplus
extern "C" {
#endif
  endiann_t GetEndian();

  uint32_t PRGH_GetPrgHandlerVersion();
  uint32_t PRGH_GetLongFromLong(uint32_t value);
  uint8_t PRGH_GetNextElementIndex(char *Memory,uint32_t* current_element_index);
  uint8_t PRGH_GetElementIndex(char *Memory,uint32_t ElementType, uint32_t UID, uint32_t* element_index);
  uint32_t PRGH_VersionCheck(char *Memory,uint32_t ToolElementType, uint32_t CurrentVersion);

  // Toolchain Specific functions
  endiann_t PRGH_GetEndian(char *Memory);
  uint32_t PRGH_FileCheck(char *Memory);
  //uint32_t PRGH_WriteElement(IFWD_MemoryStream *Output,uint32_t Type,uint32_t UID,char* PayloadPtr,uint32_t PayloadSize);
  uint32_t PRGH_FileWriteElement(FILE *fout,IFWD_MemoryStream *Output,uint32_t Type,uint32_t UID,char* PayloadPtr,uint32_t PayloadSize);
  uint32_t PRGH_ReplaceElement(IFWD_MemoryStream *Output,uint32_t Type,uint32_t UID,char* PayloadPtr,uint32_t PayloadSize);
  uint32_t PRGH_FindElementDataSize(uint32_t Type,char* PayloadPtr);
  void PRGH_SwapElementEndian(void *Memory,uint32_t ElemType);

  // FlashTool Specific functions
  void PRGH_SetFileOffset(uint32_t value);
  uint32_t PRGH_FileCheck_File(FILE *fp);
  uint32_t PRGH_VersionCheck_File(FILE *fp,uint32_t ToolElementType, uint32_t CurrentVersion);
  uint8_t PRGH_GetElementIndex_File(FILE *fp,uint32_t ElementType, uint32_t UID, uint32_t *element_index);
  uint8_t PRGH_GetNextElementIndex_File(FILE *fp,uint32_t* current_element_index);
#ifdef __cplusplus
}
#endif

typedef enum
{
  PRG_OK,
  PRG_FAIL,
  PRG_ELEMENT_NOT_FOUND,
  PRG_ELEMENT_SIZE_DIFFER,
  PRG_HASHING_ERROR,
  PRG_FILE_WRITE_ERROR,
  NUM_ERROR_CODES // This should be the last
} prg_handler_ret_type;


#endif

