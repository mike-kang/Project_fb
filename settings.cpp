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
     mapStrInsert(App, AUTH_CD, );
     mapStrInsert(App, MEMCO_CD, MC00000003);
     mapStrInsert(App, SITE_CD, ST00000005);
     mapStrInsert(App, DV_LOC, 0001);
     mapStrInsert(App, DV_NO, 6);
     mapBoolInsert(App, LOCAL_DATABASE, false);
     mapStrInsert(App, IN_OUT, I);
     mapStrInsert(App, REBOOT_TIME, );
     mapStrInsert(App, WORKING_DIRECTORY, /home/pi/acu);
     mapBoolInsert(App, DISPLAY_PHOTO, true);
     mapIntInsert(App, TIMER_INTERVAL, 60);  //60 sec
     
     //Action
     mapBoolInsert(Action, CAPTURE, true);
     //mapBoolInsert(Action, RELAY, true);
     mapBoolInsert(Action, SOUND, true);
     
     //Rfid
     mapStrInsert(Rfid, MODE, 1356M); 
     mapIntInsert(Rfid, CHECK_INTERVAL, 300);  // 300 ms
     mapStrInsert(Rfid, RFID1356_PORT, /dev/ttyAMA0); 
     mapStrInsert(Rfid, RFID800_PORT, /dev/ttyUSB0); 
     
     //Camera
     mapIntInsert(Camera, DELAY_OFF_TIME, 600);  //600 sec
     mapBoolInsert(Camera, SAVE_PICTURE_FILE, false);
     mapIntInsert(Camera, TAKEPICTURE_MAX_WAIT_TIME, 2);  // 2 sec
     
     
     //Log
     mapBoolInsert(Log, CONSOLE, false);
     mapIntInsert(Log, CONSOLE_LEVEL, 1);  // 1(VERBOSE), 2(DEBUF), 3(INFO), 4(WARN), 5(ERROR), 6(FATAL)
     mapStrInsert(Log, CONSOLE_PATH, /dev/pts/3);
     mapBoolInsert(Log, FILE, true);
     mapIntInsert(Log, FILE_LEVEL, 3);  // 1(VERBOSE), 2(DEBUF), 3(INFO), 4(WARN), 5(ERROR), 6(FATAL)
     mapStrInsert(Log, FILE_DIRECTORY, Log);

     //Server
     mapStrInsert(Server, URL, http:\/\/10.9.0.2:8080/WebService/ItlogService.asmx);

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
  mapStrInsertFromReader(App, AUTH_CD, );
  mapStrInsertFromReader(App, MEMCO_CD, MC00000003);
  mapStrInsertFromReader(App, SITE_CD, ST00000005);
  mapStrInsertFromReader(App, DV_LOC, 0001);
  mapStrInsertFromReader(App, DV_NO, 6);
  mapBoolInsertFromReader(App, LOCAL_DATABASE, false);
  mapStrInsertFromReader(App, IN_OUT, I);
  mapStrInsertFromReader(App, REBOOT_TIME, );
  mapStrInsertFromReader(App, WORKING_DIRECTORY, /home/pi/acu);
  mapBoolInsertFromReader(App, DISPLAY_PHOTO, true);
  mapIntInsertFromReader(App, TIMER_INTERVAL, 60);  //60 sec
  //Action
  mapBoolInsertFromReader(Action, CAPTURE, true);
  //mapBoolInsertFromReader(Action, RELAY, true);
  mapBoolInsertFromReader(Action, SOUND, true);

  //Rfid
  mapStrInsertFromReader(Rfid, MODE, 1356M); 
  mapIntInsertFromReader(Rfid, CHECK_INTERVAL, 300);  // 300 ms
  mapStrInsertFromReader(Rfid, RFID1356_PORT, /dev/ttyAMA0); 
  mapStrInsertFromReader(Rfid, RFID800_PORT, /dev/ttyUSB0); 

  //Camera
  mapIntInsertFromReader(Camera, DELAY_OFF_TIME, 600);  //600 sec
  mapBoolInsertFromReader(Camera, SAVE_PICTURE_FILE, false);
  mapIntInsertFromReader(Camera, TAKEPICTURE_MAX_WAIT_TIME, 2);  // 2 sec

  //Log
  mapBoolInsertFromReader(Log, CONSOLE, false);
  mapIntInsertFromReader(Log, CONSOLE_LEVEL, 1);  // 1(VERBOSE), 2(DEBUF), 3(INFO), 4(WARN), 5(ERROR), 6(FATAL)
  mapStrInsertFromReader(Log, CONSOLE_PATH, /dev/pts/3);
  mapBoolInsertFromReader(Log, FILE, true);
  mapIntInsertFromReader(Log, FILE_LEVEL, 3);  // 1(VERBOSE), 2(DEBUF), 3(INFO), 4(WARN), 5(ERROR), 6(FATAL)
  mapStrInsertFromReader(Log, FILE_DIRECTORY, Log);

  //Server
  mapStrInsertFromReader(Server, URL, http:\/\/10.9.0.2:8080/WebService/ItlogService.asmx);

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






  

