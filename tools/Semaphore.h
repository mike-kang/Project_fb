#ifndef _SEMAPHORE_HEADER
#define _SEMAPHORE_HEADER

#ifdef _WIN32
#include "windows.h"
#else
#include <semaphore.h>
#endif

//#include <iostream>
using namespace std;

class Semaphore{
public:
  Semaphore(int count):m_count(count)
  {
#ifdef _WIN32
#else
    //cout << "mutex +++" << endl;
    sem_init(&m_sem, 0, count);
    //cout << "mutex ---" << endl;
#endif
  }

  void reset()
  {
    sem_init(&m_sem, 0, m_count);
  }
  
  void wait()
  {
    //cout << "sem_wait ++" << (int)&m_sem << endl;
#ifdef _WIN32
#else
    sem_wait(&m_sem);
#endif
    //cout << "sem_wait --" << (int)&m_sem << endl;
  }

  int timedwait(int count) //sec
  {
    //cout << "sem_wait ++" << (int)&m_sem << endl;
#ifdef _WIN32
#else
    struct timespec ts_timeout;
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    ts_timeout.tv_sec += count;
    return sem_timedwait(&m_sem, &ts_timeout);
    sem_wait(&m_sem);
#endif
    //cout << "sem_wait --" << (int)&m_sem << endl;
  }
  
  void post()
  {
    //cout << "sem_post ++" << (int)&m_sem << endl;
#ifdef _WIN32
#else
    sem_post(&m_sem);
#endif
    //cout << "sem_post --" << (int)&m_sem << endl;
  }
#ifdef _WIN32
#else
  int get()
  {
    int val;
    sem_getvalue(&m_sem, &val);
    return val;
  }
#endif
  ~Semaphore()
  {
#ifdef _WIN32
#else
    sem_destroy(&m_sem);
#endif
  };
private:
#ifdef _WIN32
#else
  sem_t m_sem;
  int m_count;
#endif
};

#endif //_SEMAPHORE_HEADER
