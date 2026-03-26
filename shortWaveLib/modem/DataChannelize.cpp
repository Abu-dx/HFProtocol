#include "DataChannelize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CDataChannelize::CDataChannelize(void)
{
	m_dSamplingFreq = 0;

	// ZeroMemory(&m_SignChanPara,sizeof(SIGNAL_CHANNEL_PARAM));
	memset(&m_SignChanPara,0,sizeof(SIGNAL_CHANNEL_PARAM));


	m_pSignI = NULL;
	m_pSignQ = NULL;
	m_dwSignLen = 0;

	m_pOutBuf = NULL;
	m_dwOutLen = 0;

	m_dwIQOut = 0;

	m_dCurrentPhase = 0.0;

	m_bInterpolated= false;
	m_pInterFactor = NULL;
	m_pInterFilter = NULL;
	m_nInterFIRnum = 0;

	m_bDecimated   = false;
	m_pDecimFactor = NULL;
	m_pDecimIndex  = NULL;
	m_pDecimFilter = NULL;
	m_nDecimFIRnum = 0;	
}

CDataChannelize::~CDataChannelize(void)
{
	if (NULL != m_pSignI)
	{
		delete []m_pSignI;
		delete []m_pSignQ;

		m_pSignI = NULL;
		m_pSignQ = NULL;
	}

	if (NULL != m_pOutBuf)
	{
		delete []m_pOutBuf;
		m_pOutBuf = NULL;
	}

	if (NULL != m_pInterFactor)
	{
		delete []m_pInterFactor;
		m_pInterFactor = NULL;
	}

	if (NULL != m_pInterFilter)
	{
		for(SHORT i = 0; i < m_nInterFIRnum; i++)
		{
			delete *(m_pInterFilter+i);
		}

		delete []m_pInterFilter;
		m_pInterFilter = NULL;
	}

	if (NULL != m_pDecimFactor)
	{
		delete []m_pDecimFactor;
		delete []m_pDecimIndex;

		m_pDecimFactor = NULL;
		m_pDecimIndex  = NULL;
	}

	if (NULL != m_pDecimFilter)
	{
		for(SHORT i = 0;i < m_nDecimFIRnum; i++)
		{
			delete *(m_pDecimFilter+i);
		}

		delete []m_pDecimFilter;
		m_pDecimFilter = NULL;
	}
}

