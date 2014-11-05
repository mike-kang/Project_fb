#ifndef _THREAD_HEADER
#define _THREAD_HEADER

#ifdef _WIN32
#include "windows.h"
#else
#include <pthread.h>
#endif
#include <cstring>

#include <iostream>
#include "mutex.h"
#include "condition.h"
#ifdef _DEBUG
#include "log.h"
#endif

#define LOG_TAG "Thread"

using namespace std;

template <typename T>
class Thread{
public:
  typedef void (T::*CBFunc)(void);
  Thread(CBFunc, T*, const char* name = NULL);
  ~Thread();

  unsigned long getId() const 
  { 
    return m_threadId;
  }

  void join();
  void detach()
  {
#ifdef _WIN32
#else
    pthread_detach(m_threadId);
#endif
  }
private:
#ifdef _WIN32
  static DWORD WINAPI run(LPVOID);
#else
  static void* run(void*);
#endif

  CBFunc m_cbfunc;
  T* m_client;
#ifdef _WIN32
  DWORD m_threadId;
  HANDLE m_handle;
#else
  pthread_t m_threadId;
#endif
  char m_name[32];  //for debug
};

template<typename T>
Thread<T>::Thread(CBFunc f, T* t, const char* name):m_cbfunc(f), m_client(t)
{
#ifdef _WIN32
  m_handle = CreateThread(NULL, 0, run, this, 0, &m_threadId);
#else
  pthread_create(&m_threadId, NULL, run, this); 
#endif
  if(name)
    strncpy(m_name, name, 32);
  else
    strcpy(m_name, "No-name");
}

#ifdef _WIN32
template<typename T>
DWORD WINAPI Thread<T>::run(LPVOID arg)
{
  Thread *t = (Thread*)arg;
  (t->m_client->*(t->m_cbfunc))();

  return 0;
}
#else
template<typename T>
void* Thread<T>::run(void* arg)
{
  Thread *t = (Thread*)arg;
  (t->m_client->*(t->m_cbfunc))();

  return NULL;
}
#endif

template<typename T>
Thread<T>::~Thread()
{
  //LOGV("~Thread +++ %s(0x%x)\n", m_name, m_threadId);
  //only wait for ending of thread.
  join();
  //LOGV("~Thread --- %s(0x%x)\n", m_name, m_threadId);
}

template<typename T>
void Thread<T>::join()
{
  //LOGV("Thread join+++ %s(0x%x)\n", m_name, m_threadId);
#ifdef _WIN32
  WaitForSingleObject(m_handle, INFINITE);
#else
  pthread_join(m_threadId, NULL);
#endif
  //LOGV("Thread join %s(0x%x)\n", m_name, m_threadId);
}


#undef LOG_TAG

#endif //_THREAD_HEADER
