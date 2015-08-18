/*
 *       Copyright (C) 2011-2014 Intel Mobile Communications GmbH
 *
 *            Sec Class: Intel Confidential (IC)
 *
 *
 * This document contains proprietary information belonging to IMC.
 * Design A/S. Passing on and copying of this document, use and communication
 * of its contents is not permitted without prior written authorisation.
 *
 * Description:
 *   IFWD File handling wrappers
 *   n:\dwdsrc_int\tools\FlashTool\download_dll\Documents\Download_DLL_Internal_IF_spec.doc
 *
 * Revision Information:
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/FileIO.cpp $
 *   Resonsible: njk
 *   Comment:
 *     SMS01224729: "Flashloader logging is very slow (7 times slower)"
 */
#define _GNU_SOURCE 1    //< strndup()

#include "OS.h"
#include "FileIO.h"
#include "StringExt.h"   //< strsplit()
#include "Exceptions.h"  //< throw_exception()

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>      //< MAXPATHLEN

#ifdef WIN32
  #include "accctrl.h"
  #include "aclapi.h"
#else
  #include <pwd.h>
  #include <grp.h>
#endif

#ifndef LIBFLSTOOL
/**
 * NOTE; THIS IS A FIX BECAUSE 6360 FORGOT TO INCLUDE StringExt.h & OS.h
 *
 * TODO; This should be removed in the next version
 */
const char* GetErrorString(int error_code)
{
  static char strError[256];
  char* ptr = strError;

  #ifdef WIN32
    int err = GetLastError();
    //strerror_s(strError, sizeof(strError), err);
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, err,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  ptr, sizeof(strError), 0);

    char* nl = strchr(strError, '\r');
    if (nl) nl = '\0';
  #else
    int err = errno;
    ptr = strerror_r(err, strError, sizeof(strError));
    if (!error_code)
      snprintf(strError, sizeof(strError), "%s", ptr);
  #endif

  if (error_code)
    snprintf(strError, sizeof(strError), "%s (%d)", ptr, err);

  return strError;
}

int strfind(char* str, const char* delimiters)
{
  char* pos = NULL;
  if (!delimiters || !str) return -1;

  for(size_t i=0; i<strlen(delimiters); i++)
    if ((pos = strchr(str, delimiters[i])) != NULL)
      break;

  return pos ? strlen(str)-strlen(pos) : -1;
}

char** strsplit(const char* _str, const char* delimiters)
{
  if (!delimiters || !_str) return NULL;

  char** final = NULL;
  int pos, tmp, i=0;
  char* list[64+1];
  char* str = strdup(_str);

  if (strfind(str, delimiters) == -1)
    list[i++] = str;
  else
  {
    for (pos=0; (tmp = strfind(&str[pos], delimiters)) >= 0;)
    {
      if (pos==0 && tmp > 0)
        list[i++] = &str[pos];
      pos += tmp;
      str[pos++] = 0;
      list[i++] = &str[pos];
      if (i==64)
        break;
    }
  }
  list[i++] = NULL;

  final = (char**)malloc(sizeof(char*)*i);
  if (final)
  {
    for(pos=0; pos<i;pos++)
      final[pos] = list[pos] ? strdup(list[pos]) : NULL;
  }
  free(str);

  return final;
}
#endif



IFWD_ExtFileHandleType *IFWD_GLOBAL_ExtFileHandle = NULL;

#ifdef __cplusplus
  #include <iostream>
  #include <fstream>
  #include <sstream>
  #include <vector>
  #include <string>
  #include <map>

  using namespace std;

#endif

/**
 *  Use this tag to see whenever a file is:
 *     opened, closed, written or read from
 **/
//#define FILE_IO_DEBUGGING

#ifdef FILE_IO_DEBUGGING
  map<int, string> file_table;
#endif


#ifndef __CC_ARM
#include <dirent.h>
#include <sys/stat.h>

#ifdef WIN32
  char* realpath(const char* path, char* buffer)
  {
    if (!buffer)
      buffer = (char*)malloc(sizeof(char)*MAX_PATH);
    GetFullPathName(path, MAX_PATH, buffer, NULL);
    return buffer;
  }
#endif

/*
 * Easier, and less consuming functions...
 *
 * Asking the filesystem instead of opening the files first.
 * This way we are "almost" not even using the harddrive :)
 *
 */
int fexists(const char* path)
{
  if (!path)
    return false;

  size_t len = strnlen(path, 512);
  if (path[len-1] == SEPARATOR)
    return 2;

  if (access(path, 0) == 0)
  {
    struct stat info;
    stat(path,&info);

    if (info.st_mode & S_IFREG)
      return 1;

    if (info.st_mode & S_IFDIR)
      return 2;

    return 3;
  }
  return false;
}

uint8_t FileSystem_isfile(const char* path)
{
  return (fexists(path) == 1);
}

uint8_t FileSystem_isdir(const char* path)
{
  return (fexists(path) == 2);
}

