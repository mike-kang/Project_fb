#include "fbservice.h"
#include "tools/log.h"
#include <stdio.h>
#include <fstream>
#include <signal.h>

using namespace std;
using namespace tools;

#define LOG_TAG "FBService"

#define S_SCAN_INIT 0
#define S_SCAN_READY 1
#define USERDATA_SIZE 864

FBService::FBService(const char* path, Serial::Baud baud, IFBService::IFBServiceEventListener* fn, bool bCheckUserCode4):m_fn(fn), m_bCheckUserCode4(bCheckUserCode4)
{
  m_serial = new FBProtocolCMSerial(path, baud);
  m_protocol = new FBProtocol(m_serial);
  m_tmrScan = new Timer(cbTimerScan, this);
  m_thread = new Thread<FBService>(&FBService::run, this, "FBServiceThread");

}

FBService::~FBService()
{
  delete m_serial;
  delete m_protocol;
  delete m_tmrScan;
}

void FBService::run()
{
  int s;
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGCHLD);
  s = pthread_sigmask(SIG_BLOCK, &set, NULL);
  
  while(1)
  {
    //dispatch event
    m_event = m_eventQ.pop();
    
    //LOGI("FBService::run: %p\n", m_event);
    if(m_event){
      (this->*(m_event->ev_processFunc))(m_event->ev_data);
      delete m_event;
    }
    else{
      LOGI("Terminate MainDelegator event thread\n");
      break;
    }
  }

}
/*
struct Client_t {
  Semaphore m_SemCompleteProcessEvent;
  bool m_ret;
  Client_t():m_ret(false){};
};
*/
struct syncClient_t {
  Semaphore m_SemCompleteProcessEvent;
  syncClient_t():m_SemCompleteProcessEvent(0)
  {
  }
};
struct boolClient_t {
  bool m_val;
  boolClient_t(bool val):m_val(val)
  {
  }
};
struct syncRClient_t {
  bool m_ret;
  Semaphore m_SemCompleteProcessEvent;
  syncRClient_t():m_ret(false), m_SemCompleteProcessEvent(0)
  {
  }
};

struct openDeviceClient_t {
  bool m_check_device_id;
  Semaphore m_SemCompleteProcessEvent;
  bool m_ret;
  openDeviceClient_t(bool check_device_id):m_check_device_id(check_device_id), m_ret(false), m_SemCompleteProcessEvent(0)
  {
  }
};
bool FBService::request_openDevice(bool check_device_id) //only sync
{
  LOGV("request_openDevice\n");
  openDeviceClient_t* client = new openDeviceClient_t(check_device_id);
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::openDevice, (void*)client);
  m_eventQ.push(e);

  int ret = client->m_SemCompleteProcessEvent.timedwait(10);  //blocking for maxWaitTime.
  LOGV("request_openDevice end waiting\n");
  if(ret < 0){
    LOGE("request_openDevice time expired\n");
    delete client;
    return false;
  }
  delete client;
  return client->m_ret;
}

void FBService::request_closeDevice() //only sync
{
  LOGV("request_closeDevice\n");
  syncRClient_t* client = new syncRClient_t();
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::closeDevice, client);
  m_eventQ.push(e);

  int ret = client->m_SemCompleteProcessEvent.timedwait(5);  //blocking for maxWaitTime.
  LOGV("request_closeDevice end waiting\n");
  if(ret < 0){
    LOGE("request_closeDevice time expired\n");
  }
}

void FBService::request_sync(void) //only async
{
  LOGV("request_sync\n");
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::sync, NULL);
  m_eventQ.push(e);
}

