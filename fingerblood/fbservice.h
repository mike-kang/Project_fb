#ifndef _FB_SERVICE_HEADER
#define _FB_SERVICE_HEADER

#include "ifbservice.h"
#include "fbprotocolCMSerial.h"
#include "fb_protocol.h"
#include <iostream>
#include <list>
#include <vector>
#include <map>
#include "tools/timer.h"
#include "tools/Semaphore.h"
#include "tools/queue.h"
#include "tools/event.h"

using namespace tools;

class FBService : public IFBService{
public:
  FBService(const char* path, Serial::Baud baud, IFBService::IFBServiceEventListener* fn, bool bCheckUserCode4);
  virtual ~FBService();
  virtual bool request_openDevice(bool check_device_id); //only sync
  virtual void request_closeDevice(); //only sync
  virtual void request_sync(void); //only async
  virtual bool request_getList(list<string>* li); //only sync
  virtual void request_format(); //only async
  virtual void request_startScan(int interval); //only async
  virtual void request_stopScan(); //only async
  virtual void request_buzzer(bool val); //only async
  virtual bool request_saveUsercode(const byte* userdata, int length); //only sync
  virtual bool request_saveUsercode(const char* filename); //only sync
  virtual bool request_deleteUsercode(const char* usercode); //only sync
  virtual bool request_stopCmd(); //only sync
  virtual void request_update(std::vector<unsigned char*>* arrSave, std::vector<string>* arrDelete); //only async
  //virtual void update(std::vector<std::pair<const char*, unsigned char*> >& arrSave, std::vector<string>& arrDelete);
  
private:  
  void run();
  void openDevice(void* args);
  void closeDevice(void* args);
  void sync(void* args);
  void getList(void* args);
  void format(void* args);  //auto restart
  void startScan(void* args);
  void stopScan(void* args);
  void scan(void* args);
  void buzzer(void* args);
  void saveUsercodeFile(void* args);
  void saveUsercodeBuffer(void* args);
  void deleteUsercode(void* args);
  void stopCmd(void* arg);
  void update(void* arg);
  
  bool checkdeviceID();
  char* getVersion();

  bool m_bActive;
  FBProtocolCMSerial* m_serial;
  FBProtocol* m_protocol;
  
  //Thread<FBService>* m_thread_scan;
  bool m_scan_running;
  int m_scan_interval; //msec
  IFBService::IFBServiceEventListener* m_fn;
  Timer* m_tmrScan; //for queueing event
  static void cbTimerScan(void* arg);

  Thread<FBService>* m_thread;
  tools::Queue<TEvent<FBService> > m_eventQ;
  //Semaphore m_SemCompleteProcessEvent;
  TEvent<FBService>* m_event;

  //bool m_sync_running;
  bool m_bCheckUserCode4;
  
};




#endif

