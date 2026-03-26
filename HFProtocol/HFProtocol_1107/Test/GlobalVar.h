
extern CHFProtocol theApp;

extern short   pData[4][4*MAX_SEGMENT_SIZE/2];
extern short   gl_wChannels;
extern long	   gl_dwSamplesPerSec;
extern short   gl_wBitsPerSample;
extern long	   gl_dwDataLen;

extern OutList gl_OutList;
extern BOOL    gl_OutDisplaying;

extern short   pDataFiledemode[2][2*MAX_SEGMENT_SIZE];// 文件解调星座结果  
extern BOOL	   selprotolName[10];
extern int	   selprotolNum;
extern BOOL  gl_bRunDemode;
extern BOOL  gl_bRunDetect;
