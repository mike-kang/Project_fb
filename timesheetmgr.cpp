#include "timesheetmgr.h"
#include "tools/log.h"
#include "web/webservice.h"
#include "settings.h"
#include <sys/time.h>
#include "tools/filesystem.h"
#include <errno.h>

#define LOG_TAG "TimeSheetMgr"
#define STORE_DIRECTORY "timesheets"

using namespace std;
using namespace tools;

TimeSheetMgr::TimeSheetMgr(Settings* settings, WebService* ws):m_settings(settings), m_ws(ws)
{
  m_sMemcoCd = m_settings->get("App::MEMCO_CD");
  m_sSiteCd = m_settings->get("App::SITE_CD");
  m_sDvLoc = m_settings->get("App::DV_LOC"); // = "0001";
  m_sDvNo = m_settings->get("App::DV_NO"); // = "6";
  m_cInOut = *m_settings->get("App::IN_OUT").c_str();

  //create directory
  if(!filesystem::file_exist(STORE_DIRECTORY))
    filesystem::dir_create(STORE_DIRECTORY);
}

TimeSheetMgr::~TimeSheetMgr()
{
  for(list<TimeSheet*>::iterator itr = m_listTS.begin(); itr != m_listTS.end(); itr++)
    delete *itr;
}

TimeSheetMgr::TimeSheet::TimeSheet(string lab_no, char utype, char* img, int img_sz)
  :m_lab_no(lab_no), m_utype(utype), m_imgSz(img_sz)
{
  struct tm _tm;
  time_t t = time(NULL);
  localtime_r(&t, &_tm);
  char buf[64];
  sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday
    , _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
  m_time = buf;
  m_photo_img = new char[img_sz];
  memcpy(m_photo_img, img, img_sz);
}

TimeSheetMgr::TimeSheet::~TimeSheet()
{
  if(m_photo_img)
    delete m_photo_img;
}

void TimeSheetMgr::insert(string lab_no, char utype, char* img, int img_sz)
{
  TimeSheet* ts = new TimeSheet(lab_no, utype, img, img_sz);
  mtx.lock();
  m_listTS.push_back(ts);
  mtx.unlock();
}

bool TimeSheetMgr::upload()
{
  vector<list<TimeSheet*>::iterator> vector_erase;
  // 1. send file
  vector<string*> filelist;
  try{
    filesystem::getList(STORE_DIRECTORY, filelist);
  }
  catch(filesystem::Exception e){
    LOGE("get List error:%s\n", strerror(errno));
    return false;
  }
  for(vector<string*>::size_type i=0; i< filelist.size(); i++){
    try{
      //LOGV("entry:%s\n", filelist[i]->c_str());
      cout << "entry:" << *filelist[i] << endl;
      char path[256];
      sprintf(path, "%s/%s", STORE_DIRECTORY, filelist[i]->c_str());
      bool ret = m_ws->request_SendFile(path, 3000);
      //cout << "result:" << ret << endl;
      if(ret){
        LOGV("file_delete: %s\n", path);
        filesystem::file_delete(path);
      }
      else{
        LOGE("request_SendFile fail\n");
      }
        
    }
    catch(WebService::Except e){
      LOGE("request_SendFile: %s\n", WebService::dump_error(e));
      for(vector<string*>::size_type i=0; i < filelist.size(); i++){
        delete filelist[i];
      }
      return false;
    }
  }
  for(vector<string*>::size_type i=0; i < filelist.size(); i++){
    delete filelist[i];
  }
  
  // 2. send list
  for(list<TimeSheet*>::iterator itr = m_listTS.begin(); itr != m_listTS.end(); itr++){
    bool ret = false;
    try{
      ret = m_ws->request_TimeSheetInsertString(m_sMemcoCd.c_str(), m_sSiteCd.c_str(), (*itr)->m_lab_no.c_str(), m_cInOut, m_sDvNo.c_str(), m_sDvLoc.c_str(), (*itr)->m_utype, (*itr)->m_time.c_str(), 
        (*itr)->m_photo_img, (*itr)->m_imgSz, 3000, STORE_DIRECTORY);
    }
    catch(WebService::Except e){
      LOGE("request_TimeSheetInsertString: %s\n", WebService::dump_error(e));
    }
    vector_erase.push_back(itr);
  }

  mtx.lock();
  for(vector<list<TimeSheet*>::iterator>::size_type i=0; i< vector_erase.size(); i++){
    delete *vector_erase[i];
    m_listTS.erase(vector_erase[i]);
  }
  mtx.unlock();
  return true;
}
  

  

