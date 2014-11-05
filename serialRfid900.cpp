#include "serialRfid900.h"
#include "tools/log.h"
#include <stdio.h>

#define LOG_TAG "SerialRfid900"

#define CMD_OPEN 
bool SerialRfid900::open()
{
  LOGI("open +++\n");
  SerialRfid::open();
  static const char cmd[] = {0x7E,0x01,0x00,0x00,0x03,0x01,0x10,0x00,0x11,0x7F};
  m_serial.write(cmd, sizeof(cmd));

  
  char buf[5];
  int len = m_serial.read(buf,5);
  LOGI("open ---\n");
} 

//-1:fail 0:same 1:success
int SerialRfid900::requestData()
{
  static const char buf[] = {0x7E,0x01,0x00,0x00,0x03,0x01,0x10,0x01,0x10,0x7F};
  static time_t shadowTime = 0;

  m_serial.write(buf, sizeof(buf));

  int len = m_serial.read(m_reciveBuf, RECEIVE_BUF_SIZE);
  if(len < 27)
    return -1;
  if(m_reciveBuf[0] == 0x7E && m_reciveBuf[26] == 0xFF && m_reciveBuf[5] == 0xA0){
    time_t t = time(NULL);
    sprintf(m_serialnumberBuf, "%02x%02x", m_reciveBuf[18],m_reciveBuf[19]);
    if((t - shadowTime < 6) && !strncmp(m_serialnumberBuf, m_serialnumberShadowBuf, SERIALNUMBER_BUF_SIZE-1)){
      LOGV("same serial number\n");
      return 0;
    }
    strcpy(m_serialnumberShadowBuf, m_serialnumberBuf);
    shadowTime = t;
    return 1;
  }
  return -1;
}

