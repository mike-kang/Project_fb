#ifndef _CONDITION_HEADER
#define _CONDITION_HEADER

#ifdef _WIN32
#include "windows.h"
#else
#include <pthread.h>
#include <sys/time.h>
#endif

#include <iostream>
using namespace std;

class Condition{
public:
  Condition():m_bWait(false)
  {
#ifdef _WIN32  
    m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!m_event){
      cout << "CreateEvent fail" << endl;
      return;
    }
#else
    //cout << "Condition +++" << endl;
   
    pthread_cond_init(&m_p, NULL);
    //cout << "Condition ___" << endl;
#endif
    //cout << "CreateEvent success" << endl;
  }

  void wait(Mutex& m)
  {
    //cout << "Consition::wait() ++ " << m.get() << endl;
    m_bWait = true;
#ifdef _WIN32  
    m.unlock();
    WaitForSingleObject(m_event, INFINITE);
    m.lock();
#else
    pthread_cond_wait(&m_p, (pthread_mutex_t*)(m.get()));
#endif
    //cout << "Consition::wait() -- " << m.get() << endl;
  }
  
  int timedwait(Mutex& m, int count)
  {
    //cout << "Consition::timedwait() ++ " << m.get() << endl;
    m_bWait = true;
#ifdef _WIN32  
#else
    struct timespec ts_timeout;
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    ts_timeout.tv_sec += count;
    int ret = pthread_cond_timedwait(&m_p, (pthread_mutex_t*)(m.get()), &ts_timeout);
#endif
    //cout << "Consition::timedwait() -- " << m.get() <<" " << ret<< endl;
    return ret;
  }
  
  int timedwait(Mutex& m, int sec, int msec)
  {
    //cout << "Consition::timedwait() ++ " << m.get() << endl;
    m_bWait = true;
#ifdef _WIN32  
#else
    struct timespec ts_timeout;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long usec = tv.tv_usec + msec * 1000;
    if(usec >= 1000000){
      ts_timeout.tv_sec = tv.tv_sec + sec + 1;
      ts_timeout.tv_nsec = (usec - 1000000) * 1000;
    }
    else{
      ts_timeout.tv_sec = tv.tv_sec + sec;
      ts_timeout.tv_nsec = usec * 1000;
    }
      
    int ret = pthread_cond_timedwait(&m_p, (pthread_mutex_t*)(m.get()), &ts_timeout);
#endif
    //cout << "Consition::timedwait() -- " << m.get() <<" " << ret<< endl;
    return ret;
  }

  
  void notify_one()
  {
    //cout << "Consition::notify_one() ++ " << endl;
    if(m_bWait){
      m_bWait = false;
#ifdef _WIN32  
      SetEvent(m_event);
#else
      pthread_cond_signal(&m_p);
#endif
    }
  }

  
  ~Condition()
  {
#ifdef _WIN32  
    CloseHandle(m_event);
#else
    pthread_cond_destroy(&m_p);
#endif
  }
private:
#ifdef _WIN32
  HANDLE m_event;
#else
  pthread_cond_t m_p;
#endif
  bool m_bWait;
  
};

#endif //_CONDITION_HEADER
