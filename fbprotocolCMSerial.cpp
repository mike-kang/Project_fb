#include "fbprotocolCMSerial.h"
#include "tools/log.h"
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace tools;

void FBProtocolCMSerial::onWrite(const char* buf, int length)
{
  write(buf, length);
}
int FBProtocolCMSerial::onRead(char* buf, int len)
{
  return read(buf, len);
}


