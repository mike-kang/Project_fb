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

bool FBService::start()
{
  m_serial->open();
  try{
    m_protocol->user(listUserCode);
    for(list<string>::iterator itr = listUserCode.begin(); itr != listUserCode.end(); itr++)
      cout << *itr << endl;
    return true;
  }
  catch (FBProtocol::Exception e){
    cout << "error:" << e << endl;
  }
  return false;
}

bool FBService::format()
{
  bool ret = m_protocol->init();
  m_thread_foramt = new Thread<FBService>(&FBService::run_format, this, "FormatThread");
  return ret;
}


bool FBService::requestStartScan(int interval)
{
  m_running = true;
  m_interval = interval;
  m_thread_scan = new Thread<FBService>(&FBService::run_scan, this, "ScanThread");
}

int FBService::requestEndScan()
{
  m_running = false;
  delete m_thread_scan;
  m_thread = NULL;
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
  int interval = m_interval * 1000;
  bool bNeedInterval = true;
  char ret;
  char buf[20];
  bool bLong;
  while(m_running){
    usleep(interval);
    ret = m_protocol->stat(buf, bLong);
    printf("result %c %d\n", ret, bLong);
    if(bLong && ret == 'A')
      printf("*******************Good*****************\n");
  }

}

void FBService::run_format()
{
  sleep(3);
  while(true){
    if(getVersion()){
      
      break;
    }
    sleep(1);
  }

}


