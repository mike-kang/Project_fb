#ifndef _DWWebService
#define _DWWebService

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

class DWWebService : public WebService {
public:
  class ServerTimeGet_WebApi : public WebApi {
  //friend class WebService;
  public:
    virtual void parsing();
    
    ServerTimeGet_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, 
    t){}  //sync
    ServerTimeGet_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, 
    client){} //async
    virtual ~ServerTimeGet_WebApi(){}
  };
  

  class RfidInfoSelect_WebApi : public WebApi {
  //friend class WebService;
  public:
    virtual void parsing();
    
    RfidInfoSelect_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApi(ws, cmd, cmd_offset, t)  //sync
    {
    }
    RfidInfoSelect_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApi(ws, cmd, cmd_offset, cbfunc, client) //async
    {
    }
    virtual ~RfidInfoSelect_WebApi()
    {
    }
  };

  class TimeSheetInsertString_WebApi : public WebApiInt {
  //friend class WebService;
  public:
    //virtual void parsing();

    TimeSheetInsertString_WebApi(WebService* ws, char* cmd, int cmd_offset, int t):WebApiInt(ws, cmd, cmd_offset, t)  //sync
    {
    }
    TimeSheetInsertString_WebApi(WebService* ws, char* cmd, int cmd_offset, CCBFunc cbfunc, void* client):WebApiInt(ws, cmd, cmd_offset, cbfunc, client) //async
    {
    }
    virtual ~TimeSheetInsertString_WebApi()
    {
    }
  };
  

  DWWebService(const char* url, const char *sMemcoCode, const char* sSiteCode, const char* 
  sEmbed, const char* gateCode, char inout);
  ~DWWebService(){};
  //int start();

  //dump
  //static const char* dump_error(Except e);

//request
  char* request_ServerTime(int timelimit, CCBFunc cbfunc, void* client);
  bool request_CheckNetwork(int timelimit, CCBFunc cbfunc, void* client)
  {
    throw EXCEPTION_NOT_SUPPORTED;
  }
  void request_EmployeeInfoAll(const char *startTime, int timelimit, CCBFunc cbfunc, void* client, const char* outFilename)
  {
    throw EXCEPTION_NOT_SUPPORTED;
  }

  char* request_EmployeeInfo(const char* serialnum, int timelimit, CCBFunc cbfunc, void* client);

  bool request_UploadTimeSheet(const char* sTime, const char* pinno, int timelimit, CCBFunc cbfunc, void* 
  client, const char* outDirectory);

private:
};


};

#endif

