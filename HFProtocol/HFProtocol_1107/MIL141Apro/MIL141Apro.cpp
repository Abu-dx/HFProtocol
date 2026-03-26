// MIL141Apro.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "MIL141Apro.h"
#include "FSK8demode.h"
#include "ReSampleFSK.h"
#include <string>
#include <vector>
#include <math.h>
#include "table.h"
#include <atlconv.h>


CMIL141Apro::CMIL141Apro(void)
{
	m_Resample = NULL;
	m_FSKdemode= NULL;

}

CMIL141Apro::~CMIL141Apro(void)
{
	if(m_Resample)
	{		
		delete m_Resample;
		m_Resample = NULL;
	}

	if(m_FSKdemode)
	{
		delete m_FSKdemode;
		m_FSKdemode = NULL;
	}

}

void CMIL141Apro::MIL141Ademode_ini(float fsOld,float fsNew,int M,float f0,float FSpace,BOOL mautoset,int nLeng,int m_WinType)
{
	m_Resample = new CReSampleFSK;
	/*m_Resample->ReSample_ini(nLeng,fsOld,fsNew,64,8192);
	m_Resample->IppReSampleIni_16s(nLeng,)*/

	m_Resample->IppReSampleIni_16s(nLeng,128,fsOld,fsNew);

	m_FSKdemode = new CFSK8demode;
	m_FSKdemode->FSKdemode_ini(fsNew,125,M,f0,FSpace,nLeng,m_WinType);

	SetF0(f0,mautoset);

	reBuf = ippsMalloc_32f(2*nLeng);
	Judgeout = ippsMalloc_8u(nLeng);
	JudgeLen = 0;
	Judgedelay = ippsMalloc_8u(49*4);
	delayLen = 0;

	// 检测
	FFTleng = 8192;
	FFTorder = 13;
	ippsFFTInitAlloc_R_32f(&pSpec, FFTorder, IPP_FFT_DIV_INV_BY_N, ippAlgHintAccurate );
	fftDst = ippsMalloc_32f(FFTleng);
	fftTemp = ippsMalloc_32f(FFTleng*2);

	decoding_table = ippsMalloc_16s(2048);
	Gen_decodetable(decoding_table);
	flag = 0;
	SaveTobit_ini();
	autoset = mautoset;

}

void CMIL141Apro::MIL141Ademode_free()
{
//	m_Resample->ReSample_Free();
	m_Resample->IppReSamplefree_16s();
	delete m_Resample;
	m_Resample = NULL;

	ippsFFTFree_R_32f(pSpec);
	ippsFree(fftDst);
	ippsFree(fftTemp);

	if(m_FSKdemode)
	{
		m_FSKdemode->FSKdemode_free();
		delete m_FSKdemode;
		m_FSKdemode = NULL;		
	}
	ippsFree(reBuf);
	ippsFree(Judgedelay);
	ippsFree(Judgeout);
	ippsFree(decoding_table);

}
void CMIL141Apro::SetF0(float f0,BOOL mautoset)
{
	autoset = mautoset;
	if(!autoset) //  外部设置
		m_FSKdemode->SetFpara(8,f0,125);
}

