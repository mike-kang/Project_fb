#ifndef _FB_PROTOCOL_HEADER
#define _FB_PROTOCOL_HEADER
#include <iostream>
#include <list>

typedef unsigned char byte;
class FBProtocol {
public:
  enum Exception {
    EXCEPTION_COMMAND,
    EXCEPTION_RESP_NOT_ACK,
    EXCEPTION_RESP_COMMMETHOD,
    EXCEPTION_RESP_CHECKSUM,
    EXCEPTION_RESP_ERROR,

    EXCEPTION_DIDR,
    EXCEPTION_DIDK,
    EXCEPTION_VERS,
//    EXCEPTION_STOP,
    EXCEPTION_STAT,
    EXCEPTION_STAT_LONG,
    EXCEPTION_INIT,
    EXCEPTION_USERS,
    EXCEPTION_USERD,
    EXCEPTION_USERE,
    EXCEPTION_SAVES,
    EXCEPTION_SAVED,
    EXCEPTION_SAVEE
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
  void didk(const char* key);
  char* vers();
  bool stop();
  char stat();
  char stat(char* data, bool& bLong);
  void init();
  bool user(std::list<std::string>& li);
  bool save(const char* filename);
  bool save(const byte* buf, int length);
  bool dele(const char* usercode);
  bool optf(byte* data);


private:  
  bool stat_loop_check(char check_char, byte& ret_char, int limit_count = 10);
  bool stat_loop_check2(char check_char, byte media_char, byte& ret_char, int limit_count = 10);
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

