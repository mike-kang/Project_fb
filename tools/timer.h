#ifndef _TIMER_HEADER
#define _TIMER_HEADER

#include <pthread.h>
#include "mutex.h"
#include "condition.h"

namespace tools {
class Timer {
public:
  Timer(void (*cbFunc)(void*), void* clientData);
  Timer(void (*cbFunc)(int, void*), void* clientData);
  ~Timer(){};
  bool IsActive();
  void start(int sec, bool repeat=false);
  void start(int sec, int msec, bool repeat=false);
  void start(int* arr_msec, bool repeat=false); //ex)arr_msec = {1500, 1500, 0};
  //void reset(int count);
  void stop();

private:
  static void* run(void* arg);
  static void* run_array(void* arg);

  int m_sec; //sec
  int m_msec;
  void (*m_cbFunc)(void*);
  void (*m_cbArrayFunc)(int, void*);
  pthread_t m_threadId;
  void* m_clientData;
  bool m_repeat;
  bool m_active;
  Mutex mtx;
  Condition timerCancel;
  int m_arr_msec[100];
  int m_arr_index;
};


}
#endif //_TIMER_HEADER