//void CMIL141Apro::MIL141Ademode(Ipp16s *pSrc,int nLeng,Ipp8u *outbyte,int &byteleng,CString *message,CString *Address,int &messagenum)
void CMIL141Apro::MIL141Ademode(Ipp16s* pSrc, int nLeng, Ipp8u* outbyte, int& byteleng, 
	std::vector<std::string>& message_string, std::vector<std::string>& Address_string, int& messagenum)
{
	CString message[200];
	CString Address[200];

	int reSampleLeng;
	int detect8fsk;
	int bitLen;
	int bytetemp;
	Ipp8u *demodeOut = ippsMalloc_8u(nLeng);
	Ipp16s *temp = ippsMalloc_16s(2*nLeng);
	Ipp8u *buffind = ippsMalloc_8u(nLeng);
	int findlen=0;
	int demodeOutLen;
	byteleng = 0;
	messagenum = 0;
	int i;

	m_Resample->IppReSample_16s(pSrc,nLeng,temp,reSampleLeng);
	ippsConvert_16s32f(temp,reBuf,reSampleLeng);
	detect8fsk = 0;

	if(autoset)
	{
		frequence_detect(reBuf,reSampleLeng,f0,detect8fsk);
		if(detect8fsk)
			m_FSKdemode->SetFpara(8,f0,125);
	}
	//	frequence_detect(reBuf,reSampleLeng,f0,detect8fsk);
	//	if(autoset && detect8fsk)
	//		m_FSKdemode->SetFpara(8,f0,125);
		//if(detect8fsk)
		//{
			m_FSKdemode->FSKdemode(reBuf,reSampleLeng,demodeOut,demodeOutLen);
			ConvertTobit(demodeOut,demodeOutLen,Judgeout,bitLen);	

			// 找3倍冗余
			FindStart(Judgeout,bitLen,buffind,findlen);
			if(findlen>0)
			{
				for (i=0;i<147;i++)
				{
					TRACE("%d",buffind[i]);
				}
				TRACE("\r\n");
				int num = findlen/147;
				DataProcess(buffind,num,&outbyte[byteleng],bytetemp,&message[messagenum],&Address[messagenum]);
				messagenum = messagenum + num;
				byteleng = byteleng + bytetemp;
			}
		//}
		//else
		//{
		//	// 缓存清空
		//	delayLen=0;
		//	Judgedelay[0]=0;Judgedelay[1]=0;Judgedelay[2]=0;
		//}


			// 拷贝字符数据
			message_string.resize(messagenum);
			Address_string.resize(messagenum);
			for (int i = 0; i < messagenum; i ++)
			{
				message_string[i] = std::string(CT2A(message[i]));
				Address_string[i] = std::string(CT2A(Address[i]));
			}

	ippsFree(demodeOut);
	ippsFree(temp);
	ippsFree(buffind);
}
// nLen>49*3
void CMIL141Apro::FindStart(Ipp8u *pSrc,int nLen,Ipp8u *pDst,int &outLen)
{
	int i;
	int alllen = delayLen + nLen;
	Ipp8u *pdata = ippsMalloc_8u(alllen);
	ippsCopy_8u(Judgedelay,pdata,delayLen);
	ippsCopy_8u(pSrc,&pdata[delayLen],nLen);

	int num1 = 0;
	int num2 = 0;
	outLen= 0;
	i=0;
	while(i<=alllen-49*3)
	{
		FindSame(&pdata[i],&pdata[i+49],49,num1);
		FindSame(&pdata[i],&pdata[i+98],49,num2);
		if(num1==49 && num2==49)
		{
			ippsCopy_8u(&pdata[i],&pDst[outLen],147);
			outLen = outLen + 147;
			
			i = i + 147;
		}
		else
		{
			i = i+3;
		}
	}
	delayLen = alllen-i;
	ippsCopy_8u(&pdata[i],Judgedelay,delayLen);
	ippsFree(pdata);
}
void CMIL141Apro::DataProcess(Ipp8u *pSrc,int blocknum,Ipp8u *byteout,int &bytenum,CString *message,CString *Address)
{
	int i,j;
	Ipp8u voteOut[48];
	Ipp16s Normal[24];
	Ipp16s Inverted[24];
	long Normalcode,Invertedcode,codejoin;
	bytenum = 0;
	for (i=0;i<blocknum;i++)
	{
		vote(&pSrc[i*49*3],voteOut);
		deinterleaving(voteOut,Normal,Inverted);
		Golay_endecode(Normal,Normalcode,decoding_table);
		Golay_endecode(Inverted,Invertedcode,decoding_table);
		

		codejoint(Normalcode,Invertedcode,codejoin);
		message_show(codejoin,message[i],Address[i]);

		for (j=0;j<3;j++)
		{
			byteout[bytenum] = (codejoin>>(16-j*8))&0xFF;
			bytenum++;
		}
	}

}

