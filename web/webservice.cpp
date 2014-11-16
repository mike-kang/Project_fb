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
using namespace web;

#define LOG_TAG "WebService"

#define HTTP_OK 200
#define RCVHEADERBUFSIZE 1024

#define DUMP_CASE(x) case x: return #x;

const char* WebService::dump_error(Except e)
{
  switch(e){
    DUMP_CASE (EXCEPTION_CREATE_SOCKET)
    DUMP_CASE (EXCEPTION_CONNECT)
    DUMP_CASE (EXCEPTION_SEND_COMMAND)
    DUMP_CASE (EXCEPTION_POLL_FAIL)
    DUMP_CASE (EXCEPTION_POLL_TIMEOUT)
    DUMP_CASE (EXCEPTION_PARSING_FAIL)
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

/***********************************************************************************/
/*                                                                                 */
/*   parsing function                                                             */
/*                                                                                 */
/***********************************************************************************/
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

