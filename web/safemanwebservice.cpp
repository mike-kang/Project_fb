#include <iostream>
#include <fstream>
#include "safemanwebservice.h"
#include <sys/poll.h> 
#include "tools/log.h"
#include "tools/base64.h"
#include "tools/utils.h"
#include <fcntl.h>
#include <errno.h>

using namespace tools;
using namespace web;

#define LOG_TAG "SafemanWebService"

#define HTTP_OK 200
#define RCVHEADERBUFSIZE 1024

#define DUMP_CASE(x) case x: return #x;


SafemanWebService::SafemanWebService(const char* ip, int port) : WebService(ip, port)
{
}

/***********************************************************************************/
/*                                                                                 */
/*   parsing functions                                                             */
/*                                                                                 */
/***********************************************************************************/
bool SafemanWebService::CodeDataSelect_WebApi::parsing()
{
  char headerbuf[RCVHEADERBUFSIZE];
  char* startContent;
  int contentLength;
  int readByteContent;

  if(!parsingHeader(headerbuf, &startContent, &contentLength, &readByteContent))
    return false;

  //contents
  char* buf = new char[contentLength+1];
  
  memcpy(buf, startContent, readByteContent);
  int nleaved = contentLength - readByteContent;
  while(nleaved){
    int readlen = recv(m_sock, buf + readByteContent, nleaved, 0);
#ifdef DEBUG
    buf[readByteContent + readlen] = '\0';
    oOut << buf + readByteContent;
#endif
    nleaved -= readlen;
    readByteContent += readlen;
    LOGV("read:%d, readByteContent:%d, nleaved:%d\n", readlen, readByteContent, nleaved);
  }
        
  buf[contentLength] = '\0';
  m_pRet = buf;

  return true;
}

bool SafemanWebService::RfidInfoSelectAll_WebApi::parsing()
{
  char headerbuf[RCVHEADERBUFSIZE];
  char* startContent;
  int contentLength;
  int readByteContent;
  ofstream oRet(m_filename);
  
  if(!parsingHeader(headerbuf, &startContent, &contentLength, &readByteContent))
    return false;

  headerbuf[startContent - headerbuf + readByteContent] = '\0';
  oRet << startContent;
  
  //contents
  char* buf = new char[contentLength+1];
  
  memcpy(buf, startContent, readByteContent);
  int nleaved = contentLength - readByteContent;
  while(nleaved){
    int readlen = recv(m_sock, buf + readByteContent, nleaved, 0);
    buf[readByteContent + readlen] = '\0';
    oRet << (buf + readByteContent);
    nleaved -= readlen;
    readByteContent += readlen;
    //LOGV("read:%d, readByteContent:%d, nleaved:%d\n", readlen, readByteContent, nleaved);
  }

  delete buf;
  oRet.close();
  
  return true;
}

bool SafemanWebService::RfidInfoSelect_WebApi::parsing()
{
  char headerbuf[RCVHEADERBUFSIZE];
  char* startContent;
  int contentLength;
  int readByteContent;
  
  if(!parsingHeader(headerbuf, &startContent, &contentLength, &readByteContent))
    return false;

  //contents
  char* buf = new char[contentLength+1];
  
  memcpy(buf, startContent, readByteContent);
  int nleaved = contentLength - readByteContent;
  while(nleaved){
    int readlen = recv(m_sock, buf + readByteContent, nleaved, 0);
#ifdef DEBUG
    buf[readByteContent + readlen] = '\0';
    oOut << buf + readByteContent;
    cout << buf + readByteContent;
#endif
    nleaved -= readlen;
    readByteContent += readlen;
    LOGV("read:%d, readByteContent:%d, nleaved:%d\n", readlen, readByteContent, nleaved);
  }
        
  buf[contentLength] = '\0';
  m_pRet = buf;

  return true;
}

bool SafemanWebService::ServerTimeGet_WebApi::parsing()
{
  char headerbuf[RCVHEADERBUFSIZE];
  char* startContent;
  int contentLength;
  int readByteContent;

  if(!parsingHeader(headerbuf, &startContent, &contentLength, &readByteContent))
    return false;

  //contents
  char* p;
  p = strstr(startContent, "\n");
  p = strstr(p, "tempuri");
  p = strstr(p, ">");
  
  char* start = p + 1;
  
  p = strstr(start, "<");
  *p = '\0';
  int len = strlen(start);
  m_pRet = new char[len+1];
  strcpy((char*)m_pRet, start);

  return true;
}


