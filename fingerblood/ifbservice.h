#ifndef _IFB_SERVICE_HEADER
#define _IFB_SERVICE_HEADER

#include <iostream>
#include <list>
#include <vector>
#include <map>

#define FEATURE_FINGER_IMAGE


class IFBService {
public:
  class IFBServiceEventListener {
  public:
    enum SyncStatus{
      SS_START,
      SS_COUNT,
      SS_PROCESS,
      SS_FAIL,
      SS_SUCCESS,
    };
    virtual bool onScanStarted(bool bValid /* usercode valid */ ) = 0;
    virtual void onScanData(const char* buf) = 0;
    virtual const unsigned char* onGetFingerImg(const char*& usercode, const char*&) = 0;
    virtual void onVIMG(const unsigned char* img, int len) = 0;
    virtual bool onNeedDeviceKey(char* id, char* key) = 0;
    virtual void onNeedUserCodeList(std::vector<std::pair<const char*, unsigned char*> >& arr_16, std::vector<std::pair<const char*, unsigned char*> >& arr_4) = 0;
    virtual void onSync(IFBService::IFBServiceEventListener::SyncStatus status, int index=0) = 0;
    virtual void onFormat(bool ret) = 0;

    virtual void onUpdateBegin(int save_count, int delete_count) = 0;
    virtual void onUpdateSave(int index) = 0;
    virtual void onUpdateDelete(int index) = 0;
    virtual void onUpdateEnd() = 0;
  };

  virtual bool request_openDevice(bool check_device_id) = 0; //only sync
  virtual void request_closeDevice() = 0; //only sync
  virtual void request_sync(void) = 0; //only async
  virtual bool request_getList(std::list<std::string>* li) = 0; //only sync
  virtual void request_format() = 0; //only async
  virtual void request_startScan(int interval) = 0; //only async
  virtual void request_stopScan() = 0; //only async
  virtual void request_buzzer(bool val) = 0; //only async
  virtual bool request_saveUsercode(const unsigned char* userdata, int length) = 0; //only sync
  virtual bool request_saveUsercode(const char* filename) = 0; //only sync
  virtual bool request_deleteUsercode(const char* usercode) = 0; //only sync
  virtual bool request_stopCmd() = 0; //only sync
  virtual void request_update(std::vector<unsigned char*>* arrSave, 
  std::vector<std::string>* arrDelete) = 0; //only async
  //virtual void update(std::vector<std::pair<const char*, unsigned char*> >& arrSave, std::vector<string>& arrDelete) = 0;
#ifdef FEATURE_FINGER_IMAGE
  virtual unsigned char* request_getScanImage() = 0; //only sync
  virtual void setCompareThreshold(int val) = 0;
  virtual void setSaveVIMG(bool val) = 0;
#endif
};




#endif