struct getListClient_t {
  list<string>* m_list;
  bool m_ret;
  Semaphore m_SemCompleteProcessEvent;
  getListClient_t(list<string>* li):m_list(li), m_ret(false), m_SemCompleteProcessEvent(0)
  {
  }
};
bool FBService::request_getList(list<string>* li) //only sync
{
  LOGV("request_getList\n");
  getListClient_t* client = new getListClient_t(li);
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::getList, (void*)client);
  m_eventQ.push(e);

  int ret = client->m_SemCompleteProcessEvent.timedwait(10);  //blocking for maxWaitTime.
  LOGV("request_getList end waiting\n");
  if(ret < 0){
    LOGE("request_getList time expired\n");
    delete client;
    return false;
  }
  delete client;
  return client->m_ret;
}

void FBService::request_format() //only async
{
  LOGV("request_format\n");
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::format, NULL);
  m_eventQ.push(e);
}

struct startScanClient_t {
  int m_interval;
  startScanClient_t(int interval):m_interval(interval)
  {
  }
};
void FBService::request_startScan(int interval) //only async
{
  LOGV("request_startScan: %d\n", interval); 
  startScanClient_t* client = new startScanClient_t(interval);
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::startScan, client);
  m_eventQ.push(e);
}

void FBService::request_stopScan() //only async
{
  LOGV("request_stopScan\n");
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::stopScan, NULL);
  m_eventQ.push(e);
}

void FBService::request_buzzer(bool val) //only sync
{
  LOGV("request_buzzer\n");
  boolClient_t* client = new boolClient_t(val);
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::buzzer, client);
  m_eventQ.push(e);
}

struct saveUsercodeClient_t {
  const byte* m_userdata;
  int  m_length;
  Semaphore m_SemCompleteProcessEvent;
  bool m_ret;
  const char* m_path;
  
  saveUsercodeClient_t(const byte* userdata, int length):m_userdata(userdata), m_length(length), m_ret(false), m_SemCompleteProcessEvent(0)
  {
  }
  saveUsercodeClient_t(const char* filename):m_path(filename), m_ret(false), m_SemCompleteProcessEvent(0)
  {
  }
};
bool FBService::request_saveUsercode(const byte* userdata, int length) //only sync
{
  LOGV("request_saveUsercode\n");
  saveUsercodeClient_t* client = new saveUsercodeClient_t(userdata, length);
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::saveUsercodeBuffer, (void*)client);
  m_eventQ.push(e);

  int ret = client->m_SemCompleteProcessEvent.timedwait(5);  //blocking for maxWaitTime.
  LOGV("request_saveUsercode end waiting\n");
  if(ret < 0){
    LOGE("request_saveUsercode time expired\n");
    delete client;
    return false;
  }
  delete client;
  return client->m_ret;
}

bool FBService::request_saveUsercode(const char* filename) //only sync
{
  LOGV("request_saveUsercode\n");
  saveUsercodeClient_t* client = new saveUsercodeClient_t(filename);
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::saveUsercodeFile, (void*)client);
  m_eventQ.push(e);

  int ret = client->m_SemCompleteProcessEvent.timedwait(5);  //blocking for maxWaitTime.
  LOGV("request_saveUsercode end waiting\n");
  if(ret < 0){
    LOGE("request_saveUsercode time expired\n");
    delete client;
    return false;
  }
  delete client;
  return client->m_ret;
}

struct deleteUsercodeClient_t {
  const char* m_usercode;
  Semaphore m_SemCompleteProcessEvent;
  bool m_ret;
  
  deleteUsercodeClient_t(const char* usercode):m_usercode(usercode), m_ret(false), m_SemCompleteProcessEvent(0)
  {
  }
};
bool FBService::request_deleteUsercode(const char* usercode) //only sync
{
  LOGV("request_deleteUsercode\n");
  deleteUsercodeClient_t* client = new deleteUsercodeClient_t(usercode);
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::deleteUsercode, (void*)client);
  m_eventQ.push(e);

  int ret = client->m_SemCompleteProcessEvent.timedwait(5);  //blocking for maxWaitTime.
  LOGV("request_deleteUsercode end waiting\n");
  if(ret < 0){
    LOGE("request_deleteUsercode time expired\n");
    delete client;
    return false;
  }
  delete client;
  return client->m_ret;
}

