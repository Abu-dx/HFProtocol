
#include "stdafx.h"
#include "Link11CLEW.h"
#include <math.h>
#include "Preamble.h"
#include <malloc.h>                                                           

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLink11CLEW::CLink11CLEW(void)
{

}

CLink11CLEW::~CLink11CLEW(void)
{
}
//  包含下变频，匹配滤波，重采样
//原始滤波器阶数tapsLen		多相分解后滤波阶数taps_onech   
void CLink11CLEW::ReSample_ini(Ipp32f rFreqHz,Ipp32s nLeng,Ipp32f insample,Ipp32f outsample,int incimate,int tapsLen)
{

	m_tapsLen = tapsLen;
	m_incimate = incimate;
	m_taps_onech = tapsLen/incimate;  //  整除
	m_Decimate =Ipp64f(insample*incimate)/outsample;

	m_rFreq=rFreqHz/insample;
	m_Phase=0;
	m_Phase2=m_Phase+IPP_PI/2;
	m_nLeng=nLeng;

	pSin = ippsMalloc_32f(m_nLeng);   //   正弦函数初始化
	pCos = ippsMalloc_32f(m_nLeng); 

	xr=ippsMalloc_32f(m_nLeng);
	xi=ippsMalloc_32f(m_nLeng);

	Ipp64f *taps_root;                  // 生成重采样滤波器
	m_fstop=1500/(insample*incimate);
	taps_root = ippsMalloc_64f(m_tapsLen);
	ippsFIRGenLowpass_64f(m_fstop, taps_root, m_tapsLen, ippWinHamming, ippTrue);	//	窗化法生成低通滤波器

	ReSample_taps=new float*[m_incimate];
	int i, j;

	for(j=0; j<m_incimate; j++)
	{
		ReSample_taps[j]=new float[m_taps_onech];
		for(i=0; i<m_taps_onech; i++)
		{
			ReSample_taps[j][i]=(Ipp32f)taps_root[i*m_incimate+j]*20;
		}
	}

	pDlyLineR=ippsMalloc_32f(m_taps_onech);
	ippsSet_32f(0,pDlyLineR,m_taps_onech);
	pDlyLineI=ippsMalloc_32f(m_taps_onech);
	ippsSet_32f(0,pDlyLineI,m_taps_onech);

	history=0;
	faddress=0;
	address=0;
	ippsFree(taps_root);

}
int  CLink11CLEW::ReSample(Ipp16s *pSrc,Ipp32fc *pDst)
{
	int i,m=0;
	Ipp32f *pSrc32f = ippsMalloc_32f(m_nLeng);
	ippsConvert_16s32f(pSrc,pSrc32f,m_nLeng);

	ippsTone_Direct_32f(pCos, m_nLeng, 1, m_rFreq, &m_Phase, ippAlgHintFast);
	ippsTone_Direct_32f(pSin, m_nLeng, 1, m_rFreq, &m_Phase2, ippAlgHintFast);

	ippsMul_32f(pCos, pSrc32f, xr, m_nLeng);
	ippsMul_32f(pSin, pSrc32f, xi, m_nLeng);

	Ipp32s len=m_nLeng+history;
	Ipp32f *tempR,*tempI;
	tempR=ippsMalloc_32f(len);
	tempI=ippsMalloc_32f(len);

	ippsCopy_32f(pDlyLineR,tempR,history);
	ippsCopy_32f(xr,&tempR[history],m_nLeng);

	ippsCopy_32f(pDlyLineI,tempI,history);
	ippsCopy_32f(xi,&tempI[history],m_nLeng);

	for (i=0;i<len-m_taps_onech;i++)
	{	
		while(address>=0)
		{	
			ippsDotProd_32f(ReSample_taps[address], &tempR[i], m_taps_onech, &pDst[m].re);
			ippsDotProd_32f(ReSample_taps[address], &tempI[i], m_taps_onech, &pDst[m].im);
			faddress=faddress-m_Decimate;
			address=int(faddress+0.5);
			m++;
		}
		faddress=faddress+m_incimate;
		address=int(faddress+0.5);
	}

	history=len-i;
	ippsCopy_32f(&tempR[i],pDlyLineR,history);
	ippsCopy_32f(&tempI[i],pDlyLineI,history);
	ippsFree(tempR);ippsFree(tempI);
	ippsFree(pSrc32f);

	return m;
}
void CLink11CLEW::ReSample_Free()
{
	ippsFree(pSin);
	ippsFree(pCos);
	ippsFree(xr);
	ippsFree(xi);
	ippsFree(pDlyLineR);
	ippsFree(pDlyLineI);
	for (int i=0;i<m_incimate;i++)
	{
		//ippsFree(ReSample_taps);
		delete ReSample_taps[i];
	}
	delete ReSample_taps;
}
void CLink11CLEW::UpdateFc(Ipp32f rFreqHz,Ipp32s nLeng,Ipp32f insample)
{
	m_rFreq=rFreqHz/insample;
	m_Phase=0;
	m_Phase2=m_Phase+IPP_PI/2;
	m_nLeng=nLeng;

	pSin = ippsMalloc_32f(m_nLeng);   //   正弦函数初始化
	pCos = ippsMalloc_32f(m_nLeng); 
}
void CLink11CLEW::Link11CLEWdemode_ini(Ipp32f rFreqHz,Ipp32s nLeng,Ipp32f Insample,Ipp32f Outsample,int tapsLen)
{
	int i;
	OFDM_fs = 7040.0;
	OFDM_fc = 1705.0;
	OFDM_fft = 64;
	OFDM_fftbit = 6;
	OFDM_cpxL = 30;
	OFDM_symL = OFDM_fft + OFDM_cpxL;
	bufOFDMFFT = ippsMalloc_32fc(OFDM_fft);
	ippsFFTInitAlloc_C_32fc(&pOFDMFFTSpec, OFDM_fftbit, IPP_FFT_DIV_FWD_BY_N, ippAlgHintAccurate);
	// 
	OFDM_useNum=16;
	OFDM_useIdx=ippsMalloc_16s(OFDM_useNum);
	OFDM_usef = ippsMalloc_32f(OFDM_useNum);     
	for (i=1;i<8;i++)
	{
		OFDM_useIdx[i]=7-i;
		OFDM_usef[i]=(float)(OFDM_useIdx[i]*110)/OFDM_fs;
		OFDM_useIdx[i+7]=64-i;
		OFDM_usef[i+7]=(float)(-i*110)/OFDM_fs;
	}
	OFDM_useIdx[0]=11;
	OFDM_useIdx[15]=54;

	OFDM_usef[0]=(float)(2915-1705)/OFDM_fs;
	OFDM_usef[15]=(float)(605-1705)/OFDM_fs;

	// 256点fft检测
	detect_fft = 256;
	bufDetectFFT = ippsMalloc_32fc(detect_fft);
	idx_dopule = floor(float(54*110)/7040*detect_fft);
	idx_16 = floor(float(11*110)/7040*detect_fft);
	idx_data1 = floor(float(57*110)/7040*detect_fft);
	idx_data14 = floor(float(6*110)/7040*detect_fft);	
	delt = floor(float(110)/7040*detect_fft);
	ippsFFTInitAlloc_C_32fc(&pDetectFFTSpec, 8, IPP_FFT_DIV_FWD_BY_N, ippAlgHintAccurate);

	// 多普勒估计
	fduopule1 = 0;
	fduopule2 = 0;
	duopulefft = 4096;
	bufduopuleFFT = ippsMalloc_32fc(duopulefft);
	ippsFFTInitAlloc_C_32fc(&pduopuleFFTSpec,12, IPP_FFT_DIV_FWD_BY_N, ippAlgHintAccurate);
	frephase=0;
	frephase2=0;
	
	// 定时同步
	pfirlen = 64;
	timefir = ippsMalloc_64f(pfirlen);
	ippsFIRGenLowpass_64f(0.1,timefir,pfirlen,ippWinHamming,ippTrue);

	ReSample_ini(rFreqHz,nLeng,Insample,Outsample,8,tapsLen);
	signalbuf = ippsMalloc_32fc(20*OFDM_symL);
	signalp = 0;
	pilotp = 0;
	timeidx=0;
	flag = 0;
	pData = 0;

	phaseref = ippsMalloc_32f(OFDM_useNum);

	beginpos=0;
	endpos=0;
	countpos=0;
	m_burstnum=0;
	int posn = nLeng/OFDM_symL;
	m_burstpos = new burstpos[posn];
	for (i=0;i<posn;i++)
	{
		m_burstpos[i].m_complete=2;
	}
	SaveToBit_ini();

}
void CLink11CLEW::Link11CLEWdemode_free()
{
	ippsFree(bufDetectFFT);
	ippsFFTFree_C_32fc(pDetectFFTSpec);
	ippsFree(bufduopuleFFT);
	ippsFFTFree_C_32fc(pduopuleFFTSpec);
	ippsFree(bufOFDMFFT);
	ippsFFTFree_C_32fc(pOFDMFFTSpec);
	ippsFree(OFDM_useIdx);
	ippsFree(OFDM_usef);
	ippsFree(phaseref);
	ippsFree(signalbuf);
	ippsFree(timefir);

	ReSample_Free();

	//free(sambufI);
	//free(sambufQ);
	free(m_burstpos);

}
void CLink11CLEW::Link11CLEWdemode(Ipp16s *pSrc,int pSrcLen,Ipp32f TH1,Ipp8u *mByte,int &bytelen,Ipp32fc *expsym,int &expsymlen,int &burstnum,bool &detect)
{
	int i,j,ResamN;
	Ipp32fc *pBufresample = ippsMalloc_32fc(2*pSrcLen);
	ResamN = ReSample(pSrc,pBufresample);
	bool bdetect = false;

	Ipp32f decsum,mfre;
	Ipp16s m_codenum=0,m_codenumcur,m_expsymlen=0,err;
	Ipp32f pilotenergy,m_energy,sigenergy;
	Ipp16s refbegin,refend;

	int num=0,pos=0,m_address;
	bool findref=0;

	Ipp32fc *OFDM_fftsym;
	OFDM_fftsym = ippsMalloc_32fc(OFDM_useNum);
	Ipp32s *codeout = ippsMalloc_32s(ResamN/OFDM_symL);

	if (m_burstpos[m_burstnum].m_complete==0)
	{
		m_burstpos[0].m_begin = m_burstpos[m_burstnum].m_begin;
		m_burstpos[0].m_complete = m_burstpos[m_burstnum].m_complete;
		m_burstpos[0].m_address = m_burstpos[m_burstnum].m_address;
		m_burstpos[0].m_end = m_burstpos[m_burstnum].m_end;
		m_burstpos[0].m_type = m_burstpos[m_burstnum].m_type;
	}
	m_burstnum = 0;
	pData = 0;

	for (i=0;i<ResamN;i++)
	{
		signalbuf[signalp].re = pBufresample[i].re;	
		signalbuf[signalp].im = pBufresample[i].im;	
		signalp++;

		if(flag==0)//信号检测
		{		
			if ((signalp%OFDM_symL)==0)
			{
				fftdetect(&signalbuf[signalp-OFDM_symL],OFDM_symL,&mfre,&decsum);
				if (decsum>TH1)
				{
					pilotp++;
					if (pilotp==2) // 确保有连续2个
					{
						flag = 1;
						bdetect = TRUE;
						fduopule1 = mfre;
						frephase2 = 0;
					}
				}
				else
				{
					pilotp = 0;
					signalp = 0;
					flag = 0;
					energy = 0;
					pData = pData + OFDM_symL;
				}
			}
		}
		if (flag==1 && (signalp%OFDM_symL)==0 && signalp==10*OFDM_symL)
		{
			duopule_estimate(&signalbuf[OFDM_cpxL],4*OFDM_symL,fduopule1,&fduopule2);
			//	Remove_OFDM_fre(signalbuf,signalp,fduopule2);
			Timing_estimate(signalbuf,&timeidx);
			timeidx = timeidx + OFDM_cpxL;
			num = (10*OFDM_symL-timeidx)/OFDM_symL;

			Remove_OFDM_fre(signalbuf,timeidx+num*OFDM_symL,fduopule2);
			//// 
			pilotenergy=0;
			for (j=0;j<2;j++)
			{
				FFT_OFDM(&signalbuf[timeidx+j*OFDM_symL],OFDM_fftsym,&m_energy);
				pilotenergy = pilotenergy+m_energy/2;	
			}
			sigenergy = 0;findref=0;pilotp=0;
			for (j=2;j<=num-3;j++) // 寻找参考帧
			{
				FFT_OFDM(&signalbuf[timeidx+j*OFDM_symL],OFDM_fftsym,&m_energy);
				if (m_energy>pilotenergy*2.5 && findref ==0)  // 找到参考帧
				{
					sigenergy = m_energy/3;
					findref=1;
					ippsPhase_32fc(OFDM_fftsym,phaseref,OFDM_useNum);
					pilotp = j;
					break;
				}
				else if(findref==0) // 
				{
					pilotenergy = pilotenergy*j/(j+1)+m_energy/(j+1);
				}
			}
			if(findref)
			{
				refbegin = pilotp;
				refend = num;
			}
			else
			{
				refbegin = 3;
				refend = num;
				FFT_OFDM(&signalbuf[timeidx+refbegin*OFDM_symL],OFDM_fftsym,&m_energy);
				ippsPhase_32fc(OFDM_fftsym,phaseref,OFDM_useNum);
				sigenergy = m_energy/3;
			}

			beginpos = pData + timeidx;
			m_burstpos[m_burstnum].m_begin =  beginpos;  //  countpos
			m_burstpos[m_burstnum].m_complete = 0; 
			m_burstpos[m_burstnum].m_type = 0;//未知
			m_burstpos[m_burstnum].m_address =0;//未知

			signalp=(10*OFDM_symL-timeidx) - num*OFDM_symL ;
			flag=2;	pos = 1;
			for (j=refbegin+1;j<refbegin+3;j++) // 两个地址帧
			{
				FFT_OFDM(&signalbuf[timeidx+j*OFDM_symL],OFDM_fftsym,&m_energy);
				sigenergy = sigenergy + m_energy/3;

				Dec_demode(OFDM_fftsym,&codeout[m_codenum],pos,&expsym[m_expsymlen]);
				m_codenum++;
				m_expsymlen = m_expsymlen + OFDM_useNum-1;
				pos = 0;
			}
			DeAddress(&codeout[m_codenum-2],2,&m_address); // 地址译码
			for (j=refbegin+3;j<num;j++) // 后续数据帧
			{
				FFT_OFDM(&signalbuf[timeidx+j*OFDM_symL],OFDM_fftsym,&m_energy);
				if (m_energy*2<sigenergy)
				{
					flag=0;
					signalp=0;
					refend = j;
					pilotp = 0;
					break;
				}
				else
				{
					Dec_demode(OFDM_fftsym,&codeout[m_codenum],pos,&expsym[m_expsymlen]);
					m_codenum++;
					m_expsymlen = m_expsymlen + OFDM_useNum-1;
					Decode(&codeout[m_codenum-1],&err); // 数据译码
				}
			}
			if(refend==num)
				pData = pData + timeidx + num*OFDM_symL;
			else
			{
				m_burstpos[m_burstnum].m_end=   pData + timeidx + refend*OFDM_symL; //countpos
				m_burstpos[m_burstnum].m_address = m_address;
				m_burstpos[m_burstnum].m_type = 1;//主站轮询
				m_burstpos[m_burstnum].m_complete=1;
				m_burstpos[m_burstnum].m_fduopule = fduopule2*OFDM_fs+605;
				m_burstnum++;
				pData = pData + 10*OFDM_symL;
			}
			
			energy = sigenergy;
			ippsCopy_32fc(&signalbuf[timeidx+num*OFDM_symL],signalbuf,signalp);	
			bdetect = TRUE;
		}
		if (flag==2 && signalp==OFDM_symL)
		{
			Remove_OFDM_fre(signalbuf,signalp,fduopule2);

			fftdetect(signalbuf,OFDM_symL,&mfre,&decsum);
			FFT_OFDM(signalbuf,OFDM_fftsym,&m_energy);

			if (m_energy*2<energy || decsum>TH1)
			{
				flag = 0;
				pilotp = 0;
				m_burstpos[m_burstnum].m_end= pData;  //countpos
				m_burstpos[m_burstnum].m_complete=1;
				m_burstnum++;
			}
			else
			{
				bdetect = TRUE;
				Dec_demode(OFDM_fftsym,&codeout[m_codenum],pos,&expsym[m_expsymlen]);
				m_codenum++;
				m_expsymlen = m_expsymlen + OFDM_useNum-1;
				if (Islike(&codeout[m_codenum-1],&controlstop))
				{
					m_burstpos[m_burstnum].m_type = 2;// 主站报告
				}
				else if (Islike(&codeout[m_codenum-1],&packetstop))
				{
					m_burstpos[m_burstnum].m_type = 3;// 从站回复
				}
				else
					Decode(&codeout[m_codenum-1],&err);

				// 译地址
				if (m_burstpos[m_burstnum].m_type ==2)
				{
					DeAddress(&codeout[m_codenum-1],1,&m_address);
					if (m_address!=0)
					{
						m_burstpos[m_burstnum].m_address=m_address;
					}
				}
			}
			if(decsum>TH1)
				pilotp++;
			signalp=0;
			pData = pData + OFDM_symL;
		}
	}
	SaveToBit(codeout,m_codenum,mByte,bytelen);

	detect = bdetect;
//	countpos = countpos+ResamN;	
	expsymlen = m_expsymlen;
	burstnum = m_burstnum;
	ippsFree(OFDM_fftsym);
	ippsFree(pBufresample);
	
}
void CLink11CLEW::Link11CLEWdetect(Ipp16s *pSrc,int pSrcLen,Ipp32fc *detectout,int &detectoutlen)
{
	int i,j,ResamN;
	Ipp32fc *pBufresample = ippsMalloc_32fc(pSrcLen);
	ResamN = ReSample(pSrc,pBufresample);

	Ipp32f decsum,mfre;
	Ipp32f pilotenergy,m_energy,sigenergy;
	Ipp16s m_detectoutlen=0;
	Ipp16s refbegin,refend;

	int num=0,pos=0,m_address;
	bool findref=0;

	Ipp32fc *OFDM_fftsym;
	OFDM_fftsym = ippsMalloc_32fc(OFDM_useNum);

	for (i=0;i<ResamN;i++)
	{
		signalbuf[signalp].re = pBufresample[i].re;	
		signalbuf[signalp].im = pBufresample[i].im;	
		signalp++;
		if(flag==0)//信号检测
		{
			OFDM_fc = 1705;
			if ((signalp%OFDM_symL)==0)
			{
				fftdetect(&signalbuf[signalp-OFDM_symL],OFDM_symL,&mfre,&decsum);
				if (decsum>1.1)
				{
					pilotp++;
					if (pilotp==2) // 确保有连续2个
					{
						flag = 1;
						fduopule1 = mfre;
						frephase2 = 0;
					}
				}
				else
				{
					pilotp = 0;
					signalp = 0;
					flag = 0;
					energy = 0;
				}
			}
		}
		if (flag==1 && (signalp%OFDM_symL)==0 && signalp==10*OFDM_symL)
		{
			duopule_estimate(&signalbuf[OFDM_cpxL],4*OFDM_symL,fduopule1,&fduopule2);
		//	Remove_OFDM_fre(signalbuf,signalp,fduopule2);
			Timing_estimate(signalbuf,&timeidx);
			timeidx = timeidx + OFDM_cpxL;
			num = (10*OFDM_symL-timeidx)/OFDM_symL;

			Remove_OFDM_fre(signalbuf,timeidx+num*OFDM_symL,fduopule2);

			////////
			pilotenergy=0;
			for (j=0;j<2;j++)
			{
				FFT_OFDM(&signalbuf[timeidx+j*OFDM_symL],OFDM_fftsym,&m_energy);
				pilotenergy = pilotenergy+m_energy/2;	
			}
			sigenergy = 0;findref=0;pilotp=0;
			for (j=2;j<=num-3;j++)//寻找参考帧
			{
				FFT_OFDM(&signalbuf[timeidx+j*OFDM_symL],OFDM_fftsym,&m_energy);
				if (m_energy>pilotenergy*2 && findref ==0)  //找到参考帧
				{
					sigenergy = m_energy/3;
					findref=1;
					ippsPhase_32fc(OFDM_fftsym,phaseref,OFDM_useNum);
					pilotp = j;
					break;
				}
				else if(findref==0) // 
				{
					pilotenergy = pilotenergy*j/(j+1)+m_energy/(j+1);
				}
			}
			if(findref)
			{
				refbegin = pilotp;
				refend = num;
			}
			else
			{
				refbegin = 3;
				refend = num;
				FFT_OFDM(&signalbuf[timeidx+refbegin*OFDM_symL],OFDM_fftsym,&m_energy);
				ippsPhase_32fc(OFDM_fftsym,phaseref,OFDM_useNum);
				sigenergy = m_energy/3;
			}
			signalp=(10*OFDM_symL-timeidx) - num*OFDM_symL ;
			flag=2;

			for (j=refbegin+1;j<refbegin+3;j++)
			{
				FFT_OFDM(&signalbuf[timeidx+j*OFDM_symL],OFDM_fftsym,&m_energy);
				sigenergy = sigenergy + m_energy/3;
			}
			for (j=refbegin+3;j<num;j++)
			{
				FFT_OFDM(&signalbuf[timeidx+j*OFDM_symL],OFDM_fftsym,&m_energy);
				if (m_energy*2<sigenergy)
				{
					flag=0;
					signalp=0;
					refend = j;
					pilotp = 0;
					break;
				}
			}

			energy = sigenergy;

			signalbuf[timeidx].re = 20000;
			signalbuf[timeidx].im = 20000;
			signalbuf[timeidx+refbegin*OFDM_symL].re = 30000;
			signalbuf[timeidx+refbegin*OFDM_symL].im = 30000;

			ippsCopy_32fc(&signalbuf[timeidx],&detectout[m_detectoutlen],(refend)*OFDM_symL);
			m_detectoutlen = m_detectoutlen + (refend)*OFDM_symL;

			ippsCopy_32fc(&signalbuf[timeidx+num*OFDM_symL],signalbuf,signalp);	

		}
		if (flag==2 && signalp==OFDM_symL)
		{
			Remove_OFDM_fre(signalbuf,signalp,fduopule2);
			
			fftdetect(signalbuf,OFDM_symL,&mfre,&decsum);
			FFT_OFDM(signalbuf,OFDM_fftsym,&m_energy);

			if (m_energy*2<energy || decsum>1.1)
			{
				flag = 0;
				pilotp = 0;
			}
			else
			{
				ippsCopy_32fc(signalbuf,&detectout[m_detectoutlen],signalp);
				m_detectoutlen = m_detectoutlen + signalp;
			}
			if(decsum>1.1)
				pilotp++;
			signalp=0;
		}
	}
	detectoutlen = m_detectoutlen;
	ippsFree(OFDM_fftsym);
	ippsFree(pBufresample);

}
void CLink11CLEW::fftdetect(Ipp32fc *pSrc,Ipp16s symL,Ipp32f *fdopule,Ipp32f *decsum)
{
	short i,j;
	float suma,sumb,sumc;
	Ipp32f *fftDst,maxf,*decs;
	decs = ippsMalloc_32f(8);
	int pIndex;

	fftDst = ippsMalloc_32f(detect_fft);

	ippsZero_32fc(bufDetectFFT,detect_fft);
	ippsCopy_32fc(pSrc,bufDetectFFT,symL);
	ippsFFTFwd_CToC_32fc_I(bufDetectFFT,pDetectFFTSpec,NULL);
	ippsMagnitude_32fc(bufDetectFFT,fftDst,detect_fft);

	//	ippsMaxIndx_32f(fftDst,detect_fft,&maxf,&pIndex);
	//	*fdopule =(float)(pIndex-detect_fft)/detect_fft-OFDM_usef[15];
	// 幅度比值
	for (i=-4;i<4;i++)
	{
		suma = 0;sumb = 0;sumc = 0;
		for (j= idx_dopule+i*delt-delt/2;j<idx_dopule+i*delt+delt/2;j++)
			suma = suma + fftDst[j]/detect_fft;
		for(j= idx_16+i*delt-delt/2;j<idx_16+i*delt+delt/2;j++)
			sumb = sumb + fftDst[j]/detect_fft;
		for(j= idx_data1+i*delt-delt/2;j<detect_fft;j++)
			sumc = sumc + fftDst[j]/detect_fft;
		for(j= 0;j<idx_data14+i*delt+delt/2;j++)
			sumc = sumc + fftDst[j]/detect_fft;
		decs[i+4] = (suma+sumb)/sumc;
	}
	ippsMaxIndx_32f(decs,8,decsum,&pIndex);

	*fdopule =(float)(idx_dopule+pIndex-4-detect_fft)/detect_fft;

	ippsFree(decs);
	ippsFree(fftDst);

}
// 利用校正音进行多普勒频率细估计
void CLink11CLEW::duopule_estimate(Ipp32fc *pSrc,Ipp16s pSrcLen,Ipp32f f1,Ipp32f *fdopule)
{

	Ipp32f *fftDst,maxf,f,f2;
	int pIndex,i,j,idx=0;
	fftDst = ippsMalloc_32f(duopulefft);
	ippsZero_32fc(bufduopuleFFT,duopulefft);
	ippsCopy_32fc(pSrc,bufduopuleFFT,pSrcLen);
	ippsFFTFwd_CToC_32fc_I(bufduopuleFFT,pduopuleFFTSpec,NULL);
	ippsMagnitude_32fc(bufduopuleFFT,fftDst,duopulefft);

	idx = int(f1*duopulefft+duopulefft);
	ippsMaxIndx_32f(&fftDst[idx-50],100,&maxf,&pIndex);
	
	f =(float)(idx+pIndex-50-duopulefft)/duopulefft;

	Ipp32f sumx,xr,xi;
	maxf = 0;pIndex=0;
	for (i=-10;i<10;i++)
	{
		f2 = f+0.1*i/7040;
		sumx = 0;
		for (j=1;j<pSrcLen;j++)
		{
			xr = pSrc[j].re*cos(IPP_2PI*f2*j)+pSrc[j].im*sin(IPP_2PI*f2*j);
			xi = pSrc[j].im*cos(IPP_2PI*f2*j)-pSrc[j].re*sin(IPP_2PI*f2*j);
			sumx = sumx +(xr*xr + xi*xi)/pSrcLen ;
		}
		if(sumx>maxf)
		{
			maxf = sumx;
			pIndex = i;
		}
	}
	*fdopule = f + (0.1*pIndex)/7040-OFDM_usef[15];

	ippsFree(fftDst);

}

