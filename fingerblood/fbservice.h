#ifndef _FB_SERVICE_HEADER
#define _FB_SERVICE_HEADER

#include "fbprotocolCMSerial.h"
#include "fb_protocol.h"
#include <iostream>
#include <list>
#include "tools/timer.h"

using namespace tools;

class FBService {
public:
  class FBServiceNoti {
  public:
    virtual void onScanData(const char* buf) = 0;
    virtual void onStart(bool ret) = 0;
    virtual bool onNeedDeviceKey(char* id, char* key) = 0;
  };
  FBService(const char* path, Serial::Baud baud, FBServiceNoti* fn);
  virtual ~FBService();

  bool start(bool check = false);
  void stop();
  bool deviceKey();
  char* getVersion();
  bool getList(list<string>& li);
  bool format();  //auto restart
  bool requestStartScan(int interval);
  int requestEndScan();
  bool save(const char* filename);
  bool deleteUsercode(unsigned short usercode);
  
  void buzzer(bool val);
  
private:  
  void run_scan();
  void run_format();
  static void cbTimerFormat(void* arg);

  bool m_bActive;
  FBProtocolCMSerial* m_serial;
  FBProtocol* m_protocol;
  
  Thread<FBService>* m_thread_scan;
  bool m_scan_running;
  int m_scan_interval; //msec
  FBServiceNoti* m_fn;
  Timer* m_TimerRestart;
  
};




#endif

