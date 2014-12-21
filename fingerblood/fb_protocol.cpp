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
#define USER_COUNT 14

/* index */
#define FLAG 15
#define STATUS 5
#define BLOCKID 11

#define ENCRYPTION '2'
#define FINGER_COUNT_IN_FILE 3

#define TOSHORT(buf) (((buf)[0] << 8) + (buf)[1])
//beffer size
#define BUF_SZ_VERS 70 
#define DEVICE_ID_LENGTH 8 
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
  static char device_id[DEVICE_ID_LENGTH + 1];
  byte* receive_buf; 
  try{
    receive_buf = processCommand("DIDR", 1000);
    memcpy(device_id, &receive_buf[10], DEVICE_ID_LENGTH);
    device_id[DEVICE_ID_LENGTH] = '\0';
    delete receive_buf;
    return device_id;
  }
  catch(Exception e){
    LOGE("[didr]exception fail! %d\n", e);
    throw EXCEPTION_DIDR;
  }
}

void FBProtocol::didk(const char* key)
{
  bool ret = false;
  byte status;
  byte* receive_buf; 
  
  try {
    receive_buf = processCommand("DIDK", (const byte*)key, 8, 1000);
    status = receive_buf[STATUS];
    if(status != 'A'){
      if(!stat_loop_check('A', status)){
        LOGE("[didk]error loop_check(expect:%c result:%c\n", 'A', status);
      }
      else
        ret = true;
    }
    else
      ret = true;
  }
  catch(Exception e){
    LOGE("[didk]exception fail! %d\n", e);
  }

  if(!ret)
    throw EXCEPTION_DIDK;
}


//version 정보를 얻는다.
char* FBProtocol::vers() //with Exception
{
  static char version[BUF_SZ_VERS];
  byte* receive_buf; 
  try {
    receive_buf = processCommand("VERS", 1000);
    memcpy(version, &receive_buf[10], BUF_SZ_VERS);
    LOGV("VERS:%s\n", version);
    delete receive_buf;
    return version;
  }
  catch(Exception e){
    LOGE("[vers]exception fail! %d\n", e);
    throw EXCEPTION_VERS;
  }
}

bool FBProtocol::stop()
{
  byte status;
  byte* receive_buf; 

  try {
    receive_buf = processCommand("STOP", 10000);  //with Exception
    status = receive_buf[STATUS];
    delete receive_buf;
    if(status != 'A'){
      if(!stat_loop_check('A', status)){
        LOGE("[stop]error loop_check(expect:%c result:%c\n", 'A', status);
        return false;
      }
    }
    return true;
  }
  catch(Exception e){
    LOGE("[stop]exception fail! %d\n", e);
    return false;
  }
}


char FBProtocol::stat()
{
  char status;
  byte* receive_buf; 
  try{
    receive_buf = processCommand("STAT", 10000);  //with Exception
    status = receive_buf[STATUS];
    delete receive_buf;
    return status;  
  }
  catch(Exception e){
    throw EXCEPTION_STAT;
  }
}

#define S_INIT 0
#define S_READY 1
char FBProtocol::stat(char* data, bool& bLong)  //with Exception
{
  static int state = S_INIT;
  char status;
  byte* receive_buf; 
  try {
    receive_buf = processCommand("STAT", 10000);
    //printf("STAT 0x%x('%c')\n", receive_buf[STATUS], receive_buf[STATUS]);
    status = receive_buf[STATUS];
    short length = TOSHORT(receive_buf + 1);
    bLong = (length == 30);
    memcpy(data, &receive_buf[10], 16); 
    delete receive_buf;
    return status;  
  }
  catch(Exception e){
    if(e == EXCEPTION_RESP_ERROR)
      return 'B';
    LOGE("[statlong]exception fail! %d\n", e);
    throw EXCEPTION_STAT_LONG;
  }
}

void FBProtocol::init()
{
  char status;
  byte* receive_buf; 
  
  try {
    receive_buf = processCommand("INIT", -1);
    status = receive_buf[STATUS];
    if(status != '2')
      throw EXCEPTION_INIT;
  }
  catch(Exception e){
    LOGE("[init]exception fail! %d\n", e);
    throw EXCEPTION_INIT;
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
    userS();
    
    int blockId = 1;
    char flag;
    do {
      status = userD(blockId, li, flag);
      //printf("flag %c\n", flag);
      if(status == 'A' && flag == 'F')
        blockId++;
    }while(status != 'A' || flag != 'f');
    
    userE();
  }
  catch(Exception e){
    LOGE("[user]exception fail! %d\n", e);
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
    LOGE("[save]exception fail! %d\n", e);
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
    LOGE("[save]exception fail! %d\n", e);
    return false;
  }

  return true;
}

