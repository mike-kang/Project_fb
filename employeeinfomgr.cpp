#include "tools/log.h"
#include "tools/filesystem.h"
#include "employeeinfomgr.h"
#include "web/webservice.h"
#include "settings.h"
#include <fstream>
#include "tools/utils.h"
#include "tools/base64.h"
#include "tools/datetime.h"

#define LOG_TAG "EmployInfoMgr"

using namespace tools;
using namespace std;
using namespace web;

//#define DB_FILE "employee.xml"
#define EMPLOYEE_FILE "employee.xml"
#define DB_NAME "employee.db"
#define CREATE_TABLE_EMPLOYEE "create table employee( company TEXT, name TEXT, pinno INTEGER, usercode TEXT, userdata BLOB, blacklistinfo TEXT, pntcnt INTEGER)"
#define CREATE_TABLE_TIME "create table time( lastsync TEXT );" \
                          "insert into time(lastsync) VALUES('')"
#define INSERT_EMPLOYEE "insert into employee(company, name, pinno, usercode, userdata, blacklistinfo, pntcnt) values (?,?,?,?,?,?,?)"
#define UPDATE_EMPLOYEE "update employee set company=?, name=?, usercode=?, userdata=?, blacklistinfo=?, pntcnt=? where pinno=?"
#define GET_ALL "select * from employee"
#define DELETE_EMPLOYEE "delete from employee where pinno=?"

EmployeeInfoMgr::EmployeeInfoMgr(Settings* settings, web::IWebService* ws, EmployeeInfoMgrListener* eil): m_settings(settings), m_ws(ws), m_eil(eil), m_bUpdateThreadRunning(false)
{
  bool db_ok;
  try{
    //m_bUseLocalDB = settings->getBool("App::LOCAL_DATABASE");
    m_sMemcoCd = m_settings->get("App::MEMCO");
    m_sSiteCd = m_settings->get("App::SITE");
    m_sEmbedCd = m_settings->get("App::EMBED");
    m_check_code = m_settings->getBool("FB::CHECK_CODE_4");
  }
  catch(int e)
  {
    //m_bUseLocalDB = false;
    m_sMemcoCd = "MC00000003";
    m_sSiteCd = "ST00000005";
    m_sEmbedCd = "0000000008";
    m_check_code = false;
  }
  for(int i=0;i<3;i++){
    if(db_ok = OpenOrCreateLocalDB())
      break;
  }
  if(!db_ok)
    throw EXCEPTION_DB;
  
}

bool EmployeeInfoMgr::OpenOrCreateLocalDB()
{
  char* err;

  LOGV("OpenOrCreateLocalDB\n");
  
  int rc = sqlite3_open(DB_NAME,&m_db) ;
  if (rc != SQLITE_OK) {
    printf("Failed to open database : %s\n", sqlite3_errmsg(m_db));
    return false;
  }
  printf("Opened db %s OK!\n", DB_NAME);
  
  int db_size = filesystem::file_size(DB_NAME);
  printf("db size: %d\n", db_size);
  
  if(db_size){
    if(!checkValidate()){
      LOGE("checkValidate fail!\n");
      sqlite3_close(m_db);
      filesystem::file_delete(DB_NAME);
      return false;
    }
  }
  
  if(!db_size){
    printf("create table\n");
    rc = sqlite3_exec(m_db, CREATE_TABLE_EMPLOYEE, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
      printf("Failed to creat table : %s\n", err);
      return false;
    }
    rc = sqlite3_exec(m_db, CREATE_TABLE_TIME, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
      printf("Failed to creat table : %s\n", err);
      return false;
    }
  }
  else{
    initCache();
  }
  
  return true;
}

