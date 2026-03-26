// WorkParaThread.cpp: implementation of the CWorkParaThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileWorkThread.h"
#include "xWave.h"
#include "MainFrm.h"
#include "MIL110Apro.h"
#include "MIL110Bpro.h"
#include "Link11SLEW.h"
#include "ProtolDetect.h"
#include "NATO4285pro.h"
#include "NATO4529pro.h"
#include "Link11CLEW.h"
#include "MIL141Bpro.h"
#include "MIL141Apro.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileWorkThread::CFileWorkThread()
{

}

CFileWorkThread::~CFileWorkThread()
{
	
}
void CFileWorkThread::RunSave(void* param)
{
	


}
void CFileWorkThread::Run(void* param)
{
	/*
	"MIL_110A","MIL_110B","MIL_141A","MIL_141B","Link11_SLEW","Link11_CLEW","NATO_4285","NATO_4529"
	*/
 	CFile *fpSource = (CFile*)param;
 	if(m_opOpration == opNone)
	{
		
	}
	else if (m_opOpration == opDemode && m_fileStytle==0) // 110A
	{
		Demode110A(fpSource);
	}
	else if (m_opOpration == opDemode && m_fileStytle==1) // 110B
	{
		Demode110B(fpSource);
	}
	else if (m_opOpration == opDemode && m_fileStytle==2)//141A
	{
		Demode141A(fpSource);
	}
	else if (m_opOpration == opDemode && m_fileStytle==3)//141B
	{
		Demode141B(fpSource);
	}
	else if (m_opOpration == opDemode && m_fileStytle==4)// Link11_SLEW
	{
		DemodeSLEW(fpSource);
	}
	else if (m_opOpration == opDemode && m_fileStytle==5)// Link11_CLEW
	{
		DemodeCLEW(fpSource);
	}
	else if (m_opOpration == opDemode && m_fileStytle==6) //NATO_4285
	{
		Demode4285(fpSource);
	}
	else if (m_opOpration == opDemode && m_fileStytle==7) // NATO_4529
	{
		Demode4529(fpSource);
	}
	else if (m_opOpration == opDetect)
	{
		DetectThread(fpSource);
	}


}

void CFileWorkThread::Tell_System(LONG index, BYTE number, BYTE findout,BYTE nPercent)
{
	
/*	LONG wPara_h = index;
	LONG wPara_l = (ULONG)((number<<24)^(findout<<16)^nPercent);
	::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)wPara_h,(LPARAM)wPara_l);*/
}


