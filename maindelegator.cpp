#include "maindelegator.h"
#include "tools/log.h"
#include "serialRfid1356.h"
#include "serialRfid900.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>
#include "tools/timer.h"
#include "tools/utils.h"
#include "tools/network.h"
#include "tools/filesystem.h"
#include "timesheetmgr.h"
#include <errno.h>
#include "web/mydwwebservice.h"
using namespace tools;
using namespace std;
using namespace web;

#define LOG_TAG "MainDelegator"

//utf8
string str_nodata = " 데이터가 없습니다.";
string str_prohibition_entrance = "출입금지 ";
string str_prohibition_entrance_3out = "출입금지 삼진아웃";
string str_success = " 인증에 성공했습니다.";
string str_fail = "인증에 실패했습니다. 재시도 하세요.";

MainDelegator* MainDelegator::my = NULL;

MainDelegator* MainDelegator::createInstance(EventListener* el, const char* config)
{
  if(my){
    return my;
  }
  my = new MainDelegator(el, config);
  return my;
}   

bool MainDelegator::checkValidate(EmployeeInfoMgr::EmployeeInfo* ei, string& msg)
{
  bool bAccess;
/*
  //IN_OUT_GB
  LOGV("checkValidate\n");
  cout << "in_out_gb:" << ei->in_out_gb << endl;
  if(ei->in_out_gb == "0002")
  {
    if(ei->utype == 'C')
      return true;
    //check zone && date
    if(!checkZone(ei->zone_code)){
      LOGE("Authority Deny This Area\n");
      msg = "Authority Deny This Area";
      return false;
    }
    if(!checkDate(ei->ent_co_ymd, ei->rtr_co_ymd, msg)){
      return false;
    }
    return true;
  }
  else{
    if(ei->in_out_gb ==  "0001"){
      LOGE("Access Deny Standby at Work\n");
      msg= "Access Deny Standby at Work";
      return false;
    }
    if(ei->in_out_gb == "0003"){
      LOGE("Access Deny Retired at Work\n");
      msg = "Access Deny Retired at Work";
      return false;
    }

    LOGE("Access Deny Check Work Status\n");
    msg = "Access Deny Check Work Status";
    return false;
  }

  LOGI("checkValidate success!\n");
  */  

  return true;
}

#ifdef LOCATION
bool MainDelegator::checkZone(string& sAuth)
{
  string sLoc = m_sDvLoc;
  bool bAccess;

  if((sAuth=="" && sLoc == "0001") || sAuth == "0001")
    return true;

  vector<string> sarrAuth;
  utils::split(m_sAuthCd, ',', sarrAuth);
  //for(vector<string>::iterator i = sarrAuth.begin(); i != sarrAuth.end(); i++)
  //  cout << *i << endl;
  if(sarrAuth.size() > 0){
    for(vector<string>::iterator sZone = sarrAuth.begin(); sZone != sarrAuth.end(); sZone++){
      if(*sZone == sAuth){
        return true;
      }
    }
  }
  else
  {
    if(sAuth == "0002" && (sLoc == "0001" || sLoc == "0002" || sLoc == "0003"))
      return true;

    if(sAuth == "0003" && (sLoc == "0001" || sLoc == "0004" || sLoc == "0005"))
      return true;
        
    if(sAuth == "0004" &&(sLoc == "0001" || sLoc == "0004" || sLoc == "0005")) 
      return true;
        
    if(sAuth == "0005" && sLoc == "0001") 
      return true;

    if(sAuth == "0006" && (sLoc == "0001" || sLoc == "0003" || sLoc == "0006")) 
      return true;

     if(sAuth == "0007" &&(sLoc == "0001" || sLoc == "0003" || sLoc == "0004" || sLoc == "0005"))
      return true;

    if(sAuth == "0008" && (sLoc == "0001" || sLoc == "0003")) 
      return true;

    if(sAuth == "0009" && (sLoc == "0001" || sLoc == "0002"  || sLoc == "0003"  || sLoc == "0004" || sLoc == "0005"))
      return true;

    if(sAuth == "0010" && (sLoc == "0001" || sLoc == "0003"  || sLoc == "0004" || sLoc == "0005" || sLoc == "0006"))
      return true;
  }
  return false;
}
#endif

