#include <iostream>
#include <stdio.h>
#include "fbservice.h"
#include <fstream>
#include <vector>

using namespace std;
using namespace tools;

FBService* fbs;

class Noti: public IFBService::IFBServiceEventListener {
public:  
  virtual void onScanData(const char* buf)
  {
    cout << "onScanData:" << buf << endl;
  }
  /*
  virtual void onStart(bool ret)
  {
    cout << "onStart:" << ret << endl;
    printf("FBService %x\n", fbs);
    if(ret){
      fbs->buzzer(true);
      list<string> listUserCode;
      cout << 1 << endl;
      fbs->getList(listUserCode);
      cout << 2 << endl;
    }
  }
  */
  
  virtual bool onNeedDeviceKey(char* id, char* key)
  {
    cout << "onNeedDeviceKey:" << id << endl;
    static char num[] = { 0,1,2,3,4,5,6,7,8,9, 0,0,0,0,0,0,0, 10,11,12,13,14,15};
    try{
      string strkey = "2C2DE78A4F113209";
      const char* str = strkey.c_str();
      for(int i = 0; i < 8; i++){
        key[i] = num[str[i*2] - 48]*16 + num[str[i*2+1] - 48];
      }
      return true;
    }
    catch(int e){
      printf("onNeedDeviceKey exception %d\n", e);
    }
    return false;
  }

  void onNeedUserCodeList(std::vector<pair<const char*, unsigned char*> >& arr_16, std::vector<pair<const char*, unsigned char*> >& arr_4)
  {
  }

  void onSyncComplete(bool){}
};

#define GETLIST
//#define DELETE
#define SAVE
#define SCAN


int main()
{
  Noti noti;
  cout << "start main\n" << endl;
  fbs = new FBService("/dev/ttyUSB0", Serial::SB38400, &noti, false);
  if(!fbs->start(true)){
    cout << "start fail!" << endl;
    return 1;
  }
  
  fbs->buzzer(false);
#ifdef GETLIST
  list<string> listUserCode;
  fbs->getList(listUserCode);
  ofstream oOut("usercodelist.txt");
  for(list<string>::iterator itr = listUserCode.begin(); itr != listUserCode.end(); itr++)
    oOut << *itr << endl;
  oOut.close();
#endif
#ifdef FORMAT
  fbs->format();
  sleep(5);
#endif
#ifdef DELETE
  fbs->deleteUsercode("0000000000000036");
  fbs->deleteUsercode("0000000000000038");
#endif
  
#ifdef SAVE  
  fbs->save("FID0000000000000012.bin");
#endif

#ifdef SCAN
  fbs->requestStartScan(300);
#endif
  //fbs->deleteUsercode(12);
  //fbs->requestStartScan(300);
  while(1){
    cout << "Koong" << endl;
    sleep(1);
  }
  cout << "exit" << endl;
  return 0;
}