/***********************************************************************************/
/*                                                                                 */
/*   request functions                                                             */
/*                                                                                 */
/***********************************************************************************/
//#define SOAP_2_CODEDATASELECT //not work
#define SOAP_HEADER_SZ 112 //except ip & length

#define THROW_EXCEPTION(status)   switch(status){                  \
                                    case RET_CREATE_SOCKET_FAIL:    \
                                      throw EXCEPTION_CREATE_SOCKET;  \
                                    case RET_CONNECT_FAIL:          \
                                      throw EXCEPTION_CONNECT;      \
                                    case RET_SEND_CMD_FAIL:         \
                                      throw EXCEPTION_SEND_COMMAND; \
                                    case RET_POLL_FAIL:             \
                                      throw EXCEPTION_POLL_FAIL;    \
                                    case RET_POLL_TIMEOUT:          \
                                      throw EXCEPTION_POLL_TIMEOUT; \
                                    case RET_PARSING_FAIL:          \
                                      throw EXCEPTION_PARSING_FAIL; \
                                  }
/*
char* SafemanWebService::request_CodeDataSelect(const char *sMemcoCd, const char* sSiteCd, const char* sDvLoc, int timelimit, CCBFunc cbfunc, void* client)
{
  char* ret = NULL;
  char *cmd;
  int cmd_offset = 0;
#ifdef SOAP_2_CODEDATASELECT
  LOGV("request_CodeDataSelect SOAP 2\n");
  cmd = new char[1024];
  sprintf(cmd+400, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">\r\n"
    "  <soap12:Body>\r\n"
    "    <CodeDataSelect xmlns=\"http://tempuri.org/\">\r\n"
    "      <sMemcoCd>%s</sMemcoCd>\r\n"
    "      <sSiteCd>%s</sSiteCd>\r\n"
    "      <sType>T</sType>\r\n"
    "      <sGroupCd>0007'+AND+CODE='%s</sGroupCd>\r\n"
    "    </CodeDataSelect>\r\n"
    "  </soap12:Body>\r\n"
    "</soap12:Envelope>",  sMemcoCd, sSiteCd, sDvLoc);
  int len1 = strlen(cmd + 400);

  int headerlength = SOAP_HEADER_SZ + strlen(m_serverIP) + 3; // strlen(itoa(len1,10)) = 3
  cmd_offset = 400 - headerlength;
 

  sprintf(cmd + cmd_offset, "POST /SafemanWebService/ItlogService.asmx HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Content-Type: text/xml; charset=utf-8\r\n"
    "Content-Length: %d\r\n\r\n", m_serverIP, len1);

  //int len2 = strlen(cmd + cmd_offset);
  cmd[400] = '<';

#else
  LOGV("request_CodeDataSelect\n");
  cmd = new char[300];
  sprintf(cmd,"GET /SafemanWebService/ItlogService.asmx/CodeDataSelect?sMemcoCd=%s&sSiteCd=%s&sType=T&sGroupCd=0007'+AND+CODE='%s HTTP/1.1\r\nHost: %s\r\n\r\n"
    , sMemcoCd, sSiteCd, sDvLoc, m_serverIP);
#endif

  //LOGV("cmd:%s\n", cmd+cmd_offset);
  CodeDataSelect_WebApi* wa;

  if(cbfunc){
    wa = new CodeDataSelect_WebApi(this, cmd, cmd_offset, cbfunc, client);
    wa->processCmd();
  }
  else{
    wa = new CodeDataSelect_WebApi(this, cmd, cmd_offset, timelimit);
  
    int status = wa->processCmd();
    if(status != RET_SUCCESS){
      delete wa;
      THROW_EXCEPTION(status);
    }

    ret = (char*)wa->m_pRet;
    delete wa;
  }
  return ret;
}
*/
bool SafemanWebService::request_CheckNetwork(int timelimit, CCBFunc cbfunc, void* client)
{
  bool ret = false;
  LOGV("request_CheckNetwork\n");
  char *cmd = new char[100];
  sprintf(cmd,"GET /SafemanWebService/ItlogService.asmx/GetNetInfo? HTTP/1.1\r\nHost: %s\r\n\r\n", m_serverIP);

  GetNetInfo_WebApi* wa;

  if(cbfunc){
    wa = new GetNetInfo_WebApi(this, cmd, 0, cbfunc, client);
    wa->processCmd();
  }
  else{
    wa = new GetNetInfo_WebApi(this, cmd, 0, timelimit);
  
    int status = wa->processCmd();
    if(status != RET_SUCCESS){
      delete wa;
      THROW_EXCEPTION(status);
    }
    ret = wa->m_ret;
    printf("delete webapi\n");
    delete wa;
  }

  
  return ret;
}