bool MainDelegator::checkDate(Date* start, Date* end, string& msg)
{
  if(!start){ 
    LOGE("Access Deny: No Date information\n");
    msg = "Access Deny: No Date information";
    return false;
  }
  
  Date* today = Date::now();
  if(*start > *today){ 
    msg = "Access Deny start date:" + start->toString();
    LOGE((msg+"\n").c_str());
    return false;
  }
  if(end && *end < *today){ 
    msg = "Access Deny work Until:" + end->toString();
    LOGE((msg+"\n").c_str());
    return false;
  }
  msg = "Checked Success ";
  if(end)
    msg += "at Work Until " + end->toString();
  return true;
}

const char* MainDelegator::debug_str(AuthMode m)
{
  switch(m){
    case AM_NORMAL: return "AM_NORMAL";
    case AM_PASS_NOREGISTOR: return "AM_PASS_NOREGISTOR";
    case AM_PASS_THREEOUT: return "AM_PASS_THREEOUT";
  }
}
void MainDelegator::processAuthResult(bool result, const char* sound_path, string msg)
{
  if(m_bSound)
    m_wp->play(sound_path);
  if(result){
    m_Relay->on(1500);
    m_greenLed->on(1500);
  }
  else{
    m_redLed->on(1500);
  }
  m_el->onMessage("Msg", msg);
  m_el->onImage(result);
}
void MainDelegator::onScanData(const char* usercode)
{
  LOGI("onScanData %s +++\n", usercode);
  char* imgBuf = NULL;;
  int imgLength = 0;
  static int out_count = 0;
  string msg;
  
  m_bProcessingAuth = true;

  m_wp->stop();
  m_greenLed->off();
  m_redLed->off();
  //m_el->onMessage("RfidNo", usercode);
  if(usercode){
    string str_usercode(usercode);
    EmployeeInfoMgr::EmployeeInfo* ei;
    bool ret = m_employInfoMgr->getInfo(usercode, &ei);

    if(!ret){
      LOGE("get employee info fail!\n");
      m_el->onEmployeeInfo("", "", "");
      processAuthResult(false, "SoundFiles/authfail.wav", str_usercode + str_nodata);
      goto error;
    }

    if(m_bDisplayEmployeeInfo){
      //cout << "pinno: " << ei->pin_no << endl;
      m_el->onEmployeeInfo(ei->company_name, ei->lab_name, ei->pin_no);
    }
    if(m_admin1 == usercode){
      LOGV("Mode Change : %s -> AM_NORMAL\n", debug_str(m_authMode));   
      m_authMode = AM_NORMAL;
      out_count = 0;
    }
    else if(m_admin2 == usercode){
      LOGV("Mode Change : %s -> AM_PASS_NOREGISTOR\n", debug_str(m_authMode));   
      m_authMode = AM_PASS_NOREGISTOR;
    }
    else if(m_admin3 == usercode){
      LOGV("Mode Change : %s -> AM_PASS_THREEOUT\n", debug_str(m_authMode));   
      m_authMode = AM_PASS_THREEOUT;
    }
    else if(m_admin4 == usercode){
      LOGV("admin4\n");   
      ; // todo
    }

    if(m_bCheck){
      if(ei->blacklistinfo != ""){
        LOGV("blacklist: %s\n", ei->blacklistinfo.c_str());   
        processAuthResult(false, "SoundFiles/authcheck.wav", str_prohibition_entrance + ei->blacklistinfo);
        goto error;
      }
      if(ei->pnt_cnt >= 3){
        LOGV("penalty count: %d\n", ei->pnt_cnt);   
        processAuthResult(false, "SoundFiles/authcheck.wav", str_prohibition_entrance_3out);
        goto error;
      }
    }
    
    processAuthResult(true, "SoundFiles/authok.wav", str_usercode + str_success);
    out_count = 0;
    m_timeSheetMgr->insert(ei->pin_no);
 
  }
  else {
    m_el->onEmployeeInfo("", "", "");
    switch(m_authMode){
      case AM_NORMAL:
        processAuthResult(false, "SoundFiles/authfail.wav", str_fail);
        break;
        
      case AM_PASS_NOREGISTOR:
        processAuthResult(true, "SoundFiles/authok.wav", str_success);
        break;

      case AM_PASS_THREEOUT:
        if(out_count < 2){
          out_count++;
          processAuthResult(false, "SoundFiles/authfail.wav", str_fail);
        }
        else{
          processAuthResult(true, "SoundFiles/authok.wav", str_success);
          out_count = 0;
        }
        break;
        
    }
  }

error:
  LOGI("onScanData ---\n");
  m_bProcessingAuth = false;
}

