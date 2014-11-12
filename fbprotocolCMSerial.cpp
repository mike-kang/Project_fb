#include "fbprotocolCMSerial.h"
#include "tools/log.h"
#include <stdio.h>
#include <iostream>
#include <sys/poll.h> 

using namespace std;
using namespace tools;

int FBProtocolCMSerial::onWrite(const byte* buf, int length)
{
  return write(buf, length);
}

int FBProtocolCMSerial::onRead(byte* buf, int len, int timeout)
{
  return read(buf, len, timeout);
}

/*
int FBProtocolCMSerial::onPoll(int timeout)
{
}
*/

