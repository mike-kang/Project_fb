#ifndef _LOG_SERVICE_HEADER
#define _LOG_SERVICE_HEADER

#include "queue.h"
#include "thread.h"
#include "stdio.h"

#define LOG_TAG_SIZE	40
#define LOG_MSG_SIZE	946 //4096
#define LOG_TIME_SIZE	32

struct Time {
  int m_year;
  int m_mon;
  int m_day;
  short m_hour;
  short m_minute;
  short m_second;
  short m_ms;

  Time(short h, short m, short s, short ms, int year =0, int mon=0, int day=0):m_hour(h), m_minute(m), m_second(s), m_ms(ms), m_year(year), m_mon(mon), m_day(day){};
  char* toString()
  {
    static char buf[32];
    sprintf(buf, "%d:%d:%d:%d", m_hour, m_minute, m_second, m_ms);
    return buf;
  }
};
struct Entry {
  int m_pri;
  unsigned long m_threadID;
  char m_tag[LOG_TAG_SIZE];
  char m_msg[LOG_MSG_SIZE];
  Time *m_t;

  Entry(int pri, const char *tag, const char *msg, unsigned long tid, Time *t):m_pri(pri), m_threadID(tid), m_t(t)
  {
    strncpy(m_tag, tag, LOG_TAG_SIZE);
    strncpy(m_msg, msg, LOG_MSG_SIZE);
  }

  ~Entry()
  {
    delete m_t;
  }

};

class LogService {
public:
  enum log_t {
    TYPE_NULL,
    TYPE_FILE,
    TYPE_CONSOLE,
    TYPE_FILE_CONSOLE,
    TYPE_DEBUGCONSOLE
  };
  //static LogService* init(log_t type, const char* path);
  static LogService* init(bool bconsole, int console_level, const char* console_path, bool bfile, int file_level, const char* file_dirctory);
  static LogService* getService();
  static void setEnable(bool enable);
  void sendMessage(int prio, const char *tag, const char *msg);
  ~LogService();

private:
  LogService(bool bconsole, int console_level, const char* console_path, bool bfile, int file_level, const char* file_dirctory);
  
  void run();
  static LogService *m_instance;
  static bool m_enabled;
  Thread<LogService> *m_thread;
  tools::Queue<Entry> m_EntryQ;
  bool m_bConsole;
  bool m_bFile;
  int m_consoleLevel;
  int m_fileLevel;
  char m_consolePath[100];
  char m_fileDirectory[100];
  log_t m_log_type;
};


#endif //_LOG_SERVICE_HEADER