// A 与 B相同的个数
void CMIL141Apro::FindSame(Ipp8u *pSrcA,Ipp8u *pSrcB,int nLen,int &num)
{
	int i;
	num = 0;
	for (i=0;i<nLen;i++)
	{
		if(pSrcA[i] == pSrcB[i])
			num++;
	}	
}


void CMIL141Apro::ConvertTobit(Ipp8u *pSrc,int nLeng,Ipp8u *outbyte,int &outLen)
{
	int i,j;
	Ipp8u bittemp;
	outLen = 0;
	for(i=0;i<nLeng;i++)
	{
		for(j = 0; j < 3; j++)
		{
			bittemp = (pSrc[i] >> (2-j)) & 0x01;
			outbyte[outLen]=bittemp;
			outLen++;
		}
	}

}

/************************************************************************/
/* 判断8FSK信号并检测频率      
/************************************************************************/
void CMIL141Apro::frequence_detect(Ipp32f *pSrc,int nLeng,Ipp32f &f0,int &FSK8)
{
	int i;

	Ipp32f *reBuf1 = ippsMalloc_32f(FFTleng);
	Ipp32f *reBuf2 = ippsMalloc_32f(FFTleng);
	Ipp32f *fr_min = ippsMalloc_32f(15);
	Ipp32f *f_l = ippsMalloc_32f(7);
	Ipp32f *f_r = ippsMalloc_32f(7);
	Ipp32f meanabs=0;
	

	for (i=0;i<nLeng;i++)
	{
		meanabs = meanabs + abs(pSrc[i])*abs(pSrc[i])/nLeng;
	}

	ippsZero_32f(reBuf2,FFTleng);
	if(nLeng<FFTleng)
		for (i=0;i<nLeng;i++)
			reBuf2[i] = pSrc[i]/sqrt(meanabs);
	else
		for (i=0;i<FFTleng;i++)
			reBuf2[i] = pSrc[i]/sqrt(meanabs);

	
	//fft,存在reBuf1中,长度为length
	ippsFFTFwd_RToCCS_32f( reBuf2, fftTemp, pSpec, NULL );
	ippsMagnitude_32fc( (Ipp32fc*)fftTemp, reBuf2, FFTleng/2);

	int length = FFTleng/2-40;
	ippsCopy_32f(&reBuf2[39],reBuf1,length);


	Ipp32f fmean;
	ippsMean_32f(reBuf1,length,&fmean,ippAlgHintFast);
	ippsSubC_32f_I(fmean,reBuf1,length);
	Ipp32f pMax;
	ippsMax_32f(reBuf1,length,&pMax);
	/*if(pMax<1)
		pMax = 1;*/
	ippsDivC_32f_I(pMax,reBuf1,length);

	int winL = 10;
	int nL = length-winL;
	Ipp32f tempmean;
	for (i=0;i<nL;i++)
	{
		ippsMean_32f(&reBuf1[i],winL,&tempmean,ippAlgHintFast);
		reBuf1[i] = tempmean;
	}
	Ipp32f stddev;
	ippsStdDev_32f(reBuf1,nL,&stddev,ippAlgHintFast);
	if (stddev>0.15)  //  方差太大不是8FSK
	{
		FSK8 = 0;
		return;
	}

	// 
	int pIdx;
	ippsMaxIndx_32f(reBuf1,nL,&pMax,&pIdx);
	Ipp32f fw_op;
	fw_op = (pIdx+40)*12000/FFTleng;

	int t1=0;
	int t2=0;//用来标识f_r和f_l的个数
	for (i=1;i<8;i++)
	{
		if (fw_op+i*250<3000)
		{
			f_r[i-1]=fw_op + i*250;
			t1++;
		}
	}
	for (i=1;i<8;i++)
	{
		if (fw_op-i*250>300)
		{
			f_l[i-1]=fw_op - i*250;
			t2++;
		}
	}
	for (i=0;i<t2;i++)
		fr_min[i] = f_l[t2-i-1];
	fr_min[t2] = fw_op;
	for (i=t2+1;i<t1+t2+1;i++)
		fr_min[i] = f_r[i-t2-1];

	Ipp32f *fr_data_temp = ippsMalloc_32f(15);
	int t;
	/*for (i=0;i<t1+t2+1;i++)
	{
		TRACE("qqqqqqq");
		TRACE("%f_",fr_min[i]);
		TRACE("\n");
	}
	TRACE("aaaaaaaaaaaa");*/
	for (i=0;i<t1+t2+1;i++)
	{
		t = fr_min[i]*FFTleng/12000;
		if((fr_min[i]*FFTleng/12000 - t)>0.5)
			fr_data_temp[i] = reBuf1[t+1-40];
		else
			fr_data_temp[i] = reBuf1[t-40];
		
		/*TRACE("%f_",fr_data_temp[i]);
		TRACE("\n");*/
	}

	////////%%%判断是不是8fsk%%
	t = 0;
	for (i=0;i<t1+t2+1;i++)
	{
		if (fr_data_temp[i]>0.05)
		{
			t = t + 1;
			/*TRACE("sssssss");
			TRACE("%f-",fr_data_temp[i]);*/
		}
	}
	if (t<4)
	{
		FSK8 = 0;
		return;
	}
	else
		FSK8  = 1;

	// 计算起始频率
		int Count=0;
		Ipp32f freq_first=0;
		Count = t1+t2+1-8;
	//	TRACE("%d_",Count);
		switch(Count)
		{
		case 0: 
			freq_first = fr_min[0];
			break;
		case 1: 
			if (fr_data_temp[0]>fr_data_temp[t1+t2])
				freq_first = fr_min[0];
			else
				freq_first = fr_min[1];
			break;
		case 2:
			if ((fr_data_temp[0]>fr_data_temp[t1+t2]) && (fr_data_temp[0]>fr_data_temp[t1+t2-1]) )
				freq_first = fr_min[0];
			else if (fr_data_temp[1]>fr_data_temp[t1+t2])
				freq_first = fr_min[1];
			else
				freq_first = fr_min[2];
			break;
		case 3:
			if ((fr_data_temp[0]>fr_data_temp[t1+t2]) && (fr_data_temp[0]>fr_data_temp[t1+t2-1]) && (fr_data_temp[0]>fr_data_temp[t1+t2-2]) )
				freq_first = fr_min[0];
			else if ((fr_data_temp[1]>fr_data_temp[t1+t2]) && (fr_data_temp[1]>fr_data_temp[t1+t2-1]) )
				freq_first = fr_min[1];
			else if (fr_data_temp[2]>fr_data_temp[t1+t2])
				freq_first = fr_min[2];
			else 
				freq_first = fr_min[3];
			break;
		default:
			freq_first = fr_min[0];
			break;

		}
		f0 = freq_first +  4*250 -125;
		ippsFree(reBuf1);
		ippsFree(reBuf2);
		ippsFree(fr_min);
		ippsFree(f_l);
		ippsFree(f_r);
		ippsFree(fr_data_temp);
}
//  2/3判决
//  固定长度 49*3
void  CMIL141Apro::vote(Ipp8u *pSrc,Ipp8u *pDst)
{
	int i,j;
	int num0,num1;

	for (j=0;j<48;j++)
	{
		num0=0;num1=0;
		for (i=0;i<3;i++)
		{
			if (pSrc[i*49+j] == 0)
				num0++;
			else
				num1++;
		}

		if (num0 > num1)
			pDst[j] = 0;
		else
			pDst[j] = 1;
	}

}
//解交织
void CMIL141Apro::deinterleaving(Ipp8u *pSrc_vote,Ipp16s *Normal,Ipp16s *Inverted)
{
	int i,j;
	for (i=0;i<48;i=i+2)
		Normal[i/2] = pSrc_vote[i];
	for (i=1;i<48;i=i+2)
		Inverted[i/2] = pSrc_vote[i];

	for (j=0;j<12;j++)
		Inverted[12+j] = (Inverted[12+j] + 1)%2;
}

