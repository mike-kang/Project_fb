#include <iostream>
#include <fstream>
#include "dwwebservice.h"
#include <sys/poll.h> 
#include "tools/log.h"
#include "tools/base64.h"
#include "tools/utils.h"
#include <fcntl.h>
#include <errno.h>

using namespace tools;
using namespace web;

#define LOG_TAG "DWWebService"

#define HTTP_OK 200
#define RCVHEADERBUFSIZE 1024

#define DUMP_CASE(x) case x: return #x;


DWWebService::DWWebService(const char* url, const char *sMemcoCode, const char* sSiteCode, const char* 
  sEmbed, const char* gateCode, char inout) : WebService(url, sMemcoCode, sSiteCode, sEmbed, gateCode, inout)
{
}

/***********************************************************************************/
/*                                                                                 */
/*   parsing functions                                                             */
/*                                                                                 */
/***********************************************************************************/
void DWWebService::ServerTimeGet_WebApi::parsing()
{
  char headerbuf[RCVHEADERBUFSIZE];
  char* startContent;
  int contentLength;
  int readByteContent;

  parsingHeader(headerbuf, &startContent, &contentLength, &readByteContent);

  //contents
  char* p;
  p = strstr(startContent, "getServerTimeResult");
  p = strstr(p, ">");
  
  char* start = p + 1;
  
  p = strstr(start, "<");
  *p = '\0';
  
  int len = strlen(start);
  m_pRet = new char[len+1];
  strcpy((char*)m_pRet, start);
}

void DWWebService::RfidInfoSelect_WebApi::parsing()
{
  char headerbuf[RCVHEADERBUFSIZE];
  char* startContent;
  int contentLength;
  int readByteContent;
  
  parsingHeader(headerbuf, &startContent, &contentLength, &readByteContent);

  //contents
  char* buf = new char[contentLength+1];
  
  memcpy(buf, startContent, readByteContent);
  int nleaved = contentLength - readByteContent;
  while(nleaved){
    int readlen = recv(m_sock, buf + readByteContent, nleaved, 0);
    if(m_debug_file){
      buf[readByteContent + readlen] = '\0';
      oOut << buf + readByteContent;
    }
    nleaved -= readlen;
    readByteContent += readlen;
    LOGV("read:%d, readByteContent:%d, nleaved:%d\n", readlen, readByteContent, nleaved);
  }
        
  buf[contentLength] = '\0';
  m_pRet = buf;

  //printf("end parsing\n");
}

