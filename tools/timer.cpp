#include "timer.h"
#include <iostream>
#include <errno.h>
using namespace tools;

Timer::Timer(void (*cbFunc)(void*), void* clientData):m_cbFunc(cbFunc), m_clientData(clientData), m_active(false) 
{
}

Timer::Timer(void (*cbFunc)(int, void*), void* clientData):m_cbArrayFunc(cbFunc), m_clientData(clientData), m_active(false) 
{
}

void Timer::start(int sec, bool repeat)
{
  start(sec, 0, repeat);
}

void Timer::start(int sec, int msec, bool repeat)
{
  m_active = true;
  m_sec = sec;
  m_msec = msec;
  m_repeat = repeat;
  pthread_create(&m_threadId, NULL, run, this);
  pthread_detach(m_threadId);
}

void Timer::start(int* arr_msec, bool repeat) //ex)arr_msec = {1500, 1500, 0};
{
  m_active = true;
  int* p = m_arr_msec;
  while(*arr_msec){
    *p++ = *arr_msec++;
  }
  *p = 0;
  m_arr_index = 0;
  m_repeat = repeat;
  
  pthread_create(&m_threadId, NULL, run_array, this);
  pthread_detach(m_threadId);
}

bool Timer::IsActive()
{
  return m_active;
}


void Timer::stop()
{
  //std::cout <<"Timer::cancel\n" << std::endl;
  mtx.lock();
  m_active = false;
  m_repeat = false;
  timerCancel.notify_one();
  mtx.unlock();
}

//static
void* Timer::run(void* arg)
{
  Timer* timer = (Timer*)arg;
  
  int ret;
  //std::cout <<"Timer::run\n" << std::endl;
  timer->mtx.lock();
  do {
    if(timer->m_msec)
      ret = timer->timerCancel.timedwait(timer->mtx, timer->m_sec, timer->m_msec);
    else
      ret = timer->timerCancel.timedwait(timer->mtx, timer->m_sec);

    if(!ret) //stop
      break;
    if(ret == ETIMEDOUT){ //expired
      //std::cout << "::timer count " << count << std::endl;
      timer->m_cbFunc(timer->m_clientData);
    }
  } while(timer->m_repeat);
  timer->m_active = false;
  timer->mtx.unlock();
 
  return NULL;
}

void* Timer::run_array(void* arg)
{
  Timer* timer = (Timer*)arg;
  int sec;
  int msec;
  int ret;
  int t;
  //std::cout <<"Timer::run\n" << std::endl;
  timer->mtx.lock();
  do {
    t = timer->m_arr_msec[timer->m_arr_index];
    sec = t/1000;
    msec = t%1000;
    if(msec)
      ret = timer->timerCancel.timedwait(timer->mtx, sec, msec);
    else
      ret = timer->timerCancel.timedwait(timer->mtx, sec);

    if(!ret) //stop
      break;  
    
    if(ret == ETIMEDOUT){ //expired
      //std::cout << "::timer count " << count << std::endl;
      timer->m_cbArrayFunc(timer->m_arr_index, timer->m_clientData);
    }
    if(!timer->m_arr_msec[++timer->m_arr_index] && timer->m_repeat)
      timer->m_arr_index = 0;
  } while(timer->m_arr_msec[timer->m_arr_index] || timer->m_repeat);

  timer->m_active = false;
  timer->mtx.unlock();
 
  return NULL;
}



