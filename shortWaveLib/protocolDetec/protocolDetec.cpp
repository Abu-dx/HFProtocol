
// 必须首先引入 MFC 头文件，确保 AFX_EXT_CLASS 宏正确定义
// afx.h 和 Windows.h 头文件需在调制协议头文件之前加载，否则
// 会出现变量类型未定义的报错
#include <afx.h>
#include <Windows.h>


// IPP 计算库头文件
#include "ipps.h"
#include <atlconv.h>

#include <iostream>
#include <fstream>

// 导入 ProtocolDetect
#include "ProtolDetect.h"
#pragma comment(lib, "ProtolDetect.lib")


// #define IN_FILE_NAME	"D:/0_kunshan/短波数据db/STANAG-4529-uncoded.wav"
#define IN_FILE_NAME	"D:/0_kunshan/短波数据db/M110B.wav"

// 全局结构体定义（匹配原逻辑，窄字符）
struct OutList
{
	std::string sigType;
	std::string BeginTime;
	std::string EndTime;
	double frequency;
	double hfFre;
	int dataRate;
	int interLeng;
	std::string interType;
	int PU;
	std::string frameType;
	int CRC;
	std::string encrypt;
	std::string message;
} gl_OutList;



// SampleToTime函数（窄字符版本，格式化时间）
std::string SampleToTime(DWORD sample, float sampleRate)
{
	double time = static_cast<double>(sample) / sampleRate;
	char szTime[64] = { 0 };
	sprintf(szTime, "%.6f", time);  // 窄字符格式化
	return std::string(szTime);
}




BOOL selprotolName[10];
int selprotolNum = 8;

int main()
{
	// 打开 .wav 音频文件
	std::ifstream fin(IN_FILE_NAME, std::ios::binary | std::ios::in);
	if (!fin.is_open())
	{
		printf("Open input file failed!\n");
		return - 1;
	}

	// 获取文件长度（替换GetFileSizeEx）
	fin.seekg(0, std::ios::end);
	ULONG fileSizeBytes = static_cast<ULONG>(fin.tellg());
	fin.seekg(0, std::ios::beg);
	ULONG m_FileLength = fileSizeBytes / sizeof(short);

	DWORD dataHavePro = 0;  // 已处理的样本数
	int nProgressPos = 0;

	// 配置协议过滤数组(全检测)
	for (int i = 0; i < selprotolNum; i++) {
		selprotolName[i] = true;
	}


	// 初始化检测器
	int nLeng = 8192;
	float InSample = 9.6e3;
	float Outsample = 9.6e3;
	Ipp32f THD = 0.45f;

	CProtolDetect m_protocolDetect;
	m_protocolDetect.ProtolDetect_ini(nLeng, InSample, selprotolName, selprotolNum);
	
	short* data = (short*)malloc(nLeng * sizeof(short));

	while (1)
	{
		// 读取nLeng个short样本（二进制）
		fin.read(reinterpret_cast<char*>(data), nLeng * sizeof(short));
		// 获取实际读取的字节数 → 转换为样本数
		DWORD nBytesRead = static_cast<DWORD>(fin.gcount()) / sizeof(short);

		// 读取到文件末尾，终止循环
		if (nBytesRead == 0) break;

		// 启动协议检测
		ProtocolOut result;
		BOOL detect = m_protocolDetect.ProtolDetect(
			data,
			nLeng,
			THD,
			selprotolName,
			selprotolNum,
			result
		);


		// 检测到目标信号
		if (detect)
		{
			gl_OutList.BeginTime = SampleToTime(__int64(dataHavePro * 9600.0 / InSample) + result.index, Outsample);
			gl_OutList.sigType = result.ProtocolName.GetString();
			gl_OutList.dataRate = result.dataRate;
			gl_OutList.interLeng = result.InterLen;
			gl_OutList.frequency = result.frequency;

			std::cout << "检测到感兴趣信号: " << std::endl;
			std::cout << gl_OutList.sigType <<
				" beginTime=" << gl_OutList.BeginTime <<
				" fc=" << gl_OutList.frequency << std::endl;
		}

		dataHavePro += nBytesRead;
	}


	m_protocolDetect.ProtolDetect_free();
	free(data);
	return 0;
}
