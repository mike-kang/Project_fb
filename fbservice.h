#ifndef _FB_SERVICE_HEADER
#define _FB_SERVICE_HEADER

#include "fbprotocolCMSerial.h"
#include "fb_protocol.h"

using namespace tools;

class FBSerivce {
public:
  SerialRfid(const char* path, Serial::Baud baud):m_serial(path, baud), m_reciveBuf(NULL), m_serialnumberBuf(NULL), m_dn(NULL){
  }

  virtual ~SerialRfid()
  {
    if(m_reciveBuf) delete m_reciveBuf;
    if(m_serialnumberBuf) delete m_serialnumberBuf;
    if(m_thread) delete m_thread;
  }
  
  virtual bool open();
  virtual int close();
  virtual void start(int interval, SerialRfidDataNoti* dn);
  virtual void stop();
  
  virtual int requestData() = 0; //-1:fail 0:same 1:success
  
  void registerDataNoti(SerialRfidDataNoti* dn);

protected:
  Serial m_serial;
  char* m_reciveBuf;
  char* m_serialnumberBuf;
  
private:  
  void run();

  SerialRfidDataNoti* m_dn;
  Thread<SerialRfid>* m_thread;
  bool m_running;
  int m_interval; //msec
};




#endif

