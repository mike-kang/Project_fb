#include <iostream>
#include <stdio.h>
#ifdef _WIN32
#include "windows.h"
#endif
#include "property.h"
#include "filesystem.h"

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
static char path[256];

void Property::init()
{
#ifdef _WIN32
  GetCurrentDirectory(256, path);
  strcat(path, "\\config");
  if(!filesystem::file_exist(path))
    filesystem::dir_create(path);
  strcat(path, "\\irience.ini");
#endif  
}


int Property::getPropertyInt(const char* category, const char* name, int def /*default*/)
{
#ifdef _WIN32
  return GetPrivateProfileInt(category, name, def, path);
#else
  return 0;
#endif

}

float Property::getPropertyFloat(const char* category, const char* name, float def /*default*/)
{
#ifdef _WIN32
  char buf[32];
  GetPrivateProfileString(category, name, "x", buf, 32, path);
  if(buf[0] == 'x')
    return def;
  return atof(buf);
#else
  return 0;
#endif
}

void Property::getPropertyStr(const char* category, const char* name, const char *def /*default*/, char *buf, int size)
{
#ifdef _WIN32
  GetPrivateProfileString(category, name, def, buf, size, path);
#endif  
  return;
}

bool Property::getPropertyBool(const char* category, const char* name, bool def /*default*/)
{
#ifdef _WIN32
  char buf[7];
  GetPrivateProfileString(category, name, (def)? "true":"false", buf, 7, path);
  return (buf[0] == 't' ||buf[0] == 'T');
#endif 
  return true;
}

bool Property::setProperty(const char* category, const char* name, int val)
{
#ifdef _WIN32
  char buf[10];
  itoa(val, buf, 10);
  
  WritePrivateProfileString(category, name, buf, path);
#endif  
  return true;
}


bool Property::setProperty(const char* category, const char* name, float val)
{
#ifdef _WIN32
  char buf[10];
  sprintf(buf,"%.3f", val);
  WritePrivateProfileString(category, name, buf, path);
#endif  
  return true;
}

bool Property::setProperty(const char* category, const char* name, char* val)
{
#ifdef _WIN32
  WritePrivateProfileString(category, name, val, path);
#endif  
  return true;
}

bool Property::setProperty(const char* category, const char* name, bool val)
{
#ifdef _WIN32
  WritePrivateProfileString(category, name, (val)? "true":"false", path);
#endif  
  return true;
}


}

