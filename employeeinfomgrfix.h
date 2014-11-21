
#ifndef _EMPLOYEEINFOMGRFIX_HEADER
#define _EMPLOYEEINFOMGRFIX_HEADER

#include "tools/date.h"
#include "tools/datetime.h"
#include <vector>
#include <string>
#include "tools/mutex.h"
#include "web/iwebservice.h"

class Settings;
//class web::IWebService;

class EmployeeInfoFixMgr {
public:
  struct EmployeeInfo {
    char serial_number[16]; //rfcard
    std::string in_out_gb;
    char utype;
    tools::Date* ent_co_ymd;
    tools::Date* rtr_co_ymd;
    std::string zone_code;
    //display
    std::string company_name;
    std::string lab_no;
    std::string lab_name;
    std::string pin_no;
    unsigned char* img_buf;
    int img_size;
    
    EmployeeInfo(): ent_co_ymd(NULL), rtr_co_ymd(NULL), img_buf(NULL){}
    ~EmployeeInfo(){
      if(ent_co_ymd) delete ent_co_ymd;
      if(rtr_co_ymd) delete rtr_co_ymd;
      //if(img_buf) delete img_buf;
    }
  };
  EmployeeInfoFixMgr(Settings* settings, web::IWebService* ws);
  virtual ~EmployeeInfoFixMgr(){}

  bool createLocalDB();
  bool getInfo(const char* serialNumber, EmployeeInfo* ei);
  
private:  
  int fillEmployeeInfoes(char *xml_buf, vector<EmployeeInfo*>& elems);
  bool fillEmployeeInfo(char *xml_buf, EmployeeInfo* ei);
  EmployeeInfo* searchDB(const char* serialNumber);

  bool m_bUseLocalDB;
  string m_sMemcoCd; // = "MC00000003";
  string m_sSiteCd; //"ST00000005";
  bool m_bDisplayPhoto;
  web::IWebService* m_ws;
  Settings* m_settings;
  std::vector<EmployeeInfo*> m_vectorEmployeeInfo;
  tools::DateTime m_lastSyncTime;
  Mutex mtx;
};




#endif  //_EMPLOYEEINFOMGR_HEADER


  

