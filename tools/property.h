#ifndef _PROPERTY_HEADER
#define _PROPERTY_HEADER

#include <iostream>

namespace tools{
class Property
{
public:
  static void init();
  static int getPropertyInt(const char* category, const char* name, int Default);
  static float getPropertyFloat(const char* category, const char* name, float Default);
  static void getPropertyStr(const char* category, const char* name, const char *def, char *buf, int size);
  static bool getPropertyBool(const char* category, const char* name, bool def);
  static bool setProperty(const char* category, const char* name, int Def);
  static bool setProperty(const char* category, const char* name, float val);
  static bool setProperty(const char* category, const char* name, char* Default);
  static bool setProperty(const char* category, const char* name, bool val);
};
}
#endif //_IRISDB_HEADER
