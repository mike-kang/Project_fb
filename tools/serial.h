#ifndef _SERIAL_HEADER
#define _SERIAL_HEADER

#include <string.h>
#include "thread.h"

namespace tools {

class Serial {
public:
  enum Baud {
    SB38400,
    SB115200,
  };
/*
  class SerialReadNoti {
  public:
    virtual void onRead() = 0;
  };
*/  
  Serial(const char* path, Baud baud):m_fd(-1){
    strcpy(m_dev_name, path);
    m_baud = baud;
  }

  bool open();
  //void run();
  int close();
  //void startRead(SerialReadNoti* rn);
  int write(const char* buf, int len);
  int read(char* buf, int len);
  
  
private:
  Baud m_baud;
  char m_dev_name[50];
  int m_fd;
  //SerialReadNoti* m_rn;
  //Thread<Serial>* m_thread;
  
};



}

#endif

