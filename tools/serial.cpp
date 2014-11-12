#include <iostream> 
#include <stdlib.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/signal.h> 
#include <sys/ioctl.h> 
#include <sys/poll.h> 
#include <termios.h> 
#include "serial.h"
#include "log.h"

#define LOG_TAG "Serial"

namespace tools {

bool Serial::open()
{
  struct termios newtio; 
  m_fd = ::open( m_dev_name, O_RDWR | O_NOCTTY ); 
  if ( m_fd < 0 ) 
  { 
  // 화일 열기 실패
  LOGE( "Device OPEN FAIL %s\n", m_dev_name ); 
  return false; 
  } 
  LOGV("open %s\n", m_dev_name);
  // 시리얼 포트 환경을 설정한다. 
  memset(&newtio, 0, sizeof(newtio)); 
  newtio.c_iflag = IGNPAR; // non-parity 
  newtio.c_oflag = 0; 
  newtio.c_cflag = CS8 | CLOCAL | CREAD; // NO-rts/cts 
  switch( m_baud ) 
  { 
    case SB115200: 
      newtio.c_cflag |= B115200; break; 
    //case 57600 : newtio.c_cflag |= B57600; break; 
    case SB38400: 
      newtio.c_cflag |= B38400; break; 
    //case 19200 : newtio.c_cflag |= B19200; break; 
    //case 9600 : newtio.c_cflag |= B9600; break; 
    //case 4800 : newtio.c_cflag |= B4800; break; 
    //case 2400 : newtio.c_cflag |= B2400; break; 
    default: 
      newtio.c_cflag |= B115200; break; 
  } 
  //set input mode (non-canonical, no echo,.....) 
  newtio.c_lflag = 0; 
  newtio.c_cc[VTIME] = m_vtime; // timeout 0.1초 단위
  newtio.c_cc[VMIN] = m_vmin; // 최소 n 문자 받을 때까진 대기
  tcflush ( m_fd, TCIFLUSH ); 
  tcsetattr( m_fd, TCSANOW, &newtio ); 

  return true; 
} 

/*
void Serial::run()
{
  struct pollfd fds;
  int ret;
  
  fds.fd = m_fd;
  fds.events = POLLIN;
  while(1){
    ret = poll(&fds, 1, -1);
    m_rn->onRead();
  }
}
*/
int Serial::close()
{
  return ::close(m_fd);
}

/*
void Serial::startRead(SerialReadNoti* rn)
{
  m_rn = rn;
  m_thread = new Thread<Serial>(&Serial::run, this, "SerialThread");
}
*/
int Serial::write(const byte* buf, int len)
{
  return ::write(m_fd, buf, len);
}
int Serial::read(byte* buf, int len)
{
  return ::read(m_fd, buf, len);
}

bool Serial::setTimeout(int timeout) //ms
{
  struct termios tp; 
  if(tcgetattr(m_fd, &tp) == -1)
    return false;
  tp.c_cc[VTIME] = timeout/100;
  if(tcsetattr(m_fd, TCSANOW, &tp) == -1)
    return false;
  return true;
}

};

