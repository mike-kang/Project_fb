#include <iostream>
#include <stdio.h>
#include "fbservice.h"
#include <fstream>
#include <vector>
#include <stdlib.h>
#include "tools/timer.h"
#include "tools/log.h"

using namespace std;
using namespace tools;

FBService* fbs;
#define USERDATA_SIZE 864
#define LOG_TAG "TEST"

void cbTimerformat(void* arg)
{
  fbs->request_closeDevice();
  cout << "opendevice" << endl;
  if(!fbs->request_openDevice(true)){
    cout << "openDevice fail!" << endl;
    return ;
  }
  cout << "opendevice--" << endl;
}

class Noti: public IFBService::IFBServiceEventListener {
public:  
  virtual void onScanData(const char* buf)
  {
    if(buf)
      cout << "onScanData:" << buf << endl;
    else
      cout << "onScanData Null" << endl;
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
    LOGV("onNeedDeviceKey:%s\n");
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

  void onSync(IFBService::IFBServiceEventListener::SyncStatus status, int index=0){}

  void onFormat(bool ret)
  {
    LOGV("onformat %d\n", ret);

    Timer* timer = new Timer(cbTimerformat, NULL);
    timer->start(7);     
  }
  
  void onUpdateSave(int index){}
  void onUpdateDelete(int index){}

};

const char* save_list[] = {
//  "FID0000000000000001",
//  "FID0000000000000002",
//  "FID0000000000000003",
//  "FID0000000000000004",
//  "FID0000000000000005",
//  "FID0000000000000006",
//  "FID0000000000000007",
  "FID0000000000000012",
};
#define GETLIST
//#define DELETE
#define SAVE
//#define SCAN
//#define FORMAT

void help()
{
    printf("Usage: ./test [OPTION] [FILE]\n"
           "  -h                help\n"
           "  -s [FILE]         save\n"
           "  -f                format\n"
           "  -d usercode       delete\n"
           "  -l                list\n");
    exit(0);
}

int main(int argc, char* argv[])
{
  Noti noti;
  int opt;
  //cout << "start main\n" << endl;
  
  if(argc < 2)
    help(); //exit(0)

  log_init(true, 1, "/dev/pts/1", false, 3, "Log");
  fbs = new FBService("/dev/ttyUSB0", Serial::SB38400, &noti, false);
  cout << "opendevice" << endl;
  if(!fbs->request_openDevice(true)){
    cout << "openDevice fail!" << endl;
    return 1;
  }
  cout << "opendevice--" << endl;
  fbs->request_buzzer(true);
  
  while((opt = getopt(argc, argv, "lfs:cd:")) != -1) 
  {
      switch(opt) 
      { 
          case 'l':
            {
              list<string> listUserCode;
              bool ret = fbs->request_getList(&listUserCode);
              if(ret){
                ofstream oOut("usercodelist.txt");
                for(list<string>::iterator itr = listUserCode.begin(); itr != listUserCode.end(); itr++){
                  oOut << *itr << endl;
                }
                oOut.close();
                cout << "usercodelist.txt saved" << endl;
              }
            }
            break; 
          case 'f':
            fbs->request_format();  
            //exit(0);
            break;
          case 's':
            /*
              for(int i=0; i<sizeof(save_list)/sizeof(char*); i++){
                char buf[USERDATA_SIZE];
                ifstream infile (save_list[i], ofstream::binary);
                infile.read (buf, USERDATA_SIZE);
                infile.close();
                fbs->save((const unsigned char*)buf, USERDATA_SIZE);
              }
            */
            char filename[255];
            fbs->request_saveUsercode(optarg);
            cout << "save:" << optarg << endl;
            break;
          case 'c':
            cout << "start scan" << endl;
            fbs->request_startScan(300);
            break;
          case 'd':
            cout << "delete:" << optarg << endl;
            fbs->request_deleteUsercode(optarg);
            break;
      }
  } 

  
#ifdef DELETE
  for(int i=0; i<sizeof(save_list)/sizeof(char*); i++){
    fbs->request_deleteUsercode(save_list[i]);
  }
#endif

#ifdef SCAN
  fbs->request_startScan(300);
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
