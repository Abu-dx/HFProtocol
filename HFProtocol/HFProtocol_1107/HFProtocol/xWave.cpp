// xWave.cpp: implementation of the CxWave class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xWave.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CxWave::CxWave()
{
	ZeroMemory((void*)&m_pcmWaveFormat, sizeof(m_pcmWaveFormat));
	m_pcmWaveFormat.wFormatTag = 1;

	m_nDevice = WAVE_MAPPER ;

	m_hWaveIn=0;
	m_nIndexWaveInHdr=NUMWAVEINHDR - 1;
	m_bResetRequired=true;
	InitListOfHeader();

	m_hWaveOut=0;
	m_nIndexWaveHdr=NUMWAVEOUTHDR - 1;
	m_dwStartPos=0L;
	m_dwWaveOutBufferLength=WAVEOUT_BUFFER_SIZE;

	playflag = FALSE;
}

CxWave::CxWave(const CxWave& copy)
{
	m_pcmWaveFormat = copy.GetFormat();
	m_buffer.SetNumSamples( copy.GetNumSamples(), copy.GetFormat().nBlockAlign ) ;
	m_buffer.CopyBuffer( copy.GetBuffer(), copy.GetNumSamples(), copy.GetFormat().nBlockAlign );
}
CxWave::~CxWave()
{
	InClose();
	FreeListOfBuffer();
	FreeListOfHeader();
	
	Close();
}

//////////////////////////////////////////////////////////////////////
void CxWave::BuildFormat(WORD nChannels, DWORD nFrequency, WORD nBits)
{
	m_pcmWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_pcmWaveFormat.nChannels = nChannels;
	m_pcmWaveFormat.nSamplesPerSec = nFrequency;
	m_pcmWaveFormat.nAvgBytesPerSec = nFrequency * nChannels * nBits / 8;
	m_pcmWaveFormat.nBlockAlign = nChannels * nBits / 8;
	m_pcmWaveFormat.wBitsPerSample = nBits;
	m_buffer.SetNumSamples(0L, m_pcmWaveFormat.nBlockAlign);

}	
void CxWave::SetWaveHad(DWORD nChannels, DWORD nFrequency, DWORD nBits,DWORD nSampleNum)
{
	memcpy(m_wavHad.FileChunkID, "RIFF", 4);
	m_wavHad.dwFileChunkLen = 36 + nSampleNum * nChannels * nBits/8;
	memcpy(m_wavHad.RiffType, "WAVE", 4);
	memcpy(m_wavHad.FmtChunkID, "fmt ", 4);
	m_wavHad.dwFmtChunkLen = 16L;
	m_wavHad.wCompression = WAVE_FORMAT_PCM;  // 1 for PCM
	m_wavHad.wChannels = nChannels;
	m_wavHad.dwSamplesPerSec = nFrequency;
	m_wavHad.dwAvgBytesPerSec = nFrequency * nBits * nChannels / 8;
	m_wavHad.wBitsPerSample = nBits;
	m_wavHad.wBlockAlign = nBits * nChannels / 8 ;
	memcpy(m_wavHad.DataChunkID, "data", 4);
	m_wavHad.dwDataLen = nSampleNum * nChannels * nBits/8;
}
//////////////////////////////////////////////////////////////////////
inline WAVEFORMATEX CxWave::GetFormat() const
{
	return m_pcmWaveFormat;
}


//////////////////////////////////////////////////////////////////////
void CxWave::Load(const CString &strFile)
{
	CFile f(strFile, CFile::modeRead);
	Load(&f);
	f.Close();
}