bool FBProtocol::dele(const char* usercode)
{
  byte status;
  byte* receive_buf; 
  char sUsercode[21];
  int i;
  if(strlen(usercode) == 4){
    strncpy(sUsercode, "000000000000", 12);
    strncpy(&sUsercode[12], usercode, 4);
  }
  else
    strncpy(sUsercode, usercode, 16);
  sUsercode[16] = '\0';
  strcat(sUsercode, "0000");

  try {
    receive_buf = processCommand("DELE", (byte*)sUsercode, 20, 1000);
    status = receive_buf[STATUS];
    delete receive_buf;

    if(status != '2')
      return false;
    if(!stat_loop_check('A', status)){
      LOGE("[dele]error loop_check(expect:%c result:%c\n", 'A', status);
      return false;
    }
    return true;  
  }
  catch(Exception e){
    LOGE("[dele]exception fail! %d\n", e);
  }
  return false;
}

bool FBProtocol::optf(byte* data)
{
  byte status;
  byte* receive_buf; 
  
  try {
    receive_buf = processCommand("OPTF", data, 2, 1000);
    status = receive_buf[STATUS];
    if(status != 'A'){
      if(!stat_loop_check('A', status)){
        LOGE("[optf]error loop_check(expect:%c result:%c\n", 'A', status);
        return false;
      }
    }
    return true;
  }
  catch(Exception e){
    LOGE("[optf]exception fail! %d\n", e);
  }
  return false;
}



void FBProtocol::userS()
{
  bool ret = false;
  byte status;
  byte* receive_buf; 
  byte data[2+ 20 + 20];
  byte* p = data;
  short count = USER_COUNT;
  data[0] = count >> 8;
  data[1] = count & 0xff;
  memset(p+2, 0x00, 20);
  memset(p+22, 0xff, 20);

  try{
    receive_buf = processCommand("USERS", data, 42, 3000);
    status = receive_buf[STATUS];
    delete receive_buf;
    if(status == '2'){
      if(!stat_loop_check('3', status)){
        LOGE("[users]error loop_check(expect:%c result:%c\n", '3', status);
      }
      else
        ret = true;
    }
  }
  catch(Exception e){
    LOGE("[users]exception fail! %d\n", e);
  }
  if(!ret)
    throw EXCEPTION_USERS;
}

byte FBProtocol::userD(unsigned int block_id, list<string>& li, char& flag)
{
  byte status;
  byte* receive_buf; 
  char data[4];
  sprintf(data, "%04d",block_id);

  try{
    receive_buf = processCommand("USERD", (byte*)data, 4, 9000);
    flag = receive_buf[FLAG];
    int length = TOSHORT(receive_buf+1);
    for(int i=16;i < length; i+=21){

      char* p = (char*)&receive_buf[i];
      *(p + 16) = '\0';
      li.push_back(p);
    }
    status = receive_buf[STATUS];
    delete receive_buf;
    return status;
  }
  catch(Exception e){
    LOGE("[userd]exception fail! %d\n", e);
    throw EXCEPTION_USERD;
  }
}

void FBProtocol::userE()
{
  bool ret = false;
  //static char receive_buf[BUF_SZ_USERE];
  byte* receive_buf; 
  byte status;

  try{
    receive_buf = processCommand("USERE", 9000);
    status = receive_buf[STATUS];
    delete receive_buf;
    if(status != 'A'){
      if(!stat_loop_check('A', status)){
        LOGE("[usere]error loop_check(expect:%c result:%c\n", 'A', status);
      }
      else
        ret = true;
    }
    else
      ret = true;
  }
  catch(Exception e){
    LOGE("[userE]exception fail! %d\n", e);
  }
  if(!ret)
    throw EXCEPTION_USERE;
}

