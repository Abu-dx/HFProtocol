#ifndef __G35DDCAPI_H__
#define __G35DDCAPI_H__

#ifdef WIN32_LEAN_AND_MEAN

#include <basetyps.h>

#endif

#include <windows.h>

#define G35DDC_CLASS_ID_DEVICE_ENUMERATOR           0x35001
#define G35DDC_CLASS_ID_DEVICE                      0x35102
#define G35DDC_CLASS_ID_COH_DEVICE_SET_ENUMERATOR   0x35003
#define G35DDC_CLASS_ID_COH_DEVICE_SET              0x35104

#ifndef __G3XDDC_INTERFACE__
#define __G3XDDC_INTERFACE__ 5

#define G3XDDC_INTERFACE_TYPE_PCIE    0
#define G3XDDC_INTERFACE_TYPE_USB     1

#endif //__G3XDDC_INTERFACE__


#ifndef __G3XDDC_OPEN__
#define __G3XDDC_OPEN__ 5

#define G3XDDC_OPEN_DEMO   ((CHAR*)0x00000000)
#define G3XDDC_OPEN_FIRST  ((CHAR*)0xFFFFFFFF)

#endif //__G3XDDC_OPEN__

#if !defined(__G3XDDC_MODES__) || __G3XDDC_MODES__<1

#define G3XDDC_MODE_CW               0
#define G3XDDC_MODE_AM               1
#define G3XDDC_MODE_FM               2
#define G3XDDC_MODE_LSB              4
#define G3XDDC_MODE_USB              5
#define G3XDDC_MODE_AMS              8
#define G3XDDC_MODE_DRM              18

#endif //__G3XDDC_MODES__


#if !defined(__G3XDDC_MODES__) || __G3XDDC_MODES__<3

#undef __G3XDDC_MODES__
#define __G3XDDC_MODES__ 5

#define G3XDDC_MODE_DSB              13
#define G3XDDC_MODE_ISB              14

#endif //__G3XDDC_MODES__


#define G35DDC_MODE_COUNT            9



#ifndef __G3XDDC_SIDE_BAND__
#define __G3XDDC_SIDE_BAND__ 5

#define G3XDDC_SIDE_BAND_LOWER         0x01
#define G3XDDC_SIDE_BAND_UPPER         0x02
#define G3XDDC_SIDE_BAND_BOTH          (G3XDDC_SIDE_BAND_LOWER|G3XDDC_SIDE_BAND_UPPER)

#endif //__G3XDDC_SIDE_BAND__



#if !defined(__G3XDDC_DEMODULATOR_PARAMS__) || __G3XDDC_DEMODULATOR_PARAMS__<1

#define G3XDDC_DEMODULATOR_PARAM_AMS_SIDE_BAND          0x00000001
#define G3XDDC_DEMODULATOR_PARAM_AMS_CAPTURE_RANGE      0x00000002
#define G3XDDC_DEMODULATOR_PARAM_CW_FREQUENCY           0x00000003
#define G3XDDC_DEMODULATOR_PARAM_DRM_AUDIO_SERVICE      0x00000004
#define G3XDDC_DEMODULATOR_PARAM_DRM_MULTIMEDIA_SERVICE 0x00000005

#pragma pack(push,1)

typedef struct
{
    UINT32  Tune;   //Hz
    UINT32  Lock;   //Hz
} G3XDDC_AMS_CAPTURE_RANGE;

#pragma pack(pop)

#endif //__G3XDDC_DEMODULATOR_PARAMS__


#if !defined(__G3XDDC_DEMODULATOR_PARAMS__) || __G3XDDC_DEMODULATOR_PARAMS__<3

#undef __G3XDDC_DEMODULATOR_PARAMS__
#define __G3XDDC_DEMODULATOR_PARAMS__ 5

#define G3XDDC_DEMODULATOR_PARAM_DSB_SIDE_BAND          0x00000006
#define G3XDDC_DEMODULATOR_PARAM_ISB_SIDE_BAND          0x00000007

#endif //__G3XDDC_DEMODULATOR_PARAMS__



#if !defined(__G3XDDC_DEMODULATOR_STATES__) || __G3XDDC_DEMODULATOR_STATES__<1

#define G3XDDC_DEMODULATOR_STATE_AMS_LOCK               0x00000001
#define G3XDDC_DEMODULATOR_STATE_AMS_FREQUENCY          0x00000002
#define G3XDDC_DEMODULATOR_STATE_AM_DEPTH               0x00000003
#define G3XDDC_DEMODULATOR_STATE_TUNE_ERROR             0x00000004
#define G3XDDC_DEMODULATOR_STATE_DRM_STATUS             0x00000005
#define G3XDDC_DEMODULATOR_STATE_FM_DEVIATION           0x00000006

#endif //__G3XDDC_DEMODULATOR_STATES__

#if !defined(__G3XDDC_DEMODULATOR_STATES__) || __G3XDDC_DEMODULATOR_STATES__<3

#undef __G3XDDC_DEMODULATOR_STATES__
#define __G3XDDC_DEMODULATOR_STATES__ 5

#define G3XDDC_DEMODULATOR_STATE_DSB_LOCK               0x00000007
#define G3XDDC_DEMODULATOR_STATE_DSB_FREQUENCY          0x00000008

#endif //__G3XDDC_DEMODULATOR_STATES__

#define G35_AUDIO_SAMPLE_RATE                           48000

#ifndef __G3XDDC_DEVICE_STATE__
#define __G3XDDC_DEVICE_STATE__ 5

#define G3XDDC_DEVICE_STATE_ERROR_HIGH_TEMP             0x00000001

#endif //__G3XDDC_DEVICE_STATE__

#ifndef __G3XDDC_DRM__
#define __G3XDDC_DRM__ 5

#define G3XDDC_DRM_STATE_MODE_NOT_DETERMINED_YET       -1
#define G3XDDC_DRM_STATE_MODE_A                        0
#define G3XDDC_DRM_STATE_MODE_B                        1
#define G3XDDC_DRM_STATE_MODE_C                        2
#define G3XDDC_DRM_STATE_MODE_D                        3

#define G3XDDC_DRM_STATE_INTERLEAVER_LONG              0       //long interleaver used (2 sec)
#define G3XDDC_DRM_STATE_INTERLEAVER_SHORT             1       //short interleaver used (400 msec)

#define G3XDDC_DRM_STATE_QAM_TYPE_STD                  0       //standard
#define G3XDDC_DRM_STATE_QAM_TYPE_HIER_SYM             1       //hierarchical symmetrical
#define G3XDDC_DRM_STATE_QAM_TYPE_HIER_MIX             2       //hierarchical mixed

#define G3XDDC_DRM_STATE_SERVICE_CONTENT_EMPTY         0x00    //service is not used/contains no data
#define G3XDDC_DRM_STATE_SERVICE_CONTENT_AUDIO         0x01    //service contains audio data
#define G3XDDC_DRM_STATE_SERVICE_CONTENT_TEXTMSG       0x02    //service contains text messages
#define G3XDDC_DRM_STATE_SERVICE_CONTENT_MULTIMEDIA    0x04    //service contains multimedia data
#define G3XDDC_DRM_STATE_SERVICE_CONTENT_DATA          0x08    //service contains application specific data

#define G3XDDC_DRM_STATE_AUDIO_CODING_AAC              0       //audio coding is AAC
#define G3XDDC_DRM_STATE_AUDIO_CODING_CELP             1       //audio coding is CELP
#define G3XDDC_DRM_STATE_AUDIO_CODING_HVXC             2       //audio coding is HVXC
#define G3XDDC_DRM_STATE_AUDIO_CODING_RFU              3       //reserved for future use

#define G3XDDC_DRM_STATE_AUDIO_MODE_AAC_MONO           0       //mono
#define G3XDDC_DRM_STATE_AUDIO_MODE_AAC_PARAM_STEREO   1       //parametric stereo
#define G3XDDC_DRM_STATE_AUDIO_MODE_AAC_STEREO         2       //stereo
#define G3XDDC_DRM_STATE_AUDIO_MODE_AAC_RFU            3       //reserved for future use
	
#define G3XDDC_DRM_STATE_AUDIO_MODE_CELP_NO_CRC        0       //audio data is without CRC
#define G3XDDC_DRM_STATE_AUDIO_MODE_CELP_CRC           1       //CRC used
#define G3XDDC_DRM_STATE_AUDIO_MODE_CELP_RFU_10        2       //reserved for future use
#define G3XDDC_DRM_STATE_AUDIO_MODE_CELP_RFU_11        3       //reserved for future use
	
#define G3XDDC_DRM_STATE_AUDIO_MODE_HVXC_RFU_00        0       //reserved for future use
#define G3XDDC_DRM_STATE_AUDIO_MODE_HVXC_RFU_01        1       //reserved for future use
#define G3XDDC_DRM_STATE_AUDIO_MODE_HVXC_RFU_10        2       //reserved for future use
#define G3XDDC_DRM_STATE_AUDIO_MODE_HVXC_RFU_11        3       //reserved for future use

#pragma pack(push,1)

typedef struct
{
    BOOL            Valid;                          //entries below are invalid/valid

    struct
    {
        BOOL        SyncFound;
        BOOL        FACDecoded;
        BOOL        SDCDecoded;
        BOOL        AudioDecoded;
        SHORT       NumberOfAudioFrames;            //-1 if not available
        SHORT       NumberOfAudioErrors;            //-1 if not available	
    } DecodingState;


    INT32           Mode;                           //G3XDDC_DRM_STATE_MODE_*
	double          RFBandwidth;                    //occupied RF bandwith in kHz, 0 = invalid or information not (yet) available
    BYTE            Interleaver;                    //G3XDDC_DRM_STATE_INTERLEAVER_*
    SHORT           SDCQam;                         //coding of the SDC (QAM), 0 = not (yet) available
    SHORT           MSCQam;                         //coding of the MSC (QAM), 0 = not (yet) available
    BYTE            MSCQamType;                     //G3XDDC_DRM_STATE_QAM_TYPE_* , used QAM coding for the MSC
    double          CoderateH;				        //used coderate for hierachical coding,  values <= 0 indicate not available or not used
    double          CoderateA;				        //used coderate for protection level A,  values <= 0 indicate not available or not used
    double          CoderateB;				        //used coderate for protection level B,  values <= 0 indicate not available or not used
    double          EstimatedSNR;			        //estimated SNR in dB of the decoded signal
    WCHAR           TextMessage[128 + 1 + 16];      //text message containing max. 128 byte text + string termination + max. 16 linebreaks
   
    struct
    {
        BYTE        Content;				        //G3XDDC_DRM_STATE_SERVICE_CONTENT_*
        WCHAR       DynamicLabel[256];		        //string containing dynamic label of the service, zero terminated
        WCHAR       Country[256];			        //string containing the signaled country for this service
        WCHAR       Language[256];			        //string containing the signaled language for this service
        WCHAR       ProgramType[256];		        //string containing the signaled program type for this service
        double      AudioBitrate;			        //data rate for the audio content (0 if not available)
        double      TextMsgBitrate;			        //data rate for the text message content (0 if not available)
        double      MultimediaBitrate;		        //data rate for the multimedia content (0 if not available)
        double      DataBitrate;			        //data rate for the data content (0 if not available)
    } ServiceInfo[4];

    struct
    {
        BOOL        Valid;                          //audio decoder information valid/invalid, when invalid all other entries of the struct contain no valid data
        BYTE        AudioCoding;                    //G3XDDC_DRM_STATE_AUDIO_CODING_*        
        BOOL        SBR;                            //SBR used/not used (TRUE/FALSE)
        INT32       AudioMode;                      //depeding on the audio coding, G3XDDC_DRM_STATE_AUDIO_MODE_*
    } AudioDecoderInfo[4];
} G3XDDC_DRM_STATUS;

