#ifndef _MYDWWebService
#define _MYDWWebService

#include "tools/event.h"
#include "tools/queue.h"

#include "tools/thread.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fstream>
#include "dwwebservice.h"
#include "safemanwebservice.h"
//#define DEBUG
namespace web {

class MyDWWebService : public IWebService {
public:
  MyDWWebService(DWWebService* dw, SafemanWebService* sm):m_dw(dw), m_sm(sm){};

  ~MyDWWebService(){};
  //int start();

  //dump
  //static const char* dump_error(Except e);

//request
  char* request_ServerTime(int timelimit, CCBFunc cbfunc, void* client)
  {
    m_dw->request_ServerTime(timelimit, cbfunc, client);
  }
  bool request_CheckNetwork(int timelimit, CCBFunc cbfunc, void* client)
  {
    m_sm->request_CheckNetwork(timelimit, cbfunc, client);
  }
  void request_EmployeeInfoAll(const char *startTime, int timelimit, CCBFunc cbfunc, void* client, const char* outFilename)
  {
    m_sm->request_EmployeeInfoAllDW(startTime, timelimit, cbfunc, client, outFilename);
  }

  char* request_EmployeeInfo(const char* serialnum, int timelimit, CCBFunc cbfunc, void* client)
  {
    m_dw->request_EmployeeInfo(serialnum, timelimit, cbfunc, client);
  }

  bool request_UploadTimeSheet(const char* sTime, const char* pinno, int timelimit, CCBFunc cbfunc, void* 
  client, const char* outDirectory)
  {
    m_dw->request_UploadTimeSheet(sTime, pinno, timelimit, cbfunc, client, outDirectory);
  }
  bool request_SendFile(const char *filename, int timelimit, CCBFunc cbfunc, void* client)
  {
    m_sm->request_SendFile(filename, timelimit, cbfunc, client);
  }
private:
  DWWebService* m_dw;
  SafemanWebService* m_sm;
};


};

#endif

