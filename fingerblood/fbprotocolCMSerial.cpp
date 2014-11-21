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
  try {
    return read(buf, len, timeout);
  }
  catch(FBProtocol::FBProtocolCommMethod::Exception e){
    throw FBProtocol::FBProtocolCommMethod::EXCEPTION_READ;
  }
}

/*
int FBProtocolCMSerial::onPoll(int timeout)
{
}
*/

