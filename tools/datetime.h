#ifndef _DATETIME_HEADER
#define _DATETIME_HEADER
#include <iostream>

using namespace std;

namespace tools {
class DateTime {
  friend bool operator == (const DateTime& lh, const DateTime& rh);
  friend bool operator != (const DateTime& lh, const DateTime& rh);
  friend bool operator > (const DateTime& lh, const DateTime& rh);
  friend bool operator < (const DateTime& lh, const DateTime& rh);
  friend bool operator >= (const DateTime& lh, const DateTime& rh);
  friend bool operator <= (const DateTime& lh, const DateTime& rh);
public:
  DateTime();
  DateTime(int y, int mon, int d, int h, int min, int s):m_year(y), m_month(mon), 
  m_day(d), m_hour(h), m_minute(min), m_second(s){}
  DateTime(DateTime& datetime):m_year(datetime.m_year), m_month(datetime.m_month), 
  m_day(datetime.m_day), m_hour(datetime.m_hour), m_minute(datetime.m_minute), 
  m_second(datetime.m_second){}
  DateTime(char* datetime);
  //operator ==(
  void now();
  char* toString();
  char* toString(char);
private:
  int m_year;
  int m_month;
  int m_day;
  int m_hour;
  int m_minute;
  int m_second;
  char buf[30];
};
}
#endif //_DATE_HEADER
