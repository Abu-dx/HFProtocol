#pragma once
#include "commlib-config.h"
#include "SgnlPrcsDll.h"

class CSpectrumProbe
{
public:
	CSpectrumProbe(void);
	~CSpectrumProbe(void);

	uint32_t InitSpectrumProbeParam(
		uint32_t dwFFTSize,
		uint32_t dwFFTMove,
		uint16_t nAvgNum,
		uint16_t nWinType,
		bool  bIsComplex,
		bool  bInverse,
		bool bIslog);

	uint32_t InputData(long* lpInData, uint32_t dwInlen);

	uint32_t InputData(double* lpInData, uint32_t dwInlen);

	double* GetSpectrum(DWORD& dwLen);

protected:

	void ComputeWindow();

protected:
	// 数据通道频谱探测参数
	SPECTRUM_PROBE_PARAM m_SpecPara;

	bool m_bIsComplex;
	bool m_bIsInverse;
	bool m_bIslog;

	double* m_pAvgSpec;
	uint32_t   m_dwSpecLen;

	// 输入信号
	double* m_pDataBuff;
	uint32_t	m_dwDataLen;
	uint32_t   m_dwInputLen;

	fftw_complex* m_pFFTin;
	fftw_complex* m_pFFTout;

	fftw_plan m_pFFT;

	int16_t m_nFFTWin;

	double* m_pWindow;

};