bool MainDelegator::onNeedDeviceKey(char* id, char* key)
{
  cout << "onNeedDeviceKey:" << id << endl;
  static char num[] = { 0,1,2,3,4,5,6,7,8,9, 0,0,0,0,0,0,0, 10,11,12,13,14,15};
  char id_buf[40];
  try{
    sprintf(id_buf, "FB_KEY::%s", id);
    string& strkey = m_settings->get(id_buf);
    const char* str = strkey.c_str();
    for(int i = 0; i < 8; i++){
      key[i] = num[str[i*2] - 48]*16 + num[str[i*2+1] - 48];
    }
    return true;
  }
  catch(Settings::Exception e){
    LOGE("onNeedDeviceKey exception %d\n", e);
  }
  return false;
}

void MainDelegator::onNeedUserCodeList(std::vector<pair<const char*, unsigned char*> >& arr_16, std::vector<pair<const char*, unsigned char*> >& arr_4)
{
  m_employInfoMgr->getEmployeeList(arr_16, arr_4);
}

//init
void MainDelegator::onSync(IFBService::IFBServiceEventListener::SyncStatus status, int val)
{
  LOGV("onSync %d\n", status);
  //m_bSyncDeviceAndModule = result;
  switch(status){
    case SS_START:
      m_el->onSyncStart();
      break;
    case SS_COUNT:
      cout << "onSync SS_COUNT: " << val << endl;
      m_el->onSyncCount(val);
      break;
    case SS_PROCESS:
      m_el->onSyncIndex(val);
      break;
    case SS_SUCCESS:
      cout << "onSync SS_SUCCESS" << endl;
      m_el->onSyncEnd(true);
      if(m_bTimeAvailable){
        m_employInfoMgr->updateLocalDBfromServer();
      }
      else{
        m_timer = new Timer(cbTimer, this);
        int interval = m_settings->getInt("App::TIMER_INTERVAL");
        LOGI("timer interval= %d\n", interval);
        m_timer->start(interval, true);
        
        m_fbs->request_startScan(300);
      }
      break;
    case SS_FAIL:
      cout << "onSync SS_FAIL" << endl;
      m_el->onSyncEnd(false);
      break;
  }
}

void MainDelegator::onFormat(bool ret)
{
  LOGV("onformat %d\n", ret);
}
void MainDelegator::onUpdateBegin(int save_count, int delete_count)
{
  m_el->onUpdateFBBegin(delete_count, save_count);
}
void MainDelegator::onUpdateSave(int index)
{
  m_el->onUpdateFBSaveIndex(index);
}
void MainDelegator::onUpdateDelete(int index)
{
  m_el->onUpdateFBDeleteIndex(index);
}
void MainDelegator::onUpdateEnd()
{
  m_el->onUpdateFBEnd();
  if(!m_timer){
    m_timer = new Timer(cbTimer, this);
    int interval = m_settings->getInt("App::TIMER_INTERVAL");
    LOGI("timer interval= %d\n", interval);
    m_timer->start(interval, true);
    
    m_fbs->request_startScan(300);
  }
}

/*
void MainDelegator::run()
{
  while(true)
  {
    //dispatch event
    m_event = m_eventQ.pop();
    
    LOGI("MainDelegator::run: %p\n", m_event);
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
*/
void MainDelegator::onEmployeeMgrUpdateStart()
{
  m_el->onUpdateStart();
}

void MainDelegator::onEmployeeMgrUpdateCount(int delete_count, int update_count, int insert_count)
{
  LOGV("onEmployeeMgrUpdateCount delete:%d update:%d insert:%d\n", delete_count, update_count, insert_count);
  m_el->onUpdateCount(delete_count, update_count, insert_count);
}
/*
void MainDelegator::onEmployeeMgrUpdateInsert(const unsigned char* userdata, int index)
{
  LOGV("onEmployeeMgrUpdateInsert %d\n", index);
  m_fbs->save(userdata, USERDATA_SIZE);
  m_el->onUpdateInsertIndex(index);
}
void MainDelegator::onEmployeeMgrUpdateUpdate(string& usercode, const unsigned char* userdata, int index)
{
  LOGV("onEmployeeMgrUpdateUpdate %d\n", index);
  m_fbs->save(userdata, USERDATA_SIZE);
  m_el->onUpdateUpdateIndex(index);
}
void MainDelegator::onEmployeeMgrUpdateDelete(string& usercode, int index)
{
  LOGV("onEmployeeMgrUpdateDelete %d\n", index);
  m_fbs->deleteUsercode(usercode.c_str());
  m_el->onUpdateDeleteIndex(index);
}
*/
void MainDelegator::onEmployeeMgrUpdateEnd(const char* updatetime, vector<unsigned char*>* arrSave, vector<string>* arrDelete)
{
  LOGV("onEmployeeMgrUpdateEnd\n");
  m_el->onUpdateLocalDBEnd(updatetime);
  m_fbs->request_update(arrSave, arrDelete);  //async
}

