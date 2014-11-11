#include "fbservice.h"
#include "tools/log.h"
#include <stdio.h>

using namespace std;
using namespace tools;

#define LOG_TAG "FBService"

FBService::FBService(const char* path, Serial::Baud baud)
{
  m_serial = new FBProtocolCMSerial(path, baud);
  m_protocol = new FBProtocol(m_serial);

}

bool FBService::start()
{
  m_serial->open();
  try{
    char* version = m_protocol->vers();
    cout << version << endl;
    //get user list
    m_protocol->user(listUserCode);
    for(list<string>::iterator itr = listUserCode.begin(); itr != listUserCode.end(); itr++)
      cout << *itr << endl;

    
    //m_protocol->auth();
    return true;
  }
  catch (FBProtocol::Exception e){
    cout << "error:" << e << endl;
  }
  return false;
}

bool FBService::requestStartScan(int interval)
{
  m_running = true;
  m_interval = interval;
  m_thread = new Thread<FBService>(&FBService::run, this, "ScanThread");
}

int FBService::requestEndScan()
{
  m_running = false;
  delete m_thread;
  m_thread = NULL;
}

void FBService::run()
{
  int interval = m_interval * 1000;
  bool bNeedInterval = true;
  char ret;
  char buf[20];
  bool bLong;
  while(m_running){
    usleep(interval);
    ret = m_protocol->stat(buf, bLong);
    //printf("result %c %d\n", ret, bLong);
    if(bLong && ret == 'A')
      break;
  }

}


