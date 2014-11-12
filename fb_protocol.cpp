#include "fb_protocol.h"
#include "tools/log.h"
#include <stdio.h>
#include <cstring>
#include <iostream>
using namespace std;

#define LOG_TAG "FBProtocol"

#define SYNC 0x7e
#define NODE 0xb0
#define USER_COUNT 5
#define FLAG 15
#define STATUS 5
#define BLOCKID 11
#define LENGTH(buf) ((buf[1] << 8) + buf[2])
//beffer size
#define BUF_SZ_VERS 70 
//#define BUF_SZ_STAT_NORMAL 20
//#define BUF_SZ_USERS 13 //4+1+1+5+2
//#define BUF_SZ_USERD (16+21*USER_COUNT+2) //4+1+1+5+4+1+21*USER_COUNT + 2
//#define BUF_SZ_USERE 13
//#define BUF_SZ_AUTH 12


void dump(const char*str, byte* buf, int length)
{
  printf("[%s]\n", str);
  if(!length){
    printf("size = 0\n");
    return;
  }
  for(int i=0; i<length; i++){
    printf("0x%02x ", buf[i]);
  }
  putchar('\n');
  
}

//version 정보를 얻는다.
char* FBProtocol::vers()
{
  static char version[BUF_SZ_VERS];
  byte* receive_buf; 
  try {
    receive_buf = processCommand("VERS", 9000);
    memcpy(version, &receive_buf[10], BUF_SZ_VERS);
    printf("VERS:%s\n", version);
    delete receive_buf;
    return version;
  }
  catch(Exception e){
    cout << "[vers]exception fail! " << e << endl;
    return NULL;
  }
}

char FBProtocol::stat()
{
  char status;
  byte* receive_buf; 
  try {
    receive_buf = processCommand("STAT", 9000);
    status = receive_buf[STATUS];
    delete receive_buf;
    return status;  
  }
  catch(Exception e){
    cout << "[stat]exception fail! " << e << endl;
    return 0;
  }
}

char FBProtocol::stat(char* data, bool& bLong)
{
  char status;
  byte* receive_buf; 
  try {
    receive_buf = processCommand("STAT", 9000);
    //printf("STAT 0x%x('%c')\n", receive_buf[STATUS], receive_buf[STATUS]);
    status = receive_buf[STATUS];
    if(data){
      short length = LENGTH(receive_buf);
      bLong = (length == 30);
      memcpy(data, &receive_buf[10], length - 10); 
    }
    delete receive_buf;
    return status;  
  }
  catch(Exception e){
    cout << "[stat]exception fail! " << e << endl;
    return 0;
  }
}

/*
bool FBProtocol::auth()
{
  //static char receive_buf[BUF_SZ_AUTH];
  char status;
  int i;
  //processCommand("AUTH", receive_buf, BUF_SZ_AUTH, 9000);
  //printf("AUTH:%c\n", &receive_buf[10]);
  for(i=0;i<1000;i++){
    sleep(2);
    status = stat_normal();
    //if(status == 'A')
    //  break;
  }
  return true;
}
*/
//
bool FBProtocol::user(list<string>& li)
{
  byte status;
  int i;

  try {
    status = userS();
    
    if(status != '2'){
      printf("users fails\n");
      return false;
    }

    for(i=0;i<3;i++){
      usleep(500000);
      status = stat();
      if(status == '3')
        break;
    }
    if(i == 3){
      printf("users fails\n");
      return false;
    }

    int blockId = 1;
    char flag;
    do {
      status = userD(blockId, li, flag);
      if(flag == 'F')
        blockId++;
    }while(status != 'A');
    
    status = userE();
    if(status != 'A'){
      printf("userE fails\n");
      return false;
    }
  }
  catch(Exception e){
    cout << "[user]exception fail! " << e << endl;
    status = userE();
    printf("userE %c\n", status);
    return false;
  }

  return true;
}

byte FBProtocol::userS()
{
  byte status;
  byte* receive_buf; 
  byte data[2+ 20 + 20];
  byte* p = data;
  short count = USER_COUNT;
  data[0] = count >> 8;
  data[1] = count & 0xff;
  memset(p+2, 0x00, 20);
  memset(p+22, 0xff, 20);

  for(int i = 0; i < 3; i++){
    receive_buf = processCommand("USERS", data, 42, 9000);
    status = receive_buf[STATUS];
    delete receive_buf;
    if(status == '2')
      break;
    userE();
  }

  return status;
}

