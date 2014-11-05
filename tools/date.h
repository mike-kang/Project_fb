#ifndef _DATE_HEADER
#define _DATE_HEADER
#include <iostream>

using namespace std;

namespace tools {
class Date {
  friend bool operator == (const Date& lh, const Date& rh);
  friend bool operator != (const Date& lh, const Date& rh);
  friend bool operator > (const Date& lh, const Date& rh);
  friend bool operator < (const Date& lh, const Date& rh);
  friend bool operator >= (const Date& lh, const Date& rh);
  friend bool operator <= (const Date& lh, const Date& rh);
public:
  Date(){}
  Date(char* date);
  Date(int y, int m, int d):m_year(y), m_month(m), m_day(d){}
  Date(Date* date):m_year(date->m_year), m_month(date->m_month), m_day(date->m_day){}
  //operator ==(
  static Date* now();
  string toString();
private:
  int m_year;
  int m_month;
  int m_day;
  //int m_hour;
  //int m_minute;
  //int m_second;
};
}
#endif //_DATE_HEADER