time_t fcreated(const char *filename)
{
  struct stat st;
  if(stat(filename, &st) == 0)
    return st.st_ctime;
  return 0;
}

time_t fmodified(const char *filename)
{
  struct stat st;
  if(stat(filename, &st) == 0)
    return st.st_mtime;
  return 0;
}

const char* fusername(const char* path)
{
#ifdef WIN32
  uint8_t ret;
  HANDLE hFile;
  PSID pSidOwner = NULL;
  PSECURITY_DESCRIPTOR pSD = NULL;

  static char UserName[256];
  static char DomainName[256];
  size_t buffer_size = sizeof(UserName);
  SID_NAME_USE eUse = SidTypeUnknown;

  hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    throw_exception("Could not open "+ string(path));

  // Get the owner SID of the file.
  // #define SE_FILE_OBJECT              0x01
  // #define OWNER_SECURITY_INFORMATION  0x01

  if (GetSecurityInfo(hFile, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION,
                  &pSidOwner, NULL, NULL, NULL, &pSD) != ERROR_SUCCESS)
    throw_exception("Could not get security info for "+ string(path));

  ret = LookupAccountSid(
          NULL,                   // name of local or remote computer
          pSidOwner,              // security identifier
          UserName,               // account name buffer
          (LPDWORD)&buffer_size,   // size of account name buffer
          DomainName,             // domain name
          (LPDWORD)&buffer_size, // size of domain name buffer
          &eUse);                 // SID type

  if (!ret)
    throw_exception("Could not lookup account SID for "+ Stringify(pSidOwner));

  CloseHandle(hFile);

  return UserName;
#else
  struct stat info;
  struct passwd *pw = NULL;

  if(stat(path, &info) == 0)
    pw = getpwuid( info.st_uid );

  return (pw ? pw->pw_name : "");
#endif
}

void fset_username(const char* path, const char* new_username)
{
  #ifndef WIN32
    struct passwd *pwd;
    pwd = getpwnam(new_username);

    if (!pwd)
      throw_exception("Could not find username "+ string(new_username));

    if (chown(path, pwd->pw_uid, -1) == -1)
      throw_exception("Could not set owner of file "+ string(path));
  #else
    throw_exception("GenericFile::fset_username() not yet implemented");
  #endif
}

const char* fgroupname(const char* path)
{
#ifdef WIN32
  return fusername(path);
#else
  struct stat info;
  struct group *grp = NULL;

  if(stat(path, &info) == 0)
    grp = getgrgid( info.st_gid );

  return (grp ? grp->gr_name : "");
#endif
}

void fset_groupname(const char* path, const char* new_groupname)
{
  #ifndef WIN32
    struct group *grp;
    grp = getgrnam(new_groupname);

    if (!grp)
      throw_exception("Could not find groupname "+ string(new_groupname));

    if (chown(path, -1, grp->gr_gid) == -1)
      throw_exception("Could not set group of file "+ string(path));
  #else
    throw_exception("GenericFile::fset_groupname() not yet implemented");
  #endif
}

uint32_t fsize(const char *filename)
{
  struct stat st;
  if(stat(filename, &st) == 0)
    return (uint32_t)st.st_size;
  return 0;
}

uint8_t faccess(const char *filename)
{
  FILE *in;
  if(filename && ((in = fopen(filename, "rb")) != NULL))
  {
    fclose(in);
    return true;
  }
  return false;
}
uint8_t faccess_mode(const char *filename, const char *mode)
{
  FILE *in;
  if(filename && ((in = fopen(filename, mode)) != NULL))
  {
    fclose(in);
    return true;
  }
  return false;
}

/**
 * Returns a newly allocated string with the file path.
 *
 * Even if the file doesn't exist we manipulate the filename string.
 *
 * Windows:
 *   Tchar NPath[MAX_PATH];
 *   GetCurrentDirectory(MAX_PATH, NPath);
 */
char* fcomplete(const char *filename)
{
  char fullpath[MAX_PATH];

  if (path_is_complete(filename))
    snprintf(fullpath, sizeof(fullpath), "%s", filename);
  else
  {
    #ifdef _WIN32
      char PWD[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, PWD);
    #else
      char* PWD = getenv("PWD");
      if (!PWD)
        PWD = (char*)".";
    #endif
    snprintf(fullpath, sizeof(fullpath), "%s/%s", PWD, filename);
  }

  if (fexists(fullpath))
    return realpath(fullpath, NULL);
  else
    return strdup(fullpath);
}

/*
 *  Return the directory of a file path
 */
char* fdir(const char* path)
{
  const char* name = strrchr(path, SEPARATOR);
  if (name)
    return strndup( path, strlen(path)-strlen(name) );
  return strdup( path );
}

char* fname(const char* orig)
{
  // The path string may be saved to FLS file on a different OS
  // ... therefore check for windows or linux separator.
  const char* pathw = strrchr(orig, '\\');
  const char* pathl = strrchr(orig, '/');

  // The shortest resulting path (highest address) must be the corect one.
  if (pathw > pathl)
  {
    if (pathw && (pathw+1))
      return (char*)(pathw+1);
    return (char*)orig;
  }
  else
  {
    if (pathl && (pathl+1))
      return (char*)(pathl+1);
    return (char*)orig;
  }

}