void MainDelegator::onEmployeeCountChanged(int length_16, int length_4)
{
  if(!m_bCheckUsercode4)
    m_el->onMessage("Download", utils::itoa(length_16 + length_4, 10));
  else
    m_el->onMessage("Download", utils::itoa(length_16, 10));
}

void MainDelegator::onTimeSheetFileCountChanged(int count)
{
  LOGV("onTimeSheetFileCountChanged %d\n", count);
  m_el->onMessage("UploadFilesCount", utils::itoa(count, 10));
}
void MainDelegator::onTimeSheetCacheCountChanged(int count)
{
  LOGV("onTimeSheetCacheCountChanged %d\n", count);
  m_el->onMessage("UploadCacheCount", utils::itoa(count, 10));
}
/*
struct client_data {
  int retval;
  int timelimit;

  client_data(int t):timelimit(t){};
};

struct client_data_Rfid : client_data {
  char* m_serialnum;

  client_data_Rfid(int timelimit, char* serialnum=NULL)
    :client_data(timelimit), m_serialnum(serialnum){} 
};
*/
//blocking function
/*
bool MainDelegator::request_processRfidSerialData(char* serialnum, int timelimit)
{
  bool ret = false;
  LOGV("request_processRfidSerialData\n");
  m_rfid_mtx.lock();
  client_data_Rfid* cd = new client_data_Rfid(timelimit, serialnum);
  TEvent<MainDelegator>* e = new TEvent<MainDelegator>(&MainDelegator::_processRfidSerialData, cd);
  m_eventQ.push(e);

  m_rfid_process_completed.wait(m_rfid_mtx);
  m_rfid_mtx.unlock();

  if(cd->retval == RET_SUCCESS){
    ret = true;
  }

  delete cd;
  return ret;

}

void MainDelegator::_processRfidSerialData(void* arg)
{
  int ret;
  client_data_Rfid* cd = (client_data_Rfid*)arg;
  int sock;
  char buf[4096];
  int readlen;
  int contentsLength;

  m_rfid_mtx.lock();
  Gpio gpio(17, true);
  gpio.write(true);
  //request to web server



error:  
  m_rfid_process_completed.notify_one();
  
  m_rfid_mtx.unlock();
}
*/

//static
void MainDelegator::cbStatusUpdate(void *client_data, int status, void* ret)
{
  LOGV("cbStatusUpdate status:%d, ret:%d\n", status, *((bool*)ret));
}

bool MainDelegator::checkAndRunFBService()
{
  bool ret = true;
  if(!m_bFBServiceRunning){
    m_bFBServiceRunning = m_fbs->request_openDevice(m_settings->getBool("FB::CHECK_DEVICE_ID"));
    if(m_bFBServiceRunning){
      m_el->onMessage("FID", "FID On");
      m_fbs->request_buzzer(m_settings->getBool("FB::BUZZER"));
      m_fbs->request_sync();
    }
    else{
      m_el->onMessage("FID", "FID Off");
      ret = false;
    }
  }

  return ret;
}

//static
void MainDelegator::cbTimerDeferredInit(void* arg)
{
  MainDelegator* my = (MainDelegator*)arg;
  bool repeat = false;

  if(!my->checkAndRunFBService())
    repeat = true;

  if(!my->m_ws){
    if(my->createWebService()){
      if(my->checkServerAlive()){
        my->m_bTimeAvailable = my->getSeverTime();
      }
    }
    else
      repeat = true;
  }
  
  if(repeat){
    my->m_timer_deferredInit->start(10, false);
  }
  else{
    delete my->m_timer_deferredInit;
    my->m_timer_deferredInit = NULL;
  }
}

