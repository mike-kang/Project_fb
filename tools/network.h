#ifndef _NETWORK_HEADER
#define _NETWORK_HEADER
namespace tools {
namespace network {
enum Exception {
  EXCEPTION_IPADDR,
  EXCEPTION_MACADDR,  

};

char* GetIpAddress(const char* if_name);
  

char* GetMacAddress(const char* if_name);


}
}
#endif //_NETWORK_HEADER