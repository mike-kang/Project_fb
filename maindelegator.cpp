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

using namespace tools;
using namespace std;

#define LOG_TAG "MainDelegator"

MainDelegator* MainDelegator::my = NULL;

MainDelegator* MainDelegator::createInstance(EventListener* el)
{
  if(my){
    return my;
  }
  my = new MainDelegator(el);
  return my;
}   

bool MainDelegator::checkValidate(EmployeeInfoMgr::EmployeeInfo* ei, string& msg)
{
  bool bAccess;
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

  return true;
}

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

void MainDelegator::onData(const char* serialNumber)
{
  LOGI("onData %s +++\n", serialNumber);
  char* imgBuf = NULL;;
  int imgLength = 0;
  string msg;
  m_bProcessingRfidData = true;
  printf("onData: %s\n", serialNumber);
  m_wp->stop();
  m_greenLed->off();
  m_redLed->off();
  m_el->onMessage("RfidNo", serialNumber);
  EmployeeInfoMgr::EmployeeInfo* ei = new EmployeeInfoMgr::EmployeeInfo;
  checkNetwork();
  bool ret = m_employInfoMrg->getInfo(serialNumber, ei);

  if(!ret){
    LOGE("get employee info fail!\n");
    m_el->onEmployeeInfo("", "", "", NULL, 0);
    if(m_bSound)
      m_wp->play("SoundFiles/fail.wav");
    m_redLed->on(1500);
    m_el->onMessage("Result", "FAIL");
    m_el->onMessage("Msg", "NO DATA");
    goto error;
  }
  m_el->onEmployeeInfo(ei->company_name, ei->lab_name, ei->pin_no, ei->img_buf, ei->img_size);
  if(!checkValidate(ei, msg)){
    if(m_bSound)
      m_wp->play("SoundFiles/fail.wav");
    m_redLed->on(1500);
    m_el->onMessage("Result", "FAIL");
    m_el->onMessage("Msg", msg);
    goto error;
  }

  if(m_bSound)
    m_wp->play("SoundFiles/ok.wav");
  m_greenLed->on(1500);
  m_Relay->on(1500);
  m_el->onMessage("Msg", msg);
  m_el->onMessage("Result", "OK");

#ifdef CAMERA
  if(m_bCapture){
    if(m_cameraStill->takePicture(&imgBuf, &imgLength, m_takePictureMaxWaitTime))
    {
      //for debuf
      if(m_bSavePictureFile){
        if(imgLength < 0)
          fprintf(stderr, "takePicture error!\n");
        else{
          //LOGV("takePicture %x %d\n", imgBuf, imgLength);
          FILE* fp = fopen("test.jpeg", "wb");
          fwrite(imgBuf, 1, imgLength, fp);
          fclose(fp);
        }
      }
    }
    else{
      LOGE("take Picture fail!!!\n");
    }
  }
#endif
  m_timeSheetMgr->insert(ei->lab_no,ei->utype, imgBuf ,imgLength);

error:
  delete ei;
  LOGI("onData ---\n");
  printf("onData %d\n", __LINE__);
  m_bProcessingRfidData = false;
}
void MainDelegator::onSameData()
{
  LOGI("onSameData\n");
  m_el->onMessage("Msg", "Same Card");
}

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
  if(md->m_bProcessingRfidData){
    LOGV("cbTimer returned by processing card\n");
    return;
  }

  if(md->checkNetwork() && !md->m_bTimeAvailable)
    md->m_bTimeAvailable = md->getSeverTime();

  switch(count){
    case 3:
      break;

    case 1:
    case 6:
      if(md->m_sLocalIP == ""){
        try{
          char* p = network::GetIpAddress("eth0");  // or lo
          md->m_sLocalIP = p;
        }
        catch(Exception e)
        {
          md->m_sLocalIP = "";
        }
      }
      try{
        //ret = m_ws->request_StatusUpdate((m_sInOut == "I")?"IN":"OUT", m_sSiteCd, m_sDvLoc, m_sDvNo, m_sLocalIP, m_sLocalMacAddr, 8000);  //blocked I/O
        //LOGV("***StatusUpdate: %d\n", ret);
        md->m_ws->request_StatusUpdate((md->m_sInOut == "I")?"IN":"OUT", md->m_sSiteCd.c_str(), md->m_sDvLoc.c_str(), md->m_sDvNo.c_str(), md->m_sLocalIP.c_str()
          , md->m_sLocalMacAddr.c_str(), cbStatusUpdate, NULL);  //blocked I/O
      }
      catch(WebService::Except e){
        LOGE("request_StatusUpdate: %s\n", WebService::dump_error(e));
      }
      break;

    case 8:
      md->m_employInfoMrg->createLocalDB();
      break;
      
    case 9:
      count = -1;
      break;
    default:
      //upload timesheet
      md->m_timeSheetMgr->upload();
      break;
  }
  count++;
  
}

bool MainDelegator::checkNetwork()
{
  bool ret = false;
  try{
    ret = m_ws->request_GetNetInfo(2000);  //blocked I/O
  }
  catch(WebService::Except e){
    LOGE("request_GetNetInfo: %s\n", WebService::dump_error(e));
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
      LOGV("Server ON\n");
      m_el->onMessage("Server", "Server ON");
      m_yellowLed->off();
    }
    else{
      LOGV("Server OFF\n");
      m_el->onMessage("Server", "Server OFF");
      int arr[] = {1000, 1000, 0};
      m_yellowLed->on(arr, true);
    }
    m_bNetworkAvailable = val;
  }
  
  if(!bInitialized)
    bInitialized = true;
}

