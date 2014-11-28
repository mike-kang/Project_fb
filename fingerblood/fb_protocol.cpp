#include "fb_protocol.h"
#include "tools/log.h"
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include "tools/utils.h"

using namespace std;
using namespace tools;

#define LOG_TAG "FBProtocol"

#define SYNC 0x7e
#define NODE 0xb0
#define USER_COUNT 5

/* index */
#define FLAG 15
#define STATUS 5
#define BLOCKID 11

#define ENCRYPTION '2'
#define FINGER_COUNT_IN_FILE 3

#define TOSHORT(buf) (((buf)[0] << 8) + (buf)[1])
//beffer size
#define BUF_SZ_VERS 70 
#define BUF_SZ_DEVICE_ID 8 
//#define BUF_SZ_STAT_NORMAL 20
//#define BUF_SZ_USERS 13 //4+1+1+5+2
//#define BUF_SZ_USERD (16+21*USER_COUNT+2) //4+1+1+5+4+1+21*USER_COUNT + 2
//#define BUF_SZ_USERE 13
//#define BUF_SZ_AUTH 12
#define STAT_LOOP_CHECK(k)    do {                    \
                                usleep(100000);       \
                                status = stat();      \
                              } while(status != (k))

bool _debug = false;

char* FBProtocol::didr()
{
  static char device_id[BUF_SZ_DEVICE_ID];
  byte* receive_buf; 
  receive_buf = processCommand("DIDR", 1000);
  memcpy(device_id, &receive_buf[10], BUF_SZ_DEVICE_ID);
  delete receive_buf;
  return device_id;
}

bool FBProtocol::didk(const char* key)
{
  char status;
  byte* receive_buf; 
  
  try {
    receive_buf = processCommand("DIDK", (const byte*)key, 8, 1000);
    status = receive_buf[STATUS];
    if(status != 'A')
      STAT_LOOP_CHECK('A');
    return true;
  }
  catch(Exception e){
    cout << "[didk]exception fail! " << e << endl;
    return false;
  }
  return false;
}


//version 정보를 얻는다.
char* FBProtocol::vers()
{
  static char version[BUF_SZ_VERS];
  byte* receive_buf; 
  receive_buf = processCommand("VERS", 1000);
  memcpy(version, &receive_buf[10], BUF_SZ_VERS);
  printf("VERS:%s\n", version);
  delete receive_buf;
  return version;
}

char FBProtocol::stat()
{
  char status;
  byte* receive_buf; 
  receive_buf = processCommand("STAT", 1000);
  status = receive_buf[STATUS];
  delete receive_buf;
  return status;  
}

