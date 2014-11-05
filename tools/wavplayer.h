#ifndef _WAV_PLAYER_HEADER
#define _WAV_PLAYER_HEADER
#include <sys/types.h>

namespace tools {
namespace media {
class WavPlayer {
public:
  static WavPlayer* createInstance();
  void play(const char* filename);
  void stop();
  
private:
  WavPlayer();
  static void signal_handler(int signo);
  
  static WavPlayer* my;
  pid_t m_AudioPid;
  //snd_pcm_t *m_handle;
};

}
}
#endif //_WAV_PLAYER_HEADER

