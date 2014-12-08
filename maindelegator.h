#ifndef _MAINDELEGATOR_HEADER
#define _MAINDELEGATOR_HEADER

#include "tools/queue.h"
#include "tools/thread.h"
#include "tools/event.h"
#include "serialRfid.h"
#include "fingerblood/fbservice.h"

#include "web/iwebservice.h"
#ifdef CAMERA
#include "camera/camerastill.h"
#endif
#include "hardware/switchgpio.h"
#include "tools/date.h"
#include "settings.h"
#include "tools/timer.h"
#include "tools/wavplayer.h"
#include "employeeinfomgr.h"
#include "timesheetmgr.h"

//class TimeSheetMgr;
class MainDelegator : public EmployeeInfoMgr::EmployeeInfoMgrListener, public 
IFBService::IFBServiceEventListener, TimeSheetMgr::TimeSheetMgrEventListener {
public:
  enum Exception {
    EXCEPTION_RFID_OPEN_FAIL,
  };
  class EventListener {
  public:
    //virtual void onRFSerialNumber(char* serial) = 0;
    virtual void onSyncStart() = 0;
    virtual void onSyncCount(int count) = 0;
    virtual void onSyncIndex(int index) = 0;
    virtual void onSyncEnd(bool val) = 0;
    virtual void onUpdateStart() = 0;
    virtual void onUpdateCount(int delete_count, int update_count, int insert_count) = 0;
    virtual void onUpdateDeleteIndex(int index) = 0;
    virtual void onUpdateUpdateIndex(int index) = 0;
    virtual void onUpdateInsertIndex(int index) = 0;
    virtual void onUpdateEnd() = 0;
    

    virtual void onMessage(std::string tag, std::string data) = 0; //for lable
    virtual void onLogo(std::string data) = 0;
    virtual void onEmployeeInfo(std::string CoName, std::string Name, std::string PinNo) = 0;
    virtual void onStatus(std::string status) = 0;
    virtual void onImage(bool) = 0;
  };
  enum Ret {
    RET_SUCCESS,
  };
  enum AuthMode {
    AM_NORMAL,
    AM_PASS_NOREGISTOR,
    AM_PASS_THREEOUT,
  };
  virtual void onScanData(const char* buf);
  virtual bool onNeedDeviceKey(char* id, char* key);
  virtual void onNeedUserCodeList(std::vector<pair<const char*, unsigned char*> >& arr_16, std::vector<pair<const char*, unsigned char*> >& arr_4);
  virtual void onSync(IFBService::IFBServiceEventListener::SyncStatus status, int index);
  //virtual std::map<string, EmployeeInfo*>& onNeedUserCodeList();

  virtual void onEmployeeMgrUpdateStart();
  virtual void onEmployeeMgrUpdateCount(int delete_count, int update_count, int insert_count);
  virtual void onEmployeeMgrUpdateInsert(const unsigned char* userdata, int index);
  virtual void onEmployeeMgrUpdateUpdate(string& usercode, const unsigned char* userdata, int index);
  virtual void onEmployeeMgrUpdateDelete(string& usercode, int index);
  virtual void onEmployeeMgrUpdateEnd();
  virtual void onEmployeeCountChanged(int length_16, int length_4);

  virtual void onTimeSheetFileCountChanged(int count);
  virtual void onTimeSheetCacheCountChanged(int count);


  static MainDelegator* createInstance(EventListener* el);
  ~MainDelegator(); 


//request
  //bool request_processRfidSerialData(char* serialnum, int timelimit);
  void setEventListener(EventListener* el){ m_el = el; }

private:
  static MainDelegator* my;
  MainDelegator(EventListener* el);
  void run(); 

  //void _processRfidSerialData(void* arg);
#ifdef LOCATION
  string getLocationName();
#endif
  bool SettingInit();
  bool checkValidate(EmployeeInfoMgr::EmployeeInfo* ei, string& msg);
#ifdef LOCATION
  bool checkZone(string& sAuth);
#endif
  bool checkDate(Date* start, Date* end, string& msg);
  bool checkNetwork();
  void displayNetworkStatus(bool val);
  bool getSeverTime();
  void setRebootTimer(const char* time_buf);
  void checkAndRunFBService();
//#ifdef SIMULATOR  
  static void cbTestTimer(void* arg);
  static void test_signal_handler(int signo);
  tools::Timer* mTimerForTest;
  std::string m_test_serial_number;
//#endif
  static void cbStatusUpdate(void *client_data, int status, void* ret);
  static void cbTimer(void* arg);
  static void cbTimerCheckFBService(void* arg);
  //static void cb_ServerTimeGet(void* arg);
  //void _cb_ServerTimeGet(void* arg);
  static void cbRebootTimer(void* arg);
  const char* debug_str(AuthMode m);
  void processAuthResult(bool result, const char* sound_path, string msg);


  Thread<MainDelegator> *m_thread;
  unsigned long m_threadId;
  TEvent<MainDelegator>* m_event;
  tools::Queue<TEvent< MainDelegator> > m_eventQ;
  Condition m_rfid_process_completed;
  Mutex m_rfid_mtx;
  web::IWebService* m_ws;
  //web::IWebService* m_subws;
  SerialRfid* m_serialRfid;  
  IFBService* m_fbs;
  bool m_bFBServiceRunning;
  Settings* m_settings;
  EmployeeInfoMgr* m_employInfoMgr;
  TimeSheetMgr* m_timeSheetMgr;
  tools::Timer* m_timer;  //check network, upload status, download db, upload timesheet
  tools::media::WavPlayer* m_wp;
  EventListener* m_el;
  tools::Timer* m_RebootTimer;
  tools::Timer* m_timer_checkFBSerivce;


  bool m_bFirstDown;
  bool m_bNetworkAvailable;

  //settings
  //string sLog = "Y";
  bool m_bCapture;
  bool m_bRelay;
  bool m_bSound; //true
  bool m_bDatabase;
  bool m_check_code;

  //server
  string m_sServerType; 
  string m_sSafeIdServerUrl; 
  string m_sLotteIdServerUrl; 
  string m_sDWServerUrl; 
  
  //string m_sAuthCd; 
  string m_sMemcoCd; // = "MC00000003";
  string m_sSiteCd; //"ST00000005";
  string m_sEmbedCd; //"0000000008";
  //string m_sDvLoc; // = "0001";
  string m_sDvNo; // = "1";
  string m_sInOut; // = "I";
  bool m_bCheck;
  string m_sAuthCode;
  int m_rfidCheckInterval; //ms
  int m_rfid_processMaxTime; //ms
  string m_sRfidMode; //="1356M";
  string m_sRfid1356Port; // /dev/ttyAMA0
  string m_sRfid900Port; // /dev/ttyUSB0
  string m_admin1;
  string m_admin2;
  string m_admin3;
  string m_admin4;
  
  int m_fbCheckInterval; //ms
  string m_fbPort; // /dev/ttyUSB0
#ifdef CAMERA  
  CameraStill* m_cameraStill;
  int m_takePictureMaxWaitTime; //sec
  int m_cameraDelayOffTime; //sec
  bool m_bSavePictureFile;
#endif
  string m_sLocalIP;
  string m_sLocalMacAddr;
  //string m_sServerURL;
  //string m_consolePath;
  
  bool m_bProcessingAuth;
  bool m_bTimeAvailable;
  bool m_bSyncDeviceAndModule;
  bool m_bTestSignal; //for debug
  AuthMode m_authMode;
  bool m_bDisplayEmployeeInfo;
  bool m_bCheckUsercode4;
  //Led
  SwitchGpio* m_yellowLed;
  SwitchGpio* m_blueLed;
  SwitchGpio* m_greenLed;
  SwitchGpio* m_redLed;
  //Relay
  SwitchGpio* m_Relay;
};




#endif