#pragma pack(pop)

#endif //__G3XDDC_DRM__


#ifndef __G3XDDC_CREATE_INSTANCE__
#define __G3XDDC_CREATE_INSTANCE__ 3

typedef BOOL (__stdcall *G3XDDCAPI_CREATE_INSTANCE)(UINT32 ClassId,PVOID *Interface);

#endif //__G3XDDC_CREATE_INSTANCE__

#define G35DDC_FLAGS_EXTERNAL_REFERENCE     0x00000001
#define G35DDC_FLAGS_AUDIO_OUTPUT			0x00000002
#define G35DDC_FLAGS_COHERENT			    0x00000004

#pragma pack(push,1)

typedef struct
{
    CHAR        DevicePath[MAX_PATH];
    BYTE        InterfaceType;
    CHAR        SerialNumber[9];
    WORD        HWVersion;
    WORD        FWVersion;
    BYTE        EEPROMVersion;    
    UINT32      Flags;
    UINT32      ChannelCount;
    UINT32      DDCTypeCount;    
} G35DDC_DEVICE_INFO;

#define G35DDC_OPEN_DEMO_SET    ((G35DDC_DEVICE_INFO*)0x00000000)
#define G35DDC_OPEN_FIRST_SET   ((G35DDC_DEVICE_INFO*)0xFFFFFFFF)

#ifndef __G3XDDC_DDC_INFO__
#define __G3XDDC_DDC_INFO__ 5

typedef struct
{
    UINT32      SampleRate;
    UINT32      Bandwidth;
    UINT32      BitsPerSample;
} G3XDDC_DDC_INFO;

#endif //__G3XDDC_DDC_INFO__


#undef INTERFACE
#define INTERFACE IG35DDCDeviceEnumerator

DECLARE_INTERFACE(IG35DDCDeviceEnumerator)
{
    STDMETHOD_(LONG,AddRef)(THIS) PURE;
    STDMETHOD_(LONG,Release)(THIS) PURE;

    STDMETHOD_(BOOL,Enumerate)(THIS) PURE;
    STDMETHOD_(UINT32,GetCount)(THIS) PURE;
    STDMETHOD_(BOOL,GetDeviceInfo)(THIS_ UINT32 DeviceIndex,G35DDC_DEVICE_INFO *DeviceInfo,UINT32 BufferLength) PURE;
};

#undef INTERFACE
#define INTERFACE IG35DDCDeviceCallback

struct IG35DDCDevice;

DECLARE_INTERFACE(IG35DDCDeviceCallback)
{
    STDMETHOD_(VOID,G35DDC_IFCallback)(THIS_ struct IG35DDCDevice *Device,CONST SHORT *Buffer,UINT32 NumberOfSamples,WORD MaxADCAmplitude,UINT32 ADCSamplingRate) PURE;
    STDMETHOD_(VOID,G35DDC_DDC1StreamCallback)(THIS_ struct IG35DDCDevice *Device,CONST VOID *Buffer,UINT32 NumberOfSamples,UINT32 BitsPerSample) PURE;
    STDMETHOD_(BOOL,G35DDC_DDC1PlaybackStreamCallback)(THIS_ struct IG35DDCDevice *Device,VOID *Buffer,UINT32 NumberOfSamples,UINT32 BitsPerSample) PURE;
    STDMETHOD_(VOID,G35DDC_DDC2StreamCallback)(THIS_ struct IG35DDCDevice *Device,UINT32 Channel,CONST FLOAT *Buffer,UINT32 NumberOfSamples) PURE;        
    //SlevelPeak and SlevelRMS are in Volts
    //to convert SlevelRMS to power in dBm use SlevelRMS_dBm=10.0*log10(SlevelRMS*SlevelRMS/50.0*1000.0); where 50.0 is receiver input impendance, 1000.0 is coefficents to convert Watts to mili Watts
    STDMETHOD_(VOID,G35DDC_DDC2PreprocessedStreamCallback)(THIS_ struct IG35DDCDevice *Device,UINT32 Channel,CONST FLOAT *Buffer,UINT32 NumberOfSamples,FLOAT SlevelPeak,FLOAT SlevelRMS) PURE;        
    STDMETHOD_(VOID,G35DDC_AudioStreamCallback)(THIS_ struct IG35DDCDevice *Device,UINT32 Channel,UINT32 Type,CONST FLOAT *Buffer,UINT32 NumberOfSamples) PURE;
    STDMETHOD_(BOOL,G35DDC_AudioPlaybackStreamCallback)(THIS_ struct IG35DDCDevice *Device,UINT32 Channel,FLOAT *Buffer,UINT32 NumberOfSamples) PURE;
};

#undef INTERFACE
#define INTERFACE IG35DDCDevice


