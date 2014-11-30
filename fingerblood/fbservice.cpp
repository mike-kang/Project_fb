#include "fbservice.h"
#include "tools/log.h"
#include <stdio.h>
#include <fstream>

using namespace std;
using namespace tools;

#define LOG_TAG "FBService"

FBService::FBService(const char* path, Serial::Baud baud, IFBService::IFBServiceEventListener* fn, bool bCheckUserCode4):m_fn(fn), m_bCheckUserCode4(bCheckUserCode4)
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
    LOGE("[vers]exception fail! %d", e);
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
    LOGE("[checkdeviceID]exception fail! %d", e);
  }
  return false;
}

bool FBService::getList(list<string>& li)
{
  try{
    if(!m_protocol)
      cout << "m_protocol null" << endl;
    m_protocol->user(li);
    //for(list<string>::iterator itr = li.begin(); itr != li.end(); itr++)
    //  cout << *itr << endl;
    return true;
  }
  catch (FBProtocol::Exception e){
    LOGE("[getList]exception fail! %d", e);
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
  LOGV("format\n");
  bool ret = m_protocol->init();

  //m_TimerRestart->start(5);
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
  LOGV("save %s\n", filename);
  return m_protocol->save(filename);
}

bool FBService::save(const byte* buf, int length)
{
  LOGV("save\n");
  return m_protocol->save(buf, length);
}

bool FBService::deleteUsercode(const char* usercode)
{
  LOGV("deleteUsercode: %s\n", usercode);
  return m_protocol->dele(usercode);
}

#define S_INIT 0
#define S_READY 1
void FBService::run_scan()
{
  static int state = S_INIT;
  int interval = m_scan_interval * 1000;
  bool bNeedInterval = true;
  char ret;
  char buf[17];
  buf[16] = '\0';
  bool bLong;
  while(m_scan_running){
    usleep(interval);
    ret = m_protocol->stat(buf, bLong);
    //printf("result %c %d\n", ret, bLong);

    if(state == S_INIT){
      if(ret == '2')
        state = S_READY;
    }
    else if (state == S_READY){
      if(bLong && ret == 'A'){
        m_fn->onScanData(buf);
        state = S_INIT;
      }
      else if(ret == 'B'){
        m_fn->onScanData("");
        state = S_INIT;
      }
    }
   
  }

}

void FBService::run_sync()
{
  LOGV("run_sync\n");
  vector<pair<const char*, unsigned char*> >device_arr_16, device_arr_4;
  m_fn->onNeedUserCodeList(device_arr_16, device_arr_4);

  list<string> module_list; //only list usercode length 16
  if(!getList(module_list))
    m_fn->onSyncComplete(false);

  int modulelist_count = module_list.size();
  LOGV("module_list.size %d\n", modulelist_count);

  if(modulelist_count == 0){
    for(vector<pair<const char*, unsigned char*> >::size_type d = 0; d < device_arr_16.size() ; d++){
      save(device_arr_16[d].second, 864);
    }
  }
  else{
    LOGV("device_list16.size %d\n", device_arr_16.size());
    list<string>::iterator m;
#ifdef _DEBUG
    ofstream oOut_d("device.list");
    ofstream oOut_m("module.list");
    for(vector<pair<const char*, unsigned char*> >::size_type d = 0; d < device_arr_16.size() ; d++){
      //cout << "dev-" << d->first << endl;
      oOut_d << device_arr_16[d].first << endl;
    }
    oOut_d.close();
    for(m = module_list.begin(); m != module_list.end(); m++){
      oOut_m << *m << endl;
    }
    oOut_m.close();
#endif
    int compare;
    m = module_list.begin();
    vector<pair<const char*, unsigned char*> >::size_type d = 0;
    
    while(d < device_arr_16.size()){
      const char* device_usercode = device_arr_16[d].first;
      const char* module_usercode = m->c_str();
      LOGV("[dev]%s vs [mod]%s\n", device_usercode, module_usercode);
      compare = strcmp(device_usercode, module_usercode);
      if(!compare){
        d++, m++;
      }
      else if(compare > 0){
        deleteUsercode(module_usercode);
        m++;
      }
      else{
        save(device_arr_16[d].second, 864);
        d++;
      }
      if(m == module_list.end()){
        for(; d < device_arr_16.size() ; d++){
          save(device_arr_16[d].second, 864);
        }
        break;
      }
    }
    
    for(; m != module_list.end(); m++){
      deleteUsercode(m->c_str());
    }
    
  }
  LOGV("device_list4.size %d\n", device_arr_4.size());

  if(!m_bCheckUserCode4){
    for(vector<pair<const char*, unsigned char*> >::size_type d = 0; d < device_arr_4.size() ; d++){
        save(device_arr_4[d].second, 864);
    }
  }
  else{
    for(vector<pair<const char*, unsigned char*> >::size_type d = 0; d < device_arr_4.size() ; d++){
      deleteUsercode(device_arr_4[d].first);
    }
  }

  m_fn->onSyncComplete(true);
}




