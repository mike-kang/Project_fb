#include "wavplayer.h"
#include "sys/wait.h"
//#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <alsa/asoundlib.h>
#include <fcntl.h>
#include "log.h"

using namespace tools;
using namespace media;

#define LOG_TAG "WavPlayer"

WavPlayer* WavPlayer::my = NULL;

//static
WavPlayer* WavPlayer::createInstance()
{
  if(my)
    return my;

  snd_pcm_t *handle;

  int rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {    
    LOGE("unable to open pcm device: %s\n", snd_strerror(rc));
    return NULL;
  }
  snd_pcm_hw_params_t *params;
  int dir;

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);
  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);  
  /* Set the desired hardware parameters. */
  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

  int factor = 2;
  /* channels */
  int channel = 1;
  snd_pcm_hw_params_set_channels(handle, params, channel);

  unsigned int sample_rate = 16000;
  snd_pcm_hw_params_set_rate_near(handle, params, &sample_rate, &dir);
  /* Set period size to 32 frames. */
  int frames = 1024;
  snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
  /* Write the parameters to the driver */
  int rc = snd_pcm_hw_params(handle, params);  
  if (rc < 0) {
    LOGE("unable to set hw parameters: %s\n", snd_strerror(rc));
    return NULL;
 }  
  
  my = new WavPlayer(handle);
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

WavPlayer::WavPlayer(snd_pcm_t *handle):m_handle(handle)
{
  printf("rate = %d bps\n", val);
  //size = frames * 2 * channel;
  /* 2 bytes/sample,  channels */
  size = 4096;
  buffer = (char *) malloc(size);
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
  