char FBProtocol::stat(char* data, bool& bLong)
{
  char status;
  byte* receive_buf; 
  try {
    receive_buf = processCommand("STAT", 2000);
    //printf("STAT 0x%x('%c')\n", receive_buf[STATUS], receive_buf[STATUS]);
    status = receive_buf[STATUS];
    if(data){
      short length = TOSHORT(receive_buf + 1);
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

bool FBProtocol::init()
{
  char status;
  byte* receive_buf; 
  
  try {
    receive_buf = processCommand("INIT", 5000);
    status = receive_buf[STATUS];
    if(status == '2')
      return true;
  }
  catch(Exception e){
    cout << "[init]exception fail! " << e << endl;
    return false;
  }
  return false;
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
    userS();
    
    int blockId = 1;
    char flag;
    do {
      status = userD(blockId, li, flag);
      printf("flag %c\n", flag);
      if(status == 'A' && flag == 'F')
        blockId++;
    }while(status != 'A' || flag != 'f');
    
    userE();
  }
  catch(Exception e){
    cout << "[user]exception fail! " << e << endl;
    return false;
  }

  return true;
}

bool FBProtocol::save(const char* filename)
{
  byte status;
  int i;

  try {
    saveS();
    saveD(filename);
    saveE();
  }
  catch(Exception e){
    cout << "[save]exception fail! " << e << endl;
    return false;
  }

  return true;
}

bool FBProtocol::save(const byte* buf, int length)
{
  byte status;
  int i;

  try {
    saveS();
    saveD(buf, length);
    saveE();
  }
  catch(Exception e){
    cout << "[save]exception fail! " << e << endl;
    return false;
  }

  return true;
}

bool FBProtocol::dele(const char* usercode)
{
  char status;
  byte* receive_buf; 
  char sUsercode[21];
  int i;
  strncpy(sUsercode, usercode, 16);
  strcat(sUsercode, "0000");

  try {
    receive_buf = processCommand("DELE", (byte*)sUsercode, 20, 1000);
    status = receive_buf[STATUS];
    delete receive_buf;

    if(status != '2')
      return false;
    STAT_LOOP_CHECK('A');
    return true;  
  }
  catch(Exception e){
    cout << "[dele]exception fail! " << e << endl;
    return false;
  }
}

bool FBProtocol::optf(byte* data)
{
  char status;
  byte* receive_buf; 
  
  try {
    receive_buf = processCommand("OPTF", data, 2, 1000);
    status = receive_buf[STATUS];
    if(status != 'A')
      STAT_LOOP_CHECK('A');
    return true;
  }
  catch(Exception e){
    cout << "[optf]exception fail! " << e << endl;
    return false;
  }
  return false;
}



void FBProtocol::userS()
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

  receive_buf = processCommand("USERS", data, 42, -1);
  status = receive_buf[STATUS];
  delete receive_buf;
  if(status != '2')
    throw EXCEPTION_USERS;
  STAT_LOOP_CHECK('3');
}

byte FBProtocol::userD(unsigned int block_id, list<string>& li, char& flag)
{
  byte status;
  byte* receive_buf; 
  char data[4];
  sprintf(data, "%04d",block_id);

  receive_buf = processCommand("USERD", (byte*)data, 4, 9000);
  flag = receive_buf[FLAG];
  char temp[4+1+20+1];
  receive_buf[FLAG] = '\0';
  memcpy(temp, &receive_buf[BLOCKID], 4);
  temp[4] = ':';
  temp[25] = '\0';
  int length = TOSHORT(receive_buf+1);
  for(int i=16;i < length; i+=21){
  
    byte* p = &receive_buf[i];

    memcpy(&temp[5], p, 20);
    li.push_back(temp);
  }
  status = receive_buf[STATUS];
  delete receive_buf;
  return status;
    
}

void FBProtocol::userE()
{
  //static char receive_buf[BUF_SZ_USERE];
  byte* receive_buf; 
  byte status;

  receive_buf = processCommand("USERE", 9000);
  status = receive_buf[STATUS];
  delete receive_buf;
  if(status != 'A')
    STAT_LOOP_CHECK('A');
}

void FBProtocol::saveS()
{
  byte* receive_buf; 
  byte status;

  receive_buf = processCommand("SAVES", 1000);
  status = receive_buf[STATUS];
  delete receive_buf;
  if(status != '3')
    STAT_LOOP_CHECK('3');
}


void FBProtocol::saveD(const char* filename)
{
  int i;
  byte status;
  byte* receive_buf; 
  ifstream infile (filename, ofstream::binary);
  // get size of file
  infile.seekg (0,infile.end);
  long fingerinfo_size = infile.tellg() / FINGER_COUNT_IN_FILE;
  infile.seekg (0);

  int length = 12 + fingerinfo_size;
  byte* buf = new byte[length + 2];
  byte _xor = 0xff;
  byte sum = 0;
  unsigned short serial = 0;

  //command
  buf[0] = SYNC;
  buf[1] = length >> 8;
  buf[2] = length & 0xff;
  buf[3] = NODE;
  memcpy(&buf[4], "SAVED", 5);
  buf[9] = ENCRYPTION;

  do {

    buf[10] = serial >> 8;
    buf[11] = serial & 0xff;
    

    infile.read ((char*)&buf[12], fingerinfo_size);


    for(int i=3; i<length; i++){
      _xor ^= buf[i];
      sum += buf[i];
    }

    sum += _xor;
    buf[length] = _xor;
    buf[length+1] = sum;
    
    cout << "processCommand" << endl;
    if(_debug) utils::hexdump("SEND SAVED", buf, length + 2);
    try{
      receive_buf = processCommand(buf, length + 2, 9000); //delete buf;
    }
    catch(Exception e){
      infile.close();
      delete buf;
      throw EXCEPTION_SAVED;
    }
    
    //serial = TOSHORT(receive_buf+12);
    //errCode = TOSHORT(receive_buf+14);
    //restCmd = TOSHORT(receive_buf+16);
    //printf("serial %d, errCode %d, restCmd %d\n", serial, errCode, restCmd);

    status = receive_buf[STATUS];
    delete receive_buf;

    if(status == '3'){
      serial++;
      if(serial == FINGER_COUNT_IN_FILE)
        break;

    }
    else{
      STAT_LOOP_CHECK('3');
      serial++;
      if(serial == FINGER_COUNT_IN_FILE)
        break;
    }
    
    usleep(200000);
  }while(1);
  delete buf;
  infile.close();
}

void FBProtocol::saveD(const byte* userdata, int len)
{
  int i;
  byte status;
  byte* receive_buf; 
  int fingerinfo_size = len / FINGER_COUNT_IN_FILE;
  const byte* offset = userdata;;
  int length = 12 + fingerinfo_size;
  byte* buf = new byte[length + 2];
  byte _xor = 0xff;
  byte sum = 0;
  unsigned short serial = 0;

  //command
  buf[0] = SYNC;
  buf[1] = length >> 8;
  buf[2] = length & 0xff;
  buf[3] = NODE;
  memcpy(&buf[4], "SAVED", 5);
  buf[9] = ENCRYPTION;

  do {

    buf[10] = serial >> 8;
    buf[11] = serial & 0xff;
    
    memcpy(&buf[12], offset, fingerinfo_size);
    offset += fingerinfo_size;
    for(int i=3; i<length; i++){
      _xor ^= buf[i];
      sum += buf[i];
    }

    sum += _xor;
    buf[length] = _xor;
    buf[length+1] = sum;
    
    cout << "processCommand" << endl;
    if(_debug) utils::hexdump("SEND SAVED", buf, length + 2);
    try{
      receive_buf = processCommand(buf, length + 2, 9000); //delete buf;
    }
    catch(Exception e){
      delete buf;
      throw EXCEPTION_SAVED;
    }
    
    status = receive_buf[STATUS];
    delete receive_buf;

    if(status == '3'){
      serial++;
      if(serial == FINGER_COUNT_IN_FILE)
        break;

    }
    else{
      STAT_LOOP_CHECK('3');
      serial++;
      if(serial == FINGER_COUNT_IN_FILE)
        break;
    }
    
    usleep(200000);
  }while(1);
  delete buf;
}

void FBProtocol::saveE()
{
  byte* receive_buf; 
  byte status;

  receive_buf = processCommand("SAVEE", 1000);
  status = receive_buf[STATUS];
  delete receive_buf;
  if(status != 'A')
    STAT_LOOP_CHECK('A');
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
  buf[3] = NODE;
  memcpy(&buf[4], cmd, strlen(cmd));

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
  if(_debug)
    utils::hexdump(temp, buf, length + 2);
    
  int writebyte = m_cm->onWrite(buf, length + 2);
  if(writebyte < length + 2)
    throw EXCEPTION_COMMAND;
  
  //response
  return response(timeout);
  
}

byte* FBProtocol::processCommand(const char* cmd, const byte* data, int data_sz, int timeout/*ms*/)
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
  cout << "processCommand:" <<cmd<< endl;
  char temp[20];
  sprintf(temp, "SEND %s", cmd); 
  if(_debug) utils::hexdump(temp, buf, length + 2);
    
  int writebyte = m_cm->onWrite(buf, length + 2);
  if(writebyte < length + 2)
    throw EXCEPTION_COMMAND;
  
  //response
  return response(timeout);
    
}

