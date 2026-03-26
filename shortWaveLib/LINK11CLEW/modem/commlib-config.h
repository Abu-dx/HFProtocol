#ifndef _COMMLIB_CONFIG
#define _COMMLIB_CONFIG

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M_PI 3.1415926535897932384626433832795



/* 宏函数 */
#define MEMORY_ALLOCATION_CHECK(x)        \
  do {                                    \
    if (x == NULL) {  fprintf(stderr, "(%s:%d) Error: Memory Allocation Falied!\n", \
                    __FILE__, __LINE__);  \
                    exit(EXIT_FAILURE); } \
  } while(false)

/* 文件描述符检测，若为 NULL 则报错 */
#define FILE_DESCRIPTORS_CHECK(x)              \
  do {                                    \
    if (x == NULL) {  fprintf(stderr, "(%s:%d) Error: File Open Failed!\n", \
                    __FILE__, __LINE__);  \
                    exit(EXIT_FAILURE); } \
  } while (false)

/* 变量合法性检测：若不满足 condition 则报错 */
#define VARIABLE_VALIDITY_CHECK(condition, msg)    \
  do {                                        \
    if (!(condition)) { fprintf(stderr, "(%s:%d) Error: Illegal Variable Operation. %s\n", \
                        __FILE__, __LINE__, #msg);  \
                        exit(EXIT_FAILURE); }        \
  } while(false)



/* 类型别名 */
typedef char CHAR;
typedef long LONG;
typedef short SHORT;
typedef unsigned long DWORD;
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef float FLOAT;
typedef FLOAT *PFLOAT;
// typedef wchar_t TCHAR;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned int *PUINT;

#define CONST const
#define TEXT TEXT
#define ERROR_SUCCESS 0L
#define FALSE 0
#define ERROR_BUFFER_ALL_ZEROS 754L
#define VOID void

#define MAX(a, b) a > b ? a : b
#define MIN(a, b) a < b ? a : b


enum DATATYPE {
  TYPE_INT16,
  TYPE_INT16_CX,
  TYPE_DOUBLE,
  TYPE_DOUBLE_CX
};

typedef enum ERROR_TYPE {
  ERROR_UD_INCORRECT_DDC_PARAMETER = 0,

  ERROR_UNDEFINED_MODULATION_TYPE,

  ERROR_UNDEFINED_DECODER_TYPE,

  ERROR_DEMOD_PARAM,

  ERROR_FILTER_LEN,

  ERROR_PARAM_PROBE_LACK_DATA,
} ERROR_TYPE;



#endif /* _COMMLIB_CONFIG */
