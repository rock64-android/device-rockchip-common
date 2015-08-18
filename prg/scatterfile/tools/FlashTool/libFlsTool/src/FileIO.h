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
 * Description: Interface for: IFWD File handling wrappers
 *
 *
 * Revision Information:
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/FileIO.h $
 *   Resonsible:
 *   Comment:
 *
 */

#ifndef DLL_FileIOH
#define DLL_FileIOH
/*-------------------------------------------------------------------------------*/

#include "OS.h"
#include <time.h>

#ifdef _WIN32
  #include "StringExt.h"

  char* realpath(const char* path, char* buffer);

  #define lstat stat
  #define SEPARATOR '\\'
#else
  #define SEPARATOR '/'
#endif

#ifdef CUSTOM_FS
  typedef void FILE;
#else
  #include <stdio.h>
#endif


#define path_is_relative(name) (strchr(name, SEPARATOR))

#define path_is_complete(name) ((name[0] && name[0] == '/') || (name[1] && name[1] == ':'))

/*-------------------------------------------------------------------------------*/
typedef enum
{
  FILE_status_ok = 0,
  FILE_status_file_not_found,
  FILE_status_open_error,
  FILE_status_read_access_error,
  FILE_status_write_access_error,
  FILE_status_EOF
} FILE_status_enum;

typedef enum
{
  FILE_open_mode_read = 0,
  FILE_open_mode_write_append,
  FILE_open_mode_write_replace,
  FILE_open_mode_write_create  /* If file exists it will be reset to size 0 byte */
} FILE_open_mode_enum;


/**
 * @brief Enables external file handling in case customer does not want the DLL to access the real file system, but their own.
 */
typedef struct
{
  void *(*fopen)(const char *filename, const char *mode);   /**< function pointer to custom fopen handling */
  int (*fseek)(void *stream, long offset, int origin);      /**< function pointer to custom fseek handling */
  long(*ftell)(void *stream);                               /**< function pointer to custom ftell handling */
  int (*fclose)(void *stream);                              /**< function pointer to custom fclose handling */
  int (*Feof)(void *stream);                                /**< function pointer to custom feof handling */
  size_t (*fread)(void *buffer, size_t size, size_t count, void *stream);   /**< function pointer to custom fread handling */
  size_t (*fwrite)(const void *buffer, size_t size, size_t count, void *stream);   /**< function pointer to custom fwrite handling */
  int (*remove)(const char *path);   /**< function pointer to custom remove handling */
  char(*fgets)(char *line, int maxline, void *stream);  /**< function pointer to custom fgets handling */
} IFWD_ExtFileHandleType;