void SafemanWebService::request_EmployeeInfoAll(const char *sMemcoCd, const char* sSiteCd, int timelimit, CCBFunc cbfunc, void* client, const char* outFilename)
{
  LOGV("request_EmployeeInfoAll +++\n");
  char *cmd = new char[300];
  sprintf(cmd,"GET /SafemanWebService/ItlogService.asmx/RfidInfoSelect?sMemcoCd=%s&sSiteCd=%s&sUtype=&sMode=A&sSearchValue= HTTP/1.1\r\nHost: %s\r\n\r\n"
    , sMemcoCd, sSiteCd, m_serverIP);
  
  RfidInfoSelectAll_WebApi* wa;

  if(cbfunc){
    wa = new RfidInfoSelectAll_WebApi(this, cmd, 0, cbfunc, client, outFilename);
    wa->processCmd();
  }
  else{
    wa = new RfidInfoSelectAll_WebApi(this, cmd, 0, timelimit, outFilename);
  
    int status = wa->processCmd();
    if(status != RET_SUCCESS){
      delete wa;
      THROW_EXCEPTION(status);
    }
    delete wa;
  }
  LOGV("request_EmployeeInfoAll ---\n");
  return;
}

char* SafemanWebService::request_EmployeeInfo(const char *sMemcoCd, const char* sSiteCd, const char* serialnum, int timelimit, CCBFunc cbfunc, void* client)
{
  char* ret = NULL;
  LOGV("request_EmployeeInfo\n");
  char *cmd = new char[400]; 
  char* cmd_content = cmd + 200; 
  sprintf(cmd_content,"sMemcoCd=%s&sSiteCd=%s&sUtype=R&sMode=&sSearchValue=RFID_CAR='%s", sMemcoCd, sSiteCd, serialnum);
  int contentlen = strlen(cmd_content);

  int headerlength = 137 + strlen(m_serverIP) + strlen(utils::itoa(contentlen,10));
  int cmd_offset = 200 - headerlength;
  sprintf(cmd + cmd_offset,"POST /SafemanWebService/ItlogService.asmx/RfidInfoSelect HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n"
    , m_serverIP, contentlen);
  //LOGV("cmd_offset:%d, header length:%d\n", cmd_offset, strlen(cmd + cmd_offset));
  
  cmd[200] = 's';
  //printf("\ncmd:%s\n\n", cmd + cmd_offset);
  RfidInfoSelect_WebApi* wa;

  if(cbfunc){
    wa = new RfidInfoSelect_WebApi(this, cmd, cmd_offset, cbfunc, client);
    wa->processCmd();
  }
  else{
    wa = new RfidInfoSelect_WebApi(this, cmd, cmd_offset, timelimit);
  
    int status = wa->processCmd();
    if(status != RET_SUCCESS){
      delete wa;
      THROW_EXCEPTION(status);
    }
    
    ret = (char*)wa->m_pRet;
    delete wa;
  }
  LOGV("request_EmployeeInfo ---\n");
  return ret;
}

char* SafemanWebService::request_ServerTime(int timelimit, CCBFunc cbfunc, void* client)
{
  char* ret = NULL;
  LOGV("request_ServerTime\n");
  char *cmd = new char[500];
  //sprintf(cmd,"GET /SafeIDService.asmx/getServerTime? HTTP/1.1\r\nHost: %s\r\n\r\n", m_serverIP);
  sprintf(cmd,"GET /SafeIDService.asmx/getServerTime? HTTP/1.1\r\nHost: %s\r\n\r\n", "dev.safeman.co.kr");
  ServerTimeGet_WebApi* wa;

  if(cbfunc){
    wa = new ServerTimeGet_WebApi(this, cmd, 0, cbfunc, client);
    wa->processCmd();
  }
  else{
    wa = new ServerTimeGet_WebApi(this, cmd, 0, timelimit);
  
    int status = wa->processCmd();
    if(status != RET_SUCCESS){
      delete wa;
      THROW_EXCEPTION(status);
    }
    
    ret = (char*)wa->m_pRet;
    delete wa;
  }
  return ret;
}

