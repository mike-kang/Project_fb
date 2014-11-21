#ifndef _FB_PROTOCOL_CM_SERIAL_HEADER
#define _FB_PROTOCOL_CM_SERIAL_HEADER

#include "tools/asyncserial.h"
#include "fb_protocol.h"

using namespace tools;

class FBProtocolCMSerial : public FBProtocol::FBProtocolCommMethod, public AsyncSerial {
public:
  FBProtocolCMSerial(const char* path, Serial::Baud baud):AsyncSerial(path, baud){}
  virtual ~FBProtocolCMSerial()
  {
  }
  
  virtual int onWrite(const byte* buf, int length);
  virtual int onRead(byte* buf, int len, int timeout);
  //virtual int onPoll(int timeout);
  
  
};




#endif //_FB_PROTOCOL_CM_SERIAL_HEADER

