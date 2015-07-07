#include "settings.h"
#include "inih_r29/INIReader.h"
#include "tools/log.h"

#define LOG_TAG "Settings"

using namespace std;

#define mapStrInsert(category, key, value) \
  m_mapStr.insert(pair<string, string>(#category"::"#key, #value))
#define mapBoolInsert(category, key, value) \
    m_mapBool.insert(pair<string, bool>(#category"::"#key, value))
#define mapIntInsert(category, key, value) \
      m_mapInt.insert(pair<string, int>(#category"::"#key, value))

#define mapStrInsertFromReader(category, key, value) \
  m_mapStr.insert(pair<string, string>(#category"::"#key, reader.Get(#category, #key, #value)))
#define mapBoolInsertFromReader(category, key, value) \
    m_mapBool.insert(pair<string, bool>(#category"::"#key, reader.GetBoolean(#category, #key, value)))
#define mapIntInsertFromReader(category, key, value) \
      m_mapInt.insert(pair<string, int>(#category"::"#key, reader.GetInteger(#category, #key, value)))

Settings::Settings(const char* filename)
{
  INIReader reader(filename);
  if (reader.ParseError() < 0) {
     cout << "Can't load " << filename << endl;
     //default settings
     //App
     //mapStrInsert(App, AUTH_CODE, );
     mapStrInsert(App, MEMCO, MC00000003);
     mapStrInsert(App, SITE, ST00000005);
     mapStrInsert(App, EMBED, 0001);
     mapStrInsert(App, DV_NO, 1);
     mapStrInsert(App, IN_OUT, I);
     mapStrInsert(App, AUTH_CODE, );
     mapBoolInsert(App, CHECK, true);
     mapStrInsert(App, REBOOT_TIME, );
     mapStrInsert(App, WORKING_DIRECTORY, /home/pi/acu);
     //mapBoolInsert(App, DISPLAY_PHOTO, true);
     mapIntInsert(App, TIMER_INTERVAL, 60);  //60 sec
     mapStrInsert(App, ADMIN1, 0000001111111111);
     mapStrInsert(App, ADMIN2, 0000001111111112);
     mapStrInsert(App, ADMIN3, 0000001111111113);
     mapStrInsert(App, ADMIN4, 0000001111111114);
     mapBoolInsert(App, DISPLAY_EMPLOYEE_INFO, true);
#ifdef FEATURE_FINGER_IMAGE
     mapBoolInsert(App, PIN_FIRST_CHECK, true);
     mapBoolInsert(App, DISPLAY_VIMG, true);
#endif 
     mapBoolInsert(App, TEST_SIGNAL, false);
     mapBoolInsert(App, UPLOAD_TIMESHEET_DISABLE, false);
     mapStrInsert(App, SECURITY_NUMBER, );

     //Action
     mapBoolInsert(Action, CAPTURE, true);
     mapBoolInsert(Action, SOUND, true);
     
     //Rfid
#ifdef RFID     
     mapStrInsert(Rfid, MODE, 1356M); 
     mapIntInsert(Rfid, CHECK_INTERVAL, 300);  // 300 ms
     mapStrInsert(Rfid, RFID1356_PORT, /dev/ttyAMA0); 
     mapStrInsert(Rfid, RFID800_PORT, /dev/ttyUSB0); 
#endif 
     //FB
     mapBoolInsert(FB, CHECK_DEVICE_ID, true);
     mapIntInsert(FB, CHECK_INTERVAL, 300);  // 300 ms
     mapStrInsert(FB, PORT, /dev/ttyUSB0); 
     mapBoolInsert(FB, BUZZER, true);
     mapBoolInsert(FB, CHECK_CODE_4, false);
#ifdef FEATURE_FINGER_IMAGE
     mapIntInsert(FB, COMP_THRESHOLD, 50);  // 300 ms
     mapBoolInsert(FB, VIMG_SAVE_FILE, true);
#endif 

     //FB_KEY
     mapStrInsert(FB_KEY, 14070687 , 2C2DE78A4F113209);
     mapStrInsert(FB_KEY, 14070589 , 5EC38CB4B3AA2C9B);
     mapStrInsert(FB_KEY, 14070590 , 80D4FEB2EC4B5CB3);

     //Camera
#ifdef CAMARA    
     mapIntInsert(Camera, DELAY_OFF_TIME, 600);  //600 sec
     mapBoolInsert(Camera, SAVE_PICTURE_FILE, false);
     mapIntInsert(Camera, TAKEPICTURE_MAX_WAIT_TIME, 2);  // 2 sec
#endif     
     
     //Log
     mapBoolInsert(Log, CONSOLE, false);
     mapIntInsert(Log, CONSOLE_LEVEL, 1);  // 1(VERBOSE), 2(DEBUF), 3(INFO), 4(WARN), 5(ERROR), 6(FATAL)
     mapStrInsert(Log, CONSOLE_PATH, /dev/pts/3);
     mapBoolInsert(Log, FILE, true);
     mapIntInsert(Log, FILE_LEVEL, 3);  // 1(VERBOSE), 2(DEBUF), 3(INFO), 4(WARN), 5(ERROR), 6(FATAL)
     mapStrInsert(Log, FILE_DIRECTORY, Log);

     //Server
     mapStrInsert(Server, TYPE, SM);
     mapStrInsert(Server, SAFEIDSVC_URL, http:\x2f/dev.safeman.co.kr/SafeIDService.asmx);
     mapStrInsert(Server, LOTTEIDSVC_URL, http:\x2f/lottedev.safeman.co.kr/LotteIDService.asmx);
     mapStrInsert(Server, DWSVC_URL, http:\x2f/112.175.10.40/WebService.asmx);

     //Gpio
     mapIntInsert(Gpio, YELLOW, 27);
     mapIntInsert(Gpio, BLUE, 22);
     mapIntInsert(Gpio, GREEN, 24);
     mapIntInsert(Gpio, RED, 23);
     mapIntInsert(Gpio, RELAY, 17);
     
     dump();
     return;
  }

  //App
  //mapStrInsertFromReader(App, AUTH_CODE, );
  mapStrInsertFromReader(App, MEMCO, MC00000003);
  mapStrInsertFromReader(App, SITE, ST00000005);
  mapStrInsertFromReader(App, EMBED, 0001);
  mapStrInsertFromReader(App, DV_NO, 1);
  mapStrInsertFromReader(App, IN_OUT, I);
  mapStrInsertFromReader(App, AUTH_CODE, );
  mapBoolInsertFromReader(App, CHECK, true);
  mapStrInsertFromReader(App, REBOOT_TIME, );
  mapStrInsertFromReader(App, WORKING_DIRECTORY, /home/pi/acu);
  //mapBoolInsertFromReader(App, DISPLAY_PHOTO, true);
  mapIntInsertFromReader(App, TIMER_INTERVAL, 60);  //60 sec
  mapBoolInsertFromReader(App, TEST_SIGNAL, false);
  mapStrInsertFromReader(App, ADMIN1, 0000001111111111);
  mapStrInsertFromReader(App, ADMIN2, 0000001111111112);
  mapStrInsertFromReader(App, ADMIN3, 0000001111111113);
  mapStrInsertFromReader(App, ADMIN4, 0000001111111114);
  mapBoolInsertFromReader(App, DISPLAY_EMPLOYEE_INFO, true);
#ifdef FEATURE_FINGER_IMAGE
  mapBoolInsertFromReader(App, PIN_FIRST_CHECK, true);
  mapBoolInsertFromReader(App, DISPLAY_VIMG, true);
#endif
  mapBoolInsertFromReader(App, TEST_SIGNAL, false);
  mapBoolInsertFromReader(App, UPLOAD_TIMESHEET_DISABLE, false);
  mapStrInsertFromReader(App, SECURITY_NUMBER, );

  //Action
  mapBoolInsertFromReader(Action, CAPTURE, true);
  mapBoolInsertFromReader(Action, SOUND, true);

  //Rfid
#ifdef RFID     
  mapStrInsertFromReader(Rfid, MODE, 1356M); 
  mapIntInsertFromReader(Rfid, CHECK_INTERVAL, 300);  // 300 ms
  mapStrInsertFromReader(Rfid, RFID1356_PORT, /dev/ttyAMA0); 
  mapStrInsertFromReader(Rfid, RFID800_PORT, /dev/ttyUSB0); 
#endif
  //FB
  mapBoolInsertFromReader(FB, CHECK_DEVICE_ID, true);
  mapIntInsertFromReader(FB, CHECK_INTERVAL, 300);  // 300 ms
  mapStrInsertFromReader(FB, PORT, /dev/ttyUSB0); 
  mapBoolInsertFromReader(FB, BUZZER, true);
  mapBoolInsertFromReader(FB, CHECK_CODE_4, false);
#ifdef FEATURE_FINGER_IMAGE
  mapIntInsertFromReader(FB, COMP_THRESHOLD, 50);  // 300 ms
  mapBoolInsertFromReader(FB, VIMG_SAVE_FILE, true);
#endif

  //FB_KEY
  mapStrInsertFromReader(FB_KEY, 14070687 , 2C2DE78A4F113209);
  mapStrInsertFromReader(FB_KEY, 14070589 , 5EC38CB4B3AA2C9B);
  mapStrInsertFromReader(FB_KEY, 14070590 , 80D4FEB2EC4B5CB3);

  //Camera
#ifdef CAMARA    
  mapIntInsertFromReader(Camera, DELAY_OFF_TIME, 600);  //600 sec
  mapBoolInsertFromReader(Camera, SAVE_PICTURE_FILE, false);
  mapIntInsertFromReader(Camera, TAKEPICTURE_MAX_WAIT_TIME, 2);  // 2 sec
#endif
  //Log
  mapBoolInsertFromReader(Log, CONSOLE, false);
  mapIntInsertFromReader(Log, CONSOLE_LEVEL, 1);  // 1(VERBOSE), 2(DEBUF), 3(INFO), 4(WARN), 5(ERROR), 6(FATAL)
  mapStrInsertFromReader(Log, CONSOLE_PATH, /dev/pts/3);
  mapBoolInsertFromReader(Log, FILE, true);
  mapIntInsertFromReader(Log, FILE_LEVEL, 3);  // 1(VERBOSE), 2(DEBUF), 3(INFO), 4(WARN), 5(ERROR), 6(FATAL)
  mapStrInsertFromReader(Log, FILE_DIRECTORY, Log);

  //Server
  mapStrInsertFromReader(Server, TYPE, SM);
  mapStrInsertFromReader(Server, SAFEIDSVC_URL, http:\x2f/dev.safeman.co.kr/SafeIDService.asmx);
  mapStrInsertFromReader(Server, LOTTEIDSVC_URL, http:\x2f/lottedev.safeman.co.kr/LotteIDService.asmx);
  mapStrInsertFromReader(Server, DWSVC_URL, http:\x2f/112.175.10.40/WebService.asmx);

  //Gpio
  mapIntInsertFromReader(Gpio, YELLOW, 27);
  mapIntInsertFromReader(Gpio, BLUE, 22);
  mapIntInsertFromReader(Gpio, GREEN, 24);
  mapIntInsertFromReader(Gpio, RED, 23);
  mapIntInsertFromReader(Gpio, RELAY, 17);

  dump();
}


void Settings::dump()
{
  cout << "Settings contents start ******" << endl;
  for(map<string, string>::iterator iter=m_mapStr.begin(); iter != m_mapStr.end(); ++iter)
    cout << "(" << iter->first << ") " << iter->second << endl;
  for(map<string, int>::iterator iter=m_mapInt.begin(); iter != m_mapInt.end(); ++iter)
    cout << "(" << iter->first << ") " << iter->second << endl;
  for(map<string, bool>::iterator iter=m_mapBool.begin(); iter != m_mapBool.end(); ++iter)
    cout << "(" << iter->first << ") " << ((iter->second)?"true":"false") << endl;
  cout << "Settings contents end ******" << endl;
}

string& Settings::get(string key)
{
  map<string, string>::iterator iter = m_mapStr.find(key);
  if(iter == m_mapStr.end()){
    cout << key << " key is not exist" << endl;
    throw EXCEPTION_NO_EXIST_KEY;
  }
  return iter->second;
}
bool Settings::getBool(string key)
{ 
  map<string, bool>::iterator iter = m_mapBool.find(key);
  if(iter == m_mapBool.end()){
    cout << key << " key is not exist" << endl;
    throw EXCEPTION_NO_EXIST_KEY;
  }
  return iter->second;
}
int Settings::getInt(string key)
{ 
  map<string, int>::iterator iter = m_mapInt.find(key);
  if(iter == m_mapInt.end()){
    cout << key << " key is not exist" << endl;
    throw EXCEPTION_NO_EXIST_KEY;
  }
  return iter->second;
}






  

