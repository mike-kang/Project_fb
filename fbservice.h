#ifndef _FB_SERVICE_HEADER
#define _FB_SERVICE_HEADER

#include "fbprotocolCMSerial.h"
#include "fb_protocol.h"
#include <iostream>
#include <list>

using namespace tools;

class FBService {
public:
  class ScanDataNoti {
  public:
    virtual void onData(const char* buf) = 0;
    //virtual void onSameData() = 0;
  };
  FBService(const char* path, Serial::Baud baud);

  virtual ~FBService()
  {
    if(m_thread) delete m_thread;
  }
  
  bool start();
  void stop();
  bool requestStartScan(int interval);
  int requestEndScan();
  bool save(const char* filename);
  bool deleteUsercode(unsigned short usercode);
  
private:  
  void run();

  FBProtocolCMSerial* m_serial;
  FBProtocol* m_protocol;
  
  Thread<FBService>* m_thread;
  bool m_running;
  int m_interval; //msec
  std::list<std::string> listUserCode;
  ScanDataNoti* m_dn;
};




#endif

