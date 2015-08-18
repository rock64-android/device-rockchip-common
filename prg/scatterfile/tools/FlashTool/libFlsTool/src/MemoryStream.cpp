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
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/MemoryStream.cpp $
 *   Resonsible:
 *   Comment:
 *
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>   //memcpy()
#include <string>

#include "MemoryStream.h"
#include "FileIO.h"


using namespace std;

/*---------------------------------------------------------------------------*/
IFWD_MemoryStream::IFWD_MemoryStream(int SizeRequest)
{
  Size = 0;
  OccupiedSize = 0;
  m_Memory = NULL;
  Memory = (void*)m_Memory;
  if (SizeRequest > 0)
    SetSize( SizeRequest );
}
/*---------------------------------------------------------------------------*/

IFWD_MemoryStream::~IFWD_MemoryStream()
{
  Clear();
}

/*---------------------------------------------------------------------------*/
void IFWD_MemoryStream::Clear(void)
{
  if(m_Memory)
  {
    delete []m_Memory;
    m_Memory = NULL;
    Memory = NULL;
    Size = 0;
    OccupiedSize = 0;
  }
}

IFWD_MemoryStream::IFWD_MemoryStream(const IFWD_MemoryStream& __copy)
{
  Memory = m_Memory = NULL;
  if(__copy.m_Memory)
  {
    m_Memory = new char[__copy.Size];
    memcpy(m_Memory, __copy.m_Memory, __copy.Size);
    Memory = (void*)m_Memory;
  }
  Size = __copy.Size;
  OccupiedSize = __copy.OccupiedSize;
}

IFWD_MemoryStream& IFWD_MemoryStream::operator=(const IFWD_MemoryStream& __copy)
{
  if(this != &__copy)
  {
    Memory = m_Memory = NULL;
    if(__copy.m_Memory)
    {
      m_Memory = new char[__copy.Size];
      memcpy(m_Memory, __copy.m_Memory, __copy.Size);
      Memory = (void*)m_Memory;
    }
    Size = __copy.Size;
    OccupiedSize = __copy.OccupiedSize;
  }
  return *this;
}

/*---------------------------------------------------------------------------*/

void IFWD_MemoryStream::SetSize(int size)
{
  if(size <= 0)
    Clear();
  else
  {
    if(size != Size) // any difference?
    {
      char *NewMemory = new char[size + 1]; // Allocate memory for new size
      if(m_Memory) // any old memory data to copy?
      {
        int copyBytes;
        if(size > Size) // new size bigger than old?
        {
          copyBytes = Size;
          memset(NewMemory + Size, 0, size - Size); // clear all of new memory that we are not copying from old memory block
        }
        else
          copyBytes = size;
        memcpy(NewMemory, m_Memory, copyBytes); // copy as much of the old data as possible
      }
      else
        memset(NewMemory, 0, size);
      delete []m_Memory;
      Size = size;
      m_Memory = NewMemory;
      Memory = (void*)m_Memory;
    }
  }
}
/*---------------------------------------------------------------------------*/

void IFWD_MemoryStream::LoadFromFile(char *FileName)
{
  string file(FileName);
  LoadFromFile(file);
}

void IFWD_MemoryStream::LoadFromFile(const string& FileName)
{
  FILE *in;
  Clear();

  if((in = FILE_open((char*)FileName.c_str(), FILE_open_mode_read)) == NULL)
    return;
  Size = FILE_size(in);
  m_Memory = new char[Size + 1]; // Allocate memory for file
  if(m_Memory)
  {
    FILE_read(in, (uint8_t*)m_Memory, Size); // read it
  }
  Memory = (void*)m_Memory;
  FILE_close(in);
}

/*---------------------------------------------------------------------------*/
bool IFWD_MemoryStream::SaveToFile(char *FileName)
{
  string file(FileName);
  return SaveToFile(file);
}

bool IFWD_MemoryStream::SaveToFile(const string& FileName)
{
  FILE *out;
  if((out = FILE_open((char*)FileName.c_str(), FILE_open_mode_write_replace)) == NULL)
    return false;
  if(m_Memory)
    FILE_write(out, (uint8_t*)m_Memory, Size); // write to file
  FILE_close(out);

  return true;
}
/*---------------------------------------------------------------------------*/