void CMIL141Apro::Golay_endecode(Ipp16s *code,long &encode,Ipp16s *decoding_table)
{
	int j;
	encode = 0;
	/*for (j=0;j<12;j++)
	{
		recd = recd<<1;
		recd = recd +code[j];
	}
	for (j=13;j<24;j++)
	{
		recd = recd<<1;
		recd = recd +code[j];
	}
	encode = golayDecode(recd,decoding_table);*/
	
	for (j=0;j<12;j++)
	{
		encode = encode<<1;
		encode = encode +code[j];
	}
}
void CMIL141Apro::codejoint(long code_normal,long code_invert,long &codeout)
{
	codeout = code_normal<<12;
	codeout = codeout + code_invert;
}
void CMIL141Apro::message_show(long code,CString &message,CString &Address)
{
	int n = code>>21;
	switch(n)
	{
	case 0:
		message = "DATA";
		break;
	case 1:
		message = "THRU";
		break;
	case 2:
		message = "TO";
		break;
	case 3:
		message = "THIS WAS";
		break;
	case 4:
		message = "FROM";
		break;
	case 5:
		message = "THIS IS";
		break;
	case 6:
		message = "COMMAND";
		break;
	case 7:
		message = "REPEAT";
		break;

	}
	long m = code>>14;
	m = m & 0x7f;
	Address = table[m];

	m = code>>7;
	m = m & 0x7f;
	Address = Address + table[m];
	
	m = code & 0x7f;
	Address = Address + table[m];

}


