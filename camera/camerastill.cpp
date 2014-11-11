#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <bcm_host.h>
#include <interface/vcos/vcos.h>
#include "camerastill.h"
#include "log.h"
#include "dump.h"
#include "omxilComponent.h"

#define LOG_TAG "CameraStill"

#define OMX_INIT_STRUCTURE(a) \
  memset (&(a), 0, sizeof (a)); \
  (a).nSize = sizeof (a); \
  (a).nVersion.nVersion = OMX_VERSION; \
  (a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
  (a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
  (a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
  (a).nVersion.s.nStep = OMX_VERSION_STEP

#define NULLSINK_COMPONENT_NAME "OMX.broadcom.null_sink"

//static int verbos = 1;

CameraStill::CameraStill(int close_delay_time):m_delay_time(close_delay_time), m_bReady(false), m_takePictureSem(0)
{
  OMX_ERRORTYPE error;
  //Initialize Broadcom's VideoCore APIs
  bcm_host_init ();
  
  //Initialize OpenMAX IL
  if ((error = OMX_Init ())){
    LOGE("error: OMX_Init:%d\n", error);
    throw EXCEPTION_OMX_INIT;
  }   

  
  m_encoder_component = new OMXILEncoderComponent(CAM_WIDTH, CAM_HEIGHT, m_imgBuf, BUF_SIZE, cbEndOfFrame, cbEndOfStream, this);
  m_encoder_component->set_output_port();;
  m_encoder_component->set_jpeg_settings();
  m_encoder_component->change_state(OMX_StateIdle); //blocking

  m_preview_component = new OMXILComponent(NULLSINK_COMPONENT_NAME);
  m_preview_component->change_state(OMX_StateIdle); //blocking

  m_timer = new tools::Timer(cbTimer, this);

  m_state = READY_S;

  LOGV("CameraStill Constructed\n");
}

CameraStill::~CameraStill()
{
  m_encoder_component->disable_port(OMXILEncoderComponent::OUTPUT_PORT_NUM);
  m_preview_component->change_state(OMX_StateLoaded); //blocking
  m_encoder_component->change_state(OMX_StateLoaded); //blocking

  delete m_preview_component;
  delete m_encoder_component;

  OMX_ERRORTYPE error;
  if ((error = OMX_Deinit ())){
    LOGE("error: OMX_Deinit: %s\n", dump_OMX_ERRORTYPE (error));
  }
  
  //Deinitialize Broadcom's VideoCore APIs
  bcm_host_deinit ();

}

//static //this function is called from timer thread.
void CameraStill::cbTimer(void* clientData)
{
  CameraStill* cs = (CameraStill*)clientData;
  LOGI("cbTimer \n");
  cs->returnResources();
}

bool CameraStill::takePicture(char** buf, int* len, int maxWaitTime)
{
  LOGV("takePicture\n");
  mtx.lock();
  if(m_timer->IsActive())
    m_timer->stop();
  if(!takePictureReady()){
    LOGE("takePictureReady fail!\n");
    return false;
  }
  m_camera_component->capture(true);
  m_encoder_component->capture();
  mtx.unlock();
  m_takePictureSem.reset();
  int ret = m_takePictureSem.timedwait(maxWaitTime);  //blocking for maxWaitTime.
  LOGV("takePicture end waiting\n");
  if(ret < 0)
    return false;
  *buf = m_imgBuf;
  *len = m_imgLength;
  return true;
}

//static //this function is called from encoderComponent thread.
void CameraStill::cbEndOfFrame(int size, void* clientData)
{
  LOGV("cbEndOfFrame %d\n", size);
  CameraStill* cs = (CameraStill*)clientData;
  cs->m_imgLength = size;
  
  LOGV("cbEndOfFrame post\n");
  cs->m_takePictureSem.post();
  
}

//static //this function is called from encoderComponent thread.
void CameraStill::cbEndOfStream(void* clientData)
{
  LOGV("cbEndOfStream\n");
  CameraStill* cs = (CameraStill*)clientData;
  cs->mtx.lock();
  cs->m_camera_component->capture(false);
  cs->m_timer->start(cs->m_delay_time);
  cs->mtx.unlock();
}


bool CameraStill::takePictureReady()
{
  if(m_state == IREADY_S) return true;

  try{
    m_camera_component = new OMXILCameraComponent(CAM_WIDTH, CAM_HEIGHT);
    m_camera_component->load_camera_drivers();
    
    m_camera_component->change_state(OMX_StateIdle); //blocking
    
    m_camera_component->configure_sensor_settings();
    m_camera_component->set_output_port();
    m_camera_component->set_camera_settings();
    
    //Setup tunnels: camera (still) -> image_encode, camera (preview) -> null_sink
    if(!OMXILComponent::connect(m_camera_component, OMXILCameraComponent::OUTPUT_CAPTURE_PORT_NUM, m_encoder_component, OMXILEncoderComponent::INPUT_PORT_NUM)){
      return false;
    }
    if(!OMXILComponent::connect(m_camera_component, OMXILCameraComponent::OUTPUT_PREVIEW_PORT_NUM, m_preview_component, 240)){
      return false;
    }
    
    //Enable the tunnel ports
    m_camera_component->enable_port (OMXILCameraComponent::OUTPUT_CAPTURE_PORT_NUM);
    m_camera_component->enable_port (OMXILCameraComponent::OUTPUT_PREVIEW_PORT_NUM);
    m_preview_component->enable_port (240);
    m_encoder_component->enable_port (OMXILEncoderComponent::INPUT_PORT_NUM);
    m_encoder_component->enable_port (OMXILEncoderComponent::OUTPUT_PORT_NUM);
    
    LOGV("Change state to EXECUTING\n");
    //Change state to EXECUTING
    m_camera_component->change_state(OMX_StateExecuting); //blocking
    m_preview_component->change_state(OMX_StateExecuting); //blocking
    m_encoder_component->change_state(OMX_StateExecuting); //blocking
    
    m_state = IREADY_S;
    return true;
  }
  catch(OMXILComponent::Except e){
    return false;
  }
  
}

void CameraStill::returnResources()
{
  LOGV("returnResources\n");
  mtx.lock();
  m_camera_component->change_state(OMX_StateIdle); //blocking
  m_encoder_component->change_state(OMX_StateIdle); //blocking
  m_preview_component->change_state(OMX_StateIdle); //blocking

  m_camera_component->disable_port (OMXILCameraComponent::OUTPUT_CAPTURE_PORT_NUM);
  m_camera_component->disable_port (OMXILCameraComponent::OUTPUT_PREVIEW_PORT_NUM);
  m_preview_component->disable_port (240);
  m_encoder_component->disable_port (OMXILEncoderComponent::INPUT_PORT_NUM);
  m_encoder_component->disable_port (OMXILEncoderComponent::OUTPUT_PORT_NUM);

  m_camera_component->change_state(OMX_StateLoaded); //blocking
  delete m_camera_component;
  m_state = READY_S;

  mtx.unlock();
}


