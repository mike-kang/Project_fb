#include <iostream>
#include "fbservice.h"

using namespace std;
using namespace tools;

int main()
{
  cout << "start main\n" << endl;
  FBService* fbs = new FBService("/dev/ttyUSB0", Serial::SB38400);
  fbs->start();
  
  while(1){
    cout << "Koong" << endl;
    sleep(100);
  }
  cout << "exit" << endl;
  return 0;
}
