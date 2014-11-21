#ifndef _FB_SERVICE_HEADER
#define _FB_SERVICE_HEADER

#include "fbprotocolCMSerial.h"
#include "fb_protocol.h"
#include <iostream>
#include <list>

using namespace tools;

class FBService {
public:
  class FBServiceNoti {
  public:
    virtual void onScanData(const char* buf) = 0;
    virtual void onFinishFormat(void* client) = 0;
  };
  FBService(const char* path, Serial::Baud baud, FBServiceNoti* fn);

  virtual ~FBService()
  {
    if(m_thread) delete m_thread;
  }

  
  char* getVersion();
  bool start();
  void stop();
  bool format();
  bool requestStartScan(int interval);
  int requestEndScan();
  bool save(const char* filename);
  bool deleteUsercode(unsigned short usercode);
  
private:  
  void run_scan();
  void run_format();

  FBProtocolCMSerial* m_serial;
  FBProtocol* m_protocol;
  
  Thread<FBService>* m_thread_scan;
  Thread<FBService>* m_thread_foramt;
  bool m_running;
  int m_interval; //msec
  std::list<std::string> listUserCode;
  ScanDataNoti* m_dn;
  
};




#endif

