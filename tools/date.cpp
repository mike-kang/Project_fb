#include "date.h"
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
    sprintf(buf, "%4d%02d%02d_%d%d%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
  return buf;
}
#else
Date::Date(char* date) //data=20140101
{
  char buf[5];
  if(!date) return;
  memcpy(buf, date, 4); buf[4] = '\0'; 
  m_year = atoi(buf);
  memcpy(buf, date+4, 2); buf[2] = '\0'; 
  m_month = atoi(buf);
  memcpy(buf, date+6, 2); buf[2] = '\0'; 
  m_day = atoi(buf);
  
}

Date* Date::now()
{
  //struct timeval  tv;
  struct tm _tm;
  time_t t = time(NULL);
  localtime_r(&t, &_tm);
  Date* date = new Date(_tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday);
  return date;  
}

string Date::toString()
{
  string ret;
  char buf[30];
  sprintf(buf, "%4d-%02d-%02d", m_year + 1900, m_month + 1, m_day);
  ret = buf;
  return ret;  
}

namespace tools {
bool operator == (const Date& lh, const Date& rh)
{
  return (lh.m_year == rh.m_year && lh.m_month == rh.m_month && lh.m_day == rh.m_day);
}
bool operator != (const Date& lh, const Date& rh)
{
  return !(lh == rh);
}
bool operator > (const Date& lh, const Date& rh)
{
  return (lh.m_year > rh.m_year) || (lh.m_year == rh.m_year && lh.m_month > rh.m_month) || (lh.m_year == rh.m_year && lh.m_month == rh.m_month && lh.m_day > rh.m_day);
}
bool operator < (const Date& lh, const Date& rh)
{
  return (lh.m_year < rh.m_year) || (lh.m_year == rh.m_year && lh.m_month < rh.m_month) || (lh.m_year == rh.m_year && lh.m_month == rh.m_month && lh.m_day < rh.m_day);
}
bool operator >= (const Date& lh, const Date& rh)
{
  return !(lh < rh);
}
bool operator <= (const Date& lh, const Date& rh)
{
  return !(lh > rh);
}
}
#endif



