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

EmployeeInfoMgr::EmployeeInfoMgr(Settings* settings, web::IWebService* ws, EmployeeInfoMgrListener* eil): m_settings(settings), m_ws(ws), m_eil(eil)
{
  try{
    //m_bUseLocalDB = settings->getBool("App::LOCAL_DATABASE");
    m_sMemcoCd = m_settings->get("App::MEMCO");
    m_sSiteCd = m_settings->get("App::SITE");
    m_sEmbedCd = m_settings->get("App::EMBED");
  }
  catch(int e)
  {
    //m_bUseLocalDB = false;
    m_sMemcoCd = "MC00000003";
    m_sSiteCd = "ST00000005";
    m_sEmbedCd = "0000000008";
  }
  OpenOrCreateLocalDB();
  
}

bool EmployeeInfoMgr::OpenOrCreateLocalDB()
{
  char* err;

  int rc = sqlite3_open(DB_NAME,&m_db) ;
  if (rc != SQLITE_OK) {
    printf("Failed to open database : %s\n", sqlite3_errmsg(m_db));
    return false;
  }
  printf("Opened db %s OK!\n", DB_NAME);
  
  int db_size = filesystem::file_size(DB_NAME);
  printf("db size: %d\n", db_size);
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
    updateCache();
  }
  
  return true;
}