DECLARE_INTERFACE(IG35DDCDevice)
{
    STDMETHOD_(LONG,AddRef)(THIS) PURE;
    STDMETHOD_(LONG,Release)(THIS) PURE;

    STDMETHOD_(BOOL,Open)(THIS_ CONST CHAR *DevicePath) PURE;
    STDMETHOD_(VOID,Close)(THIS) PURE;
    STDMETHOD_(BOOL,IsOpen)(THIS) PURE;

    STDMETHOD_(BOOL,IsConnected)(THIS) PURE;

    STDMETHOD_(VOID,SetCallback)(THIS_ IG35DDCDeviceCallback *Callback) PURE;
    STDMETHOD_(IG35DDCDeviceCallback*,GetCallback)(THIS) PURE;

    STDMETHOD_(BOOL,GetDeviceInfo)(THIS_ G35DDC_DEVICE_INFO *Info,UINT32 BufferLength) PURE;

    STDMETHOD_(BOOL,SetPower)(THIS_ BOOL Power) PURE;
    STDMETHOD_(BOOL,GetPower)(THIS_ BOOL *Power) PURE;

    STDMETHOD_(BOOL,SetExternalReference)(THIS_ BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetExternalReference)(THIS_ BOOL *Enabled) PURE;

    STDMETHOD_(BOOL,SetAttenuator)(THIS_ UINT32 Attenuator) PURE;
    STDMETHOD_(BOOL,GetAttenuator)(THIS_ UINT32 *Attenuator) PURE;

    STDMETHOD_(BOOL,SetPreselectors)(THIS_ UINT32 Low,UINT32 High) PURE;
    STDMETHOD_(BOOL,GetPreselectors)(THIS_ UINT32 *Low,UINT32 *High) PURE;

    STDMETHOD_(BOOL,SetPreamp)(THIS_ BOOL Preamp) PURE;
    STDMETHOD_(BOOL,GetPreamp)(THIS_ BOOL *Preamp) PURE;    

    STDMETHOD_(BOOL,SetInverted)(THIS_ BOOL Inverted) PURE;
    STDMETHOD_(BOOL,GetInverted)(THIS_ BOOL *Inverted) PURE;       

    STDMETHOD_(BOOL,SetADCNoiseBlanker)(THIS_ BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetADCNoiseBlanker)(THIS_ BOOL *Enabled) PURE;
    STDMETHOD_(BOOL,SetADCNoiseBlankerThreshold)(THIS_ WORD Threshold) PURE;
    STDMETHOD_(BOOL,GetADCNoiseBlankerThreshold)(THIS_ WORD *Threshold) PURE;

    STDMETHOD_(BOOL,StartIF)(THIS_ WORD Period) PURE;
    STDMETHOD_(BOOL,StopIF)(THIS) PURE;

    STDMETHOD_(BOOL,GetDDCInfo)(THIS_ UINT32 DDCTypeIndex,G3XDDC_DDC_INFO *Info) PURE;
    STDMETHOD_(BOOL,GetDDC1Count)(THIS_ UINT32 *Count) PURE;
    STDMETHOD_(BOOL,SetDDC1)(THIS_ UINT32 DDCTypeIndex) PURE;
    STDMETHOD_(BOOL,GetDDC1)(THIS_ UINT32 *DDCTypeIndex,G3XDDC_DDC_INFO *DDCInfo) PURE;

    STDMETHOD_(BOOL,StartDDC1)(THIS_ UINT32 SamplesPerBuffer) PURE;
    STDMETHOD_(BOOL,StopDDC1)(THIS) PURE;
    
    STDMETHOD_(BOOL,StartDDC1Playback)(THIS_ UINT32 SamplesPerBuffer,UINT32 BitsPerSample) PURE; //BitsPerSample: 0 - default by DDC Info, 16 or 32
    STDMETHOD_(BOOL,PauseDDC1Playback)(THIS) PURE;
    STDMETHOD_(BOOL,ResumeDDC1Playback)(THIS) PURE;

    STDMETHOD_(BOOL,SetDDC1Frequency)(THIS_ UINT32 Frequency) PURE;
    STDMETHOD_(BOOL,GetDDC1Frequency)(THIS_ UINT32 *Frequency) PURE;

    STDMETHOD_(BOOL,GetDDC2)(THIS_ UINT32 *DDCTypeIndex,G3XDDC_DDC_INFO *DDCInfo) PURE;

    STDMETHOD_(BOOL,StartDDC2)(THIS_ UINT32 Channel,UINT32 SamplesPerBuffer) PURE;
    STDMETHOD_(BOOL,StopDDC2)(THIS_ UINT32 Channel) PURE;

    STDMETHOD_(BOOL,SetDDC2Frequency)(THIS_ UINT32 Channel,INT32 Frequency) PURE;
    STDMETHOD_(BOOL,GetDDC2Frequency)(THIS_ UINT32 Channel,INT32 *Frequency) PURE;

    STDMETHOD_(BOOL,SetDDC2NoiseBlanker)(THIS_ UINT32 Channel,BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetDDC2NoiseBlanker)(THIS_ UINT32 Channel,BOOL *Enabled) PURE;
    STDMETHOD_(BOOL,SetDDC2NoiseBlankerThreshold)(THIS_ UINT32 Channel,double Threshold) PURE;
    STDMETHOD_(BOOL,GetDDC2NoiseBlankerThreshold)(THIS_ UINT32 Channel,double *Threshold) PURE;
    STDMETHOD_(BOOL,GetDDC2NoiseBlankerExcessValue)(THIS_ UINT32 Channel,double *Value) PURE;

    STDMETHOD_(BOOL,GetSignalLevel)(THIS_ UINT32 Channel,FLOAT *Peak,FLOAT *RMS) PURE;

    STDMETHOD_(BOOL,SetAGC)(THIS_ UINT32 Channel,BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetAGC)(THIS_ UINT32 Channel,BOOL *Enabled) PURE;
    STDMETHOD_(BOOL,SetAGCParams)(THIS_ UINT32 Channel,double AttackTime,double DecayTime,double ReferenceLevel) PURE;
    STDMETHOD_(BOOL,GetAGCParams)(THIS_ UINT32 Channel,double *AttackTime,double *DecayTime,double *ReferenceLevel) PURE;
    STDMETHOD_(BOOL,SetMaxAGCGain)(THIS_ UINT32 Channel,double MaxGain) PURE;
    STDMETHOD_(BOOL,GetMaxAGCGain)(THIS_ UINT32 Channel,double *MaxGain) PURE;
    STDMETHOD_(BOOL,SetGain)(THIS_ UINT32 Channel,double Gain) PURE;
    STDMETHOD_(BOOL,GetGain)(THIS_ UINT32 Channel,double *Gain) PURE;
    STDMETHOD_(BOOL,GetCurrentGain)(THIS_ UINT32 Channel,double *Gain) PURE;

    STDMETHOD_(BOOL,SetNotchFilter)(THIS_ UINT32 Channel,UINT32 NotchFilterIndex,BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetNotchFilter)(THIS_ UINT32 Channel,UINT32 NotchFilterIndex,BOOL *Enabled) PURE;
    STDMETHOD_(BOOL,SetNotchFilterFrequency)(THIS_ UINT32 Channel,UINT32 NotchFilterIndex,INT32 Frequency) PURE;
    STDMETHOD_(BOOL,GetNotchFilterFrequency)(THIS_ UINT32 Channel,UINT32 NotchFilterIndex,INT32 *Frequency) PURE;
    STDMETHOD_(BOOL,SetNotchFilterBandwidth)(THIS_ UINT32 Channel,UINT32 NotchFilterIndex,UINT32 Bandwidth) PURE;
    STDMETHOD_(BOOL,GetNotchFilterBandwidth)(THIS_ UINT32 Channel,UINT32 NotchFilterIndex,UINT32 *Bandwidth) PURE;
    STDMETHOD_(BOOL,SetNotchFilterLength)(THIS_ UINT32 Channel,UINT32 NotchFilterIndex,UINT32 Length) PURE;         //Length: min=64,max=32768,step=64
    STDMETHOD_(BOOL,GetNotchFilterLength)(THIS_ UINT32 Channel,UINT32 NotchFilterIndex,UINT32 *Length) PURE;


    STDMETHOD_(BOOL,SetDemodulatorMode)(THIS_ UINT32 Channel,UINT32 Mode) PURE;
    STDMETHOD_(BOOL,GetDemodulatorMode)(THIS_ UINT32 Channel,UINT32 *Mode) PURE;
    STDMETHOD_(BOOL,SetDemodulatorFrequency)(THIS_ UINT32 Channel,INT32 Frequency) PURE;
    STDMETHOD_(BOOL,GetDemodulatorFrequency)(THIS_ UINT32 Channel,INT32 *Frequency) PURE;
    STDMETHOD_(BOOL,SetDemodulatorFilterBandwidth)(THIS_ UINT32 Channel,UINT32 Bandwidth) PURE;
    STDMETHOD_(BOOL,GetDemodulatorFilterBandwidth)(THIS_ UINT32 Channel,UINT32 *Bandwidth) PURE;
    STDMETHOD_(BOOL,SetDemodulatorFilterShift)(THIS_ UINT32 Channel,INT32 Shift) PURE;
    STDMETHOD_(BOOL,GetDemodulatorFilterShift)(THIS_ UINT32 Channel,INT32 *Shift) PURE;
    STDMETHOD_(BOOL,SetDemodulatorFilterLength)(THIS_ UINT32 Channel,UINT32 Length) PURE;   //Length: min=64,max=32768,step=64
    STDMETHOD_(BOOL,GetDemodulatorFilterLength)(THIS_ UINT32 Channel,UINT32 *Length) PURE;

    STDMETHOD_(BOOL,SetDemodulatorParam)(THIS_ UINT32 Channel,UINT32 Code,CONST VOID *Buffer,UINT32 BufferSize) PURE;
    STDMETHOD_(BOOL,GetDemodulatorParam)(THIS_ UINT32 Channel,UINT32 Code,VOID *Buffer,UINT32 BufferSize) PURE;
    STDMETHOD_(BOOL,GetDemodulatorState)(THIS_ UINT32 Channel,UINT32 Code,VOID *Buffer,UINT32 BufferSize) PURE;  

    STDMETHOD_(BOOL,StartAudio)(THIS_ UINT32 Channel,UINT32 SamplesPerBuffer) PURE;
    STDMETHOD_(BOOL,StopAudio)(THIS_ UINT32 Channel) PURE;
    
    STDMETHOD_(BOOL,StartAudioPlayback)(THIS_ UINT32 Channel,UINT32 SamplesPerBuffer) PURE;
    STDMETHOD_(BOOL,PauseAudioPlayback)(THIS_ UINT32 Channel) PURE;
    STDMETHOD_(BOOL,ResumeAudioPlayback)(THIS_ UINT32 Channel) PURE;

    STDMETHOD_(BOOL,SetAudioFilter)(THIS_ UINT32 Channel,BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetAudioFilter)(THIS_ UINT32 Channel,BOOL *Enabled) PURE;
    STDMETHOD_(BOOL,SetAudioFilterParams)(THIS_ UINT32 Channel,UINT32 CutOffLow,UINT32 CutOffHigh,double Deemphasis) PURE;    
    STDMETHOD_(BOOL,GetAudioFilterParams)(THIS_ UINT32 Channel,UINT32 *CutOffLow,UINT32 *CutOffHigh,double *Deemphasis) PURE;    
    STDMETHOD_(BOOL,SetAudioFilterLength)(THIS_ UINT32 Channel,UINT32 Length) PURE;         //Length: min=64,max=32768,step64
    STDMETHOD_(BOOL,GetAudioFilterLength)(THIS_ UINT32 Channel,UINT32 *Length) PURE;
    STDMETHOD_(BOOL,SetAudioGain)(THIS_ UINT32 Channel,double Gain) PURE;
    STDMETHOD_(BOOL,GetAudioGain)(THIS_ UINT32 Channel,double *Gain) PURE;  
    STDMETHOD_(BOOL,SetVolume)(THIS_ UINT32 Channel,BYTE Volume) PURE;
    STDMETHOD_(BOOL,GetVolume)(THIS_ UINT32 Channel,BYTE *Volume) PURE;    
    STDMETHOD_(BOOL,SetMute)(THIS_ UINT32 Channel,BOOL Mute) PURE;
    STDMETHOD_(BOOL,GetMute)(THIS_ UINT32 Channel,BOOL *Mute) PURE;

    STDMETHOD_(BOOL,SetDRMKey)(THIS_ CONST CHAR *DRMKeyFileDirectory) PURE;
    STDMETHOD_(BOOL,IsDRMUnlocked)(THIS) PURE;

    STDMETHOD_(BOOL,GetSpectrumCompensation)(THIS_ UINT32 CenterFrequency,UINT32 Width,FLOAT *Buffer,UINT32 Count) PURE;

    STDMETHOD_(BOOL,SetFrequency)(THIS_ UINT32 Channel,UINT32 Frequency) PURE;
    STDMETHOD_(BOOL,GetFrequency)(THIS_ UINT32 Channel,UINT32 *Frequency) PURE;

    STDMETHOD_(BOOL,GetTemperature)(THIS_ UINT32 *Temperature) PURE;
    STDMETHOD_(BOOL,GetDeviceState)(THIS_ UINT32 *State) PURE;

    STDMETHOD_(BOOL,SetDAC)(THIS_ UINT32 DAC) PURE;
    STDMETHOD_(BOOL,GetDAC)(THIS_ UINT32 *DAC) PURE;
};


#undef INTERFACE
#define INTERFACE ICohG35DDCDeviceSetEnumerator

DECLARE_INTERFACE(ICohG35DDCDeviceSetEnumerator)
{
    STDMETHOD_(LONG,AddRef)(THIS) PURE;
    STDMETHOD_(LONG,Release)(THIS) PURE;

    STDMETHOD_(BOOL,Enumerate)(THIS) PURE;
    STDMETHOD_(UINT32,GetDeviceSetCount)(THIS) PURE;
    STDMETHOD_(BOOL,GetDeviceSetInfo)(THIS_ UINT32 SetIndex,G35DDC_DEVICE_INFO *DeviceInfos,UINT32 *DeviceInfosCount) PURE;
};


#undef INTERFACE
#define INTERFACE ICohG35DDCDeviceSetCallback

struct ICohG35DDCDeviceSet;

DECLARE_INTERFACE(ICohG35DDCDeviceSetCallback)
{
    STDMETHOD_(VOID,CohG35DDC_DDC1StreamCallback)(THIS_ struct ICohG35DDCDeviceSet *DeviceSet,UINT32 DeviceCount,CONST VOID **Buffers,UINT32 NumberOfSamples,UINT32 BitsPerSample) PURE;
    STDMETHOD_(VOID,CohG35DDC_IFCallback)(THIS_ struct ICohG35DDCDeviceSet *DeviceSet,UINT32 DeviceIndex,CONST SHORT *Buffer,UINT32 NumberOfSamples,WORD MaxADCAmplitude,UINT32 ADCSamplingRate) PURE;    
    STDMETHOD_(VOID,CohG35DDC_DDC2StreamCallback)(THIS_ struct ICohG35DDCDeviceSet *DeviceSet,UINT32 DeviceIndex,CONST FLOAT *Buffer,UINT32 NumberOfSamples) PURE;        
    //SlevelPeak and SlevelRMS are in Volts
    //to convert SlevelRMS to power in dBm use SlevelRMS_dBm=10.0*log10(SlevelRMS*SlevelRMS/50.0*1000.0); where 50.0 is receiver input impendance, 1000.0 is coefficents to convert Watts to mili Watts
    STDMETHOD_(VOID,CohG35DDC_DDC2PreprocessedStreamCallback)(THIS_ struct ICohG35DDCDeviceSet *Device,UINT32 DeviceIndex,CONST FLOAT *Buffer,UINT32 NumberOfSamples,FLOAT SlevelPeak,FLOAT SlevelRMS) PURE;        
    STDMETHOD_(VOID,CohG35DDC_AudioStreamCallback)(THIS_ struct ICohG35DDCDeviceSet *Device,UINT32 DeviceIndex,UINT32 Type,CONST FLOAT *Buffer,UINT32 NumberOfSamples) PURE;    
};

#undef INTERFACE
#define INTERFACE ICohG35DDCDeviceSet