byte FBProtocol::userD(unsigned int id, list<string>& li, char& flag)
{
  byte status;
  byte* receive_buf; 
  char data[4];
  sprintf(data, "%04d",id);

  receive_buf = processCommand("USERD", (byte*)data, 4, 9000);
  flag = receive_buf[FLAG];
  char temp[4+1+20+1];
  receive_buf[FLAG] = '\0';
  memcpy(temp, &receive_buf[BLOCKID], 4);
  for(int i=16;i<LENGTH(receive_buf); i+=21){
  temp[4] = ':';
    byte* p = &receive_buf[i];
    p[20] = '\0';
    memcpy(&temp[5], p, 20);
    li.push_back(temp);
  }
  status = receive_buf[STATUS];
  delete receive_buf;
  return status;
    
}

byte FBProtocol::userE()
{
  //static char receive_buf[BUF_SZ_USERE];
  byte* receive_buf; 
  byte status;

  receive_buf = processCommand("USERE", 9000);
  status = receive_buf[STATUS];
  delete receive_buf;
  return status;
}


byte* FBProtocol::processCommand(const char* cmd, int timeout/*ms*/)
{
  byte buf[255];
  char _xor=0xff;
  unsigned char sum=0;

  //command
  short length = 4 + strlen(cmd);
  buf[0] = SYNC;
  buf[1] = length >> 8;
  buf[2] = length & 0xff;
  buf[3] = 0xb0;
  memcpy(&buf[4], cmd, 4);

  for(int i=3; i<length; i++){
    _xor ^= buf[i];
    sum += buf[i];
  }

  sum += _xor;
  buf[length] = _xor;
  buf[length+1] = sum;
  cout << "processCommand" << endl;
  char temp[20];
  sprintf(temp, "SEND %s", cmd); 
  dump(temp, buf, length + 2);
    
  int writebyte = m_cm->onWrite(buf, length + 2);
  if(writebyte < length + 2)
    throw EXCEPTION_COMMAND;
  
  //response
  return response(timeout);
  
}

byte* FBProtocol::processCommand(const char* cmd, const byte* data, int data_sz, int timeout)
{
  byte buf[255];
  byte _xor=0xff;
  byte sum=0;

  //command
  int cmd_sz = strlen(cmd);
  short length = 4 + cmd_sz + data_sz;
  buf[0] = SYNC;
  buf[1] = length >> 8;
  buf[2] = length & 0xff;
  buf[3] = NODE;
  memcpy(&buf[4], cmd, strlen(cmd));
  memcpy(&buf[4 + strlen(cmd)], data, data_sz);

  for(int i=3; i<length; i++){
    _xor ^= buf[i];
    sum += buf[i];
  }

  sum += _xor;
  buf[length] = _xor;
  buf[length+1] = sum;
  cout << "processCommand" << endl;
  char temp[20];
  sprintf(temp, "SEND %s", cmd); 
  dump(temp, buf, length + 2);
    
  int writebyte = m_cm->onWrite(buf, length + 2);
  if(writebyte < length + 2)
    throw EXCEPTION_COMMAND;
  
  //response
  return response(timeout);
    
}

byte* FBProtocol::response(int timeout)
{
  byte _xor = 0xff;
  byte sum = 0;
  byte tempBuf[3];
  int readbyte = m_cm->onRead(tempBuf, 3, timeout);
  while(readbyte < 3){
    int t = m_cm->onRead(tempBuf + readbyte, 3 - readbyte, timeout);
    readbyte += t;
  }
  if(tempBuf[0] != SYNC)
    throw EXCEPTION_NOT_ACK;
  printf("readbyte %d\n", readbyte);
  //dump("RECEIVE", receiveBuf, readbyte);
  short length = LENGTH(tempBuf) + 2;
  byte* receiveBuf = new byte[length];
  memcpy(receiveBuf, tempBuf, readbyte);
  
  //cout << "sendCommandNoData: readbyte: " << readbyte << ", length: " << length << endl;
  int leavebyte = length - readbyte;
  if(leavebyte){
    byte* p = receiveBuf + readbyte;
    while(leavebyte){
      readbyte = m_cm->onRead(p, leavebyte, timeout);
      //cout << "sendCommandNoData: readbyte: " << readbyte<< endl;
      leavebyte -= readbyte;
      p += readbyte;
    }
  }
  dump("RECEIVE", receiveBuf, length);
  printf("status 0x%x('%c')\n", receiveBuf[STATUS], receiveBuf[STATUS]);

  //check checksum
  _xor=0xff;
  sum=0;
  for(int i=4; i< length - 2; i++){
    _xor ^= receiveBuf[i];
    sum += receiveBuf[i];
  }
  sum += _xor;
  if(receiveBuf[length-2] != _xor || receiveBuf[length-1] != sum){
    delete receiveBuf;
    throw EXCEPTION_CHECKSUM;  
  }
  return receiveBuf;
}
