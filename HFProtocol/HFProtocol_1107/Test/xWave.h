// xWave.h: interface for the CxWave class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XWAVE_H__CBA560A3_8D29_43DD_8608_329735D01984__INCLUDED_)
#define AFX_XWAVE_H__CBA560A3_8D29_43DD_8608_329735D01984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#pragma comment(lib,"Winmm.lib")
#endif // _MSC_VER > 1000

#include <mmsystem.h>
#include <mmreg.h>


class CWaveBuffer  
{
public:
	CWaveBuffer();
	virtual ~CWaveBuffer();

	int GetSampleSize() const;
	void SetBuffer(void* pBuffer, DWORD dwNumSamples, int nSize);
	void SetNumSamples(DWORD dwNumSamples, int nSize = sizeof(short));
	void CopyBuffer(void* pBuffer, DWORD dwNumSamples, int nSize = sizeof(short));
	DWORD GetNumSamples() const;
	void* GetBuffer() const;
	
private:
	int m_nSampleSize;
	void* m_pBuffer;
	DWORD m_dwNum;
};


//////////////////////////////////////////////////////////////////////
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

//////////////////////////////////////////////////////////////////////
#ifdef WAVE_OUT_BUFFER_SIZE
#undef WAVE_OUT_BUFFER_SIZE
#endif
#define WAVEOUT_BUFFER_SIZE 4096

#define NUMWAVEOUTHDR 3
#define INFINITE_LOOP INT_MAX

//////////////////////////////////////////////////////////////////////
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

//////////////////////////////////////////////////////////////////////
#ifdef WAVE_IN_BUFFER_SIZE
#undef WAVE_IN_BUFFER_SIZE
#endif
#define WAVEIN_BUFFER_SIZE 4096

#define NUMWAVEINHDR 2
typedef struct
{
	char 	FileChunkID[4];
	long 	dwFileChunkLen;
	char 	RiffType[4];
	char 	FmtChunkID[4];
	long	dwFmtChunkLen;
	short	wCompression;
	short 	wChannels;
	long	dwSamplesPerSec;
	long 	dwAvgBytesPerSec;
	short 	wBlockAlign;
	short 	wBitsPerSample;
	char 	DataChunkID[4];
	long	dwDataLen;
}WavHdr;

class CxWave  
{
public:
	CxWave();
	CxWave(const CxWave& copy);
	virtual ~CxWave();

	__int64 AllTime();
	int GetAve();
public:
	void SetBuffer(void* pBuffer, DWORD dwNumSamples, bool bCopy = false);
	void Load(const CString& strFile);
	void Save(const CString& strFile);
	DWORD GetBufferLength() const;
	DWORD GetNumSamples() const;
	void* GetBuffer() const;
	void Save(CFile* f);
	void Load(CFile* f);
	WAVEFORMATEX GetFormat() const;
	void BuildFormat(WORD nChannels, DWORD nFrequency, WORD nBits);
	void SetWaveHad(DWORD nChannels, DWORD nFrequency, DWORD nBits,DWORD nSampleNum);
	WAVEFORMATEX m_pcmWaveFormat;
	WavHdr m_wavHad;
private:
	CWaveBuffer m_buffer;
	

	DWORD dwNum;
	bool playflag;
public:
	UINT GetDevice() const;
	bool IsOutputFormat();
	bool IsInputFormat();	
private:
	UINT m_nDevice;

//////////////////////////*Waveout*//////////////////////////////
	friend void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
public:
	CString GetError() const;
	DWORD GetPosition();
	bool IsPlaying();
	
	bool Close();
	bool Continue();
	bool FullPlay(int nLoop = -1, DWORD dwStart = -1, DWORD dwEnd = -1);
	bool Open();
	bool Pause();
	bool Play(DWORD dwStart = -1, DWORD dwEnd = -1);
	bool Stop();
	
	void ModifyWaveOutBufferLength(DWORD dwLength);

private:
	bool AddFullHeader(HWAVEOUT hwo, int nLoop);
	bool AddNewHeader(HWAVEOUT hwo);
	DWORD GetOutBufferLength();
	bool IsError(MMRESULT nResult);
	bool ResetRequired(CxWave* pWaveOut);
private:
	
	DWORD m_dwEndPos;
	DWORD m_dwStartPos;
	DWORD m_dwWaveOutBufferLength;
	HWAVEOUT m_hWaveOut;
	UINT m_nError;
	int m_nIndexWaveHdr;
	WAVEHDR m_tagWaveHdr[NUMWAVEOUTHDR];


//////////////////////////*WaveIn*//////////////////////////////
	friend void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
public:
	CString GetInError() const;
	DWORD GetInPosition();
	bool IsRecording();
	void MakeWave();
	
	bool InClose();
	bool InContinue();
	bool InOpen();
	bool InPause();
	bool Record(UINT nTaille = 4096);
	bool InStop();
	void SetWaveFormat(WAVEFORMATEX tagFormat);

private:
	bool AddNewBuffer(WAVEHDR* pWaveHdr);
	bool AddNewHeader(HWAVEIN hwi);
	void FreeListOfBuffer();
	DWORD GetNumSamples();
	void FreeListOfHeader();
	void InitListOfHeader();
	bool IsInError(MMRESULT nResult);
	bool ResetInRequired(CxWave* pWaveIn);
private:
	bool m_bResetRequired;
	HWAVEIN	m_hWaveIn;
	CPtrList m_listOfBuffer;
	UINT m_nBufferSize;
	int m_nIndexWaveInHdr;
	WAVEHDR m_tagWaveInHdr[NUMWAVEINHDR];

};

#endif // !defined(AFX_XWAVE_H__CBA560A3_8D29_43DD_8608_329735D01984__INCLUDED_)