void CFileWorkThread::Demode110A(CFile *m_File)
{
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength())/sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit,m_FileWritemod;
	CString strFileName;
	strFileName = m_File->GetFilePath()+".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	CString addwrite;

	int nLeng=6000,i,j;
	Ipp16s *data=ippsMalloc_16s(nLeng);
	Ipp8u *outbyte = ippsMalloc_8u(3000);
	Ipp32f *signal = ippsMalloc_32f(nLeng);
	int OutLen,byteLeng;

	CMIL110Apro m_MIL110Apro;
	float Insample =m_FileInsample;
	float Outsample =9600.0;
	float rFreq = m_FileFc/Insample;
	float roll = 0.35;
	short SrctapLen = 512;
	float Baud=2400.0;
	HeadType110A *headType=new HeadType110A[30];
	int headNum;
	int allheadNum=0;

	m_MIL110Apro.Demode_PSKrealFSE_ini(Insample,Outsample,nLeng,m_FileFc,4,512,roll,Baud,SrctapLen);

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	nProgressPos = 0;
	int p1=0,p2;

	while (brun)  // 循环处理AD数据
	{ 
		m_File->Seek(dataHavePro*sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data,nLeng*sizeof(short));
		nBytesRead = nBytesRead/sizeof(short);

		m_MIL110Apro.Demode_PSKrealFSE(data,nLeng,4,512,6,m_DemodeTh,OutLen,outbyte,byteLeng,headType,headNum);
		//	m_MIL110Apro.Demode_PSKrealFSE(signal,nLeng,m_MIL110Apro.UWleng,4,0.45,OutLen,outbyte,byteLeng);
		if (byteLeng>0)
		{
			m_FileWritebit.Write(outbyte, byteLeng*sizeof(Ipp8u));
		}
		if (OutLen>0)
		{
			for (i=0;i<OutLen;i++)
				pDataFiledemode[p1][i] = (Ipp16s)(data[i]);

			m_FileWritemod.Write(pDataFiledemode[p1], OutLen*sizeof(Ipp16s));
			p2 = OutLen/2;
			::SendMessage(m_wndOut,WM_UPDATE_CONSTR,(WPARAM)&p2 ,(LPARAM)&p1);
			p1 = (p1+1)%2;
		}
		if(headNum>0)
		{
			for(i=0;i<headNum;i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro+headType[i].position,m_FileInsample);
				gl_OutList.sigType.Format("MIL110A");
				gl_OutList.frequency = headType[i].fre;
				gl_OutList.dataRate = headType[i].dataRate;
				gl_OutList.interLeng = headType[i].interLeng;

				addwrite.Format("%d\t%s\t%f\t%d\t%d",allheadNum,gl_OutList.sigType,gl_OutList.frequency,gl_OutList.dataRate,gl_OutList.interLeng);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
				::SendMessage(m_wndOut,WM_UPDATE_OUT,0,0);
			}
		}

		dataHavePro = dataHavePro+nBytesRead;
		nProgressPos = (int)(100.0*dataHavePro/(double)m_FileLength);
		if(nProgressPos>=100)nProgressPos = 100;
		::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)dataHavePro,0);
		if(nBytesRead<nLeng)
		{
			nProgressPos = 100;
			::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)0,0);
			break;
		}
	}
	brun = FALSE;
	m_MIL110Apro.Demode_PSKrealFSE_free();
	m_FileWritemod.Close();
	m_FileWritebit.Close();
	myFileWrite.Close();
	delete headType;
	ippsFree(data);
	ippsFree(signal);
	ippsFree(outbyte);
	return;

}
void CFileWorkThread::Demode110B(CFile *m_File)
{
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength())/sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit,m_FileWritemod;
	CString strFileName;
	strFileName = m_File->GetFilePath()+".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	CString addwrite;

	int nLeng=6000,i,j;
	Ipp16s *data=ippsMalloc_16s(nLeng);
	Ipp8u *outbyte = ippsMalloc_8u(3000);
	Ipp32f *signal = ippsMalloc_32f(nLeng);
	int OutLen,byteLeng;

	CMIL110Bpro m_MIL110Bpro;
	float Insample =m_FileInsample;
	float Outsample =9600.0;
	float rFreq = m_FileFc/Insample;
	float roll = 0.35;
	short SrctapLen = 512;
	float Baud=2400.0;
	HeadType110B *headType=new HeadType110B[10];
	int headNum;
	int allheadNum=0;

	m_MIL110Bpro.Demode_PSKrealFSE_ini(Insample,Outsample,nLeng,m_FileFc,4,512,roll,Baud,SrctapLen);
	//	m_MIL110Bpro.Demode_PSKrealFSE_ini(Insample,Outsample,8,rFreq,nLeng,SrctapLen,roll,Baud,287,4,8,0);

	UINT nBytesRead,nRead;
	DWORD dataHavePro = 0;
	nProgressPos = 0;
	int p1=0,p2;

	int outmoduType,outinterType;
	while (brun)  // 循环处理AD数据
	{ 
		m_File->Seek(dataHavePro*sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data,nLeng*sizeof(short));
		nBytesRead = nBytesRead/sizeof(short);

		//	m_MIL110Bpro.Demode_PSKrealFSE(signal,nLeng,m_MIL110Bpro.UWleng,4,0.45,OutLen,outbyte,byteLeng);
		m_MIL110Bpro.Demode_PSKrealFSE(data,nLeng,4,512,6,m_DemodeTh,OutLen,outbyte,byteLeng,headType,headNum);

		if (byteLeng>0)
		{
			m_FileWritebit.Write(outbyte, byteLeng*sizeof(Ipp8u));
		}
		if (OutLen>0)
		{
			for (i=0;i<OutLen;i++)
				pDataFiledemode[p1][i] = (Ipp16s)(data[i]);

			m_FileWritemod.Write(pDataFiledemode[p1], OutLen*sizeof(Ipp16s));
			p2 = OutLen/2;
			::SendMessage(m_wndOut,WM_UPDATE_CONSTR,(WPARAM)&p2 ,(LPARAM)&p1);
			p1 = (p1+1)%2;
		}
		if(headNum>0)
		{
			for(i=0;i<headNum;i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro+headType[i].position,m_FileInsample);
				gl_OutList.sigType.Format("MIL110B");
				gl_OutList.frequency = headType[i].fre;
				gl_OutList.dataRate = headType[i].dataRate;
				gl_OutList.interLeng = headType[i].interLeng;

				addwrite.Format("%d\t%s\t%f\t%d\t%d",allheadNum,gl_OutList.sigType,gl_OutList.frequency,gl_OutList.dataRate,gl_OutList.interLeng);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
				::SendMessage(m_wndOut,WM_UPDATE_OUT,0,0);
			}
		}

		dataHavePro = dataHavePro+nBytesRead;
		nProgressPos = (int)(100.0*dataHavePro/(double)m_FileLength);
		if(nProgressPos>=100)nProgressPos = 100;
		::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)dataHavePro,0);
		if(nBytesRead<nLeng)
		{
			nProgressPos = 100;
			::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)0,0);
			break;
		}
	}
	brun = FALSE;
	m_MIL110Bpro.Demode_PSKrealFSE_free();
	m_FileWritemod.Close();
	m_FileWritebit.Close();
	myFileWrite.Close();
	ippsFree(data);
	ippsFree(signal);
	ippsFree(outbyte);
	delete headType;
	return;


}
void CFileWorkThread::DemodeSLEW(CFile *m_File)
{
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength())/sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit,m_FileWritemod;
	CString strFileName;
	strFileName = m_File->GetFilePath()+".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	myFileWrite.SeekToEnd();
	CString addwrite;

	int nLeng=6000,i,j;
	Ipp16s *data=ippsMalloc_16s(nLeng*3);
	Ipp8u *outbyte = ippsMalloc_8u(3000);
	int OutLen,byteLeng;

	CLink11SLEW m_Link11SLEW;
	float Insample =m_FileInsample;
	float Outsample =9600.0;
	float rFreq = m_FileFc/Insample;
	float roll = 0.45;
	short SrctapLen = 512;
	float Baud=2400.0;
	m_Link11SLEW.Demode_PSKrealFSE_ini(Insample,Outsample,nLeng,m_FileFc,4,512,roll,Baud,SrctapLen);
	HeadType *headType=new HeadType[10];
	int headNum;
	int allheadNum=0;

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	nProgressPos = 0;
	int p1=0,p2;
	//	radio=0;
	while (brun)  // 循环处理AD数据
	{ 
		m_File->Seek(dataHavePro*sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data,nLeng*sizeof(short));
		nBytesRead = nBytesRead/sizeof(short);

		m_Link11SLEW.Demode_PSKrealFSE(data,nLeng,4,512,6,m_DemodeTh,OutLen,outbyte,byteLeng,headType,headNum);	
		if (byteLeng>0)
		{
			m_FileWritebit.Write(outbyte, byteLeng*sizeof(Ipp8u));
		}
		if (OutLen>0)
		{
			for (i=0;i<OutLen;i++)
				pDataFiledemode[p1][i] = (Ipp16s)(data[i]);

			m_FileWritemod.Write(pDataFiledemode[p1], OutLen*sizeof(Ipp16s));
			p2 = OutLen/2;
			::SendMessage(m_wndOut,WM_UPDATE_CONSTR,(WPARAM)&p2 ,(LPARAM)&p1);
			p1 = (p1+1)%2;
		}
		if(headNum>0)
		{
			for(i=0;i<headNum;i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro+headType[i].position,m_FileInsample);
				gl_OutList.sigType.Format("Link11SLEW");
				gl_OutList.PU =(int)headType[i].address;
				gl_OutList.CRC = (int)headType[i].crcerr;
				gl_OutList.frequency = headType[i].frequency;
				//if(gl_OutList.CRC==0)
				//	radio = radio+1;

				if(headType[i].Type==0)
					gl_OutList.frameType.Format("主站轮询"); 
				else if(headType[i].Type==2)
					gl_OutList.frameType.Format("主站报告");
				else if(headType[i].Type==3)
					gl_OutList.frameType.Format("前哨回复");
				else 
					gl_OutList.frameType.Format("");

				addwrite.Format("%d\t\t%d\t%s\t\t%f\t\t%s\t\t%d",allheadNum,gl_OutList.PU,gl_OutList.BeginTime,gl_OutList.frequency,gl_OutList.frameType,gl_OutList.CRC);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
				::SendMessage(m_wndOut,WM_UPDATE_OUT,0,0);
			}
		}
		dataHavePro = dataHavePro+nBytesRead;
		nProgressPos = (int)(100.0*dataHavePro/(double)m_FileLength);
		if(nProgressPos>=100)nProgressPos = 100;
		::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)dataHavePro,0);
		if(nBytesRead<nLeng)
		{
			nProgressPos = 100;
			::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)0,0);
			break;
		}
	}
	brun = FALSE;
	m_Link11SLEW.Demode_PSKrealFSE_free();
	m_FileWritemod.Close();
	m_FileWritebit.Close();
	myFileWrite.Close();
	ippsFree(data);
	ippsFree(outbyte);
	delete headType;
	return;

}
void CFileWorkThread::Demode141B(CFile *m_File)
{
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength())/sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit,m_FileWritemod;
	CString strFileName;
	strFileName = m_File->GetFilePath()+".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	myFileWrite.SeekToEnd();
	CString addwrite;

	int nLeng=6000,i,j;
	Ipp16s *data=ippsMalloc_16s(nLeng*3);
	Ipp8u *outbyte = ippsMalloc_8u(nLeng*3);
	int OutLen,byteLeng;

	CMIL141Bpro m_MIL141Bpro;
	float Insample =m_FileInsample;
	float Outsample =9600.0;
	float rFreq = m_FileFc/Insample;
	float roll = 0.45;
	short SrctapLen = 512;
	float Baud=2400.0;
	m_MIL141Bpro.MIL141Bdemode_ini(nLeng,Insample,Outsample,4,roll,Baud,SrctapLen);

	HeadType141B *headType=new HeadType141B[10];
	int headNum;
	int allheadNum=0;

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	nProgressPos = 0;
	int p1=0,p2;

	while (brun)  // 循环处理AD数据
	{ 
		m_File->Seek(dataHavePro*sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data,nLeng*sizeof(short));
		nBytesRead = nBytesRead/sizeof(short);

		m_MIL141Bpro.MIL141Bdemode(data,nLeng,4,pDataFiledemode[p1],OutLen,outbyte,byteLeng,headType,headNum);
		if (byteLeng>0)
		{
			m_FileWritebit.Write(outbyte, byteLeng*sizeof(Ipp8u));
		}
		if (OutLen>0)
		{
			m_FileWritemod.Write(pDataFiledemode[p1], OutLen*sizeof(Ipp16s));
			p2 = OutLen/2;
			::SendMessage(m_wndOut,WM_UPDATE_CONSTR,(WPARAM)&p2 ,(LPARAM)&p1);
			p1 = (p1+1)%2;
		}
		if(headNum>0)
		{
			for(i=0;i<headNum;i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro+headType[i].position,m_FileInsample);
				gl_OutList.sigType.Format("MIL141B");
				gl_OutList.frequency = headType[i].fre;

				if(headType[i].BWtype==wMIL141BBW0)
					gl_OutList.frameType.Format("BW0"); 
				else if(headType[i].BWtype==wMIL141BBW1)
					gl_OutList.frameType.Format("BW1");
				else if(headType[i].BWtype==wMIL141BBW2)
					gl_OutList.frameType.Format("BW2");
				else if(headType[i].BWtype==wMIL141BBW3)
					gl_OutList.frameType.Format("BW3");
				else if(headType[i].BWtype==wMIL141BBW5)
					gl_OutList.frameType.Format("BW5");
				else if(headType[i].BWtype==wMIL141BTLC)
					gl_OutList.frameType.Format("TLC");
				else 
					gl_OutList.frameType.Format("");

				addwrite.Format("%d\t\t%s\t\t%s\t\t%f\t\t%s",allheadNum,gl_OutList.BeginTime,gl_OutList.sigType,gl_OutList.frequency,gl_OutList.frameType);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
				::SendMessage(m_wndOut,WM_UPDATE_OUT,0,0);
			}
		}
		dataHavePro = dataHavePro+nBytesRead;
		nProgressPos = (int)(100.0*dataHavePro/(double)m_FileLength);
		if(nProgressPos>=100)nProgressPos = 100;
		::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)dataHavePro,0);
		if(nBytesRead<nLeng)
		{
			nProgressPos = 100;
			::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)0,0);
			break;
		}
	}
	brun = FALSE;
	m_MIL141Bpro.MIL141Bdemode_free();
	m_FileWritebit.Close();
	myFileWrite.Close();
	m_FileWritemod.Close();
	ippsFree(data);
	ippsFree(outbyte);
	delete headType;
	return;
}
void CFileWorkThread::Demode141A(CFile *m_File)
{
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength())/sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit;
	CString strFileName;
	strFileName = m_File->GetFilePath()+".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	myFileWrite.SeekToEnd();
	CString addwrite;

	int nLeng=6000,i,j;
	Ipp16s *data=ippsMalloc_16s(nLeng*3);
	Ipp8u *outbyte = ippsMalloc_8u(nLeng*3);
	int OutLen,byteLeng;
	CString message[100];
	CString address[100];
	int messagenum;

	CMIL141Apro m_MIL141Apro;
	float Insample =m_FileInsample;
	float Outsample =12000.0;
	float rFreq = m_FileFc/Insample;
	float roll = 0.45;
	short SrctapLen = 512;
	float Baud=2400.0;

	m_MIL141Apro.MIL141Ademode_ini(Insample,Outsample,8,m_FileFc,125,FALSE,nLeng,4);

	int headNum;
	int allheadNum=0;

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	nProgressPos = 0;
	int p1=0,p2;

	while (brun)  // 循环处理AD数据
	{ 
		m_File->Seek(dataHavePro*sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data,nLeng*sizeof(short));
		nBytesRead = nBytesRead/sizeof(short);

		m_MIL141Apro.MIL141Ademode(data,nLeng,outbyte,byteLeng,message,address,messagenum);
		if (byteLeng>0)
		{
			m_FileWritebit.Write(outbyte, byteLeng*sizeof(Ipp8u));
		}
		if(messagenum>0)
		{
			for(i=0;i<messagenum;i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro,m_FileInsample);
				gl_OutList.sigType.Format("MIL141A");
				gl_OutList.frequency = m_FileFc;

				gl_OutList.message = message[i] +" "+ address[i];

				addwrite.Format("%d\t\t%s\t\t%s\t\t%f\t\t%s",allheadNum,gl_OutList.BeginTime,gl_OutList.sigType,gl_OutList.frequency,gl_OutList.message);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
				::SendMessage(m_wndOut,WM_UPDATE_OUT,0,0);
			}
		}
		dataHavePro = dataHavePro+nBytesRead;
		nProgressPos = (int)(100.0*dataHavePro/(double)m_FileLength);
		if(nProgressPos>=100)nProgressPos = 100;
		::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)dataHavePro,0);
		if(nBytesRead<nLeng)
		{
			nProgressPos = 100;
			::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)0,0);
			break;
		}
	}
	brun = FALSE;
	m_MIL141Apro.MIL141Ademode_free();
	m_FileWritebit.Close();
	myFileWrite.Close();
	ippsFree(data);
	ippsFree(outbyte);
	return;
}