char* fext(const char* _path)
{
  const char* ext = strrchr(fname(_path), '.');
  if (ext)
    return (char*)(ext+1);
  return (char*)"";
}


string FileSystem_path(const string& path1, const string& path2,
                       const string& path3, const string& path4)
{
  string path = path1;
  if (path[path.size()-1] != SEPARATOR && path2[0] != SEPARATOR)
    path.append(1,SEPARATOR);
  path += path2;

  if (!path3.empty())
  {
    if (path[path.size()-1] != SEPARATOR && path3[0] != SEPARATOR)
      path.append(1,SEPARATOR);
    path.append(path3);
  }
  if (!path4.empty())
  {
    if (path[path.size()-1] != SEPARATOR && path4[0] != SEPARATOR)
      path.append(1,SEPARATOR);
    path.append(path4);
  }

  return path;
}

int FileSystem_mkdir(const char *path)
{
  if (!path)
    return false;

  int i=0, ret=true;
  char sep[2] = { SEPARATOR };
  char**dirs = strsplit(path, sep);
  if (!dirs)
    return false;

  #ifdef WIN32
    string current = path_is_complete(path) ? dirs[i++] : ".";
  #else
    string current = path_is_complete(path) ? "" : ".";
  #endif

  for(; dirs[i]; ++i)
  {
    current += SEPARATOR;
    current += dirs[i];
    free(dirs[i]);
    if (fexists(current.c_str()))
      continue;

    #ifdef WIN32
      if (CreateDirectory(current.c_str(), 0) == 0)
        ret = false;
    #else
      if (mkdir(current.c_str(), 0777) != 0)
        ret = false;
    #endif
  }
  if (dirs)
    free(dirs);
  return ret;
}

int FileSystem_rmdir(const char* path)
{
  int              ret;
  DIR             *dir;
  struct dirent   *entry;

  if (!FileSystem_isdir(path))
    return false;

  if(!(dir = opendir(path))) { perror("opendir"); return false; }
  for(entry = readdir( dir); entry; entry = readdir( dir))
  {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;

    static char     spath[MAXPATHLEN] = "";
    snprintf(spath, sizeof(spath), "%s/%s", path, entry->d_name);

    if (FileSystem_isdir(spath))
      ret = FileSystem_rmdir(spath);
    else
      ret = (unlink(spath) == 0);

    if (!ret)
    {
      closedir(dir);
      return false;
    }
  }

  closedir(dir);
  return (rmdir(path) == 0);
}

int FileSystem_remove(const char* file)
{
  return (remove(file) == 0);
}

int  FileSystem_copy(const char *src_file, const char *dest_file)
{
  try {
    ifstream source(src_file, ios::binary);
    ofstream dest(dest_file, ios::binary);

    if (!source || !dest)
      return false;

    dest << source.rdbuf();

    dest.close();
    source.close();
    return true;
  }
  catch (exception& e) {
    return false;
  }
}

int  FileSystem_move(const char *src_file, const char *dest_file)
{
  if (!FileSystem_copy(src_file, dest_file))
    return false;

  return (unlink(src_file) == 0);
}

#ifdef WIN32
char* mkdtemp(string& path)
{
  srand(time(NULL));
  char tmp[path.size()+1];
  std::copy(path.begin(), path.end(), tmp);

  // int id = GetCurrentProcessId();
  char* root = strchr(tmp, 'X');
  if (!root)
    return NULL;

  for (int times=0; times < 50; times++)
  {
    //< Generate a six digit random number
    int id = (int)(99999 + ((double)rand() / RAND_MAX*900003));
    char* p = root;
    for (; *p; ++p)
    {
      *p = id%10 + '0';
      id /= 10;
    }

    //< If no entity exists we found a unique dir name
    if (GetFileAttributes(tmp) == INVALID_FILE_ATTRIBUTES
     && CreateDirectory(tmp, 0) != 0)
    {
      path = tmp;
      return (char*)path.c_str();
    }
  }

  return NULL;
}
#else
char* mkdtemp(string& dir)
{
  return mkdtemp((char*)dir.c_str());
}
#endif

string FileSystem_tmpdir(string dirname)
{
  string tmpdir;
  static string default_name = "tmp-XXXXXX";

  #ifdef WIN32
    char tmp[128];
    GetTempPath(sizeof(tmp), (char*)&tmp);
    tmpdir = tmp + (dirname.empty() ? default_name : dirname);
  #else
    tmpdir = "/tmp/" + (dirname.empty() ? default_name : dirname);
  #endif

  size_t Xs = tmpdir.find("XXXXXX");
  if (Xs != string::npos)
  {
    if (!mkdtemp(tmpdir))
    {
    #ifdef __EXCEPTIONS
      throw logic_error("Could not create tmp directory["+tmpdir+"]: "+ GetErrorString(false));
    #else
      return string("");
    #endif
    }

    // Only do this once, next times it will not find any X's above
    if (dirname.empty())
      default_name = fname(tmpdir);
  }

  return tmpdir+SEPARATOR;
}


