#ifndef _SWITCHGPIO_HEADER
#define _SWITCHGPIO_HEADER

#include "gpio.h"
#include "../tools/timer.h"

//only out direct
class SwitchGpio : private Gpio{
public:
  SwitchGpio(int num, bool init=false):Gpio(num, true, init), m_timer(NULL){}
  virtual ~SwitchGpio(){};
  void on(){ write(true); }
  void on(int msec);
  void on(int*, bool repeat);
  void off();

private:
  static void cbOnTimer(void* arg);
  static void cbOnArrayTimer(int i, void* arg);
  tools::Timer* m_timer;
};

#endif