void CFileWorkThread::Demode4285(CFile *m_File)
{
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength())/sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit,m_FileWritemod;
	CString strFileName;
	strFileName = m_File->GetFilePath()+".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	CString addwrite;

	int nLeng=6000,i,j;
	Ipp16s *data=ippsMalloc_16s(nLeng*2);
	Ipp8u *outbyte = ippsMalloc_8u(3000);
	int OutLen,byteLeng;

	CNATO4285pro m_NATO4285pro;
	float Insample =m_FileInsample;
	float Outsample =9600.0;
	float rFreq = m_FileFc/Insample;
	float roll = 0.35;
	short SrctapLen = 512;
	float Baud=2400.0;

	//	m_NATO4285pro.Demode_PSKrealFSE_ini(Insample,Outsample,8,rFreq,nLeng,SrctapLen,roll,Baud,80,4,8,dataRate,FECType,InterType);
	m_NATO4285pro.Demode_PSKrealFSE_ini(Insample,Outsample,nLeng,m_FileFc,4,128,roll,Baud,SrctapLen,dataRate,FECType,InterType);
	HeadType4285 *headType=new HeadType4285[nLeng/80];
	int headNum;
	int allheadNum=0;

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	nProgressPos = 0;
	int p1=0,p2;

	while (brun)  // 循环处理AD数据
	{ 
		m_File->Seek(dataHavePro*sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data,nLeng*sizeof(short));
		nBytesRead = nBytesRead/sizeof(short);

		//	m_NATO4285pro.Demode_PSKrealFSE(signal,nLeng,m_NATO4285pro.UWleng,4,0.8,OutLen,outbyte,byteLeng);
		m_NATO4285pro.Demode_PSKrealFSE(data,nLeng,4,128,7,m_DemodeTh,OutLen,outbyte,byteLeng,headType,headNum);	

		if (byteLeng>0)
		{
			m_FileWritebit.Write(outbyte, byteLeng*sizeof(Ipp8u));
		}
		if (OutLen>0)
		{
			for (i=0;i<OutLen;i++)
				pDataFiledemode[p1][i] = (Ipp16s)(data[i]);

			m_FileWritemod.Write(pDataFiledemode[p1], OutLen*sizeof(Ipp16s));
			p2 = OutLen/2;
			::SendMessage(m_wndOut,WM_UPDATE_CONSTR,(WPARAM)&p2 ,(LPARAM)&p1);
			p1 = (p1+1)%2;
		}
		if(headNum>0)
		{
			for(i=0;i<headNum;i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro+headType[i].position,m_FileInsample);
				gl_OutList.sigType.Format("NATO4285");
				gl_OutList.frequency = headType[i].fre;
				gl_OutList.dataRate = headType[i].dataRate;
				gl_OutList.interType = headType[i].interType;

				addwrite.Format("%d\t%s\t%f\t%d\t%s",allheadNum,gl_OutList.sigType,gl_OutList.frequency,gl_OutList.dataRate,gl_OutList.interType);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
				::SendMessage(m_wndOut,WM_UPDATE_OUT,0,0);
			}
		}

		dataHavePro = dataHavePro+nBytesRead;
		nProgressPos = (int)(100.0*dataHavePro/(double)m_FileLength);
		if(nProgressPos>=100)nProgressPos = 100;
		::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)dataHavePro,0);
		if(nBytesRead<nLeng)
		{
			nProgressPos = 100;
			::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)0,0);
			break;
		}
	}
	brun = FALSE;
	m_NATO4285pro.Demode_PSKrealFSE_free();
	m_FileWritemod.Close();
	m_FileWritebit.Close();
	myFileWrite.Close();
	ippsFree(data);
	ippsFree(outbyte);
	delete []headType;
	return;
}
void CFileWorkThread::Demode4529(CFile *m_File)
{
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength())/sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit,m_FileWritemod;
	CString strFileName;
	strFileName = m_File->GetFilePath()+".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	CString addwrite;

	int nLeng=6000,i,j;
	Ipp16s *data=ippsMalloc_16s(nLeng);
	Ipp8u *outbyte = ippsMalloc_8u(3000);
	int OutLen,byteLeng;

	CNATO4529pro m_NATO4529pro;
	float Insample =m_FileInsample;
	float Outsample =9600.0;
	float rFreq = m_FileFc/Insample;
	float roll = 0.35;
	short SrctapLen = 2048;
	float Baud=1200.0;
	m_NATO4529pro.Demode_PSKrealFSE_ini(Insample,Outsample,nLeng,m_FileFc,8,128,roll,Baud,SrctapLen,dataRate,FECType,InterType);