// From "Helpers.h"
//    FileGetName
const char* FileSystem_get_name(const char* _path)
{
  const char* path = strrchr(_path, SEPARATOR);
  if (path)
    return path+1;
  return _path;
}

// From "Helpers.h"
//    FileGetPath
const char* FileSystem_get_dir(char* _path)
{
  char* path = strrchr(_path, SEPARATOR);
  if (path)
    *path = 0;
  return _path;
}

/**
 * Frees a NULL terminated array of strings (such as returned by
 * FileSystem_dirlist and FileSystem_find). Both the array and the strings
 * pointed to by the elements in the array are freed.
 *
 * @param list Array of strings. Last element in the list must be NULL. If
 *             list is NULL, nothing is done.
 */
void FileSystem_free_string_array(char **list)
{
  if (list != NULL)
  {
    int i = 0;

    while (list[i] != NULL)
    {
      free(list[i]);
      i++;
    }
    free(list);
  }
}

/**
 * Returns list of directories found in a filepath.
 *
 * @param _path Path to search for directories.
 *
 * @return      Returns NULL terminated array of strings (char *) containing
 *              found directories. This list and the strings in the list must
 *              be freed by caller - FileSystem_free_string_array() can be used
 *              for this. If no directories are found or if memory allocation
 *              fails, NULL is returned.
 */
char** FileSystem_dirlist(const char* _path, uint8_t fullpath)
{
  const size_t chunk_size = 256;
  size_t num_dirs = 0;
  size_t alloc_size = 0;
  char **list = NULL;

  size_t path_len = strlen(_path);
  char *temp = NULL;

  DIR* dir = opendir(_path);
  if(dir)
  {
    struct dirent *ent;
    while((ent = readdir(dir)))
    {
      //< Ignore hidden files
      if (ent->d_name[0]=='.')
      {
        continue;
      }


      if (fullpath)
      {
        temp = (char*)malloc(path_len+strlen(ent->d_name)+2); // +2 is SEPARATOR and zero-termination
        if (temp == NULL)
          goto fail;

        strcpy(temp, _path);
        temp[path_len] = SEPARATOR;
        temp[path_len+1] = 0;
        strcat(temp,ent->d_name);
      }
      // Reallocate mem if list is full
      if (num_dirs + 1 >= alloc_size) // We add 1 to make room for NULL-termination of list
      {
        char **new_list;

        alloc_size += chunk_size;
        new_list = (char **) realloc(list, alloc_size * sizeof(char *));
        if (new_list == NULL)
          goto fail;

        list = new_list;
      }

      // Add dir to list
      if (!list)
        goto fail;

      if (fullpath)
      {
        list[num_dirs] = strdup(temp);
      }
      else
      {
        list[num_dirs] = strdup(ent->d_name);
      }

      if (!list[num_dirs])
      {
        FileSystem_free_string_array(list);
        list = NULL;
        goto fail;
      }

      // NULL-terminate list)
      if (temp)
      {
        free(temp);
        temp = NULL;
      }
      list[num_dirs + 1] = NULL;
      num_dirs++;
    }

    closedir(dir);
  }

  return list;

fail:
   free(temp);
   if (list)
     free(list);
   closedir(dir);
   return NULL;
}

/**
 * Locate files like the Unix find command
 *
 * Remember to free the list after use!
 */
// TODO: Rewrite - the function has been removed due to memory leaks.
#if 0
unsigned char isDir  = 0x4;
unsigned char isFile = 0x8;

char** FileSystem_find(const char* _path, const char* _str, uint8_t recursive)
{
  struct stat info;
  struct dirent *ent;
  string path = _path;
  string str = _str;

  if (path[path.size()-1] != '/')
    path += '/';

  vector<char*> *matches = new vector<char*>();
  DIR* dir = opendir( path.c_str() );
  if(!dir)
    return NULL; // TODO: memory leak - matches not freed

  while((ent = readdir(dir)))
  {
    //< Ignore hidden files
    if (ent->d_name[0]=='.') continue;

    string name = ent->d_name;
    string fpath = path+name;
    lstat(fpath.c_str(),&info);

    //printf("Searching %s (%d) %s...\n", fpath.c_str(),
    //        ent->d_type, recursive ? "(recursive)" : "");
    if (recursive && S_ISDIR(info.st_mode))
    {
      //printf("  -> DIRECTORY (%d)!\n", info.st_mode);
      char** found = FileSystem_find(fpath.c_str(), _str, true);
      if (found != NULL)
      {
        for (size_t i=0; found[i]; i++)
          matches->push_back(found[i]);
      }
    }
    else if (name.find( str ) != string::npos)
    {
      //printf("  -> FOUND\n");
      matches->push_back( strdup(fpath.c_str()) );
    }
  }
  matches->push_back( NULL );
  closedir(dir);

  return &(matches->at(0));  // TODO: memory leak - matches not freed
}
#endif


