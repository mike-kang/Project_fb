#ifndef _FB_PROTOCOL_HEADER
#define _FB_PROTOCOL_HEADER

class FBProtocol {
public:
  enum Exception {
    EXCEPTION_WRITE,
    EXCEPTION_NOT_ACK,
  };
  class FBProtocolCommMethod {
  public:
    virtual int onWrite(const char* buf, int length) = 0;
    virtual int onRead(char* buf, int len) = 0;
  };


  FBProtocol(FBProtocolCommMethod* cm):m_cm(cm){}
  virtual ~FBProtocol()
  {
  }
  
  char* vers();
  //bool stat();
  //bool save();
  //bool dele();

private:  
  bool sendCommandNoData(const char* cmd, char* receiveBuf, int receiveBufSize);

  FBProtocolCommMethod* m_cm;
};




#endif //_FB_PROTOCOL_HEADER

