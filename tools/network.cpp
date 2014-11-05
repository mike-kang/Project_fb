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
#include "network.h"
#include "log.h"

#define LOG_TAG "network"

namespace tools {
namespace network {
  
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
  mac[17] = '\0';
  return mac;
}

}
}