/**
 * Load the contents of a file into a buffer reference
 *
 *  @param path    Path to the file
 *  @param buffer  Reference to the buffer
 *
 *  @return        A pointer to the beginning of the buffer
 */
char* FILE_getContent(std::string path, std::vector<char> &buffer, uint64_t length, uint64_t offset)
{
  std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
  if (file.is_open())
  {
    if (!length)
      length = file.tellg();
    buffer.resize(length);
    file.seekg(offset);
    file.read(&buffer[0], buffer.size());
    file.close();
  }
  return &buffer[0];
}

/**
 * Load the contents of a file into a buffer reference
 *
 *  @param path    Path to the file
 *  @param buffer  Reference to the buffer
 *
 *  @return        A pointer to the beginning of the buffer
 */
uint8_t* FILE_getContent(std::string path, std::vector<uint8_t> &buffer, uint64_t length, uint64_t offset)
{
  std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
  if (file.is_open())
  {
    if (!length)
      length = file.tellg();
    buffer.resize(length);
    file.seekg(offset);
    file.read((char*)&buffer[0], buffer.size());
    file.close();
  }
  return &buffer[0];
}


/**
 * Load the contents of a file into a string
 *
 *  @param path    Path to the file
 *
 *  @return        A string with the loaded content
 */
string FILE_getContent(string path)
{
  stringstream memblock;

  if (!fexists(path.c_str()))
    return memblock.str();

  ifstream file( path.c_str(), ios_base::in | ios_base::binary );
  if (file.is_open())
  {
    memblock << (streambuf*)file.rdbuf();
    file.close();
  }
  return memblock.str();
}

/**
 * Allocates and load the contents of a file. Remember to delete the buffer after use
 *
 *  @param path    Path to the file
 *  @param size    Size reference. This will be loaded with the file size.
 *
 *  @return        A pointer to the beginning of the newly allocated buffer
 */
char* FILE_getContent(const char* path, size_t *size)
{
  char* content = NULL;

  if (fexists(path))
  {
    *size = fsize(path);
    FILE* fd = fopen(path, "rb");
    if (fd)
    {
      content = new char[*size];
      fread(content, 1, *size, fd);
      fclose(fd);
    }
  }

  return content;
}

/**
 * Fill a file with a number of similar chars
 *
 *  @param path   Path to the file
 *  @param c      The character to fill into the file
 *  @param size   The amount of characteres to fill in the file
 */
void FILE_setContent(string path, char c, size_t size)
{
  ofstream file(path.c_str(), fstream::binary);
  if (!file)
  {
    #ifdef __EXCEPTIONS
      throw runtime_error("Could not open file "+ path);
    #else
      cerr << "Could not open file " << path << endl;
      return;
    #endif
  }

  char* buf = new char[size];
  memset(buf, c, size);
  file.write(buf, size);
  delete[] buf;

  file.close();
}

/**
 * Set the contents of a file from a buffer
 *
 *  @param path     Path to the file
 *  @param content  Pointer to an allocated buffer
 *  @param size     Size of the buffer. Also the size to read
 *
 *  @return         True if the contents was written to the file.
 */
uint8_t FILE_setContent(const char* path, const char* content, size_t size)
{
  //if (fexists(path))
  FILE* fd = fopen(path, "wb");
  if (fd)
  {
    size = fwrite(content, 1, size, fd);
    fclose(fd);
    return true;
  }

  return false;
}

/**
 * Set the contents of a file from a buffer
 *
 *  @param path     Path to the file
 *  @param buffer   Reference to a buffer vector
 */
void FILE_setContent(string path, std::vector<char> &buffer)
{
  FILE_setContent(path.c_str(), &buffer[0], buffer.size());
}

/**
 * Set the contents of a file from a buffer
 *
 *  @param path     Path to the file
 *  @param buffer   Reference to a buffer vector
 */
void FILE_setContent(string path, std::vector<uint8_t> &buffer)
{
  FILE_setContent(path.c_str(), (char*)&buffer[0], buffer.size());
}



/**
 *  Save a stream to an output file
 *
 * @param stream    An open input stream to read from
 * @param output    A path to an output file
 * @param size      Bytes to write from stream
 **/
void save_stream_to_file(istream& stream, string output, size_t size)
{
  char buf[1024*1024]; // 1MB
  const size_t buf_size = sizeof(buf);

  ofstream out(output.c_str(), fstream::binary);
  if (!out.is_open())
    throw_exception("Could not open "+ output);

  while (size > buf_size)
  {
    stream.read(buf, buf_size);
    out.write(buf, buf_size);
    size -= buf_size;
  }
  stream.read(buf, size);
  out.write(buf, size);

  out.close();
}

#endif

