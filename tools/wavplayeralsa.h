#ifndef _WAV_PLAYER_ALSA_HEADER
#define _WAV_PLAYER_ALSA_HEADER
#ifdef __cplusplus
extern "C" {
#endif

int alsa_init();
void alsa_play(const char* filename);
void alsa_stop();
void alsa_destroy();

#ifdef __cplusplus
}
#endif

#endif