//static
//check network, upload status, download db, upload timesheet
void MainDelegator::cbTimer(void* arg)
{
  MainDelegator* md = (MainDelegator*)arg;

  static int count = 0;
  bool ret = false;

  struct tm now;
  time_t t = time(NULL);
  localtime_r(&t, &now);
  if(now.tm_hour == 0 && now.tm_min == 0){
    LOGE("**********************************************************\n");
    LOGE("*****%4d-%02d-%02dT%02d-%02d***\n", now.tm_year+1900, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min);
    LOGE("**********************************************************\n");
  }

  LOGV("cbTimer count=%d\n", count);
/*
  if(md->m_bProcessingAuth){
    LOGV("cbTimer returned by processing card\n");
    return;
  }
*/
  if(md->checkServerAlive() && !md->m_bTimeAvailable)
    md->m_bTimeAvailable = md->getSeverTime();

  switch(count){
    case 4:
      if(md->m_bTimeAvailable/* && md->m_bSyncDeviceAndModule*/){
        md->m_employInfoMgr->updateLocalDBfromServer();
      }
      count = 0;
      break;
      
    default:
      //upload timesheet
      md->m_timeSheetMgr->upload();
      count++;
      break;
  }
  
}

bool MainDelegator::checkServerAlive()
{
  bool ret = false;
  if(!m_ws){
    LOGE("m_ws is NULL!\n");
    displayNetworkStatus(false);
    return false;
  }
  
  try{
    ret = m_ws->request_CheckNetwork(2000);  //blocked I/O
  }
  catch(Except e){
    LOGE("request_CheckNetwork: %s\n", WebService::dump_error(e));
    goto error;
  }
  
  if(ret){
    displayNetworkStatus(true);
    return true;
  }

error:  
  displayNetworkStatus(false);
  return false;
}

void MainDelegator::displayNetworkStatus(bool val)
{
  static bool bInitialized = false;
  
  if(!bInitialized || val != m_bNetworkAvailable){
    if(val){
      LOGV("Network ON\n");
      m_el->onMessage("Network", "Network On");
      m_yellowLed->off();
    }
    else{
      LOGV("Network OFF\n");
      m_el->onMessage("Network", "Network Off");
      int arr[] = {1000, 1000, 0};
      m_yellowLed->on(arr, true);
    }
    m_bNetworkAvailable = val;
  }
  
  if(!bInitialized)
    bInitialized = true;
}

bool MainDelegator::SettingInit(const char* configPath)
{
  if(configPath)
    m_settings = new Settings(configPath);
  else
    m_settings = new Settings("/etc/acufb/FID.ini");

  //App
  //m_sAuthCd = m_settings->get("App::AUTH_CD");
  m_sMemcoCd = m_settings->get("App::MEMCO");
  m_sSiteCd = m_settings->get("App::SITE");
  //m_sDvLoc = m_settings->get("App::DV_LOC"); // = "0001";
  m_sDvNo = m_settings->get("App::DV_NO"); // = "1";
  m_sInOut = m_settings->get("App::IN_OUT");
  m_sEmbedCd = m_settings->get("App::EMBED"); // = "0000000008";
  m_bCheck = m_settings->getBool("App::CHECK");
  m_sAuthCode = m_settings->get("App::AUTH_CODE");
  m_bTestSignal = m_settings->getBool("App::TEST_SIGNAL");
  m_admin1 = m_settings->get("App::ADMIN1");
  m_admin2 = m_settings->get("App::ADMIN2");
  m_admin3 = m_settings->get("App::ADMIN3");
  m_admin4 = m_settings->get("App::ADMIN4");
  m_bDisplayEmployeeInfo = m_settings->getBool("App::DISPLAY_EMPLOYEE_INFO");
  m_bCheckUsercode4 = m_settings->getBool("FB::CHECK_CODE_4");
  
  //Action
  m_bCapture = m_settings->getBool("Action::CAPTURE");
  m_bSound = m_settings->getBool("Action::SOUND");

  //Rfid
#ifdef RFID  
  m_sRfidMode = m_settings->get("Rfid::MODE"); //="1356M";
  m_rfidCheckInterval = m_settings->getInt("Rfid::CHECK_INTERVAL"); //300 ms
  m_sRfid1356Port = m_settings->get("Rfid::RFID1356_PORT"); // /dev/ttyAMA0
  m_sRfid900Port = m_settings->get("Rfid::RFID800_PORT"); // /dev/ttyUSB0
#endif
  m_fbCheckInterval = m_settings->getInt("FB::CHECK_INTERVAL"); //300 ms
  m_fbPort = m_settings->get("FB::PORT"); // /dev/ttyUSB0

  //Server
  m_sServerType = m_settings->get("Server::TYPE");
  m_sSafeIdServerUrl = m_settings->get("Server::SAFEIDSVC_URL");
  m_sLotteIdServerUrl = m_settings->get("Server::LOTTEIDSVC_URL");
  m_sDWServerUrl = m_settings->get("Server::DWSVC_URL");

  //Local IP.Mac Address
  try{
    char* p = network::GetIpAddress("eth0");  // or lo
    if(!p)
      m_sLocalIP = "";
    else
      m_sLocalIP = p;
  }
  catch(network::Exception e)
  {
    m_sLocalIP = "";
  }

  m_sLocalMacAddr = network::GetMacAddress("eth0");

  //Log
  //m_consolePath = m_settings->get("Log::CONSOLE_PATH");
  
  return true;
}

