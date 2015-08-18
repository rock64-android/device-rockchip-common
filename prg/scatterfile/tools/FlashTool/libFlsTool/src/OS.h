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
 *	This file can be used in Windows specific code, in order to change the default windows.h
 *	into known Linux & Mac source, without actually changing the source.
 *
 *
 * Revision Information:
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/OS.h $
 *   Resonsible:
 *   Comment:
 *
 */
#ifndef _IMC_OS_H
#define _IMC_OS_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#ifdef __GNUC__
  #include <errno.h>
  #include <ctype.h>        //< isdigit()
  #include <unistd.h>       //< access()
  #include <stdint.h>
  #include <sys/stat.h>     //< stat, mkdir()
  #include <sys/types.h>    //< types
#endif

#ifndef true
  #define true  1
  #define false 0
#endif

#ifndef __func__
  #define __func__ __FUNCTION__
#endif

#define PARAM_NOT_USED(X)

#define ISPATHDELIMITER(CH) ((CH)==(CHAR)'\\' || (CH)==(CHAR)'/')

#ifndef EXTERN_C
  #ifdef __cplusplus
    #define EXTERN_C extern "C"
  #else
    #define EXTERN_C
  #endif
#endif

EXTERN_C const char* GetErrorString(int error_code);
EXTERN_C char* GetTimestamp(time_t rawtime);
// EXTERN_C char* GetUserHomePath();

#ifdef __cplusplus
  #include <string>

  std::string GetUserHomePath();
#endif

/**
 * OS Specific defines (Mac & Linux)
 **/
#ifdef WIN32
  #include <windows.h>
  #include <direct.h>

  #define SEPARATOR '\\'
  #define SecLibrary "\\IFWD_Coder.dll"
  extern char dll_file_path[];

  #define WaitForMutex( mutex )   (void)WaitForSingleObject(mutex, INFINITE)
  #define CloseMutex( mutex )     CloseHandle(mutex)

  #ifndef DirExists
    #define DirExists          !access(dir, 0)
  #endif

  #ifndef DirCreate
    #define DirCreate(dir)     mkdir(dir)
  #endif

  #if defined (__BORLANDC__) || (LINT)
    #define strnlen(str,len)   strlen(str)
  #endif

  // "strnlen" is not defined in string.h for "Mingw32 version <= 3.18"
  #if defined(__MINGW32__) && !defined(_INC_STRING) && \
      ((!defined(_XOPEN_SOURCE)  || _XOPEN_SOURCE < 700) || \
      (!defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L))
//      (__GNUC__ < 5 && __GNUC_MINOR__ < 8 && __GNUC_PATCHLEVEL__ < 3)

    EXTERN_C size_t strnlen(const char* str, size_t len);
    #define STRNLEN_DEFINED

  #endif

#elif defined (__CC_ARM)

//strdup is posix - not std C/C++
EXTERN_C char *strdup(const char *s);

#else //Fall back to plain Linux build
  #include <dlfcn.h>        //< dlopen()
  #include <pthread.h>
  #include <time.h>
  #include <sys/time.h>

  /* Filesystem Functions */
  #define DirectoryExists(dir)   DirExists(dir)
  #define DirExists(dir)         !access(dir, 0)
  #define DirCreate(dir)         mkdir(dir, 0777)

  #define MkDir(path)            mkdir(path, 0777)
  #define ForceDirectories(path) mkdir(path, 0777)


  /* Library functions */
  #define LoadLibrary( file )                   dlopen(file, RTLD_LAZY)
  #define LoadLibraryA( file )                  dlopen(file, RTLD_LAZY)
  #define LoadLibraryW( file )                  dlopen(file, RTLD_LAZY)
  #define FreeLibrary( handle )                 dlclose( handle )
  #define GetProcAddress( handle, func )        dlsym( handle, func )
  //#define FlashLessLibrary "/libIFWD_Coder.so"

  #define SEPARATOR '/'
  #define INFINITE 0xFFFFFFFF
  #define HINSTANCE void*
  #define DWORD int
  #define LPVOID void*
  #define DLL_PROCESS_ATTACH 0
  #define DLL_PROCESS_DETACH 1
  #define WINAPI

  #ifndef MAX_PATH
    #define MAX_PATH  256
  #endif

  #ifndef Sleep
    #define Sleep(ms) usleep(ms*1000)
  #endif

  /*lint -e18 */
  typedef struct mutex_handle_t {
     int set;
     pthread_cond_t cond;
     pthread_mutex_t mutex;
  } HANDLE;
  /*lint +e18 */

  /* Mutex functions */
  #define CreateMutex( handle )   pthread_mutex_init( &handle.mutex, NULL )
  #define WaitForMutex( handle )  pthread_mutex_lock( &handle.mutex )
  #define ReleaseMutex( handle )  pthread_mutex_unlock( &handle.mutex )
  #define CloseMutex( handle )    pthread_mutex_destroy( &handle.mutex )

  #define CreateEvent( event ) { \
     event.set = 0; \
     pthread_mutex_init( &event.mutex, NULL ); \
     pthread_cond_init( &event.cond, NULL ); \
  }

#ifdef ANDROID
  #define WaitForSingleObject( event, dummy_param ) \
  do { \
     pthread_mutex_lock( &event.mutex ); \
     while(event.set == 0) { \
        pthread_cond_wait( &event.cond, &event.mutex ); \
     } \
     event.set = 0; \
     pthread_mutex_unlock( &event.mutex ); \
  } while (0)
#else
#if 0
  #define WaitForSingleObject( event, ms ) \
    do { \
    pthread_mutex_lock( &event.mutex ); \
    if (ms) { \
      struct timeval tp; \
      struct timespec ts; \
      gettimeofday( &tp, NULL ); \
      ts.tv_sec  = tp.tv_sec; \
      ts.tv_nsec = tp.tv_usec * 1000; \
      ts.tv_sec += (time_t)ceil(ms/100.00); \
      while (!event.set) \
        pthread_cond_timedwait( &event.cond, &event.mutex, &ts ); \
    } else { \
      while (!event.set) \
        pthread_cond_wait( &event.cond, &event.mutex ); \
    } \
    event.set = 0; \
    pthread_mutex_unlock( &event.mutex ); \
  } while (0)
#else
  #define WaitForSingleObject( event, ms ) \
    do { \
      pthread_mutex_lock( &event.mutex ); \
      struct timeval tp; \
      struct timespec ts; \
      gettimeofday( &tp, NULL ); \
      ts.tv_sec  = tp.tv_sec; \
      ts.tv_nsec = tp.tv_usec * 1000; \
      ts.tv_sec += (time_t)ceil(ms/100.00); \
      int rc = ETIMEDOUT; \
      while (!event.set && rc == ETIMEDOUT) \
        rc = pthread_cond_timedwait( &event.cond, &event.mutex, &ts ); \
      event.set = 0; \
      pthread_mutex_unlock( &event.mutex ); \
    } while (0)
#endif
#endif

  #define SetEvent( event ) { \
     pthread_mutex_lock( &event.mutex ); \
     event.set = 1; \
     pthread_cond_signal( &event.cond ); \
     pthread_mutex_unlock( &event.mutex ); \
  }

  #define ResetEvent( dummy ) {}

  #define CloseHandle( handle ) { \
     pthread_cond_destroy( &handle.cond ); \
     pthread_mutex_destroy( &handle.mutex ); \
  }

  /* Time functions */
  EXTERN_C uint32_t GetTickCount( void );
  #define TICK_COUNT_DEFINED

#endif

#endif
