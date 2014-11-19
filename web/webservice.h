#ifndef _WEBSERVICE_HEADER
#define _WEBSERVICE_HEADER

#include "tools/event.h"
#include "tools/queue.h"

#include "tools/thread.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fstream>
#include "iwebservice.h"
//#define DEBUG
namespace web {
class WebService : public IWebService {
public:

  class WebApi {
  //friend class WebService;
  public:
    enum Exception{
      EXCEPTION_PARSING,
      EXCEPTION_NOT_HTTP_OK
    };

    WebApi(WebService* ws, char* cmd, int cmd_offset, int t, const char* 
    debug_file=NULL):m_ws(ws), m_cmd(cmd), m_cmd_offset(cmd_offset), 
    timelimit(t), m_pRet(&m_ret), m_debug_file(debug_file) 
    {
      m_cbfunc=NULL;
      if(m_debug_file) oOut.open(m_debug_file);
    } //sync
    WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client, const char* 
    debug_file = NULL):m_ws(ws), m_cmd(cmd), m_cmd_offset(cmd_offset), m_cbfunc(cbfunc), m_client(client), m_pRet(&m_ret), m_debug_file(debug_file)
    {
      timelimit=-1;
      if(m_debug_file) oOut.open(m_debug_file);
    }  //async
    virtual ~WebApi();
    virtual void parsing();
    
    void parsingHeader(char* buf, char **startContent, int* contentLength, int* readByteContent);
    int processCmd();
    int getStatus() const { return m_status; }

    void* m_pRet;
    bool m_ret;

  protected:
    int m_status;
    int m_sock;
    int timelimit;
    CCBFunc m_cbfunc;
    void *m_client;
    Condition m_request_completed;
    Mutex mtx;
    char* m_cmd;
    int m_cmd_offset;
    const char* m_debug_file;
    ofstream oOut; //for debug

  private:
    void run();

    WebService* m_ws;
    Thread<WebApi>* m_thread;
  };

  class GetNetInfo_WebApi : public WebApi {
  //friend class WebService;
  public:
    GetNetInfo_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, t)  //sync
    {
    }
    GetNetInfo_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
    }
    virtual ~GetNetInfo_WebApi()
    {
    }


  };
  
  WebService(const char* url, const char *sMemcoCode, const char* sSiteCode, const char* 
  sEmbed, const char* gateCode, char inout);
  ~WebService(){};
  //int start();

  //dump
  static const char* dump_error(Except e);

  bool request_CheckNetwork(int timelimit, CCBFunc cbfunc, void* client);
  bool request_SendFile(const char *filename, int timelimit, CCBFunc cbfunc, void* client);

protected:
  char m_url_addr[255];
  char m_service_name[255];
  char m_serverIP[16]; //XXX.XXX.XXX.XXX
  int m_port;
  
  char m_sMemcoCd[11];
  char m_sSiteCd[11];
  char m_sEmbed[11];
  char m_sGateCode[5];
  char m_cInOut;
  
  struct sockaddr_in m_remote;
};


};

#endif