/*-------------------------------------------------------------------------------*/
#ifdef __cplusplus

  /* exporting the C++ functions */
  #include <string>
  #include <vector>
  using namespace std;

  string FILE_getContent(std::string path);
  char* FILE_getContent(string path, std::vector<char> &buffer, uint64_t length = 0, uint64_t offset = 0);
  uint8_t* FILE_getContent(string path, std::vector<uint8_t> &buffer, uint64_t length = 0, uint64_t offset = 0);

  void FILE_setContent(string path, char c, size_t size);
  void FILE_setContent(string path, std::vector<char> &buffer);
  void FILE_setContent(string path, std::vector<uint8_t> &buffer);

  void save_stream_to_file(istream& stream, string output, size_t size);

  extern "C" {
#endif

  /* Not used externally (only inside the DLL) */
  extern IFWD_ExtFileHandleType *IFWD_GLOBAL_ExtFileHandle;

  extern              char *FILE_getContent(const char* filename, size_t *size);
  extern              uint8_t  FILE_setContent(const char* filename, const char* content, size_t size);

/* STAT - Move to FileSystem.cpp - START */
  // extern              int  FileSystem_path(const char *path);
  extern              int  FileSystem_mkdir(const char *path);
  extern              int  FileSystem_rmdir(const char *path);
  extern              int  FileSystem_remove(const char *path);
  extern              int  FileSystem_copy(const char *src_file, const char *dest_file);
  extern              int  FileSystem_move(const char *src_file, const char *dest_file);
  extern       const char *FileSystem_get_name(const char* _path);
  extern       const char *FileSystem_get_dir(char* _path);

  extern             void FileSystem_free_string_array(char **list);
  extern             char **FileSystem_dirlist(const char* _path, uint8_t fullpath);
#if 0 // TODO: Removed due to memory leaks
  extern             char **FileSystem_find(const char* path, const char* str, uint8_t recursive);
#endif

  extern              int  fexists(const char *filename);         //< Does the path exist?
  extern              uint8_t FileSystem_isfile(const char* path);  //< Is path file?
  extern              uint8_t FileSystem_isdir(const char* path);   //< Is path directory?

  extern             time_t fcreated(const char *filename);       //< Time of creation
  extern             time_t fmodified(const char *filename);      //< Time of last modification

  extern       const char* fusername(const char* path);
  extern       const char* fgroupname(const char* path);
  extern              void fset_username(const char* path, const char* new_username);
  extern              void fset_groupname(const char* path, const char* new_groupname);

  extern            uint8_t faccess(const char *filename);        //< Can this user access the file?
  extern           uint32_t fsize(const char *filename);          //< How big is the file?
  extern              char *fcomplete(const char *filename);     //< What is the full filename?
  extern              char *fdir(const char *filename);           //< What is the filename directory?
  extern              char *fname(const char *filename);          //< What is the filename?
  extern              char *fext(const char *filename);           //< What is the filename extension?
/* END - Move to FileSystem.cpp - END */

  extern             FILE  *FILE_open(const char *file_name, FILE_open_mode_enum open_mode);
  extern FILE_status_enum   FILE_close(FILE *handle);
  extern         uint32_t   FILE_size(FILE *handle);
  extern          uint8_t   FILE_exists(const char *file_name);
  extern FILE_status_enum   FILE_read(FILE *handle, uint8_t *mem_pointer, uint32_t length);
  extern FILE_status_enum   FILE_write(FILE *handle, uint8_t *mem_pointer, uint32_t length);
  extern FILE_status_enum   FILE_seek(FILE *handle, uint32_t offset);
  extern          int32_t   FILE_pos(FILE *handle);
  extern             char  *FILE_ExtractFileName(char *filename);
  extern             char  *FILE_ExtractFileExt(char *filename);
  extern          uint8_t   FILE_ReadLine(FILE *in, char *dest, uint32_t BufferSize);
  extern             void   FILE_WriteLine(FILE *out, char *string);
  extern          uint8_t   FILE_EOF(FILE *in);
  extern             char  *FILE_get_last_error(void);
  extern FILE_status_enum   FILE_flush(FILE *handle);

#ifdef __cplusplus
}
  /**
   * Get a system temporary directory for the process.
   *
   * Dirname is default "tmp-XXXXXX". The X's will be replaced by
   * a unique identifier & a new directory will be created.
   * All subsequent calls without a dirname will give the same name.
   */
  string FileSystem_tmpdir(string dirname = "");

  /**
   * Construct a path from several subpaths
   */
  string FileSystem_path(const string& path1, const string& path2,
                         const string& path3 = "", const string& path4 = "");

  /* exporting the C++ functions */
  inline string fcomplete(string path)
  {
    char* fpath = fcomplete(path.c_str());
    string p = fpath ? fpath : "";
    if (fpath) free(fpath);
    return p;
  }

  inline string fdir(string path)
  {
    char* dir = fdir(path.c_str());
    string d = dir ? dir : "";
    if (dir) free(dir);
    return d;
  }

  inline int     fexists(string path)   { return fexists(path.c_str()); }
  inline uint8_t faccess(string path)   { return faccess(path.c_str()); }
  inline time_t  fmodified(string path) { return fmodified(path.c_str()); }
  inline time_t  fcreated(string path) { return fcreated(path.c_str()); }
  inline const char* fusername(string path) { return fusername(path.c_str()); }
  inline size_t  fsize(string path)      { return fsize(path.c_str()); }
  inline char*   fname(string path)      { return fname(path.c_str()); }
  inline char*   fext(string path)       { return fext(path.c_str()); }

  inline int  FileSystem_mkdir(string path)  { return FileSystem_mkdir(path.c_str()); }
  inline int  FileSystem_rmdir(string path)  { return FileSystem_rmdir(path.c_str()); }
  inline int  FileSystem_remove(string path)  { return FileSystem_remove(path.c_str()); }
  inline int  FileSystem_copy(string from, string to)  { return FileSystem_copy(from.c_str(), to.c_str()); }
  inline int  FileSystem_move(string from, string to)  { return FileSystem_move(from.c_str(), to.c_str()); }
  inline uint8_t FileSystem_isfile(string path) { return (fexists(path.c_str()) == 1); }
  inline uint8_t FileSystem_isdir(string path)  { return (fexists(path.c_str()) == 2); }

#endif


/*-------------------------------------------------------------------------------*/

#endif