DECLARE_INTERFACE(ICohG35DDCDeviceSet)
{
    STDMETHOD_(LONG,AddRef)(THIS) PURE;
    STDMETHOD_(LONG,Release)(THIS) PURE;

    STDMETHOD_(BOOL,Open)(THIS_ G35DDC_DEVICE_INFO *DeviceInfos,UINT32 Count) PURE;
    STDMETHOD_(VOID,Close)(THIS) PURE;
    STDMETHOD_(BOOL,IsOpen)(THIS) PURE;
    
    STDMETHOD_(BOOL,GetDeviceCount)(THIS_ UINT32 *Count) PURE;
    
    STDMETHOD_(BOOL,IsConnected)(THIS) PURE;

    STDMETHOD_(VOID,SetCallback)(THIS_ ICohG35DDCDeviceSetCallback *Callback) PURE;
    STDMETHOD_(ICohG35DDCDeviceSetCallback*,GetCallback)(THIS) PURE;

    STDMETHOD_(BOOL,GetDeviceInfo)(THIS_ UINT32 DeviceIndex,G35DDC_DEVICE_INFO *Info) PURE;
    STDMETHOD_(BOOL,GetDeviceInfos)(THIS_ G35DDC_DEVICE_INFO *Infos,UINT32 *InfoCount) PURE;

    STDMETHOD_(BOOL,SetPower)(THIS_ BOOL Power) PURE;
    STDMETHOD_(BOOL,GetPower)(THIS_ BOOL *Power) PURE;
    
    STDMETHOD_(BOOL,SetAttenuator)(THIS_ UINT32 Attenuator) PURE;
    STDMETHOD_(BOOL,GetAttenuator)(THIS_ UINT32 *Attenuator) PURE;

    STDMETHOD_(BOOL,SetPreselectors)(THIS_ UINT32 Low,UINT32 High) PURE;
    STDMETHOD_(BOOL,GetPreselectors)(THIS_ UINT32 *Low,UINT32 *High) PURE;

    STDMETHOD_(BOOL,SetPreamp)(THIS_ BOOL Preamp) PURE;
    STDMETHOD_(BOOL,GetPreamp)(THIS_ BOOL *Preamp) PURE;    
    
    STDMETHOD_(BOOL,SetInverted)(THIS_ BOOL Inverted) PURE;
    STDMETHOD_(BOOL,GetInverted)(THIS_ BOOL *Inverted) PURE;       


    STDMETHOD_(BOOL,SetADCNoiseBlanker)(THIS_ BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetADCNoiseBlanker)(THIS_ BOOL *Enabled) PURE;
    STDMETHOD_(BOOL,SetADCNoiseBlankerThreshold)(THIS_ WORD Threshold) PURE;
    STDMETHOD_(BOOL,GetADCNoiseBlankerThreshold)(THIS_ WORD *Threshold) PURE;

    STDMETHOD_(BOOL,StartIF)(THIS_ UINT32 DeviceIndex,WORD Period) PURE;
    STDMETHOD_(BOOL,StopIF)(THIS_ UINT32 DeviceIndex) PURE;

    STDMETHOD_(BOOL,GetDDCInfo)(THIS_ UINT32 DDCTypeIndex,G3XDDC_DDC_INFO *Info) PURE;
    STDMETHOD_(BOOL,GetDDC1Count)(THIS_ UINT32 *Count) PURE;
    STDMETHOD_(BOOL,SetDDC1)(THIS_ UINT32 DDCTypeIndex) PURE;
    STDMETHOD_(BOOL,GetDDC1)(THIS_ UINT32 *DDCTypeIndex,G3XDDC_DDC_INFO *DDCInfo) PURE;
    STDMETHOD_(BOOL,StartDDC1)(THIS_ UINT32 SamplesPerBuffer) PURE;
    STDMETHOD_(BOOL,StopDDC1)(THIS) PURE;  
    STDMETHOD_(BOOL,SetDDC1Frequency)(THIS_ UINT32 Frequency) PURE;
    STDMETHOD_(BOOL,GetDDC1Frequency)(THIS_ UINT32 *Frequency) PURE;
    STDMETHOD_(BOOL,SetDDC1PhaseShift)(THIS_ UINT32 DeviceIndex,double PhaseShift) PURE;
    STDMETHOD_(BOOL,GetDDC1PhaseShift)(THIS_ UINT32 DeviceIndex,double *PhaseShift) PURE;

    STDMETHOD_(BOOL,GetDDC2)(THIS_ UINT32 *DDCTypeIndex,G3XDDC_DDC_INFO *DDCInfo) PURE;

    STDMETHOD_(BOOL,StartDDC2)(THIS_ UINT32 DeviceIndex,UINT32 SamplesPerBuffer) PURE;
    STDMETHOD_(BOOL,StopDDC2)(THIS_ UINT32 DeviceIndex) PURE;

    STDMETHOD_(BOOL,SetDDC2Frequency)(THIS_ UINT32 DeviceIndex,INT32 Frequency) PURE;
    STDMETHOD_(BOOL,GetDDC2Frequency)(THIS_ UINT32 DeviceIndex,INT32 *Frequency) PURE;

    STDMETHOD_(BOOL,SetDDC2NoiseBlanker)(THIS_ UINT32 DeviceIndex,BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetDDC2NoiseBlanker)(THIS_ UINT32 DeviceIndex,BOOL *Enabled) PURE;
    STDMETHOD_(BOOL,SetDDC2NoiseBlankerThreshold)(THIS_ UINT32 DeviceIndex,double Threshold) PURE;
    STDMETHOD_(BOOL,GetDDC2NoiseBlankerThreshold)(THIS_ UINT32 DeviceIndex,double *Threshold) PURE;
    STDMETHOD_(BOOL,GetDDC2NoiseBlankerExcessValue)(THIS_ UINT32 DeviceIndex,double *Value) PURE;

    STDMETHOD_(BOOL,GetSignalLevel)(THIS_ UINT32 DeviceIndex,FLOAT *Peak,FLOAT *RMS) PURE;

    STDMETHOD_(BOOL,SetAGC)(THIS_ UINT32 DeviceIndex,BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetAGC)(THIS_ UINT32 DeviceIndex,BOOL *Enabled) PURE;
    STDMETHOD_(BOOL,SetAGCParams)(THIS_ UINT32 DeviceIndex,double AttackTime,double DecayTime,double ReferenceLevel) PURE;
    STDMETHOD_(BOOL,GetAGCParams)(THIS_ UINT32 DeviceIndex,double *AttackTime,double *DecayTime,double *ReferenceLevel) PURE;
    STDMETHOD_(BOOL,SetMaxAGCGain)(THIS_ UINT32 DeviceIndex,double MaxGain) PURE;
    STDMETHOD_(BOOL,GetMaxAGCGain)(THIS_ UINT32 DeviceIndex,double *MaxGain) PURE;
    STDMETHOD_(BOOL,SetGain)(THIS_ UINT32 DeviceIndex,double Gain) PURE;
    STDMETHOD_(BOOL,GetGain)(THIS_ UINT32 DeviceIndex,double *Gain) PURE;
    STDMETHOD_(BOOL,GetCurrentGain)(THIS_ UINT32 DeviceIndex,double *Gain) PURE;

    STDMETHOD_(BOOL,SetNotchFilter)(THIS_ UINT32 DeviceIndex,UINT32 NotchFilterIndex,BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetNotchFilter)(THIS_ UINT32 DeviceIndex,UINT32 NotchFilterIndex,BOOL *Enabled) PURE;
    STDMETHOD_(BOOL,SetNotchFilterFrequency)(THIS_ UINT32 DeviceIndex,UINT32 NotchFilterIndex,INT32 Frequency) PURE;
    STDMETHOD_(BOOL,GetNotchFilterFrequency)(THIS_ UINT32 DeviceIndex,UINT32 NotchFilterIndex,INT32 *Frequency) PURE;
    STDMETHOD_(BOOL,SetNotchFilterBandwidth)(THIS_ UINT32 DeviceIndex,UINT32 NotchFilterIndex,UINT32 Bandwidth) PURE;
    STDMETHOD_(BOOL,GetNotchFilterBandwidth)(THIS_ UINT32 DeviceIndex,UINT32 NotchFilterIndex,UINT32 *Bandwidth) PURE;
    STDMETHOD_(BOOL,SetNotchFilterLength)(THIS_ UINT32 DeviceIndex,UINT32 NotchFilterIndex,UINT32 Length) PURE;         //Length: min=64,max=32768,step=64
    STDMETHOD_(BOOL,GetNotchFilterLength)(THIS_ UINT32 DeviceIndex,UINT32 NotchFilterIndex,UINT32 *Length) PURE;


    STDMETHOD_(BOOL,SetDemodulatorMode)(THIS_ UINT32 DeviceIndex,UINT32 Mode) PURE;
    STDMETHOD_(BOOL,GetDemodulatorMode)(THIS_ UINT32 DeviceIndex,UINT32 *Mode) PURE;
    STDMETHOD_(BOOL,SetDemodulatorFrequency)(THIS_ UINT32 DeviceIndex,INT32 Frequency) PURE;
    STDMETHOD_(BOOL,GetDemodulatorFrequency)(THIS_ UINT32 DeviceIndex,INT32 *Frequency) PURE;
    STDMETHOD_(BOOL,SetDemodulatorFilterBandwidth)(THIS_ UINT32 DeviceIndex,UINT32 Bandwidth) PURE;
    STDMETHOD_(BOOL,GetDemodulatorFilterBandwidth)(THIS_ UINT32 DeviceIndex,UINT32 *Bandwidth) PURE;
    STDMETHOD_(BOOL,SetDemodulatorFilterShift)(THIS_ UINT32 DeviceIndex,INT32 Shift) PURE;
    STDMETHOD_(BOOL,GetDemodulatorFilterShift)(THIS_ UINT32 DeviceIndex,INT32 *Shift) PURE;
    STDMETHOD_(BOOL,SetDemodulatorFilterLength)(THIS_ UINT32 DeviceIndex,UINT32 Length) PURE;   //Length: min=64,max=32768,step=64
    STDMETHOD_(BOOL,GetDemodulatorFilterLength)(THIS_ UINT32 DeviceIndex,UINT32 *Length) PURE;

    STDMETHOD_(BOOL,SetDemodulatorParam)(THIS_ UINT32 DeviceIndex,UINT32 Code,CONST VOID *Buffer,UINT32 BufferSize) PURE;
    STDMETHOD_(BOOL,GetDemodulatorParam)(THIS_ UINT32 DeviceIndex,UINT32 Code,VOID *Buffer,UINT32 BufferSize) PURE;
    STDMETHOD_(BOOL,GetDemodulatorState)(THIS_ UINT32 DeviceIndex,UINT32 Code,VOID *Buffer,UINT32 BufferSize) PURE;  

    STDMETHOD_(BOOL,StartAudio)(THIS_ UINT32 DeviceIndex,UINT32 SamplesPerBuffer) PURE;
    STDMETHOD_(BOOL,StopAudio)(THIS_ UINT32 DeviceIndex) PURE;
       
    STDMETHOD_(BOOL,SetAudioFilter)(THIS_ UINT32 DeviceIndex,BOOL Enabled) PURE;
    STDMETHOD_(BOOL,GetAudioFilter)(THIS_ UINT32 DeviceIndex,BOOL *Enabled) PURE;
    STDMETHOD_(BOOL,SetAudioFilterParams)(THIS_ UINT32 DeviceIndex,UINT32 CutOffLow,UINT32 CutOffHigh,double Deemphasis) PURE;    
    STDMETHOD_(BOOL,GetAudioFilterParams)(THIS_ UINT32 DeviceIndex,UINT32 *CutOffLow,UINT32 *CutOffHigh,double *Deemphasis) PURE;    
    STDMETHOD_(BOOL,SetAudioFilterLength)(THIS_ UINT32 DeviceIndex,UINT32 Length) PURE;         //Length: min=64,max=32768,step64
    STDMETHOD_(BOOL,GetAudioFilterLength)(THIS_ UINT32 DeviceIndex,UINT32 *Length) PURE;
    STDMETHOD_(BOOL,SetAudioGain)(THIS_ UINT32 DeviceIndex,double Gain) PURE;
    STDMETHOD_(BOOL,GetAudioGain)(THIS_ UINT32 DeviceIndex,double *Gain) PURE;  
    STDMETHOD_(BOOL,SetVolume)(THIS_ UINT32 DeviceIndex,BYTE Volume) PURE;
    STDMETHOD_(BOOL,GetVolume)(THIS_ UINT32 DeviceIndex,BYTE *Volume) PURE;    
    STDMETHOD_(BOOL,SetMute)(THIS_ UINT32 DeviceIndex,BOOL Mute) PURE;
    STDMETHOD_(BOOL,GetMute)(THIS_ UINT32 DeviceIndex,BOOL *Mute) PURE;

    STDMETHOD_(BOOL,SetDRMKey)(THIS_ CONST CHAR *DRMKeyFileDirectory) PURE;
    STDMETHOD_(BOOL,IsDRMUnlocked)(THIS) PURE;

    STDMETHOD_(BOOL,GetSpectrumCompensation)(THIS_ UINT32 DeviceIndex,UINT32 CenterFrequency,UINT32 Width,FLOAT *Buffer,UINT32 Count) PURE;    

    STDMETHOD_(BOOL,GetTemperature)(THIS_ UINT32 DeviceIndex,UINT32 *Temperature) PURE;
    STDMETHOD_(BOOL,GetDeviceState)(THIS_ UINT32 DeviceIndex,UINT32 *State) PURE;

    STDMETHOD_(BOOL,SetDAC)(THIS_ UINT32 DeviceIndex,UINT32 DAC) PURE;
    STDMETHOD_(BOOL,GetDAC)(THIS_ UINT32 DeviceIndex,UINT32 *DAC) PURE;
};

