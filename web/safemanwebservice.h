#ifndef _SafemanWebService
#define _SafemanWebService

#include "tools/event.h"
#include "tools/queue.h"

#include "tools/thread.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fstream>
#include "webservice.h"
//#define DEBUG
namespace web {

class SafemanWebService : public WebService {
public:

  class ServerTimeGet_WebApi : public WebApi {
  //friend class WebService;
  public:
    virtual void parsing();
    
    ServerTimeGet_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, t)  //sync
    {
    }
    ServerTimeGet_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
    }
    virtual ~ServerTimeGet_WebApi()
    {
    }
  };

  class RfidInfoSelectAll_WebApi : public WebApi {
  //friend class WebService;
  public:
    virtual void parsing();
    
    RfidInfoSelectAll_WebApi(WebService* ws, char* cmd, int cmd_offset, int t, 
    const char* outFilename):WebApi(ws, cmd, cmd_offset, t)  //sync
    {
      strcpy(m_filename, outFilename);
    }
    RfidInfoSelectAll_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client, 
    const char* outFilename):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
      strcpy(m_filename, outFilename);
    }

    virtual ~RfidInfoSelectAll_WebApi()
    {
    }

  private:
    char m_filename[255];
  };

  class RfidInfoSelect_WebApi : public WebApi {
  //friend class WebService;
  public:
    virtual void parsing();
    
    RfidInfoSelect_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, t, 
    "RfidInfoSelect_WebApi.txt"){}  //sync
    RfidInfoSelect_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, 
    client, "RfidInfoSelect_WebApi.txt"){} //async
    virtual ~RfidInfoSelect_WebApi(){}
  };
  
  class TimeSheetInsertString_WebApi : public WebApi {
  //friend class WebService;
  public:
    virtual void parsing();
    
    TimeSheetInsertString_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, 
    t){}  //sync
    TimeSheetInsertString_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, 
    client){}//async
    virtual ~TimeSheetInsertString_WebApi(){}
  };
  

  SafemanWebService(const char* url, const char *sMemcoCode, const char* sSiteCode, const char* 
  sEmbed, const char* gateCode, char inout);
  ~SafemanWebService(){};
  //int start();

  //dump
  //static const char* dump_error(Except e);

//request
  char* request_ServerTime(int timelimit, CCBFunc cbfunc, void* client);
  void request_EmployeeInfoAll(const char *startTime, int timelimit, CCBFunc cbfunc, void* client, const char* outFilename);
  void request_EmployeeInfoAllDW(const char *startTime, int timelimit, CCBFunc cbfunc, void* client, const char* 
  outFilename);

  char* request_EmployeeInfo(const char* serialnum, int timelimit, CCBFunc cbfunc, void* client);

  bool request_UploadTimeSheet(const char* sTime, const char* pinno, int timelimit, CCBFunc cbfunc, void* 
  client, const char* outDirectory);

private:
};


};

#endif