/*-------------------------------------------------------------------------------*/
uint8_t FILE_exists(const char *file_name)
{
  uint8_t result = false;
#ifdef CUSTOM_FS
  if(IFWD_GLOBAL_ExtFileHandle)
  {
    if(IFWD_GLOBAL_ExtFileHandle->fopen && IFWD_GLOBAL_ExtFileHandle->fclose) /* are the needed functions present? */
    {
      void *in;
      if((in = IFWD_GLOBAL_ExtFileHandle->fopen(file_name, "rb")) != NULL)
      {
        result = true;
        (void)IFWD_GLOBAL_ExtFileHandle->fclose(in);
      }
    }
  }
#else
  FILE *in;
  if(file_name && ((in = fopen(file_name, "rb")) != NULL))
  {
    result = true;
    fclose(in);
  }
#endif

  return result;
}
/*-------------------------------------------------------------------------------*/

uint32_t FILE_size(FILE *handle)
{
  uint32_t curpos;
  uint32_t length=0;

  if(handle)
  {
#ifdef CUSTOM_FS
    if(IFWD_GLOBAL_ExtFileHandle)
    {
      if(IFWD_GLOBAL_ExtFileHandle->ftell && IFWD_GLOBAL_ExtFileHandle->fseek)
      {
        curpos = IFWD_GLOBAL_ExtFileHandle->ftell(handle);
        (void)IFWD_GLOBAL_ExtFileHandle->fseek(handle, 0L, SEEK_END);
        length = IFWD_GLOBAL_ExtFileHandle->ftell(handle);
        (void)IFWD_GLOBAL_ExtFileHandle->fseek(handle, curpos, SEEK_SET);
      }
      else
        length = 0; /* if we don't have the needed functions - we can't find the file size */
    }
#else
    curpos = ftell(handle);
    fseek(handle, 0L, SEEK_END);
    length = ftell(handle);
    fseek(handle, curpos, SEEK_SET);
#endif
  }
  else
    length = 0;
  return length;
}

/*-------------------------------------------------------------------------------*/

FILE *FILE_open(const char *file_name, FILE_open_mode_enum open_mode)
{
#ifdef FILE_IO_DEBUGGING
  printf("  -> FILE_open(%s)\n", fname(file_name));
#endif

  FILE *in = NULL;
  if(file_name && *file_name)
  {

#ifdef CUSTOM_FS
    if(IFWD_GLOBAL_ExtFileHandle)
    {
      if(IFWD_GLOBAL_ExtFileHandle->fopen)
      {
        switch(open_mode)
        {
        case FILE_open_mode_read:
          in = (FILE *)IFWD_GLOBAL_ExtFileHandle->fopen(file_name, "rb");
          break;
        case FILE_open_mode_write_append:
          in = (FILE *)IFWD_GLOBAL_ExtFileHandle->fopen(file_name, "a+b");
          break;
        case FILE_open_mode_write_replace:
          in = (FILE *)IFWD_GLOBAL_ExtFileHandle->fopen(file_name, "w+b");
          break;
        case FILE_open_mode_write_create:
          if(FILE_exists(file_name))
          {
            if(IFWD_GLOBAL_ExtFileHandle->remove)
              (void)IFWD_GLOBAL_ExtFileHandle->remove(file_name);
          }
          in = (FILE *)IFWD_GLOBAL_ExtFileHandle->fopen(file_name, "wb");
          break;
        }
      }
    }
#else

    if(file_name)
    {
      switch(open_mode)
      {
      case FILE_open_mode_read:
        in = fopen(file_name, "rb");
        break;
      case FILE_open_mode_write_append:
        in = fopen(file_name, "a+b");
        break;
      case FILE_open_mode_write_replace:
        in = fopen(file_name, "w+b");
        break;
      case FILE_open_mode_write_create:
        if(FILE_exists(file_name))
          (void)remove(file_name);
        in = fopen(file_name, "wb");
        break;
      }
    }
#endif
  }
#ifdef FILE_IO_DEBUGGING
  file_table[(int)in] = fname(file_name);
#endif
  return in;
}
/*-------------------------------------------------------------------------------*/
FILE_status_enum FILE_close(FILE *handle)
{
#ifdef FILE_IO_DEBUGGING
  if(!handle)
    printf(" Trying to CLOSE an already closed handle!!! %s:%d \n", __FILE__, __LINE__);
  else
    printf("  -> FILE_close(%s)\n", file_table[(int)handle].c_str());
#endif

  FILE_status_enum  result = FILE_status_EOF; /* assume error for now */
#ifdef CUSTOM_FS
  if(IFWD_GLOBAL_ExtFileHandle)
  {
    if(IFWD_GLOBAL_ExtFileHandle->fclose)
    {
      if(handle && IFWD_GLOBAL_ExtFileHandle->fclose(handle) == 0) /* no error? */
        result = FILE_status_ok;
    }
  }
#else
  if(handle && fclose(handle) == 0) /* no error? */
    result = FILE_status_ok;
#endif
  return result;
}
/*-------------------------------------------------------------------------------*/
FILE_status_enum  FILE_flush(FILE *handle)
{
  FILE_status_enum  result = FILE_status_EOF;
#ifdef CUSTOM_FS
  /* do nothing */
  result = FILE_status_ok;
#else
  if(handle && fflush(handle) == 0) /* Success? */
    result = FILE_status_ok;
#endif
  return result;
}
/*-------------------------------------------------------------------------------*/