MainDelegator::MainDelegator(EventListener* el, const char* configPath) : m_el(el), m_bProcessingAuth(false), m_bFBServiceRunning(false), m_bSyncDeviceAndModule(false), m_timer_deferredInit(NULL), m_timer(NULL), m_ws(NULL)
{
  bool ret;
  bool bNeedDeferredTimer = false;
  
  cout << "start" << endl;
  el->onStatus("System Start");
  SettingInit(configPath);
  string curdir = m_settings->get("App::WORKING_DIRECTORY");
  filesystem::dir_chdir(curdir.c_str());
  cout << "chdir:" << curdir.c_str() << endl;
  
  bool log_console = m_settings->getBool("Log::CONSOLE");
  int log_console_level = m_settings->getInt("Log::CONSOLE_LEVEL");;
  string log_console_path = m_settings->get("Log::CONSOLE_PATH");
  bool file_log = m_settings->getBool("Log::FILE");
  int log_file_level = m_settings->getInt("Log::FILE_LEVEL");;
  string log_file_directory = m_settings->get("Log::FILE_DIRECTORY");
  
  log_init(log_console, log_console_level, log_console_path.c_str(), file_log, log_file_level, log_file_directory.c_str());

  //LED & RELAY
  m_yellowLed = new SwitchGpio(m_settings->getInt("Gpio::YELLOW"));
  m_blueLed = new SwitchGpio(m_settings->getInt("Gpio::BLUE"));
  m_greenLed = new SwitchGpio(m_settings->getInt("Gpio::GREEN"));
  m_redLed = new SwitchGpio(m_settings->getInt("Gpio::RED"));
  m_Relay = new SwitchGpio(m_settings->getInt("Gpio::RELAY"));
  
  //m_thread = new Thread<MainDelegator>(&MainDelegator::run, this, "MainDelegatorThread");
  //LOGV("MainDelegator tid=%lu\n", m_thread->getId());
#ifdef RFID
  if(m_sRfidMode == "1356M")
    m_serialRfid = new SerialRfid1356(m_sRfid1356Port.c_str());
  else if(m_sRfidMode == "1356M2")
    m_serialRfid = new SerialRfid1356_(m_sRfid1356Port.c_str());
  else if(m_sRfidMode == "900M")
    m_serialRfid = new SerialRfid900(m_sRfid1356Port.c_str());
  
  ret = m_serialRfid->open();
  if(!ret){
    LOGE("SerialRfid open fail!\n");
    delete m_serialRfid;
    delete m_settings;
    throw EXCEPTION_RFID_OPEN_FAIL;
  }
  m_serialRfid->start(m_rfidCheckInterval, this); //interval=300ms  
  m_el->onMessage("Rfid", m_sRfidMode + " ON");
#endif  
  m_authMode = AM_NORMAL;

  //m_el->onStatus("WebService Url:" + m_sServerUrl);
  //m_ws = new WebService("192.168.0.7", 8080);

  LOGV("m_sServerURL= %s\n", m_sDWServerUrl.c_str()); 
  LOGV("m_sServerURL= %s\n", m_sLotteIdServerUrl.c_str()); 
  LOGV("m_sServerURL= %s\n", m_sSafeIdServerUrl.c_str()); 
  m_el->onLogo(m_sServerType.c_str());
  
  if(!createWebService()){
    bNeedDeferredTimer = true;
  }

  if(checkServerAlive()){
    m_bTimeAvailable = getSeverTime();
  }
  
  m_employInfoMgr = new EmployeeInfoMgr(m_settings, m_ws, this);
  
  m_fbs = new FBService(m_settings->get("FB::PORT").c_str(), Serial::SB38400, this, m_bCheckUsercode4);

  if(!checkAndRunFBService())
    bNeedDeferredTimer = true;

  //m_timesheetFilesCount = m_timesheetCacheCount = 0;

  m_timeSheetMgr = new TimeSheetMgr(m_settings, m_ws, this);

  if(m_bTestSignal){
    signal(SIGUSR1, test_signal_handler);
    signal(SIGUSR2, test_signal_handler);
    mTimerForTest = new Timer(cbTestTimer, this);
  }
#ifdef LOCATION
  string locationName = getLocationName();
  m_el->onMessage("GateLoc", locationName);
#endif
  m_el->onMessage("Embed", m_sEmbedCd);
  

  m_el->onMessage("GateNo", "No." + m_sDvNo);
  
  m_wp = media::WavPlayer::createInstance(); 
  //m_wp->play("SoundFiles/start.wav");

  string reboot_time = m_settings->get("App::REBOOT_TIME");
  LOGV("reboot time= %s\n", reboot_time.c_str()); 
  if(reboot_time != "")
    setRebootTimer(reboot_time.c_str());

  if(bNeedDeferredTimer){
    m_timer_deferredInit = new Timer(cbTimerDeferredInit, this);
    m_timer_deferredInit->start(10, false);
  }
  
  LOGV("MainDelegator ---\n");
}

