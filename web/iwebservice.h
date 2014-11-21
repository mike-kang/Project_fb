#ifndef _IWEBSERVICE_HEADER
#define _IWEBSERVICE_HEADER

#include "web.h"

namespace web {

class IWebService {
public:
//request
  virtual bool request_CheckNetwork(int timelimit, CCBFunc cbfunc, void* client) = 0;
  virtual void request_EmployeeInfoAll(const char *startTime, int timelimit, CCBFunc cbfunc, void* client, const char* outFilename) = 0;
  virtual char* request_EmployeeInfo(const char* serialnum, int timelimit, CCBFunc cbfunc, void* client) = 0;
  virtual char* request_ServerTime(int timelimit, CCBFunc cbfunc, void* client) = 0;
  virtual bool request_UploadTimeSheet(const char* sTime, const char* pinno, int timelimit, CCBFunc cbfunc, void* client, const char* outDirectory) = 0;
  virtual bool request_SendFile(const char *filename, int timelimit, CCBFunc cbfunc, void* client) = 0;



  bool request_CheckNetwork(int timelimit)
  {
    return request_CheckNetwork(timelimit, NULL, NULL);
  }
  bool request_CheckNetwork(CCBFunc cbfunc, void* client)
  {
    return request_CheckNetwork(0, cbfunc, client);
  }
  
  void request_EmployeeInfoAll(const char *startTime, int timelimit, 
  const char* outFilename)
  {
    request_EmployeeInfoAll(startTime, timelimit, NULL, NULL, outFilename);
  }
  void request_EmployeeInfoAll(const char *startTime, CCBFunc cbfunc, void* client, 
  const char* outFilename)
  {
    request_EmployeeInfoAll(startTime, 0, cbfunc, client, outFilename);
  }

  char* request_EmployeeInfo(const char* serialnum, int timelimit)
  {
    return request_EmployeeInfo(serialnum, timelimit, NULL, NULL);
  }
  char* request_EmployeeInfo(const char* serialnum, CCBFunc cbfunc, void* 
  client)
  {
    return request_EmployeeInfo(serialnum, 0, cbfunc, client);
  }

  char* request_ServerTime(int timelimit)
  {
    return request_ServerTime(timelimit, NULL, NULL);
  }
  char* request_ServerTime(CCBFunc cbfunc, void* client)
  {
    return request_ServerTime(0, cbfunc, client);
  }

  bool request_UploadTimeSheet(const char* sTime, const char* pinno, int timelimit, const char* outDirectory)
  {
    return request_UploadTimeSheet(sTime, pinno, timelimit, NULL, NULL, outDirectory);
  }
  bool request_UploadTimeSheet(const char* sTime, const char* pinno, CCBFunc cbfunc, void* client, const char* outDirectory)
  {
    return request_UploadTimeSheet(sTime, pinno, 0, NULL, NULL, outDirectory);
  }

  bool request_SendFile(const char *filename, int timelimit)
  {
    return request_SendFile(filename, timelimit, NULL, NULL);
  }
  bool request_SendFile(const char *filename, CCBFunc cbfunc, void* client)
  {
    return request_SendFile(filename, 0, cbfunc, client);
  }

/*
  virtual bool request_StatusUpdate(const char *sGateType, const char* sSiteCd, const char* sDvLoc, const char* sdvNo, const char* sIpAddress, const char* sMacAddress, int timelimit, CCBFunc cbfunc, void* 
  client) = 0;
  bool request_StatusUpdate(const char *sGateType, const char* sSiteCd, const char* sDvLoc, const char* sdvNo, const char* sIpAddress, const char* sMacAddress, int timelimit)
  {
    return request_StatusUpdate(sGateType, sSiteCd, sDvLoc, sdvNo, sIpAddress, sMacAddress, timelimit, NULL, NULL);
  }
  bool request_StatusUpdate(const char *sGateType, const char* sSiteCd, const char* sDvLoc, const char* sdvNo, const char* sIpAddress, const char* sMacAddress, CCBFunc cbfunc, void* 
  client)
  {
    return request_StatusUpdate(sGateType, sSiteCd, sDvLoc, sdvNo, sIpAddress, sMacAddress, 0, cbfunc, client);
  }
*/
};


};

#endif