DWORD CDataChannelize::InitChannelizeParam(double dSamplingFreq, double dSingalFreq, double dPassWidth, double dStopWidth, 
										   double &dOutSamplingFreq,short* pFactor,short nFIRnum)
{
	DWORD dwErrorID = ERROR_SUCCESS;	

	double dPassFreq = .0;
	double dStopFreq = .0;
//	double dFactor = dSamplingFreq/dPassWidth;

	m_bInterpolated = false;
	m_bDecimated    = false;	

	//--------------------------------------------------------
	// 抽取不能超过最大倍数 //
	//if (dFactor > g_dllExtractMax)
	//{
	//	dwErrorID = ERROR_UD_INCORRECT_DDC_PARAMETER;
	//	return(dwErrorID);
	//}		

	//-------------------------------------------------------------
	// 获取重采样方案 //
	if (pFactor != NULL && nFIRnum > 0)
	{
		SHORT nCountI = 0;
		SHORT nCountD = 0;
		for (SHORT n = 0; n < nFIRnum; n++)
		{
			if (pFactor[n] < 0)
			{				
				nCountI++;
			}
			else
			{
				nCountD++;
			}
		}

		dOutSamplingFreq = dSamplingFreq;
		if (nCountI > 0)
		{
			m_bInterpolated = true;
			
			//滤波器组的长度//
			m_nInterFIRnum = nCountI;
			m_pInterFactor = new SHORT[m_nInterFIRnum];

			nCountI = 0;
			for (SHORT n = 0; n < nFIRnum; n++)
			{
				if (pFactor[n] < 0)
				{
					m_pInterFactor[nCountI++] = - pFactor[n];
					dOutSamplingFreq *= - pFactor[n];
				}
			}
		}

		if (nCountD > 0)
		{
			m_bDecimated = true;

			//滤波器组的长度//
			m_nDecimFIRnum = nCountD + 1;
			m_pDecimFactor = new SHORT[m_nDecimFIRnum];	

			nCountD = 0;
			for (SHORT n = 0; n < nFIRnum; n++)
			{
				if (pFactor[n] > 0)
				{
					m_pDecimFactor[nCountD++] = pFactor[n];
					dOutSamplingFreq /= pFactor[n];
				}
			}

			m_pDecimFactor[m_nDecimFIRnum-1 ]= 1;

			m_pDecimIndex  = new DWORD[m_nDecimFIRnum];
			// ZeroMemory(m_pDecimIndex,m_nDecimFIRnum*sizeof(DWORD));
			memset(m_pDecimIndex,0,m_nDecimFIRnum*sizeof(DWORD));

		}
	}
	else
	{
		if( !GetResamplingFactor(dSamplingFreq,dPassWidth,dOutSamplingFreq) )
		{
			dwErrorID = ERROR_UD_INCORRECT_DDC_PARAMETER;
			return(dwErrorID);
		}	
	}
	
	//生成内插滤波器组 //
	if (m_bInterpolated)
	{		
		m_pInterFilter = new CDataFIRDF* [m_nInterFIRnum];

		dPassFreq = dPassWidth/dSamplingFreq;
		dStopFreq = dStopWidth/dSamplingFreq;

		for(SHORT i=0; i<m_nInterFIRnum; i++)
		{			
			dPassFreq /= m_pInterFactor[i];
			dStopFreq /= m_pInterFactor[i];

			m_pInterFilter[i] = new CDataFIRDF;

			if( !m_pInterFilter[i]->IinitialLPFPara(g_dllLPFType,dPassFreq,dStopFreq,g_dllPassRipple,g_dllStopRipple) )
			{
				dwErrorID = ERROR_UD_INCORRECT_DDC_PARAMETER;
				break;
			}			
		}	
	}	
	
	//生成抽取滤波器组 //
	if (m_bDecimated)
	{		
		m_pDecimFilter = new CDataFIRDF* [m_nDecimFIRnum];

		for(SHORT i=0; i<m_nDecimFIRnum; i++)
		{
			dPassFreq = dPassWidth/dOutSamplingFreq;
			dStopFreq = dStopWidth/dOutSamplingFreq;

			m_pDecimFilter[i] = new CDataFIRDF;

			if( !m_pDecimFilter[i]->IinitialLPFPara(g_dllLPFType,dPassFreq,dStopFreq,g_dllPassRipple,g_dllStopRipple) )
			{
				dwErrorID = ERROR_UD_INCORRECT_DDC_PARAMETER;
				break;
			}
		}	
	}

	m_dSamplingFreq = dSamplingFreq;
	m_SignChanPara.dCenterFreq = dSingalFreq;
	m_SignChanPara.dBandwidth = dPassWidth;
	m_SignChanPara.dStopWidth = dStopWidth;
	m_SignChanPara.dOutSamplingFreq = dOutSamplingFreq;

	return(dwErrorID);
}

