#include "StdAfx.h"
#include "RLS.h"
#include <math.h>

RLS::RLS()
{
	RlsW = NULL;
	conjTemp = NULL;
	RlsG = NULL;
	RlsK = NULL;
	RlsP = NULL;
	xBuf = NULL;
}

RLS::~RLS()
{
	if(RlsW){
		ippsFree(RlsW);
		RlsW = NULL;
	}
	if(conjTemp){
		ippsFree(conjTemp);
		conjTemp = NULL;
	}
	if(RlsG){
		ippsFree(RlsG);
		RlsG = NULL;
	}
	if(RlsK){
		ippsFree(RlsK);
		RlsK = NULL;
	}
	if(RlsP){
		for ( int i=0;i<RlsLen;i++)
			ippsFree(RlsP[i]);
		delete RlsP; 
		RlsP = NULL;
	}
	if(xBuf)
	{
		ippsFree(xBuf);
		xBuf = NULL;
	}
}
void RLS::RLS_ini(int mOrderA,int mOrderB,float mlamda ,float mdelta)
{
	
	OrderA = mOrderA;
	OrderB = mOrderB;

	RlsLen = OrderA + OrderB;
	delta.re = mdelta;
	delta.im = 0;
	lamda = mlamda;

	RlsW=ippsMalloc_32fc(RlsLen);
	RlsP=new Ipp32fc*[RlsLen];
	conjTemp = ippsMalloc_32fc(RlsLen);
	RlsG = ippsMalloc_32fc(RlsLen);
	RlsK = ippsMalloc_32fc(RlsLen);

	int i;
	for ( i=0;i<RlsLen;i++)
	{
		RlsP[i]=ippsMalloc_32fc(RlsLen);
		ippsZero_32fc(RlsP[i], RlsLen);
		RlsP[i][i]=delta;
	}

	xBuf = ippsMalloc_32fc(RlsLen);
	ippsZero_32fc(xBuf,RlsLen);
	
}
void RLS::RLS_free()
{
	if(RlsW){
		ippsFree(RlsW);
		RlsW = NULL;
	}
	if(conjTemp){
		ippsFree(conjTemp);
		conjTemp = NULL;
	}
	if(RlsG){
		ippsFree(RlsG);
		RlsG = NULL;
	}
	if(RlsK){
		ippsFree(RlsK);
		RlsK = NULL;
	}
	if(RlsP){
		for ( int i=0;i<RlsLen;i++)
			ippsFree(RlsP[i]);
		delete RlsP; 
		RlsP = NULL;
	}
	if(xBuf)
	{
		ippsFree(xBuf);
		xBuf = NULL;
	}
}
void RLS::RLS_SetZero()
{
	ippsZero_32fc(RlsW,RlsLen);
	RlsW[(OrderA/2)].re=1;
	ippsZero_32fc(RlsG,RlsLen);
	ippsZero_32fc(RlsK,RlsLen);
	for (int i=0;i<RlsLen;i++)
	{
		ippsZero_32fc(RlsP[i], RlsLen);
		RlsP[i][i]=delta;
	}
	ippsZero_32fc(xBuf,RlsLen);
}
void RLS::RLS_Update(Ipp32fc pSrc,Ipp32fc yd,Ipp32fc &pDst,BOOL fseupdate,BOOL aid)
{
	int i,j;
	Ipp32fc g;
	Ipp32fc sumtemp,err;
	Ipp32fc temp,ydec;

	for (j=OrderA-1;j>=1;j--)
		xBuf[j] = xBuf[j-1];
	xBuf[0] = pSrc;

	if(fseupdate)
	{
			ippsConj_32fc(xBuf,conjTemp,RlsLen);
			for (i=0;i<RlsLen;i++)
			{			
				sumtemp.re = 0;
				sumtemp.im = 0;
				for (j = 0;j<RlsLen;j++)
				{
					temp = CmplxMult(conjTemp[j],RlsP[j][i]);
					sumtemp.re = sumtemp.re + temp.re;
					sumtemp.im = sumtemp.im + temp.im;
				}
				RlsG[i] = sumtemp;
			}
			ippsDotProd_32fc(RlsG, xBuf, RlsLen, &g);
			g.re=g.re+lamda;	//  (lmda+G*X)

			for (i=0;i<RlsLen;i++)
			{
				ippsDotProd_32fc(RlsP[i], xBuf, RlsLen,&temp);
				RlsK[i].re =  temp.re/g.re;
				RlsK[i].im =  temp.im/g.re;// K=(CP*Z)/g;

				ippsMulC_32fc(RlsG, RlsK[i], conjTemp, RlsLen);
				ippsSub_32fc_I(conjTemp, RlsP[i], RlsLen); 
				for (j=0;j<RlsLen;j++)
				{
					RlsP[i][j].re = RlsP[i][j].re/lamda;
					RlsP[i][j].im = RlsP[i][j].im/lamda;
				}
			}
			ippsConj_32fc(RlsW,conjTemp,RlsLen);
			ippsDotProd_32fc(conjTemp, xBuf, RlsLen, &temp);
			if(aid){
				err.re = yd.re - temp.re;
				err.im = -(yd.im - temp.im);
				if (fabs(err.re)>100)
					RLS_SetZero();

				for (i=0;i<RlsLen;i++)
				{
					temp = CmplxMult(RlsK[i],err);
					RlsW[i].re = RlsW[i].re + temp.re;
					RlsW[i].im = RlsW[i].im + temp.im;
				}
				ippsConj_32fc(RlsW,conjTemp,RlsLen);
				ippsDotProd_32fc(conjTemp, xBuf, RlsLen, &pDst);

				for (j=OrderA + OrderB-1;j>=OrderA+1;j--)
					xBuf[j] = xBuf[j-1];
				xBuf[OrderA] = yd;
			}
			else{
				DecPSK(temp,ydec);
				err.re = ydec.re - temp.re;
				err.im = -(ydec.im - temp.im);
				if (fabs(err.re)>100)
					RLS_SetZero();

				for (i=0;i<RlsLen;i++)
				{
					temp = CmplxMult(RlsK[i],err);
					RlsW[i].re = RlsW[i].re + temp.re;
					RlsW[i].im = RlsW[i].im + temp.im;
				}
				ippsConj_32fc(RlsW,conjTemp,RlsLen);
				ippsDotProd_32fc(conjTemp, xBuf, RlsLen, &pDst);

				for (j=OrderA + OrderB-1;j>=OrderA+1;j--)
					xBuf[j] = xBuf[j-1];
				xBuf[OrderA] = ydec;

			}

		}

}
Ipp32fc RLS::CmplxMult(Ipp32fc pSrcA,Ipp32fc pSrcB)
{
	Ipp32fc pSrcC;
	pSrcC.re = pSrcA.re*pSrcB.re - pSrcA.im*pSrcB.im;
	pSrcC.im  = pSrcA.re*pSrcB.im + pSrcA.im*pSrcB.re;
	return pSrcC;
}
void RLS::DecPSK(Ipp32fc pSrc,Ipp32fc &pDst)
{

	Ipp32f angle = atan2(pSrc.im,pSrc.re);
	if (angle>=-IPP_PI/8 && angle<IPP_PI/8)
	{
		pDst.re = 1;
		pDst.im = 0;
	}
	else if (angle>=IPP_PI/8 && angle<IPP_PI*3/8)	
	{												
		pDst.re = 0.707;
		pDst.im = 0.707;
	}									   
	else if(angle>=IPP_PI*3/8 && angle<IPP_PI*5/8)		
	{
		pDst.re = 0;
		pDst.im = 1;
	}										    
	else if(angle>=IPP_PI*5/8 && angle<IPP_PI*7/8)	
	{
		pDst.re = -0.707;
		pDst.im = 0.707;
	}										 	
	else if (angle>=-IPP_PI*7/8 && angle<-IPP_PI*5/8)	
	{
		pDst.re = -0.707;
		pDst.im = -0.707;
	}
	else if (angle>=-IPP_PI*5/8 && angle<-IPP_PI*3/8)
	{
		pDst.re = 0;
		pDst.im = -1;
	}
	else if (angle>=-IPP_PI*3/8 && angle<-IPP_PI*1/8)
	{
		pDst.re = 0.707;
		pDst.im = -0.707;
	}
	else
	{
		pDst.re = -1;
		pDst.im = 0;
	}
}
