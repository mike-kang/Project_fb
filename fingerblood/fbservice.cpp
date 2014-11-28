#include "fbservice.h"
#include "tools/log.h"
#include <stdio.h>

using namespace std;
using namespace tools;

#define LOG_TAG "FBService"

FBService::FBService(const char* path, Serial::Baud baud, FBServiceNoti* fn, bool bCheckUserCode4):m_fn(fn), m_bCheckUserCode4(bCheckUserCode4)
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
  m_bActive = false;
  
  if(!m_serial->open())
    return false;
  
  char* ver = getVersion();
  if(!ver)
    return false;
  LOGV("Version: %s\n", ver);
  
  if(check_device_id && !checkdeviceID()){
    return false;
  }

  m_bActive = true;
  //m_fn->onStart(true);
  return true;
}

void FBService::stop()
{
  m_bActive = false;
  m_serial->close();
}

void FBService::sync()
{
  m_sync_running = true;
  m_thread_sync = new Thread<FBService>(&FBService::run_sync, this, "SyncThread");
}


bool FBService::checkdeviceID()
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

int FBService::requestStopScan()
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

bool FBService::save(const byte* buf, int length)
{
  return m_protocol->save(buf, length);
}

bool FBService::deleteUsercode(const char* usercode)
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

void FBService::run_sync()
{
  LOGV("run_sync\n");
  map<const char*, unsigned char*> device_arr_16, device_arr_4;
  m_fn->onNeedUserCodeList(device_arr_16, device_arr_4);

  list<string> module_list; //only 16
  if(!getList(module_list))
    m_fn->onSync(false);

  map<const char*, unsigned char*>::iterator d = device_arr_16.begin();

  int count = module_list.size();
  LOGV("module_list.size %d\n", count);

  if(count == 0){
    for(; d != device_arr_16.end(); d++){
      save(d->second, 864);
    }
  }
  else{
    list<string>::iterator m = module_list.begin();
    int compare;
    for(; d != device_arr_16.end(); d++){
      const char* device_usercode = d->first;
      compare = strcmp(device_usercode, m->c_str());
      if(!compare){
        m++;
      }
      else if(compare > 0){
        deleteUsercode(device_usercode);
        m++;
      }
      else
        save(d->second, 864);
    }
  }

  d = device_arr_4.begin();

  if(!m_bCheckUserCode4){
    for(; d != device_arr_4.end(); d++){
      save(d->second, 864);
    }
  }
  else{
    deleteUsercode(d->first);
  }

  m_fn->onSync(true);
}