//////////////////////////////////////////////////////////////////////
void CxWave::Load(CFile *f)
{
	char szTmp[10];
	WAVEFORMATEX pcmWaveFormat;
	ZeroMemory(szTmp, 10 * sizeof(char));
	f->Read(szTmp, 4 * sizeof(char)) ;
	if (strncmp(szTmp, _T("RIFF"), 4) != 0) 
		::AfxThrowFileException(CFileException::invalidFile, -1, f->GetFileName());
	DWORD dwFileSize/* = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign + 36*/ ;
	f->Read(&dwFileSize, sizeof(dwFileSize)) ;
	ZeroMemory(szTmp, 10 * sizeof(char));
	f->Read(szTmp, 8 * sizeof(char)) ;
	if (strncmp(szTmp, _T("WAVEfmt "), 8) != 0) 
		::AfxThrowFileException(CFileException::invalidFile, -1, f->GetFileName());
	DWORD dwFmtSize /*= 16L*/;
	f->Read(&dwFmtSize, sizeof(dwFmtSize)) ;
	f->Read(&pcmWaveFormat.wFormatTag, sizeof(pcmWaveFormat.wFormatTag)) ;
	f->Read(&pcmWaveFormat.nChannels, sizeof(pcmWaveFormat.nChannels)) ;
	f->Read(&pcmWaveFormat.nSamplesPerSec, sizeof(pcmWaveFormat.nSamplesPerSec)) ;
	f->Read(&pcmWaveFormat.nAvgBytesPerSec, sizeof(pcmWaveFormat.nAvgBytesPerSec)) ;
	f->Read(&pcmWaveFormat.nBlockAlign, sizeof(pcmWaveFormat.nBlockAlign)) ;
	f->Read(&pcmWaveFormat.wBitsPerSample, sizeof(pcmWaveFormat.wBitsPerSample)) ;
	ZeroMemory(szTmp, 10 * sizeof(char));
	f->Read(szTmp, 4 * sizeof(char)) ;
	if (strncmp(szTmp, _T("data"), 4) != 0) 
		::AfxThrowFileException(CFileException::invalidFile, -1, f->GetFileName());
	m_pcmWaveFormat = pcmWaveFormat;

	f->Read(&dwNum, sizeof(dwNum)) ;
	m_buffer.SetNumSamples(dwNum / pcmWaveFormat.nBlockAlign, pcmWaveFormat.nBlockAlign);
	f->Read(m_buffer.GetBuffer(), dwNum) ;
}

__int64 CxWave::AllTime()
{
	return dwNum*1000/m_pcmWaveFormat.nAvgBytesPerSec;
}
int CxWave::GetAve()
{
	return	m_pcmWaveFormat.nAvgBytesPerSec;
}
//////////////////////////////////////////////////////////////////////
void CxWave::Save(const CString &strFile)
{
	CFile f(strFile, CFile::modeCreate | CFile::modeWrite);
	Save(&f);
	f.Close();
	AfxMessageBox("文件已保存！");
}

//////////////////////////////////////////////////////////////////////
void CxWave::Save(CFile *f)
{
	ASSERT( m_buffer.GetNumSamples() > 0 );

	f->Write("RIFF", 4) ;
	DWORD dwFileSize = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign + 36 ;
	f->Write(&dwFileSize, sizeof(dwFileSize)) ;
	f->Write("WAVEfmt ", 8) ;
	DWORD dwFmtSize = 16L;
	f->Write(&dwFmtSize, sizeof(dwFmtSize)) ;
	f->Write(&m_pcmWaveFormat.wFormatTag, sizeof(m_pcmWaveFormat.wFormatTag)) ;
	f->Write(&m_pcmWaveFormat.nChannels, sizeof(m_pcmWaveFormat.nChannels)) ;
	f->Write(&m_pcmWaveFormat.nSamplesPerSec, sizeof(m_pcmWaveFormat.nSamplesPerSec)) ;
	f->Write(&m_pcmWaveFormat.nAvgBytesPerSec, sizeof(m_pcmWaveFormat.nAvgBytesPerSec)) ;
	f->Write(&m_pcmWaveFormat.nBlockAlign, sizeof(m_pcmWaveFormat.nBlockAlign)) ;
	f->Write(&m_pcmWaveFormat.wBitsPerSample, sizeof(m_pcmWaveFormat.wBitsPerSample)) ;
	f->Write("data", 4) ;
	DWORD dwNum = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign;
	f->Write(&dwNum, sizeof(dwNum)) ;
	f->Write(m_buffer.GetBuffer(), dwNum) ;
}

//////////////////////////////////////////////////////////////////////
void* CxWave::GetBuffer() const
{
	return m_buffer.GetBuffer();
}