void FBProtocol::saveS()
{
  bool ret = false;
  byte* receive_buf; 
  byte status;

  try{
    receive_buf = processCommand("SAVES", 5000);
    status = receive_buf[STATUS];
    delete receive_buf;
    if(status != '3'){
      if(!stat_loop_check('3', status)){
        LOGE("[saves]error loop_check(expect:%c result:%c\n", '3', status);
      }
      else
        ret = true;
    }
    else
      ret = true;
  }
  catch(Exception e){
    LOGE("[saves]exception fail! %d\n", e);
  }
  if(!ret)
    throw EXCEPTION_SAVES;
}


void FBProtocol::saveD(const char* filename)
{
  bool ret = false;
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

  try{
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
      
      if(_debug) 
        utils::hexdump("SEND SAVED", buf, length + 2);
      receive_buf = processCommand(buf, length + 2, 9000); //delete buf;
      
      //serial = TOSHORT(receive_buf+12);
      //errCode = TOSHORT(receive_buf+14);
      //restCmd = TOSHORT(receive_buf+16);
      //printf("serial %d, errCode %d, restCmd %d\n", serial, errCode, restCmd);

      status = receive_buf[STATUS];
      delete receive_buf;

      if(status == '3'){
        serial++;
        if(serial == FINGER_COUNT_IN_FILE){
          ret = true;
          break;
        }
      }
      else{
        if(!stat_loop_check('3', status)){
          LOGE("[saved]error loop_check(expect:%c result:%c\n", '3', status);
          break;
        }
        else{
          serial++;
          if(serial == FINGER_COUNT_IN_FILE){
            ret = true;
            break;
          }
        }
      }
    }while(1);

  }
  catch(Exception e){
    LOGE("[saved]exception fail! %d\n", e);
  }
  
  delete buf;
  infile.close();
  
  if(!ret)
    throw EXCEPTION_SAVED;
}

void FBProtocol::saveD(const byte* userdata, int len)
{
  bool ret = false;
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

  try{
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
      //usleep(200000);
      if(_debug) 
        utils::hexdump("SEND SAVED", buf, length + 2);
      receive_buf = processCommand(buf, length + 2, 9000); //delete buf;
      
      status = receive_buf[STATUS];
      delete receive_buf;

      if(status == '3'){
        serial++;
        if(serial == FINGER_COUNT_IN_FILE){
          ret = true;
          break;
        }
      }
      else{
        if(!stat_loop_check('3', status)){
          LOGE("[saved]error loop_check(expect:%c result:%c\n", '3', status);
          break;
        }
        else{
          serial++;
          if(serial == FINGER_COUNT_IN_FILE){
            ret = true;
            break;
          }
        }
      }
    }while(1);
  }
  catch(Exception e){
    LOGE("[saveD] exception %d\n", e);
  }
  delete buf;

  if(!ret)
    throw EXCEPTION_SAVED;
}

void FBProtocol::saveE()
{
  byte* receive_buf; 
  byte status;

  receive_buf = processCommand("SAVEE", 1000);
  status = receive_buf[STATUS];
  delete receive_buf;
  if(status != 'A'){
    if(!stat_loop_check('A', status)){
      LOGE("[savee]error loop_check(expect:%c result:%c\n", 'A', status);
      throw EXCEPTION_SAVEE;
    }
  }
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
  if(_debug) cout << "processCommand:" << cmd << endl;
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
  if(_debug) cout << "processCommand:" << cmd << endl;
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
    throw EXCEPTION_RESP_NOT_ACK;
  }
  //printf("readbyte %d\n", readbyte);
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
    throw EXCEPTION_RESP_COMMMETHOD;
  }
  if(_debug) utils::hexdump("RECEIVE", receiveBuf, length);
  if(_debug) printf("status 0x%x('%c')\n", receiveBuf[STATUS], receiveBuf[STATUS]);

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
    throw EXCEPTION_RESP_CHECKSUM;  
  }

  if(receiveBuf[STATUS] == 'B'){
    delete receiveBuf;
    throw EXCEPTION_RESP_ERROR;
  }
  return receiveBuf;
}

bool FBProtocol::stat_loop_check(char check_char, byte& ret_char, int limit_count)
{
  for(int i = 0; i < limit_count; i++){   
    usleep(100000);       
    ret_char = stat();      
    if(ret_char == check_char){
      return true;
    }
  }
  return false;
}

