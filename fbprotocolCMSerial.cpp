#include "fbprotocolCMSerial.h"
#include "tools/log.h"
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace tools;

int FBProtocolCMSerial::onWrite(const char* buf, int length)
{
  return write(buf, length);
}
int FBProtocolCMSerial::onRead(char* buf, int len)
{
  return read(buf, len);
}