MainDelegator::~MainDelegator()
{
  m_timer->stop();
} 

bool MainDelegator::createWebService()
{
  bool ret = false;
  try{
    if(m_sServerType == "DW"){
      DWWebService* dw = new DWWebService(m_sDWServerUrl.c_str(), m_sMemcoCd.c_str(), m_sSiteCd.c_str(), m_sEmbedCd.c_str(), m_sDvNo.c_str(), m_sInOut.c_str()[0]);
      SafemanWebService* sm = new SafemanWebService(m_sSafeIdServerUrl.c_str(), m_sMemcoCd.c_str(), m_sSiteCd.c_str(), m_sEmbedCd.c_str(), m_sDvNo.c_str(), m_sInOut.c_str()[0]);
      m_ws = new MyDWWebService(dw, sm);
      m_el->onStatus(m_sDWServerUrl.c_str());
    }
    else if(m_sServerType == "LT"){
      m_ws = new SafemanWebService(m_sLotteIdServerUrl.c_str(), m_sMemcoCd.c_str(), m_sSiteCd.c_str(), m_sEmbedCd.c_str(), m_sDvNo.c_str(), m_sInOut.c_str()[0]);
      m_el->onStatus(m_sLotteIdServerUrl.c_str());
    }
    else{
      m_ws = new SafemanWebService(m_sSafeIdServerUrl.c_str(), m_sMemcoCd.c_str(), m_sSiteCd.c_str(), m_sEmbedCd.c_str(), m_sDvNo.c_str(), m_sInOut.c_str()[0]);
      m_el->onStatus(m_sSafeIdServerUrl.c_str());
    }
    ret = true;
  }
  catch(network::Exception e){
    LOGE("createWebService fail\n");
  }
  return ret;
}

//#ifdef SIMULATOR
void MainDelegator::cbTestTimer(void* arg)
{
  MainDelegator* my = (MainDelegator*)arg;
  
  if(my->m_signo == SIGUSR1)
    my->onScanData(my->m_test_serial_number.c_str());
  else{
    my->m_fbs->request_saveUsercode("/home/pi/Project_fb/fingerblood/FID0000000000000012.bin");
  }
}

void MainDelegator::test_signal_handler(int signo)
{
  my->m_signo = signo; 
  if(signo == SIGUSR1){
    LOGI("signal_handler SIGUSR1\n");
    //my->m_test_serial_number = "253161024009"; //validate
    my->m_test_serial_number = "0000000000000005"; //validate + Photo image
    my->mTimerForTest->start(1);
  }
  else if(signo == SIGUSR2){
    LOGI("signal_handler SIGUSR2\n");
    my->mTimerForTest->start(1);
  }
}
//#endif

