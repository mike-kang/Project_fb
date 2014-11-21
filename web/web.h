#ifndef _WEB_HEADER
#define _WEB_HEADER

namespace web {
enum Except{
  EXCEPTION_PARSING_URL,
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

typedef  void (*CCBFunc)(void *client_data, int status, void* ret);

};

#endif //_WEB_HEADER

