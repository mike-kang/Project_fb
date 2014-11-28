#include "asyncserial.h"
#include <stdio.h>
#include <iostream>
#include <sys/poll.h> 

using namespace std;
using namespace tools;

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

  cout << "read: "<< timeout << endl;
  fds.fd = m_fd;
  fds.events = POLLIN;
  ret = poll(&fds, 1, timeout);
  if(ret == -1){
    printf("EXCEPTION_POLL\n");
    throw EXCEPTION_POLL;
  }
  else if(ret == 0){
    printf("EXCEPTION_TIMEOUT\n");
    throw EXCEPTION_TIMEOUT;
  }
  
  return Serial::read(buf, len);
}

bool AsyncSerial::open()
{
  return Serial::open();
}

int AsyncSerial::close()
{
  return Serial::close();
}


