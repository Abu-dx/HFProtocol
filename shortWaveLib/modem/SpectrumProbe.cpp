#include "SpectrumProbe.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ostream>
#include <fstream>
#include <mutex>

std::mutex fftwMtx;                // FFTW ��


CSpectrumProbe::CSpectrumProbe(void)
{
	m_pDataBuff = nullptr;
	m_dwDataLen = 0;

	m_dwSpecLen = 0;
	m_pAvgSpec = nullptr;

	m_nFFTWin = Hanning;

	m_pFFT = nullptr;

	m_pFFTin = nullptr;
	m_pFFTout = nullptr;

	m_pWindow = nullptr;

	m_bIslog = false;
	m_bIsComplex = false;
	m_bIsInverse = false;

	memset(&m_SpecPara,0,sizeof(SPECTRUM_PROBE_PARAM));
}

CSpectrumProbe::~CSpectrumProbe(void)
{
	///释放资源///
	if (m_pFFTin != nullptr)
	{
		fftw_free(m_pFFTin);

		m_pFFTin = nullptr;
	}
	if (m_pFFTout != nullptr)
	{
		fftw_free(m_pFFTout);

		m_pFFTout = nullptr;
	}
	if (m_pFFT != nullptr)
	{
		fftw_destroy_plan(m_pFFT);

		m_pFFT = nullptr;
	}	

	if (m_pWindow != nullptr)
	{
		delete []m_pWindow;

		m_pWindow = nullptr;
	}

	if (nullptr != m_pDataBuff)
	{
		delete []m_pDataBuff;

		m_pDataBuff = nullptr;
	}	

	if (nullptr != m_pAvgSpec)
	{
        delete[] m_pAvgSpec;

		m_pAvgSpec = nullptr;
	}
}


uint32_t CSpectrumProbe::InitSpectrumProbeParam(uint32_t dwFFTSize, 
											 uint32_t dwFFTMove, 
											 uint16_t nAvgNum, 
											 uint16_t nWinType, 
											 bool bIsComplex,
											 bool bInverse,
											 bool bIslog)
{
	uint32_t dwErrorID = ERROR_SUCCESS;

	m_SpecPara.dwFFTMove = dwFFTMove;
	m_SpecPara.dwFFTSize = dwFFTSize;
	m_SpecPara.nAverNum = nAvgNum;
	m_SpecPara.nWindType = nWinType;

	m_bIsComplex = bIsComplex;
	m_bIsInverse = bInverse;
	m_bIslog = bIslog;

	if (m_pFFTin != nullptr)
	{
		fftw_free(m_pFFTin);

		m_pFFTin = nullptr;
	}
	if (m_pFFTout != nullptr)
	{
		fftw_free(m_pFFTout);

		m_pFFTout = nullptr;
	}
	if (m_pFFT != nullptr)
	{
		fftw_destroy_plan(m_pFFT);

		m_pFFT = nullptr;
	}	
	if (m_pWindow != nullptr)
	{
		delete []m_pWindow;

		m_pWindow = nullptr;
	}

	if (nullptr != m_pDataBuff)
	{
		delete []m_pDataBuff;

		m_pDataBuff = nullptr;
	}

	if (nullptr != m_pAvgSpec)
	{
		delete []m_pAvgSpec;

		m_pAvgSpec = nullptr;
	}

	///申请资源///
	m_pFFTin = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*dwFFTSize);
	m_pFFTout = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*dwFFTSize);

	fftwMtx.lock();
	m_pFFT = fftw_plan_dft_1d(dwFFTSize,m_pFFTin,m_pFFTout,
		FFTW_FORWARD,FFTW_ESTIMATE);
	fftwMtx.unlock();

	m_pWindow = new double[dwFFTSize];

	///产生窗函�?///
	ComputeWindow();
	
	// 数据缓存长度


	m_dwDataLen = ((nAvgNum - 1) * dwFFTMove + dwFFTSize);
	m_dwDataLen = bIsComplex ? m_dwDataLen * 2 : m_dwDataLen;

	m_pDataBuff = new double[m_dwDataLen];
	memset(m_pDataBuff,0,m_dwDataLen*sizeof(double));

	m_dwInputLen = 0;
	
	
	// m_dwSpecLen = (bIsComplex + 1) * (dwFFTSize / 2);
	m_dwSpecLen = bIsComplex ? dwFFTSize : dwFFTSize / 2;



	m_pAvgSpec = new double[m_dwSpecLen];

	memset(m_pAvgSpec,0,m_dwSpecLen*sizeof(double));

	return(dwErrorID);	
}

uint32_t CSpectrumProbe::InputData(long* lpInData, uint32_t dwInlen)
{
	uint32_t i = 0;
	uint32_t dwErrorID = ERROR_SUCCESS;

	memset(m_pDataBuff,0,m_dwDataLen*sizeof(double));

	m_dwInputLen = dwInlen > m_dwDataLen ? m_dwDataLen:dwInlen;

	for (i = 0; i < m_dwInputLen; i++)
	{
		m_pDataBuff[i] = lpInData[i];
	}

	return(dwErrorID);
}

uint32_t CSpectrumProbe::InputData(double* lpInData, uint32_t dwInlen)
{
	uint32_t dwErrorID = ERROR_SUCCESS;

	memset(m_pDataBuff,0,m_dwDataLen*sizeof(double));

	m_dwInputLen = dwInlen > m_dwDataLen ? m_dwDataLen:dwInlen;

	memcpy(m_pDataBuff,lpInData,m_dwInputLen*sizeof(double));


	return(dwErrorID);
}