bool SafemanWebService::request_UploadTimeSheet(const char *sMemcoCd, const char* sSiteCd, const char* sLabNo, char cInOut, const char* sGateNo, const char* sGateLoc, char cUtype, const char* sInTime, char* imageBuf, int imageSz, int timelimit, CCBFunc cbfunc, void* 
  client, const char* outDirectory)
{
  bool ret;
  LOGV("request_UploadTimeSheet\n");
  int encoded_buf_sz = 0;
  if(imageBuf)
    encoded_buf_sz = base64::base64e2_get_needbufSize(imageSz);
  char *cmd = new char[400 + encoded_buf_sz]; 
  char* cmd_content = cmd + 200; 
  sprintf(cmd_content,"sMemcoCd=%s&sSiteCd=%s&sLabNo=%s&sInOut=%c&sGateNo=%s&sGateLoc=%s&sUtype=%c&sAttendGb=&sEventfunctionkey=&sInTime=%s&sPhotoImage="
    , sMemcoCd, sSiteCd, sLabNo, cInOut, sGateNo, sGateLoc, cUtype, sInTime);
  int cmd_content_prefix = strlen(cmd_content);
  
  int base64_encoded_len = 0;
  if(imageBuf)
    base64_encoded_len = base64::base64e2_http(imageBuf, imageSz, cmd_content + cmd_content_prefix);
  int contentlen = cmd_content_prefix + base64_encoded_len;
  //LOGV("base64_encoded_len: %d\n", base64_encoded_len);

  int headerlength = 144 + strlen(m_serverIP) + strlen(utils::itoa(contentlen,10));
  int cmd_offset = 200 - headerlength;
  sprintf(cmd + cmd_offset,"POST /SafemanWebService/ItlogService.asmx/TimeSheetInsertString HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n"
    , m_serverIP, contentlen);
  //LOGV("cmd_offset:%d, header length:%d\n", cmd_offset, strlen(cmd + cmd_offset));
  
  cmd[200] = 's';

  //ofstream oOut("request_UploadTimeSheet.txt");
  //oOut << (cmd+ cmd_offset) << endl;
  //oOut.close();
  
  TimeSheetInsertString_WebApi* wa;

  if(cbfunc){
    wa = new TimeSheetInsertString_WebApi(this, cmd, cmd_offset, cbfunc, client);
    wa->processCmd();
  }
  else{
    wa = new TimeSheetInsertString_WebApi(this, cmd, cmd_offset, timelimit);
  
    int status = wa->processCmd();
    if(status != RET_SUCCESS){
      char filename[255];
      sprintf(filename, "%s/%s", outDirectory, sInTime);
      LOGV("save file: %s\n", filename);
      ofstream oRet(filename);
      oRet << (cmd + cmd_offset);
      oRet.close();
      delete wa;
      THROW_EXCEPTION(status);
    }
    
    ret = wa->m_ret;
    delete wa;
  }
  return ret;
}

bool SafemanWebService::request_SendFile(const char *filename, int timelimit, CCBFunc cbfunc, void* client)
{
  bool ret = false;
  LOGV("request_SendFile\n");

  ifstream infile (filename);
  // get size of file
  infile.seekg (0,infile.end);
  long size = infile.tellg();
  infile.seekg (0);
  // allocate memory for file content
  cout << "size:" << size << endl;
  char* cmd = new char[size];
  // read content of infile
  infile.read (cmd, size);
  infile.close();

  WebApi* wa;

  if(cbfunc){
    wa = new WebApi(this, cmd, 0, cbfunc, client);
    wa->processCmd();
  }
  else{
    wa = new WebApi(this, cmd, 0, timelimit);
  
    int status = wa->processCmd();
    if(status != RET_SUCCESS){
      delete wa;
      THROW_EXCEPTION(status);
    }
    ret = wa->m_ret;
    printf("delete webapi %d\n", ret);
    delete wa;
  }
  
  return ret;
}


