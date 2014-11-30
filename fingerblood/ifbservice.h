#ifndef _IFB_SERVICE_HEADER
#define _IFB_SERVICE_HEADER

#include <iostream>
#include <list>
#include <vector>
#include <map>

class IFBService {
public:
  class IFBServiceEventListener {
  public:
    virtual void onScanData(const char* buf) = 0;
    virtual bool onNeedDeviceKey(char* id, char* key) = 0;
    virtual void onNeedUserCodeList(std::vector<std::pair<const char*, unsigned char*> >& arr_16, std::vector<std::pair<const char*, unsigned char*> >& arr_4) = 0;
    virtual void onSyncComplete(bool result) = 0;
  };

  virtual bool start(bool check = false) = 0;
  virtual void stop() = 0;
  virtual char* getVersion() = 0;
  virtual bool getList(std::list<std::string>& li) = 0;
  virtual bool format() = 0;  //auto restart
  virtual bool requestStartScan(int interval)= 0;
  virtual int requestStopScan() = 0;
  virtual bool save(const char* filename) = 0;
  virtual bool save(const unsigned char* buf, int length) = 0;
  virtual bool deleteUsercode(const char* usercode) = 0;
  virtual void buzzer(bool val) = 0;
  virtual void sync() = 0;
};




#endif

