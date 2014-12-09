#include <iostream>
#include <fstream>
#include "webservice.h"
#include <sys/poll.h> 
#include "tools/log.h"
#include "tools/base64.h"
#include "tools/utils.h"
#include <fcntl.h>
#include <errno.h>
#include "tools/network.h"

using namespace tools;
using namespace web;

#define LOG_TAG "WebService"

#define HTTP_OK 200
#define RCVHEADERBUFSIZE 1024

#define DUMP_CASE(x) case x: return #x;

const char* WebService::dump_error(Except e)
{
  switch(e){
    DUMP_CASE (EXCEPTION_NOT_SUPPORTED)
    DUMP_CASE (EXCEPTION_CREATE_SOCKET)
    DUMP_CASE (EXCEPTION_CONNECT)
    DUMP_CASE (EXCEPTION_SEND_COMMAND)
    DUMP_CASE (EXCEPTION_POLL_FAIL)
    DUMP_CASE (EXCEPTION_POLL_TIMEOUT)
    DUMP_CASE (EXCEPTION_PARSING_FAIL)
  }

}

WebService::WebService(const char* url, const char *sMemcoCode, const char* sSiteCode, const char* 
  sEmbed, const char* gateCode, char inout) : m_port(80), m_cInOut(inout)
{
  strncpy(m_sMemcoCd, sMemcoCode, 10);
  m_sMemcoCd[10] = '\0';
  strncpy(m_sSiteCd, sSiteCode, 10);
  m_sSiteCd[10] = '\0';
  strncpy(m_sEmbed, sEmbed, 10);
  m_sEmbed[10] = '\0';
  strncpy(m_sGateCode, gateCode, 4);
  m_sGateCode[4] = '\0';

  char* start = strstr((char*)url, "//");
  if(!start) 
    throw EXCEPTION_PARSING_URL;

  start += 2;
  char *end = strstr((char*)start, "/");
  if(!end) 
    throw EXCEPTION_PARSING_URL;

  char *p;
  for(p = start+1; p<end; p++)
    if(*p == ':')
      break;
  
  if(p != end){
    m_port = atoi(p+1);
  }

  strncpy(m_url_addr, start, end - start);  //000.000.000.000 or aaa.sssssss.ss
  m_url_addr[end-start] = '\0';
  printf("m_url_addr %s\n", m_url_addr); 
  //ar ip[21];
  char* pIP;
  if(network::isIPv4(m_url_addr)){
    pIP = m_url_addr;
  }
  else{
    pIP = network::ResolveName(m_url_addr);
  }
  strcpy(m_serverIP, pIP);
  
  LOGV("Server IP: %s\n", pIP);
  printf("Server IP: %s\n", pIP);

  inet_pton(AF_INET, pIP, (void *)(&(m_remote.sin_addr.s_addr)));

  m_remote.sin_port = htons(m_port);

  m_remote.sin_family = AF_INET;

  strcpy(m_service_name, end);
  
}

WebService::WebApi::~WebApi()
{
  LOGV("~WebApi+++\n");
  close(m_sock);
  if(m_debug_file) oOut.close();
  delete m_thread;
  delete m_cmd;
  LOGV("~WebApi---\n");
}

void WebService::WebApi::parsingHeader(char* buf, char **startContent, int* contentLength, int* readByteContent)
{
  int readlen = recv(m_sock, buf, RCVHEADERBUFSIZE - 1, 0);
  //header
  if(readlen <= 0){
    LOGE("Parsing fail: receive=<0\n"); 
    throw EXCEPTION_PARSING;
  }
      
  LOGV("received: %d\n", readlen);

  if(m_debug_file){
    buf[readlen] = '\0';
    oOut << buf;
  }
  char* p = strstr(buf, " ");  // buf is "HTTP/1.1 200 OK ..."
  char* e = strstr(p+1, " "); *e = '\0';
  int retVal = atoi(p+1);
  //LOGV("return val: %d\n", retVal);
  if(retVal != HTTP_OK){
    LOGE("not HTTP_OK %d\n", retVal); 
    throw EXCEPTION_NOT_HTTP_OK;
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
}

/***********************************************************************************/
/*                                                                                 */
/*   parsing function                                                             */
/*                                                                                 */
/***********************************************************************************/
void WebService::WebApi::parsing()
{
  char headerbuf[RCVHEADERBUFSIZE];
  char* startContent;
  int contentLength;
  int readByteContent;

  parsingHeader(headerbuf, &startContent, &contentLength, &readByteContent);

  //contents
  char* p;
  //cout << "parsing:" << startContent << endl;
  p = strstr(startContent, "\n");
  p = strstr(p, "boolean");
  p = strstr(p, ">");
  
  m_ret = (*(p+1)=='t');
}

void WebService::WebApiInt::parsing()
{
  char headerbuf[RCVHEADERBUFSIZE];
  char* startContent;
  int contentLength;
  int readByteContent;

  parsingHeader(headerbuf, &startContent, &contentLength, &readByteContent);

  //contents
  char* p;
  cout << "parsing:" << startContent << endl;
  p = strstr(startContent, "\n");
  p = strstr(p, "int");
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
  try{
    parsing();
    m_status = RET_SUCCESS;
  }
  catch(WebApi::Exception e){
    m_status = RET_PARSING_FAIL;
    goto error;
  }

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


bool WebService::request_CheckNetwork(int timelimit, CCBFunc cbfunc, void* client)
{
  bool ret = false;
  LOGV("request_CheckNetwork\n");
  char *cmd = new char[100];
  sprintf(cmd,"GET %s/NetworkOn? HTTP/1.1\r\nHost: %s\r\n\r\n", m_service_name, m_url_addr);

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
    printf("delete webapi\n");
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
    wa = new WebApiInt(this, cmd, 0, cbfunc, client);
    wa->processCmd();
  }
  else{
    wa = new WebApiInt(this, cmd, 0, timelimit);
  
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

