#ifndef _IWEBSERVICE_HEADER
#define _IWEBSERVICE_HEADER

namespace web {
enum Except{
  EXCEPTION_NOT_SUPPORTED,
  EXCEPTION_CREATE_SOCKET,
  EXCEPTION_CONNECT,
  EXCEPTION_SEND_COMMAND,
  EXCEPTION_POLL_FAIL,
  EXCEPTION_POLL_TIMEOUT,
  EXCEPTION_PARSING_FAIL
};

enum Ret {
  RET_SUCCESS,
  RET_CREATE_SOCKET_FAIL,
  RET_CONNECT_FAIL,
  RET_SEND_CMD_FAIL,
  RET_FCNTL_FAIL,
  RET_POLL_FAIL,
  RET_POLL_TIMEOUT,
  RET_PARSING_FAIL
};

class IWebService {
public:
  static const int MAX_POLL_TIME = 3000;
  typedef  void (*CCBFunc)(void *client_data, int status, void* ret);


//request
  virtual bool request_CheckNetwork(int timelimit, CCBFunc cbfunc, void* client) = 0;
  bool request_CheckNetwork(int timelimit)
  {
    return request_CheckNetwork(timelimit, NULL, NULL);
  }
  bool request_CheckNetwork(CCBFunc cbfunc, void* client)
  {
    return request_CheckNetwork(0, cbfunc, client);
  }
  virtual void request_EmployeeInfoAll(const char *sMemcoCd, const char* sSiteCd, int timelimit, CCBFunc cbfunc, void* client, 
  const char* outFilename) = 0;
  void request_EmployeeInfoAll(const char *sMemcoCd, const char* sSiteCd, int timelimit, 
  const char* outFilename)
  {
    request_EmployeeInfoAll(sMemcoCd, sSiteCd, timelimit, NULL, NULL, outFilename);
  }
  void request_EmployeeInfoAll(const char *sMemcoCd, const char* sSiteCd, CCBFunc cbfunc, void* client, 
  const char* outFilename)
  {
    request_EmployeeInfoAll(sMemcoCd, sSiteCd, 0, cbfunc, client, outFilename);
  }

  virtual char* request_EmployeeInfo(const char *sMemcoCd, const char* sSiteCd, const char* serialnum, int timelimit, CCBFunc cbfunc, void* 
  client) = 0;
  char* request_EmployeeInfo(const char *sMemcoCd, const char* sSiteCd, const char* serialnum, int timelimit)
  {
    return request_EmployeeInfo(sMemcoCd, sSiteCd, serialnum, timelimit, NULL, NULL);
  }
  char* request_EmployeeInfo(const char *sMemcoCd, const char* sSiteCd, const char* serialnum, CCBFunc cbfunc, void* 
  client)
  {
    return request_EmployeeInfo(sMemcoCd, sSiteCd, serialnum, 0, cbfunc, client);
  }
  virtual char* request_ServerTime(int timelimit, CCBFunc cbfunc, void* client) = 0;
  char* request_ServerTime(int timelimit)
  {
    return request_ServerTime(timelimit, NULL, NULL);
  }
  char* request_ServerTime(CCBFunc cbfunc, void* client)
  {
    return request_ServerTime(0, cbfunc, client);
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
  virtual bool request_UploadTimeSheet(const char *sMemcoCd, const char* sSiteCd, const char* sLabNo, char cInOut, const char* sGateNo, const char* sGateLoc, char cUtype, const char* sInTime, char* imageBuf, int imageSz, int timelimit, CCBFunc cbfunc, void* 
  client, const char* outDirectory) = 0;
  bool request_UploadTimeSheet(const char *sMemcoCd, const char* sSiteCd, const char* sLabNo, char cInOut, const char* sGateNo, const char* sGateLoc, char cUtype, const char* sInTime, char* imageBuf, int imageSz, int timelimit, const char* outDirectory)
  {
    return request_UploadTimeSheet(sMemcoCd, sSiteCd, sLabNo, cInOut, sGateNo, sGateLoc, cUtype, sInTime, 
    imageBuf, imageSz, timelimit, NULL, NULL, outDirectory);
  }
  bool request_UploadTimeSheet(const char *sMemcoCd, const char* sSiteCd, const char* sLabNo, char cInOut, const char* sGateNo, const char* sGateLoc, char cUtype, const char* sInTime, char* imageBuf, int imageSz, CCBFunc cbfunc, void* 
  client, const char* outDirectory)
  {
    return request_UploadTimeSheet(sMemcoCd, sSiteCd, sLabNo, cInOut, sGateNo, sGateLoc, cUtype, sInTime, imageBuf, imageSz, 0, cbfunc, 
    client, outDirectory);
  }
  virtual bool request_SendFile(const char *filename, int timelimit, CCBFunc cbfunc, void* client) = 0;
  bool request_SendFile(const char *filename, int timelimit)
  {
    return request_SendFile(filename, timelimit, NULL, NULL);
  }
  bool request_SendFile(const char *filename, CCBFunc cbfunc, void* client)
  {
    return request_SendFile(filename, 0, cbfunc, client);
  }
};


};

#endif