#pragma pack(pop)

//non-object API

typedef VOID (__stdcall *G35DDC_IF_CALLBACK)(CONST SHORT *Buffer,UINT32 NumberOfSamples,WORD MaxADCAmplitude,UINT32 ADCSamplingRate,DWORD_PTR UserData);
typedef VOID (__stdcall *G35DDC_DDC1_STREAM_CALLBACK)(CONST VOID *Buffer,UINT32 NumberOfSamples,UINT32 BitsPerSample,DWORD_PTR UserData);
typedef BOOL (__stdcall *G35DDC_DDC1_PLAYBACK_STREAM_CALLBACK)(VOID *Buffer,UINT32 NumberOfSamples,UINT32 BitsPerSample,DWORD_PTR UserData);
typedef VOID (__stdcall *G35DDC_DDC2_STREAM_CALLBACK)(UINT32 Channel,CONST FLOAT *Buffer,UINT32 NumberOfSamples,DWORD_PTR UserData);
typedef VOID (__stdcall *G35DDC_DDC2_PREPROCESSED_STREAM_CALLBACK)(UINT32 Channel,CONST FLOAT *Buffer,UINT32 NumberOfSamples,FLOAT SlevelPeak,FLOAT SlevelRMS,DWORD_PTR UserData);
typedef VOID (__stdcall *G35DDC_AUDIO_STREAM_CALLBACK)(UINT32 Channel,UINT32 Type,CONST FLOAT *Buffer,UINT32 NumberOfSamples,DWORD_PTR UserData);
typedef BOOL (__stdcall *G35DDC_AUDIO_PLAYBACK_STREAM_CALLBACK)(UINT32 Channel,FLOAT *Buffer,UINT32 NumberOfSamples,DWORD_PTR UserData);

#pragma pack(push,1)

typedef struct
{
    G35DDC_IF_CALLBACK                          IFCallback;
    G35DDC_DDC1_STREAM_CALLBACK                 DDC1StreamCallback;
    G35DDC_DDC1_PLAYBACK_STREAM_CALLBACK        DDC1PlaybackStreamCallback;
    G35DDC_DDC2_STREAM_CALLBACK                 DDC2StreamCallback;
    G35DDC_DDC2_PREPROCESSED_STREAM_CALLBACK    DDC2PreprocessedStreamCallback;
    G35DDC_AUDIO_STREAM_CALLBACK                AudioStreamCallback;
    G35DDC_AUDIO_PLAYBACK_STREAM_CALLBACK       AudioPlaybackStreamCallback;
} G35DDC_CALLBACKS;

#pragma pack(pop)

