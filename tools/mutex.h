#ifndef _MUTEX_HEADER
#define _MUTEX_HEADER

#ifdef _WIN32
#include "windows.h"
#else
#include <pthread.h>
#endif

#include <iostream>
using namespace std;

class Mutex{
public:
  Mutex()
  {
#ifdef _WIN32
    m_p = new CRITICAL_SECTION;
    InitializeCriticalSection(m_p);
#else
    //cout << "mutex +++" << endl;
    m_p = new pthread_mutex_t; 
    pthread_mutex_init(m_p, NULL);
    //cout << "mutex ---" << endl;
#endif
};
  
  Mutex(const Mutex& m):m_p(m.m_p)
  {
    //cout << "Mutex copy " << (int)m_p << endl;
  };

  void lock()
  {
    //cout << "EnterCriticalSection ++" << (int)m_p << endl;
#ifdef _WIN32
    EnterCriticalSection(m_p);
#else
    pthread_mutex_lock(m_p);
#endif
    //cout << "EnterCriticalSection --" << (int)m_p << endl;
  }
  
  int timedlock(int count)
  {
    //cout << "EnterCriticalSection ++" << (int)m_p << endl;
#ifdef _WIN32
#else
    struct timespec ts_timeout;
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    ts_timeout.tv_sec += count;
    return pthread_mutex_timedlock(m_p, &ts_timeout);
#endif
    //cout << "EnterCriticalSection --" << (int)m_p << endl;
  }
  
  void unlock()
  {
    //cout << "LeaveCriticalSection ++" << (int)m_p << endl;
#ifdef _WIN32
    LeaveCriticalSection(m_p);
#else
    pthread_mutex_unlock(m_p);
#endif
    //cout << "LeaveCriticalSection --" << (int)m_p << endl;
  }
#ifdef _WIN32
  CRITICAL_SECTION* get()
  {
    return m_p;
  }
#else
  pthread_mutex_t* get()
  {
    return m_p;
  }
#endif
  ~Mutex()
  {
#ifdef _WIN32
    DeleteCriticalSection(m_p);
#else
    pthread_mutex_destroy(m_p);
#endif
  };
private:
#ifdef _WIN32
  CRITICAL_SECTION *m_p;
#else
  pthread_mutex_t *m_p;
#endif
};

#endif //_MUTEX_HEADER
