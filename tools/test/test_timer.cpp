#include <stdio.h>
#include <stdlib.h>
#include "../timer.h"
#include <iostream>

using namespace std;
using namespace tools;

void cbTimerArr(int i, void* arg)
{
  cout << "cbTimerArr cb:" << i<< endl;
}
void cbTimer(void* arg)
{
  cout << "timer cb:"<< endl;
}

int main()
{
  int arr[] = {1000, 1000, 0};
  Timer* timer = new Timer(cbTimer, NULL);
  Timer* timer_arr = new Timer(cbTimerArr, NULL);
  timer->start(3);
  timer_arr->start(arr, true);

  sleep(15);
  return 0;
}
