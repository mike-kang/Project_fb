#include "fb_protocol.h"
#include "tools/log.h"
#include <stdio.h>
#include <cstring>
#include <iostream>

using namespace std;

#define LOG_TAG "FBProtocol"

#define SYNC 0x7e
#define NODE 0xb0

void dump(char*str, char* buf, int length)
{
  printf("[%s] ", str);
  for(int i=0; i<length; i++){
    printf("0x%x, ", buf[i]);
  }
  putchar('\n');
  

}

char* FBProtocol::vers()
{
  static char buf[82];
  sendCommandNoData("VERS", buf, 82);
  printf("VERS:%s\n", &buf[10]);
  return &buf[10];
}

bool FBProtocol::sendCommandNoData(const char* cmd, char* receiveBuf, int receiveBufSize)
{
  char buf[255];
  char _xor=0;
  unsigned char sum=0;

  short length = 4 + strlen(cmd);
  buf[0] = SYNC;
  buf[1] = length >> 8;
  buf[2] = length & 0xff;
  buf[3] = NODE;
  strcpy(&buf[4], cmd);

  for(int i=3; i<length; i++){
    _xor ^= buf[i];
    sum += buf[i];
  }
  _xor = ~_xor;
  sum += _xor;
  buf[length] = _xor;
  buf[length+1] = sum;
  cout << "sendCommandNoData" << endl;
  dump("SEND", buf, length + 2);
    
  int writebyte = m_cm->onWrite(buf, length + 2);
  if(writebyte < length + 2)
    throw EXCEPTION_WRITE;
  
  //response
  int readbyte = m_cm->onRead(receiveBuf, receiveBufSize);
  if(readbyte < 3){
    int t = m_cm->onRead(receiveBuf + readbyte, receiveBufSize - readbyte);
    readbyte += t;
  }
  
  dump("RECEIVE", receiveBuf, readbyte);
  length = (receiveBuf[1]<<8) + receiveBuf[2] + 2;
  if(receiveBuf[0] != SYNC)
    throw EXCEPTION_NOT_ACK;
  cout << "sendCommandNoData: readbyte: " << readbyte << ", length: " << length << endl;
  int leavebyte = length - readbyte;
  if(leavebyte){
    char* p = receiveBuf + readbyte;
    while(leavebyte){
      readbyte = m_cm->onRead(p, 255);
      cout << "sendCommandNoData: readbyte: " << readbyte<< endl;
      leavebyte -= readbyte;
      p += readbyte;
    }
  }
  
}

