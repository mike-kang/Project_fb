#ifndef _FB_PROTOCOL_CM_SERIAL_HEADER
#define _FB_PROTOCOL_CM_SERIAL_HEADER

#include "tools/serial.h"
#include "fb_protocol.h"

using namespace tools;

class FBProtocolCMSerial : public FBProtocolCommMethod, public Serial {
public:
  FBProtocolCMSerial(const char* path, Baud baud):Serial(path, baud){}
  virtual ~FBProtocolCMSerial()
  {
  }
  
  virtual void onWrite(const char* buf, int length);
  virtual int onRead(char* buf, int len);

private:  

};




#endif //_FB_PROTOCOL_CM_SERIAL_HEADER