double* CSpectrumProbe::GetSpectrum(DWORD& dwSpecLen)
{
	if (m_pAvgSpec == nullptr)
	{
		dwSpecLen = 0;

		return(nullptr);		
	}

	uint16_t K = 0;
	uint32_t i = 0;
	uint32_t N = 0;

	double dTemp = 0.0;

	memset(m_pAvgSpec,0,m_dwSpecLen*sizeof(double));

	if (m_bIsComplex) // IQ数据
	{
		for (K = 0; K < m_SpecPara.nAverNum; K++)
		{
			memset(m_pFFTin,0,m_SpecPara.dwFFTSize*sizeof(fftw_complex));
			memset(m_pFFTout,0,m_SpecPara.dwFFTSize*sizeof(fftw_complex));

			for (i = 0;i < m_SpecPara.dwFFTSize; i++)
			{
				N = K*m_SpecPara.dwFFTMove + i;				

				m_pFFTin[i][0] = (*(m_pDataBuff + 2*N))*m_pWindow[i];
				m_pFFTin[i][1] = (*(m_pDataBuff + 2*N + 1))*m_pWindow[i];
			}

			fftw_execute(m_pFFT);			

			N = m_SpecPara.dwFFTSize/2;

			for (i = 0; i<m_SpecPara.dwFFTSize/2;i++)
			{
				m_pAvgSpec[i] += sqrt(m_pFFTout[N][0]*m_pFFTout[N][0] + 
					m_pFFTout[N][1]*m_pFFTout[N][1])/m_SpecPara.dwFFTSize;

				m_pAvgSpec[N] += sqrt(m_pFFTout[i][0]*m_pFFTout[i][0] + 
					m_pFFTout[i][1]*m_pFFTout[i][1])/m_SpecPara.dwFFTSize;				

				N++;
			}
		}
	}
	else  // AD数据
	{
		for (K = 0; K < m_SpecPara.nAverNum; K++)
		{
			memset(m_pFFTin,0,m_SpecPara.dwFFTSize*sizeof(fftw_complex));
			memset(m_pFFTout,0,m_SpecPara.dwFFTSize*sizeof(fftw_complex));

			for (i = 0;i < m_SpecPara.dwFFTSize; i++)
			{
				N = K*m_SpecPara.dwFFTMove + i;

				m_pFFTin[i][0] = (*(m_pDataBuff + N))*m_pWindow[i];
			}

			fftw_execute(m_pFFT);	

			for (i = 0; i<m_SpecPara.dwFFTSize/2;i++)
			{
				m_pAvgSpec[i] += sqrt(m_pFFTout[i][0]*m_pFFTout[i][0] + 
					m_pFFTout[i][1]*m_pFFTout[i][1])/m_SpecPara.dwFFTSize;	
			}
		}		
	}

	if (m_SpecPara.nAverNum > 1)
	{
		// 取平�?
		for (i = 0; i<m_dwSpecLen;i++)
		{
			m_pAvgSpec[i] /= m_SpecPara.nAverNum;
		}
	}

	// 取对�?
	if (m_bIslog)
	{
		for (i = 0; i<m_dwSpecLen;i++)
		{
			m_pAvgSpec[i] = 20*log10(m_pAvgSpec[i]);
		}
	}

	// 取反�? //
	if (m_bIsInverse)
	{
		for (i = 0; i<m_dwSpecLen/2;i++)
		{
			dTemp = m_pAvgSpec[i];

			m_pAvgSpec[i] = m_pAvgSpec[m_dwSpecLen-1-i];

			m_pAvgSpec[m_dwSpecLen-1-i] = dTemp;
		}
	}

	dwSpecLen = m_dwSpecLen;

	return(m_pAvgSpec);	
}


///产生窗函�?///
void CSpectrumProbe::ComputeWindow()
{	
	double dOmiga;
	dOmiga = 2.0*M_PI/m_SpecPara.dwFFTSize;

	uint32_t i;
	switch(m_nFFTWin)
	{
	case Hanning:
		for(i=0; i<m_SpecPara.dwFFTSize; i++)
		{
			*(m_pWindow+i) = (1.0-cos(i*dOmiga))*0.5;
		}
		break;
	case Hamming:
		for(i=0; i<m_SpecPara.dwFFTSize; i++)
		{
			*(m_pWindow+i) =  0.54 - 0.46*cos(i*dOmiga);
		}
		break;
	case Blackman:
		for(i=0; i<m_SpecPara.dwFFTSize; i++)
		{
			*(m_pWindow+i) =  0.42 - 0.5*cos(i*dOmiga) 
				+ 0.08*cos(2*i*dOmiga);
		}
		break;
	case Blackman_Harris:
		for(i=0; i<m_SpecPara.dwFFTSize; i++)
		{
			*(m_pWindow+i) =  0.42323 - 0.49755*cos(i*dOmiga) 
				+ 0.07922*cos(2.0*i*dOmiga);
		}
		break;
	default:	//Rectangular
		for(i=0; i<m_SpecPara.dwFFTSize; i++)
		{
			*(m_pWindow+i) = 1.0;
		}
		break;		
	}	
}