DWORD CDataChannelize::InputData(LONG* lpData, DWORD dwlen, bool bIsComplex)
{
	DWORD dwErrorID = ERROR_SUCCESS;

	if (m_pDecimFilter == NULL && m_pInterFilter == NULL)
	{
		dwErrorID = ERROR_UD_INCORRECT_DDC_PARAMETER;
		return(dwErrorID);
	}

	SHORT n = 0;
	DWORD i,k,dwIdx,dwCnt = 0;
	DWORD dwIQLen = dwlen / (bIsComplex + 1);
	DWORD dwIQOut = dwIQLen;

	if (m_bInterpolated)
	{
		for(n = 0; n < m_nInterFIRnum; n++)
		{
			dwIQOut *= m_pInterFactor[n];
		}
	}	

	if (dwIQOut > m_dwSignLen && m_pSignI != NULL)
	{
		delete []m_pSignI;
		delete []m_pSignQ;
		m_pSignI = NULL;
		m_pSignQ = NULL;
	}

	if (m_pSignI == NULL)
	{
		m_dwSignLen = dwIQOut;			
		m_pSignI = new double[m_dwSignLen];
		m_pSignQ = new double[m_dwSignLen];
	}	

	//----------------------------------------------------------------
	//// 信号正交变换 ////	
	double	dStep = 2*PI*m_SignChanPara.dCenterFreq / m_dSamplingFreq;	

	// ZeroMemory(m_pSignI,m_dwSignLen*sizeof(double));
	memset(m_pSignI,0,m_dwSignLen*sizeof(double));
	// ZeroMemory(m_pSignQ,m_dwSignLen*sizeof(double));
	memset(m_pSignQ,0,m_dwSignLen*sizeof(double));


	if(bIsComplex) ////中频IQ信号///
	{
		for(i = 0; i < dwIQLen; i++)
		{
			m_dCurrentPhase += dStep;
			while( m_dCurrentPhase > 2*PI ) 
			{
				m_dCurrentPhase -= 2*PI;
			}
			while( m_dCurrentPhase < -2*PI ) 
			{
				m_dCurrentPhase += 2*PI;
            }
			m_pSignI[i] = lpData[2*i]*cos(m_dCurrentPhase) + lpData[2*i+1]*sin(m_dCurrentPhase);
			m_pSignQ[i] = lpData[2*i+1]*cos(m_dCurrentPhase) - lpData[2*i]*sin(m_dCurrentPhase);
		}
	}
	else         ////中频AD信号/////
	{
		for(i = 0; i < dwIQLen; i++)
		{
			m_dCurrentPhase += dStep;
			while( m_dCurrentPhase > 2*PI ) 
			{
				m_dCurrentPhase -= 2*PI;
			}
			while( m_dCurrentPhase < -2*PI ) 
			{
				m_dCurrentPhase += 2*PI;
			}
			m_pSignI[i] = lpData[i]*cos(m_dCurrentPhase);
			m_pSignQ[i] = -lpData[i]*sin(m_dCurrentPhase);
		}
	}

	//----------------------------------------------------------------
	m_dwIQOut = dwIQLen;

	// 先内插 //
	if (m_bInterpolated)
	{
		for(n = 0; n < m_nInterFIRnum; n++)
		{
			for(i = m_dwIQOut - 1; i > 0; i--)
			{
				k = m_pInterFactor[n];

				m_pSignI[k*i] = m_pSignI[i];
				m_pSignQ[k*i] = m_pSignQ[i];
				m_pSignI[i] = .0;
				m_pSignQ[i] = .0;
			}

			m_dwIQOut *= m_pInterFactor[n];
			m_pInterFilter[n]->Filter(m_pSignI,m_pSignQ,m_dwIQOut);
		}
	}

	// 再抽取 //
	if (m_bDecimated)
	{
		for (n = 0; n < m_nDecimFIRnum - 1; n++)
		{
			m_pDecimFilter[n]->Filter(m_pSignI,m_pSignQ,m_dwIQOut);

			dwIdx = 0;
			dwCnt = m_pDecimIndex[n];
			
			while ( dwCnt < m_dwIQOut)
			{
				m_pSignI[dwIdx] = m_pSignI[dwCnt];
				m_pSignQ[dwIdx] = m_pSignQ[dwCnt];

				dwIdx++;

				if (dwCnt + m_pDecimFactor[n] >= m_dwIQOut)
				{
					dwCnt += m_pDecimFactor[n];
					dwCnt -= m_dwIQOut;

					m_pDecimIndex[n] = dwCnt;
					m_dwIQOut = dwIdx;

					break;
				}

				dwCnt += m_pDecimFactor[n];
			}			
		}
		m_pDecimFilter[n]->Filter(m_pSignI,m_pSignQ,m_dwIQOut);
	}	

	return(dwErrorID);
}

double* CDataChannelize::GetSignal(DWORD& dwDataLen)
{
	if (m_pOutBuf != NULL && 2*m_dwIQOut>m_dwOutLen)
	{
		delete []m_pOutBuf;
		m_pOutBuf = NULL;
	}

	if (m_pOutBuf == NULL)
	{
		m_dwOutLen = 2*m_dwIQOut;
		m_pOutBuf = new double[m_dwOutLen];		
	}

	for(DWORD i = 0;i<m_dwIQOut;i++)
	{
		m_pOutBuf[2*i] = m_pSignI[i];
		m_pOutBuf[2*i+1] = m_pSignQ[i];
	}

	dwDataLen = 2*m_dwIQOut;
	
	return(m_pOutBuf);
}