//	m_NATO4529pro.Demode_PSKrealFSE_ini(Insample,Outsample,8,rFreq,nLeng,SrctapLen,roll,Baud,80,4,8,dataRate,FECType,InterType);
	HeadType4529 *headType=new HeadType4529[nLeng/80];
	int headNum;
	int allheadNum=0;


	UINT nBytesRead;
	DWORD dataHavePro = 0;
	nProgressPos = 0;
	int p1=0,p2;

	while (brun)  // 循环处理AD数据
	{ 
		m_File->Seek(dataHavePro*sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data,nLeng*sizeof(short));
		nBytesRead = nBytesRead/sizeof(short);

		m_NATO4529pro.Demode_PSKrealFSE(data,nLeng,8,128,7,m_DemodeTh,OutLen,outbyte,byteLeng,headType,headNum);	

		if (byteLeng>0)
		{
			m_FileWritebit.Write(outbyte, byteLeng*sizeof(Ipp8u));
		}
		if (OutLen>0)
		{
			for (i=0;i<OutLen;i++)
				pDataFiledemode[p1][i] = (Ipp16s)(data[i]);

			m_FileWritemod.Write(pDataFiledemode[p1], OutLen*sizeof(Ipp16s));
			p2 = OutLen/2;
			::SendMessage(m_wndOut,WM_UPDATE_CONSTR,(WPARAM)&p2 ,(LPARAM)&p1);
			p1 = (p1+1)%2;
		}
		if(headNum>0)
		{
			for(i=0;i<headNum;i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro+headType[i].position,m_FileInsample);
				gl_OutList.sigType.Format("NATO4529");
				gl_OutList.frequency = headType[i].fre;
				gl_OutList.dataRate = headType[i].dataRate;
				gl_OutList.interType = headType[i].interType;

				addwrite.Format("%d\t%s\t%f\t%d\t%s",allheadNum,gl_OutList.sigType,gl_OutList.frequency,gl_OutList.dataRate,gl_OutList.interType);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
				::SendMessage(m_wndOut,WM_UPDATE_OUT,0,0);
			}
		}

		dataHavePro = dataHavePro+nBytesRead;
		nProgressPos = (int)(100.0*dataHavePro/(double)m_FileLength);
		if(nProgressPos>=100)nProgressPos = 100;
		::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)dataHavePro,0);
		if(nBytesRead<nLeng)
		{
			nProgressPos = 100;
			::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)0,0);
			break;
		}
	}
	brun = FALSE;
	m_NATO4529pro.Demode_PSKrealFSE_free();
	m_FileWritemod.Close();
	m_FileWritebit.Close();
	myFileWrite.Close();
	ippsFree(data);
	ippsFree(outbyte);
	delete []headType;
	return;
}


