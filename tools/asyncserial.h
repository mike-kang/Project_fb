#ifndef _ASYNCSERIAL_HEADER
#define _ASYNCSERIAL_HEADER

#include "serial.h"

using namespace tools;

class AsyncSerial : public Serial {
public:
  enum Exception {
    EXCEPTION_POLL,
    EXCEPTION_TIMEOUT,
  };
  AsyncSerial(const char* path, Serial::Baud baud);
  virtual ~AsyncSerial()
  {
  }
  
  bool open();
  //void run();
  int close();
  int write(const byte* buf, int length);
  int read(byte* buf, int len, int timeout);

};




#endif //_ASYNCSERIAL_HEADER

