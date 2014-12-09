#include "filesystem.h"
#include <iostream>
#include <stdio.h>
#ifdef _WIN32
#include "io.h"
#include "windows.h"
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#endif

#ifdef _DEBUG_MEMORY_LEAK
#ifdef _WIN32
#ifndef __MINGW32__
#include <crtdbg.h>
#define CRTDBG_MAP_ALLOC
#define new new(_CLIENT_BLOCK, __FILE__, __LINE__)
#endif
#endif
#endif

using namespace std;

namespace tools {
namespace filesystem {
#ifdef _WIN32

bool file_exist(const char* path)
{
  return (!_access(path, 6));
}
 
int file_size(const char* path)
{
  WIN32_FILE_ATTRIBUTE_DATA fad;
  if (!GetFileAttributesEx(path, GetFileExInfoStandard, &fad))
      return -1; // error condition, could call GetLastError to find out more
  LARGE_INTEGER size;
  size.HighPart = fad.nFileSizeHigh;
  size.LowPart = fad.nFileSizeLow;
  return size.QuadPart;
}
char* dir_current()
{
  static char curr[255];
  GetCurrentDirectory(255, curr);
  return curr;
}
void dir_create(const char* path)
{
  CreateDirectory(path, NULL);
}
#else
bool file_exist(const char* path)
{
  return (access(path, F_OK) == 0);
}

char* dir_current()
{
  static char curr[255];
  getcwd(curr, 255);
  return curr;
}

void dir_create(const char* path)
{
  mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
int file_size(const char* path)
{
  struct stat fileStats;
  if(stat(path, &fileStats) < 0)
  {
    cout << "fils is not exist" << endl;
    return -1;
  }
  return fileStats.st_size;
}

void getList(const char* directory, vector<string*>& arr)
{
  DIR* dir;
  struct dirent *entry;
  dir = opendir(directory);
  if(!dir){
    throw EXCEPTION_OPEN_DIR;
  }
  errno = 0;

  while((entry = readdir(dir)) != NULL){
    if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")){
      string* s = new string(entry->d_name);
      arr.push_back(s);
    }
  }
  closedir(dir);
  
  //for(vector<string*>::size_type i; i < arr.size(); i++){
  //  cout << "list:" << *arr[i] << endl;
  //}
  if(!entry && errno){
    for(vector<string*>::size_type i; i < arr.size(); i++){
      delete arr[i];
    }
    throw EXCEPTION_EBADF;
  }
}

int getListCount(const char* directory)
{
  DIR* dir;
  int count = 0;
  struct dirent *entry;
  dir = opendir(directory);
  if(!dir){
    throw EXCEPTION_OPEN_DIR;
  }
  errno = 0;

  while((entry = readdir(dir)) != NULL){
    if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")){
      count++;
    }
  }
  closedir(dir);

  if(!entry && errno){
    throw EXCEPTION_EBADF;
  }

  return count;
}

bool file_delete(const char* path)
{
  if(remove(path) < 0)
    return false;
  return true;
}

bool dir_chdir(const char* path)
{
  if(chdir(path) < 0)
    return false;
  return true;
}
#endif
}
}
