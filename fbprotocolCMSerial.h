#ifndef _FB_PROTOCOL_CM_SERIAL_HEADER
#define _FB_PROTOCOL_CM_SERIAL_HEADER

#include "tools/serial.h"
#include "fb_protocol.h"

using namespace tools;

class FBProtocolCMSerial : public FBProtocol::FBProtocolCommMethod, public Serial {
public:
  FBProtocolCMSerial(const char* path, Baud baud):Serial(path, baud, 0, 0){}
  virtual ~FBProtocolCMSerial()
  {
  }
  
  virtual int onWrite(const char* buf, int length);
  virtual int onRead(char* buf, int len);
  virtual int onPoll(int timeout);
  
  
private:  

};




#endif //_FB_PROTOCOL_CM_SERIAL_HEADER

