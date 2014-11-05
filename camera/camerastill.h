#ifndef _CAMERA_STILL_HEADER
#define _CAMERA_STILL_HEADER

#include <iostream>
#include "../tools/mutex.h"
#include "../tools/timer.h"
#include "../tools/Semaphore.h"

class OMXILComponent;
class OMXILEncoderComponent;
class OMXILCameraComponent;

class CameraStill {
public:
#if 1  
  const static int CAM_WIDTH = 320;
  const static int CAM_HEIGHT = 240;
#else
  const static int CAM_WIDTH = 2592;
  const static int CAM_HEIGHT = 1944;
#endif
  const static int BUF_SIZE = 1024*1024;
  
  enum Except{
    EXCEPTION_OMX_INIT,
  };

  enum State {
    NONE_S,
    INIT_S,
    READY_S,
    ACQUIRING_S,
    IREADY_S,
    CAPTURING_S,
    RETURING_S
  };

  CameraStill(int close_delay_time);

  virtual ~CameraStill();
  
  bool takePicture(char** buf, int* len, int maxWaitTime); //blocking function 
  
private:  
  void run();
  bool takePictureReady();
  int m_delay_time; //sec
  OMXILCameraComponent* m_camera_component;
  OMXILComponent* m_preview_component;
  OMXILEncoderComponent* m_encoder_component;
  
  char m_imgBuf[BUF_SIZE];
  int m_imgLength;
  bool m_bReady;
  State m_state;
  static void cbEndOfFrame(int size, void*clientData);
  static void cbEndOfStream(void*clientData);
  Mutex mtx;
  tools::Timer* m_timer;
  Semaphore m_takePictureSem;
  static void cbTimer(void*);
  void returnResources();
  
};




#endif