void CFileWorkThread::DemodeCLEW(CFile *m_File)
{
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength())/sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit,m_FileWritemod;
	CString strFileName;
	strFileName = m_File->GetFilePath()+".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath()+".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	CString addwrite;

	int nLeng=10000,i,j;
	Ipp16s *data=ippsMalloc_16s(nLeng);
	Ipp8u *outbyte = ippsMalloc_8u(3000);
	Ipp32fc *expsym = ippsMalloc_32fc(nLeng);
	int OutLen,byteLeng;
	bool bdetect;

	CLink11CLEW m_Link11CLEW;
	float Insample =m_FileInsample;
	float Outsample =7040.0;
	float rFreq = m_FileFc/Insample;// m_FileFc = 605
	float roll = 0.35;
	int SrctapLen = 1024;
	float Baud=2400.0;
	int headNum;
	int allheadNum=0;

	m_Link11CLEW.Link11CLEWdemode_ini(m_FileFc+1100,nLeng,Insample,Outsample,SrctapLen);

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	nProgressPos = 0;
	int p1=0,p2;
	//	radio=0;
	while (brun)  // 循环处理AD数据
	{ 
		m_File->Seek(dataHavePro*sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data,nLeng*sizeof(short));
		nBytesRead = nBytesRead/sizeof(short);

		m_Link11CLEW.Link11CLEWdemode(data,nLeng,1,outbyte,byteLeng,expsym,OutLen,headNum,bdetect);
		if (byteLeng>0)
		{
			m_FileWritebit.Write(outbyte, byteLeng*sizeof(Ipp8u));
		}
		if (OutLen>0)
		{
			for (i=0;i<OutLen;i++)
			{
				pDataFiledemode[p1][2*i] = (Ipp16s)(expsym[i].re);
				pDataFiledemode[p1][2*i+1] = (Ipp16s)(expsym[i].im);
			}
			OutLen = OutLen*2;
			m_FileWritemod.Write(pDataFiledemode[p1], OutLen*sizeof(Ipp16s));
			p2 = OutLen/2;
			::SendMessage(m_wndOut,WM_UPDATE_CONSTR,(WPARAM)&p2 ,(LPARAM)&p1);
			p1 = (p1+1)%2;
		}
		if(headNum>0)
		{
			for(i=0;i<headNum;i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro+m_Link11CLEW.m_burstpos[i].m_begin,m_FileInsample);
				gl_OutList.EndTime = SampleToTime(dataHavePro+m_Link11CLEW.m_burstpos[i].m_end,m_FileInsample);
				gl_OutList.frequency = m_Link11CLEW.m_burstpos[i].m_fduopule;
				gl_OutList.sigType.Format("Link11CLEW");
				gl_OutList.PU =  m_Link11CLEW.m_burstpos[i].m_address;
				if(m_Link11CLEW.m_burstpos[i].m_type==1)
					gl_OutList.frameType.Format("主站轮询"); 
				else if(m_Link11CLEW.m_burstpos[i].m_type==2)
					gl_OutList.frameType.Format("主站报告");
				else if(m_Link11CLEW.m_burstpos[i].m_type==3)
					gl_OutList.frameType.Format("前哨回复");
				else 
					gl_OutList.frameType.Format("");

				//	if(m_Link11CLEW.m_burstpos[i].m_address==1)
				//		radio++;

				addwrite.Format("%d\t%s\t%s\t%s\t%d\t%s",allheadNum,gl_OutList.sigType,gl_OutList.BeginTime,gl_OutList.EndTime,gl_OutList.PU,gl_OutList.frameType);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
				::SendMessage(m_wndOut,WM_UPDATE_OUT,0,0);
			}
		}

		dataHavePro = dataHavePro+nBytesRead;
		nProgressPos = (int)(100.0*dataHavePro/(double)m_FileLength);
		if(nProgressPos>=100)nProgressPos = 100;
		::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)dataHavePro,0);
		if(nBytesRead<nLeng)
		{
			nProgressPos = 100;
			::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)0,0);
			break;
		}
	}

	//addwrite.Format("%f\t",radio/1000);
	//RadioFile.WriteString(addwrite);

	brun = FALSE;
	m_Link11CLEW.Link11CLEWdemode_free();
	m_FileWritemod.Close();
	m_FileWritebit.Close();
	myFileWrite.Close();

	ippsFree(data);
	ippsFree(expsym);
	ippsFree(outbyte);
	return;

}
CString CFileWorkThread::SampleToTime(__int64 pos,int fs)
{
	CString m_CurTime;
	__int64 t=1000*pos/fs+1; // ms
	__int32 hout = t/(60000*60);
	t = t%(60000*60);
	__int32	minute = t/60000;
	t = t%60000;
	__int32 second = t/1000;
	__int32 milsec = t%1000;
	m_CurTime.Format("%02d:%02d:%02d.%03d",hout,minute,second,milsec);
	return m_CurTime;
}