FILE_status_enum  FILE_read(FILE *handle, uint8_t *mem_pointer, uint32_t length)
{
  FILE_status_enum  result = FILE_status_EOF;
  uint32_t n = 0;

#ifdef FILE_IO_DEBUGGING
  if(!handle)
    printf("  error> Trying to read from a closed handle!\n");
  else
    printf("  -> FILE_read(%u bytes from %s)\n", length, file_table[(int)handle].c_str());
#endif

  if(handle && mem_pointer)
  {
#ifdef CUSTOM_FS
    if(IFWD_GLOBAL_ExtFileHandle)
    {
      if(IFWD_GLOBAL_ExtFileHandle->fread)
        n = IFWD_GLOBAL_ExtFileHandle->fread(mem_pointer, 1, length, handle);
    }
#else
    n = fread(mem_pointer, 1, length, handle);
#endif
  }
  if(n == length)
    result = FILE_status_ok;
  return result;
}
/*-------------------------------------------------------------------------------*/
uint8_t FILE_ReadLine(FILE *in, char *dest, uint32_t BufferSize)
{
  uint8_t result = false;

#ifdef FILE_IO_DEBUGGING
  if(!in)
    printf("  error> Trying to read from a closed handle!\n");
  else
    printf("  -> FILE_readline(%s)\n", file_table[(int)in].c_str());
#endif

  if(in && dest && BufferSize > 0)
  {
    *dest = 0;
    if(!FILE_EOF(in))
    {
      int len;
#ifdef CUSTOM_FS
      if(IFWD_GLOBAL_ExtFileHandle)
      {
        if(IFWD_GLOBAL_ExtFileHandle->fgets)
          IFWD_GLOBAL_ExtFileHandle->fgets(dest, BufferSize, in);
      }
#else
      if(!fgets(dest, BufferSize, in))
        result = FILE_status_ok;
#endif
      dest[BufferSize - 1] = 0; /* Make sure it's null-terminated */
      len = strlen(dest);
      if(dest[len - 1] == 10 || dest[len - 1] == 13)
        dest[len - 1] = 0; /* Remove last char (LineFeed) */
      result = true;

    }
  }
  return result;
}
/*-------------------------------------------------------------------------------*/

FILE_status_enum  FILE_write(FILE *handle, uint8_t *mem_pointer, uint32_t length)
{
  FILE_status_enum  result = FILE_status_write_access_error;
  uint32_t n = 0;

#ifdef FILE_IO_DEBUGGING
  if(!handle)
    printf("  error> Trying to write to a closed handle!\n");
  else
    printf("  -> FILE_write(%ul bytes to %s)\n", length, file_table[(int)handle].c_str());
#endif

  if(handle)
  {
#ifdef CUSTOM_FS
    if(IFWD_GLOBAL_ExtFileHandle)
    {
      if(IFWD_GLOBAL_ExtFileHandle->fwrite)
        n = IFWD_GLOBAL_ExtFileHandle->fwrite(mem_pointer, 1, length, handle);
    }
#else
    n = fwrite(mem_pointer, 1, length, handle);
#endif
  }
  if(n == length)
    result = FILE_status_ok;
  return result;
}
/*-------------------------------------------------------------------------------*/

FILE_status_enum  FILE_seek(FILE *handle, uint32_t offset)
{
  FILE_status_enum  result = FILE_status_EOF;

#ifdef FILE_IO_DEBUGGING
  if(!handle)
    printf("  error> Trying to seek a closed handle!\n");
  else
    printf("  -> FILE_seek(position %i in %s)\n", offset, file_table[(int)handle].c_str());
#endif

#ifdef CUSTOM_FS
  if(IFWD_GLOBAL_ExtFileHandle)
  {
    if(IFWD_GLOBAL_ExtFileHandle->fseek)
    {
      if(IFWD_GLOBAL_ExtFileHandle->fseek(handle, offset, SEEK_SET) == 0) /* Success? */
        result = FILE_status_ok;
    }
  }
#else
  int res = 0;
//    if(handle && fseek(handle,offset,SEEK_SET) == 0) /* Success? */
  if(handle)
  {
    res &= fseek(handle, offset, SEEK_SET);

    if(handle && (res == 0))
    {
      result = FILE_status_ok;
    }
  }
#endif
  return result;
}
/*-------------------------------------------------------------------------------*/

int32_t FILE_pos(FILE *handle)
{
#ifdef CUSTOM_FS
  if(IFWD_GLOBAL_ExtFileHandle)
  {
    if(IFWD_GLOBAL_ExtFileHandle->ftell)
      return IFWD_GLOBAL_ExtFileHandle->ftell(handle);
  }
#else
  if(handle)
    return ftell(handle);
#endif
  return 0;
}
/*-------------------------------------------------------------------------------*/

