#ifndef _FB_PROTOCOL_HEADER
#define _FB_PROTOCOL_HEADER
#include <iostream>
#include <list>

typedef unsigned char byte;
class FBProtocol {
public:
  enum Exception {
    EXCEPTION_COMMAND,
    EXCEPTION_NOT_ACK,
    EXCEPTION_CHECKSUM,
    EXCEPTION_POLL,
    EXCEPTION_TIMEOUT,
  };
  class FBProtocolCommMethod {
  public:
    virtual int onWrite(const byte* buf, int length) = 0;
    virtual int onRead(byte* buf, int len, int timeout) = 0;
  };


  FBProtocol(FBProtocolCommMethod* cm):m_cm(cm){}
  virtual ~FBProtocol()
  {
  }
  
  char* vers();
  char stat();
  char stat(char* data, bool& bLong);
  bool user(std::list<std::string>& li);
  //bool auth();
  //bool stat();
  //bool save();
  //bool dele();

private:  
  byte* processCommand(const char* cmd, int timeout);
  byte* processCommand(const char* cmd, const byte* data, int data_sz, int timeout);
  byte* response(int timeout);

  byte userS();
  byte userD(unsigned int id, std::list<std::string>& li, char& flag);
  byte userE();

  
  FBProtocolCommMethod* m_cm;
};




#endif //_FB_PROTOCOL_HEADER