//GetDeviceList
typedef INT32 (__stdcall *G35DDC_GET_DEVICE_LIST)(G35DDC_DEVICE_INFO *DeviceList,UINT32 BufferSize);
//OpenDevice
typedef INT32 (__stdcall *G35DDC_OPEN_DEVICE)(const char *SerialNumber);
//CloseDevice
typedef BOOL (__stdcall *G35DDC_CLOSE_DEVICE)(INT32 hDevice);
//IsDeviceConnected
typedef BOOL (__stdcall *G35DDC_IS_DEVICE_CONNECTED)(INT32 hDevice);
//GetDeviceInfo
typedef BOOL (__stdcall *G35DDC_GET_DEVICE_INFO)(INT32 hDevice,G35DDC_DEVICE_INFO *Info,UINT32 BufferLength);
//SetPower
typedef BOOL (__stdcall *G35DDC_SET_POWER)(INT32 hDevice,BOOL Power);
//GetPower
typedef BOOL (__stdcall *G35DDC_GET_POWER)(INT32 hDevice,BOOL *Power);
//SetExternalReference
typedef BOOL (__stdcall *G35DDC_SET_EXTERNAL_REFERENCE)(INT32 hDevice,BOOL Enabled);
//GetExternalReference
typedef BOOL (__stdcall *G35DDC_GET_EXTERNAL_REFERENCE)(INT32 hDevice,BOOL *Enabled);
//SetAttenuator
typedef BOOL (__stdcall *G35DDC_SET_ATTENUATOR)(INT32 hDevice,UINT32 Attenuator);
//GetAttenuator
typedef BOOL (__stdcall *G35DDC_GET_ATTENUATOR)(INT32 hDevice,UINT32 *Attenuator);
//SetPreselectors
typedef BOOL (__stdcall *G35DDC_SET_PRESELECTORS)(INT32 hDevice,UINT32 Low,UINT32 High);
//GetPreselectors
typedef BOOL (__stdcall *G35DDC_GET_PRESELECTORS)(INT32 hDevice,UINT32 *Low,UINT32 *High);
//SetPreamp
typedef BOOL (__stdcall *G35DDC_SET_PREAMP)(INT32 hDevice,BOOL Preamp);
//GetPreamp
typedef BOOL (__stdcall *G35DDC_GET_PREAMP)(INT32 hDevice,BOOL *Preamp);
//SetInverted    
typedef BOOL (__stdcall *G35DDC_SET_INVERTED)(INT32 hDevice,BOOL Inverted);
//GetInverted    
typedef BOOL (__stdcall *G35DDC_GET_INVERTED)(INT32 hDevice,BOOL *Inverted);
//SetADCNoiseBlanker
typedef BOOL (__stdcall *G35DDC_SET_ADC_NOISE_BLANKER)(INT32 hDevice,BOOL Enabled);
//GetADCNoiseBlanker
typedef BOOL (__stdcall *G35DDC_GET_ADC_NOISE_BLANKER)(INT32 hDevice,BOOL *Enabled);
//SetADCNoiseBlankerThreshold
typedef BOOL (__stdcall *G35DDC_SET_ADC_NOISE_BLANKER_THRESHOLD)(INT32 hDevice,WORD Threshold);
//GetADCNoiseBlankerThreshold
typedef BOOL (__stdcall *G35DDC_GET_ADC_NOISE_BLANKER_THRESHOLD)(INT32 hDevice,WORD *Threshold);
//StartIF
typedef BOOL (__stdcall *G35DDC_START_IF)(INT32 hDevice,WORD Period);
//StopIF
typedef BOOL (__stdcall *G35DDC_STOP_IF)(INT32 hDevice);
//GetDDCInfo
typedef BOOL (__stdcall *G35DDC_GET_DDC_INFO)(INT32 hDevice,UINT32 DDCTypeIndex,G3XDDC_DDC_INFO *Info);
//GetDDC1Count
typedef BOOL (__stdcall *G35DDC_GET_DDC1_COUNT)(INT32 hDevice,UINT32 *Count);
//SetDDC1
typedef BOOL (__stdcall *G35DDC_SET_DDC1)(INT32 hDevice,UINT32 DDCTypeIndex);
//GetDDC1
typedef BOOL (__stdcall *G35DDC_GET_DDC1)(INT32 hDevice,UINT32 *DDCTypeIndex,G3XDDC_DDC_INFO *DDCInfo);
//StartDDC1
typedef BOOL (__stdcall *G35DDC_START_DDC1)(INT32 hDevice,UINT32 SamplesPerBuffer);
//StopDDC1
typedef BOOL (__stdcall *G35DDC_STOP_DDC1)(INT32 hDevice);
//StartDDC1Playback
typedef BOOL (__stdcall *G35DDC_START_DDC1_PLAYBACK)(INT32 hDevice,UINT32 SamplesPerBuffer,UINT32 BitsPerSample);
//PauseDDC1Playback
typedef BOOL (__stdcall *G35DDC_PAUSE_DDC1_PLAYBACK)(INT32 hDevice);
//ResumeDDC1Playback
typedef BOOL (__stdcall *G35DDC_RESUME_DDC1_PLAYBACK)(INT32 hDevice);
//SetDDC1Frequency
typedef BOOL (__stdcall *G35DDC_SET_DDC1_FREQUENCY)(INT32 hDevice,UINT32 Frequency);
//GetDDC1Frequency
typedef BOOL (__stdcall *G35DDC_GET_DDC1_FREQUENCY)(INT32 hDevice,UINT32 *Frequency);
//GetDDC2
typedef BOOL (__stdcall *G35DDC_GET_DDC2)(INT32 hDevice,UINT32 *DDCTypeIndex,G3XDDC_DDC_INFO *DDCInfo);
//StartDDC2
typedef BOOL (__stdcall *G35DDC_START_DDC2)(INT32 hDevice,UINT32 Channel,UINT32 SamplesPerBuffer);
//StopDDC2
typedef BOOL (__stdcall *G35DDC_STOP_DDC2)(INT32 hDevice,UINT32 Channel);
//SetDDC2Frequency
typedef BOOL (__stdcall *G35DDC_SET_DDC2_FREQUENCY)(INT32 hDevice,UINT32 Channel,INT32 Frequency);
//GetDDC2Frequency
typedef BOOL (__stdcall *G35DDC_GET_DDC2_FREQUENCY)(INT32 hDevice,UINT32 Channel,INT32 *Frequency);
//SetDDC2NoiseBlanker
typedef BOOL (__stdcall *G35DDC_SET_DDC2_NOISE_BLANKER)(INT32 hDevice,UINT32 Channel,BOOL Enabled);
//GetDDC2NoiseBlanker
typedef BOOL (__stdcall *G35DDC_GET_DDC2_NOISE_BLANKER)(INT32 hDevice,UINT32 Channel,BOOL *Enabled);
//SetDDC2NoiseBlankerThreshold
typedef BOOL (__stdcall *G35DDC_SET_DDC2_NOISE_BLANKER_THRESHOLD)(INT32 hDevice,UINT32 Channel,double Threshold);
//GetDDC2NoiseBlankerThreshold
typedef BOOL (__stdcall *G35DDC_GET_DDC2_NOISE_BLANKER_THRESHOLD)(INT32 hDevice,UINT32 Channel,double *Threshold);
//GetDDC2NoiseBlankerExcessValue
typedef BOOL (__stdcall *G35DDC_GET_DDC2_NOISE_BLANKER_EXCESS_VALUE)(INT32 hDevice,UINT32 Channel,double *Value);
//GetSignalLevel
typedef BOOL (__stdcall *G35DDC_GET_SIGNAL_LEVEL)(INT32 hDevice,UINT32 Channel,FLOAT *Peak,FLOAT *RMS);
//SetAGC
typedef BOOL (__stdcall *G35DDC_SET_AGC)(INT32 hDevice,UINT32 Channel,BOOL Enabled);
//GetAGC
typedef BOOL (__stdcall *G35DDC_GET_AGC)(INT32 hDevice,UINT32 Channel,BOOL *Enabled);
//SetAGCParams
typedef BOOL (__stdcall *G35DDC_SET_AGC_PARAMS)(INT32 hDevice,UINT32 Channel,double AttackTime,double DecayTime,double ReferenceLevel);
//GetAGCParams
typedef BOOL (__stdcall *G35DDC_GET_AGC_PARAMS)(INT32 hDevice,UINT32 Channel,double *AttackTime,double *DecayTime,double *ReferenceLevel);
//SetMaxAGCGain
typedef BOOL (__stdcall *G35DDC_SET_MAX_AGC_GAIN)(INT32 hDevice,UINT32 Channel,double MaxGain);
//GetMaxAGCGain
typedef BOOL (__stdcall *G35DDC_GET_MAX_AGC_GAIN)(INT32 hDevice,UINT32 Channel,double *MaxGain);
//SetGain
typedef BOOL (__stdcall *G35DDC_SET_GAIN)(INT32 hDevice,UINT32 Channel,double Gain);
//GetGain
typedef BOOL (__stdcall *G35DDC_GET_GAIN)(INT32 hDevice,UINT32 Channel,double *Gain);
//GetCurrentGain
typedef BOOL (__stdcall *G35DDC_GET_CURRENT_GAIN)(INT32 hDevice,UINT32 Channel,double *Gain);
//SetNotchFilter
typedef BOOL (__stdcall *G35DDC_SET_NOTCH_FILTER)(INT32 hDevice,UINT32 Channel,UINT32 NotchFilterIndex,BOOL Enabled);
//GetNotchFilter
typedef BOOL (__stdcall *G35DDC_GET_NOTCH_FILTER)(INT32 hDevice,UINT32 Channel,UINT32 NotchFilterIndex,BOOL *Enabled);
//SetNotchFilterFrequency
typedef BOOL (__stdcall *G35DDC_SET_NOTCH_FILTER_FREQUENCY)(INT32 hDevice,UINT32 Channel,UINT32 NotchFilterIndex,INT32 Frequency);
//GetNotchFilterFrequency
typedef BOOL (__stdcall *G35DDC_GET_NOTCH_FILTER_FREQUENCY)(INT32 hDevice,UINT32 Channel,UINT32 NotchFilterIndex,INT32 *Frequency);
//SetNotchFilterBandwidth
typedef BOOL (__stdcall *G35DDC_SET_NOTCH_FILTER_BANDWIDTH)(INT32 hDevice,UINT32 Channel,UINT32 NotchFilterIndex,UINT32 Bandwidth);
//GetNotchFilterBandwidth
typedef BOOL (__stdcall *G35DDC_GET_NOTCH_FILTER_BANDWIDTH)(INT32 hDevice,UINT32 Channel,UINT32 NotchFilterIndex,UINT32 *Bandwidth);
//SetNotchFilterLength
typedef BOOL (__stdcall *G35DDC_SET_NOTCH_FILTER_LENGTH)(INT32 hDevice,UINT32 Channel,UINT32 NotchFilterIndex,UINT32 Length);
//GetNotchFilterLength
typedef BOOL (__stdcall *G35DDC_GET_NOTCH_FILTER_LENGTH)(INT32 hDevice,UINT32 Channel,UINT32 NotchFilterIndex,UINT32 *Length);
//SetDemodulatorMode
typedef BOOL (__stdcall *G35DDC_SET_DEMODULATOR_MODE)(INT32 hDevice,UINT32 Channel,UINT32 Mode);
//GetDemodulatorMode
typedef BOOL (__stdcall *G35DDC_GET_DEMODULATOR_MODE)(INT32 hDevice,UINT32 Channel,UINT32 *Mode);
//SetDemodulatorFrequency
typedef BOOL (__stdcall *G35DDC_SET_DEMODULATOR_FREQUENCY)(INT32 hDevice,UINT32 Channel,INT32 Frequency);
//GetDemodulatorFrequency
typedef BOOL (__stdcall *G35DDC_GET_DEMODULATOR_FREQUENCY)(INT32 hDevice,UINT32 Channel,INT32 *Frequency);
//SetDemodulatorFilterBandwidth
typedef BOOL (__stdcall *G35DDC_SET_DEMODULATOR_FILTER_BANDWIDTH)(INT32 hDevice,UINT32 Channel,UINT32 Bandwidth);
//GetDemodulatorFilterBandwidth
typedef BOOL (__stdcall *G35DDC_GET_DEMODULATOR_FILTER_BANDWIDTH)(INT32 hDevice,UINT32 Channel,UINT32 *Bandwidth);
//SetDemodulatorFilterShift
typedef BOOL (__stdcall *G35DDC_SET_DEMODULATOR_FILTER_SHIFT)(INT32 hDevice,UINT32 Channel,INT32 Shift);
//GetDemodulatorFilterShift
typedef BOOL (__stdcall *G35DDC_GET_DEMODULATOR_FILTER_SHIFT)(INT32 hDevice,UINT32 Channel,INT32 *Shift);
//SetDemodulatorFilterLength
typedef BOOL (__stdcall *G35DDC_SET_DEMODULATOR_FILTER_LENGTH)(INT32 hDevice,UINT32 Channel,UINT32 Length);
//GetDemodulatorFilterLength
typedef BOOL (__stdcall *G35DDC_GET_DEMODULATOR_FILTER_LENGTH)(INT32 hDevice,UINT32 Channel,UINT32 *Length);
//SetDemodulatorParam
typedef BOOL (__stdcall *G35DDC_SET_DEMODULATOR_PARAM)(INT32 hDevice,UINT32 Channel,UINT32 Code,CONST VOID *Buffer,UINT32 BufferSize);
//GetDemodulatorParam
typedef BOOL (__stdcall *G35DDC_GET_DEMODULATOR_PARAM)(INT32 hDevice,UINT32 Channel,UINT32 Code,VOID *Buffer,UINT32 BufferSize);
//GetDemodulatorState
typedef BOOL (__stdcall *G35DDC_GET_DEMODULATOR_STATE)(INT32 hDevice,UINT32 Channel,UINT32 Code,VOID *Buffer,UINT32 BufferSize);
//StartAudio
typedef BOOL (__stdcall *G35DDC_START_AUDIO)(INT32 hDevice,UINT32 Channel,UINT32 SamplesPerBuffer);
//StopAudio
typedef BOOL (__stdcall *G35DDC_STOP_AUDIO)(INT32 hDevice,UINT32 Channel);
//StartAudioPlayback
typedef BOOL (__stdcall *G35DDC_START_AUDIO_PLAYBACK)(INT32 hDevice,UINT32 Channel,UINT32 SamplesPerBuffer);
//PauseAudioPlayback
typedef BOOL (__stdcall *G35DDC_PAUSE_AUDIO_PLAYBACK)(INT32 hDevice,UINT32 Channel);
//ResumeAudioPlayback
typedef BOOL (__stdcall *G35DDC_RESUME_AUDIO_PLAYBACK)(INT32 hDevice,UINT32 Channel);
//SetAudioFilter
typedef BOOL (__stdcall *G35DDC_SET_AUDIO_FILTER)(INT32 hDevice,UINT32 Channel,BOOL Enabled);
//GetAudioFilter
typedef BOOL (__stdcall *G35DDC_GET_AUDIO_FILTER)(INT32 hDevice,UINT32 Channel,BOOL *Enabled);
//SetAudioFilterParams
typedef BOOL (__stdcall *G35DDC_SET_AUDIO_FILTER_PARAMS)(INT32 hDevice,UINT32 Channel,UINT32 CutOffLow,UINT32 CutOffHigh,double Deemphasis);
//GetAudioFilterParams
typedef BOOL (__stdcall *G35DDC_GET_AUDIO_FILTER_PARAMS)(INT32 hDevice,UINT32 Channel,UINT32 *CutOffLow,UINT32 *CutOffHigh,double *Deemphasis);
//SetAudioFilterLength
typedef BOOL (__stdcall *G35DDC_SET_AUDIO_FILTER_LENGTH)(INT32 hDevice,UINT32 Channel,UINT32 Length);
//GetAudioFilterLength
typedef BOOL (__stdcall *G35DDC_GET_AUDIO_FILTER_LENGTH)(INT32 hDevice,UINT32 Channel,UINT32 *Length);
//SetAudioGain
typedef BOOL (__stdcall *G35DDC_SET_AUDIO_GAIN)(INT32 hDevice,UINT32 Channel,double Gain);
//GetAudioGain
typedef BOOL (__stdcall *G35DDC_GET_AUDIO_GAIN)(INT32 hDevice,UINT32 Channel,double *Gain);
//SetVolume
typedef BOOL (__stdcall *G35DDC_SET_VOLUME)(INT32 hDevice,UINT32 Channel,BYTE Volume);
//GetVolume
typedef BOOL (__stdcall *G35DDC_GET_VOLUME)(INT32 hDevice,UINT32 Channel,BYTE *Volume);
//SetMute
typedef BOOL (__stdcall *G35DDC_SET_MUTE)(INT32 hDevice,UINT32 Channel,BOOL Mute);
//GetMute
typedef BOOL (__stdcall *G35DDC_GET_MUTE)(INT32 hDevice,UINT32 Channel,BOOL *Mute);
//SetDRMKey
typedef BOOL (__stdcall *G35DDC_SET_DRM_KEY)(INT32 hDevice,CONST CHAR *DRMKeyFileDirectory);
//IsDRMUnlocked
typedef BOOL (__stdcall *G35DDC_IS_DRM_UNLOCKED)(INT32 hDevice);
//GetSpectrumCompensation
typedef BOOL (__stdcall *G35DDC_GET_SPECTRUM_COMPENSATION)(INT32 hDevice,UINT32 CenterFrequency,UINT32 Width,FLOAT *Buffer,UINT32 Count);
//SetFrequency
typedef BOOL (__stdcall *G35DDC_SET_FREQUENCY)(INT32 hDevice,UINT32 Channel,UINT32 Frequency);
//GetFrequency
typedef BOOL (__stdcall *G35DDC_GET_FREQUENCY)(INT32 hDevice,UINT32 Channel,UINT32 *Frequency);
//GetTemperature
typedef BOOL (__stdcall *G35DDC_GET_TEMPERATURE)(INT32 hDevice,UINT32 *Temperature);
//GetDeviceState
typedef BOOL (__stdcall *G35DDC_GET_DEVICE_STATE)(INT32 hDevice,UINT32 *State);
//SetDAC
typedef BOOL (__stdcall *G35DDC_SET_DAC)(INT32 hDevice,UINT32 DAC);
//GetDAC
typedef BOOL (__stdcall *G35DDC_GET_DAC)(INT32 hDevice,UINT32 *DAC);
//SetCallbacks
typedef BOOL (__stdcall *G35DDC_SET_CALLBACKS)(INT32 hDevice,CONST G35DDC_CALLBACKS *Callbacks,DWORD_PTR UserData);


