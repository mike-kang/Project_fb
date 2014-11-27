#include "fbservice.h"
#include "tools/log.h"
#include <stdio.h>

using namespace std;
using namespace tools;

#define LOG_TAG "FBService"

FBService::FBService(const char* path, Serial::Baud baud, FBServiceNoti* fn):m_fn(fn)
{
  m_serial = new FBProtocolCMSerial(path, baud);
  m_protocol = new FBProtocol(m_serial);
  m_TimerRestart = new Timer(cbTimerFormat, this);

}

FBService::~FBService()
{
  delete m_serial;
  delete m_protocol;
  delete m_TimerRestart;
}

char* FBService::getVersion()
{
  try {
    char* ret =  m_protocol->vers();
    return ret;
  }
  catch(FBProtocol::Exception e){
    cout << "[vers]exception fail! " << e << endl;
    return NULL;
  }
}

bool FBService::start(bool check_device_id)
{
  bool ret = false;

  if(m_serial->open()){
    char* ver = getVersion();
    if(ver){
      LOGV("Version: %s\n", ver);
      if(!check_device_id)
        ret = true;
      else
        ret = deviceKey();
    }
  }

  m_bActive = ret;
  m_fn->onStart(ret);
  return ret;
}

void FBService::stop()
{
  m_bActive = false;
  m_serial->close();
}

bool FBService::deviceKey()
{
  try{
    char* device_id = m_protocol->didr();
    char key[8];
    if(m_fn->onNeedDeviceKey(device_id, key))
      if(m_protocol->didk(key))
        return true;
  }
  catch (FBProtocol::Exception e){
    cout << "error:" << e << endl;
  }
  return false;
}

bool FBService::getList(list<string>& li)
{
  try{
    if(!m_protocol)
      cout << "m_protocol null" << endl;
    m_protocol->user(li);
    for(list<string>::iterator itr = li.begin(); itr != li.end(); itr++)
      cout << *itr << endl;
    return true;
  }
  catch (FBProtocol::Exception e){
    cout << "error:" << e << endl;
  }
  return false;
}

//static
void FBService::cbTimerFormat(void* arg)
{
   FBService* my = (FBService*)arg;
   my->start();
}

bool FBService::format()
{
  bool ret = m_protocol->init();

  m_TimerRestart->start(5);
  return ret;
}


bool FBService::requestStartScan(int interval)
{
  m_scan_running = true;
  m_scan_interval = interval;
  m_thread_scan = new Thread<FBService>(&FBService::run_scan, this, "ScanThread");
}

int FBService::requestEndScan()
{
  m_scan_running = false;
  delete m_thread_scan;
  m_thread_scan = NULL;
}

void FBService::buzzer(bool val)
{
  byte buf[2] = {0x40, 0x00};
  if(val)
    buf[0] = 0x40;
  else
    buf[0] = 0xc0;
  
  m_protocol->optf(buf);
}

bool FBService::save(const char* filename)
{
  return m_protocol->save(filename);
}

bool FBService::deleteUsercode(unsigned short usercode)
{
  return m_protocol->dele(usercode);
}

void FBService::run_scan()
{
  int interval = m_scan_interval * 1000;
  bool bNeedInterval = true;
  char ret;
  char buf[20];
  bool bLong;
  while(m_scan_running){
    usleep(interval);
    ret = m_protocol->stat(buf, bLong);
    printf("result %c %d\n", ret, bLong);
    if(bLong && ret == 'A')
      printf("*******************Good*****************\n");
  }

}



