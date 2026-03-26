#pragma once
#include "ipps.h"

/************************************************************************/
/*   识别110B
/************************************************************************/

class CPreambleConv;
class CDetect110B
{
public:
	CDetect110B(void);
	~CDetect110B(void);

public:
	void Detect110B(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,int &dataRate,int &interLeng,BOOL &detect);


	// 添加其他参数识别

protected:

	void IdentifyType(Ipp32fc *pSrc,int nLeng,Ipp32fc **Typesym,int typeNum,int &dataRate,int &interLeng);
	CPreambleConv	*m_CPreambleConv;
	Ipp32fc			*PreambleSym;
	int				PreambleLen;
	Ipp32fc			**pTypeSym;

};