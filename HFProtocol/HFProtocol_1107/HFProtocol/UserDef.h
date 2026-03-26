
#define MAX_SEGMENT_SIZE   8192 


struct DdemodePara
{
	int Channelnum;
	bool ChannelUse[4];
	bool ChannelBegin[4];
	double fc[4];
	double fs[4];
	int stytle[4];//调制方式  0:ANDVT;1:CLEW
	double frequency[4];
};
struct OutList
{		
	CString sigType;
	CString BeginTime;
	CString EndTime;
	double frequency;
	double hfFre;
	int dataRate;
	int interLeng;
	CString interType;
	int PU;
	CString frameType;
	int CRC;
	CString encrypt;
	CString message;
};