typedef VOID (__stdcall *COH_G35DDC_DDC1_STREAM_CALLBACK)(UINT32 DeviceCount,CONST VOID **Buffers,UINT32 NumberOfSamples,UINT32 BitsPerSample,DWORD_PTR UserData);
typedef VOID (__stdcall *COH_G35DDC_IF_CALLBACK)(UINT32 DeviceIndex,CONST SHORT *Buffer,UINT32 NumberOfSamples,WORD MaxADCAmplitude,UINT32 ADCSamplingRate,DWORD_PTR UserData);
typedef VOID (__stdcall *COH_G35DDC_DDC2_STREAM_CALLBACK)(UINT32 DeviceIndex,CONST FLOAT *Buffer,UINT32 NumberOfSamples,DWORD_PTR UserData);
typedef VOID (__stdcall *COH_G35DDC_DDC2_PREPROCESSED_STREAM_CALLBACK)(UINT32 DeviceIndex,CONST FLOAT *Buffer,UINT32 NumberOfSamples,FLOAT SlevelPeak,FLOAT SlevelRMS,DWORD_PTR UserData);
typedef VOID (__stdcall *COH_G35DDC_AUDIO_STREAM_CALLBACK)(UINT32 DeviceIndex,UINT32 Type,CONST FLOAT *Buffer,UINT32 NumberOfSamples,DWORD_PTR UserData);

#pragma pack(push,1)

typedef struct
{
    COH_G35DDC_DDC1_STREAM_CALLBACK                 DDC1StreamCallback;
    COH_G35DDC_IF_CALLBACK                          IFCallback;
    COH_G35DDC_DDC2_STREAM_CALLBACK                 DDC2StreamCallback;
    COH_G35DDC_DDC2_PREPROCESSED_STREAM_CALLBACK    DDC2PreprocessedStreamCallback;
    COH_G35DDC_AUDIO_STREAM_CALLBACK                AudioStreamCallback;
} COH_G35DDC_CALLBACKS;

typedef struct
{
    UINT32              DeviceCount;
    G35DDC_DEVICE_INFO  *DeviceInfo;
} COH_G35DDC_DEVICE_SET;

typedef struct
{
    UINT32                  DeviceSetCount;
    COH_G35DDC_DEVICE_SET   *DeviceSet;    
} COH_G35DDC_DEVICE_SET_LIST;

#pragma pack(pop)

