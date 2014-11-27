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
    EXCEPTION_ERROR,
    EXCEPTION_USERS,
    EXCEPTION_SAVED,
    EXCEPTION_COMMMETHOD,
  };
  class FBProtocolCommMethod {
  public:
    enum Exception {
      EXCEPTION_WRITE,
      EXCEPTION_READ,
    };
    virtual int onWrite(const byte* buf, int length) = 0;
    virtual int onRead(byte* buf, int len, int timeout) = 0;
  };


  FBProtocol(FBProtocolCommMethod* cm):m_cm(cm){}
  virtual ~FBProtocol()
  {
  }

  
  char* didr();
  bool didk(const char* key);
  char* vers();
  char stat();
  char stat(char* data, bool& bLong);
  bool init();
  bool user(std::list<std::string>& li);
  bool save(const char* filename);
  bool save(const byte* buf, int length);
  bool dele(unsigned short usercode);
  bool optf(byte* data);


private:  
  byte* processCommand(const char* cmd, int timeout);
  byte* processCommand(const char* cmd, const byte* data, int data_sz, int timeout);
  byte* processCommand(const byte* chunk, int chunk_sz, int timeout);
  byte* response(int timeout);

  void userS();
  byte userD(unsigned int id, std::list<std::string>& li, char& flag);
  void userE();
  void saveS();
  void saveD(const char* filename);
  void saveD(const byte* buf, int length);
  void saveE();

  
  FBProtocolCommMethod* m_cm;
};




#endif //_FB_PROTOCOL_HEADER

