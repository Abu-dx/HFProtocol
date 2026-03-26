
#include "StdAfx.h"

short   pData[4][4*MAX_SEGMENT_SIZE/2];
short   gl_wChannels;
long	gl_dwSamplesPerSec;
short   gl_wBitsPerSample;
long	gl_dwDataLen;

OutList gl_OutList;
BOOL    gl_OutDisplaying;

short   pDataFiledemode[2][2*MAX_SEGMENT_SIZE];// 文件解调星座结果  

BOOL	   selprotolName[10];
int	   selprotolNum;
BOOL  gl_bRunDemode;
BOOL  gl_bRunDetect;