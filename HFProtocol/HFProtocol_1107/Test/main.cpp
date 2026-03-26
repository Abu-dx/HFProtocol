#include <cstdio>
#include <iostream>
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
#include "ipps.h"

#define FILE_INPUT			"G:/0_研究生项目汇总/昆山/db/"
#define DEMOD_OUTPUTPATH	"G:/王泽群资料/HFProtocol_1107/Test/demod/"

#pragma comment(lib,"Link11CLEW.lib")
#pragma comment(lib,"Link11SLEW.lib")
#pragma comment(lib,"MIL110Apro.lib")
#pragma comment(lib,"MIL110Bpro.lib")
#pragma comment(lib,"MIL141Apro.lib")
#pragma comment(lib,"MIL141Bpro.lib")
#pragma comment(lib,"NATO4285pro.lib")
#pragma comment(lib,"NATO4529pro.lib")
#pragma comment(lib,"ProtolDetect.lib")


BOOL	selprotolName[10] = {
	TRUE, TRUE, TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE, TRUE, TRUE};		// 设置为 true

int		selprotolNum = 8;				// 一共有8种短波协议


CString SampleToTime(__int64 pos, int fs)
{
	CString m_CurTime;
	__int64 t = 1000 * pos / fs + 1; // ms
	__int32 hout = t / (60000 * 60);
	t = t % (60000 * 60);
	__int32	minute = t / 60000;
	t = t % 60000;
	__int32 second = t / 1000;
	__int32 milsec = t % 1000;
	m_CurTime.Format("%02d:%02d:%02d.%03d", hout, minute, second, milsec);
	return m_CurTime;
}


OutList gl_OutList;
short   pDataFiledemode[2][2 * MAX_SEGMENT_SIZE];// 文件解调星座结果  



