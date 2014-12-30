#ifndef _NETWORK_HEADER
#define _NETWORK_HEADER
namespace tools {
namespace network {
enum Exception {
  EXCEPTION_RESOLVENAME,
  EXCEPTION_IPADDR,
  EXCEPTION_MACADDR,  

};

bool isIPv4(const char* str);
char* ResolveName(char* name);  //getIP
char* GetIpAddress(const char* if_name);
  

char* GetMacAddress(const char* if_name);


}
}
#endif //_NETWORK_HEADER
