#include "datetime.h"
#include <stdio.h>
#ifdef _WIN32
#include "windows.h"
#endif
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef _DEBUG
#ifdef _WIN32
#ifndef __MINGW32__
#include <crtdbg.h>
#define CRTDBG_MAP_ALLOC
#define new new(_CLIENT_BLOCK, __FILE__, __LINE__)
#endif
#endif
#endif

using namespace tools;
#ifdef _WIN32
char* getString(char *buf, int type)
{
  SYSTEMTIME st;
  GetLocalTime(&st);
  if(type == 0)
    sprintf(buf, "%4d-%02d-%02d %d:%d:%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
  else if(type == 1)  
    sprintf(buf, "%4d-%02d-%02d-%d", st.wYear, st.wMonth, st.wDay, (st.wHour *60+ st.wMinute)*60 + st.wSecond);
  else if(type == 2)  
    sprintf(buf, "%4d%02d%02d %d%d%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
  return buf;
}
#else
DateTime::DateTime()
{
  struct tm _tm;
  time_t t = time(NULL);
  localtime_r(&t, &_tm);
  m_year = _tm.tm_year + 1900;
  m_month = _tm.tm_mon + 1;
  m_day = _tm.tm_mday;
  m_hour = _tm.tm_hour;
  m_minute = _tm.tm_min;
  m_second = _tm.tm_sec;
}

DateTime::DateTime(char* date) //data=20140101T010101
{
  char buf[5];
  if(!date) return;
  memcpy(buf, date, 4); buf[4] = '\0'; 
  m_year = atoi(buf);
  memcpy(buf, date+4, 2); buf[2] = '\0'; 
  m_month = atoi(buf);
  memcpy(buf, date+6, 2); buf[2] = '\0'; 
  m_day = atoi(buf);
  memcpy(buf, date+9, 2); buf[2] = '\0'; 
  m_hour = atoi(buf);
  memcpy(buf, date+11, 2); buf[2] = '\0'; 
  m_minute = atoi(buf);
  memcpy(buf, date+13, 2); buf[2] = '\0'; 
  m_second= atoi(buf);
  
}

void DateTime::now()
{
  struct tm _tm;
  time_t t = time(NULL);
  localtime_r(&t, &_tm);
  m_year = _tm.tm_year + 1900;
  m_month = _tm.tm_mon + 1;
  m_day = _tm.tm_mday;
  m_hour = _tm.tm_hour;
  m_minute = _tm.tm_min;
  m_second = _tm.tm_sec;
}

char* DateTime::toString()
{
  sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d", m_year, m_month, m_day, m_hour, m_minute, m_second);
  return buf;  
}
char* DateTime::toString(char c)
{
  sprintf(buf, "%4d-%02d-%02d%c%02d:%02d:%02d", m_year, m_month, m_day, c, m_hour, m_minute, m_second);
  return buf;  
}
char* DateTime::toString2()
{
  sprintf(buf, "%4d%02d%02d_%02d%02d%02d", m_year, m_month, m_day, m_hour, m_minute, m_second);
  return buf;  
}



namespace tools {
bool operator == (const DateTime& lh, const DateTime& rh)
{
  return (lh.m_year == rh.m_year && lh.m_month == rh.m_month && lh.m_day == rh.m_day && lh.m_hour == rh.m_hour && lh.m_minute == rh.m_minute && lh.m_second == rh.m_second);
}
bool operator != (const DateTime& lh, const DateTime& rh)
{
  return !(lh == rh);
}
bool operator > (const DateTime& lh, const DateTime& rh)
{
  return (lh.m_year > rh.m_year) || (lh.m_year == rh.m_year && lh.m_month > rh.m_month) || (lh.m_year == rh.m_year && lh.m_month == rh.m_month && lh.m_day > rh.m_day)
    || (lh.m_year == rh.m_year && lh.m_month == rh.m_month && lh.m_day == rh.m_day && lh.m_hour > rh.m_hour)
    || (lh.m_year == rh.m_year && lh.m_month == rh.m_month && lh.m_day == rh.m_day && lh.m_hour == rh.m_hour && lh.m_minute > rh.m_minute)
    || (lh.m_year == rh.m_year && lh.m_month == rh.m_month && lh.m_day == rh.m_day && lh.m_hour == rh.m_hour && lh.m_minute == rh.m_minute && lh.m_second > rh.m_second);    
}
bool operator < (const DateTime& lh, const DateTime& rh)
{
  return ( lh != rh ) && !( lh > rh );
}
bool operator >= (const DateTime& lh, const DateTime& rh)
{
  return !(lh < rh);
}
bool operator <= (const DateTime& lh, const DateTime& rh)
{
  return !(lh > rh);
}
}
#endif



