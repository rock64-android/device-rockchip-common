/*
 *       Copyright (C) 2011-2013 Intel Mobile Communications GmbH
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
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/MemoryStream.h $
 *   Resonsible:
 *   Comment:
 *
 */
#ifndef IFWD_MemoryStreamH
#define IFWD_MemoryStreamH

#include <string>

using namespace std;

class IFWD_MemoryStream
{
private:
  char *m_Memory;

public:
  int  Size;
  int  OccupiedSize; // Only used by Prg Handler
  void *Memory;
  void SetSize(int size);
  void Clear(void);

  void LoadFromFile(const string& FileName);
  bool SaveToFile(const string& FileName);

  //Deprecated
  void LoadFromFile(char *FileName);
  bool SaveToFile(char *FileName);

  IFWD_MemoryStream(int SizeRequest = 0);
  IFWD_MemoryStream(const IFWD_MemoryStream& __copy);     //< Copy constructor (Klocworks)
  ~IFWD_MemoryStream();

  IFWD_MemoryStream& operator=(const IFWD_MemoryStream& __copy); //< Class clone (Klocworks)
};

#endif