void Link11CLEWDemod() {
	float m_FileInsample = 9600;
	float m_FileFc = 605;

	// 设置文件名字，并打开文件
	CFile* m_File = new CFile();
	CString strFileName(_T("G:/0_研究生项目汇总/昆山/db/Link-11-CLEW.dat"));
	m_File->Open(strFileName, CFile::modeRead);

	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength()) / sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit, m_FileWritemod;
	
	strFileName = DEMOD_OUTPUTPATH + m_File->GetFileName() + ".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		printf("Create file failed\n");
		return;
	}
	strFileName = DEMOD_OUTPUTPATH + m_File->GetFileName() + ".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		// AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		printf("Create file failed\n");
		return;
	}
	strFileName = DEMOD_OUTPUTPATH + m_File->GetFileName() + ".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	CString addwrite;

	int nLeng = 10000, i, j;
	Ipp16s *data = ippsMalloc_16s(nLeng);
	Ipp8u *outbyte = ippsMalloc_8u(3000);
	Ipp32fc *expsym = ippsMalloc_32fc(nLeng);
	int OutLen, byteLeng;
	bool bdetect;


	CLink11CLEW m_Link11CLEW;
	float Insample = m_FileInsample;
	float Outsample = 7040.0;
	float rFreq = m_FileFc / Insample;// m_FileFc = 605
	float roll = 0.35;
	int SrctapLen = 1024;
	float Baud = 2400.0;
	int headNum;
	int allheadNum = 0;

	m_Link11CLEW.Link11CLEWdemode_ini(m_FileFc + 1100, nLeng, Insample, Outsample, SrctapLen);

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	int nProgressPos = 0;
	int p1 = 0, p2;
	//	radio=0;
	BOOL brun = true;
	while (brun)  // 循环处理AD数据
	{
		m_File->Seek(dataHavePro*sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data, nLeng*sizeof(short));
		nBytesRead = nBytesRead / sizeof(short);

		m_Link11CLEW.Link11CLEWdemode(data, nLeng, 1, outbyte, byteLeng, expsym, OutLen, headNum, bdetect);
		if (byteLeng > 0)
		{
			m_FileWritebit.Write(outbyte, byteLeng*sizeof(Ipp8u));
		}
		if (OutLen > 0)
		{
			for (i = 0; i < OutLen; i++)
			{
				pDataFiledemode[p1][2 * i] = (Ipp16s)(expsym[i].re);
				pDataFiledemode[p1][2 * i + 1] = (Ipp16s)(expsym[i].im);
			}
			OutLen = OutLen * 2;
			m_FileWritemod.Write(pDataFiledemode[p1], OutLen*sizeof(Ipp16s));
			p2 = OutLen / 2;
			// ::SendMessage(m_wndOut, WM_UPDATE_CONSTR, (WPARAM)&p2, (LPARAM)&p1);
			p1 = (p1 + 1) % 2;
		}
		if (headNum > 0)
		{
			for (i = 0; i < headNum; i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro + m_Link11CLEW.m_burstpos[i].m_begin, m_FileInsample);
				gl_OutList.EndTime = SampleToTime(dataHavePro + m_Link11CLEW.m_burstpos[i].m_end, m_FileInsample);
				gl_OutList.frequency = m_Link11CLEW.m_burstpos[i].m_fduopule;
				gl_OutList.sigType.Format("Link11CLEW");
				gl_OutList.PU = m_Link11CLEW.m_burstpos[i].m_address;
				if (m_Link11CLEW.m_burstpos[i].m_type == 1)
					gl_OutList.frameType.Format("主站轮询");
				else if (m_Link11CLEW.m_burstpos[i].m_type == 2)
					gl_OutList.frameType.Format("主站报告");
				else if (m_Link11CLEW.m_burstpos[i].m_type == 3)
					gl_OutList.frameType.Format("前哨回复");
				else
					gl_OutList.frameType.Format("");

				//	if(m_Link11CLEW.m_burstpos[i].m_address==1)
				//		radio++;

				addwrite.Format("%d\t%s\t%s\t%s\t%d\t%s", allheadNum, gl_OutList.sigType, gl_OutList.BeginTime, gl_OutList.EndTime, gl_OutList.PU, gl_OutList.frameType);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
				// ::SendMessage(m_wndOut, WM_UPDATE_OUT, 0, 0);
			}
		}

		dataHavePro = dataHavePro + nBytesRead;
		nProgressPos = (int)(100.0*dataHavePro / (double)m_FileLength);
		if (nProgressPos >= 100)nProgressPos = 100;
		// ::SendMessage(m_wndCtr, WM_SYSTEM_STATE, (WPARAM)dataHavePro, 0);
		if (nBytesRead < nLeng)
		{
			nProgressPos = 100;
			// ::SendMessage(m_wndCtr, WM_SYSTEM_STATE, (WPARAM)0, 0);
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
}


