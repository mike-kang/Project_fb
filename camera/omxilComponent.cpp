#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <bcm_host.h>
#include <interface/vcos/vcos.h>
#include "omxilComponent.h"
#include "log.h"
#include "dump.h"

#define LOG_TAG "OMXILComponent"

#define OMX_INIT_STRUCTURE(a) \
  memset (&(a), 0, sizeof (a)); \
  (a).nSize = sizeof (a); \
  (a).nVersion.nVersion = OMX_VERSION; \
  (a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
  (a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
  (a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
  (a).nVersion.s.nStep = OMX_VERSION_STEP

static int verbos = 1;

bool OMXILComponent::connect(OMXILComponent* cmpA, int portA, OMXILComponent* cmpB, int portB)
{
  OMX_ERRORTYPE error;
  if (verbos == 1) LOGV ("configuring tunnels\n");
  if ((error = OMX_SetupTunnel (cmpA->m_handle, portA, cmpB->m_handle, portB))){
    LOGE("error: OMX_SetupTunnel: %s\n",
        dump_OMX_ERRORTYPE (error));
    return false;
  }
  return true;
}

OMXILComponent::OMXILComponent(const char* name, FillBufferDoneType fillbufferdone)
{
  if (verbos == 1) printf ("initializing component '%s'\n", name);
  
  OMX_ERRORTYPE error;

  //Each component has an event_handler and fill_buffer_done functions
  OMX_CALLBACKTYPE callbacks_st;
  callbacks_st.EventHandler = EventHandler;
  callbacks_st.FillBufferDone = fillbufferdone;
  
  //Get the handle
  if ((error = OMX_GetHandle (&m_handle, (char*)name, this,
      &callbacks_st))){
    LOGE("error: OMX_GetHandle: %d\n", error);
    throw EXCEPTION_CONSTRUCT;
  }
  sem_init(&sem_EventComplete, 0, 0);
  strcpy(m_name, name);
  //Disable all the ports
  disable_allport(m_handle);
}

bool OMXILComponent::disable_allport(OMX_HANDLETYPE handle)
{
  OMX_ERRORTYPE error;

  OMX_INDEXTYPE types[] = {
    OMX_IndexParamAudioInit,
    OMX_IndexParamVideoInit,
    OMX_IndexParamImageInit,
    OMX_IndexParamOtherInit
  };
  OMX_PORT_PARAM_TYPE ports_st;
  OMX_INIT_STRUCTURE (ports_st);

  int i;
  for (i=0; i<4; i++){
    if ((error = OMX_GetParameter (m_handle, types[i], &ports_st))){
      LOGE("error: OMX_GetParameter: %d\n", error);
      return false;
    }
    
    OMX_U32 port;
    for (port=ports_st.nStartPortNumber;
        port<ports_st.nStartPortNumber + ports_st.nPorts; port++){
      //Disable the port
      disable_port (port);
    }
  }
  return true;

}

OMXILComponent::~OMXILComponent()
{
  if (verbos == 1) LOGV("deinitializing component '%s'\n", m_name);
  
  OMX_ERRORTYPE error;
  
  sem_destroy(&sem_EventComplete);
  if ((error = OMX_FreeHandle (m_handle))){
    LOGE( "error: OMX_FreeHandle: %s\n", dump_OMX_ERRORTYPE (error));
  }
}

bool OMXILComponent::enable_port (OMX_U32 port){
  LOGV("enabling port %d ('%s')\n", port, m_name);
  
  OMX_ERRORTYPE error;
  
  if ((error = OMX_SendCommand (m_handle, OMX_CommandPortEnable,
      port, 0))){
    LOGE("error: OMX_SendCommand: %d\n", error);
    return false;
  }
  sem_wait(&sem_EventComplete);
  return true;
}

bool OMXILComponent::disable_port (OMX_U32 port){
  if (verbos == 1) printf ("disabling port %d ('%s')\n", port, m_name);
  
  OMX_ERRORTYPE error;
  
  if ((error = OMX_SendCommand (m_handle, OMX_CommandPortDisable,
      port, 0))){
    LOGE("error: OMX_SendCommand: %d\n", error);
    return false;
  }
  sem_wait(&sem_EventComplete);
  return true;
}

#if 1
OMX_ERRORTYPE OMXILComponent::EventHandler (
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
{
  OMXILComponent* component = (OMXILComponent*)pAppData;
  switch (eEvent){
    case OMX_EventCmdComplete:
      switch (nData1){
        case OMX_CommandStateSet:
          sem_post(&component->sem_EventComplete);
          break;
        case OMX_CommandPortEnable:
          sem_post(&component->sem_EventComplete);
          break;
        case OMX_CommandPortDisable:
          sem_post(&component->sem_EventComplete);
          break;
      }
      break;
    case OMX_EventParamOrConfigChanged:
      sem_post(&component->sem_EventComplete);
      break;
      
  }
  return OMX_ErrorNone;
}
#else
OMX_ERRORTYPE OMXILComponent::EventHandler (
    OMX_IN OMX_HANDLETYPE comp,
    OMX_IN OMX_PTR app_data,
    OMX_IN OMX_EVENTTYPE event,
    OMX_IN OMX_U32 data1,
    OMX_IN OMX_U32 data2,
    OMX_IN OMX_PTR event_data){
  OMXILComponent* component = (OMXILComponent*)app_data;
//  struct timespec  ts;
  
  switch (event){
    case OMX_EventCmdComplete:
      switch (data1){
        case OMX_CommandStateSet:
          if (verbos == 1) printf ("event: %s, OMX_CommandStateSet, state: %s\n",
              component->m_name, dump_OMX_STATETYPE ((OMX_STATETYPE)data2));
          sem_post(&component->sem_EventComplete);
          break;
        case OMX_CommandPortDisable:
          if (verbos == 1) printf ("event: %s, OMX_CommandPortDisable, port: %d\n",
              component->m_name, data2);
          sem_post(&component->sem_EventComplete);
          break;
        case OMX_CommandPortEnable:
          if (verbos == 1) printf ("event: %s, OMX_CommandPortEnable, port: %d\n",
              component->m_name, data2);
          sem_post(&component->sem_EventComplete);
          break;
        case OMX_CommandFlush:
          if (verbos == 1) printf ("event: %s, OMX_CommandFlush, port: %d\n",
              component->m_name, data2);
          sem_post(&component->sem_EventComplete);
          break;
        case OMX_CommandMarkBuffer:
          if (verbos == 1) printf ("event: %s, OMX_CommandMarkBuffer, port: %d\n",
              component->m_name, data2);
          sem_post(&component->sem_EventComplete);
          break;
      }
      break;
    case OMX_EventError:
      if (verbos == 1) printf ("event: %s, %s\n", component->m_name, dump_OMX_ERRORTYPE ((OMX_ERRORTYPE)data1));
      sem_post(&component->sem_EventComplete);
      break;
    case OMX_EventMark:
      if (verbos == 1) printf ("event: %s, OMX_EventMark\n", component->m_name);
      sem_post(&component->sem_EventComplete);
      break;
    case OMX_EventPortSettingsChanged:
      if (verbos == 1) printf ("event: %s, OMX_EventPortSettingsChanged, port: %d\n",
          component->m_name, data1);
      sem_post(&component->sem_EventComplete);
      break;
    case OMX_EventParamOrConfigChanged:
      if (verbos == 1) printf ("event: %s, OMX_EventParamOrConfigChanged, data1: %d, data2: "
          "%X\n", component->m_name, data1, data2);
      sem_post(&component->sem_EventComplete);
      break;
    case OMX_EventBufferFlag:
      if (verbos == 1) printf ("event: %s, OMX_EventBufferFlag, port: %d\n",
          component->m_name, data1);
      //sem_post(&component->sem_EventComplete);
      break;
    case OMX_EventResourcesAcquired:
      if (verbos == 1) printf ("event: %s, OMX_EventResourcesAcquired\n", component->m_name);
      sem_post(&component->sem_EventComplete);
      break;
    case OMX_EventDynamicResourcesAvailable:
      if (verbos == 1) printf ("event: %s, OMX_EventDynamicResourcesAvailable\n",
          component->m_name);
      sem_post(&component->sem_EventComplete);
      break;
    default:
      //This should never execute, just ignore
      if (verbos == 1) printf ("event: unknown (%X)\n", event);
      break;
  }

  return OMX_ErrorNone;
}
#endif

bool OMXILComponent::change_state (OMX_STATETYPE state)
{
  LOGV ("changing '%s' state to %s\n", m_name,
      dump_OMX_STATETYPE (state));
  
  OMX_ERRORTYPE error;
  
  if ((error = OMX_SendCommand (m_handle, OMX_CommandStateSet, state,
      0))){
    LOGE("error: OMX_SendCommand: %s\n",
        dump_OMX_ERRORTYPE (error));
    return false;
  }

  sem_wait(&sem_EventComplete);
  
  return true;
}

//======================================================================================
#undef LOG_TAG
#define LOG_TAG "OMXILCameraComponent"

int round_up (int value, int divisor){
  return (divisor + value - 1) & ~(divisor - 1);
}
bool OMXILCameraComponent::load_camera_drivers ()
{
  /*
  This is a specific behaviour of the Broadcom's Raspberry Pi OpenMAX IL
  implementation module because the OMX_SetConfig() and OMX_SetParameter() are
  blocking functions but the drivers are loaded asynchronously, that is, an
  event is fired to signal the completion. Basically, what you're saying is:
  
  "When the parameter with index OMX_IndexParamCameraDeviceNumber is set, load
  the camera drivers and emit an OMX_EventParamOrConfigChanged event"
  
  The red LED of the camera will be turned on after this call.
  */
  static bool initedStructure1 = false; //for performance
  static bool initedStructure2 = false;
  if (verbos == 1) printf ("loading '%s' drivers\n", m_name);
  
  OMX_ERRORTYPE error;

  static OMX_CONFIG_REQUESTCALLBACKTYPE cbs_st;
  if(!initedStructure1){
    OMX_INIT_STRUCTURE (cbs_st);
    cbs_st.nPortIndex = OMX_ALL;
    cbs_st.nIndex = OMX_IndexParamCameraDeviceNumber;
    cbs_st.bEnable = OMX_TRUE;
    initedStructure1 = true;
  }
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigRequestCallback,
      &cbs_st))){
    LOGE("error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }

  static OMX_PARAM_U32TYPE dev_st;
  if(!initedStructure2){
    OMX_INIT_STRUCTURE (dev_st);
    dev_st.nPortIndex = OMX_ALL;
    //ID for the camera device
    dev_st.nU32 = 0;
    initedStructure2 = true;
  }
  if ((error = OMX_SetParameter (m_handle,
      OMX_IndexParamCameraDeviceNumber, &dev_st))){
    LOGE( "error: OMX_SetParameter: %s\n",
        dump_OMX_ERRORTYPE (error));
    return false;
  }
  
  sem_wait(&sem_EventComplete);
  
  return true;
}

bool OMXILCameraComponent::configure_sensor_settings()
{
  OMX_ERRORTYPE error;

  if (verbos == 1) printf ("configuring '%s' sensor\n", m_name);
  OMX_PARAM_SENSORMODETYPE sensor;
  OMX_INIT_STRUCTURE (sensor);
  sensor.nPortIndex = OMX_ALL;
  OMX_INIT_STRUCTURE (sensor.sFrameSize);
  sensor.sFrameSize.nPortIndex = OMX_ALL;
  if ((error = OMX_GetParameter (m_handle, OMX_IndexParamCommonSensorMode,
      &sensor))){
    LOGE( "error: OMX_GetParameter: %s\n",
        dump_OMX_ERRORTYPE (error));
    return false;
  }
  sensor.bOneShot = OMX_TRUE;
  sensor.sFrameSize.nWidth = m_width;
  sensor.sFrameSize.nHeight = m_height;
  if ((error = OMX_SetParameter (m_handle, OMX_IndexParamCommonSensorMode,
      &sensor))){
    LOGE( "error: OMX_SetParameter: %s\n",
        dump_OMX_ERRORTYPE (error));
    return false;
  }
  return true;
}

bool OMXILCameraComponent::set_output_port()
{
  OMX_ERRORTYPE error;
  OMX_PARAM_PORTDEFINITIONTYPE port_def;

  if (verbos == 1) printf ("configuring '%s' port definition(%dx%d)\n", m_name, m_width, m_height);
  OMX_INIT_STRUCTURE (port_def);
  port_def.nPortIndex = OUTPUT_CAPTURE_PORT_NUM;
  if ((error = OMX_GetParameter (m_handle, OMX_IndexParamPortDefinition,
      &port_def))){
    LOGE( "error: OMX_GetParameter: %s\n",
        dump_OMX_ERRORTYPE (error));
    return false;
  }
  port_def.format.image.nFrameWidth = m_width;
  port_def.format.image.nFrameHeight = m_height;
  port_def.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
  port_def.format.image.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
  //Stride is byte-per-pixel*width, YUV has 1 byte per pixel, so the stride is
  //the width (rounded up to the nearest multiple of 16).
  //See mmal/util/mmal_util.c, mmal_encoding_width_to_stride()
  port_def.format.image.nStride = round_up (m_width, 16);
  
  if ((error = OMX_SetParameter (m_handle, OMX_IndexParamPortDefinition,
      &port_def))){
    LOGE( "error: OMX_SetParameter: %s\n",
        dump_OMX_ERRORTYPE (error));
    return false;
  }
  return true;

}

bool OMXILCameraComponent::set_camera_settings()
{
  if (verbos == 1) printf ("configuring '%s' settings\n", m_name);

  OMX_ERRORTYPE error;
  
  //Sharpness
  OMX_CONFIG_SHARPNESSTYPE sharpness_st;
  OMX_INIT_STRUCTURE (sharpness_st);
  sharpness_st.nPortIndex = OMX_ALL;
  sharpness_st.nSharpness = CAM_SHARPNESS;
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigCommonSharpness,
      &sharpness_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }
  
  //Contrast
  OMX_CONFIG_CONTRASTTYPE contrast_st;
  OMX_INIT_STRUCTURE (contrast_st);
  contrast_st.nPortIndex = OMX_ALL;
  contrast_st.nContrast = CAM_CONTRAST;
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigCommonContrast,
      &contrast_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }
  
  //Saturation
  OMX_CONFIG_SATURATIONTYPE saturation_st;
  OMX_INIT_STRUCTURE (saturation_st);
  saturation_st.nPortIndex = OMX_ALL;
  saturation_st.nSaturation = CAM_SATURATION;
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigCommonSaturation,
      &saturation_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }
  
  //Brightness
  OMX_CONFIG_BRIGHTNESSTYPE brightness_st;
  OMX_INIT_STRUCTURE (brightness_st);
  brightness_st.nPortIndex = OMX_ALL;
  brightness_st.nBrightness = CAM_BRIGHTNESS;
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigCommonBrightness,
      &brightness_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }
  
  //Exposure value
  OMX_CONFIG_EXPOSUREVALUETYPE exposure_value_st;
  OMX_INIT_STRUCTURE (exposure_value_st);
  exposure_value_st.nPortIndex = OMX_ALL;
  exposure_value_st.eMetering = CAM_METERING;
  exposure_value_st.xEVCompensation = (CAM_EXPOSURE_COMPENSATION << 16)/6;
  exposure_value_st.nShutterSpeedMsec = CAM_SHUTTER_SPEED;
  exposure_value_st.bAutoShutterSpeed = CAM_SHUTTER_SPEED_AUTO;
  exposure_value_st.nSensitivity = CAM_ISO;
  exposure_value_st.bAutoSensitivity = CAM_ISO_AUTO;
  if ((error = OMX_SetConfig (m_handle,
      OMX_IndexConfigCommonExposureValue, &exposure_value_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }
  
  //Exposure control
  OMX_CONFIG_EXPOSURECONTROLTYPE exposure_control_st;
  OMX_INIT_STRUCTURE (exposure_control_st);
  exposure_control_st.nPortIndex = OMX_ALL;
  exposure_control_st.eExposureControl = CAM_EXPOSURE;
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigCommonExposure,
      &exposure_control_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }
  
  //Frame stabilisation
  OMX_CONFIG_FRAMESTABTYPE frame_stabilisation_st;
  OMX_INIT_STRUCTURE (frame_stabilisation_st);
  frame_stabilisation_st.nPortIndex = OMX_ALL;
  frame_stabilisation_st.bStab = CAM_FRAME_STABILIZATION;
  if ((error = OMX_SetConfig (m_handle,
      OMX_IndexConfigCommonFrameStabilisation, &frame_stabilisation_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }
  
  //White balance
  OMX_CONFIG_WHITEBALCONTROLTYPE white_balance_st;
  OMX_INIT_STRUCTURE (white_balance_st);
  white_balance_st.nPortIndex = OMX_ALL;
  white_balance_st.eWhiteBalControl = CAM_WHITE_BALANCE;
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigCommonWhiteBalance,
      &white_balance_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }
  
  //White balance gains (if white balance is set to off)
  if (!CAM_WHITE_BALANCE){
    OMX_CONFIG_CUSTOMAWBGAINSTYPE white_balance_gains_st;
    OMX_INIT_STRUCTURE (white_balance_gains_st);
    white_balance_gains_st.xGainR = (CAM_WHITE_BALANCE_RED_GAIN << 16)/1000;
    white_balance_gains_st.xGainB = (CAM_WHITE_BALANCE_BLUE_GAIN << 16)/1000;
    if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigCustomAwbGains,
        &white_balance_gains_st))){
      LOGE( "error: OMX_SetConfig: %s\n",
          dump_OMX_ERRORTYPE (error));
      return false;
    }
  }
  
  //Image filter
  OMX_CONFIG_IMAGEFILTERTYPE image_filter_st;
  OMX_INIT_STRUCTURE (image_filter_st);
  image_filter_st.nPortIndex = OMX_ALL;
  image_filter_st.eImageFilter = CAM_IMAGE_FILTER;
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigCommonImageFilter,
      &image_filter_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }
  
  //Mirror
  OMX_CONFIG_MIRRORTYPE mirror_st;
  OMX_INIT_STRUCTURE (mirror_st);
  mirror_st.nPortIndex = OUTPUT_CAPTURE_PORT_NUM;
  mirror_st.eMirror = CAM_MIRROR;
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigCommonMirror,
      &mirror_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }

  //Rotation
  OMX_CONFIG_ROTATIONTYPE rotation_st;
  OMX_INIT_STRUCTURE (rotation_st);
  rotation_st.nPortIndex = OUTPUT_CAPTURE_PORT_NUM;
  rotation_st.nRotation = CAM_ROTATION;
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigCommonRotate,
      &rotation_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }

  //Color enhancement
  OMX_CONFIG_COLORENHANCEMENTTYPE color_enhancement_st;
  OMX_INIT_STRUCTURE (color_enhancement_st);
  color_enhancement_st.nPortIndex = OMX_ALL;
  color_enhancement_st.bColorEnhancement = CAM_COLOR_ENABLE;
  color_enhancement_st.nCustomizedU = CAM_COLOR_U;
  color_enhancement_st.nCustomizedV = CAM_COLOR_V;
  if ((error = OMX_SetConfig (m_handle,
      OMX_IndexConfigCommonColorEnhancement, &color_enhancement_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }

  //Denoise
  OMX_CONFIG_BOOLEANTYPE denoise_st;
  OMX_INIT_STRUCTURE (denoise_st);
  denoise_st.bEnabled = CAM_NOISE_REDUCTION;
  if ((error = OMX_SetConfig (m_handle,
      OMX_IndexConfigStillColourDenoiseEnable, &denoise_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }

  //ROI
  OMX_CONFIG_INPUTCROPTYPE roi_st;
  OMX_INIT_STRUCTURE (roi_st);
  roi_st.nPortIndex = OMX_ALL;
  roi_st.xLeft = (CAM_ROI_LEFT << 16)/100;
  roi_st.xTop = (CAM_ROI_TOP << 16)/100;
  roi_st.xWidth = (CAM_ROI_WIDTH << 16)/100;
  roi_st.xHeight = (CAM_ROI_HEIGHT << 16)/100;
  if ((error = OMX_SetConfig (m_handle,
      OMX_IndexConfigInputCropPercentages, &roi_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }

  //DRC
  OMX_CONFIG_DYNAMICRANGEEXPANSIONTYPE drc_st;
  OMX_INIT_STRUCTURE (drc_st);
  drc_st.eMode = CAM_DRC;
  if ((error = OMX_SetConfig (m_handle,
      OMX_IndexConfigDynamicRangeExpansion, &drc_st))){
    LOGE( "error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }
  return true;
}

bool OMXILCameraComponent::capture(bool on)
{
  LOGV("capture: %d\n", on);
  OMX_ERRORTYPE error;
  static OMX_CONFIG_PORTBOOLEANTYPE cameraCapturePort;
  static bool inited = false;

  if(!inited){
    OMX_INIT_STRUCTURE (cameraCapturePort);
    cameraCapturePort.nPortIndex = OUTPUT_CAPTURE_PORT_NUM;
    inited = true;
  }
  
  cameraCapturePort.bEnabled = on? OMX_TRUE: OMX_FALSE;
  if ((error = OMX_SetConfig (m_handle, OMX_IndexConfigPortCapturing,
      &cameraCapturePort))){
    LOGE("error: OMX_SetConfig: %s\n", dump_OMX_ERRORTYPE (error));
    return false;
  }

  return true;
}

//======================================================================================
#undef LOG_TAG
#define LOG_TAG "OMXILEncoderComponent"

bool OMXILEncoderComponent::enable_port (OMX_U32 port){
  OMX_ERRORTYPE error;
  LOGV("enabling port %d ('%s')\n", port, m_name);
  
  if ((error = OMX_SendCommand (m_handle, OMX_CommandPortEnable,
      port, 0))){
    LOGE("error: OMX_SendCommand: %d\n", error);
    return false;
  }
  if(port == (OMX_U32)OUTPUT_PORT_NUM){
#if 1    
    if ((error = OMX_UseBuffer (m_handle, &m_output_buffer, OUTPUT_PORT_NUM,
        0, m_szbuf, (OMX_U8*)m_imgBuf))){
      LOGE("error: OMX_AllocateBuffer: %s\n",
          dump_OMX_ERRORTYPE (error));
      return false;
    }
#else
    OMX_PARAM_PORTDEFINITIONTYPE def_st;
    OMX_INIT_STRUCTURE (def_st);
    def_st.nPortIndex = 341;
    if ((error = OMX_GetParameter (m_handle, OMX_IndexParamPortDefinition,
        &def_st))){
      fprintf (stderr, "error: OMX_GetParameter: %s\n",
          dump_OMX_ERRORTYPE (error));
      return false;
    }
    
    if (verbos == 1) printf ("allocating %s output buffer\n", m_name);
    
    def_st.nBufferSize = 4096;

    if ((error = OMX_AllocateBuffer (m_handle, &m_output_buffer, 341,
        0, def_st.nBufferSize))){
      fprintf (stderr, "error: OMX_AllocateBuffer: %s\n",
          dump_OMX_ERRORTYPE (error));
      return false;
    }
#endif
  }
  
  sem_wait(&sem_EventComplete);
  return true;
}

bool OMXILEncoderComponent::disable_port (OMX_U32 port){
  OMX_ERRORTYPE error;
  
  if(port == (OMX_U32)OUTPUT_PORT_NUM && m_output_buffer){
    if ((error = OMX_FreeBuffer (m_handle, OUTPUT_PORT_NUM, m_output_buffer))){
      LOGE("error: OMX_FreeBuffer: %s\n", dump_OMX_ERRORTYPE (error));
      return false;
    }
    m_output_buffer = NULL;
  }

  OMXILComponent::disable_port(port);
  return true;
}

bool OMXILEncoderComponent::set_output_port()
{
  OMX_PARAM_PORTDEFINITIONTYPE port_def;
  OMX_ERRORTYPE error;

  if (verbos == 1) printf ("configuring '%s' port definition\n", m_name);
  OMX_INIT_STRUCTURE (port_def);
  port_def.nPortIndex = OUTPUT_PORT_NUM;
  if ((error = OMX_GetParameter (m_handle, OMX_IndexParamPortDefinition,
      &port_def))){
    LOGE( "error: OMX_SetParameter: %d\n",
        error);
    return false;
  }
  port_def.format.image.nFrameWidth = m_width;
  port_def.format.image.nFrameHeight = m_height;
  port_def.format.image.eCompressionFormat = OMX_IMAGE_CodingJPEG;
  port_def.format.image.eColorFormat = OMX_COLOR_FormatUnused;
  if ((error = OMX_SetParameter (m_handle, OMX_IndexParamPortDefinition,
      &port_def))){
    LOGE( "error: OMX_SetParameter: %d\n",
        error);
    return false;
  }
  return true;
}

bool OMXILEncoderComponent::set_jpeg_settings()
{
  OMX_ERRORTYPE error;
  
  //Quality
  OMX_IMAGE_PARAM_QFACTORTYPE quality;
  OMX_INIT_STRUCTURE (quality);
  quality.nPortIndex = OUTPUT_PORT_NUM;
  quality.nQFactor = JPEG_QUALITY;
  if ((error = OMX_SetParameter (m_handle, OMX_IndexParamQFactor,
      &quality))){
    LOGE( "error: OMX_SetParameter: %d\n",
        error);
    return false;
  }
  
  //Disable EXIF tags
  OMX_CONFIG_BOOLEANTYPE exif;
  OMX_INIT_STRUCTURE (exif);
  exif.bEnabled = JPEG_EXIF_DISABLE;
  if ((error = OMX_SetParameter (m_handle, OMX_IndexParamBrcmDisableEXIF,
      &exif))){
    LOGE( "error: OMX_SetParameter: %d\n",
        error);
    return false;
  }
  
  //Enable IJG table
  OMX_PARAM_IJGSCALINGTYPE ijg;
  OMX_INIT_STRUCTURE (ijg);
  ijg.nPortIndex = 341;
  ijg.bEnabled = JPEG_IJG_ENABLE;
  if ((error = OMX_SetParameter (m_handle,
      OMX_IndexParamBrcmEnableIJGTableScaling, &ijg))){
    LOGE( "error: OMX_SetParameter: %d\n",
        error);
    return false;
  }
  
  //Thumbnail
  OMX_PARAM_BRCMTHUMBNAILTYPE thumbnail;
  OMX_INIT_STRUCTURE (thumbnail);
  thumbnail.bEnable = JPEG_THUMBNAIL_ENABLE;
  thumbnail.bUsePreview = JPEG_PREVIEW;
  thumbnail.nWidth = JPEG_THUMBNAIL_WIDTH;
  thumbnail.nHeight = JPEG_THUMBNAIL_HEIGHT;
  if ((error = OMX_SetParameter (m_handle, OMX_IndexParamBrcmThumbnail,
      &thumbnail))){
    LOGE( "error: OMX_SetParameter: %d\n",
        error);
    return false;
  }
  
  //EXIF tags
  //See firmware/documentation/ilcomponents/image_decode.html for valid keys
  char key[] = "IFD0.Make";
  char value[] = "Raspberry Pi";
  
  int key_length = strlen (key);
  const int value_length = 12; //strlen (value);
  
  struct {
    //These two fields need to be together
    OMX_CONFIG_METADATAITEMTYPE metadata_st;
    char metadata_padding[12];
  } item;
  
  OMX_INIT_STRUCTURE (item.metadata_st);
  item.metadata_st.nSize = sizeof (item);
  item.metadata_st.eScopeMode = OMX_MetadataScopePortLevel;
  item.metadata_st.nScopeSpecifier = 341;
  item.metadata_st.eKeyCharset = OMX_MetadataCharsetASCII;
  item.metadata_st.nKeySizeUsed = key_length;
  memcpy (item.metadata_st.nKey, key, key_length);
  item.metadata_st.eValueCharset = OMX_MetadataCharsetASCII;
  item.metadata_st.nValueMaxSize = sizeof (item.metadata_padding);
  item.metadata_st.nValueSizeUsed = value_length;
  memcpy (item.metadata_st.nValue, value, value_length);
  
  if ((error = OMX_SetConfig (m_handle,
      OMX_IndexConfigMetadataItem, &item))){
    LOGE( "OMX_SetConfig: %d", error);
    return false;
  }

  
  return true;
}

bool OMXILEncoderComponent::capture()
{
  LOGV("capture\n");
  pthread_create(&m_captureThreadId, NULL, capturing, this); 
  return true;
}

//static
void* OMXILEncoderComponent::capturing(void* arg)
{
  OMX_ERRORTYPE error;
  OMXILEncoderComponent* component = (OMXILEncoderComponent*)arg;
  component->m_bufferStatus = NONE_S;
  
  while(1){
    //Get the buffer data (a slice of the image)
    if ((error = OMX_FillThisBuffer (component->m_handle, component->m_output_buffer))){
      LOGE("error: OMX_FillThisBuffer: %s\n", dump_OMX_ERRORTYPE (error));
      pthread_exit(NULL);
    }
    
    //Wait until it's filled
    sem_wait(&component->sem_EventComplete);

    if(component->m_bufferStatus == EOF_S){
      if(component->m_cbEndOfFrame) component->m_cbEndOfFrame(component->m_flength, component->m_clientData);
    }
    else if(component->m_bufferStatus == EOS_S){
      if(component->m_cbEndOfStream) component->m_cbEndOfStream(component->m_clientData);
      break;
    }
    //When it's the end of the stream, an OMX_EventBufferFlag is emitted in the
    //camera and image_encode components. Then the FillBufferDone function is
    //called in the image_encode
  }
  return NULL;
}

//static
OMX_ERRORTYPE OMXILEncoderComponent::FillBufferDone (
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
  OMXILEncoderComponent* component = (OMXILEncoderComponent*)pAppData;
  
  LOGV ("event: %s, fill_buffer_done %d (0x%x)\n", component->m_name, pBuffer->nFilledLen, pBuffer->nFlags);

  if(pBuffer->nFlags == OMX_BUFFERFLAG_ENDOFFRAME){
    component->m_bufferStatus = EOF_S;    
    component->m_flength = pBuffer->nFilledLen;
  }
  if(pBuffer->nFlags == OMX_BUFFERFLAG_EOS){
    component->m_bufferStatus = EOS_S;    
  }
  
  sem_post(&component->sem_EventComplete);
  return OMX_ErrorNone;
}


