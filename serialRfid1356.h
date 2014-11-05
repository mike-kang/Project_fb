#ifndef _SERIALR_RFID1356_HEADER
#define _SERIALR_RFID1356_HEADER

#include "serialRfid.h"

#define RECEIVE_BUF_SIZE 100
//#define SERIALNUMBER_BUF_SIZE 13 //3*4 +1

class SerialRfid1356 : public SerialRfid{
public:
  static const int SERIALNUMBER_BUF_SIZE = 13; //3*4 +1
  SerialRfid1356(const char* path):SerialRfid(path, Serial::SB38400)
  {
    m_reciveBuf = new char[RECEIVE_BUF_SIZE];
    m_serialnumberBuf = new char[SERIALNUMBER_BUF_SIZE];
    m_serialnumberShadowBuf = new char[SERIALNUMBER_BUF_SIZE];
    m_serialnumberShadowBuf[0] = '\0';
  }

  virtual bool open();
  virtual int requestData();

private:
  char* m_serialnumberShadowBuf;
};

class SerialRfid1356_ : public SerialRfid1356{
public:
  SerialRfid1356_(const char* path):SerialRfid1356(path){}

  virtual bool open();
};



#endif