bool EmployeeInfoMgr::updateLocalDB()
{
  try{
    m_ws->request_EmployeeInfoAll(m_lastSyncTime.c_str(), 8000, EMPLOYEE_FILE);
    LOGV("downloaded %s\n", EMPLOYEE_FILE);
  }
  catch(web::Except e){
    LOGE("download %s fail\n", EMPLOYEE_FILE);
    return false;
  }

  ifstream infile (EMPLOYEE_FILE, ofstream::binary);
  // get size of file
  infile.seekg (0,infile.end);
  long size = infile.tellg();
  infile.seekg (0);
  // allocate memory for file content
  char* xml_buf = new char[size];
  // read content of infile
  infile.read (xml_buf,size);
  infile.close();
  fillEmployeeInfoes(xml_buf);
  //cout << "members = " << num << endl;
  delete xml_buf;

  //update lastsync time
  char* err;
  char buf[100];
  DateTime dt;
  m_lastSyncTime = dt.toString('+');
  sprintf(buf, "update time set lastsync = '%s'", m_lastSyncTime.c_str());
  int rc = sqlite3_exec(m_db, buf, NULL, NULL, &err);
  if (rc != SQLITE_OK) {
    LOGE("Failed to update : %s\n", err);
  }
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

void EmployeeInfoMgr::updateCache()
{
  char* err;
  sqlite3_stmt *stmt;
  const char* tail;
  
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
    m_arrEmployee.insert(pair<string, EmployeeInfo*>(usercode, ei));
  }
  sqlite3_finalize(stmt);
  //dump(m_arrEmployee);

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

void EmployeeInfoMgr::fillEmployeeInfoes(char *xml_buf)
{
  char *p = xml_buf;
  char *start, *end;
  vector<pair<string, EmployeeInfo*> > arrEmployeeInsert;
  vector<pair<string, EmployeeInfo*> > arrEmployeeDelete;
  vector<pair<string, EmployeeInfo*> > arrEmployeeUpdate;
  //cout << xml_buf << endl;
  //LOGV("***fillEmployeeInfo:%s\n", xml_buf);
  mtx.lock();
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
      cout << ei->lab_name << endl;
    }
    catch(int e){}

    try {
      ei->company_name = utils::getElementData(start, "COMPANY");
    }
    catch(int e){}

    try {
      ei->pin_no = utils::getElementData(p, "PIN_NO");
      p += strlen(p) + 1;
      cout << ei->pin_no << endl;
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

  if(arrEmployeeInsert.size())
    insertEmployee(arrEmployeeInsert);
  if(arrEmployeeUpdate.size())
    updateEmployee(arrEmployeeUpdate);
  if(arrEmployeeDelete.size())
    deleteEmployee(arrEmployeeUpdate);

  mtx.unlock();
}

void EmployeeInfoMgr::insertEmployee(vector<pair<string, EmployeeInfo*> >& elems)
{
  int rc;
  sqlite3_stmt *stmt;
  const char* tail;
  LOGV("insertEmployee\n");
  rc = sqlite3_prepare_v2(m_db, INSERT_EMPLOYEE, -1, &stmt, &tail);

  for(vector<pair<string,EmployeeInfo*> >::size_type i=0; i< elems.size(); i++){
    //cache
    m_arrEmployee.insert(elems[i]);

    EmployeeInfo* ei = elems[i].second;
    sqlite3_bind_text(stmt, 1, ei->company_name.c_str(), ei->company_name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ei->lab_name.c_str(), ei->lab_name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ei->pin_no.c_str(), ei->pin_no.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, elems[i].first.c_str(), elems[i].first.length(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 5, ei->userdata, USERDATA_SIZE, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, ei->blacklistinfo.c_str(), ei->blacklistinfo.length(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, ei->pnt_cnt);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
      LOGE("Failed to insert : %s\n", sqlite3_errmsg(m_db));
      sqlite3_finalize(stmt);
      throw 1;
    }
    sqlite3_reset(stmt);

    m_eil->onEmployeeInfoInsert(ei->userdata);
  }
  sqlite3_finalize(stmt);

}

void EmployeeInfoMgr::updateEmployee(vector<pair<string, EmployeeInfo*> >& elems)
{
  int rc;

  sqlite3_stmt *stmt;
  const char* tail;
  LOGV("updateEmployee\n");
  rc = sqlite3_prepare_v2(m_db, UPDATE_EMPLOYEE, -1, &stmt, &tail);

  for(vector<pair<string,EmployeeInfo*> >::size_type i=0; i< elems.size(); i++){
    
    string& usercode = elems[i].first;
    EmployeeInfo* ei = elems[i].second;
    //cache
    delete m_arrEmployee[usercode];
    m_arrEmployee[usercode] = ei;

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
      throw 1;
    }
    sqlite3_reset(stmt);
    
    m_eil->onEmployeeInfoUpdate(usercode, ei->userdata);
  }
  sqlite3_finalize(stmt);

}

void EmployeeInfoMgr::deleteEmployee(vector<pair<string, EmployeeInfo*> >& elems)
{
  int rc;

  sqlite3_stmt *stmt;
  const char* tail;
  LOGV("deleteEmployee\n");
  rc = sqlite3_prepare_v2(m_db, DELETE_EMPLOYEE, -1, &stmt, &tail);
  //printf("delete size:%d\n", arrEmployeeDelete.size());
  for(vector<pair<string,EmployeeInfo*> >::size_type i=0; i< elems.size(); i++){
    string& usercode = elems[i].first;
    EmployeeInfo* ei = elems[i].second;

    //cache
    m_arrEmployee.erase(usercode);
    
    sqlite3_bind_text(stmt, 1, ei->pin_no.c_str(), ei->pin_no.length(), SQLITE_STATIC);
    //printf("pin %s\n", ei->pin_no.c_str());
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
      LOGE("Failed to insert : %s\n", sqlite3_errmsg(m_db));
      sqlite3_finalize(stmt);
      throw 1;
    }
    sqlite3_reset(stmt);
    m_eil->onEmployeeInfoDelete(usercode);
    delete ei;
  }
  sqlite3_finalize(stmt);
    

}

bool EmployeeInfoMgr::search(string pin_no)
{
  for(map<string, EmployeeInfo*>::iterator itr=m_arrEmployee.begin(); itr != m_arrEmployee.end(); itr++){
    EmployeeInfo* ei = itr->second;
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


