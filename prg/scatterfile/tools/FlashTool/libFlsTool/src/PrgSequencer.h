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
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/PrgSequencer.h $
 *   Resonsible:
 *   Comment:
 *
 */
#ifndef PrgSequencerH
#define PrgSequencerH

#include "header.h"
#include "MemoryStream.h"

#include <stdio.h>
#include <stdint.h>

#if defined(_WIN32)
	#include <mem.h>
#elif defined(__CC_ARM)
	#include <stdlib.h>
#else
	#include <memory.h>
#endif

//---------------------------------------------------------------------------

#define PRG_SEQUENCER_VERSION PRGSEQUENCER_TOOL_REQ_VER

#define MAX_NUM_ELEMENTS 200

class PRGSequencer
{
  private:
    IFWD_MemoryStream *Input;
    void (*NewElementFunc)(void* ElementPtr);
    uint32_t FileHash[5];
    uint32_t HeaderOffset;
    char *MemPtr;
    bool AddToGlobalStruct(char* Mem);
    bool AllocateAndCopy(char* Mem, uint32_t ElemType);
    void TamperDataAfterRead(uint32_t ElemType,char*Mem);
    bool WriteElementToFile(IFWD_MemoryStream* Output, uint32_t element_type,uint32_t UID,char* Mem);
    void InitializeElement(void *mem, uint32_t ElemType,int size);
    uint32_t WriteToolElementToFile(IFWD_MemoryStream* Output, uint32_t ToolElemType, uint32_t ReqVer, uint32_t UsedVer);
    uint32_t FindFileSizeNeeded();
    uint32_t FindElementSize(uint32_t element_type,char* Mem);
    FILE *fout;
	char* m_filename;
	public:
    void* ElemPtrs[MAX_NUM_ELEMENTS];
    int ElemCount;
    uint32_t MachEndian;
    uint32_t CallingTool;
    uint32_t AddingNewElement;
    uint32_t CurrentFileType;
    PRGSequencer();
    PRGSequencer(const PRGSequencer& __copy);     //< Copy constructor (Klocworks)
    PRGSequencer& operator=(const PRGSequencer& __copy); //< Class clone (Klocworks)

    ~PRGSequencer();
    bool IsDynamicElement(uint32_t ElemType);
    void SetCallBackFunc(void (*NewElementF)(void* ElementPtr));
    uint32_t ReadFile(char* Filename, uint32_t ElementType, uint32_t ElementVersion,void (*NewElementF)(void* ElementPtr));
    uint32_t WriteFile(char* Filename);
    void OpenFile(char* Filename);
    void CloseFile(void);
    void* CreateElement(uint32_t ElementType);
    bool RemoveElement(void *element_ptr);
    void CleanUp();
    uint32_t CompareHash();
    void TamperToolDataBeforeWrite(uint32_t MakePrgVersion);
    char* FindElementStructure(uint32_t element_type,uint32_t UID);
    uint32_t GetFileLength();

    // Only for Storage Tool
    void* CreateStorageElement(uint32_t ElementType, char*data_ptr, uint32_t data_len);
    uint32_t WriteStorageFile(char* Filename,uint32_t version);
};
#endif
