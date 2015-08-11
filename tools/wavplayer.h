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
  ~WavPlayer();
  
  static WavPlayer* my;
};

}
}
#endif //_WAV_PLAYER_HEADER

