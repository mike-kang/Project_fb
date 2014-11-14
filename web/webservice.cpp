#include <iostream>
#include <fstream>
#include "webservice.h"
#include <sys/poll.h> 
#include "tools/log.h"
#include "tools/base64.h"
#include "tools/utils.h"
#include <fcntl.h>
#include <errno.h>

using namespace tools;

#define LOG_TAG "WebService"

#define HTTP_OK 200
#define RCVHEADERBUFSIZE 1024

#define DUMP_CASE(x) case x: return #x;

const char* WebService::dump_error(Except e)
{
  switch(e){
    DUMP_CASE (WebService::EXCEPTION_CREATE_SOCKET)
    DUMP_CASE (WebService::EXCEPTION_CONNECT)
    DUMP_CASE (WebService::EXCEPTION_SEND_COMMAND)
    DUMP_CASE (WebService::EXCEPTION_POLL_FAIL)
    DUMP_CASE (WebService::EXCEPTION_POLL_TIMEOUT)
    DUMP_CASE (WebService::EXCEPTION_PARSING_FAIL)
  }

}

WebService::WebService(const char* ip, int port)
{
  strcpy(m_serverIP, ip);
  inet_pton(AF_INET, m_serverIP, (void *)(&(m_remote.sin_addr.s_addr)));

  m_port = port;
  m_remote.sin_port = htons(m_port);

  m_remote.sin_family = AF_INET;
}

WebService::WebApi::~WebApi()
{
  LOGV("~WebApi+++\n");
  close(m_sock);
  delete m_thread;
  delete m_cmd;
  LOGV("~WebApi---\n");
}

bool WebService::WebApi::parsingHeader(char* buf, char **startContent, int* contentLength, int* readByteContent)
{
  int readlen = recv(m_sock, buf, RCVHEADERBUFSIZE - 1, 0);
  //header
  if(readlen <= 0){
    LOGE("Parsing fail: receive=<0\n"); 
    return false;
  }
      
  LOGV("received: %d\n", readlen);

#ifdef DEBUG
  buf[readlen] = '\0';
  oOut << buf;
#endif  
  char* p = strstr(buf, " ");  // buf is "HTTP/1.1 200 OK ..."
  char* e = strstr(p+1, " "); *e = '\0';
  int retVal = atoi(p+1);
  //LOGV("return val: %d\n", retVal);
  if(retVal != HTTP_OK){
    LOGE("not HTTP_OK %d\n", retVal); 
    return false;
  }
  p = strstr(e+1, "\nContent-Length:");
  //LOGV("p: %x\n", p);
  p+=17; //"\nContent-Length: " 
  e = strstr(p, "\r"); *e = '\0';
  *contentLength = atoi(p);
  LOGV("length: %d\n", *contentLength);
  *startContent = e + 4;  // \r\n\r\n
  *readByteContent = readlen - (*startContent - buf);
  LOGV("parsingHeader: contentLength=%d, readByteContent=%d\n", *contentLength, *readByteContent);
  return true;
}

bool WebService::WebApi::parsing()
{
  char headerbuf[RCVHEADERBUFSIZE];
  char* startContent;
  int contentLength;
  int readByteContent;

  if(!parsingHeader(headerbuf, &startContent, &contentLength, &readByteContent))
    return false;

  //contents
  char* p;
  //cout << "parsing:" << startContent << endl;
  p = strstr(startContent, "\n");
  p = strstr(p, "boolean");
  p = strstr(p, ">");
  
  m_ret = (*(p+1)=='t');
  return true;
}

/***********************************************************************************/
/*                                                                                 */
/*   parsing functions                                                             */
/*                                                                                 */
/***********************************************************************************/
bool WebService::CodeDataSelect_WebApi::parsing()
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

bool WebService::RfidInfoSelectAll_WebApi::parsing()
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

bool WebService::RfidInfoSelect_WebApi::parsing()
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

bool WebService::ServerTimeGet_WebApi::parsing()
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
  p = strstr(p, "dateTime");
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
char* WebService::request_CodeDataSelect(const char *sMemcoCd, const char* sSiteCd, const char* sDvLoc, int timelimit, CCBFunc cbfunc, void* client)
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
 

  sprintf(cmd + cmd_offset, "POST /WebService/ItlogService.asmx HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Content-Type: text/xml; charset=utf-8\r\n"
    "Content-Length: %d\r\n\r\n", m_serverIP, len1);

  //int len2 = strlen(cmd + cmd_offset);
  cmd[400] = '<';

