#include <stdio.h>
#include "../network.h"

using namespace tools::network;

int main(int argc, char *argv[])
{
  char* ip = NULL;
  char* mac = NULL;
  
  try{
    ip = GetIpAddress("eth0");  // or lo
    if(ip)
      printf("ip %s\n", ip);
    else
      printf("ip not found\n");
  }
  catch(Exception e)
  {
    ;
  }

  mac = GetMacAddress("eth0");
  if(mac)
    printf("mac %s\n", mac);
  else
    printf("mac not found\n");
  return 0;
}
