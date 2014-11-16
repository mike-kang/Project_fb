#include <iostream>
#include <fstream>
#include "safemanwebservice.h"
#include "../tools/log.h"

using namespace std;
using namespace web;
#define LOG_TAG "TEST_MAIN"

#define RCVHEADERBUFSIZE 1024
#define RCVBUFSIZE 4096

IWebService* iws;

#define DUMP_CASE(x) case x: return #x;

char* image_buffer;


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
  log_init(true, 1, "/dev/pts/2", false, 3, "Log");
  //iws = new WebService("112.216.243.146", 8080);
  iws = new SafemanWebService("125.141.204.31", 80); //dev.safeman.co.kr
  //m_ws = new WebService("192.168.0.7", 8080);

  bool ret;
  char* xml_buf;
  char* time_buf;
/*
  try{
    //ret = m_ws->request_CheckNetwork(3000);  //blocked I/O
    ret = m_ws->request_CheckNetwork(cbGetNetInfo, NULL);  //blocked I/O
    LOGV("***GetNetInfo: %d\n", ret);
  }
  catch(WebService::Except e){
    LOGE("request_CheckNetwork: %s\n", dump_error(e));
  }

  try{
    m_ws->request_EmployeeInfoAll("MC00000003", "ST00000005", 7000, "employee.xml");  //blocked I/O
    //m_ws->request_EmployeeInfoAll("MC00000003", "ST00000005", cbRfidInfoSelectAll, NULL, "employee.xml");  //blocked I/O
  }
  catch(WebService::Except e){
    LOGE("request_EmployeeInfoAll: %s\n", dump_error(e));
  }

  try{
    xml_buf = m_ws->request_EmployeeInfo("MC00000003", "ST00000005", "253153215009", 3000);  //blocked I/O
    //xml_buf = m_ws->request_EmployeeInfo("MC00000003", "ST00000005", "253153215009", cbRfidInfoSelect, NULL);  //blocked I/O
    if(xml_buf){
      cout << "***RfidInfoSelect: " << xml_buf << endl;
      delete xml_buf;
    }
  }
  catch(WebService::Except e){
    LOGE("request_EmployeeInfo: %s\n", dump_error(e));
  }
*/
  try{
    time_buf = iws->request_ServerTime(3000);  //blocked I/O
    //time_buf = m_ws->request_ServerTime(cbServerTimeGet, NULL);  //blocked I/O
    if(time_buf){
      cout << "***ServerTimeGet: " << time_buf << endl;
      delete time_buf;
    }
  }
  catch(Except e){
    LOGE("request_ServerTime: %s\n", WebService::dump_error(e));
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
    ret = m_ws->request_UploadTimeSheet("MC00000003", "ST00000005", "LM00000811", 'I', "1", "0001",'L', "2014-10-18+09:00:00", image_buffer, size, 8000, "timesheets");  //blocked I/O
    if(ret)
      delete image_buffer;
    //ret = m_ws->request_UploadTimeSheet("MC00000003", "ST00000005", "LM00000811", 'I', "1", "0001",'L', "2014-10-18+09:00:00", image_buffer, size, TimeSheetInsertString, NULL, "timesheets");  //blocked I/O
    LOGV("***request_UploadTimeSheet: %d\n", ret);
  }
  catch(WebService::Except e){
    LOGE("request_UploadTimeSheet: %s\n", dump_error(e));
  }
  */

  
  while(1)
    sleep(1);

  return 0;
}

