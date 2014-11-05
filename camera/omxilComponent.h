#ifndef _OMXIL_COMPONENT_HEADER
#define _OMXIL_COMPONENT_HEADER

#include <IL/OMX_Broadcom.h>
#include <iostream>

#define JPEG_QUALITY 75 //1 .. 100
#define JPEG_EXIF_DISABLE OMX_FALSE
#define JPEG_IJG_ENABLE OMX_FALSE
#define JPEG_THUMBNAIL_ENABLE OMX_TRUE
#define JPEG_THUMBNAIL_WIDTH 64 //0 .. 1024
#define JPEG_THUMBNAIL_HEIGHT 48 //0 .. 1024
#define JPEG_PREVIEW OMX_FALSE

#define RAW_BAYER OMX_FALSE

#define CAM_SHARPNESS 0 //-100 .. 100
#define CAM_CONTRAST 0 //-100 .. 100
#define CAM_BRIGHTNESS 50 //0 .. 100
#define CAM_SATURATION 0 //-100 .. 100
#define CAM_SHUTTER_SPEED_AUTO OMX_TRUE
//In microseconds, (1/8)*1e6
#define CAM_SHUTTER_SPEED 125000 //1 ..
#define CAM_ISO_AUTO OMX_TRUE
#define CAM_ISO 100 //100 .. 800
#define CAM_EXPOSURE OMX_ExposureControlAuto
#define CAM_EXPOSURE_COMPENSATION 0 //-24 .. 24
#define CAM_MIRROR OMX_MirrorNone
#define CAM_ROTATION 0 //0 90 180 270
#define CAM_COLOR_ENABLE OMX_FALSE
#define CAM_COLOR_U 128 //0 .. 255
#define CAM_COLOR_V 128 //0 .. 255
#define CAM_NOISE_REDUCTION OMX_TRUE
#define CAM_FRAME_STABILIZATION OMX_FALSE
#define CAM_METERING OMX_MeteringModeAverage
#define CAM_WHITE_BALANCE OMX_WhiteBalControlAuto
//The gains are used if the white balance is set to off
#define CAM_WHITE_BALANCE_RED_GAIN 1000 //0 ..
#define CAM_WHITE_BALANCE_BLUE_GAIN 1000 //0 ..
#define CAM_IMAGE_FILTER OMX_ImageFilterNone
#define CAM_ROI_TOP 0 //0 .. 100
#define CAM_ROI_LEFT 0 //0 .. 100
#define CAM_ROI_WIDTH 100 //0 .. 100
#define CAM_ROI_HEIGHT 100 //0 .. 100
#define CAM_DRC OMX_DynRangeExpOff

typedef OMX_ERRORTYPE (*FillBufferDoneType) (
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

class OMXILComponent {
public:
  enum Except{
    EXCEPTION_CONSTRUCT,
  };
  
  OMXILComponent(const char* name, FillBufferDoneType fillbufferdone=NULL);
  virtual ~OMXILComponent();

  static bool connect(OMXILComponent* cmpA, int portA, OMXILComponent* cmpB, int 
  portB);
  virtual bool enable_port (OMX_U32 port);
  virtual bool disable_port (OMX_U32 port);
  bool change_state (OMX_STATETYPE state);

protected:
  OMX_HANDLETYPE m_handle;
  char m_name[30];
  sem_t sem_EventComplete;

private:
  bool disable_allport(OMX_HANDLETYPE handle);
  static OMX_ERRORTYPE EventHandler (
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_EVENTTYPE eEvent,
          OMX_IN OMX_U32 nData1,
          OMX_IN OMX_U32 nData2,
          OMX_IN OMX_PTR pEventData);

};

class OMXILCameraComponent : public OMXILComponent{
public:
  const static int OUTPUT_PREVIEW_PORT_NUM = 70;
  const static int OUTPUT_CAPTURE_PORT_NUM = 72;

  OMXILCameraComponent(int width, int 
  height):OMXILComponent("OMX.broadcom.camera"), m_width(width), m_height(height){}
  virtual ~OMXILCameraComponent(){};

  bool load_camera_drivers ();
  bool configure_sensor_settings();
  bool set_output_port();
  bool set_camera_settings();
  bool capture(bool on);
private:
  int m_width;
  int m_height;

};

class OMXILEncoderComponent : public OMXILComponent{
public:
  class EventListener {
  public:  
    virtual void bufferFilled(int size) = 0;
  };    
  enum BUFFER_STATUS_T {
    NONE_S,
    EOF_S,  //End of Frame
    EOS_S,  //End of Stream
  };
  const static int INPUT_PORT_NUM = 340;
  const static int OUTPUT_PORT_NUM = 341;
  virtual bool enable_port (OMX_U32 port);
  virtual bool disable_port (OMX_U32 port);
  
  OMXILEncoderComponent(int width, int height, char* imgBuf, int szbuf, 
      void (*cbEndOfFrame)(int, void*), void (*cbEndOfStream)(void*), void* 
      clientData):OMXILComponent("OMX.broadcom.image_encode", FillBufferDone), 
      m_width(width), m_height(height), m_imgBuf(imgBuf), m_szbuf(szbuf), 
      m_output_buffer(NULL), m_bufferStatus(NONE_S), 
      m_cbEndOfFrame(cbEndOfFrame), m_cbEndOfStream(cbEndOfStream), 
      m_clientData(clientData){}
  virtual ~OMXILEncoderComponent(){};

  bool set_output_port();
  bool set_jpeg_settings();
  bool capture();
private:
  int m_width;
  int m_height;
  char* m_imgBuf;
  int m_szbuf;
  OMX_BUFFERHEADERTYPE* m_output_buffer;
  BUFFER_STATUS_T m_bufferStatus;
  void (*m_cbEndOfFrame)(int, void*);
  void (*m_cbEndOfStream)(void*);
  void* m_clientData;
  int m_flength;
  pthread_t m_captureThreadId;


  static OMX_ERRORTYPE FillBufferDone (
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);
  static void* capturing(void* arg);
};



#endif