#ifdef LOCATION
string MainDelegator::getLocationName()
{
  string locationName;
  try{
    char* xml_buf = m_ws->request_CodeDataSelect(m_sMemcoCd.c_str(), m_sSiteCd.c_str(), m_sDvLoc.c_str(), 3000);  //blocked I/O
    //xml_buf = m_ws->request_CodeDataSelect("MC00000003", "ST00000005", "0001", cbCodeDataSelect, NULL);  //blocked I/O
    if(xml_buf){
      try{
        char* name = utils::getElementData(xml_buf, "EN_CODE_NM");
        locationName = name;
        return locationName;
      }
      catch(int e){}
      delete xml_buf;
    }
  }
  catch(WebService::Except e){
    LOGE("request_CodeDataSelect: %s\n", WebService::dump_error(e));
  }
  
  if(m_sDvLoc == "0001")
    locationName = "Main Gate";
  else if(m_sDvLoc == "0002")
    locationName = "A Camp";
  else if(m_sDvLoc == "0003")
    locationName = "Office";
  else if(m_sDvLoc == "0004")
    locationName = "C Camp 400";
  else if(m_sDvLoc == "0005")
    locationName = "C Camp 800";
  else if(m_sDvLoc == "0006")
    locationName = "F Camp";

  return locationName;
}
#endif

/*
void MainDelegator::cb_ServerTimeGet(void* arg)
{
  LOGV("cb_ServerTimeGet\n");
  WebService::req_data* rd = (WebService::req_data*)arg;
  MainDelegator* md = (MainDelegator*)rd->m_client;
  TEvent<MainDelegator>* e = new TEvent<MainDelegator>(&MainDelegator::_cb_ServerTimeGet, arg);
  md->m_eventQ.push(e);
  return;
}
*/

bool MainDelegator::getSeverTime()
{
  LOGV("getServerTime\n");
  if(!m_ws){
    LOGE("m_ws is NULL!\n");
    return false;
  }
  
  try{
    char* time_buf = m_ws->request_ServerTime(3000);  //blocked I/O
    //time_buf = m_ws->request_ServerTime(cbServerTimeGet, NULL);  //blocked I/O
    if(time_buf){
      LOGV("getSeverTime %s\n", time_buf);
      char * tok = strtok(time_buf, "-T:");
      int t[10];
      int i = 0;
      t[i++] = atoi(tok);
      while((tok = strtok(NULL, "-T:"))){
        t[i++] = atoi(tok);
        if(i > 5)
          break;
      }
      delete time_buf;
      if(i == 6){ //ok
        struct tm tm;
        tm.tm_year = t[0] - 1900;
        tm.tm_mon = t[1] - 1;
        tm.tm_mday = t[2];
        tm.tm_hour = t[3];
        tm.tm_min = t[4];
        tm.tm_sec = t[5];
        time_t tt = mktime(&tm);
        if (tt < 0){
          LOGE("mktime error: %d-%d-%dT%d:%d:%d\n", t[0], t[1], t[2], t[3], t[4], t[5]);
          return false;
        }
        if(stime(&tt) < 0){
          LOGE("stime error: %s", strerror(errno));
          return false;
        }
      }
      return true;  
    }
  }
  catch(web::Except e){
    LOGE("request_ServerTime: %s\n", WebService::dump_error(e));
    return false;
  }
}

//static
void MainDelegator::cbRebootTimer(void* arg)
{
  MainDelegator* md = (MainDelegator*)arg;

  LOGV("cbRebootTimer\n");

  system("reboot");
}

void MainDelegator::setRebootTimer(const char* time_buf)
{
  LOGV("setRebootTimer %s\n", time_buf);
  char buf[25];
  strcpy(buf, time_buf);
  char * tok = strtok(buf, ":");
  int hour, min;
  int offset = 0;
  hour = atoi(tok);
  tok = strtok(NULL, ":");
  min = atoi(tok);
  
  struct tm now;
  time_t t = time(NULL);
  localtime_r(&t, &now);

  int diff = (hour - now.tm_hour)*60*60 + (min - now.tm_min)*60 - now.tm_sec;
  if(diff <= 0)
    diff += (24*60*60);
  LOGV("Reboot after:%d sec\n", diff);
  m_RebootTimer = new Timer(cbRebootTimer, this);
  m_RebootTimer->start(diff);

}
