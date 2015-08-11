#include "sys/wait.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <alsa/asoundlib.h>
#include <pthread.h>

#define FRAME_SIZE 2

struct _header_file {
     char type[4];    //"RIFF"
     int length;
     char format[4];   //"WAVE"
};
struct _header_wave {
     char type[4];  //"fmt "
     int length; // 0x10
     short format; // 0x01
     short channels;
     int sample_rate; //hz
     int byte_rate;
     short frame_size;
     short bit_per_sample;
};
struct _header_data {
     char type[4];  //"data"
     int length;
};
struct _header {
     struct _header_file h1;
     struct _header_wave h2;
     struct _header_data h3;
};   

static char *buf = NULL;
static int buf_size = 0;
static char bPlaying = 0;
static snd_pcm_t *m_playback_handle;
static snd_pcm_hw_params_t *m_hw_params;


//SND_PCM_ACCESS_RW_INTERLEAVED
//SND_PCM_FORMAT_S16_LE
//MONO
//sample rate = 44100
int alsa_init()
{
	int err;

  unsigned int sample_rate = 44100;
  int dir;
  snd_pcm_uframes_t frames;
  int channels = 1;

	if ((err = snd_pcm_open (&m_playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		printf("cannot open audio device default (%s)\n", snd_strerror (err));
    return 1;
	}

  if ((err = snd_pcm_hw_params_malloc (&m_hw_params)) < 0) {
		printf("cannot allocate hardware parameter structure (%s)\n", snd_strerror (err));
		return 1;
	}

	if ((err = snd_pcm_hw_params_any (m_playback_handle, m_hw_params)) < 0) {
		printf( "cannot initialize hardware parameter structure (%s)\n", snd_strerror (err));
		return 1;
	}

	if ((err = snd_pcm_hw_params_set_access (m_playback_handle, m_hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		printf( "cannot set access type (%s)\n", snd_strerror (err));
		return 1;
	}
	if ((err = snd_pcm_hw_params_set_format (m_playback_handle, m_hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		printf( "cannot set sample format (%s)\n", snd_strerror (err));
		return 1;
	}
  
	if ((err = snd_pcm_hw_params_set_channels (m_playback_handle, m_hw_params, channels)) < 0) {
		printf( "cannot set channel count (%s)\n", snd_strerror (err));
		return 1;
	}

  if ((err = snd_pcm_hw_params_set_rate_near (m_playback_handle, m_hw_params, &sample_rate, NULL)) < 0) {
    printf( "cannot set sample rate (%s)\n", snd_strerror (err));
    return 1;
  }
  printf("sample_rate=%d\n", sample_rate);
  printf("dir=%d\n", dir);

  /* Set period size to 32 frames. */
  frames = 32;
  snd_pcm_hw_params_set_period_size_near(m_playback_handle, m_hw_params, &frames, NULL);
  printf("period size=%d frames\n", (int)frames);

	if ((err = snd_pcm_hw_params (m_playback_handle, m_hw_params)) < 0) {
		printf( "cannot set parameters (%s)\n", snd_strerror (err));
		return 1;
	}

  //snd_pcm_hw_params_get_period_size(m_hw_params, &frames, &dir);

  buf_size = frames * 2 * 1;
  buf = (char*)malloc(buf_size);

  return 0;
}

void* run(void* arg);

void alsa_play(const char* filename)
{
  pthread_t threadId;
  bPlaying = 1;
  pthread_create(&threadId, NULL, run, (void*)filename); 
  pthread_detach(threadId);
}

void* run(void* arg)
{
  int err;
  struct _header header;
  FILE* fp = fopen((const char*)arg, "r");
  fread(&header, sizeof(struct _header), 1, fp);
  //printf("sample rate %d\n", header.h2.sample_rate);
  //printf("header size = %d\n", sizeof(struct _header)); 
  //printf("data length = %d\n", header.h3.length);

	if ((err = snd_pcm_prepare (m_playback_handle)) < 0) {
		printf("cannot prepare audio interface for use (%s)\n", snd_strerror (err));
    return NULL;
	}

  //int shouldReadByte = header.h3.length;
  int leavedByte = header.h3.length;
  while(bPlaying && leavedByte > 0){
    int count = fread(buf, 1, buf_size, fp);
    int write_frame = count / FRAME_SIZE;
		if ((err = snd_pcm_writei (m_playback_handle, buf, write_frame)) != write_frame) {
			printf("write to audio interface failed (%s)\n", snd_strerror (err));
		}
    leavedByte -= count;
    //printf("leavedByte = %d\n", leavedByte);
  }

  if(bPlaying)
    snd_pcm_drain(m_playback_handle);
  fclose(fp);

  return NULL;
}

void alsa_stop()
{
  bPlaying = 0;
}

void alsa_destroy()
{
	snd_pcm_hw_params_free (m_hw_params);
	snd_pcm_close(m_playback_handle);
  free(buf);
}