// 
bool EmployeeInfoMgr::updateLocalDBfromServer()
{
  LOGV("updateLocalDBfromServer\n");

  if(m_bUpdateThreadRunning){
    LOGE("Update thread is running\n");
    return false;
  }
  
  try{
    m_ws->request_EmployeeInfoAll(m_lastSyncTime.c_str(), 8000, EMPLOYEE_FILE);
    LOGV("downloaded %s\n", EMPLOYEE_FILE);
  }
  catch(web::Except e){
    LOGE("download %s fail\n", EMPLOYEE_FILE);
    return false;
  }
  
  DateTime dt;
  m_lastSyncTime = dt.toString('+');

  ifstream infile (EMPLOYEE_FILE, ofstream::binary);
  // get size of file
  infile.seekg (0,infile.end);
  long size = infile.tellg();
  infile.seekg (0);
  // allocate memory for file content
  m_xml_buf = new char[size + 1];
  // read content of infile
  infile.read (m_xml_buf, size);
  m_xml_buf[size] = '\0';
  infile.close();
  
  //updateLocalDB(xml_buf);
  m_thread_update = new Thread<EmployeeInfoMgr>(&EmployeeInfoMgr::run_updateLocalDB, this, "UpdateThread");
  m_thread_update->detach();

  return true;

}



bool EmployeeInfoMgr::getInfo(const char* usercode, EmployeeInfo** ei)
{
  LOGI("Local DB check!\n");
  map<string, EmployeeInfo*>::iterator itr = m_arrEmployee.find(usercode);
  if(itr == m_arrEmployee.end())
    return false;
    
  *ei = itr->second;
  return true;
    
}

bool EmployeeInfoMgr::checkValidate()
{
  char* err;
  sqlite3_stmt *stmt;
  const char* tail;
  string lastSyncTime;

  int rc = sqlite3_prepare(m_db, "select lastsync from time", -1, &stmt, &tail);
  if (rc != SQLITE_OK) {
    printf("Failed to read: %s\n", err);
  }
  if(sqlite3_step(stmt) == SQLITE_ROW){
    lastSyncTime = (const char*)sqlite3_column_text(stmt , 0);
  }
  sqlite3_finalize(stmt);
  return lastSyncTime != "";
}

void EmployeeInfoMgr::initCache()
{
  char* err;
  sqlite3_stmt *stmt;
  const char* tail;
  
  LOGV("initCache\n");
  
  int rc = sqlite3_prepare_v2(m_db, GET_ALL, -1, &stmt, &tail);
  if (rc != SQLITE_OK) {
    LOGE("Failed to read: %s\n", err);
  }
  string usercode;
  while(sqlite3_step(stmt) == SQLITE_ROW){
    EmployeeInfo* ei = new EmployeeInfo();
    ei->company_name = (const char*)sqlite3_column_text(stmt , 0);
    ei->lab_name = (const char*)sqlite3_column_text(stmt , 1);
    ei->pin_no = (const char*)sqlite3_column_text(stmt , 2);
    usercode = (const char*)sqlite3_column_text(stmt , 3);
    memcpy(ei->userdata, sqlite3_column_blob(stmt,4), USERDATA_SIZE);
    ei->blacklistinfo = (const char*)sqlite3_column_text(stmt , 5);
    ei->pnt_cnt = sqlite3_column_int(stmt , 6);
    
    if(usercode.length() == 16)
      m_arrEmployee.insert(pair<string, EmployeeInfo*>(usercode, ei));
    else
      m_arrEmployee_4.insert(pair<string, EmployeeInfo*>(usercode, ei));
  }
  sqlite3_finalize(stmt);
  //dump(m_arrEmployee);
  m_eil->onEmployeeCountChanged(m_arrEmployee.size(), m_arrEmployee_4.size());

  //m_lastSyncTime
  rc = sqlite3_prepare(m_db, "select lastsync from time", -1, &stmt, &tail);
  if (rc != SQLITE_OK) {
    printf("Failed to read: %s\n", err);
  }
  if(sqlite3_step(stmt) == SQLITE_ROW){
    m_lastSyncTime = (const char*)sqlite3_column_text(stmt , 0);
    printf("lastsync:%s\n", m_lastSyncTime.c_str());
  }
  sqlite3_finalize(stmt);

}

/*
void EmployeeInfoMgr::updateLocalDB(char *xml_buf)
{
  LOGV("updateLocalDB +++\n");

}
*/