byte* FBProtocol::processCommand(const byte* chunk, int chunk_sz, int timeout/*ms*/)
{
  int writebyte = m_cm->onWrite(chunk, chunk_sz);
  if(writebyte < chunk_sz)
    throw EXCEPTION_COMMAND;
  
  //response
  return response(timeout);
    
}

byte* FBProtocol::response(int timeout)
{
  byte _xor = 0xff;
  byte sum = 0;
  byte tempBuf[10];
  int readbyte = m_cm->onRead(tempBuf, 10, timeout);
  while(readbyte < 3){
    int t = m_cm->onRead(tempBuf + readbyte, 3 - readbyte, timeout);
    readbyte += t;
  }
  
  if(tempBuf[0] != SYNC){
    utils::hexdump("RECEIVE", tempBuf, readbyte);
    throw EXCEPTION_NOT_ACK;
  }
  printf("readbyte %d\n", readbyte);
  //dump("RECEIVE", receiveBuf, readbyte);
  short length = TOSHORT(tempBuf+1) + 2;
  byte* receiveBuf = new byte[length];
  memcpy(receiveBuf, tempBuf, readbyte);
  
  //cout << "sendCommandNoData: readbyte: " << readbyte << ", length: " << length << endl;
  try{
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
  }
  catch(FBProtocolCommMethod::Exception e){
    delete receiveBuf;
    throw EXCEPTION_COMMMETHOD;
  }
  if(_debug) utils::hexdump("RECEIVE", receiveBuf, length);
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

  if(receiveBuf[STATUS] == 'B'){
    delete receiveBuf;
    throw EXCEPTION_ERROR;
  }
  return receiveBuf;
}