#else
  LOGV("request_CodeDataSelect\n");
  cmd = new char[300];
  sprintf(cmd,"GET /WebService/ItlogService.asmx/CodeDataSelect?sMemcoCd=%s&sSiteCd=%s&sType=T&sGroupCd=0007'+AND+CODE='%s HTTP/1.1\r\nHost: %s\r\n\r\n"
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
bool WebService::request_CheckNetwork(int timelimit, CCBFunc cbfunc, void* client)
{
  bool ret = false;
  LOGV("request_CheckNetwork\n");
  char *cmd = new char[100];
  sprintf(cmd,"GET /WebService/ItlogService.asmx/GetNetInfo? HTTP/1.1\r\nHost: %s\r\n\r\n", m_serverIP);

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

void WebService::request_EmployeeInfoAll(const char *sMemcoCd, const char* sSiteCd, int timelimit, CCBFunc cbfunc, void* client, const char* outFilename)
{
  LOGV("request_EmployeeInfoAll +++\n");
  char *cmd = new char[300];
  sprintf(cmd,"GET /WebService/ItlogService.asmx/RfidInfoSelect?sMemcoCd=%s&sSiteCd=%s&sUtype=&sMode=A&sSearchValue= HTTP/1.1\r\nHost: %s\r\n\r\n"
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

char* WebService::request_EmployeeInfo(const char *sMemcoCd, const char* sSiteCd, const char* serialnum, int timelimit, CCBFunc cbfunc, void* client)
{
  char* ret = NULL;
  LOGV("request_EmployeeInfo\n");
  char *cmd = new char[400]; 
  char* cmd_content = cmd + 200; 
  sprintf(cmd_content,"sMemcoCd=%s&sSiteCd=%s&sUtype=R&sMode=&sSearchValue=RFID_CAR='%s", sMemcoCd, sSiteCd, serialnum);
  int contentlen = strlen(cmd_content);

  int headerlength = 137 + strlen(m_serverIP) + strlen(utils::itoa(contentlen,10));
  int cmd_offset = 200 - headerlength;
  sprintf(cmd + cmd_offset,"POST /WebService/ItlogService.asmx/RfidInfoSelect HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n"
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

char* WebService::request_ServerTime(int timelimit, CCBFunc cbfunc, void* client)
{
  char* ret = NULL;
  LOGV("request_ServerTime\n");
  char *cmd = new char[300];
  sprintf(cmd,"GET /SafeIDService.asmx/getServerTime? HTTP/1.1\r\nHost: %s\r\n\r\n", m_serverIP);
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

bool WebService::request_UploadTimeSheet(const char *sMemcoCd, const char* sSiteCd, const char* sLabNo, char cInOut, const char* sGateNo, const char* sGateLoc, char cUtype, const char* sInTime, char* imageBuf, int imageSz, int timelimit, CCBFunc cbfunc, void* 
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
  sprintf(cmd + cmd_offset,"POST /WebService/ItlogService.asmx/TimeSheetInsertString HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n"
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

bool WebService::request_SendFile(const char *filename, int timelimit, CCBFunc cbfunc, void* client)
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

void WebService::WebApi::run()
{
  int len = 0;
  int ret;
  int flags;
  int send_length = 0;
  
  if((m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
    LOGE("RET_CREATE_SOCKET_FAIL: %s\n", strerror(errno));
    m_status = RET_CREATE_SOCKET_FAIL;
    goto error;
  }

  LOGV("connect\n");
  flags = fcntl(m_sock, F_GETFL);
  if(fcntl(m_sock, F_SETFL, O_NONBLOCK | flags) < 0){
    LOGE("RET_FCNTL_FAIL\n");
    m_status = RET_FCNTL_FAIL;
    goto error;
  }
  (void)connect(m_sock, (struct sockaddr *)&m_ws->m_remote, sizeof(struct sockaddr));
  
  if(errno != EINPROGRESS){
    LOGE("RET_CONNECT_FAIL\n");
    m_status = RET_CONNECT_FAIL;
    goto error;
  }
  //poll
  struct pollfd fds;
  
  fds.fd = m_sock;
  fds.events = POLLOUT;
  ret = poll(&fds, 1, 1000);
  if(ret == -1){
    LOGE("RET_POLL_FAIL\n");
    m_status = RET_POLL_FAIL;
    goto error;
  }
  else if(ret == 0){
    LOGE("RET_POLL_TIMEOUT\n");
    m_status = RET_POLL_TIMEOUT;
    goto error;
  }
  if(fcntl(m_sock, F_SETFL, flags) < 0){
    LOGE("RET_FCNTL_FAIL\n");
    m_status = RET_FCNTL_FAIL;
    goto error;
  }

  LOGV("send command\n");
  send_length = strlen(m_cmd + m_cmd_offset);
  while(send_length){
    len = send(m_sock, m_cmd + m_cmd_offset + len, send_length, 0);
    if(len == -1){
      LOGE("RET_SEND_CMD_FAIL :%s\n", strerror(errno));
      m_status = RET_SEND_CMD_FAIL;
      goto error;
    }
    cout << "send size=" << len << endl;
    send_length -= len;
  }
  

  //poll
  //struct pollfd fds;
  
  fds.fd = m_sock;
  fds.events = POLLIN;
  ret = poll(&fds, 1, timelimit);
  if(ret == -1){
    LOGE("RET_POLL_FAIL\n");
    m_status = RET_POLL_FAIL;
    goto error;
  }
  else if(ret == 0){
    LOGE("RET_POLL_TIMEOUT\n");
    m_status = RET_POLL_TIMEOUT;
    goto error;
  }

  //receive & parsing
  if(!parsing()){
    m_status = RET_PARSING_FAIL;
    goto error;
  }
  m_status = RET_SUCCESS;

error:
  if(m_cbfunc){
    m_cbfunc(m_client, m_status, m_pRet); 
    printf("delete webapi\n");
    delete this;
  }
}



int WebService::WebApi::processCmd()
{
  m_thread = new Thread<WebApi>(&WebService::WebApi::run, this, "WebApi");
  
  if(!m_cbfunc)
    m_thread->join();
  else
    m_thread->detach();
  
  return m_status;
}

