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
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/StringExt.h $
 *   Resonsible:
 *   Comment:
 *
 */
#ifndef _STRING_EXT_H_
#define _STRING_EXT_H_

#include <stdio.h>
#include <stdlib.h>       //< malloc()
#include <ctype.h>        //< isspace()
#include <stdarg.h>       //< va_list()
#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
  #include <string>
  #include <sstream>
  #include <iomanip>
  //#include <cstdint>

  #ifdef __EXCEPTIONS
    #include <stdexcept>
  #endif

  using namespace std;

  int stoi(const string& str);
  string itos(uint32_t obj);

  template<typename T>
  T StringTo(const string& str)
  {
    T x = 0;
    istringstream iss(str);
    if (str.empty())
      x = 0;
    else if (str.find("0x") == 0)
      iss >> hex >> x;
    else
      iss >> x;
    return x;
  }

  string StringToLower(string str);
  string StringToUpper(string str);

  template<typename T>
  string Stringify(const T& obj, int width = 0)
  {
    stringstream ss;
    if (width) ss.precision(width);
    if (!(ss << setw(width) << setfill('0') << obj))
    {
      #ifdef __EXCEPTIONS
        throw logic_error("Could not stringify!");
      #else
        fprintf(stderr, "Could not stringify!");
      #endif
    }
    return ss.str();
  }

  string getHexString(uint64_t value, int width = 8, char fill = '0');
  string getIntString(uint64_t value, int width = 0, char fill = '0');

  bool StringCaseCompare(const string& str1, const string& str2);
  bool replace(string& str, const string& from, const string& to);

  string& TrimRight(string& s, const string& delimiters = " \f\n\r\t\v" );
  string& TrimLeft(string& s, const string& delimiters = " \f\n\r\t\v" );
  string& Trim(string& s, const string& delimiters = " \f\n\r\t\v" );


extern "C" {
#endif

  #ifdef _WIN32
    char* strndup(const char* s, size_t n);
  #endif

  /* Convert an array of chars to lower case */
  char* str_to_lower(char* str);


  /* Strip whitespace chars off end of given string, in place. Return s. */
  char* rtrim(char* s);

  /* Return pointer to first non-whitespace char in given string. */
  char* ltrim(const char* str);

  char* trim(char* str);

  /**
   *  Version of strncpy that ensures dest (size bytes) is null-terminated.
   **/
  char* strncpy0(char* dest, const char* src, size_t size);

  /**
   * Wrapper for snprintf for returning zero when the buffer is too small
   **/
  size_t snprintf0(char* str, size_t size, const char* append, ...);

  /**
   * Return pointer to byte *AFTER* NULL-termination
   * which is NOT standard stpcpy
   **/
  char *stpcpy0(char *dest, const char *src);

  /**
   * Locates any of the delimiters in the string str.
   * Returns the position of the last found delimiter.
   */
  int strfind(char* str, const char* delimiters);

  /**
   * Split a string on a number of delimiters or just a single.
   * Returns a newly allocated list of strings, which have been splitted.
   */
  char** strsplit(const char* str, const char* delimiters);

#ifdef __cplusplus
}
#endif

#endif
