#include<math.h>
#include"SignalProcessing.h"
// #pragma comment(lib,"./demo_rec_new.lib")
SignalProcessing::SignalProcessing()
{
}
SignalProcessing::~SignalProcessing()
{
}
void SignalProcessing::InitSingalProcessing(DWORD dwFFTsize,DWORD SamplingRate) // DWORD unsigned long
{
	m_dwFFTSize = dwFFTsize;
	m_SamplingRate = SamplingRate;

	m_pFFTAD = new CSpectrumProbe;
	m_pFFTAD->InitSpectrumProbeParam(2 * m_dwFFTSize, m_dwFFTSize, 8, Blackman, false, false, false);

	m_pFFTIQ = new CSpectrumProbe;
	m_pFFTIQ->InitSpectrumProbeParam(m_dwFFTSize, 1, 1, Hanning, true, false, false);

	m_pTmpBufI = new double[dwFFTsize];
}
//获取信号瞬时包络
double* SignalProcessing::CalSingalEnvelope(double* pData, long nDataLen)
{
	double* pResult = new double[nDataLen / 2];
	for (long i = 0; i < nDataLen / 2; i++)
	{
		pResult[i] = sqrt(pData[2 * i] * pData[2 * i] + pData[2 * i + 1] * pData[2 * i + 1]);
	}
	return pResult;
}
//获取信号瞬时频率
double* SignalProcessing::CalSingalFrequency(double* pData, long nDataLen)
{
	double* pResult = new double[nDataLen / 2];
	double ti, tr;
	for (long i = 0; i < nDataLen / 2 - 1; i++)
	{
		ti = pData[2 * i] * pData[2 * i + 3] - pData[2 * i + 2] * pData[2 * i + 1];
		tr = pData[2 * i + 2] * pData[2 * i] + pData[2 * i + 3] * pData[2 * i + 1];

		pResult[i] = atan2(ti, tr);
	}
	pResult[nDataLen / 2 - 1] = 0.0;
	return pResult;
}
//获取信号瞬时相位
double* SignalProcessing::CalSingalPhase(double* pData, long nDataLen)
{
	double* pResult = new double[nDataLen / 2];
	for (long i = 0; i < nDataLen / 2; i++)
	{
		pResult[i] = atan2(pData[2 * i + 1], pData[2 * i]);
	}
	return pResult;
}
//获取信号平方IQ
void SignalProcessing::CalSignalSquare(double *pData,long nDataLen)
{
	double dDataI = 0.0;
	double dDataQ = 0.0;

	for (long i = 0; i < nDataLen / 2; i++)
	{
		dDataI = pData[2 * i] * pData[2 * i] - pData[2 * i + 1] * pData[2 * i + 1];
		dDataQ = 2 * pData[2 * i] * pData[2 * i + 1];

		pData[2 * i] = dDataI;
		pData[2 * i + 1] = dDataQ;
	}
}
//信号下变频
void SignalProcessing::Signalddc(double *pData, long nDataLen,long nDDCFreq)
{
	long fc = nDDCFreq;
	long fs = m_SamplingRate;
	double* t = new double[nDataLen/2];
	double* DataI = new double[nDataLen / 2];
	double* DataQ = new double[nDataLen / 2];
	for (long i = 0; i < nDataLen / 2; i++)
	{
		t[i] = i *1.0 / fs;
		DataI[i] = pData[2 * i];
		DataQ[i] = pData[2 * i + 1];
	}
	for (int i = 0; i < nDataLen / 2; i++)
	{
		pData[2 * i] = DataI[i] * cos(2 * PI*fc*t[i]) - DataQ[i] * sin(2 * PI*fc*t[i]);
		pData[2 * i + 1] = DataI[i] * sin(2 * PI*fc*t[i]) + DataQ[i] * cos(2 * PI*fc*t[i]);
	}

	delete[]t;
	delete[]DataI;
	delete[]DataQ;
}
double* SignalProcessing::GetDiffSpecParam(double* pSignAmpl, DWORD dwDataLen, DWORD& dwSpecLen, bool bIsFreq)
{
	m_pTmpBufI[0] = 0;
	//Amplitude difference
	if (bIsFreq)
	{
		for (DWORD n = 1; n < dwDataLen; n++)
		{
			m_pTmpBufI[n] = fabs(pSignAmpl[n]) - fabs(pSignAmpl[n - 1]);
		}
	}
	else
	{
		for (DWORD n = 1; n < dwDataLen; n++)
		{
			m_pTmpBufI[n] = pSignAmpl[n] - pSignAmpl[n - 1];
		}
	}
	m_pFFTAD->InputData(m_pTmpBufI, dwDataLen);
	double*pSpecBuf = m_pFFTAD->GetSpectrum(dwSpecLen);
	return pSpecBuf;
}
bool SignalProcessing::GetOrderSpectrum(double* dIndata,DWORD Len,double* spectrum, double* spectrum2, double* spectrum4,double* spectrum8)
{
	//功率谱
	m_pFFTIQ->InputData(dIndata, m_dwFFTSize * 2);
	DWORD SpectrumLen = 0;
	spectrum = m_pFFTIQ->GetSpectrum(SpectrumLen);

	//二次方谱
	CalSignalSquare(dIndata, m_dwFFTSize * 2);
	m_pFFTIQ->InputData(dIndata, m_dwFFTSize * 2);
	DWORD SpectrumLen2 = 0;
	spectrum2 = m_pFFTIQ->GetSpectrum(SpectrumLen2);

	//四次方谱
	CalSignalSquare(dIndata, m_dwFFTSize * 2);
	m_pFFTIQ->InputData(dIndata, m_dwFFTSize * 2);
	DWORD SpectrumLen4 = 0;
	spectrum4 = m_pFFTIQ->GetSpectrum(SpectrumLen4);

	//八次方谱
	CalSignalSquare(dIndata, m_dwFFTSize * 2);
	m_pFFTIQ->InputData(dIndata, m_dwFFTSize * 2);
	DWORD SpectrumLen8 = 0;
	spectrum8 = m_pFFTIQ->GetSpectrum(SpectrumLen8);

	return true;
}