void CMIL141Apro::SaveTobit_ini()
{
	byte_flag = 0x80;
	data_byte = 0;
	bit_num = 8;
	symbol = 0; //bit流组成byte

}
void CMIL141Apro::SaveTobit(Ipp8u *pSrc,int nLeng,Ipp8u *outbyte,int &byteLeng,int M)
{
	Ipp32s i,m,j;
	switch(M)
	{
	case 2:  //  2FSK
		m=0;
		for (i=0;i<nLeng;i++)
		{
			////////////　bit流组成byte存放　//////////////
			if(pSrc[i] == 1)
				data_byte |= byte_flag;
			byte_flag >>= 1;
			if(byte_flag == 0)//字节存满
			{
				outbyte[m]=data_byte;
				m++;				
				byte_flag = 0x80;
				data_byte = 0;
			}	
		}
		byteLeng = byteLeng /8;
		break;
	case 4: // 4FSK
		m=0;
		for (i=0;i<nLeng;i++)
		{
			////////////　bit流组成byte存放　//////////////		
			bit_num -= 2;
			data_byte |= (pSrc[i] << bit_num);			
			if(bit_num == 0)
			{
				outbyte[m]=data_byte;
				m++;
				bit_num = 8;
				data_byte = 0;
			}
		}
		break;
	case 3: //字节转换成比特
		m=0;
		for(i=0;i<nLeng;i++)
		{
			//
			for(j = 0; j < 3; j++)
			{
				bit_num = (pSrc[i] >> (2-j)) & 0x01;
				outbyte[m]=bit_num;
				m++;
			}
		}
		break;
	case 8:  // 8FSK 按比特存成字节（8比特）即1（001）2（010）3（011）存成41（00101001）
		m=0;
		for(i=0;i<nLeng;i++)
		{
			//
			for(j = 0; j < 3; j++)
			{
				bit_num = (pSrc[i] >> (2-j)) & 0x01;
				if(bit_num == 1)
					data_byte |= byte_flag;
				byte_flag = byte_flag>>1;
				if(byte_flag == 0) //字节存满
				{
					outbyte[m]=data_byte;
					m++;
					byte_flag = 0x80;
					data_byte = 0;
				}
			}	
		}
		break;
	}
	byteLeng = m;
}


