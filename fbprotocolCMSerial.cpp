#include "fbprotocolCMSerial.h"
#include "tools/log.h"
#include <stdio.h>
#include <iostream>
#include <sys/poll.h> 

using namespace std;
using namespace tools;

int FBProtocolCMSerial::onWrite(const char* buf, int length)
{
  return write(buf, length);
}

int FBProtocolCMSerial::onRead(char* buf, int len, int timeout)
{
  struct pollfd fds;
  int ret;

  fds.fd = m_fd;
  fds.events = POLLIN;
  ret = poll(&fds, 1, timeout);
  if(ret == -1){
    throw EXCEPTION_POLL;
  }
  else if(ret == 0){
    throw EXCEPTION_TIMEOUT;
  }
  
  return read(buf, len);
}

/*
int FBProtocolCMSerial::onPoll(int timeout)
{
}
*/

