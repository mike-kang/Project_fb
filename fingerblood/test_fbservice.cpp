#include <iostream>
#include "fbservice.h"

using namespace std;
using namespace tools;

FBService* fbs;

class Noti: public FBService::FBServiceNoti {
public:  
  virtual void onScanData(const char* buf)
  {
    cout << "onScanData:" << buf << endl;
  }
  virtual void onStart(bool ret)
  {
    cout << "onStart:" << ret << endl;
    if(ret){
      list<string> listUserCode;
      cout << 1 << endl;
      fbs->getList(listUserCode);
      cout << 2 << endl;
      fbs->format();
      cout << 3 << endl;
    }
  }
};

int main()
{
  Noti noti;
  cout << "start main\n" << endl;
  fbs = new FBService("/dev/ttyUSB0", Serial::SB38400, &noti);
  
  //fbs->save("FID0000000000000012.bin");
  //fbs->deleteUsercode(12);
  //fbs->requestStartScan(300);
  while(1){
    cout << "Koong" << endl;
    sleep(1);
  }
  cout << "exit" << endl;
  return 0;
}
