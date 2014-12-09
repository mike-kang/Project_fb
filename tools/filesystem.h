#ifndef _FILESYSTEM_HEADER
#define _FILESYSTEM_HEADER

#include <iostream>
#include <vector>
namespace tools {
namespace filesystem {

bool file_exist(const char*);
int file_size(const char*);
bool file_delete(const char* path);
void dir_create(const char*);
char* dir_current();
bool dir_chdir(const char* path);

enum Exception {
  EXCEPTION_NO_DIRECTORY,
  EXCEPTION_OPEN_DIR,
  EXCEPTION_EBADF,
  
};
void getList(const char* directory, std::vector<std::string*>& arr);
int getListCount(const char* directory);

}
}
#endif //_FILESYSTEM_HEADER
