 // SgnlPrcsDll.cpp : Defines the initialization routines for the DLL.
//

#include "SgnlPrcsDll.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

typedef long LONG;

////////////////////////// ???????????????? ////////////////////////


std::unordered_map<int, std::string> umap_input_uint8 = {
    {SMT_UNKNOWN, "UNKNOWN" },
    {SMT_NOISE, "NOISE" },
    {SMT_CW, "CW"},
    {SMT_AM, "AM"},
    {SMT_FM, "FM"},
    {SMT_MSK, "MSK" },
    {SMT_GMSK, "GMSK" },
    {SMT_2ASK, "2ASK" },
    {SMT_2FSK, "2FSK" },
    {SMT_4FSK, "4FSK" },
    {SMT_BPSK, "BPSK" },
    {SMT_QPSK, "QPSK" },
    {SMT_8PSK, "8PSK" },
    {SMT_OQPSK, "OQPSK" },
    {SMT_PI4DQPSK, "PI4DQPSK" },
    {SMT_16QAM, "16QAM" },
    {SMT_32QAM, "32QAM" },
    {SMT_64QAM, "64QAM" },
    {SMT_16APSK, "16APSK" },
};

std::unordered_map<std::string, int> umap_input_string = {
    {"UNKNOWN", SMT_UNKNOWN },
    {"NOISE", SMT_NOISE},
    {"CW", SMT_CW},
    {"AM", SMT_AM},
    {"FM", SMT_FM},
    {"MSK", SMT_MSK },
    {"GMSK", SMT_GMSK },
    {"2ASK", SMT_2ASK },
    {"2FSK", SMT_2FSK },
    {"4FSK", SMT_4FSK },
    {"BPSK", SMT_BPSK },
    {"QPSK", SMT_QPSK },
    {"8PSK", SMT_8PSK },
    {"OQPSK", SMT_OQPSK },
    {"PI4DQPSK", SMT_PI4DQPSK },
    {"16QAM", SMT_16QAM },
    {"32QAM", SMT_32QAM },
    {"64QAM", SMT_64QAM },
    {"16APSK", SMT_16APSK },
};





 wchar_t g_pzdllSetupFile[OSA_MAX_PATH];
 wchar_t g_pzdllTmpFilePath[OSA_MAX_PATH];

//wchar_t** const g_pdllSysModuName = g_SysModuName;

short	 g_ndllModuTypeNum = SMT_NUM_MAX;
// short*  const g_pdllSysModuType = g_SysModuType;

short	 g_ndllFHModuNum = SMT_NUM_MAX/2;
// short*  const g_pdllFHModuType = g_SysFHModuType;

short	g_dllDataChanNum = 8;
short	g_dllSignalChanNum = 8;

short  g_dllLPFType = FIRLPFDT_Blackman;
double g_dllStopwidthFactor = 1.25;
double g_dllPassRipple = 0.001;
double g_dllStopRipple = 60;

double g_dllDemodFactor = 1.35;
double g_dllExtractMax = 1024.0;

////////////////////////////////////////////////////////////////////////

//===========================================
 //??????????? //
//void MsgBoxLastErrorShow( DWORD dwErrorID )
//{
//	switch(dwErrorID)
//	{
//	case ERROR_SUCCESS:
//		break;
//	default:
//		{
//			HLOCAL hlocal = NULL;
//
//			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, dwErrorID,
//				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (PTSTR) &hlocal, 0, NULL);
//			if (hlocal != NULL)
//			{
//				AfxMessageBox((PCTSTR) LocalLock(hlocal));
//				LocalFree(hlocal);
//			}
//			else
//			{
//				AfxMessageBox(TEXT("\r\n?????Error number not found. \r\n"));
//			}
//		}
//		break;
//	}
//}

//==========================================================================
// ??????? //
long GetMaxVal(long* X,int N)
{
    long dMaxValue = X[0];
    for(int i=0;i<N;i++)
    {
        dMaxValue = MAX(X[i],dMaxValue);
    }
    return(dMaxValue);
}

double GetMaxVal(double* X,int N)
{
    double dMaxValue = X[0];
    for(int i=0;i<N;i++)
    {
        dMaxValue = MAX(X[i],dMaxValue);
    }
    return(dMaxValue);
}

long GetMaxIndex(double* X, long N, double& dMaxValue)
{
    long lMaxIndex = 0;

    dMaxValue = X[0];
    for(int i=0;i<N;i++)
    {
        if(X[i] > dMaxValue)
        {
            lMaxIndex = i;
            dMaxValue = X[i];
        }
    }
    return(lMaxIndex);
}

//==========================================================================
//  ????��? //
long GetMinVal(long* X,int N)
{
    long dMinValue = X[0];
    for(int i=0;i<N;i++)
    {
        dMinValue = MIN(X[i],dMinValue);
    }
    return(dMinValue);
}

double GetMinVal(double* X,int N)
{
    double dMinValue = X[0];
    for(int i=0;i<N;i++)
    {
        dMinValue = MIN(X[i],dMinValue);
    }
    return(dMinValue);
}


//==========================================================================
// ??????? //
bool SignalNormalize(double* pData,long dwDataLen)
{
    long i = 0;
    ////???????????????////
    double dSignNormal=0.0;
    for(i=0;i<dwDataLen/2;i++)
    {
        dSignNormal += pData[2*i]*pData[2*i]+pData[2*i+1]*pData[2*i+1];
    }

    dSignNormal = sqrt(dSignNormal/dwDataLen);

    if (dSignNormal < DBL_EPSILON)
    {
        printf("warning: Sum of Data is Zero.");

        dSignNormal = 1.0;
    }

    for(i=0;i<dwDataLen;i++)
    {
        pData[i] /= dSignNormal;
    }

    return(true);
}

