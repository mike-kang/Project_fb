#include "fbservice.h"
#include "tools/log.h"
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace tools;

#define LOG_TAG "FBService"

FBService::FBService(const char* path, Serial::Baud baud)
{
  m_serial = new FBProtocolCMSerial(path, baud);
  m_protocol = new FBProtocol(m_serial);
}

bool FBService::start()
{
  m_serial->open();
  try{
    char* version = m_protocol->vers();
    cout << version << endl;
    return true;
  }
  catch (FBProtocol::Exception e){
    cout << "error" << endl;
  }
  return false;
}