/************************************************************************
  根据长度求出最接近的fft点数                                         
/************************************************************************/
void CMIL141Apro::GetFFT_len(int dataLen,int &FFTLeng,int &FFTorder)
{
	int FFTm=0;
	int i,j,m,n=0;
	for (i=0,j=1;i<16;i++,j=j*2)
	{		
		m=dataLen&j;
		if(m==j)
		{
			n++;
			FFTm=i;
		}
	}
	//if(n!=1)
	//	FFTm=FFTm+1;
	FFTorder = FFTm;
	FFTLeng= 1<<FFTm;	
}
long CMIL141Apro::arr2int(int *a,int r)
/*
 * Convert a binary vector of Hamming weight r, and nonzero positions in
 * array a[1]...a[r], to a long integer \sum_{i=1}^r 2^{a[i]-1}.
 */
{
   int i;
   long mul, result = 0, temp;
 
   for (i=1; i<=r; i++) 
   {
      mul = 1;
      temp = a[i]-1;
      while (temp--)
         mul = mul << 1;
      result += mul;
      }
   return(result);
}
void CMIL141Apro::nextcomb(int n,int r,int *a)
/*
 * Calculate next r-combination of an n-set.
 */
{
  int  i, j;
 
  a[r]++;
  if (a[r] <= n)
      return;
  j = r - 1;
  while (a[j] == n - r + j)
     j--;
  for (i = r; i >= j; i--)
      a[i] = a[j] + i - j + 1;
  return;
}
long CMIL141Apro::get_syndrome(long pattern)
/*
 * Compute the syndrome corresponding to the given pattern, i.e., the
 * remainder after dividing the pattern (when considering it as the vector
 * representation of a polynomial) by the generator polynomial, GENPOL.
 * In the program this pattern has several meanings: (1) pattern = infomation
 * bits, when constructing the encoding table; (2) pattern = error pattern,
 * when constructing the decoding table; and (3) pattern = received vector, to
 * obtain its syndrome in decoding.
 */
{
    long aux = X22;
	long aux2;
 
    if (pattern >= X11)
       while (pattern & MASK12) 
	   {
           while (!(aux & pattern))
              aux = aux >> 1;
           pattern ^= (aux/X11) * GENPOL;
           }

    return(pattern); 
}
void CMIL141Apro::Gen_decodetable(Ipp16s *decoding_table)
{
	//Ipp16s *decoding_table = ippsMalloc_8u(2048);
	int i;
	long temp;
	int a[4];
	decoding_table[0] = 0;
    decoding_table[1] = 1;
    temp = 1; 
    for (i=2; i<= 23; i++) 
	{
        temp *= 2;
        decoding_table[get_syndrome(temp)] = temp;
     }
   /*            
    * (2) Error patterns of WEIGHT 2 (DOUBLE ERRORS)
    */
    a[1] = 1; a[2] = 2;
    temp = arr2int(a,2);
    decoding_table[get_syndrome(temp)] = temp;    
    for (i=1; i<253; i++) 
	{
        nextcomb(23,2,a);
        temp = arr2int(a,2);
        decoding_table[get_syndrome(temp)] = temp;
     }
   /*            
    * (3) Error patterns of WEIGHT 3 (TRIPLE ERRORS)
    */
    a[1] = 1; a[2] = 2; a[3] = 3;
    temp = arr2int(a,3);
    decoding_table[get_syndrome(temp)] = temp;
    for (i=1; i<1771; i++) 
	{
        nextcomb(23,3,a);
        temp = arr2int(a,3);
        decoding_table[get_syndrome(temp)] = temp;
    }
}
long CMIL141Apro::golayDecode(long recd,Ipp16s *decoding_table)
{
	long data,m_recd;
	m_recd = recd^decoding_table[get_syndrome(recd)];
	data = (m_recd>>11);
	return data;
}

