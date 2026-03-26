// CFileWorkThread.h: interface for the CWorkParaThread class.
//
//////////////////////////////////////////////////////////////////////

//#if !defined(AFX_WORKPARATHREAD_H__05A9D439_5025_4F5D_9B59_1E3F34EAB7A3__INCLUDED_)
//#define AFX_WORKPARATHREAD_H__05A9D439_5025_4F5D_9B59_1E3F34EAB7A3__INCLUDED_
//#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XThread.h"


class CFileWorkThread : public CXThread  
{
public:
	
	CString m_strSourceFile;
	HWND m_wndCtr;
	HWND m_wndOut;
	BOOL brun;
	
	int nProgressPos ;

	int m_fileStytle;
	int m_FileInsample;
	int m_FileOutsample;
	
	float m_FileFc;
	int m_Order;
	int m_FreSpec;
	int m_FileMag;
	int dataRate,FECType,InterType;

	int protocolNum;
	int protocolType[9];
	CString protocolName[9];
	float m_FileBaud;
	float m_TH1,m_TH2;
	float m_DemodeTh;
	
	CFileWorkThread();
	virtual ~CFileWorkThread();
	enum EDSPOperation {
		opNone,
		opDemode,
		opDetect
	};	

	int m_opOpration;
	
	virtual void Run(void* param);
	virtual void RunSave(void* param);	
	void Tell_System(LONG index, BYTE number, BYTE findout,BYTE nPercent);
	CString SampleToTime(__int64 pos,int fs);

	void Demode110A(CFile *m_File);
	void Demode110B(CFile *m_File);
	void DemodeSLEW(CFile *m_File);
	void Demode4285(CFile *m_File);
	void Demode4529(CFile *m_File);
	void DemodeCLEW(CFile *m_File);
	void Demode141B(CFile *m_File);
	void DetectThread(CFile *m_File);
	void Demode141A(CFile *m_File);

};

// // !defined(AFX_WORKPARATHREAD_H__05A9D439_5025_4F5D_9B59_1E3F34EAB7A3__INCLUDED_)
