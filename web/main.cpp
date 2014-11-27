#include <iostream>
#include <fstream>
#include "safemanwebservice.h"
#include "mydwwebservice.h"
#include "../tools/log.h"

using namespace std;
using namespace web;

#define LOG_TAG "TEST_MAIN"

#define RCVHEADERBUFSIZE 1024
#define RCVBUFSIZE 4096


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

#define LOTT
//#define DW

#ifdef DW
const char pinno[] = "4716";
#else
const char pinno[] = "4321";
#endif

int main()
{
  IWebService* iws;

  log_init(true, 1, "/dev/pts/3", false, 3, "Log");
  //iws = new WebService("112.216.243.146", 8080);
  //iws = new SafemanWebService("http://lottedev.safeman.co.kr/LotteIDService.asmx", "MC00000007", "ST00000024", "0000000008"); //dev.safeman.co.kr
  //iws = new SafemanWebService("http://dev.safeman.co.kr/SafeIDService.asmx", "MC00000007", "ST00000024", "0000000008"); //dev.safeman.co.kr
#ifdef LOTT
  //iws = new SafemanWebService("http://lottedev.safeman.co.kr/LotteIDService.asmx", "MC00000007", "ST00000024", "0000000008", "1", 'I'); //dev.safeman.co.kr
  iws = new SafemanWebService("http://lottedev.safeman.co.kr/LotteIDService.asmx", "MC00000007", "ST00000024", "1000000022", "1", 'I'); //dev.safeman.co.kr
#elif defined DW
  DWWebService* iws1;
  SafemanWebService* iws2;
  iws1 = new DWWebService("http://112.175.10.40/WebService.asmx", "MC00000007", "ST00000024", "KMUD0", "1", 'I'); //dev.safeman.co.kr
  iws2 = new SafemanWebService("http://dev.safeman.co.kr/SafeIDService.asmx", "MC00000007", "ST00000024", "KMUD0", "1", 'I'); //dev.safeman.co.kr
  iws = new MyDWWebService(iws1, iws2);
#else
  iws = new SafemanWebService("http://dev.safeman.co.kr/SafeIDService.asmx", "MC00000007", "ST00000024", "0000000008", "1", 'I'); //dev.safeman.co.kr
#endif
  bool ret;
  char* xml_buf;
  char* time_buf;
  try{
    ret = iws->request_CheckNetwork(3000);  //blocked I/O
    //ret = m_ws->request_CheckNetwork(cbGetNetInfo, NULL);  //blocked I/O
    printf("***GetNetInfo: %d\n", ret);
  }
  catch(Except e){
    printf("request_CheckNetwork: %s\n", WebService::dump_error(e));
  }

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

  try{
    iws->request_EmployeeInfoAll("", 7000, "employee.xml");  //blocked I/O
  }
  catch(Except e){
    printf("request_EmployeeInfoAll: %s\n", WebService::dump_error(e));
  }
  
  try{
    xml_buf = iws->request_EmployeeInfo(pinno, 3000);  //blocked I/O
    //xml_buf = m_ws->request_EmployeeInfo("MC00000003", "ST00000005", "253153215009", cbRfidInfoSelect, NULL);  //blocked I/O
    if(xml_buf){
      cout << "***RfidInfoSelect: " << endl;
      cout <<  xml_buf << endl;
      delete xml_buf;
    }
  }
  catch(Except e){
    printf("Except: request_EmployeeInfo: %s\n", WebService::dump_error(e));
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
  */

  try{
    ret = iws->request_UploadTimeSheet("2014-11-19 09:00:00", pinno, 8000, "timesheets");  //blocked I/O
    //ret = m_ws->request_UploadTimeSheet("MC00000003", "ST00000005", "LM00000811", 'I', "1", "0001",'L', "2014-10-18+09:00:00", image_buffer, size, TimeSheetInsertString, NULL, "timesheets");  //blocked I/O
    printf("***request_UploadTimeSheet: %d\n", ret);
  }
  catch(Except e){
    printf("request_UploadTimeSheet: %s\n", WebService::dump_error(e));
  }

  printf("end!\n");
  
  while(1)
    sleep(1);

  return 0;
}