void DWWebService::TimeSheetInsertString_WebApi::parsing()
{
  char headerbuf[RCVHEADERBUFSIZE];
  char* startContent;
  int contentLength;
  int readByteContent;

  parsingHeader(headerbuf, &startContent, &contentLength, &readByteContent);

  //contents
  char* p;
  p = strstr(startContent, "ModifyEQPWorkTimeInfoResult");
  p = strstr(p, ">");
  
  char* start = p + 1;
  
  p = strstr(start, "<");
  *p = '\0';
  
  m_ret = atoi(start) > 0;

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

char* DWWebService::request_ServerTime(int timelimit, CCBFunc cbfunc, void* client)
{
  char* ret = NULL;
  char *cmd;
  int cmd_offset = 0;
  LOGV("request_ServerTime SOAP 2\n");
  cmd = new char[1024];
  strcpy(cmd+400, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">\r\n"
    "  <soap12:Body>\r\n"
    "    <getServerTime xmlns=\"http://tempuri.org/\" />\r\n"
    "  </soap12:Body>\r\n"
    "</soap12:Envelope>");
  int len1 = strlen(cmd + 400);

  int headerlength = 95 + strlen(m_service_name) + strlen(m_url_addr) + strlen(utils::itoa(len1,10));
  cmd_offset = 400 - headerlength;
 

  sprintf(cmd + cmd_offset, "POST %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Content-Type: application/soap+xml; charset=utf-8\r\n"
    "Content-Length: %d\r\n\r\n", m_service_name, m_url_addr, len1);

  //int len2 = strlen(cmd + cmd_offset);
  cmd[400] = '<';

  //LOGV("cmd:%s\n", cmd+cmd_offset);
  WebApi* wa;

  if(cbfunc){
    wa = new ServerTimeGet_WebApi(this, cmd, cmd_offset, cbfunc, client);
    wa->processCmd();
  }
  else{
    wa = new ServerTimeGet_WebApi(this, cmd, cmd_offset, timelimit);
  
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


char* DWWebService::request_EmployeeInfo(const char* serialnum, int timelimit, CCBFunc cbfunc, void* client)
{
  char* ret = NULL;
  char *cmd;
  int cmd_offset = 0;
  LOGV("request_EmployeeInfo\n");

  cmd = new char[1024];
  sprintf(cmd+400, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">\r\n"
    "  <soap12:Body>\r\n"
    "    <FindByEMPInfoPin xmlns=\"http://tempuri.org/\">\r\n"
    "      <SITE_CD>%s</SITE_CD>\r\n"
    "      <RFID>%s</RFID>\r\n"
    "    </FindByEMPInfoPin>\r\n"
    "  </soap12:Body>\r\n"
    "</soap12:Envelope>", m_sEmbed, serialnum);
  int len1 = strlen(cmd + 400);

  int headerlength = 95 + strlen(m_service_name) + strlen(m_url_addr) + strlen(utils::itoa(len1,10));
  cmd_offset = 400 - headerlength;
 

  sprintf(cmd + cmd_offset, "POST %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Content-Type: application/soap+xml; charset=utf-8\r\n"
    "Content-Length: %d\r\n\r\n", m_service_name, m_url_addr, len1);

  //int len2 = strlen(cmd + cmd_offset);
  cmd[400] = '<';

  //LOGV("cmd:%s\n", cmd+cmd_offset);
  WebApi* wa;

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


bool DWWebService::request_UploadTimeSheet(const char* sTime, const char* pinno, int timelimit, CCBFunc cbfunc, void* 
  client, const char* outDirectory)
{
  bool ret;
  char *cmd;
  int cmd_offset = 0;

  LOGV("request_UploadTimeSheet\n");
  printf("request_UploadTimeSheet\n");
  cmd = new char[1024];
  sprintf(cmd+400, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">\r\n"
    "  <soap12:Body>\r\n"
    "    <ModifyEQPWorkTimeInfo xmlns=\"http://tempuri.org/\">\r\n"
    "      <SITE_CD>%s</SITE_CD>\r\n"
    "      <EQP_NO>%s</EQP_NO>\r\n"
    "      <WORK_DT>%s</WORK_DT>\r\n"
    "      <RFID>%s</RFID>\r\n"
    "      <IO_TP>%c</IO_TP>\r\n"
    "      <FILE_NM></FILE_NM>\r\n"
    "      <bytePic></bytePic>\r\n"
    "    </ModifyEQPWorkTimeInfo>\r\n"
    "  </soap12:Body>\r\n"
    "</soap12:Envelope>", m_sEmbed, m_sGateCode, sTime, pinno, m_cInOut);
  int len1 = strlen(cmd + 400);

  int headerlength = 95 + strlen(m_service_name) + strlen(m_url_addr) + strlen(utils::itoa(len1,10));
  cmd_offset = 400 - headerlength;
 

  sprintf(cmd + cmd_offset, "POST %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Content-Type: application/soap+xml; charset=utf-8\r\n"
    "Content-Length: %d\r\n\r\n", m_service_name, m_url_addr, len1);

  //int len2 = strlen(cmd + cmd_offset);
  cmd[400] = '<';

  //ofstream oOut("request_UploadTimeSheet.txt");
  //oOut << (cmd+ cmd_offset) << endl;
  //oOut.close();
  
  WebApi* wa;

  if(cbfunc){
    wa = new TimeSheetInsertString_WebApi(this, cmd, cmd_offset, cbfunc, client);
    wa->processCmd();
  }
  else{
    wa = new TimeSheetInsertString_WebApi(this, cmd, cmd_offset, timelimit);
  
    int status = wa->processCmd();
    if(status != RET_SUCCESS || !wa->m_ret){
      char filename[255];
      sprintf(filename, "%s/%s", outDirectory, sTime);
      LOGV("save file: %s\n", filename);
      ofstream oRet(filename);
      oRet << (cmd + cmd_offset);
      oRet.close();
    }

    if(status != RET_SUCCESS){
      delete wa;
      THROW_EXCEPTION(status);
    }
    
    ret = wa->m_ret;
    delete wa;
  }
  return ret;
}




