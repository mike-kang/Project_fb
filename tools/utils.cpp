#ifndef _UTILS_HEADER
#define _UTILS_HEADER
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <stdio.h>

using namespace std;

namespace tools {
namespace utils {
char* itoa(int val, int base){
	
	static char buf[32];
  buf[31] = '\0';

	int i = 30;

  if(!val){
    buf[i] = '0';
    return &buf[i];
  }

  for(; val && i ; --i, val /= base)
	  buf[i] = "0123456789abcdef"[val % base];
  
	return &buf[i+1];
	
}  


void hexdump(const char* title, unsigned char* buf, int length)
{
  printf("[%s]\n", title);
  if(!length){
    printf("size = 0\n");
    return;
  }
  for(int i=0; i<length; i++){
    printf("0x%02x ", buf[i]);
  }
  putchar('\n');
  
}

const int EXCEPTION_NOT_FOUND = 0;
char* getElementData(char* xml_buf, const char* tag)
{
  char* p;
  char* ret = NULL;
  int tag_len = strlen(tag);
  char* key = new char[tag_len + 2]; // <XXXX + NULL
  key[0] = '<';
  strcpy(key+1, tag);
  if(p = strstr(xml_buf, key)){
    ret = p+tag_len+2;
    p = strstr(ret, "<");
    *p = '\0';
  }
  else{
    delete key;
    throw EXCEPTION_NOT_FOUND;
  }
  delete key;
  return ret;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

}
}
#endif //_UTILS_HEADER
