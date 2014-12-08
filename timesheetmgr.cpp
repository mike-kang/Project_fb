#include "timesheetmgr.h"
#include "tools/log.h"
#include "settings.h"
#include <sys/time.h>
#include "tools/filesystem.h"
#include <errno.h>
#include <web/webservice.h>


#define LOG_TAG "TimeSheetMgr"
#define STORE_DIRECTORY "timesheets"

using namespace std;
using namespace tools;
using namespace web;


TimeSheetMgr::TimeSheetMgr(Settings* settings, web::IWebService* ws, TimeSheetMgrEventListener* el):m_settings(settings), m_ws(ws), m_el(el)
{
  m_sMemcoCd = m_settings->get("App::MEMCO");
  m_sSiteCd = m_settings->get("App::SITE");
  //m_sDvLoc = m_settings->get("App::DV_LOC"); // = "0001";
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

TimeSheetMgr::TimeSheet::TimeSheet(string pinno)
  :m_pinno(pinno)
{
}

TimeSheetMgr::TimeSheet::~TimeSheet()
{
  //if(m_photo_img)
  //  delete m_photo_img;
}

void TimeSheetMgr::insert(string pinno)
{
  TimeSheet* ts = new TimeSheet(pinno);
  mtx.lock();
  m_listTS.push_back(ts);
  mtx.unlock();
  
  m_el->onTimeSheetCacheCountChanged(m_listTS.size());
}

bool TimeSheetMgr::upload()
{
  vector<list<TimeSheet*>::iterator> vector_erase;
  
  // 1. send files
  vector<string*> filelist;
  try{
    filesystem::getList(STORE_DIRECTORY, filelist);
  }
  catch(filesystem::Exception e){
    LOGF("get List error:%s\n", strerror(errno));
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
    catch(web::Except e){
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
  m_el->onTimeSheetFileCountChanged(filelist.size());
  
  // 2. send list
  for(list<TimeSheet*>::iterator itr = m_listTS.begin(); itr != m_listTS.end(); itr++){
    bool ret = false;
    try{
      ret = m_ws->request_UploadTimeSheet((*itr)->m_time.toString(), (*itr)->m_pinno.c_str()
        , 3000, STORE_DIRECTORY);
    }
    catch(web::Except e){
      LOGE("request_UploadTimeSheet: %s\n", WebService::dump_error(e));
    }
    vector_erase.push_back(itr);
  }

  mtx.lock();
  for(vector<list<TimeSheet*>::iterator>::size_type i=0; i< vector_erase.size(); i++){
    delete *vector_erase[i];
    m_listTS.erase(vector_erase[i]);
  }
  mtx.unlock();

  m_el->onTimeSheetCacheCountChanged(m_listTS.size());
  return true;
}
  

  

