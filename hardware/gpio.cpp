#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h> 

#include <sys/types.h>
#include <sys/stat.h>
#include "gpio.h"
#include "tools/log.h"
#include "tools/filesystem.h"
#include "tools/utils.h"

#define LOG_TAG "Gpio"

#define EXPORT_PATH "/sys/class/gpio/export"
#define UNEXPORT_PATH "/sys/class/gpio/unexport"
#define PREFIX_PATH "/sys/class/gpio/gpio"
using namespace tools;

Gpio::Gpio(int num, bool out, bool init): m_number(num), m_out(out)
{
  int fd;
  char path[45];
  LOGV("Gpio +++\n");
  sprintf(path, "%s%d", PREFIX_PATH, num);
  if(!filesystem::file_exist(path)){
    fd = open(EXPORT_PATH, O_WRONLY);
    char* strnum = utils::itoa(num, 10);
    ::write(fd, strnum, strlen(strnum));
    close(fd);
  }
  char* p = path + strlen(path);
  strcat(path, "/direction");
  fd = open(path, O_WRONLY);
  ::write(fd, out? "out":"in", out? 3:2);
  close(fd);

  strcpy(p, "/value");
  if(out)
    m_fd = open(path, O_WRONLY);
  else
    m_fd = open(path, O_RDONLY);

  //set init value
  write(init);
  
  LOGV("Gpio %d ---\n", m_fd);
}

Gpio::~Gpio()
{
  int fd;
  char path[45];
  
  write(false);
  close(m_fd);

  sprintf(path, "%s%d", PREFIX_PATH, m_number);
  if(!filesystem::file_exist(path))
    return;

  fd = open(UNEXPORT_PATH, O_WRONLY);
  char* strnum = utils::itoa(m_number, 10);
  ::write(fd, strnum, strlen(strnum));
  close(fd);
}

void Gpio::write(bool v)
{
  ::write(m_fd, v? "1":"0", 1);
}
bool Gpio::read()
{
  char buf;
  ::read(m_fd, &buf, 1);
  return buf;
}


