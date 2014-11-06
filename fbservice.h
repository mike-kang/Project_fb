#ifndef _FB_SERVICE_HEADER
#define _FB_SERVICE_HEADER

#include "fbprotocolCMSerial.h"
#include "fb_protocol.h"

using namespace tools;

class FBService {
public:
  FBService(const char* path, Serial::Baud baud);

  virtual ~FBService()
  {
    if(m_thread) delete m_thread;
  }
  
  bool start();
  void stop();
  int requestStartScan(int interval);
  int requestEndScan();

private:  
  void run();

  FBProtocolCMSerial* m_serial;
  FBProtocol* m_protocol;
  
  Thread<FBService>* m_thread;
  bool m_running;
  int m_interval; //msec
};




#endif

