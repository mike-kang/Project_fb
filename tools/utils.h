#ifndef _UTILS_HEADER
#define _UTILS_HEADER
#include <string>
#include <vector>

namespace tools {
namespace utils {
char* itoa(int val, int base);
void hexdump(const char* title, unsigned char* buf, int length);
const int EXCEPTION_NOT_FOUND = 0;
char* getElementData(char* xml_buf, const char* tag);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> 
&elems);

}
}
#endif //_UTILS_HEADER
