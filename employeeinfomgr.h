
#ifndef _EMPLOYEEINFOMGR_HEADER
#define _EMPLOYEEINFOMGR_HEADER

#include "tools/date.h"
#include "tools/datetime.h"
#include <vector>
#include <string>
#include "tools/mutex.h"
#include "web/iwebservice.h"
#include "sqlite3.h"
#include <map>

#define USERDATA_SIZE 864
class Settings;
//class web::IWebService;

class EmployeeInfoMgr {
public:
  enum Exception {
    EXCEPTION_USERDATA_SIZE,
  };
  struct EmployeeInfo {
    //display
    std::string company_name;
    std::string lab_name;
    std::string pin_no;
    unsigned char userdata[USERDATA_SIZE];
    std::string blacklistinfo;
    int pnt_cnt;
    //char usercode[17];
    
    EmployeeInfo():pnt_cnt(0){};
    ~EmployeeInfo(){}
  };
  
  class EmployeeInfoMgrListener {
  public:
    virtual void onEmployeeInfoTotal(int insert_count, int update_count, int delete_count) = 0;
    virtual void onEmployeeInfoInsert(const unsigned char* userdata, int index) = 0;
    virtual void onEmployeeInfoUpdate(string& usercode, const unsigned char* userdata, int index) = 0;
    virtual void onEmployeeInfoDelete(string& usercode, int index) = 0;
  };



  EmployeeInfoMgr(Settings* settings, web::IWebService* ws, EmployeeInfoMgrListener* eil);
  virtual ~EmployeeInfoMgr()
  {
    sqlite3_close(m_db);
  }

  void getEmployeeList(std::vector<pair<const char*, unsigned char*> >& arr_16, std::vector<pair<const char*, unsigned char*> >& arr_4);
  bool updateLocalDB(); //from Server
  bool getInfo(const char* serialNumber, EmployeeInfo** ei);
  //std::map<string, EmployeeInfo*>& getEmployeeList();
  
private:  
  bool OpenOrCreateLocalDB();
  void updateCache();
  void insertEmployee(vector<pair<string, EmployeeInfo*> >& elems);
  void updateEmployee(vector<pair<string, EmployeeInfo*> >& elems);
  void deleteEmployee(vector<pair<string, EmployeeInfo*> >& elems);
  void fillEmployeeInfoes(char *xml_buf);
  bool fillEmployeeInfo(char *xml_buf, EmployeeInfo* ei);
  bool search(string pin_no);
  void dump(map<string, EmployeeInfo*>& arr);

  bool m_bUseLocalDB;
  string m_sMemcoCd; // = "MC00000003";
  string m_sSiteCd; //"ST00000005";
  string m_sEmbedCd; //"0000000008";
  bool m_check_code;
  bool m_bDisplayPhoto;
  web::IWebService* m_ws;
  Settings* m_settings;

  map<string, EmployeeInfo*> m_arrEmployee;
  map<string, EmployeeInfo*> m_arrEmployee_4;
  string m_lastSyncTime;
  Mutex mtx;
  sqlite3 *m_db;
  EmployeeInfoMgrListener* m_eil;
};




#endif  //_EMPLOYEEINFOMGR_HEADER


  

