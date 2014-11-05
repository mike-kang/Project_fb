#include "fb_protocol.h"
#include "tools/log.h"
#include <stdio.h>
#include <iostream>

using namespace std;

#define LOG_TAG "FBProtocol"

#define SYNC 0x7e
#define NODE 0xb0

char* FBProtocol::vers()
{
  static char buf[82];
  sendCommandNoData("vers", buf, 82);
  cout << &buf[10] << endl;
}

bool FBProtocol::sendCommandNoData(const char* cmd, char* receiveBuf, int receiveBufSize)
{
  char buf[255];
  
  short length = 4 + strlen(cmd);
  buf[0] = SYNC;
  buf[1] = length >> 8;
  buf[2] = length & 0xff;
  buf[3] = NODE;
  strcpy(&buf[4], cmd);

  int xor=0, sum=0;
  for(int i=3; i<length; i++){
    xor ^= buf[i];
    sum += buf[i];
  }
  sum += xor;
  buf[length] = xor;
  buf[length+1] = sum;
    
  m_cm->onWrite(buf, length + 2);

  //response
  int readbyte = m_el->onRead(receiveBuf, receiveBufSize);
  length = buf[1]<<8 + buf[2] + 4 + 2;
  int leavebyte = length - readbyte;
  cout << "sendCommandNoData: " << leavebyte << endl;
  if(leavebyte){
    char* p = receiveBuf + readbyte;
    while(leavebyte){
      readbyte = m_cm->onRead(p, 255);
      leavebyte -= readbyte;
      p += readbyte;
    }
  }
  
}

