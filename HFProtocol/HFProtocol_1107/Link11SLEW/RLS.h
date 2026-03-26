#pragma once

#include "ipps.h"
class RLS
{
public:
	
	// RLS_DFE
	int RlsLen;
	Ipp32fc *RlsW;
	Ipp32fc *RlsG;
	Ipp32fc *RlsK;
	Ipp32fc **RlsP;
	Ipp32fc *conjTemp;
	Ipp32f lamda;
	Ipp32fc delta;
	Ipp32fc *xBuf;
	int OrderA,OrderB;

public:
	RLS(void);
	~RLS(void);

	void RLS_ini(int mOrderA,int mOrderB,float mlamda ,float mdelta);
	void RLS_free();
	void RLS_SetZero();
	void RLS_Update(Ipp32fc pSrc,Ipp32fc yd,Ipp32fc &pDst,BOOL fseupdate,BOOL aid);
	void DecPSK(Ipp32fc pSrc,Ipp32fc &pDst);
	Ipp32fc CmplxMult(Ipp32fc pSrcA,Ipp32fc pSrcB);
};