bool FBService::request_stopCmd() //only sync
{
  LOGV("request_stopCmd\n");
  syncRClient_t* client = new syncRClient_t();
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::stopCmd, client);
  m_eventQ.push(e);

  int ret = client->m_SemCompleteProcessEvent.timedwait(5);  //blocking for maxWaitTime.

  LOGV("request_stopCmd end waiting\n");
  if(ret < 0){
    LOGE("request_stopCmd time expired\n");
    delete client;
    return false;
  }
  delete client;
  return client->m_ret;
}

struct updateClient_t {
  vector<unsigned char*>* m_arrSave;
  vector<string>* m_arrDelete;
  updateClient_t(vector<unsigned char*>* arrSave, vector<string>* arrDelete):m_arrSave(arrSave), m_arrDelete(arrDelete)
  {
  }
};
void FBService::request_update(vector<unsigned char*>* arrSave, vector<string>* arrDelete) //only async
{
  LOGV("request_update\n"); 
  updateClient_t* client = new updateClient_t(arrSave, arrDelete);
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::update, client);
  m_eventQ.push(e);
}


//////////////////////////////////////////////////////////////////////////////////////////

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

void FBService::openDevice(void* arg)
{
  openDeviceClient_t* client = (openDeviceClient_t*)arg;

  m_bActive = false;
  
  if(!m_serial->open()){
    return;
  }
  
  m_protocol->stop();
  
  char* ver = getVersion();
  if(!ver){
    return;
  }
  LOGV("Version: %s\n", ver);
  
  if(client->m_check_device_id && !checkdeviceID()){
    LOGE("openDevice-check device fail\n");
    return;
  }

  m_bActive = true;
  //m_fn->onStart(true);
  client->m_ret = true; 

  client->m_SemCompleteProcessEvent.post();
}

void FBService::closeDevice(void* arg)
{
  syncRClient_t* client = (syncRClient_t*)arg;
  m_bActive = false;
  m_serial->close();
  
  client->m_SemCompleteProcessEvent.post();
}

