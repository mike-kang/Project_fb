#include "wavplayer.h"
#include "sys/wait.h"
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "log.h"
#include "wavplayeralsa.h"

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

//SND_PCM_ACCESS_RW_INTERLEAVED
//SND_PCM_FORMAT_S16_LE
//MONO
//sample rate = 44100
WavPlayer::WavPlayer()
{
  if(alsa_init()){
    LOGE("init fail\n");
    throw 1;
  }
}

WavPlayer::~WavPlayer()
{
  alsa_destroy();
}

void WavPlayer::play(const char* filename)
{
  //printf("WavPlayer::play\n");
  alsa_play(filename);
}

void WavPlayer::stop()
{
  LOGE("stop\n");
  alsa_stop();
}
  