void EmployeeInfoMgr::run_updateLocalDB()
{
  LOGV("run_updateLocalDB\n");
  m_bUpdateThreadRunning = true;

  m_eil->onEmployeeMgrUpdateStart();
  
  char *p = m_xml_buf;
  char *start, *end;
  vector<pair<string, EmployeeInfo*> > arrEmployeeInsert;
  vector<pair<string, EmployeeInfo*> > arrEmployeeDelete;
  vector<pair<string, EmployeeInfo*> > arrEmployeeUpdate;
  //cout << xml_buf << endl;
  //LOGV("***fillEmployeeInfo:%s\n", xml_buf);
  //mtx.lock();
  
  string usercode;
  string status;
  while(p = strstr(p, "<Table")){
    //printf("start %x\n", p);
    end = strstr(p+7, "</Table");
    *end = '\0';
    EmployeeInfo* ei = new EmployeeInfo;
    start = p + 6;
    
    try {
      ei->lab_name = p = utils::getElementData(start, "LAB_NM");
      *p = '\0'; //for COMPANY element
      p += ei->lab_name.length() + 1;
      //cout << ei->lab_name << endl;
    }
    catch(int e){}

    try {
      ei->company_name = utils::getElementData(start, "COMPANY");
    }
    catch(int e){}

    try {
      ei->pin_no = utils::getElementData(p, "PIN_NO");
      p += strlen(p) + 1;
      //cout << ei->pin_no << endl;
    }
    catch(int e){}
    try {
      status = p = utils::getElementData(p, "STATUS");
      p += strlen(p) + 1;
    }
    catch(int e){}

    try {
      usercode = utils::getElementData(p, "USERCODE");
      p += strlen(p) + 1;
    }
    catch(int e){}

    if(status != "D"){
      try {
        p = utils::getElementData(p, "USER_DATA");  //base64 encoded
        int length = strlen(p);
        int size = 0;
        base64::base64d(p, length, (char*)(ei->userdata), &size);
        if(size != USERDATA_SIZE){
          LOGE("userdata size: %d\n", size);
          throw EXCEPTION_USERDATA_SIZE;
        }
        p += length + 1;
      }
      catch(int e){}
      try {
        ei->blacklistinfo = utils::getElementData(p, "BLACK_LIST");
        p += strlen(p) + 1;
      }
      catch(int e){}
      try {
        p = utils::getElementData(p, "PNT_CNT");
        ei->pnt_cnt = atoi(p);
        p += strlen(p) + 1;
      }
      catch(int e){}
    }
      
    if(status == "I"){
      LOGV("insert PIN_NO: %s\n", ei->pin_no.c_str());
      arrEmployeeInsert.push_back(pair<string,EmployeeInfo*>(usercode, ei));
    }
    else if(status == "D"){
      LOGV("delete PIN_NO: %s\n", ei->pin_no.c_str());
      arrEmployeeDelete.push_back(pair<string,EmployeeInfo*>(usercode, ei));
    }
    else if(status == "U"){
      LOGV("update PIN_NO: %s\n", ei->pin_no.c_str());
      if(search(ei->pin_no))
        arrEmployeeUpdate.push_back(pair<string,EmployeeInfo*>(usercode, ei));
      else
        arrEmployeeInsert.push_back(pair<string,EmployeeInfo*>(usercode, ei));
    }
    p = end + 8;
  }

  int insert_count = arrEmployeeInsert.size();
  int update_count = arrEmployeeUpdate.size();
  int delete_count = arrEmployeeDelete.size();

  m_eil->onEmployeeMgrUpdateCount(delete_count, update_count, insert_count);

  if(delete_count)
    deleteEmployee(arrEmployeeDelete);
  if(update_count)
    updateEmployee(arrEmployeeUpdate);
  if(insert_count)
    insertEmployee(arrEmployeeInsert);

  //mtx.unlock();
   //cout << "members = " << num << endl;
  delete m_xml_buf;
  m_xml_buf = NULL;
  
  //update lastsync time
  char* err;
  char buf[100];

  sprintf(buf, "update time set lastsync = '%s'", m_lastSyncTime.c_str());
  int rc = sqlite3_exec(m_db, buf, NULL, NULL, &err);
  if (rc != SQLITE_OK) {
    LOGE("Failed to update : %s\n", err);
  }
  m_eil->onEmployeeMgrUpdateEnd(m_lastSyncTime.c_str());
  m_eil->onEmployeeCountChanged(m_arrEmployee.size(), m_arrEmployee_4.size());   
  m_bUpdateThreadRunning = false;
  LOGV("updateLocalDB ---\n");
    
}