bool MainDelegator::SettingInit()
{
  m_settings = new Settings("/etc/acu/FID.ini");

#ifdef CAMERA  
  m_cameraDelayOffTime = m_settings->getInt("Camera::DELAY_OFF_TIME"); //600 sec
  m_takePictureMaxWaitTime = m_settings->getInt("Camera::TAKEPICTURE_MAX_WAIT_TIME"); // 2 sec
  m_bSavePictureFile = m_settings->getBool("Camera::SAVE_PICTURE_FILE");
#endif
  //App
  m_sAuthCd = m_settings->get("App::AUTH_CD");
  m_sMemcoCd = m_settings->get("App::MEMCO_CD");
  m_sSiteCd = m_settings->get("App::SITE_CD");
  m_sDvLoc = m_settings->get("App::DV_LOC"); // = "0001";
  m_sDvNo = m_settings->get("App::DV_NO"); // = "6";
  m_bDatabase = m_settings->getBool("App::LOCAL_DATABASE");
  m_sInOut = m_settings->get("App::IN_OUT");

  //Action
  m_bCapture = m_settings->getBool("Action::CAPTURE");
  //m_bRelay = m_settings->getBool("Action::RELAY");
  m_bSound = m_settings->getBool("Action::SOUND");

  //Rfid
  m_sRfidMode = m_settings->get("Rfid::MODE"); //="1356M";
  m_rfidCheckInterval = m_settings->getInt("Rfid::CHECK_INTERVAL"); //300 ms
  m_sRfid1356Port = m_settings->get("Rfid::RFID1356_PORT"); // /dev/ttyAMA0
  m_sRfid900Port = m_settings->get("Rfid::RFID800_PORT"); // /dev/ttyUSB0

  //Server
  m_sUrl = m_settings->get("Server::URL");

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

MainDelegator::MainDelegator(EventListener* el) : m_el(el), m_bProcessingRfidData(false)
{
  bool ret;
  cout << "start" << endl;
  el->onStatus("System Start");
  SettingInit();
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
#ifndef SIMULATOR
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
#endif  
  m_el->onMessage("Rfid", m_sRfidMode + " ON");

  m_el->onStatus("WebService Url:" + m_sUrl);
  //m_ws = new WebService("192.168.0.7", 8080);
  m_sServerURL = m_settings->get("Server::URL");
  LOGV("m_sServerURL= %s\n", m_sServerURL.c_str()); 
  char ip[21];
  strncpy(ip, m_sServerURL.c_str() + 7, 20);
  for(int i=0;i<21;i++){
    if(ip[i] == ':'){
      ip[i] = '\0';
      break;
    }
  }
  LOGV("Server IP: %s\n", ip);
  m_ws = new WebService(ip, 8080);
  //m_ws = new WebService(ip, 17552);
  checkNetwork();
  m_employInfoMrg = new EmployeeInfoMgr(m_settings, m_ws);
  m_timeSheetMgr = new TimeSheetMgr(m_settings, m_ws);

#ifdef CAMERA  
  m_cameraStill = new CameraStill(m_cameraDelayOffTime);
#endif

#ifdef SIMULATOR
  signal(SIGUSR1, test_signal_handler);
  signal(SIGUSR2, test_signal_handler);
  mTimerForTest = new Timer(cbTestTimer, this);
#endif
  string locationName = getLocationName();
  m_el->onMessage("GateLoc", locationName);
  m_el->onMessage("GateNo", "No." + m_sDvNo);
  
  m_wp = media::WavPlayer::createInstance(); 
  m_wp->play("SoundFiles/start.wav");

  m_timer = new Timer(cbTimer, this);
  int interval = m_settings->getInt("App::TIMER_INTERVAL");
  LOGI("timer interval= %d\n", interval);
  m_timer->start(interval, true);
  
  m_bTimeAvailable = getSeverTime();
  string reboot_time = m_settings->get("App::REBOOT_TIME");
  LOGV("reboot time= %s\n", reboot_time.c_str()); 
  if(reboot_time != "")
    setRebootTimer(reboot_time.c_str());
  LOGV("MainDelegator ---\n");
}

MainDelegator::~MainDelegator()
{
  m_timer->stop();
} 

#ifdef SIMULATOR
void MainDelegator::cbTestTimer(void* arg)
{
   my->onData(my->m_test_serial_number.c_str());
}

void MainDelegator::test_signal_handler(int signo)
{
  if(signo == SIGUSR1){
    LOGI("signal_handler SIGUSR1\n");
    //my->m_test_serial_number = "253161024009"; //validate
    my->m_test_serial_number = "253178087009"; //validate + Photo image
    my->mTimerForTest->start(1);
  }
  else if(signo == SIGUSR2){
    LOGI("signal_handler SIGUSR2\n");
    my->m_test_serial_number = "253153215009"; //invalidate
    my->mTimerForTest->start(1);
  }
}
#endif

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
  try{
    char* time_buf = m_ws->request_ServerTimeGet(3000);  //blocked I/O
    //time_buf = m_ws->request_ServerTimeGet(cbServerTimeGet, NULL);  //blocked I/O
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
  catch(WebService::Except e){
    LOGE("request_ServerTimeGet: %s\n", WebService::dump_error(e));
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