void Link11SLEWDemod() {
	float m_FileInsample = 9600;
	float m_FileFc = 2400.0;
	float m_DemodeTh = 6;

	// 设置文件名字，并打开文件
	CFile* m_File = new CFile();
	CString strFileName(_T("G:/0_研究生项目汇总/昆山/db/Link-11-SLEW.dat"));
	if (!m_File->Open(strFileName, CFile::modeRead)) {
		printf("无法打开文件\n");
		return;
	}

	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength()) / sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit, m_FileWritemod;

	strFileName = DEMOD_OUTPUTPATH + m_File->GetFileName() + ".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = DEMOD_OUTPUTPATH + m_File->GetFileName() + ".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = DEMOD_OUTPUTPATH + m_File->GetFileName() + ".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	myFileWrite.SeekToEnd();
	CString addwrite;

	int nLeng = 6000, i, j;
	Ipp16s* data = ippsMalloc_16s(nLeng * 3);
	Ipp8u* outbyte = ippsMalloc_8u(3000);
	int OutLen, byteLeng;

	CLink11SLEW m_Link11SLEW;
	float Insample = m_FileInsample;
	float Outsample = 9600.0;
	float rFreq = m_FileFc / Insample;
	float roll = 0.45;
	short SrctapLen = 512;
	float Baud = 2400.0;

	/// <summary>
	/// 目前，解调函数中检测不到信号！
	/// </summary>
	m_Link11SLEW.Demode_PSKrealFSE_ini(
		Insample, Outsample, nLeng, m_FileFc, 4, 512, roll, Baud, SrctapLen);
	HeadType* headType = new HeadType[10];
	int headNum;
	int allheadNum = 0;

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	int nProgressPos = 0;
	int p1 = 0, p2;
	//	radio=0;
	BOOL brun = true;
	while (brun)  // 循环处理AD数据
	{
		m_File->Seek(dataHavePro * sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data, nLeng * sizeof(short));
		nBytesRead = nBytesRead / sizeof(short);

		int deciP = 1;

		m_Link11SLEW.Demode_PSKrealFSE(data, nLeng, deciP, 512, 6, m_DemodeTh, OutLen, outbyte, byteLeng, headType, headNum);
		if (byteLeng > 0)
		{
			m_FileWritebit.Write(outbyte, byteLeng * sizeof(Ipp8u));
		}
		if (OutLen > 0)
		{
			for (i = 0; i < OutLen; i++)
				pDataFiledemode[p1][i] = (Ipp16s)(data[i]);

			m_FileWritemod.Write(pDataFiledemode[p1], OutLen * sizeof(Ipp16s));
			p2 = OutLen / 2;
			p1 = (p1 + 1) % 2;
		}
		if (headNum > 0)
		{
			for (i = 0; i < headNum; i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro + headType[i].position, m_FileInsample);
				gl_OutList.sigType.Format("Link11SLEW");
				gl_OutList.PU = (int)headType[i].address;
				gl_OutList.CRC = (int)headType[i].crcerr;
				gl_OutList.frequency = headType[i].frequency;
				//if(gl_OutList.CRC==0)
				//	radio = radio+1;

				if (headType[i].Type == 0)
					gl_OutList.frameType.Format("主站轮询");
				else if (headType[i].Type == 2)
					gl_OutList.frameType.Format("主站报告");
				else if (headType[i].Type == 3)
					gl_OutList.frameType.Format("前哨回复");
				else
					gl_OutList.frameType.Format("");

				addwrite.Format("%d\t\t%d\t%s\t\t%f\t\t%s\t\t%d", allheadNum, gl_OutList.PU, gl_OutList.BeginTime, gl_OutList.frequency, gl_OutList.frameType, gl_OutList.CRC);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
			}
		}
		dataHavePro = dataHavePro + nBytesRead;
		nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
		if (nProgressPos >= 100)nProgressPos = 100;
		if (nBytesRead < nLeng)
		{
			nProgressPos = 100;
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

// 目前 141A 解调函数会发生报错
/*
void MIL141ADemod() {
	float m_FileInsample = 9600;


	// 141A 中频
	double m_FileFc = 1800.0;

	// 设置文件名字，并打开文件
	CFile* m_File = new CFile();
	CString strFileName(_T("G:/0_研究生项目汇总/昆山/db/2-ALE.pcm"));
	m_File->Open(strFileName, CFile::modeRead);
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength()) / sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit;
	strFileName = m_File->GetFileName() + ".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFileName() + ".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	myFileWrite.SeekToEnd();
	CString addwrite;

	int nLeng = 6000, i, j;
	Ipp16s* data = ippsMalloc_16s(nLeng * 3);
	Ipp8u* outbyte = ippsMalloc_8u(nLeng * 3);
	int OutLen, byteLeng;
	CString message[100];
	CString address[100];
	int messagenum;

	CMIL141Apro m_MIL141Apro;
	float Insample = 9600;
	float Outsample = 12000.0;
	float rFreq = m_FileFc / Insample;
	float roll = 0.45;
	short SrctapLen = 512;
	float Baud = 2400.0;

	m_MIL141Apro.MIL141Ademode_ini(Insample, Outsample, 8, m_FileFc, 125, FALSE, nLeng, 4);

	int headNum;
	int allheadNum = 0;

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	int nProgressPos = 0;
	int p1 = 0, p2;
	BOOL brun = true;
	while (brun)  // 循环处理AD数据
	{
		m_File->Seek(dataHavePro * sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data, nLeng * sizeof(short));
		nBytesRead = nBytesRead / sizeof(short);

		m_MIL141Apro.MIL141Ademode(data, nLeng, outbyte, byteLeng, message, address, messagenum);
		if (byteLeng > 0)
		{
			m_FileWritebit.Write(outbyte, byteLeng * sizeof(Ipp8u));
		}
		if (messagenum > 0)
		{
			for (i = 0; i < messagenum; i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro, m_FileInsample);
				gl_OutList.sigType.Format("MIL141A");
				gl_OutList.frequency = m_FileFc;

				gl_OutList.message = message[i] + " " + address[i];

				addwrite.Format("%d\t\t%s\t\t%s\t\t%f\t\t%s", allheadNum, gl_OutList.BeginTime, gl_OutList.sigType, gl_OutList.frequency, gl_OutList.message);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
			}
		}
		dataHavePro = dataHavePro + nBytesRead;
		nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
		if (nProgressPos >= 100)nProgressPos = 10;
		if (nBytesRead < nLeng)
		{
			nProgressPos = 100;
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
*/


void MIL141BDemod() {
	float m_FileInsample = 9600;
	float m_FileFc = 1800;

	// 设置文件名字，并打开文件
	CFile* m_File = new CFile();
	CString strFileName(_T("G:/0_研究生项目汇总/昆山/db/3G-ALE.dat"));
	m_File->Open(strFileName, CFile::modeRead);
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength()) / sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit, m_FileWritemod;


	strFileName = DEMOD_OUTPUTPATH + m_File->GetFileName() + ".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = DEMOD_OUTPUTPATH + m_File->GetFileName() + ".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = DEMOD_OUTPUTPATH + m_File->GetFileName() + ".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	myFileWrite.SeekToEnd();
	CString addwrite;

	int nLeng = 6000, i, j;
	Ipp16s* data = ippsMalloc_16s(nLeng * 3);
	Ipp8u* outbyte = ippsMalloc_8u(nLeng * 3);
	int OutLen, byteLeng;

	CMIL141Bpro m_MIL141Bpro;
	float Insample = m_FileInsample;
	float Outsample = 9600.0;
	float rFreq = m_FileFc / Insample;
	float roll = 0.45;
	short SrctapLen = 512;
	float Baud = 2400.0;
	m_MIL141Bpro.MIL141Bdemode_ini(nLeng, Insample, Outsample, 4, roll, Baud, SrctapLen);

	HeadType141B* headType = new HeadType141B[10];
	int headNum;
	int allheadNum = 0;

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	int nProgressPos = 0;
	int p1 = 0, p2;
	BOOL brun = true;
	while (brun)  // 循环处理AD数据
	{
		m_File->Seek(dataHavePro * sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data, nLeng * sizeof(short));
		nBytesRead = nBytesRead / sizeof(short);

		m_MIL141Bpro.MIL141Bdemode(data, nLeng, 4, pDataFiledemode[p1], OutLen, outbyte, byteLeng, headType, headNum);
		if (byteLeng > 0)
		{
			m_FileWritebit.Write(outbyte, byteLeng * sizeof(Ipp8u));
		}
		if (OutLen > 0)
		{
			m_FileWritemod.Write(pDataFiledemode[p1], OutLen * sizeof(Ipp16s));
			p2 = OutLen / 2;
			p1 = (p1 + 1) % 2;
		}
		if (headNum > 0)
		{
			for (i = 0; i < headNum; i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro + headType[i].position, m_FileInsample);
				gl_OutList.sigType.Format("MIL141B");
				gl_OutList.frequency = headType[i].fre;

				if (headType[i].BWtype == wMIL141BBW0)
					gl_OutList.frameType.Format("BW0");
				else if (headType[i].BWtype == wMIL141BBW1)
					gl_OutList.frameType.Format("BW1");
				else if (headType[i].BWtype == wMIL141BBW2)
					gl_OutList.frameType.Format("BW2");
				else if (headType[i].BWtype == wMIL141BBW3)
					gl_OutList.frameType.Format("BW3");
				else if (headType[i].BWtype == wMIL141BBW5)
					gl_OutList.frameType.Format("BW5");
				else if (headType[i].BWtype == wMIL141BTLC)
					gl_OutList.frameType.Format("TLC");
				else
					gl_OutList.frameType.Format("");

				addwrite.Format("%d\t\t%s\t\t%s\t\t%f\t\t%s", allheadNum, gl_OutList.BeginTime, gl_OutList.sigType, gl_OutList.frequency, gl_OutList.frameType);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
			}
		}
		dataHavePro = dataHavePro + nBytesRead;
		nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
		if (nProgressPos >= 100)nProgressPos = 100;
		if (nBytesRead < nLeng)
		{
			nProgressPos = 100;
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

//// dataRate, FECType, InterType 无法确定？
/* 
void Demode4285()
{
	double m_FileInsample = 9600;
	double m_FileFc = 1800;
	double m_DemodeTh = 8;

	CFile* m_File = new CFile();
	CString strFileName(_T("G:/0_研究生项目汇总/昆山/db/S4285.dat"));
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength()) / sizeof(short)); // 文件长度(字)	

	CFile m_FileWritebit, m_FileWritemod;
	strFileName = m_File->GetFilePath() + ".bit";
	char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritebit.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath() + ".demode";
	pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
	if (!m_FileWritemod.Open(pFileName, CFile::modeCreate | CFile::modeWrite)) // 打开文件
	{
		AfxMessageBox("创建文件对象失败...", MB_ICONSTOP, 0);
		return;
	}
	strFileName = m_File->GetFilePath() + ".txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
	CString addwrite;

	int nLeng = 6000, i, j;
	Ipp16s* data = ippsMalloc_16s(nLeng * 2);
	Ipp8u* outbyte = ippsMalloc_8u(3000);
	int OutLen, byteLeng;

	CNATO4285pro m_NATO4285pro;
	float Insample = m_FileInsample;
	float Outsample = 9600.0;
	float rFreq = m_FileFc / Insample;
	float roll = 0.35;
	short SrctapLen = 512;
	float Baud = 2400.0;

	//// 需要手动输入的参数
	double dataRate = 3600;


	//	m_NATO4285pro.Demode_PSKrealFSE_ini(Insample,Outsample,8,rFreq,nLeng,SrctapLen,roll,Baud,80,4,8,dataRate,FECType,InterType);
	m_NATO4285pro.Demode_PSKrealFSE_ini(Insample, Outsample, nLeng, m_FileFc, 4, 128, roll, Baud, SrctapLen, 
		dataRate, FECType, InterType);
	HeadType4285* headType = new HeadType4285[nLeng / 80];
	int headNum;
	int allheadNum = 0;

	UINT nBytesRead;
	DWORD dataHavePro = 0;
	int nProgressPos = 0;
	int p1 = 0, p2;

	BOOL brun = true;
	while (brun)  // 循环处理AD数据
	{
		m_File->Seek(dataHavePro * sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(data, nLeng * sizeof(short));
		nBytesRead = nBytesRead / sizeof(short);

		//	m_NATO4285pro.Demode_PSKrealFSE(signal,nLeng,m_NATO4285pro.UWleng,4,0.8,OutLen,outbyte,byteLeng);
		m_NATO4285pro.Demode_PSKrealFSE(data, nLeng, 4, 128, 7, m_DemodeTh, OutLen, outbyte, byteLeng, headType, headNum);

		if (byteLeng > 0)
		{
			m_FileWritebit.Write(outbyte, byteLeng * sizeof(Ipp8u));
		}
		if (OutLen > 0)
		{
			for (i = 0; i < OutLen; i++)
				pDataFiledemode[p1][i] = (Ipp16s)(data[i]);

			m_FileWritemod.Write(pDataFiledemode[p1], OutLen * sizeof(Ipp16s));
			p2 = OutLen / 2;
			p1 = (p1 + 1) % 2;
		}
		if (headNum > 0)
		{
			for (i = 0; i < headNum; i++)
			{
				gl_OutList.BeginTime = SampleToTime(dataHavePro + headType[i].position, m_FileInsample);
				gl_OutList.sigType.Format("NATO4285");
				gl_OutList.frequency = headType[i].fre;
				gl_OutList.dataRate = headType[i].dataRate;
				gl_OutList.interType = headType[i].interType;

				addwrite.Format("%d\t%s\t%f\t%d\t%s", allheadNum, gl_OutList.sigType, gl_OutList.frequency, gl_OutList.dataRate, gl_OutList.interType);
				addwrite = addwrite + "\n";
				myFileWrite.WriteString(addwrite);
				allheadNum++;
			}
		}

		dataHavePro = dataHavePro + nBytesRead;
		nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
		if (nProgressPos >= 100)nProgressPos = 100;
		if (nBytesRead < nLeng)
		{
			nProgressPos = 100;
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
	delete[]headType;
	return;
}
*/


void DetectThread(CFile* m_File)
{	
	float m_FileInsample = 9600;

	int i, j;
	m_File->Seek(0, CFile::begin);
	ULONG m_FileLength = (ULONG)((m_File->GetLength()) / sizeof(short)); // 文件长度(字)	

	CString strFileName;
	strFileName = "G:/王泽群资料/HFProtocol_1107/Test/demod/detect.txt";
	CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);

	int nLeng = 8192;
	Ipp16s* inbuffer = ippsMalloc_16s(nLeng);

	float Insample = m_FileInsample;
	float Outsample = 9600.0;
	CProtolDetect* m_ProtolDetect;
	m_ProtolDetect = new CProtolDetect;

	m_ProtolDetect->ProtolDetect_ini(nLeng, Insample, selprotolName, selprotolNum);
	ProtocolOut m_ProtocolOut;
	BOOL detect;

	int allnum = 0;
	UINT nBytesRead;
	DWORD dataHavePro = 0;
	CString burstWrite;
	int nProgressPos = 0;
	bool brun = true;
	while (brun)  // 循环处理AD数据
	{
		m_File->Seek(dataHavePro * sizeof(short), CFile::begin);
		nBytesRead = m_File->Read(inbuffer, nLeng * sizeof(short));
		nBytesRead = nBytesRead / sizeof(short);

		detect = m_ProtolDetect->ProtolDetect(inbuffer, nLeng, 0.55, selprotolName, selprotolNum, m_ProtocolOut);

		if (detect)
		{
			gl_OutList.BeginTime = SampleToTime(__int64(dataHavePro * 9600.0 / Insample) + m_ProtocolOut.index, Outsample);
			gl_OutList.sigType = m_ProtocolOut.ProtocolName;
			gl_OutList.frequency = m_ProtocolOut.frequency;
			gl_OutList.dataRate = m_ProtocolOut.dataRate;
			gl_OutList.interLeng = m_ProtocolOut.InterLen;
			if (gl_OutList.sigType == "MIL141B")
			{
				if (m_ProtocolOut.waveType == wMIL141BBW0)
					gl_OutList.frameType = "BW0";
				else if (m_ProtocolOut.waveType == wMIL141BBW1)
					gl_OutList.frameType = "BW1";
				else if (m_ProtocolOut.waveType == wMIL141BBW3)
					gl_OutList.frameType = "BW3";
				else if (m_ProtocolOut.waveType == wMIL141BTLC)
					gl_OutList.frameType = "TLC";
			}

			burstWrite.Format("%d\t%s\t%s\t%f", allnum, gl_OutList.sigType, gl_OutList.BeginTime, gl_OutList.frequency);
			burstWrite = burstWrite + "\n";
			allnum++;
			myFileWrite.WriteString(burstWrite);
		}

		dataHavePro = dataHavePro + nBytesRead;
		nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
		if (nProgressPos >= 100)nProgressPos = 100;
		if (nBytesRead < nLeng)
		{
			nProgressPos = 100;
			break;
		}
	}
	m_ProtolDetect->ProtolDetect_free();
	delete m_ProtolDetect;
	brun = FALSE;
	myFileWrite.Close();
	ippsFree(inbuffer);
}





int main() {
	//CFile* m_file = new CFile();
	//CString strFileName = (_T("G:/0_研究生项目汇总/昆山/db/3G-ALE.dat"));
	//m_file->Open(strFileName, CFile::modeRead);
	//DetectThread(m_file);
	//m_file->Close();

	Link11CLEWDemod();
	//Link11SLEWDemod();
	// MIL141ADemod();
	//MIL141BDemod();
	// Demode4285();

	return 0;
}