//cache & DB
void EmployeeInfoMgr::insertEmployee(vector<pair<string, EmployeeInfo*> >& elems)
{
  int rc;
  sqlite3_stmt *stmt;
  const char* tail;
  int index = 0;
  LOGV("insertEmployee size=%d\n", elems.size());
  rc = sqlite3_prepare_v2(m_db, INSERT_EMPLOYEE, -1, &stmt, &tail);
  mtx.lock();

  for(vector<pair<string,EmployeeInfo*> >::size_type i=0; i< elems.size(); i++){
    //cache
    if(elems[i].first.length() == 16)
      m_arrEmployee.insert(elems[i]);
    else
      m_arrEmployee_4.insert(elems[i]);
    
    //db
    string& usercode = elems[i].first;
    EmployeeInfo* ei = elems[i].second;
    sqlite3_bind_text(stmt, 1, ei->company_name.c_str(), ei->company_name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ei->lab_name.c_str(), ei->lab_name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ei->pin_no.c_str(), ei->pin_no.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, usercode.c_str(), usercode.length(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 5, ei->userdata, USERDATA_SIZE, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, ei->blacklistinfo.c_str(), ei->blacklistinfo.length(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, ei->pnt_cnt);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
      LOGE("Failed to insert : %s\n", sqlite3_errmsg(m_db));
      sqlite3_finalize(stmt);
      mtx.unlock();
      throw 1;
    }
    sqlite3_reset(stmt);
//debug
//    ofstream oOut2(usercode.c_str(), ofstream::binary);
//    oOut2.write((const char*)ei->userdata, USERDATA_SIZE);
//    oOut2.close();
//end debug
    if(!m_check_code || usercode.length() != 4)
      m_eil->onEmployeeMgrUpdateInsert(ei->userdata, index++);
  }
  mtx.unlock();
  sqlite3_finalize(stmt);

}

//cache & DB
void EmployeeInfoMgr::updateEmployee(vector<pair<string, EmployeeInfo*> >& elems)
{
  int rc;
  int index = 0;

  sqlite3_stmt *stmt;
  const char* tail;
  LOGV("updateEmployee size=%d\n", elems.size());
  rc = sqlite3_prepare_v2(m_db, UPDATE_EMPLOYEE, -1, &stmt, &tail);
  mtx.lock();

  for(vector<pair<string,EmployeeInfo*> >::size_type i=0; i< elems.size(); i++){
    
    string& usercode = elems[i].first;
    EmployeeInfo* ei = elems[i].second;
    
    //cache
    if(usercode.length() == 16){
      delete m_arrEmployee[usercode];
      m_arrEmployee[usercode] = ei;
    }  
    else{
      delete m_arrEmployee_4[usercode];
      m_arrEmployee_4[usercode] = ei;
    }

    //db
    sqlite3_bind_text(stmt, 1, ei->company_name.c_str(), ei->company_name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ei->lab_name.c_str(), ei->lab_name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, usercode.c_str(), usercode.length(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 4, ei->userdata, USERDATA_SIZE, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, ei->blacklistinfo.c_str(), ei->blacklistinfo.length(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, ei->pnt_cnt);
    sqlite3_bind_text(stmt, 7, ei->pin_no.c_str(), ei->pin_no.length(), SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
      LOGE("Failed to insert : %s\n", sqlite3_errmsg(m_db));
      sqlite3_finalize(stmt);
      mtx.unlock();
      throw 1;
    }
    sqlite3_reset(stmt);
    
    if(!m_check_code || usercode.length() != 4)
      m_eil->onEmployeeMgrUpdateUpdate(usercode, ei->userdata, index++);
  }
  mtx.unlock();
  sqlite3_finalize(stmt);

}

//cache & DB
void EmployeeInfoMgr::deleteEmployee(vector<pair<string, EmployeeInfo*> >& elems)
{
  int rc;
  int index = 0;

  sqlite3_stmt *stmt;
  const char* tail;
  LOGV("deleteEmployee size=%d\n", elems.size());
  rc = sqlite3_prepare_v2(m_db, DELETE_EMPLOYEE, -1, &stmt, &tail);
  //printf("delete size:%d\n", arrEmployeeDelete.size());
  mtx.lock();
  for(vector<pair<string,EmployeeInfo*> >::size_type i=0; i< elems.size(); i++){
    string& usercode = elems[i].first;
    EmployeeInfo* ei = elems[i].second;

    //cache
    if(usercode.length() == 16){
      delete m_arrEmployee[usercode];
      m_arrEmployee_4.erase(usercode);
    }  
    else{
      delete m_arrEmployee_4[usercode];
      m_arrEmployee_4.erase(usercode);
    }
    
    sqlite3_bind_text(stmt, 1, ei->pin_no.c_str(), ei->pin_no.length(), SQLITE_STATIC);
    //printf("pin %s\n", ei->pin_no.c_str());
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
      LOGE("Failed to insert : %s\n", sqlite3_errmsg(m_db));
      sqlite3_finalize(stmt);
      mtx.unlock();
      throw 1;
    }
    sqlite3_reset(stmt);
    m_eil->onEmployeeMgrUpdateDelete(usercode, index++);
    delete ei;
  }
  mtx.unlock();
  sqlite3_finalize(stmt);
}

void EmployeeInfoMgr::getEmployeeList(std::vector<pair<const char*, unsigned char*> >& arr_16, std::vector<pair<const char*, unsigned char*> >& arr_4)
{
  LOGV("getEmployeeList size: %d\n", m_arrEmployee.size());
  mtx.lock();
  arr_16.reserve(m_arrEmployee.size());
  for(map<string, EmployeeInfo*>::iterator itr=m_arrEmployee.begin(); itr != m_arrEmployee.end(); itr++){
    EmployeeInfo* ei = itr->second;
    //LOGV("usercode size: %d\n", itr->first.length());
    arr_16.push_back(pair<const char*, unsigned char*>(itr->first.c_str(), ei->userdata));
  }
  
  for(map<string, EmployeeInfo*>::iterator itr=m_arrEmployee_4.begin(); itr != m_arrEmployee_4.end(); itr++){
    EmployeeInfo* ei = itr->second;
    //LOGV("usercode size: %d\n", itr->first.length());
    arr_4.push_back(pair<const char*, unsigned char*>(itr->first.c_str(), ei->userdata));
  }
  mtx.unlock();
}

void EmployeeInfoMgr::getEmployeeCount(int& count_16, int& count_4)
{
  mtx.lock();
  count_16 = m_arrEmployee.size();
  count_4 = m_arrEmployee_4.size();
  mtx.unlock();
}

/*
map<string, EmployeeInfo*>& EmployeeInfoMgr::getEmployeeList()
{
  return m_arrEmployee;
}
*/

bool EmployeeInfoMgr::search(string pin_no)
{
  map<string, EmployeeInfo*>::iterator itr;
  EmployeeInfo* ei;
  for(itr=m_arrEmployee.begin(); itr != m_arrEmployee.end(); itr++){
    ei = itr->second;
    if(ei->pin_no == pin_no)
      return true;
  }
  for(itr = m_arrEmployee_4.begin(); itr != m_arrEmployee_4.end(); itr++){
    ei = itr->second;
    if(ei->pin_no == pin_no)
      return true;
  }
  return false;
}

void EmployeeInfoMgr::dump(map<string, EmployeeInfo*>& arr)
{
  printf("size %d\n", (int)arr.size());
  for(map<string, EmployeeInfo*>::iterator itr=arr.begin(); itr != arr.end(); itr++){
    EmployeeInfo* ei = itr->second;
    printf("company: %s\n", ei->company_name.c_str());
    printf("name :%s\n", ei->lab_name.c_str());
    printf("pinno :%s\n", ei->pin_no.c_str());
    printf("usercode :%s\n", itr->first.c_str());
    utils::hexdump("userdata", ei->userdata, USERDATA_SIZE);
  }
}


