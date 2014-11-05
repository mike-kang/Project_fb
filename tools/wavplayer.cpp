#include "wavplayer.h"
#include "sys/wait.h"
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "log.h"

using namespace tools;
using namespace media;

#define LOG_TAG "WavPlayer"

WavPlayer* WavPlayer::my = NULL;

//static
WavPlayer* WavPlayer::createInstance()
{
  if(!my)
    my = new WavPlayer();
  return my;
}

//static
void WavPlayer::signal_handler(int signo)
{
  int status;
  if(signo == SIGCHLD){
    LOGV("signal_handler SIGCHLD\n");
    wait(&status);
    my->m_AudioPid = 0;
  }
}

WavPlayer::WavPlayer():m_AudioPid(0)
{
  signal(SIGCHLD, signal_handler);
}

void WavPlayer::play(const char* filename)
{
  int pid;
  if((pid = fork()) == -1){
    LOGE("fork fail:%s\n", strerror(errno));
    return;
  }
  if(!pid){ //child
    printf("aplay %s\n", filename);
    char* const args[] = { "aplay", (char*)filename, NULL};
    int ret = execv("/usr/bin/aplay", args);
    if(ret == -1){
      printf("execv fail:%s(%d)\n", strerror(errno), errno);
      exit(EXIT_FAILURE);
    }
  }
  else{ //parent
    m_AudioPid = pid;
    
  }
}

void WavPlayer::stop()
{
  LOGE("stop:%d\n", m_AudioPid);

  if(m_AudioPid)
    kill(m_AudioPid, SIGTERM);
}
  


