#include <stdio.h>
#include <stropts.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>

#include "network.h"
#include "log.h"

#define LOG_TAG "network"
#define ISNUM(x) ((x) >= '0' && (x) <= '9' )

namespace tools {
namespace network {

bool isIPv4(const char* str)
{
  int count = 0;
  const char* p = str;
  int state = 0;
  
  while(p){
    switch(state){
      case 0:
        if(ISNUM(*p)){
          state = 1;
        }
        else
          return false;
        break;
      case 1:
      case 2:
        if(ISNUM(*p)){
          state++;
        }
        else if(*p == '.'){
          state = 0;
          count ++;
        }
        else
          return false;
        break;
      case 3:
        if(*p == '.'){
          state = 0;
          count ++;
        }
        else
          return false;
        break;
    }
    p++;
  }
  if(count == 3)
    return true;
  return false;
}

char* ResolveName(char* name)  //getIP
{
  static char ip[21];
  
  struct hostent *host;            /* Structure containing host information */

  if ((host = gethostbyname(name)) == NULL){
    LOGE("gethostbyname() failed\n");
    throw EXCEPTION_RESOLVENAME;
  }
  printf("host=%p\n", host);
  /* Return the binary, network byte ordered address */
  inet_ntop(AF_INET, host->h_addr_list[0], ip, 20);

  return ip;
}

char* GetIpAddress(const char* if_name)
{
  static char ip[INET_ADDRSTRLEN];
  struct ifconf ifconf;
  struct ifreq ifr[5];
  int if_count;
  int i;

  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    LOGE("socket: %s\n", strerror(errno));
    throw EXCEPTION_IPADDR;
  }

  ifconf.ifc_buf = (char *) ifr;
  ifconf.ifc_len = sizeof ifr;

  if (ioctl(s, SIOCGIFCONF, &ifconf) == -1) {
    LOGE("ioctrl: %s\n", strerror(errno));
    close(s);
    throw EXCEPTION_IPADDR;
  }
  close(s);

  if_count = ifconf.ifc_len / sizeof(struct ifreq);


  for (i = 0; i < if_count; i++) {
    struct sockaddr_in *s_in = (struct sockaddr_in *) &ifr[i].ifr_addr;
    if(strcmp(ifr[i].ifr_name, if_name))
      continue;
    if (!inet_ntop(AF_INET, &s_in->sin_addr, ip, sizeof(ip))) {
      LOGE("inet_ntop: %s\n", strerror(errno));
      close(s);
      throw EXCEPTION_IPADDR;
    }
    return ip;
  }
  return NULL;
}

char* GetMacAddress(const char* if_name)
{
  static char mac[17 + 1]; //xx:xx:xx:xx:xx:xx
  char path[255];
  int fd;
  sprintf(path, "/sys/class/net/%s/address", if_name);
  if((fd = open(path, O_RDONLY)) < 0)
    return NULL;
  if( read(fd, mac, 17) != 17){
    close(fd);
    return NULL;
  }
  
  close(fd);
  mac[17] = '\0';
  return mac;
}

}
}