//////////////////////////////////////////////////////////////////////
DWORD CxWave::GetNumSamples() const
{
	return m_buffer.GetNumSamples();
}

//////////////////////////////////////////////////////////////////////
DWORD CxWave::GetBufferLength() const
{
	return ( m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign );
}

//////////////////////////////////////////////////////////////////////
void CxWave::SetBuffer(void* pBuffer, DWORD dwNumSample, bool bCopy)
{
	ASSERT(pBuffer);
	ASSERT(dwNumSample > 0);
	ASSERT(m_pcmWaveFormat.nBlockAlign > 0);

	if (bCopy) {
		m_buffer.CopyBuffer(pBuffer, dwNumSample, m_pcmWaveFormat.nBlockAlign);
	}
	else {
		m_buffer.SetBuffer(pBuffer, dwNumSample, m_pcmWaveFormat.nBlockAlign);
	}
}


//////////////////////////////////////////////////////////////////////
bool CxWave::IsInputFormat()
{
	return (waveInOpen(
		NULL,
		GetDevice(),
		&GetFormat(),
		NULL,
		NULL,
		WAVE_FORMAT_QUERY) == MMSYSERR_NOERROR);
}

//////////////////////////////////////////////////////////////////////
bool CxWave::IsOutputFormat()
{
	
	return (waveOutOpen(
		NULL,
		GetDevice(),
		&GetFormat(),
		NULL,
		NULL,
		WAVE_FORMAT_QUERY) == MMSYSERR_NOERROR);
}

//////////////////////////////////////////////////////////////////////
inline UINT CxWave::GetDevice() const
{
	return m_nDevice;
}




