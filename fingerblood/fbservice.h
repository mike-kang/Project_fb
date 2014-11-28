#ifndef _FB_SERVICE_HEADER
#define _FB_SERVICE_HEADER

#include "fbprotocolCMSerial.h"
#include "fb_protocol.h"
#include <iostream>
#include <list>
#include <vector>
#include <map>
#include "tools/timer.h"

using namespace tools;

class FBService {
public:
  class FBServiceNoti {
  public:
    virtual void onScanData(const char* buf) = 0;
    //virtual void onStart(bool ret) = 0;
    virtual bool onNeedDeviceKey(char* id, char* key) = 0;
    virtual void onNeedUserCodeList(std::map<const char*, unsigned char*>& arr_16, std::map<const char*, unsigned char*>& arr_4) = 0;
    //virtual std::map<string, EmployeeInfo*>& onNeedUserCodeList() = 0;
    virtual void onSync(bool) = 0;
  };
  FBService(const char* path, Serial::Baud baud, FBServiceNoti* fn, bool bCheckUserCode4);
  virtual ~FBService();

  bool start(bool check = false);
  void stop();
  char* getVersion();
  bool getList(list<string>& li);
  bool format();  //auto restart
  bool requestStartScan(int interval);
  int requestStopScan();
  bool save(const char* filename);
  bool save(const byte* buf, int length);
  bool deleteUsercode(const char* usercode);
  
  void buzzer(bool val);
  void sync();
private:  
  bool checkdeviceID();
  void run_scan();
  void run_sync();
  static void cbTimerFormat(void* arg);

  bool m_bActive;
  FBProtocolCMSerial* m_serial;
  FBProtocol* m_protocol;
  
  Thread<FBService>* m_thread_scan;
  bool m_scan_running;
  int m_scan_interval; //msec
  FBServiceNoti* m_fn;
  Timer* m_TimerRestart;

  Thread<FBService>* m_thread_sync;
  bool m_sync_running;
  bool m_bCheckUserCode4;
};




#endif

