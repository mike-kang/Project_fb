#ifndef _FB_PROTOCOL_HEADER
#define _FB_PROTOCOL_HEADER
#include <iostream>
#include <list>

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
    enum Exception {
      EXCEPTION_POLL,
      EXCEPTION_TIMEOUT,
    };
    virtual int onWrite(const char* buf, int length) = 0;
    virtual int onRead(char* buf, int len, int timeout) = 0;
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
  char* processCommand(const char* cmd, int timeout);
  char* processCommand(const char* cmd, const char* data, int data_sz, int timeout);
  char* response(int timeout);

  char userS();
  char userD(unsigned int id, std::list<std::string>& li, char& flag);
  char userE();

  
  FBProtocolCommMethod* m_cm;
};




#endif //_FB_PROTOCOL_HEADER

