#ifndef _FB_PROTOCOL_HEADER
#define _FB_PROTOCOL_HEADER

class FBProtocol {
public:
  class FBProtocolCommMethod {
  public:
    virtual void onWrite(const char* buf, int length) = 0;
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
  bool sendCommandNoData(const char* cmd);

  FBProtocolCommMethod* m_cm;
};




#endif //_FB_PROTOCOL_HEADER