//////////////////////////////////////////////////////////////////////
// CWaveOut Construction/Destruction
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	
	switch(uMsg) {
	case MM_WOM_DONE:
		WAVEHDR* pWaveHdr = ( (WAVEHDR*)dwParam1 );
		CxWave* pWaveOut = (CxWave*)(pWaveHdr->dwUser);
		
		if (pWaveHdr && hwo && pWaveOut) {
			if (pWaveHdr->dwFlags & WHDR_DONE == WHDR_DONE) {
				pWaveHdr->dwFlags = 0;
				if ( pWaveOut->IsError(waveOutUnprepareHeader(hwo, pWaveHdr, sizeof(WAVEHDR))) ) {
					break;
				}
				pWaveHdr->lpData = NULL;
			}
			if ( ! pWaveOut->ResetRequired(pWaveOut) ) {
				if ( !pWaveOut->AddNewHeader(hwo) ) {
					break;
				}
			}
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////
bool CxWave::Close()
{
	if (m_hWaveOut != NULL) {
		if ( !Stop() ) {
			return false;
		}
		if ( IsError( waveOutClose(m_hWaveOut)) ) {
			return false;
		}
		m_hWaveOut = 0;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::Continue()
{
	if (m_hWaveOut) {
		return !IsError( waveOutRestart(m_hWaveOut) );
	}
	playflag = TRUE;
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::FullPlay(int nLoop, DWORD dwStart/*=-1*/, DWORD dwEnd/*=-1*/)
{
	DWORD oldBufferLength = m_dwWaveOutBufferLength;
	m_dwWaveOutBufferLength = GetBufferLength();

	if ( IsError(waveOutReset(m_hWaveOut)) ) {
		return false;
	}
	m_dwStartPos = (dwStart == -1) ? 0L : dwStart * GetFormat().nBlockAlign;
	m_dwEndPos = (dwEnd == -1) ? GetBufferLength() : __min( GetBufferLength(), dwEnd ) * GetFormat().nBlockAlign;
	nLoop = (nLoop == -1) ? 0 : nLoop;
	m_nIndexWaveHdr = NUMWAVEOUTHDR - 1;

	if ( !AddFullHeader(m_hWaveOut, nLoop) ) {
		m_dwWaveOutBufferLength = oldBufferLength;
		return false;
	}

	m_dwWaveOutBufferLength = oldBufferLength;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::Open()
{
	return !IsError( waveOutOpen(&m_hWaveOut, GetDevice(), &GetFormat(), (DWORD)waveOutProc, NULL, CALLBACK_FUNCTION) );
}

//////////////////////////////////////////////////////////////////////
bool CxWave::Pause()
{
	if (m_hWaveOut) {
		return !IsError( waveOutPause(m_hWaveOut) );
	}
	playflag = FALSE;
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::Play(DWORD dwStart/*=-1*/, DWORD dwEnd/*=-1*/)
{
	if ( !Stop() ) {
		return false;
	}
	m_dwStartPos = (dwStart == -1) ? 0L : dwStart * GetFormat().nBlockAlign;
	m_dwEndPos = (dwEnd == -1) ? GetBufferLength() : __min( GetBufferLength(), dwEnd ) * GetFormat().nBlockAlign;
	m_nIndexWaveHdr = NUMWAVEOUTHDR - 1;
	for (int i = 0; i < NUMWAVEOUTHDR; i++) {
		if ( !AddNewHeader(m_hWaveOut) ) {
			return false;
		}
	}
	playflag = TRUE;
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::Stop()
{
	if (m_hWaveOut != NULL) {
		m_dwStartPos = m_dwEndPos;
		if ( IsError(waveOutReset(m_hWaveOut)) ) {
			return false;
		}
	}
	playflag = FALSE;
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::AddFullHeader(HWAVEOUT hwo, int nLoop)
{
	if ( GetOutBufferLength() == 0) {
		return false;
	}
	m_nIndexWaveHdr = (m_nIndexWaveHdr == NUMWAVEOUTHDR - 1) ? 0 : m_nIndexWaveHdr + 1;
	m_tagWaveHdr[m_nIndexWaveHdr].lpData = (char*)GetBuffer() + m_dwStartPos;
	m_tagWaveHdr[m_nIndexWaveHdr].dwBufferLength = m_dwEndPos - m_dwStartPos;
	m_tagWaveHdr[m_nIndexWaveHdr].dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
	m_tagWaveHdr[m_nIndexWaveHdr].dwLoops = nLoop;
	m_tagWaveHdr[m_nIndexWaveHdr].dwUser = (DWORD)(void*)this;
	if ( IsError(waveOutPrepareHeader(hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) {
		return false;
	}
	if ( IsError(waveOutWrite(hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) {
		waveOutUnprepareHeader( hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR) );
		m_tagWaveHdr[m_nIndexWaveHdr].lpData = NULL;
		m_tagWaveHdr[m_nIndexWaveHdr].dwBufferLength = 0;
		m_tagWaveHdr[m_nIndexWaveHdr].dwFlags = 0;
		m_tagWaveHdr[m_nIndexWaveHdr].dwUser = NULL;
		m_nIndexWaveHdr--;
		return false;
	}
	m_dwStartPos = m_dwEndPos - m_dwStartPos;
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::AddNewHeader(HWAVEOUT hwo)
{
	if ( GetOutBufferLength() == 0) {
		return false;
	}
	m_nIndexWaveHdr = (m_nIndexWaveHdr == NUMWAVEOUTHDR - 1) ? 0 : m_nIndexWaveHdr + 1;
	m_tagWaveHdr[m_nIndexWaveHdr].lpData = (char*)GetBuffer() + m_dwStartPos;
	m_tagWaveHdr[m_nIndexWaveHdr].dwBufferLength = GetOutBufferLength();
	m_tagWaveHdr[m_nIndexWaveHdr].dwFlags = 0;
	m_tagWaveHdr[m_nIndexWaveHdr].dwUser = (DWORD)(void*)this;
	if ( IsError(waveOutPrepareHeader(hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) {
		return false;
	}
	if ( IsError(waveOutWrite(hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) { 
		waveOutUnprepareHeader( hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR) );
		m_tagWaveHdr[m_nIndexWaveHdr].lpData = NULL;
		m_tagWaveHdr[m_nIndexWaveHdr].dwBufferLength = 0;
		m_tagWaveHdr[m_nIndexWaveHdr].dwFlags = 0;
		m_tagWaveHdr[m_nIndexWaveHdr].dwUser = NULL;
		m_nIndexWaveHdr--;
		return false;
	}
	m_dwStartPos += GetOutBufferLength();
	return true;
}

//////////////////////////////////////////////////////////////////////
// GET
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
DWORD CxWave::GetOutBufferLength()
{
	return __min( m_dwWaveOutBufferLength, m_dwEndPos - m_dwStartPos );
}

//////////////////////////////////////////////////////////////////////
CString CxWave::GetError() const
{
	if (m_nError != MMSYSERR_NOERROR) {
		TCHAR szText[MAXERRORLENGTH + 1];
		if ( waveOutGetErrorText(m_nError, szText, MAXERRORLENGTH) == MMSYSERR_NOERROR ) {
			return szText;
		}
	}
	return "";
}

//////////////////////////////////////////////////////////////////////
DWORD CxWave::GetPosition()
{
	if (m_hWaveOut) {
		MMTIME mmt;
		mmt.wType = TIME_SAMPLES;
		if ( IsError(waveOutGetPosition(m_hWaveOut, &mmt, sizeof(MMTIME))) ) {
			return -1;
		}
		else {
			return mmt.u.sample;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::IsError(MMRESULT nResult)
{
	m_nError = nResult;
	return (m_nError != MMSYSERR_NOERROR);
}

//////////////////////////////////////////////////////////////////////
bool CxWave::IsPlaying()
{
	bool bResult = false;
	if (m_nIndexWaveHdr > -1 && m_tagWaveHdr[m_nIndexWaveHdr].dwFlags != 0) {
		bResult |= !(m_tagWaveHdr[m_nIndexWaveHdr].dwFlags & WHDR_DONE == WHDR_DONE);
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::ResetRequired(CxWave* pWaveOut)
{
	return (pWaveOut->m_dwStartPos >= pWaveOut->m_dwEndPos);
}


//////////////////////////////////////////////////////////////////////
// CWaveBuffer Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CWaveBuffer::CWaveBuffer() : m_dwNum(0), m_pBuffer(NULL), m_nSampleSize(0)
{
}

//////////////////////////////////////////////////////////////////////
CWaveBuffer::~CWaveBuffer()
{
	m_dwNum = 0L;
	delete[] m_pBuffer;
	m_pBuffer = NULL;
}

//////////////////////////////////////////////////////////////////////
void* CWaveBuffer::GetBuffer() const
{
	return m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
DWORD CWaveBuffer::GetNumSamples() const
{
	return m_dwNum;
}

//////////////////////////////////////////////////////////////////////
void CWaveBuffer::CopyBuffer(void* pBuffer, DWORD dwNumSamples, int nSize)
{
	ASSERT(dwNumSamples >= 0);
	ASSERT(nSize);
	
	if (!m_pBuffer)
		SetNumSamples(dwNumSamples, nSize);
	
	if (__min(m_dwNum, dwNumSamples) * nSize > 0) {
		ZeroMemory(m_pBuffer, m_dwNum * m_nSampleSize);
		CopyMemory(m_pBuffer, pBuffer, __min(m_dwNum, dwNumSamples) * nSize);
	}
}

//////////////////////////////////////////////////////////////////////
void CWaveBuffer::SetNumSamples(DWORD dwNumSamples, int nSize)
{
	ASSERT(dwNumSamples >= 0);
	ASSERT(nSize > 0);
	
	void* pBuffer = NULL;
	
	pBuffer = new char[nSize * dwNumSamples];
	SetBuffer(pBuffer, dwNumSamples, nSize);
}

//////////////////////////////////////////////////////////////////////
void CWaveBuffer::SetBuffer(void *pBuffer, DWORD dwNumSamples, int nSize)
{
	ASSERT(dwNumSamples >= 0);
	ASSERT(nSize);
	
	delete[] m_pBuffer;
	m_pBuffer = pBuffer;
	m_dwNum = dwNumSamples;
	m_nSampleSize = nSize;
}

//////////////////////////////////////////////////////////////////////
int CWaveBuffer::GetSampleSize() const
{
	return m_nSampleSize;
}


//////////////////////////////////////////////////////////////////////
// CWaveIn Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch(uMsg) {
	case MM_WIM_DATA:
		WAVEHDR* pWaveHdr = ( (WAVEHDR*)dwParam1 );
		CxWave* pWaveIn = (CxWave*)(pWaveHdr->dwUser);
		
		if (pWaveHdr && hwi && pWaveIn) {
			if (pWaveHdr->dwFlags & WHDR_DONE == WHDR_DONE) {
				pWaveHdr->dwFlags = 0;
				if ( pWaveIn->IsInError(waveInUnprepareHeader(hwi, pWaveHdr, sizeof(WAVEHDR))) ) {
					break;
				}
				if (pWaveHdr->dwBytesRecorded > 0) {
					pWaveIn->AddNewBuffer(pWaveHdr);
				}
				delete[] pWaveHdr->lpData;
				pWaveHdr->lpData = NULL;
			}
			
			if ( !pWaveIn->ResetInRequired(pWaveIn) ) {
				if ( !pWaveIn->AddNewHeader(hwi) ) {
					break;
				}
			}
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////
void CxWave::SetWaveFormat(WAVEFORMATEX tagFormat)
{
	BuildFormat(tagFormat.nChannels, tagFormat.nSamplesPerSec, tagFormat.wBitsPerSample);
}

//////////////////////////////////////////////////////////////////////
void CxWave::InitListOfHeader()
{
	for (int i = 0; i < NUMWAVEINHDR; i++) {
		m_tagWaveInHdr[i].lpData = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Son
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
bool CxWave::InClose()
{
	if (m_hWaveIn != NULL) {
		if ( !InStop() ) {
			return false;
		}
		if ( IsInError( waveInClose(m_hWaveIn)) ) {
			return false;
		}
		m_hWaveIn = 0;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::InContinue()
{
	if (m_hWaveIn) {
		return !IsInError( waveInStart(m_hWaveIn) );
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::InOpen()
{
	return !IsInError( waveInOpen(&m_hWaveIn, GetDevice(), &GetFormat(), (DWORD)waveInProc, NULL, CALLBACK_FUNCTION) );
}

//////////////////////////////////////////////////////////////////////
bool CxWave::InPause()
{
	if (m_hWaveIn) {
		return !IsInError( waveInStop(m_hWaveIn) );
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::Record(UINT nTaille/* = 4096*/)
{
	ASSERT(nTaille > 0);
	ASSERT(m_hWaveIn);

	if ( !InStop() ) {
		return false;
	}
	m_bResetRequired = false;
	FreeListOfBuffer();
	FreeListOfHeader();
	SetWaveFormat( GetFormat() );
	m_nIndexWaveInHdr = NUMWAVEINHDR - 1;
	m_nBufferSize = nTaille;
	for (int i = 0; i < NUMWAVEINHDR; i++) {
		if ( !AddNewHeader(m_hWaveIn) ) {
			return false;
		}
	}
	if ( IsInError(waveInStart(m_hWaveIn)) ) {
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::InStop()
{
	if (m_hWaveIn != NULL) {
		m_bResetRequired = true;
		::Sleep(15);
        //MMRESULT error1 = waveInReset(m_hWaveIn);********出错了！

		if ( IsInError(waveInReset(m_hWaveIn)) ) {
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::AddNewBuffer(WAVEHDR *pWaveHdr)
{
	ASSERT(pWaveHdr);

	m_listOfBuffer.AddTail(new CWaveBuffer);
	( (CWaveBuffer*)m_listOfBuffer.GetTail() )->CopyBuffer( pWaveHdr->lpData, \
		pWaveHdr->dwBytesRecorded / GetFormat().nBlockAlign, \
		GetFormat().nBlockAlign );
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::AddNewHeader(HWAVEIN hwi)
{
	ASSERT(m_nBufferSize > 0);

	m_nIndexWaveInHdr = (m_nIndexWaveInHdr == NUMWAVEINHDR - 1) ? 0 : m_nIndexWaveInHdr + 1;
	if (m_tagWaveInHdr[m_nIndexWaveInHdr].lpData == NULL) {
		m_tagWaveInHdr[m_nIndexWaveInHdr].lpData = new char[m_nBufferSize];
	}
	ZeroMemory(m_tagWaveInHdr[m_nIndexWaveInHdr].lpData, m_nBufferSize);
	m_tagWaveInHdr[m_nIndexWaveInHdr].dwBufferLength = m_nBufferSize;
	m_tagWaveInHdr[m_nIndexWaveInHdr].dwFlags = 0;
	m_tagWaveInHdr[m_nIndexWaveInHdr].dwUser = (DWORD)(void*)this;
	if ( IsInError(waveInPrepareHeader(hwi, &m_tagWaveInHdr[m_nIndexWaveInHdr], sizeof(WAVEHDR))) ) {
		return false;
	}
	if ( IsInError(waveInAddBuffer(hwi, &m_tagWaveInHdr[m_nIndexWaveInHdr], sizeof(WAVEHDR))) ) {
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
void CxWave::FreeListOfHeader()
{
	for (int i = 0; i < NUMWAVEINHDR; i++) {
		delete[] m_tagWaveInHdr[i].lpData;
		m_tagWaveInHdr[i].lpData = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
void CxWave::FreeListOfBuffer()
{
	POSITION pos = m_listOfBuffer.GetHeadPosition();
	while (pos) {
		CWaveBuffer* pBuf = (CWaveBuffer*)m_listOfBuffer.GetNext(pos);
		if (pBuf) {
			delete pBuf;
			pBuf = NULL;
		}
	}
	m_listOfBuffer.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// GET
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
DWORD CxWave::GetNumSamples()
{
	DWORD dwTotal = 0L;
	POSITION pos = m_listOfBuffer.GetHeadPosition();
	while (pos) {
		CWaveBuffer* p_waveBuffer = (CWaveBuffer*) m_listOfBuffer.GetNext(pos);
		dwTotal += p_waveBuffer->GetNumSamples();
	}
	return dwTotal;
}

//////////////////////////////////////////////////////////////////////
CString CxWave::GetInError() const
{
	if (m_nError != MMSYSERR_NOERROR) {
		TCHAR szText[MAXERRORLENGTH + 1];
		if ( waveInGetErrorText(m_nError, szText, MAXERRORLENGTH) == MMSYSERR_NOERROR ) {
			return szText;
		}
	}
	return "";
}

//////////////////////////////////////////////////////////////////////
DWORD CxWave::GetInPosition()
{
	if (m_hWaveIn) {
		MMTIME mmt;
		mmt.wType = TIME_SAMPLES;
		if ( IsInError(waveInGetPosition(m_hWaveIn, &mmt, sizeof(MMTIME))) ) {
			return -1;
		}
		else {
			return mmt.u.sample;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////
bool CxWave::IsInError(MMRESULT nResult)
{
	m_nError = nResult;
	return (m_nError != MMSYSERR_NOERROR);
}

//////////////////////////////////////////////////////////////////////
bool CxWave::IsRecording()
{
	bool bResult = false;
	if (m_nIndexWaveInHdr > -1 && m_tagWaveInHdr[m_nIndexWaveInHdr].dwFlags != 0) {
		bResult |= !(m_tagWaveInHdr[m_nIndexWaveInHdr].dwFlags & WHDR_DONE == WHDR_DONE);
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////
void CxWave::MakeWave()
{
	void* pBuffer = new char[GetNumSamples() * GetFormat().nBlockAlign];
	DWORD dwPosInBuffer = 0L;
	POSITION pos = m_listOfBuffer.GetHeadPosition();
	while (pos) {
		CWaveBuffer* p_waveBuffer = (CWaveBuffer*) m_listOfBuffer.GetNext(pos);
		CopyMemory( (char*)pBuffer + dwPosInBuffer, p_waveBuffer->GetBuffer(), p_waveBuffer->GetNumSamples() * p_waveBuffer->GetSampleSize() );
		dwPosInBuffer += p_waveBuffer->GetNumSamples() * p_waveBuffer->GetSampleSize();
	}
	SetBuffer( pBuffer, GetNumSamples() );
	//return m_wave;
}


//////////////////////////////////////////////////////////////////////
bool CxWave::ResetInRequired(CxWave* pWaveOut)
{
	return m_bResetRequired;
}