void FBService::sync(void* arg)
{
  //m_sync_running = true;
  LOGV("run_sync\n");
  m_fn->onSync(IFBService::IFBServiceEventListener::SS_START);
  vector<pair<const char*, unsigned char*> >device_arr_16, device_arr_4;
  m_fn->onNeedUserCodeList(device_arr_16, device_arr_4);

  list<string> module_list; //only list usercode length 16
  try{
    m_protocol->user(module_list);
    //for(list<string>::iterator itr = li.begin(); itr != li.end(); itr++)
    //  cout << *itr << endl;
  }
  catch (FBProtocol::Exception e){
    LOGE("[sync-list]exception fail! %d", e);
    m_fn->onSync(IFBService::IFBServiceEventListener::SS_FAIL);
    return;
  }

  int modulelist_count = module_list.size();
  LOGV("module_list.size %d\n", modulelist_count);

  vector<pair<const char*, unsigned char*> >::size_type d = 0;
  vector<pair<const char*, unsigned char*> >::size_type devicelist_count = device_arr_16.size();
  vector<pair<const char*, unsigned char*> >::size_type devicelist_4_count = device_arr_4.size();

  m_fn->onSync(IFBService::IFBServiceEventListener::SS_COUNT, devicelist_count + devicelist_4_count);
  if(modulelist_count == 0){
    while(d < devicelist_count){
      m_fn->onSync(IFBService::IFBServiceEventListener::SS_PROCESS, d);
      if(!m_protocol->save(device_arr_16[d].second, USERDATA_SIZE)){
        m_protocol->stop();
      }
      else
        d++;
    }
  }
  else{
    LOGV("device_list16.size %d\n", devicelist_count);
    list<string>::iterator m;
#ifdef _DEBUG
    ofstream oOut_d("device.list");
    ofstream oOut_m("module.list");
    for( d = 0; d < devicelist_count ; d++){
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
    d = 0;
    
    while(d < devicelist_count){
      m_fn->onSync(IFBService::IFBServiceEventListener::SS_PROCESS, d);
      const char* device_usercode = device_arr_16[d].first;
      const char* module_usercode = m->c_str();
      LOGV("[dev]%s vs [mod]%s\n", device_usercode, module_usercode);
      compare = strcmp(device_usercode, module_usercode);
      if(!compare){
        d++, m++;
      }
      else if(compare > 0){
        m_protocol->dele(module_usercode);
        m++;
      }
      else{
        if(!m_protocol->save(device_arr_16[d].second, USERDATA_SIZE))
          m_protocol->stop();
        else
          d++;
      }
      
      if(m == module_list.end()){
        while(d < device_arr_16.size()){
          m_fn->onSync(IFBService::IFBServiceEventListener::SS_PROCESS, d);
          if(!m_protocol->save(device_arr_16[d].second, USERDATA_SIZE))
            m_protocol->stop();
          else
            d++;
        }
        break;
      }
    }
    
    for(; m != module_list.end(); m++){
      m_protocol->dele(m->c_str());
    }
    
  }
  LOGV("device_list4.size %d\n", devicelist_4_count);

  if(!m_bCheckUserCode4){
    d = 0;
    while(d < devicelist_4_count){
      m_fn->onSync(IFBService::IFBServiceEventListener::SS_PROCESS, devicelist_count + d);
      if(!m_protocol->save(device_arr_4[d].second, USERDATA_SIZE))
        m_protocol->stop();
      else
        d++;
    }
  }
  else{
    for(d = 0; d < devicelist_4_count ; d++){
      m_fn->onSync(IFBService::IFBServiceEventListener::SS_PROCESS, devicelist_count + d);
      m_protocol->dele(device_arr_4[d].first);
    }
  }

  m_fn->onSync(IFBService::IFBServiceEventListener::SS_SUCCESS);
  
  //m_sync_running = false;
}

bool FBService::checkdeviceID()
{
  try{
    char* device_id = m_protocol->didr();
    char key[8];
    if(m_fn->onNeedDeviceKey(device_id, key)){
      m_protocol->didk(key);
      return true;
    }
  }
  catch (FBProtocol::Exception e){
    LOGE("[checkdeviceID]exception fail! %d", e);
  }
  return false;
}

void FBService::getList(void* arg)
{
  getListClient_t* client = (getListClient_t*)arg;
  try{
    m_protocol->user(*client->m_list);
    //for(list<string>::iterator itr = li.begin(); itr != li.end(); itr++)
    //  cout << *itr << endl;
    client->m_ret = true; 
  }
  catch (FBProtocol::Exception e){
    LOGE("[getList]exception fail! %d", e);
    client->m_ret = false; 
  }
  //m_fn->onStart(true);

  client->m_SemCompleteProcessEvent.post();

}


void FBService::format(void* arg)
{
  LOGV("format +++\n");
  try{
    m_protocol->init();
    m_fn->onFormat(true);
  }
  catch(FBProtocol::Exception e){
    LOGE("[format]exception fail! %d\n", e);
    m_fn->onFormat(false);
  }
  LOGV("format ---\n");
}

//static
void FBService::cbTimerScan(void* arg)
{
  FBService* my = (FBService*)arg;
  TEvent<FBService>* e = new TEvent<FBService>(&FBService::scan, NULL);
  my->m_eventQ.push(e);
}

void FBService::startScan(void* arg)
{
  startScanClient_t* client = (startScanClient_t*)arg;

  m_scan_running = true;
  m_scan_interval = client->m_interval;
  delete client;
  cout << "interval:" << m_scan_interval << endl;

  TEvent<FBService>* e = new TEvent<FBService>(&FBService::scan, NULL);
  m_eventQ.push(e);
}

void FBService::scan(void* arg)
{
  if(!m_scan_running)
    return;

  static int state = S_SCAN_INIT;
  char ret;
  char buf[17];
  buf[16] = '\0';
  bool bLong;
  ret = m_protocol->stat(buf, bLong);
  //printf("result %c %d\n", ret, bLong);

  if(state == S_SCAN_INIT){
    if(ret == '2')
      state = S_SCAN_READY;
  }
  else if (state == S_SCAN_READY){
    if(bLong && ret == 'A'){
      m_fn->onScanData(buf);
      state = S_SCAN_INIT;
    }
    else if(ret == 'B'){
      m_fn->onScanData(NULL);
      state = S_SCAN_INIT;
    }
  }

  m_tmrScan->start(0, m_scan_interval);
}

void FBService::stopScan(void* arg)
{
  m_scan_running = false;
}

void FBService::buzzer(void* arg)
{
  boolClient_t* client = (boolClient_t*)arg;
  
  bool val = client->m_val;
  delete client;
  byte buf[2] = {0x40, 0x00};
  if(val)
    buf[0] = 0x40;
  else
    buf[0] = 0xc0;
  
  m_protocol->optf(buf);
}

void FBService::saveUsercodeFile(void* arg)
{
  saveUsercodeClient_t* client = (saveUsercodeClient_t*)arg;
  
  LOGV("saveUsercodeFile %s\n", client->m_path);
  client->m_ret = m_protocol->save(client->m_path);
  if(client->m_ret)
    m_protocol->stop();
  client->m_SemCompleteProcessEvent.post();
}

void FBService::saveUsercodeBuffer(void* arg)
{
  LOGV("saveUsercodeBuffer\n");
  saveUsercodeClient_t* client = (saveUsercodeClient_t*)arg;
  client->m_ret = m_protocol->save(client->m_userdata, client->m_length);
  if(client->m_ret)
    m_protocol->stop();
  client->m_SemCompleteProcessEvent.post();
}

void FBService::deleteUsercode(void* arg)
{
  deleteUsercodeClient_t* client = (deleteUsercodeClient_t*)arg;
  LOGV("deleteUsercode: %s\n", client->m_usercode);
  
  client->m_ret = m_protocol->dele(client->m_usercode);

  client->m_SemCompleteProcessEvent.post();
}

void FBService::stopCmd(void* arg)
{
  syncRClient_t* client = (syncRClient_t*)arg;
  
  client->m_ret = client->m_ret = m_protocol->stop();

  client->m_SemCompleteProcessEvent.post();
}

void FBService::update(void* arg)
{
  updateClient_t* client = (updateClient_t*)arg;
  int delete_count = client->m_arrDelete->size();
  int save_count = client->m_arrSave->size();
  
  LOGV("update: delete %d, save %d\n", delete_count, save_count); 
  m_fn->onUpdateBegin(client->m_arrSave->size(), client->m_arrDelete->size());

  for(vector<string>::size_type i=0;i<delete_count;i++){
    if(m_protocol->dele( (*(client->m_arrDelete))[i].c_str())){
      m_fn->onUpdateDelete(i+1);
    }
    else{
      LOGE("update - delete fail %s\n", (*(client->m_arrDelete))[i].c_str() );
    }
  }
  
  vector<unsigned char*>::size_type d = 0;
  while(d < save_count){
    if(m_protocol->save( (*(client->m_arrSave))[d], USERDATA_SIZE)){
      //LOGV("update - save[%d] success\n", d+1);
      m_fn->onUpdateSave(d+1);
      d++;
    }
    else{
      LOGE("update - save[%d] fail\n", d+1);
      while(!m_protocol->stop());
    }
  }
  m_fn->onUpdateEnd();
}