//CohGetDeviceSetList
typedef COH_G35DDC_DEVICE_SET_LIST* (__stdcall *COH_G35DDC_GET_DEVICE_SET_LIST)(void);
//CohFreeDeviceSetList
typedef void (__stdcall *COH_G35DDC_FREE_DEVICE_SET_LIST)(COH_G35DDC_DEVICE_SET_LIST *DeviceSetList);
//CohOpenDeviceSet
typedef INT32 (__stdcall *COH_G35DDC_OPEN_DEVICE_SET)(G35DDC_DEVICE_INFO *DeviceInfos,UINT32 Count);
//CohCloseDeviceSet
typedef BOOL (__stdcall *COH_G35DDC_CLOSE_DEVICE_SET)(INT32 hDeviceSet);
//CohIsConnected
typedef BOOL (__stdcall *COH_G35DDC_IS_CONNECTED)(INT32 hDeviceSet);
//CohSetCallbacks
typedef BOOL (__stdcall *COH_G35DDC_SET_CALLBACKS)(INT32 hDeviceSet,const COH_G35DDC_CALLBACKS *Callbacks,DWORD_PTR UserData);
//CohGetDeviceCount
typedef BOOL (__stdcall *COH_G35DDC_GET_DEVICE_COUNT)(INT32 hDeviceSet,UINT32 *Count);
//CohGetDeviceInfo
typedef BOOL (__stdcall *COH_G35DDC_GET_DEVICE_INFO)(INT32 hDeviceSet,UINT32 DeviceIndex,G35DDC_DEVICE_INFO *Info);
//CohGetDeviceInfos
typedef BOOL (__stdcall *COH_G35DDC_GET_DEVICE_INFOS)(INT32 hDeviceSet,G35DDC_DEVICE_INFO *Infos,UINT32 *InfoCount);
//CohSetPower
typedef BOOL (__stdcall *COH_G35DDC_SET_POWER)(INT32 hDeviceSet,BOOL Power);
//CohGetPower
typedef BOOL (__stdcall *COH_G35DDC_GET_POWER)(INT32 hDeviceSet,BOOL *Power);
//CohSetAttenuator
typedef BOOL (__stdcall *COH_G35DDC_SET_ATTENUATOR)(INT32 hDeviceSet,UINT32 Attenuator);
//CohGetAttenuator
typedef BOOL (__stdcall *COH_G35DDC_GET_ATTENUATOR)(INT32 hDeviceSet,UINT32 *Attenuator);
//CohSetPreselectors
typedef BOOL (__stdcall *COH_G35DDC_SET_PRESELECTORS)(INT32 hDeviceSet,UINT32 Low,UINT32 High);
//CohGetPreselectors
typedef BOOL (__stdcall *COH_G35DDC_GET_PRESELECTORS)(INT32 hDeviceSet,UINT32 *Low,UINT32 *High);
//CohSetPreamp
typedef BOOL (__stdcall *COH_G35DDC_SET_PREAMP)(INT32 hDeviceSet,BOOL Preamp);
//CohGetPreamp
typedef BOOL (__stdcall *COH_G35DDC_GET_PREAMP)(INT32 hDeviceSet,BOOL *Preamp);
//CohSetInverted
typedef BOOL (__stdcall *COH_G35DDC_SET_INVERTED)(INT32 hDeviceSet,BOOL Inverted);
//CohGetInverted
typedef BOOL (__stdcall *COH_G35DDC_GET_INVERTED)(INT32 hDeviceSet,BOOL *Inverted);
//CohSetADCNoiseBlanker
typedef BOOL (__stdcall *COH_G35DDC_SET_ADC_NOISE_BLANKER)(INT32 hDeviceSet,BOOL Enabled);
//CohGetADCNoiseBlanker
typedef BOOL (__stdcall *COH_G35DDC_GET_ADC_NOISE_BLANKER)(INT32 hDeviceSet,BOOL *Enabled);
//CohSetADCNoiseBlankerThreshold
typedef BOOL (__stdcall *COH_G35DDC_SET_ADC_NOISE_BLANKER_THRESHOLD)(INT32 hDeviceSet,WORD Threshold);
//CohGetADCNoiseBlankerThreshold
typedef BOOL (__stdcall *COH_G35DDC_GET_ADC_NOISE_BLANKER_THRESHOLD)(INT32 hDeviceSet,WORD *Threshold);
//CohStartIF
typedef BOOL (__stdcall *COH_G35DDC_START_IF)(INT32 hDeviceSet,UINT32 DeviceIndex,WORD Period);
//CohStopIF
typedef BOOL (__stdcall *COH_G35DDC_STOP_IF)(INT32 hDeviceSet,UINT32 DeviceIndex);
//CohGetDDCInfo
typedef BOOL (__stdcall *COH_G35DDC_GET_DDC_INFO)(INT32 hDeviceSet,UINT32 DDCTypeIndex,G3XDDC_DDC_INFO *Info);
//CohGetDDC1Count
typedef BOOL (__stdcall *COH_G35DDC_GET_DDC1_COUNT)(INT32 hDeviceSet,UINT32 *Count);
//CohSetDDC1
typedef BOOL (__stdcall *COH_G35DDC_SET_DDC1)(INT32 hDeviceSet,UINT32 DDCTypeIndex);
//CohGetDDC1
typedef BOOL (__stdcall *COH_G35DDC_GET_DDC1)(INT32 hDeviceSet,UINT32 *DDCTypeIndex,G3XDDC_DDC_INFO *DDCInfo);
//CohStartDDC1
typedef BOOL (__stdcall *COH_G35DDC_START_DDC1)(INT32 hDeviceSet,UINT32 SamplesPerBuffer);
//CohStopDDC1
typedef BOOL (__stdcall *COH_G35DDC_STOP_DDC1)(INT32 hDeviceSet);
//CohSetDDC1Frequency
typedef BOOL (__stdcall *COH_G35DDC_SET_DDC1_FREQUENCY)(INT32 hDeviceSet,UINT32 Frequency);
//CohGetDDC1Frequency
typedef BOOL (__stdcall *COH_G35DDC_GET_DDC1_FREQUENCY)(INT32 hDeviceSet,UINT32 *Frequency);
//CohSetDDC1PhaseShift
typedef BOOL (__stdcall *COH_G35DDC_SET_DDC1_PHASE_SHIFT)(INT32 hDeviceSet,UINT32 DeviceIndex,double PhaseShift);
//CohGetDDC1PhaseShift
typedef BOOL (__stdcall *COH_G35DDC_GET_DDC1_PHASE_SHIFT)(INT32 hDeviceSet,UINT32 DeviceIndex,double *PhaseShift);
//CohGetDDC2
typedef BOOL (__stdcall *COH_G35DDC_GET_DDC2)(INT32 hDeviceSet,UINT32 *DDCTypeIndex,G3XDDC_DDC_INFO *DDCInfo);
//CohStartDDC2
typedef BOOL (__stdcall *COH_G35DDC_START_DDC2)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 SamplesPerBuffer);
//CohStopDDC2
typedef BOOL (__stdcall *COH_G35DDC_STOP_DDC2)(INT32 hDeviceSet,UINT32 DeviceIndex);
//CohSetDDC2Frequency
typedef BOOL (__stdcall *COH_G35DDC_SET_DDC2_FREQUENCY)(INT32 hDeviceSet,UINT32 DeviceIndex,INT32 Frequency);
//CohGetDDC2Frequency
typedef BOOL (__stdcall *COH_G35DDC_GET_DDC2_FREQUENCY)(INT32 hDeviceSet,UINT32 DeviceIndex,INT32 *Frequency);
//CohSetDDC2NoiseBlanker
typedef BOOL (__stdcall *COH_G35DDC_SET_DDC2_NOISE_BLANKER)(INT32 hDeviceSet,UINT32 DeviceIndex,BOOL Enabled);
//CohGetDDC2NoiseBlanker
typedef BOOL (__stdcall *COH_G35DDC_GET_DDC2_NOISE_BLANKER)(INT32 hDeviceSet,UINT32 DeviceIndex,BOOL *Enabled);
//CohSetDDC2NoiseBlankerThreshold
typedef BOOL (__stdcall *COH_G35DDC_SET_DDC2_NOISE_BLANKER_THRESHOLD)(INT32 hDeviceSet,UINT32 DeviceIndex,double Threshold);
//CohGetDDC2NoiseBlankerThreshold
typedef BOOL (__stdcall *COH_G35DDC_GET_DDC2_NOISE_BLANKER_THRESHOLD)(INT32 hDeviceSet,UINT32 DeviceIndex,double *Threshold);
//CohGetDDC2NoiseBlankerExcessValue
typedef BOOL (__stdcall *COH_G35DDC_GET_DDC2_NOISE_BLANKER_EXCESS_VALUE)(INT32 hDeviceSet,UINT32 DeviceIndex,double *Value);
//CohGetSignalLevel
typedef BOOL (__stdcall *COH_G35DDC_GET_SIGNAL_LEVEL)(INT32 hDeviceSet,UINT32 DeviceIndex,FLOAT *Peak,FLOAT *RMS);
//CohSetAGC
typedef BOOL (__stdcall *COH_G35DDC_SET_AGC)(INT32 hDeviceSet,UINT32 DeviceIndex,BOOL Enabled);
//CohGetAGC
typedef BOOL (__stdcall *COH_G35DDC_GET_AGC)(INT32 hDeviceSet,UINT32 DeviceIndex,BOOL *Enabled);
//CohSetAGCParams
typedef BOOL (__stdcall *COH_G35DDC_SET_AGC_PARAMS)(INT32 hDeviceSet,UINT32 DeviceIndex,double AttackTime,double DecayTime,double ReferenceLevel);
//CohGetAGCParams
typedef BOOL (__stdcall *COH_G35DDC_GET_AGC_PARAMS)(INT32 hDeviceSet,UINT32 DeviceIndex,double *AttackTime,double *DecayTime,double *ReferenceLevel);
//CohSetMaxAGCGain
typedef BOOL (__stdcall *COH_G35DDC_SET_MAX_AGC_GAIN)(INT32 hDeviceSet,UINT32 DeviceIndex,double MaxGain);
//CohGetMaxAGCGain
typedef BOOL (__stdcall *COH_G35DDC_GET_MAX_AGC_GAIN)(INT32 hDeviceSet,UINT32 DeviceIndex,double *MaxGain);
//CohSetGain
typedef BOOL (__stdcall *COH_G35DDC_SET_GAIN)(INT32 hDeviceSet,UINT32 DeviceIndex,double Gain);
//CohGetGain
typedef BOOL (__stdcall *COH_G35DDC_GET_GAIN)(INT32 hDeviceSet,UINT32 DeviceIndex,double *Gain);
//CohGetCurrentGain
typedef BOOL (__stdcall *COH_G35DDC_GET_CURRENT_GAIN)(INT32 hDeviceSet,UINT32 DeviceIndex,double *Gain);
//CohSetNotchFilter
typedef BOOL (__stdcall *COH_G35DDC_SET_NOTCH_FILTER)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 NotchFilterIndex,BOOL Enabled);
//CohGetNotchFilter
typedef BOOL (__stdcall *COH_G35DDC_GET_NOTCH_FILTER)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 NotchFilterIndex,BOOL *Enabled);
//CohSetNotchFilterFrequency
typedef BOOL (__stdcall *COH_G35DDC_SET_NOTCH_FILTER_FREQUENCY)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 NotchFilterIndex,INT32 Frequency);
//CohGetNotchFilterFrequency
typedef BOOL (__stdcall *COH_G35DDC_GET_NOTCH_FILTER_FREQUENCY)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 NotchFilterIndex,INT32 *Frequency);
//CohSetNotchFilterBandwidth
typedef BOOL (__stdcall *COH_G35DDC_SET_NOTCH_FILTER_BANDWIDTH)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 NotchFilterIndex,UINT32 Bandwidth);
//CohGetNotchFilterBandwidth
typedef BOOL (__stdcall *COH_G35DDC_GET_NOTCH_FILTER_BANDWIDTH)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 NotchFilterIndex,UINT32 *Bandwidth);
//CohSetNotchFilterLength
typedef BOOL (__stdcall *COH_G35DDC_SET_NOTCH_FILTER_LENGTH)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 NotchFilterIndex,UINT32 Length);
//CohGetNotchFilterLength
typedef BOOL (__stdcall *COH_G35DDC_GET_NOTCH_FILTER_LENGTH)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 NotchFilterIndex,UINT32 *Length);
//CohSetDemodulatorMode
typedef BOOL (__stdcall *COH_G35DDC_SET_DEMODULATOR_MODE)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 Mode);
//CohGetDemodulatorMode
typedef BOOL (__stdcall *COH_G35DDC_GET_DEMODULATOR_MODE)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 *Mode);
//CohSetDemodulatorFrequency
typedef BOOL (__stdcall *COH_G35DDC_SET_DEMODULATOR_FREQUENCY)(INT32 hDeviceSet,UINT32 DeviceIndex,INT32 Frequency);
//CohGetDemodulatorFrequency
typedef BOOL (__stdcall *COH_G35DDC_GET_DEMODULATOR_FREQUENCY)(INT32 hDeviceSet,UINT32 DeviceIndex,INT32 *Frequency);
//CohSetDemodulatorFilterBandwidth
typedef BOOL (__stdcall *COH_G35DDC_SET_DEMODULATOR_FILTER_BANDWIDTH)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 Bandwidth);
//CohGetDemodulatorFilterBandwidth
typedef BOOL (__stdcall *COH_G35DDC_GET_DEMODULATOR_FILTER_BANDWIDTH)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 *Bandwidth);
//CohSetDemodulatorFilterShift
typedef BOOL (__stdcall *COH_G35DDC_SET_DEMODULATOR_FILTER_SHIFT)(INT32 hDeviceSet,UINT32 DeviceIndex,INT32 Shift);
//CohGetDemodulatorFilterShift
typedef BOOL (__stdcall *COH_G35DDC_GET_DEMODULATOR_FILTER_SHIFT)(INT32 hDeviceSet,UINT32 DeviceIndex,INT32 *Shift);
//CohSetDemodulatorFilterLength
typedef BOOL (__stdcall *COH_G35DDC_SET_DEMODULATOR_FILTER_LENGTH)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 Length);
//CohGetDemodulatorFilterLength
typedef BOOL (__stdcall *COH_G35DDC_GET_DEMODULATOR_FILTER_LENGTH)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 *Length);
//CohSetDemodulatorParam
typedef BOOL (__stdcall *COH_G35DDC_SET_DEMODULATOR_PARAM)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 Code,CONST VOID *Buffer,UINT32 BufferSize);
//CohGetDemodulatorParam
typedef BOOL (__stdcall *COH_G35DDC_GET_DEMODULATOR_PARAM)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 Code,VOID *Buffer,UINT32 BufferSize);
//CohGetDemodulatorState
typedef BOOL (__stdcall *COH_G35DDC_GET_DEMODULATOR_STATE)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 Code,VOID *Buffer,UINT32 BufferSize);
//CohStartAudio
typedef BOOL (__stdcall *COH_G35DDC_START_AUDIO)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 SamplesPerBuffer);
//CohStopAudio
typedef BOOL (__stdcall *COH_G35DDC_STOP_AUDIO)(INT32 hDeviceSet,UINT32 DeviceIndex);
//CohSetAudioFilter
typedef BOOL (__stdcall *COH_G35DDC_SET_AUDIO_FILTER)(INT32 hDeviceSet,UINT32 DeviceIndex,BOOL Enabled);
//CohGetAudioFilter
typedef BOOL (__stdcall *COH_G35DDC_GET_AUDIO_FILTER)(INT32 hDeviceSet,UINT32 DeviceIndex,BOOL *Enabled);
//CohSetAudioFilterParams
typedef BOOL (__stdcall *COH_G35DDC_SET_AUDIO_FILTER_PARAMS)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 CutOffLow,UINT32 CutOffHigh,double Deemphasis);
//CohGetAudioFilterParams
typedef BOOL (__stdcall *COH_G35DDC_GET_AUDIO_FILTER_PARAMS)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 *CutOffLow,UINT32 *CutOffHigh,double *Deemphasis);
//CohSetAudioFilterLength
typedef BOOL (__stdcall *COH_G35DDC_SET_AUDIO_FILTER_LENGTH)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 Length);
//CohGetAudioFilterLength
typedef BOOL (__stdcall *COH_G35DDC_GET_AUDIO_FILTER_LENGTH)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 *Length);
//CohSetAudioGain
typedef BOOL (__stdcall *COH_G35DDC_SET_AUDIO_GAIN)(INT32 hDeviceSet,UINT32 DeviceIndex,double Gain);
//CohGetAudioGain
typedef BOOL (__stdcall *COH_G35DDC_GET_AUDIO_GAIN)(INT32 hDeviceSet,UINT32 DeviceIndex,double *Gain);
//CohSetVolume
typedef BOOL (__stdcall *COH_G35DDC_SET_VOLUME)(INT32 hDeviceSet,UINT32 DeviceIndex,BYTE Volume);
//CohGetVolume
typedef BOOL (__stdcall *COH_G35DDC_GET_VOLUME)(INT32 hDeviceSet,UINT32 DeviceIndex,BYTE *Volume);
//CohSetMute
typedef BOOL (__stdcall *COH_G35DDC_SET_MUTE)(INT32 hDeviceSet,UINT32 DeviceIndex,BOOL Mute);
//CohGetMute
typedef BOOL (__stdcall *COH_G35DDC_GET_MUTE)(INT32 hDeviceSet,UINT32 DeviceIndex,BOOL *Mute);
//CohSetDRMKey
typedef BOOL (__stdcall *COH_G35DDC_SET_DRM_KEY)(INT32 hDeviceSet,CONST CHAR *DRMKeyFileDirectory);
//CohIsDRMUnlocked
typedef BOOL (__stdcall *COH_G35DDC_IS_DRM_UNLOCKED)(INT32 hDeviceSet);
//CohGetSpectrumCompensation
typedef BOOL (__stdcall *COH_G35DDC_GET_SPECTRUM_COMPENSATION)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 CenterFrequency,UINT32 Width,FLOAT *Buffer,UINT32 Count);
//CohGetTemperature
typedef BOOL (__stdcall *COH_G35DDC_GET_TEMPERATURE)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 *Temperature);
//CohGetDeviceState
typedef BOOL (__stdcall *COH_G35DDC_GET_DEVICE_STATE)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 *State);
//CohSetDAC
typedef BOOL (__stdcall *COH_G35DDC_SET_DAC)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 DAC);
//CohGetDAC
typedef BOOL (__stdcall *COH_G35DDC_GET_DAC)(INT32 hDeviceSet,UINT32 DeviceIndex,UINT32 *DAC);


#endif