char *FILE_ExtractFileName(char *filename)
{
  if(filename && *filename)
  {
    char *p = filename;
    int len = strlen(filename);
    p = p + len - 1; /* go to last character in string (just before NULL termination) */
    while(len > 0 && *p != (char)'\\' && *p != (char)'/')
    {
      p--;
      len--;
    }
    if(len > 0) /* did we exit loop because we found a delimiter? */
      return p + 1; /* return the next character AFTER the found path delimiter */
  }
  return filename;
}
/*---------------------------------------------------------------------------*/
/* "readme.txt" => returns a pointer to ".txt" */
/* If no extension is present it will return a pointer to the end of the function */
char *FILE_ExtractFileExt(char *filename)
{
  if(filename && *filename)
  {
    char *p, *p_end;
    int len = strlen((char*)filename);
    p_end = filename + len; /* pointer to null termination */
    p = p_end - 1; /* go to last character in string (just before NULL termination) */
    while(len > 0 && *p != (char)'.')
    {
      p--;
      len--;
    }
    if(len > 0)   /* did we exit loop because we found a delimiter? */
      return p;   /* return the full extension WITH '.', e.g. ".txt"  */
    return p_end; /* there was no extension so return a pointer to the end of the string */
  }
  return filename;
}
/*---------------------------------------------------------------------------*/
/* Helper function to write a string to a open file: */
void FILE_WriteLine(FILE *out, char *string)
{
  if(out && string && *string)
    (void)FILE_write(out, (uint8_t*)string, strlen(string));
}
/*---------------------------------------------------------------------------*/
uint8_t FILE_EOF(FILE *in)
{
#ifdef CUSTOM_FS
  if(IFWD_GLOBAL_ExtFileHandle)
  {
    if(IFWD_GLOBAL_ExtFileHandle->Feof)
      return (IFWD_GLOBAL_ExtFileHandle->Feof(in) != 0);
  }
#else
  if(in)
    return (feof(in) != 0);
#endif
  return true;
}

/*---------------------------------------------------------------------------*/
char *FILE_get_last_error(void)
{
  /*
    static char strError[256];
    strError[0] = 0;
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)strError, 256, 0);
    return strError;
  */
#ifdef __BORLANDC__
#ifdef CUSTOM_FS
  return "";
#endif
  return (char *)strerror(errno);
#else
  return (char*)"";
#endif
}
/*-------------------------------------------------------------------------------*/

/**
 *    C++ Standard Library Template replacement code for FileIO using "fstream"
 *
 *    This is just an idea and is not yet ready for the world...
 *
 *    This implementation makes use of try-catch and is much smaller in size.
 **/
#if 0

#include <string>
#include <fstream>

using namespace std;

fstream file;
file.exceptions( ifstream::failbit | ifstream::badbit );

try
{
  file.open("test.txt", ios::in |ios::out | ios::binary );

  file.seekg( ifstream::end );
  size_t end = file.tellg()
  file.seekg( 0 );

  cout << "File size: " << size << endl;

  cout << "File Content (line by line)" << endl;
  while (getline(file, line))
  cout << line << endl;

  cout << "Seeking has both read and write pointer..." << endl;
  size_t offset = 50;
  file.seekg( offset );     //< Moving the Get pointer
  file.seekp( offset );     //< Moving the Put pointer
  cout << "The positions are now Get/Put => " << file.tellg() << "/" << file.tellp() << endl;

  cout << "Read/Write are stream operations" << endl;
  file << "Inserting a line into the file..." << endl;
  line << file;
        //< These also works:
        //<       file.put( char );
        //<       file.write(memblock, size);

  cout << "..but working with binaries we have to do this:" << endl;
  char* memblock = new char[ size ];
  file.read( memblock, size );
        //< These also works:
        //<       file.get(memblock, size);
        //<       while (!file.eof()) c = file.get();

  file.flush();
  file.close();
}
catch(ifstream::failure e) {
  cout << "Exception during file operation..." << endl;
}


/**
 *    These functions should be defined in another module, since they are Not files, but filesystem
 **/
#if defined (_WIN32)
  #include <io.h>                //< access()
  //#define SEPARATOR "\\"
#elif defined (__CC_ARM)

#else
  #include <unistd.h>
  //#define SEPARATOR "/"
#endif

#include <sys/types.h>           // stat() defines
#include <sys/stat.h>            // stat()

uint8_t Exists(const char* file)
{
  if ( access(filename, 0) == 0)
  {
  struct stat info;
  stat(file, &info);
  if (info.st_mode & S_IFDIR)
    return false;
  else
    return true;
  }
  return false;
}

const char* ExtractEnding(const char* file, char* after)
{
  size_t pos = file.rfind( after );
  if (pos != string::npos)
    return file.substr(pos+1);
  return file;
}

const char* ExtractFileName(const char* file)  {
  return ExtractEnding(file, SEPARATOR);
}

const char* ExtractFileExt(const char* file)  {
  return ExtractEnding(file, ".");
}


/*-------------------------------------------------------------------------------*/

#endif