bool SignalNormalize(long* pData,double* pSigIQ,long dwDataLen)
{
    long i = 0;

    ////???????????????////
    double dSignNormal=0.0;
    for(i=0;i<dwDataLen/2;i++)
    {
        dSignNormal += pData[2*i]*pData[2*i]+pData[2*i+1]*pData[2*i+1];
    }
    dSignNormal = sqrt(dSignNormal/dwDataLen);

    if (dSignNormal < DBL_EPSILON)
    {
        printf("warning: Sum of Data is Zero.");

        dSignNormal = 1.0;
    }

    for(i=0;i<dwDataLen;i++)
    {
        pSigIQ[i] = pData[i]/dSignNormal;
    }

    return(true);
}

//==========================================================================
// ??????????? //
void CalSignalSquare(double *pDataI, double *pDataQ, long lDataLen)
{
    double dDataI = 0.0;
    double dDataQ = 0.0;

    for(long i=0;i<lDataLen;i++)
    {
        dDataI = pDataI[i]*pDataI[i] - pDataQ[i]*pDataQ[i];
        dDataQ = 2*pDataI[i]*pDataQ[i];

        pDataI[i] = dDataI;
        pDataQ[i] = dDataQ;
    }
}

void CalSignalSquare(double *pData, long nDataLen)
{
    double dDataI = 0.0;
    double dDataQ = 0.0;

    for(LONG i=0;i<nDataLen/2;i++)
    {
        dDataI = pData[2*i]*pData[2*i] - pData[2*i+1]*pData[2*i+1];
        dDataQ = 2*pData[2*i]*pData[2*i+1];

        pData[2*i] = dDataI;
        pData[2*i+1] = dDataQ;
    }
}

//==========================================================================
// ????????? //
void CalSingalEnvelope(double* pData,long nDataLen,double* pResult)
{
    for(long i=0;i<nDataLen/2;i++)
    {
        pResult[i] = sqrt(pData[2*i]*pData[2*i]+pData[2*i+1]*pData[2*i+1]);
    }
}
//==========================================================================
// ???????? //
void CalSingalFrequency(double* pData,long nDataLen,double* pResult)
{
    double ti,tr;
    for(long i=0;i<nDataLen/2-1;i++)
    {
        ti = pData[2*i]*pData[2*i+3] - pData[2*i+2]*pData[2*i+1];
        tr = pData[2*i+2]*pData[2*i] + pData[2*i+3]*pData[2*i+1];

        pResult[i] = atan2(ti,tr);
    }
    pResult[nDataLen/2-1]=0.0;
}

//==========================================================================
// ???????�� //
void CalSingalPhase(double* pData,long nDataLen,double* pResult)
{
    for(long i=0;i<nDataLen/2;i++)
    {
        pResult[i] = atan2(pData[2*i+1],pData[2*i]);
    }
}

//==========================================================================
// ????????? //
double GetSignalFreq(double dSignalFreq,DATA_CHANNEL_PARAM* pChanPara,bool bInv)
{
    double dTuneFreq = pChanPara->dTuneFreq;
    double dIntermediateFreq = pChanPara->dIntermediateFreq;
    double dSamplingFreq = pChanPara->dSamplingFreq;
    bool  bInverse = pChanPara->bIsInverse && bInv;

    double dFreq = 0;

    //if (pChanPara == NULL)
    //{
    //	jiaAssert(pChanPara != NULL);
    //	return dFreq;
    //}

    // ??��?????? //
    if (fabs(dTuneFreq) < DBL_EPSILON)
    {
        dTuneFreq = dIntermediateFreq;
    }

    if (pChanPara->bIsComplex)
    {
        dFreq = dTuneFreq + (bInverse ? -1.0 : 1.0)*dSignalFreq;
    }
    else
    {
        int n = static_cast<int>(2*dIntermediateFreq/dSamplingFreq);
        double dInterFreq = 0;

        if (n%2==0)
        {
            dInterFreq = dIntermediateFreq - n*dSamplingFreq/2;
        }
        else
        {
            dInterFreq = dSamplingFreq/2-(dIntermediateFreq - n*dSamplingFreq/2);
        }


        dFreq = dTuneFreq + (bInverse ? -1.0 : 1.0)*(dSignalFreq - dInterFreq);
    }

    return dFreq;
}
//==========================================================================
// ?????????? //
double GetDigitalFreq(double dSignalFreq,DATA_CHANNEL_PARAM* pChanPara,bool bInv)
{
    double dTuneFreq = pChanPara->dTuneFreq;
    double dIntermediateFreq = pChanPara->dIntermediateFreq;
    double dSamplingFreq = pChanPara->dSamplingFreq;
    bool  bInverse = pChanPara->bIsInverse && bInv;

    double dFreq = 0;

    //if (pChanPara == NULL)
    //{
    //	jiaAssert(pChanPara != NULL);
    //	return dFreq;
    //}

    // ??��?????? //
    if (fabs(dTuneFreq) < DBL_EPSILON)
    {
        dTuneFreq = dIntermediateFreq;
    }

    if (pChanPara->bIsComplex)
    {
        dFreq = (bInverse ? -1.0 : 1.0)*(dSignalFreq - dTuneFreq);//
    }
    else
    {
        int n = static_cast<int>(2*dIntermediateFreq/dSamplingFreq);
        double dInterFreq = 0;

        if (n%2==0)
        {
            dInterFreq = dIntermediateFreq - n*dSamplingFreq/2;
        }
        else
        {
            dInterFreq = dSamplingFreq/2-(dIntermediateFreq - n*dSamplingFreq/2);
        }


        dFreq = (bInverse ? -1.0 : 1.0)*(dSignalFreq - dTuneFreq) +  dInterFreq;
    }

    return dFreq;
}


