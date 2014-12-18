#include "asyncserial.h"
#include <stdio.h>
#include <iostream>
#include <sys/poll.h> 
#include "log.h"

using namespace std;
using namespace tools;

#define LOG_TAG "AsyncSerial"

AsyncSerial::AsyncSerial(const char* path, Serial::Baud baud):Serial(path, baud, 0, 0)
{
}

int AsyncSerial::write(const byte* buf, int length)
{
  return Serial::write(buf, length);
}

int AsyncSerial::read(byte* buf, int len, int timeout)
{
  struct pollfd fds;
  int ret;
  int count = 3;

  //cout << "read: "<< timeout << endl;
  //LOGV("read: time %d\n", timeout);
  fds.fd = m_fd;
  fds.events = POLLIN;
  
  ret = poll(&fds, 1, timeout);
  if(ret > 0)
    return Serial::read(buf, len);
  
  if(ret == -1){
    LOGE("EXCEPTION_POLL\n");
    throw EXCEPTION_POLL;
  }
  else if(ret == 0){
    LOGE("EXCEPTION_TIMEOUT\n");
    throw EXCEPTION_TIMEOUT;
  }
  
}

bool AsyncSerial::open()
{
  return Serial::open();
}

int AsyncSerial::close()
{
  return Serial::close();
}


