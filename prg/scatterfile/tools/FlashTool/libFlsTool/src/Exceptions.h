/* -------------------------------------------------------------------------
 Copyright (C) 2013-2014 Intel Mobile Communications GmbH

      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /msw_tools/FlashTool/libFlsTool/src/Exceptions.h $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ----------------------------------------------------------------------
 Description:
   Include to handle exceptions. This file is for supporting -fno-exceptions,
   which is the Android way.
 ---------------------------------------------------------------------- */
#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#include "FileIO.h"
#include "StringExt.h"

#include <string>
#include <vector>

#ifdef ANDROID
  #ifndef LOG_TAG
    #define LOG_TAG "lib_fls"    //< Default Android logcat name
  #endif

  #define LOG_NDEBUG 0         //< Default verbose behaviour
  #define PRINT LOGI

  #include <utils/Log.h>
#else
  #define PRINT printf
#endif

/* Android doesn't support exceptions */
#ifdef __EXCEPTIONS
  #include <stdexcept>

  #define THROW_EXCEPTION_1_ARGS(message) \
    throw std::runtime_error( std::string(fname(__FILE__)) +":"+ Stringify(__LINE__) +" - "+ message )

  #define THROW_EXCEPTION_2_ARGS(message, exit_value) \
    THROW_EXCEPTION_1_ARGS(message)
#else

  class myexception
  {
    string m_msg;
    public:
      myexception(string msg)
        : m_msg(msg) { }

      const char* what() { return m_msg.c_str(); }

      operator bool() const { return !m_msg.empty(); }
  };

  extern std::vector<myexception> chatched_exceptions;

  #define try        if (true)
  #define catch(ex)  if (chatched_exceptions.size() > 0)

  /////////////////////////////////////////
  // TODO: Figure out how to do this     //
  /////////////////////////////////////////

  //#define catch(ex)  if (chatched_exceptions.size() > 0 && (bool)(myexception& e = chatched_exceptions.front()))

  #define THROW_EXCEPTION_1_ARGS(message) \
  { \
    char* filename = fname(__FILE__); \
    chatched_exceptions.push_back( myexception(string(filename) +":"+ Stringify(__LINE__) +" - "+ message) ); \
    PRINT( "%s:%d - %s\n", filename, __LINE__, string(message).c_str() ); \
    return; \
  }

  #define THROW_EXCEPTION_2_ARGS(message, exit_value) \
  { \
    char* filename = fname(__FILE__); \
    chatched_exceptions.push_back( myexception(std::string(filename) +":"+ Stringify(__LINE__) +" - "+ message) ); \
    PRINT( "%s:%d - %s\n", filename, __LINE__, std::string(message).c_str() ); \
    return exit_value; \
  }
#endif

/* Overall defines */
#define GET_3RD_ARG(arg1, arg2, arg3, ...) arg3

#define THROW_EXCEPTION_MACRO(...) \
  GET_3RD_ARG(__VA_ARGS__, THROW_EXCEPTION_2_ARGS, THROW_EXCEPTION_1_ARGS, )

/**
 * Interface define
 */
#define throw_exception(...) \
  THROW_EXCEPTION_MACRO(__VA_ARGS__)(__VA_ARGS__)

#endif