void CFileWorkThread::DetectThread(CFile *m_File)
{
	int i,j;
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength())/sizeof(short)); // 文件长度(字)	

	CString strFileName;
	strFileName = m_File->GetFilePath()+"detect.txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);

	int nLeng = 8192;
	Ipp16s *inbuffer = ippsMalloc_16s(nLeng);

	float Insample =m_FileInsample;
	float Outsample =9600.0;
	CProtolDetect *m_ProtolDetect;
	m_ProtolDetect = new CProtolDetect;

	m_ProtolDetect->ProtolDetect_ini(nLeng,Insample,selprotolName,selprotolNum);
	ProtocolOut m_ProtocolOut;
	BOOL detect;

	int allnum=0;
	UINT nBytesRead;
	DWORD dataHavePro = 0;
	CString burstWrite;
	nProgressPos = 0;

	while (brun)  // 循环处理AD数据
	{ 
		m_File->Seek(dataHavePro*sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(inbuffer,nLeng*sizeof(short));
		nBytesRead = nBytesRead/sizeof(short);

		detect = m_ProtolDetect->ProtolDetect(inbuffer,nLeng,0.55,selprotolName,selprotolNum,m_ProtocolOut);

		if(detect)
		{
			gl_OutList.BeginTime = SampleToTime(__int64(dataHavePro*9600.0/Insample)+m_ProtocolOut.index,Outsample);
			gl_OutList.sigType = m_ProtocolOut.ProtocolName;
			gl_OutList.frequency = m_ProtocolOut.frequency;
			gl_OutList.dataRate = m_ProtocolOut.dataRate;
			gl_OutList.interLeng = m_ProtocolOut.InterLen;
			if(gl_OutList.sigType=="MIL141B")
			{
				if(m_ProtocolOut.waveType==wMIL141BBW0)
					gl_OutList.frameType = "BW0";
				else if(m_ProtocolOut.waveType==wMIL141BBW1)
					gl_OutList.frameType = "BW1";
				else if(m_ProtocolOut.waveType==wMIL141BBW3)
					gl_OutList.frameType = "BW3";
				else if(m_ProtocolOut.waveType==wMIL141BTLC)
					gl_OutList.frameType = "TLC";
			}

			burstWrite.Format("%d\t%s\t%s\t%f",allnum,gl_OutList.sigType,gl_OutList.BeginTime,gl_OutList.frequency);
			burstWrite = burstWrite + "\n";
			allnum++;
			myFileWrite.WriteString(burstWrite);	
			::SendMessage(m_wndOut,WM_UPDATE_OUT,0,0);
		}

		dataHavePro = dataHavePro+nBytesRead;
		nProgressPos = (int)(100.0*dataHavePro/(double)m_FileLength);
		if(nProgressPos>=100)nProgressPos = 100;
		::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)dataHavePro,0);
		if(nBytesRead<nLeng)
		{
			nProgressPos = 100;
			::SendMessage(m_wndCtr,WM_SYSTEM_STATE,(WPARAM)0,0);
			break;
		}
	}
	m_ProtolDetect->ProtolDetect_free();
	delete m_ProtolDetect;
	brun = FALSE;
	myFileWrite.Close();
	ippsFree(inbuffer);
}
