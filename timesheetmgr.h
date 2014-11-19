#ifndef _TIMESHEETMGR_HEADER
#define _TIMESHEETMGR_HEADER

#include <iostream>
#include <list>
#include "tools/mutex.h"

class WebService;
class Settings;
class TimeSheetMgr {
public:
  struct TimeSheet {
    std::string m_lab_no;
    char m_utype;
    std::string m_time;
    char* m_photo_img;
    int m_imgSz;
    TimeSheet(std::string lab_no, char utype, char* img, int img_sz);
    ~TimeSheet();
  };
  TimeSheetMgr(Settings* settings, WebService* ws);
  virtual ~TimeSheetMgr();

  void insert(std::string lab_no, char utype, char* img, int img_sz);
  bool upload();
  
private:  
  //void dump();
  
  std::list<TimeSheet*> m_listTS;
  WebService* m_ws;
  Settings* m_settings;
  std::string m_sMemcoCd; // = "MC00000003";
  std::string m_sSiteCd; //"ST00000005";
  std::string m_sDvLoc; // = "0001";
  std::string m_sDvNo; // = "1";
  char m_cInOut; // = "I";
  Mutex mtx;
};

#endif  //_TIMESHEETMGR_HEADER


  