bool CDataChannelize::GetResamplingFactor(double dSamplingFreq, double dPassWidth, double &dOutSamplingFreq)
{
	SHORT n = 0;
	SHORT nCount = 0;
	SHORT nInerval=1;	
	double dFactor = .0;	

	//----------------------------------------
	if (dOutSamplingFreq < FLT_EPSILON) // 自适应输出采样4~8带宽 //
	{
		dFactor = dSamplingFreq/dPassWidth;
		dOutSamplingFreq = dSamplingFreq;

		/// 2倍内插滤波器组 ///
		if (dFactor < RATIO_MIN) //RATIO_MIN = 3.5
		{
			m_bInterpolated = true;

			nCount = 0;
			while(dFactor < RATIO_MIN && nCount < 2)
			{
				nInerval = 2;
				////输出信号采样速率
				dOutSamplingFreq *=  nInerval;
				dFactor = dOutSamplingFreq/dPassWidth;

				nCount++;
			}

			////滤波器组的长度
			m_nInterFIRnum = nCount;
			m_pInterFactor = new SHORT[m_nInterFIRnum];

			for (SHORT n = 0; n < m_nInterFIRnum; n++)
			{
				m_pInterFactor[n] = 2;
			}
		}
		else
		{
			m_bDecimated = true;

			// 设计抽取滤波器组 //
			nCount = 0;
			while(dFactor > RATIO_MAX)
			{			
				nInerval = 2;
				for( n = 2; n <= 7; n++ ) // 2、3、5、7
				{	
					if ((0 == static_cast<int>(dOutSamplingFreq)%n) && (dFactor/n > RATIO_MIN))
					{
						nInerval = n;
						break;	
					}				
				}

				nCount ++;
				dOutSamplingFreq /= nInerval;				
				dFactor = dOutSamplingFreq/dPassWidth;	
			}

			////滤波器组的长度
			m_nDecimFIRnum = nCount + 1;
			m_pDecimFactor = new SHORT[m_nDecimFIRnum];		

			// 保存抽取因子 //
			nCount = 0;
			dOutSamplingFreq = dSamplingFreq;	
			dFactor = dSamplingFreq/dPassWidth;
			while(dFactor > RATIO_MAX)
			{			
				nInerval = 2;
				for( n = 2; n <= 7; n++ ) // 2、3、5、7
				{	
					if ((0 == static_cast<int>(dOutSamplingFreq)%n) && (dFactor/n > RATIO_MIN))
					{
						nInerval = n;
						break;	
					}				
				}

				m_pDecimFactor[nCount] = nInerval;

				nCount ++;

				dOutSamplingFreq /= nInerval;				
				dFactor = dOutSamplingFreq/dPassWidth;	

				// 抽取不能超过最大倍数 //
				if (dSamplingFreq/dOutSamplingFreq > g_dllExtractMax)
				{
					break;
				}
			}

			m_pDecimFactor[m_nDecimFIRnum-1 ]= 1;

			m_pDecimIndex  = new DWORD[m_nDecimFIRnum];
			// ZeroMemory(m_pDecimIndex,m_nDecimFIRnum*sizeof(DWORD));
			memset(m_pDecimIndex,0,m_nDecimFIRnum*sizeof(DWORD));

		}
	}
	else	// 特定输出采样 dOutSamplingFreq ~ 2*dOutSamplingFreq//
	{
		dFactor = log10(dSamplingFreq/dOutSamplingFreq)/log10(2.0);

		m_bDecimated = true;

		// 滤波器组的长度 //
		m_nDecimFIRnum = SHORT(dFactor) + 1;
		m_pDecimFactor = new SHORT[m_nDecimFIRnum];	
		// ZeroMemory(m_pDecimFactor,m_nDecimFIRnum*sizeof(SHORT));
		memset(m_pDecimFactor,0,m_nDecimFIRnum*sizeof(SHORT));


		for (n = 0; n < m_nDecimFIRnum - 1; n++)
		{
			m_pDecimFactor[n] = 2;
		}
		m_pDecimFactor[m_nDecimFIRnum - 1 ]= 1;

		m_pDecimIndex  = new DWORD[m_nDecimFIRnum];
		// ZeroMemory(m_pDecimIndex,m_nDecimFIRnum*sizeof(DWORD));
		memset(m_pDecimIndex,0,m_nDecimFIRnum*sizeof(DWORD));


		dOutSamplingFreq = dSamplingFreq/pow(2.0,SHORT(dFactor));		
	}	
	
	return true;
}
