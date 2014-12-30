
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
#include "tools/thread.h"

#define USERDATA_SIZE 864
class Settings;
//class web::IWebService;

class EmployeeInfoMgr {
public:
  enum Exception {
    EXCEPTION_USERDATA_SIZE,
    EXCEPTION_DB,
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
    virtual void onEmployeeMgrUpdateStart() = 0;
    virtual void onEmployeeMgrUpdateCount(int delete_count, int update_count, int insert_count) = 0;
    //virtual void onEmployeeMgrUpdateInsert(const unsigned char* userdata, int index) = 0;
    //virtual void onEmployeeMgrUpdateUpdate(string& usercode, const unsigned char* userdata, int index) = 0;
    //virtual void onEmployeeMgrUpdateDelete(string& usercode, int index) = 0;
    virtual void onEmployeeMgrUpdateEnd(const char* updatetime, vector<unsigned char*>* arrSave, vector<string>* arrDelete) = 0;
    virtual void onEmployeeCountChanged(int length_16, int length_4) = 0;
  };



  EmployeeInfoMgr(Settings* settings, web::IWebService* ws, EmployeeInfoMgrListener* eil);
  virtual ~EmployeeInfoMgr()
  {
    sqlite3_close(m_db);
  }

  void setServer(web::IWebService* ws);
  void getEmployeeList(std::vector<pair<const char*, unsigned char*> >& arr_16, std::vector<pair<const char*, unsigned char*> >& arr_4);
  void getEmployeeCount(int& count_16, int& count_4);
  bool updateLocalDBfromServer(); //from Server
  bool getInfo(const char* serialNumber, EmployeeInfo** ei);
  //std::map<string, EmployeeInfo*>& getEmployeeList();
  
private:  
  bool OpenOrCreateLocalDB();
  bool checkValidate();
  void initCache();
  void insertEmployee(vector<pair<string, EmployeeInfo*> >& elems);
  void updateEmployee(vector<pair<string, EmployeeInfo*> >& elems);
  void deleteEmployee(vector<pair<string, EmployeeInfo*> >& elems);
  //void updateLocalDB(char *xml_buf);
  void run_updateLocalDB();
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
  //string m_lastSyncTime_tmp;
  string m_lastSyncTime;
  Mutex mtx;
  sqlite3 *m_db;
  EmployeeInfoMgrListener* m_eil;
  Thread<EmployeeInfoMgr>* m_thread_update;
  char* m_xml_buf;
  bool m_bUpdateThreadRunning;
  
  vector<unsigned char*> m_arrUpdateUserCode; //insert or update
  vector<string> m_arrUpdateUserCodeDelete; 
};




#endif  //_EMPLOYEEINFOMGR_HEADER


  

