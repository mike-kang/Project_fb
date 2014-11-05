#ifndef _Queue_HEADER
#define _Queue_HEADER

#include <iostream>
#include <queue>
#include "mutex.h"
#include "condition.h"


using namespace std;

namespace tools {
  
template<typename E>
class Queue
{
public:
  Queue():m_available(true){};
  void push(E* m);
  E* pop();
  void exit();  
private:
  std::queue<E*> buf;
  Condition buffer_not_empty;
  Mutex mtx;
  bool m_available;
};

template<typename E>
void Queue<E>::push(E* e) {
  //cout << "Queue<E>::pushx ++ " << endl;
  mtx.lock();
  if(m_available){
    buf.push(e);
    buffer_not_empty.notify_one();
  }
  else
  {
    delete e;
  }
  mtx.unlock();    
  //cout << "Queue<E>::pushx -- " << endl;
}

template<typename E>
E* Queue<E>::pop() {
  //cout << "Queue<E>::popx ++ " << endl;
  mtx.lock();
  while (buf.empty())
    buffer_not_empty.wait(mtx);
  E* m = buf.front();
  buf.pop();
  mtx.unlock();
  //cout << "Queue<E>::popx -- " << endl;
  return m;
}

template<typename E>
void Queue<E>::exit() {
  mtx.lock();
  m_available = false;
  buf.push(NULL);
  buffer_not_empty.notify_one();
  mtx.unlock();    
}
}

#endif //_Queue_HEADER
