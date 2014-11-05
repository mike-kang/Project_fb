#ifndef _GPIO_HEADER
#define _GPIO_HEADER

#include <stdio.h>

class Gpio {
public:
  Gpio(int num, bool out=true/*direct*/, bool init=false);
  virtual ~Gpio();
  void write(bool v);
  bool read();
  
private:
  int m_number;
  bool m_out;
  //char* m_path;
  int m_fd;
};




#endif

