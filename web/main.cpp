#include <iostream>
#include <fstream>
#include "webservice.h"
#include "../tools/log.h"

using namespace std;

#define LOG_TAG "TEST_MAIN"

#define RCVHEADERBUFSIZE 1024
#define RCVBUFSIZE 4096

WebService* m_ws;

#define DUMP_CASE(x) case x: return #x;

char* image_buffer;

char* dump_error(WebService::Except e)
{
  switch(e){
    DUMP_CASE (WebService::EXCEPTION_CREATE_SOCKET)
    DUMP_CASE (WebService::EXCEPTION_CONNECT)
    DUMP_CASE (WebService::EXCEPTION_SEND_COMMAND)
    DUMP_CASE (WebService::EXCEPTION_POLL_FAIL)
    DUMP_CASE (WebService::EXCEPTION_POLL_TIMEOUT)
    DUMP_CASE (WebService::EXCEPTION_PARSING_FAIL)
  }

}

void cbGetNetInfo(void *client_data, int status, void* ret)
{
  LOGV("cbGetNetInfo status:%d, ret:%d\n", status, *((bool*)ret));
}

void cbCodeDataSelect(void *client_data, int status, void* ret)
{
  char* xml_buf = (char*)ret;
  LOGV("cbCodeDataSelect status:%d, ret:%s\n", status, xml_buf);
  cout << "***cbCodeDataSelect: " << xml_buf << endl;
  delete xml_buf;
}

void cbRfidInfoSelectAll(void *client_data, int status, void* ret)
{
  LOGV("cbRfidInfoSelectAll status:%d\n", status);
}

void cbRfidInfoSelect(void *client_data, int status, void* ret)
{
  char* xml_buf = (char*)ret;
  LOGV("cbRfidInfoSelect status:%d, ret:%s\n", status, xml_buf);
  cout << "***cbRfidInfoSelect: " << xml_buf << endl;
  delete xml_buf;
}

void cbServerTimeGet(void *client_data, int status, void* ret)
{
  char* time_buf = (char*)ret;
  LOGV("cbServerTimeGet status:%d, ret:%s\n", status, time_buf);
  delete time_buf;
}

void cbStatusUpdate(void *client_data, int status, void* ret)
{
  LOGV("cbStatusUpdate status:%d, ret:%d\n", status, *((bool*)ret));
}

void cbTimeSheetInsertString(void *client_data, int status, void* ret)
{
  LOGV("cbTimeSheetInsertString status:%d, ret:%d\n", status, *((bool*)ret));
  if(*((bool*)ret))
    delete image_buffer;
}

int main()
{
  log_init(TYPE_CONSOLE, "/dev/pts/3");

  
  m_ws = new WebService("112.216.243.146", 8080);
  //m_ws = new WebService("192.168.0.7", 8080);

  bool ret;
  char* xml_buf;
  char* time_buf;
  try{
    //xml_buf = m_ws->request_CodeDataSelect("MC00000003", "ST00000005", "0001", 3000);  //blocked I/O
    xml_buf = m_ws->request_CodeDataSelect("MC00000003", "ST00000005", "0001", cbCodeDataSelect, NULL);  //blocked I/O
    if(xml_buf){
      cout << "***CodeDataSelect: " << xml_buf << endl;
      delete xml_buf;
    }
  }
  catch(WebService::Except e){
    LOGE("request_CodeDataSelect: %s\n", dump_error(e));
  }
/*
  try{
    //ret = m_ws->request_GetNetInfo(3000);  //blocked I/O
    ret = m_ws->request_GetNetInfo(cbGetNetInfo, NULL);  //blocked I/O
    LOGV("***GetNetInfo: %d\n", ret);
  }
  catch(WebService::Except e){
    LOGE("request_GetNetInfo: %s\n", dump_error(e));
  }

  try{
    m_ws->request_RfidInfoSelectAll("MC00000003", "ST00000005", 7000, "employee.xml");  //blocked I/O
    //m_ws->request_RfidInfoSelectAll("MC00000003", "ST00000005", cbRfidInfoSelectAll, NULL, "employee.xml");  //blocked I/O
  }
  catch(WebService::Except e){
    LOGE("request_RfidInfoSelectAll: %s\n", dump_error(e));
  }

  try{
    xml_buf = m_ws->request_RfidInfoSelect("MC00000003", "ST00000005", "253153215009", 3000);  //blocked I/O
    //xml_buf = m_ws->request_RfidInfoSelect("MC00000003", "ST00000005", "253153215009", cbRfidInfoSelect, NULL);  //blocked I/O
    if(xml_buf){
      cout << "***RfidInfoSelect: " << xml_buf << endl;
      delete xml_buf;
    }
  }
  catch(WebService::Except e){
    LOGE("request_RfidInfoSelect: %s\n", dump_error(e));
  }
*/
  try{
    time_buf = m_ws->request_ServerTimeGet(3000);  //blocked I/O
    //time_buf = m_ws->request_ServerTimeGet(cbServerTimeGet, NULL);  //blocked I/O
    if(time_buf){
      cout << "***ServerTimeGet: " << time_buf << endl;
      delete time_buf;
    }
  }
  catch(WebService::Except e){
    LOGE("request_ServerTimeGet: %s\n", dump_error(e));
  }
/*
  try{
    ret = m_ws->request_StatusUpdate("IN", "ST00000005", "0001", "1", "192.168.190.130", "00-0c-29-95-30-24", 3000);  //blocked I/O
    //ret = m_ws->request_StatusUpdate("IN", "ST00000005", "0001", "1", "192.168.190.130", "00-0c-29-95-30-24", cbStatusUpdate, NULL);  //blocked I/O
    LOGV("***StatusUpdate: %d\n", ret);
  }
  catch(WebService::Except e){
    LOGE("request_StatusUpdate: %s\n", dump_error(e));
  }
  ifstream infile ("org.jpg",ofstream::binary);
  // get size of file
  infile.seekg (0,infile.end);
  long size = infile.tellg();
  infile.seekg (0);
  // allocate memory for file content
  image_buffer = new char[size];
  // read content of infile
  infile.read (image_buffer,size);
  infile.close();

  try{
    ret = m_ws->request_TimeSheetInsertString("MC00000003", "ST00000005", "LM00000811", 'I', "1", "0001",'L', "2014-10-18+09:00:00", image_buffer, size, 8000, "timesheets");  //blocked I/O
    if(ret)
      delete image_buffer;
    //ret = m_ws->request_TimeSheetInsertString("MC00000003", "ST00000005", "LM00000811", 'I', "1", "0001",'L', "2014-10-18+09:00:00", image_buffer, size, TimeSheetInsertString, NULL, "timesheets");  //blocked I/O
    LOGV("***request_TimeSheetInsertString: %d\n", ret);
  }
  catch(WebService::Except e){
    LOGE("request_TimeSheetInsertString: %s\n", dump_error(e));
  }
  */

  
  while(1)
    sleep(1);

  return 0;
}

