/*
      g++ test_db.cpp -lsqlite3 -L../tools/ -ltool -lrt
*/
#include <iostream>
#include <stdio.h>
#include "sqlite3.h"
#include "../tools/filesystem.h"


using namespace std;
using namespace tools;

#define DB_NAME "employee.db"
#define CREATE_TABLE_EMPLOYEE "create table employee( pinno INTEGER, usercode TEXT, userdata BLOB)"
#define CREATE_TABLE_TIME "create table time( lastsync TEXT )"

int main()
{
  sqlite3 * m_connection;
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
  return 0;
}

