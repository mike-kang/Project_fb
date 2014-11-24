/*
      g++ test_db.cpp -lsqlite3 -L../tools/ -ltool -lrt
*/
#include <iostream>
#include <stdio.h>
#include "sqlite3.h"
#include "../tools/filesystem.h"
#include <map>
#include <string.h>

using namespace std;
using namespace tools;

#define USERDATA_SIZE 864

struct EmployeeInfo {
    //display
    std::string company_name;
    std::string lab_name;
    std::string pin_no;
    unsigned char userdata[USERDATA_SIZE];
    std::string blacklistinfo;
    int pnt_cnt;
    //char usercode[17];
    
    EmployeeInfo():pnt_cnt(0){};
    ~EmployeeInfo(){}
  };

sqlite3 * m_connection;
map<string, EmployeeInfo*> arrEmployee;
map<string, EmployeeInfo*> arrEmployeeInsert;
vector<EmployeeInfo*> arrEmployeeDelete;
map<string, EmployeeInfo*> arrEmployeeUpdate;

  
#define DB_NAME "employee.db"
#define CREATE_TABLE_EMPLOYEE "create table employee( company TEXT, name TEXT, pinno INTEGER, usercode TEXT, userdata BLOB, blacklistinfo TEXT, pntcnt INTEGER)"
#define CREATE_TABLE_TIME "create table time( lastsync TEXT );" \
                          "insert into time(lastsync) VALUES('')"
#define INSERT_EMPLOYEE "insert into employee(company, name, pinno, usercode, userdata, blacklistinfo, pntcnt) values (?,?,?,?,?,?,?)"
#define UPDATE_EMPLOYEE "update employee set company=?, name=?, usercode=?, userdata=?, blacklistinfo=?, pntcnt=? where pinno=?"
#define GET_ALL "select * from employee"
#define DELETE_EMPLOYEE "delete from employee where pinno=?"

int cbSelect(void* data, int ncols, char** values, char** headers)
{
  int i;
  cout << "value:" << values[0] << endl;
  return 0;
}

