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
	xBuffilter = NULL;
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
	if(xBuffilter)
	{
		ippsFree(xBuffilter);
		xBuffilter = NULL;
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
	xBuffilter = ippsMalloc_32fc(RlsLen);
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
	if(xBuffilter)
	{
		ippsFree(xBuffilter);
		xBuffilter = NULL;
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
void RLS::RLSFilter_ini()
{
	ippsCopy_32fc(xBuf,xBuffilter,RlsLen);
}
void RLS::RLSFilter(Ipp32fc pSrc,Ipp32fc &pDst,BOOL fseupdate)
{
	int i,j;
	Ipp8u num;
	Ipp32fc g;
	Ipp32fc sumtemp,err;
	Ipp32fc temp,ydec;

	for (j=OrderA-1;j>=1;j--)
		xBuffilter[j] = xBuffilter[j-1];
	xBuffilter[0] = pSrc;
	if(fseupdate){
		ippsConj_32fc(RlsW,conjTemp,RlsLen);
		ippsDotProd_32fc(conjTemp, xBuffilter, RlsLen, &pDst);

		DecPSK(pDst,ydec);

		for (j=OrderA + OrderB-1;j>=OrderA+1;j--)
			xBuffilter[j] = xBuffilter[j-1];
		xBuffilter[OrderA] = ydec;

	}

}
void RLS::RLS_Update(Ipp32fc pSrc,Ipp32fc yd,int M,Ipp32fc &pDst,BOOL fseupdate,BOOL aid)
{
	int i,j;
	Ipp8u num;
	Ipp32fc g;
	Ipp32fc sumtemp,err;
	Ipp32fc temp,ydec;

	for (j=OrderA-1;j>=1;j--)
		xBuf[j] = xBuf[j-1];
	xBuf[0] = pSrc;

	if(fseupdate){
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
				if (abs(err.re)>100)
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
				if(M<=8)
					DecPSK(temp,ydec);
				else
					DecQAM(temp,M,ydec,num);
				err.re = ydec.re - temp.re;
				err.im = -(ydec.im - temp.im);
				if (abs(err.re)>100)
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
void RLS::DecQAM(Ipp32fc pSrc,int M,Ipp32fc &pDst,Ipp8u &num)
{
	int i;
	float *I_map,*Q_map;
	I_map = new float[M];
	Q_map = new float[M];
	float temp,mintemp=10000;
	int minidx=0;

	PSK_map(1,M,I_map,Q_map);  // style = 1 Ë³ÐòÓ³Éä
	for (i=0; i<M; i++)
	{
		temp = (pSrc.re-I_map[i])*(pSrc.re-I_map[i])+(pSrc.im-Q_map[i])*(pSrc.im-Q_map[i]);
		if (temp<mintemp)
		{
			mintemp = temp;
			minidx = i;
		}
	}
	pDst.re = I_map[minidx];
	pDst.im = Q_map[minidx];
	num = minidx;
	free(I_map);
	free(Q_map);
}
void RLS::PSK_map(short style, short M, float I_map[],float Q_map[])
{
	switch(M)
	{
	case 2:
		if(style==0)
		{
			I_map[0]=1;Q_map[0]=0;
			I_map[1]=-1;Q_map[1]=0;
		}
		else if(style==1)
		{
			I_map[0]=-1;Q_map[0]=0;
			I_map[1]=1;Q_map[1]=0;
		}
		break;
	case 4:
		if(style==0)
		{
			I_map[0]=1;Q_map[0]=0;
			I_map[1]=0;Q_map[1]=1;
			I_map[2]=0;Q_map[2]=-1;
			I_map[3]=-1;Q_map[3]=0;
		}
		else if(style==1)
		{
			I_map[0]=1;Q_map[0]=0;
			I_map[1]=0;Q_map[1]=1;
			I_map[2]=-1;Q_map[2]=0;
			I_map[3]=0;Q_map[3]=-1;
		}
		break;
	case 8:
		if(style==0)
		{
			I_map[0]=1;		Q_map[0]=0;
			I_map[1]=0.707; Q_map[1]=0.707;
			I_map[2]=-0.707;Q_map[2]=0.707;
			I_map[3]=0;		Q_map[3]=1;
			I_map[4]=0.707;	Q_map[4]=-0.707;
			I_map[5]=0;		Q_map[5]=-1;
			I_map[6]=-1;	Q_map[6]=0;
			I_map[7]=-0.707;Q_map[7]=-0.707;
		}
		else if(style==1)
		{
			I_map[0]=1;		Q_map[0]=0;
			I_map[1]=0.707; Q_map[1]=0.707;
			I_map[2]=0;		Q_map[2]=1;
			I_map[3]=-0.707;Q_map[3]=0.707;
			I_map[4]=-1;	Q_map[4]=0;
			I_map[5]=-0.707;Q_map[5]=-0.707;
			I_map[6]=0;		Q_map[6]=-1;
			I_map[7]=0.707;	Q_map[7]=-0.707;
		}
		break;
	case 16:
		I_map[0]=0.866025;  Q_map[0]=0.500000 ;
		I_map[1]=0.500000;  Q_map[1]=0.866025 ;
		I_map[2]=1.000000;  Q_map[2]=0.000000; 
		I_map[3]=0.258819;  Q_map[3]=0.258819; 
		I_map[4]=-0.50000;  Q_map[4]=0.866025; 
		I_map[5]=0.000000;  Q_map[5]=1.000000; 
		I_map[6]=-0.86602;  Q_map[6]=0.500000; 
		I_map[7]=-0.25881;  Q_map[7]=0.258819; 
		I_map[8]=0.500000;  Q_map[8]=-0.866025; 
		I_map[9]=0.000000;  Q_map[9]=-1.000000; 
		I_map[10]=0.866025; Q_map[10]=-0.500000; 
		I_map[11]=0.258819; Q_map[11]=-0.258819;
		I_map[12]=-0.86602; Q_map[12]=-0.500000; 
		I_map[13]=-0.50000; Q_map[13]=-0.866025; 
		I_map[14]=-1.00000; Q_map[14]=0.000000; 
		I_map[15]=-0.25882; Q_map[15]=-0.258819; 
		break;
	case 32:
		I_map[0]=0.866380;  Q_map[0]=0.499386;  I_map[16]=0.866380;  Q_map[16]=-0.499386;
		I_map[1]=0.984849;  Q_map[1]=0.173415;  I_map[17]=0.984849;  Q_map[17]=-0.173415; 
		I_map[2]=0.499386;  Q_map[2]=0.866380;  I_map[18]=0.499386;  Q_map[18]=-0.866380; 
		I_map[3]=0.173415;  Q_map[3]=0.984849;  I_map[19]=0.173415; Q_map[19]=-0.984849; 
		I_map[4]=0.520246;  Q_map[4]=0.520246;  I_map[20]=0.520246;  Q_map[20]=-0.520246; 
		I_map[5]=0.520246;  Q_map[5]=0.173415;  I_map[21]=0.520246;  Q_map[21]=-0.173415; 
		I_map[6]=0.173415;  Q_map[6]=0.520246;  I_map[22]=0.173415;  Q_map[22]=-0.520246; 
		I_map[7]=0.173415;  Q_map[7]=0.173415;  I_map[23]=0.173415;  Q_map[23]=-0.173415; 
		I_map[8]=-0.866380;  Q_map[8]=0.499386;  I_map[24]=-0.866380;  Q_map[24]=-0.499386; 
		I_map[9]=-0.984849;  Q_map[9]=0.173415;  I_map[25]=-0.984849;  Q_map[25]=-0.173415; 
		I_map[10]=-0.499386;  Q_map[10]=0.866380;  I_map[26]=-0.499386;  Q_map[26]=-0.866380; 
		I_map[11]=-0.173415;  Q_map[11]=0.984849;  I_map[27]=-0.173415;  Q_map[27]=-0.984849; 
		I_map[12]=-0.520246;  Q_map[12]=0.520246;  I_map[28]=-0.520246;  Q_map[28]=-0.520246; 
		I_map[13]=-0.520246;  Q_map[13]=0.173415;  I_map[29]=-0.520246;  Q_map[29]=-0.173415; 
		I_map[14]=-0.173415;  Q_map[14]=0.520246;  I_map[30]=-0.173415;  Q_map[30]=-0.520246; 
		I_map[15]=-0.173415;  Q_map[15]=0.173415;  I_map[31]=-0.173415;  Q_map[31]=-0.173415; 
		break;
	case 64:
		I_map[0]=1.000000;  Q_map[0]=0.000000 ;  I_map[32]=0.000000;   Q_map[32]=1.000000; 
		I_map[1]=0.822878;  Q_map[1]=0.568218 ;  I_map[33]=-0.822878;  Q_map[33]=0.568218; 
		I_map[2]=0.821137;  Q_map[2]=0.152996 ;  I_map[34]=-0.821137;  Q_map[34]=0.152996; 
		I_map[3]=0.932897;  Q_map[3]=0.360142 ;  I_map[35]=-0.932897;  Q_map[35]=0.360142 ;
		I_map[4]=0.000000;  Q_map[4]=-1.000000;  I_map[36]=-1.000000;  Q_map[36]=0.000000; 
		I_map[5]=0.822878;  Q_map[5]=-0.568218;  I_map[37]=-0.822878;  Q_map[37]=-0.568218; 
		I_map[6]=0.821137;  Q_map[6]=-0.152996;  I_map[38]=-0.821137;  Q_map[38]=-0.152996 ;
		I_map[7]=0.932897;  Q_map[7]=-0.360142;  I_map[39]=-0.932897;  Q_map[39]=-0.360142; 
		I_map[8]=0.568218;  Q_map[8]=0.822878 ;  I_map[40]=-0.568218;  Q_map[40]=0.822878; 
		I_map[9]=0.588429;  Q_map[9]=0.588429 ;  I_map[41]=-0.588429;  Q_map[41]=0.588429; 
		I_map[10]=0.588429;  Q_map[10]=0.117686 ;  I_map[42]=-0.588429;  Q_map[42]=0.117686 ;
		I_map[11]=0.588429;  Q_map[11]=0.353057 ;  I_map[43]=-0.588429;  Q_map[43]=0.353057; 
		I_map[12]=0.568218;  Q_map[12]=-0.822878;  I_map[44]=-0.568218;  Q_map[44]=-0.822878; 
		I_map[13]=0.588429;  Q_map[13]=-0.588429;  I_map[45]=-0.588429;  Q_map[45]=-0.588429; 
		I_map[14]=0.588429;  Q_map[14]=-0.117686;  I_map[46]=-0.588429;  Q_map[46]=-0.117686 ;
		I_map[15]=0.588429;  Q_map[15]=-0.353057;  I_map[47]=-0.588429;  Q_map[47]=-0.353057 ;
		I_map[16]=0.152996;  Q_map[16]=0.821137 ;  I_map[48]=-0.152996;  Q_map[48]=0.821137; 
		I_map[17]=0.117686;  Q_map[17]=0.588429 ;  I_map[49]=-0.117686;  Q_map[49]=0.588429; 
		I_map[18]=0.117686;  Q_map[18]=0.117686 ;  I_map[50]=-0.117686;  Q_map[50]=0.117686; 
		I_map[19]=0.117686;  Q_map[19]=0.353057 ;  I_map[51]=-0.117686;  Q_map[51]=0.353057; 
		I_map[20]=0.152996;  Q_map[20]=-0.821137;  I_map[52]=-0.152996;  Q_map[52]=-0.821137; 
		I_map[21]=0.117686;  Q_map[21]=-0.588429;  I_map[53]=-0.117686;  Q_map[53]=-0.588429; 
		I_map[22]=0.117686;  Q_map[22]=-0.117686;  I_map[54]=-0.117686;  Q_map[54]=-0.117686 ;
		I_map[23]=0.117686;  Q_map[23]=-0.353057;  I_map[55]=-0.117686;  Q_map[55]=-0.353057; 
		I_map[24]=0.360142;  Q_map[24]=0.932897 ;  I_map[56]=-0.360142;  Q_map[56]=0.932897; 
		I_map[25]=0.353057;  Q_map[25]=0.588429 ;  I_map[57]=-0.353057;  Q_map[57]=0.588429 ;
		I_map[26]=0.353057;  Q_map[26]=0.117686 ;  I_map[58]=-0.353057;  Q_map[58]=0.117686; 
		I_map[27]=0.353057;  Q_map[27]=0.353057 ;  I_map[59]=-0.353057;  Q_map[59]=0.353057 ;
		I_map[28]=0.360142;  Q_map[28]=-0.932897;  I_map[60]=-0.360142;  Q_map[60]=-0.932897; 
		I_map[29]=0.353057;  Q_map[29]=-0.588429;  I_map[61]=-0.353057;  Q_map[61]=-0.588429; 
		I_map[30]=0.353057;  Q_map[30]=-0.117686;  I_map[62]=-0.353057;  Q_map[62]=-0.117686; 
		I_map[31]=0.353057;  Q_map[31]=-0.353057;  I_map[63]=-0.353057;  Q_map[63]=-0.353057 ;
		break;
	default:
		break;
	}
}