////////////////////////////////////////////////////////////////
//  有限冲激响应滤波器，完成滤波：输入一个样点，输出一个样点  //
////////////////////////////////////////////////////////////////
float CLink11CLEW::do_fir(float *base, int cur, double *coeff, int len)
{
	int c=0, i;
	float sum=0;
	for(i=cur; i<len; i++,c++) sum+=coeff[c]*base[i];
	for(i=0; i<cur; i++,c++) sum+=coeff[c]*base[i];
	return sum;
}
void CLink11CLEW::Remove_OFDM_fre(Ipp32fc *Buf_32fc,Ipp16s Buf_len,Ipp32f frequency)
{
	Ipp16s i;
	Ipp32f temp;
	for (i=0;i<Buf_len;i++)
	{
		frephase2=frephase2-IPP_2PI*frequency;
		if (frephase2>=IPP_2PI || frephase2<=-IPP_2PI)
			frephase2=fmod((double)frephase2,(double)IPP_2PI);

		temp=Buf_32fc[i].re;
		Buf_32fc[i].re=Buf_32fc[i].re*cos(frephase2)-Buf_32fc[i].im*sin(frephase2);
		Buf_32fc[i].im=Buf_32fc[i].im*cos(frephase2)+temp*sin(frephase2);
	}
}
//pSrcLen>2*OFDM_symL+pfirlen
void CLink11CLEW::Timing_estimate(Ipp32fc *pSrc,Ipp16s *idx)
{
	int i,j,nleng=pfirlen+2*OFDM_symL-1;
	int m_pidx=0,buffull=0;
	float *sumbufI = new float[pfirlen];
	float *sumbufQ = new float[pfirlen];

	float *xr = new float[2*OFDM_symL];
	float *xi = new float[2*OFDM_symL];
	float maxf;
	int maxidx=0;
	float sumr,sumi,abssum;
	float f = 1210.0;

	j = 0;
	for (i=0;i<nleng; i++)
	{
		sumbufI[m_pidx]=pSrc[i].re*cos(IPP_2PI*f/OFDM_fs*i)+pSrc[i].im*sin(IPP_2PI*f/OFDM_fs*i);
		sumbufQ[m_pidx]=pSrc[i].im*cos(IPP_2PI*f/OFDM_fs*i)-pSrc[i].re*sin(IPP_2PI*f/OFDM_fs*i);
		if(m_pidx==pfirlen-1)
			buffull=1;
		m_pidx = (m_pidx+1)%pfirlen;

		if(buffull==1)
		{
			xr[j] = do_fir(sumbufI, m_pidx, timefir, pfirlen);
			xi[j] = do_fir(sumbufQ, m_pidx, timefir, pfirlen);
			j++;
		}
	}
	maxf = 0;
	for (i=0;i<OFDM_symL;i++)
	{
		sumr = 0;
		sumi = 0;
		for (j=0;j<OFDM_symL;j++)
		{
			sumr = sumr + xr[i+j]/OFDM_symL;
			sumi = sumi + xi[i+j]/OFDM_symL;
		}
		abssum = sumr*sumr+sumi*sumi;
		if (abssum>maxf)
		{
			maxf = abssum;
			maxidx = i;
		}
	}
	*idx = maxidx;
	free(xr);free(xi);
	free(sumbufI);free(sumbufQ);

}
void CLink11CLEW::FFT_OFDM(Ipp32fc *Buf_32fc,Ipp32fc *OFDM_fftsym,Ipp32f *energy)
{
	Ipp32f m_energya=0;
	ippsFFTFwd_CToC_32fc(&Buf_32fc[15],bufOFDMFFT,pOFDMFFTSpec, NULL);
	for (int i=1;i<OFDM_useNum-1;i++)
	{
		OFDM_fftsym[i]=bufOFDMFFT[OFDM_useIdx[i]];
		m_energya = m_energya + sqrt(OFDM_fftsym[i].re*OFDM_fftsym[i].re+OFDM_fftsym[i].im*OFDM_fftsym[i].im)/(OFDM_useNum-2);
		//m_energya = m_energya + sqrt(OFDM_fftsym[i].re*OFDM_fftsym[i].re+OFDM_fftsym[i].im*OFDM_fftsym[i].im)/OFDM_fft;
	}
	OFDM_fftsym[0]=bufOFDMFFT[OFDM_useIdx[0]];
	OFDM_fftsym[OFDM_useNum-1] = bufOFDMFFT[OFDM_useIdx[OFDM_useNum-1]];

	*energy = m_energya;

}
/************************************************************************/
/* M=1,2,3 对应BPSK、QPSK、8PSK                                         */
/************************************************************************/
//theta_e=1/2*angle(mean((abs(yfft_sym(k,:)).^2).*exp(1i*angle(yfft_sym(k,:))*2)));  %  VV算法 
void CLink11CLEW::OFDM_phase_VV(Ipp32f *OFDM_phase,Ipp32f *OFDM_ref,Ipp32f *decphase,Ipp16s nLeng,Ipp16s M)
{
	Ipp16s j; 
	Ipp32fc pmean;
	Ipp32f arg=0;
	Ipp64f powm=pow((double)2,(double)M);

	pmean.re=0;
	pmean.im=0;
	for (j=0;j<nLeng;j++)
	{
		decphase[j]  =fmod((double)(OFDM_phase[j]-OFDM_ref[j]-IPP_2PI*OFDM_usef[j]*OFDM_symL),IPP_2PI);
	//	decphase[j]  = OFDM_phase[j];
		if(decphase[j]<0)
			decphase[j]=decphase[j]+IPP_2PI;
		pmean.re = pmean.re + cos(decphase[j]*powm);
		pmean.im = pmean.im + sin(decphase[j]*powm);
	}
	arg=atan2((double)pmean.im,(double)pmean.re)/powm;
	if(arg<0)
		arg = arg + IPP_PI/4;
	else
		arg = arg - IPP_PI/4;

	for (j=0;j<nLeng;j++)
	{
		decphase[j] = decphase[j]-arg;
	}

}
/************************************************************************/
/* 差分解调                                                             */
/************************************************************************/
void CLink11CLEW::Dec_demode(Ipp32fc *OFDM_fftsym,Ipp32s *bitout,Ipp16s pos,Ipp32fc *expsym)
{
	int i;
	Ipp32s bit=0;
	Ipp32f *phasecur,decphase,*phasetemp;
	phasecur = ippsMalloc_32f(OFDM_useNum);
	phasetemp = ippsMalloc_32f(OFDM_useNum);

	ippsPhase_32fc(OFDM_fftsym,phasecur,OFDM_useNum);

	/*for (i=0;i<OFDM_useNum;i++)
	{
		decphase  =fmod((double)(phasecur[i]-phaseref[i]-IPP_2PI*OFDM_usef[i]*OFDM_symL),IPP_2PI);
		expsym[i].re = cos(decphase)*1000;
		expsym[i].im = sin(decphase)*1000;
	}*/
	/*Ipp32f timphase;
	timingPLL(expsym,OFDM_useNum-1,4,2,timphase);
	int fidx[16] = {11,6,5,4,3,2,1,0,-1,-2,-3,-4,-5,-6,-7,-10};
	Remove_timephase(expsym,timphase,fidx,OFDM_useNum);*/
	//ippsPhase_32fc(expsym,phasecur,OFDM_useNum);

	OFDM_phase_VV(phasecur,phaseref,phasetemp,OFDM_useNum-1,2);
	
	for (i=0;i<OFDM_useNum-1;i++)
	{
	//	phasetemp = phasecur[i]-phaseref[i]-IPP_2PI*OFDM_usef[i]*OFDM_symL;
		decphase = fmod((double)phasetemp[i],IPP_2PI)-IPP_2PI;
		if(decphase<-IPP_2PI)
			decphase=decphase+IPP_2PI;

		expsym[i].re = cos(decphase)*1000;
		expsym[i].im = sin(decphase)*1000;

		if (decphase>-IPP_PI2 && decphase<=0)
			bit = (bit<<2)|3;
		else if (decphase>-IPP_PI && decphase<=-IPP_PI2)
			bit = (bit<<2)|2;
		else if(decphase>-3*IPP_PI2 && decphase<=-IPP_PI)
			bit = (bit<<2)|0;
		else if(decphase>-IPP_2PI && decphase<=-3*IPP_PI2)
			bit = (bit<<2)|1;
	}
	/*Ipp32f timphase;
	timingPLL(expsym,OFDM_useNum-1,2,2,timphase);
	int fidx[16] = {12,7,6,5,4,3,2,1,0,-1,-2,-3,-4,-5,-6,-9};
	Remove_timephase(expsym,timphase,fidx,OFDM_useNum);
	ippsPhase_32fc(expsym,phasetemp,OFDM_useNum);
	for(i=0;i<OFDM_useNum-1;i++)
	{
		phasetemp[i] = fmod((double)phasetemp[i],IPP_2PI)-IPP_2PI;
		if(phasetemp[i]<-IPP_2PI)
			phasetemp[i]=phasetemp[i]+IPP_2PI;

		if (phasetemp[i]>-IPP_PI2 && phasetemp[i]<=0)
			bit = (bit<<2)|3;
		else if (phasetemp[i]>-IPP_PI && phasetemp[i]<=-IPP_PI2)
			bit = (bit<<2)|2;
		else if(phasetemp[i]>-3*IPP_PI2 && phasetemp[i]<=-IPP_PI)
			bit = (bit<<2)|0;
		else if(phasetemp[i]>-IPP_2PI && phasetemp[i]<=-3*IPP_PI2)
			bit = (bit<<2)|1;
	}*/
	bit = bit&(0x3fffffff);
	bit = bit|(pos<<30);//第31位做标记
	*bitout = bit;

	ippsCopy_32f(phasecur,phaseref,OFDM_useNum);
	ippsFree(phasecur);
	ippsFree(phasetemp);

}
/************************************************************************/
/*   % 定时恢复环,从频域恢复时延误差
% yfft: 输入的频域数据,单个符号
% w: 频域窗长
% M：去调制阶数                                                                  
/************************************************************************/
void CLink11CLEW::timingPLL(Ipp32fc *pFFT,int fidxLen,int WinLen,int M,Ipp32f &phase)
{
	int i,j,k;
	int p = fidxLen/WinLen;

	Ipp32fc *Qp = ippsMalloc_32fc(p);
	Ipp32fc *tempM = ippsMalloc_32fc(WinLen);
	for (i=0;i<p;i++)
	{
		for(j=0;j<WinLen;j++)
		{
			tempM[j] = pFFT[i*WinLen+j];
			for(k=0;k<M-1;k++){
				ippsMul_32fc_I(&pFFT[i*WinLen+j],&tempM[j],1);
			}
		}
		ippsMean_32fc(tempM,WinLen,&Qp[i],ippAlgHintFast);
	}
	Ipp32fc corsum;
	corsum.re=0;corsum.im = 0;
	for (i=1;i<p;i++)
	{
		corsum.re = corsum.re + Qp[i].re*Qp[i-1].re + Qp[i].im*Qp[i-1].im;
		corsum.im = corsum.im + Qp[i].im*Qp[i-1].re - Qp[i].re*Qp[i-1].im;
	}

	phase = atan2(corsum.im,corsum.re)/(IPP_2PI*WinLen*M);
	
	ippsFree(Qp);
	ippsFree(tempM);

}
/************************************************************************/
/* 去除时延相位                                                                     */
/************************************************************************/
void CLink11CLEW::Remove_timephase(Ipp32fc *pSrcDst,Ipp32f tphase,int *fidx,int fidxLen)
{
	int i;
	Ipp32fc temp;
	for (i=0;i<fidxLen;i++)
	{
		temp.re = pSrcDst[i].re*cos(IPP_2PI*tphase*fidx[i]) + pSrcDst[i].im*sin(IPP_2PI*tphase*fidx[i]);
		temp.im = pSrcDst[i].im*cos(IPP_2PI*tphase*fidx[i]) - pSrcDst[i].re*sin(IPP_2PI*tphase*fidx[i]);
		pSrcDst[i].re = temp.re;
		pSrcDst[i].im = temp.im;
	}

}
void CLink11CLEW::DeAddress(Ipp32s *codein,Ipp16s codeinnum,int *address)
{
	int i,j;
	int m_addressA=0,m_addressB=0;		
	if(codeinnum==2)
	{
		for (i=0;i<63;i++)
		{
			if(Islike(&codein[0],&addressA[i]))
			{
				m_addressA = i+1;
				*address = m_addressA;
				return ;
			}
		}
		for (i=0;i<63;i++)
		{
			if(Islike(&codein[1],&addressB[i]))
			{
				m_addressB = i+1;
				*address = m_addressB;
				return;
			}
		}
		*address = m_addressA;
	}
	else if(codeinnum==1)
	{
		for (i=0;i<63;i++)
		{
			if(Islike(&codein[0],&addressA[i]) || Islike(&codein[0],&addressB[i]))
			{
				m_addressA = i+1;
				*address = m_addressA;
				return ;
			}
		}
		*address = m_addressA;
	}
	
	
}
bool CLink11CLEW::Islike(Ipp32s *codeinA,Ipp32s *codeinB)
{
	Ipp32s codeA = *codeinA;
	Ipp32s codeB = *codeinB;
	Ipp32s temp = codeA^codeB;
	short num=0;
	for (int i=0;i<30;i++)
	{
		if(((temp>>i)&1)==0)
			num++;
	}
	if(num>26)//允许错3个
		return TRUE;
	else
		return FALSE;
}
/************************************************************************/
/* 输入码字30bit
输出码字 24bit
(30,24)缩短汉明码译码
err = 1 表示有错误
/************************************************************************/
void CLink11CLEW::Decode(Ipp32s *codein,Ipp16s *err)
{
	int i,j;
	Ipp8s errorpattern=0;
	Ipp8s temp;
	Ipp32s code = *codein;
	Ipp32s r;
	*err = 0;
	for(i=0;i<6;i++)
	{
		temp = 1;
		r=code & parityH[i];
		for (j=0;j<30;j++)
		{
			temp = temp^((r>>j)&1);
		}
		errorpattern = errorpattern | (temp<<i);
	}
	if (errorpattern==0x07)
		*codein = code^1;
	else if(errorpattern==0x0B)
		*codein = code^(1<<1);
	else if(errorpattern==0x0D)
		*codein = code^(1<<2);
	else if(errorpattern==0x0F)
		*codein = code^(1<<3);
	else if(errorpattern==0x13)
		*codein = code^(1<<4);
	else if(errorpattern==0x15)
		*codein = code^(1<<5);
	else if(errorpattern==0x17)
		*codein = code^(1<<6);
	else if(errorpattern==0x19)
		*codein = code^(1<<7);
	else if(errorpattern==0x1B)
		*codein = code^(1<<8);
	else if(errorpattern==0x1D)
		*codein = code^(1<<9);
	else if(errorpattern==0x1F)
		*codein = code^(1<<10);
	else if(errorpattern==0x23)
		*codein = code^(1<<11);
	else if(errorpattern==0x25)
		*codein = code^(1<<12);
	else if(errorpattern==0x27)
		*codein = code^(1<<13);
	else if(errorpattern==0x29)
		*codein = code^(1<<14);
	else if(errorpattern==0x2B)
		*codein = code^(1<<15);
	else if(errorpattern==0x2D)
		*codein = code^(1<<16);
	else if(errorpattern==0x2F)
		*codein = code^(1<<17);
	else if(errorpattern==0x31)
		*codein = code^(1<<18);
	else if(errorpattern==0x33)
		*codein = code^(1<<19);
	else if(errorpattern==0x35)
		*codein = code^(1<<20);
	else if(errorpattern==0x37)
		*codein = code^(1<<21);
	else if(errorpattern==0x39)
		*codein = code^(1<<22);	
	else if(errorpattern==0x3B)
		*codein = code^(1<<23);
	else if(errorpattern==0x01)
		*codein = code^(1<<24);
	else if(errorpattern==0x03)
		*codein = code^(1<<25);
	else if(errorpattern==0x05)
		*codein = code^(1<<26);
	else if(errorpattern==0x09)
		*codein = code^(1<<27);
	else if(errorpattern==0x11)
		*codein = code^(1<<28);
	else if(errorpattern==0x21)
		*codein = code^(1<<29);
	else if(errorpattern==0 || errorpattern==0x3F)
		*codein = code;
	else
	{
		*codein = code;
		*err = 1;
	}

}
void CLink11CLEW::SaveToBit_ini()
{
	byte_flag = 0x80;
	data_byte = 0;
	bit_num = 8;
	symbol = 0; //bit流组成byte
}
void CLink11CLEW::SaveToBit(Ipp32s *code,short codenum,Ipp8u *mByte,int &Byteleng)
{
	short i,j,m=0;
	for (i=0;i<codenum;i++)
	{		
		for (j=29;j>=0;j--)
		{
			bit_num = (code[i]>>j)&1;
			if(bit_num == 1)
				data_byte |= byte_flag;
			byte_flag = byte_flag>>1;
			if(byte_flag == 0)//字节存满
			{
				mByte[m]=data_byte;
				m++;
				byte_flag = 0x80;
				data_byte = 0;
			}
		}
	}
	Byteleng = m;
}