void insertToDB()
{
  int rc;

  sqlite3_stmt *stmt;
  const char* tail;

  rc = sqlite3_prepare_v2(m_connection, INSERT_EMPLOYEE, -1, &stmt, &tail);

  for(map<string, EmployeeInfo*>::iterator itr=arrEmployeeInsert.begin(); itr != arrEmployeeInsert.end(); itr++){
    EmployeeInfo* ei = itr->second;
    sqlite3_bind_text(stmt, 1, ei->company_name.c_str(), ei->company_name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ei->lab_name.c_str(), ei->lab_name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ei->pin_no.c_str(), ei->pin_no.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, itr->first.c_str(), itr->first.length(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 5, ei->userdata, USERDATA_SIZE, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, ei->blacklistinfo.c_str(), ei->blacklistinfo.length(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, ei->pnt_cnt);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
      printf("Failed to insert : %s\n", sqlite3_errmsg(m_connection));
      sqlite3_finalize(stmt);
      throw 1;
    }
    sqlite3_reset(stmt);

  }
  sqlite3_finalize(stmt);

}

void updateToDB()
{
  int rc;

  sqlite3_stmt *stmt;
  const char* tail;

  rc = sqlite3_prepare_v2(m_connection, UPDATE_EMPLOYEE, -1, &stmt, &tail);

  for(map<string, EmployeeInfo*>::iterator itr=arrEmployeeUpdate.begin(); itr != arrEmployeeUpdate.end(); itr++){
    EmployeeInfo* ei = itr->second;
    sqlite3_bind_text(stmt, 1, ei->company_name.c_str(), ei->company_name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ei->lab_name.c_str(), ei->lab_name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, itr->first.c_str(), itr->first.length(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 4, ei->userdata, USERDATA_SIZE, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, ei->blacklistinfo.c_str(), ei->blacklistinfo.length(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, ei->pnt_cnt);
    sqlite3_bind_text(stmt, 7, ei->pin_no.c_str(), ei->pin_no.length(), SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
      printf("Failed to insert : %s\n", sqlite3_errmsg(m_connection));
      sqlite3_finalize(stmt);
      throw 1;
    }
    sqlite3_reset(stmt);

  }
  sqlite3_finalize(stmt);

}

void dump(const char*str, unsigned char* buf, int length)
{
  printf("[%s]\n", str);
  if(!length){
    printf("size = 0\n");
    return;
  }
  for(int i=0; i<length; i++){
    printf("0x%02x ", buf[i]);
  }
  putchar('\n');
  
}

void dump(map<string, EmployeeInfo*>& arrEmployee)
{
  printf("size %d\n", arrEmployee.size());
  for(map<string, EmployeeInfo*>::iterator itr=arrEmployee.begin(); itr != arrEmployee.end(); itr++){
    EmployeeInfo* ei = itr->second;
    printf("company: %s\n", ei->company_name.c_str());
    printf("name :%s\n", ei->lab_name.c_str());
    printf("pinno :%s\n", ei->pin_no.c_str());
    printf("usercode :%s\n", itr->first.c_str());
    dump("userdata", ei->userdata, USERDATA_SIZE);
  }
}
void getFromDBToCache()
{
  char* err;
  sqlite3_stmt *stmt;
  const char* tail;
  
  int rc = sqlite3_prepare_v2(m_connection, GET_ALL, -1, &stmt, &tail);
  if (rc != SQLITE_OK) {
    printf("Failed to read: %s\n", err);
  }
  string usercode;
  while(sqlite3_step(stmt) == SQLITE_ROW){
    EmployeeInfo* ei = new EmployeeInfo();
    ei->company_name = (const char*)sqlite3_column_text(stmt , 0);
    ei->lab_name = (const char*)sqlite3_column_text(stmt , 1);
    ei->pin_no = (const char*)sqlite3_column_text(stmt , 2);
    usercode = (const char*)sqlite3_column_text(stmt , 3);
    memcpy(ei->userdata, sqlite3_column_blob(stmt,4), USERDATA_SIZE);
    ei->blacklistinfo = (const char*)sqlite3_column_text(stmt , 5);
    ei->pnt_cnt = sqlite3_column_int(stmt , 6);
    arrEmployee.insert(pair<string, EmployeeInfo*>(usercode, ei));
  }
  sqlite3_finalize(stmt);
  printf("size %d\n", arrEmployee.size());
  dump(arrEmployee);

}

void deleteRecords()
{
  int rc;

  sqlite3_stmt *stmt;
  const char* tail;

  rc = sqlite3_prepare_v2(m_connection, DELETE_EMPLOYEE, -1, &stmt, &tail);
  printf("delete size:%d\n", arrEmployeeDelete.size());
  for(vector<EmployeeInfo*>::size_type i=0; i < arrEmployeeDelete.size(); i++){
    EmployeeInfo* ei = arrEmployeeDelete[i];
    sqlite3_bind_text(stmt, 1, ei->pin_no.c_str(), ei->pin_no.length(), SQLITE_STATIC);
    printf("pin %s\n", ei->pin_no.c_str());
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
      printf("Failed to insert : %s\n", sqlite3_errmsg(m_connection));
      sqlite3_finalize(stmt);
      throw 1;
    }
    sqlite3_reset(stmt);

  }
  sqlite3_finalize(stmt);
    

}

int main()
{
  char* err;

  cout << "start main\n" << endl;
  int rc = sqlite3_open(DB_NAME,&m_connection);
  if (rc != SQLITE_OK) {
    printf("Failed to open database : %s\n", sqlite3_errmsg(m_connection));
  }
  printf("Opened db %s OK!\n", DB_NAME);
  
  int db_size = filesystem::file_size(DB_NAME);
  printf("db size: %d\n", db_size);
  if(!db_size){
    printf("create table\n");
    rc = sqlite3_exec(m_connection, CREATE_TABLE_EMPLOYEE, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
      printf("Failed to creat table : %s\n", err);
    }
    rc = sqlite3_exec(m_connection, CREATE_TABLE_TIME, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
      printf("Failed to creat table : %s\n", err);
    }
  }

  
  //read lastsync
  sqlite3_stmt *stmt;
  const char* tail;
  rc = sqlite3_prepare(m_connection, "select lastsync from time", -1, &stmt, &tail);
  if (rc != SQLITE_OK) {
    printf("Failed to read: %s\n", err);
  }
  if(sqlite3_step(stmt) == SQLITE_ROW){
    string lastsync = (const char*)sqlite3_column_text(stmt , 0);
    printf("lastsync:%s\n", lastsync.c_str());
  }
  sqlite3_finalize(stmt);
    
  //update lastsync
  rc = sqlite3_exec(m_connection, "update time set lastsync = '2014-11-22 13:04:00'", NULL, NULL, &err);
  if (rc != SQLITE_OK) {
    printf("Failed to update : %s\n", err);
  }

  //get all 
  getFromDBToCache();

  //insert records into employee
  EmployeeInfo e1;
  e1.company_name = "aaa";
  e1.lab_name = "강산";
  e1.pin_no = "1234";
  memcpy(e1.userdata, "xxxxxxx\0x", 9);
  e1.blacklistinfo = "no";
  e1.pnt_cnt = 1;
  
  EmployeeInfo e2;
  e2.company_name = "aaa";
  e2.lab_name = "baby1";
  e2.pin_no = "1235";
  memcpy(e2.userdata, "xxxxxxx\0x", 9);
  e2.blacklistinfo = "no";
  e2.pnt_cnt = 0;
  

  arrEmployeeInsert.insert(pair<string, EmployeeInfo*>("0000000000000013",&e1));
  arrEmployeeInsert.insert(pair<string, EmployeeInfo*>("0000000000000014",&e2));
  //insertToDB();
  
  //update
  EmployeeInfo e3;
  e3.company_name = "aava";
  e3.lab_name = "강산";
  e3.pin_no = "1234";
  memcpy(e3.userdata, "xxxxxxx\0x", 9);
  e3.blacklistinfo = "no";
  e3.pnt_cnt = 1;
  
  EmployeeInfo e4;
  e4.company_name = "a";
  e4.lab_name = "baby__a";
  e4.pin_no = "1235";
  memcpy(e4.userdata, "xxxxxxx\0xx", 10);
  e4.blacklistinfo = "no";
  e4.pnt_cnt = 2;
  

  arrEmployeeUpdate.insert(pair<string, EmployeeInfo*>("0000000000000013",&e3));
  arrEmployeeUpdate.insert(pair<string, EmployeeInfo*>("0000000000000014",&e4));
  updateToDB();

  //delete
  EmployeeInfo e5;
  e5.company_name = "aava";
  e5.lab_name = "강산";
  e5.pin_no = "1234";
  memcpy(e5.userdata, "xxxxxxx\0x", 9);
  e5.blacklistinfo = "no";
  e5.pnt_cnt = 1;
  
  EmployeeInfo e6;
  e6.company_name = "a";
  e6.lab_name = "baby__a";
  e6.pin_no = "1235";
  memcpy(e6.userdata, "xxxxxxx\0xx", 10);
  e6.blacklistinfo = "no";
  e6.pnt_cnt = 2;
  

  arrEmployeeDelete.push_back(&e5);
  arrEmployeeDelete.push_back(&e6);
  deleteRecords();

  
  return 0;
}

