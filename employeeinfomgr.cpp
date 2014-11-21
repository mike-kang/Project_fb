#include "tools/log.h"
#include "tools/filesystem.h"
#include "employeeinfomgr.h"
#include "web/webservice.h"
#include "settings.h"
#include <fstream>
#include "tools/utils.h"
#include "tools/base64.h"

#define LOG_TAG "EmployInfoMgr"

using namespace tools;
using namespace std;
using namespace web;

//#define DB_FILE "employee.xml"
#define DB_NAME "employee.db"
#define CREATE_TABLE_EMPLOYEE "create table employee( pinno INTEGER, usercode TEXT, userdata BLOB)"
#define CREATE_TABLE_TIME "create table time( lastsync TEXT )"

EmployeeInfoMgr::EmployeeInfoMgr(Settings* settings, IWebService* ws): m_settings(settings), m_ws(ws)
{
  try{
    //m_bUseLocalDB = settings->getBool("App::LOCAL_DATABASE");
    m_sMemcoCd = m_settings->get("App::MEMCO_CD");
    m_sSiteCd = m_settings->get("App::SITE_CD");
    m_bDisplayPhoto = settings->getBool("App::DISPLAY_PHOTO");
  }
  catch(int e)
  {
    //m_bUseLocalDB = false;
    m_sMemcoCd = "MC00000003";
    m_sSiteCd = "ST00000005";
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
  
  return true;
}


bool EmployeeInfoMgr::updateLocalDB()
{
  try{
    m_ws->request_EmployeeInfoAll(m_lastSyncTime.toString().c_str(), 8000, DB_FILE);
    LOGV("downloaded %s\n", DB_FILE);
  }
  catch(web::Except e){
    LOGE("download %s fail\n", DB_FILE);
    return false;
  }

  ifstream infile (DB_FILE, ofstream::binary);
  // get size of file
  infile.seekg (0,infile.end);
  long size = infile.tellg();
  infile.seekg (0);
  // allocate memory for file content
  char* xml_buf = new char[size];
  // read content of infile
  infile.read (xml_buf,size);
  infile.close();
  int num = fillEmployeeInfoes(xml_buf, m_vectorEmployeeInfo);
  cout << "members = " << num << endl;
  delete xml_buf;
  return true;

}



bool EmployeeInfoMgr::getInfo(const char* serialNumber, EmployeeInfo* ei)
{
  //bool bNetAvailable = false;
  //cout << "getInfo" << endl;
  if(m_bUseLocalDB){
    goto localDB;
  }

  try{
    char* xml_buf = m_ws->request_EmployeeInfo(serialNumber, 3000);
    if(xml_buf){
      //cout << xml_buf << endl;
      bool ret = fillEmployeeInfo(xml_buf, ei);
      delete xml_buf;
      return ret;
    }
    return false;
  }
  catch(web::Except e){
    LOGE("request_EmployeeInfo: %s\n", WebService::dump_error(e));
  }

localDB:
  LOGI("Local DB check!\n");
  EmployeeInfo* t = searchDB(serialNumber);
  if(!t) return false;
  //memcpy(ei, t, sizeof(EmployeeInfo));
  strcpy(ei->serial_number, t->serial_number);
  ei->company_name = t->company_name;
  if(t->ent_co_ymd)
    ei->ent_co_ymd = new Date(t->ent_co_ymd);
  if(t->rtr_co_ymd)
    ei->rtr_co_ymd = new Date(t->rtr_co_ymd);
  ei->in_out_gb = t->in_out_gb;
  ei->lab_no = t->lab_no;
  ei->lab_name = t->lab_name;
  ei->pin_no = t->pin_no;
  ei->utype = t->utype;
  ei->zone_code = t->zone_code;
  
  return true;
    
}

int EmployeeInfoMgr::fillEmployeeInfoes(char *xml_buf, vector<EmployeeInfo*>& elems)
{
  char *p = xml_buf;
  char *end;
  //cout << xml_buf << endl;
  //LOGV("***fillEmployeeInfo:%s\n", xml_buf);
  mtx.lock();
  while(p = strstr(p, "<Table")){
    //printf("start %x\n", p);
    end = strstr(p+7, "</Table");
    *end = '\0';
    EmployeeInfo* ei = new EmployeeInfo;

    try {
      ei->pin_no = utils::getElementData(p, "PIN_NO");
      p += strlen(p) + 1;
    }
    catch(int e){}
    try {
      ei->lab_no = p = utils::getElementData(p, "LAB_NO");
      p += strlen(p) + 1;
    }
    catch(int e){}
    try {
      ei->lab_name = p = utils::getElementData(p, "LAB_NM");
      p += strlen(p) + 1;
    }
    catch(int e){}
    try {
      ei->company_name = p = utils::getElementData(p, "CO_NM");
      p += strlen(p) + 1;
    }
    catch(int e){}
    try {
      p = utils::getElementData(p, "RFID_CAR");
      strcpy(ei->serial_number, p); 
      p += strlen(p) + 1;
    }
    catch(int e){}
    try {
      ei->in_out_gb = p = utils::getElementData(p, "IN_OUT_GB");
      p += strlen(p) + 1;
    }
    catch(int e){}
    try {
      ei->zone_code = p = utils::getElementData(p, "ZONE_CD");
      p += strlen(p) + 1;
    }
    catch(int e){}
    try {
      ei->ent_co_ymd = new Date(p = utils::getElementData(p, "ENT_CO_YMD"));
      p += strlen(p) + 1;
    }
    catch(int e){}
    try {
      ei->rtr_co_ymd = new Date(p = utils::getElementData(p, "RTR_CO_YMD"));
      p += strlen(p) + 1;
    }
    catch(int e){}
    try {
      ei->utype = *utils::getElementData(p, "UTYPE");
    }
    catch(int e){}

    elems.push_back(ei);
    
    p = end + 8;
  }
  mtx.unlock();
  return elems.size();
}

EmployeeInfoMgr::EmployeeInfo* EmployeeInfoMgr::searchDB(const char* serialNumber)
{
  mtx.lock();

  for(vector<EmployeeInfo*>::size_type i=0; i< m_vectorEmployeeInfo.size(); i++)
  {
    //cout << m_vectorEmployeeInfo[i]->serial_number << endl;
    if(!strcmp(m_vectorEmployeeInfo[i]->serial_number, serialNumber)){
      cout << "searchDB:" <<  m_vectorEmployeeInfo[i]->in_out_gb << endl;
      mtx.unlock();
      return m_vectorEmployeeInfo[i];
    }
  }
  mtx.unlock();
  return NULL;
} 

