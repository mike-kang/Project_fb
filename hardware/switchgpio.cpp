#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h> 

#include <sys/types.h>
#include <sys/stat.h>
#include "switchgpio.h"
#include "tools/log.h"
#include "tools/utils.h"
#include <iostream>

#define LOG_TAG "SwitchGpio"

using namespace std;
using namespace tools;

void SwitchGpio::cbOnTimer(void* arg)
{
  SwitchGpio* my = (SwitchGpio*)arg;
  my->write(false);
}
void SwitchGpio::cbOnArrayTimer(int i, void* arg)
{
  SwitchGpio* my = (SwitchGpio*)arg;
  if(i & 0x01)
    my->write(true);
  else
    my->write(false);
}

void SwitchGpio::on(int t) //ms
{
  m_timer = new Timer(cbOnTimer, this);
  m_timer->start(t/1000, t%1000);
  on();
}

void SwitchGpio::on(int* arr, bool repeat)
{
  m_timer = new Timer(cbOnArrayTimer, this);
  m_timer->start(arr, repeat);
  on();
}

void SwitchGpio::off()
{ 
  if(m_timer && m_timer->IsActive())
    m_timer->stop();
  write(false);
}


