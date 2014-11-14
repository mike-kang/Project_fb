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

class WebService : public IWebService {
public:
  static const int MAX_POLL_TIME = 3000;
  typedef  void (*CCBFunc)(void *client_data, int status, void* ret);

  class WebApi {
  friend class WebService;
  public:
    enum Exception{
      Exception_Parsing,


    };

    WebApi(WebService* ws, char* cmd, int cmd_offset, int t):m_ws(ws), m_cmd(cmd), m_cmd_offset(cmd_offset), 
    timelimit(t), m_pRet(&m_ret) { m_cbfunc=NULL;} //sync
    WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):m_ws(ws), m_cmd(cmd), m_cmd_offset(cmd_offset), m_cbfunc(cbfunc), m_client(client), m_pRet(&m_ret){timelimit=-1;};  //async
    virtual ~WebApi();
    virtual bool parsing();
    
    bool parsingHeader(char* buf, char **startContent, int* contentLength, int* readByteContent);
    int processCmd();
    int getStatus() const { return m_status; }
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
    void* m_pRet;
    bool m_ret;
#ifdef DEBUG
    ofstream oOut; //for debug
#endif    

  private:
    void run();

    WebService* m_ws;
    Thread<WebApi>* m_thread;
  };
  
  class CodeDataSelect_WebApi : public WebApi {
  //friend class WebService;
  public:
    virtual bool parsing();
    
    CodeDataSelect_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, t) //sync
    {
#ifdef DEBUG
      oOut.open("received_CodeDataSelect.txt");
#endif
    }
    CodeDataSelect_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
#ifdef DEBUG
      oOut.open("received_CodeDataSelect.txt");
#endif
    }
    virtual ~CodeDataSelect_WebApi()
    {
#ifdef DEBUG
      oOut.close();
#endif  
    }

  };
  class GetNetInfo_WebApi : public WebApi {
  //friend class WebService;
  public:
    GetNetInfo_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, t)  //sync
    {
#ifdef DEBUG
      oOut.open("received_GetNetInfo.txt");
#endif
    }
    GetNetInfo_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
#ifdef DEBUG
      oOut.open("received_GetNetInfo.txt");
#endif
    }
    virtual ~GetNetInfo_WebApi()
    {
#ifdef DEBUG
      oOut.close();
#endif  
    }


  };

  class RfidInfoSelectAll_WebApi : public WebApi {
  //friend class WebService;
  public:
    virtual bool parsing();
    
    RfidInfoSelectAll_WebApi(WebService* ws, char* cmd, int cmd_offset, int t, 
    const char* outFilename):WebApi(ws, cmd, cmd_offset, t)  //sync
    {
#ifdef DEBUG
      oOut.open("received_RfidInfoSelectAll.txt");
#endif
      strcpy(m_filename, outFilename);
    }
    RfidInfoSelectAll_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client, 
    const char* outFilename):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
#ifdef DEBUG
      oOut.open("received_RfidInfoSelectAll.txt");
#endif
      strcpy(m_filename, outFilename);
    }

    virtual ~RfidInfoSelectAll_WebApi()
    {
#ifdef DEBUG
      oOut.close();
#endif  
    }

  private:
    char m_filename[255];
  };

  class RfidInfoSelect_WebApi : public WebApi {
  //friend class WebService;
  public:
    virtual bool parsing();
    
    RfidInfoSelect_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, t)  //sync
    {
#ifdef DEBUG
      oOut.open("received_RfidInfoSelect.txt");
#endif
    }
    RfidInfoSelect_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
#ifdef DEBUG
      oOut.open("received_RfidInfoSelect.txt");
#endif
    }
    virtual ~RfidInfoSelect_WebApi()
    {
#ifdef DEBUG
      oOut.close();
#endif  
    }
  };

  class ServerTimeGet_WebApi : public WebApi {
  //friend class WebService;
  public:
    virtual bool parsing();
    
    ServerTimeGet_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, t)  //sync
    {
#ifdef DEBUG
      oOut.open("received_ServerTimeGet.txt");
#endif
    }
    ServerTimeGet_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
#ifdef DEBUG
      oOut.open("received_ServerTimeGet.txt");
#endif
    }
    virtual ~ServerTimeGet_WebApi()
    {
#ifdef DEBUG
      oOut.close();
#endif  
    }
  };
  
  class StatusUpdate_WebApi : public WebApi {
  //friend class WebService;
  public:
    StatusUpdate_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, t)  //sync
    {
#ifdef DEBUG
      oOut.open("received_StatusUpdate.txt");
#endif
    }
    StatusUpdate_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
#ifdef DEBUG
      oOut.open("received_StatusUpdate.txt");
#endif
    }
    virtual ~StatusUpdate_WebApi()
    {
#ifdef DEBUG
      oOut.close();
#endif  
    }
  };

  class TimeSheetInsertString_WebApi : public WebApi {
  //friend class WebService;
  public:
    TimeSheetInsertString_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, t)  //sync
    {
#ifdef DEBUG
      oOut.open("received_TimeSheetInsertString.txt");
#endif
    }
    TimeSheetInsertString_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
#ifdef DEBUG
      oOut.open("received_TimeSheetInsertString.txt");
#endif
    }
    virtual ~TimeSheetInsertString_WebApi()
    {
#ifdef DEBUG
      oOut.close();
#endif  
    }
  };
  

  WebService(const char* ip, int port);
  ~WebService(){};
  //int start();

  //dump
  static const char* dump_error(Except e);

//request
  bool request_CheckNetwork(int timelimit, CCBFunc cbfunc, void* client);
  void request_EmployeeInfoAll(const char *sMemcoCd, const char* sSiteCd, int timelimit, CCBFunc cbfunc, void* client, 
  const char* outFilename);

  char* request_EmployeeInfo(const char *sMemcoCd, const char* sSiteCd, const char* serialnum, int timelimit, CCBFunc cbfunc, void* 
  client);
  char* request_ServerTime(int timelimit, CCBFunc cbfunc, void* client);

  bool request_UploadTimeSheet(const char *sMemcoCd, const char* sSiteCd, const char* sLabNo, char cInOut, const char* sGateNo, const char* sGateLoc, char cUtype, const char* sInTime, char* imageBuf, int imageSz, int timelimit, CCBFunc cbfunc, void* 
  client, const char* outDirectory);
  bool request_SendFile(const char *filename, int timelimit, CCBFunc cbfunc, void* client);

private:
  Thread<WebService> *m_thread;
  TEvent<WebService>* m_event;
  tools::Queue<TEvent< WebService> > m_requestQ;
  Condition m_request_completed;
  Mutex mtx;
  char m_serverIP[16]; //XXX.XXX.XXX.XXX
  int m_port;
  struct sockaddr_in m_remote;
};




#endif

