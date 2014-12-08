#ifndef _TIMESHEETMGR_HEADER
#define _TIMESHEETMGR_HEADER

#include <iostream>
#include <list>
#include "tools/mutex.h"
#include "web/iwebservice.h"
#include "tools/datetime.h"

class Settings;
class TimeSheetMgr {
public:
  class TimeSheetMgrEventListener {
    virtual void onTimeSheetFileCountChanged(int count) = 0;
    virtual void onTimeSheetCacheCountChanged(int count) = 0;
  };
  struct TimeSheet {
    std::string m_pinno;
    char m_utype;
    tools::DateTime m_time;
    char* m_photo_img;
    int m_imgSz;
    TimeSheet(std::string pin_no);
    ~TimeSheet();
  };
  TimeSheetMgr(Settings* settings, web::IWebService* ws, TimeSheetMgrEventListener* el);
  virtual ~TimeSheetMgr();

  void insert(std::string lab_no);
  bool upload();
  
private:  
  //void dump();
  
  std::list<TimeSheet*> m_listTS;
  web::IWebService* m_ws;
  Settings* m_settings;
  std::string m_sMemcoCd; // = "MC00000003";
  std::string m_sSiteCd; //"ST00000005";
  std::string m_sDvLoc; // = "0001";
  std::string m_sDvNo; // = "1";
  char m_cInOut; // = "I";
  Mutex mtx;
  TimeSheetMgrEventListener* m_el;
};

#endif  //_TIMESHEETMGR_HEADER


  

