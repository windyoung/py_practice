//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#ifndef _COMMON_DEF_H_
#define _COMMON_DEF_H_


#include <cstdlib>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cerrno>
using namespace std;

#define MaxPerfStatePointNbr 512 //���֧�ֶ��ٸ����ܼ���

#define U_INT32_MAX_VALUE	0xFFFFFFFF /* 4294967295 */


#ifdef  _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif  //_UNIX


//////////////////////////////////////////////////////////////////////////
#ifndef _ORACLE
#define _ORACLE
#endif

#if (_MSC_VER >= 1400) // VC8+
#pragma warning(disable : 4996) // Either disable all deprecation warnings,
#endif // VC8+

using namespace std;

//����ϵͳ:  _SUNOS,_HPUX,_AIX,_TRUE64, _WIN32,_LINUX
//���ݿ�:    _INFORMIX, _ORACLE
//������:    _GNUC, _MSVC, _BORLANDC

#if defined(WIN32) || defined(_WIN32)
#define __OS_WINDOWS__
#endif

//����ϵͳʶ��
#if defined(_SUN)||defined(_HPUX)||defined(_AIX)||defined(_LINUX)||defined(_IBM)||defined(_HP)
#define __OS_UNIX__
#endif

//·���ָ�����
#if defined(__OS_WINDOWS__)
#define PATH_DELIMITATED_CHAR '\\'
#define PATH_DELIMITATED      "\\"
#else
#define PATH_DELIMITATED_CHAR '/'
#define PATH_DELIMITATED      "/"
#endif


//Ӧ�ó����ܷ���ֵ����
#define SUCCESS_APP          0
#define ERROR_PARAMS        -1
#define ERROR_CONFIG        -2
#define ERROR_SHMTABLE      -3
#define ERROR_INIFILE       -4
#define ERROR_DBCONN        -5
#define ERROR_MONITOR       -6
#define ERROR_ALREADY_INIT  -7
#define ERROR_NOT_INIT      -8
#define ERROR_APP           -9

#define MAX_PATH_FILE       255
#define DATE_TIME_SIZE      15   //����ʱ���ʽ��СYYYYMMDDHHMISS
#define DATE_YEAR_SIZE      4    //�������ʽΪYYYY
#define DATE_MONTH_SIZE     4    //�����¸�ʽΪMM
#define DATE_DAY_SIZE       4    //�����ո�ʽΪDD
#define DATE_SIZE  8    //format: YYYYMMDD

#define MANAGER_NAME_SIZE 256    //zmp:844999


#ifdef _MSC_VER
#pragma warning(disable: 4290)
#pragma warning(disable: 4267)
#pragma warning(disable: 4345)
#pragma warning(disable: 4244)
#pragma warning(disable: 4819)
#pragma warning(disable: 4996)
#endif

//����llong����
#ifndef llong
#define llong long
#endif


//����INT64/UNIT64����
#if defined WIN32
//typedef  unsigned __int64    UINT64;
//typedef  signed __int64      INT64;
#define  I64MFT              "%I64d"
#elif defined LONG32
//typedef  unsigned long long  UINT64;
//typedef  long long           INT64;
#define  I64MFT              "%lld"
#elif defined LONG64
//typedef  unsigned long       UINT64;
//typedef  long                INT64;
#define  I64MFT              "%ld"
#endif

//@Deprecated,Suggest Useing Inline Function
#ifndef ABS
#define ABS(n)      ((n)>=0?(n):-(n))
#endif

#ifndef FLOAT_PRECISION
#define FLOAT_PRECISION         0.0000001 
#endif

inline double FABS(double dParm)
{
    return ((dParm) >= FLOAT_PRECISION?(dParm):-(dParm));
}
inline llong LABS(llong lParm)
{
    return ((lParm) >= 0?(lParm):-(lParm));
}
inline int IABS(int iParm)
{
    return ((iParm) >= 0?(iParm):-(iParm));
}

//@Deprecated,Suggest Useing Inline Function
#ifndef MIN
#define MIN(a,b)    (((a)<(b))?(a):(b))
#endif 

inline llong LMIN(llong lParmA,llong lParmB)
{
    return (((lParmA)<(lParmB))?(lParmA):(lParmB));
}
inline int IMIN(int iParmA,int iParmB)
{
    return (((iParmA)<(iParmB))?(iParmA):(iParmB));
}

//@Deprecated,Suggest Useing Inline Function
#ifndef MAX
#define MAX(a,b)    (((a)>(b))?(a):(b))
#endif 
inline llong LMAX(llong lParmA,llong lParmB)
{
    return (((lParmA)>(lParmB))?(lParmA):(lParmB));
}
inline int IMAX(int iParmA,int iParmB)
{
    return (((iParmA)>(iParmB))?(iParmA):(iParmB));
}


//@Deprecated,Suggest Useing Inline Function
#ifndef SIGN
#define SIGN(a,b)       ((a)==(b)?(0):((a)<(b))?(-1):(0))
#endif 
inline int LSIGN(llong lParmA,llong lParmB)
{
    return ((lParmA)==(lParmB)?(0):((lParmA)<(lParmB))?(-1):(0));
}
inline int ISIGN(int iParmA,int iParmB)
{
    return ((iParmA)==(iParmB)?(0):((iParmA)<(iParmB))?(-1):(0));
}

#ifndef _STR
#ifndef __STR
#define __STR(s) #s
#endif
#define _STR(s) __STR(s)
#endif

//�ײ���־��

#define LOG_CODE_APPLICATION             10000
#define LOG_CODE_HELPER                  20000
#define LOG_CODE_DBC                     30000
#define LOG_CODE_ODBC                    40000
#define LOG_CODE_PUBLIC                  50000

//��ִ�г���Ӧ�ô���
#define LOG_PROC_CA                     PROC_CA_LOG_CODE+100
#define WARN_PROC_CA                    PROC_CA_LOG_CODE+200

//PUBLIC
#define PROC_CA_LOG_CODE                LOG_CODE_PUBLIC+1000
#define BILL_LOG_CODE                   LOG_CODE_PUBLIC+2000
#define COMPONENT_LOG_CODE              LOG_CODE_PUBLIC+3000



//////////////////////////////////////////////////////////////////////////
#define OBJECT_NAME_SIZE 40

/*SQLӦ�ú궨��*/
#ifdef  _ORACLE
#define SYSDATE             " SYSDATE "
#define TRUNC_SYSDATE       " TRUNC(SYSDATE) " 
#define DAY(strValue)        "(" strValue ")"
#define DATE_TIME_FORMAT    "YYYYMMDDHH24MISS"
#define DATE_FORMAT            "YYYYMMDD"
//#define MIN_DATETIME        "TO_DATE('19710101000000','YYYYMMDDHH24MISS')"
//#define MAX_DATETIME        "TO_DATE('21000101000000','YYYYMMDDHH24MISS')"
#define MIN_TIME_STR_SQL      "TO_DATE('19700102000000','YYYYMMDDHH24MISS')"
#define MAX_TIME_STR_SQL      "TO_DATE('20360102000000','YYYYMMDDHH24MISS')" //���ܳ���32λ�޷����������ܱ�ʶ��������ڣ���1900�꿪ʼ��
#endif  //_ORACLE

#define MIN_TIME_T          172800     //����StringToGMTime("19700102000000" + 1day)
#define MAX_TIME_T          2082758400 //����StringToGMTime("20360102000000" - 1day)

#ifdef  _INFORMIX
#define SYSDATE             " CURRENT "
#define TRUNC_SYSDATE       " TODAY "
#define DAY(strValue)        "(" strValue ")"
#define DATE_TIME_FORMAT    "%Y%m%d%H%M%S"
#define DATE_FORMAT            "%Y%m%d"
//#define MIN_DATETIME        " EXTEND(TO_DATE('1971','%Y'),YEAR TO SECOND) "
//#define MAX_DATETIME        " EXTEND(TO_DATE('2100','%Y'),YEAR TO SECOND) "
#define MIN_TIME_STR_SQL      " EXTEND(TO_DATE('1970','%Y'),YEAR TO SECOND) "
#define MAX_TIME_STR_SQL      " EXTEND(TO_DATE('2036','%Y'),YEAR TO SECOND) " //���ܳ���32λ�޷����������ܱ�ʶ��������ڣ���1900�꿪ʼ��
#endif  //_INFORMIX

namespace RECORD_TYPE 
{
    const int ALL_TYPE         = 0;//˳���ύ��ʽ
    const int AUTO_UPDATE_TYPE = 1;//�Զ��ύ��ʽ(Insert->Update->Delete)
    const int INSERT_RECORD    = 2;//���ύ��������
    const int UPDATE_RECORD    = 3;//���ύ�޸�����
    const int REMOVE_RECORD    = 4;//���ύɾ������
}

static const char * const RECORD_TYPE_STR[5]=
{
    "ALL_TYPE",
    "AUTO_UPDATE_TYPE",
    "INSERT_RECORD",   
    "UPDATE_RECORD",   
    "REMOVE_RECORD"   
};

static const char * const BOOL_STR[]=
{
    "FALSE",
    "TRUE "
};

static const char * const IS_NULL[]=
{
    "NULL    ",
    "NOT NULL"
};


#define DATA_PATH_SIZE    2  //ÿ������ռ2���ַ�
#define DATA_PATH_COUNT   2  //ÿ������2λ���ݣ����99����ͬ�����,Ҫ��999��ô����3,9999��ô����4λ

#define ODB_EXCEPTION   "ORACLE_DB_EXCEPTION"
#define TDB_EXCEPTION   "TTDB EXCEPTION"
#define MDB_EXCEPTION   "MDB_EXCEPTION"
#define EXCEPTION       "EXCEPTION"
#define OTHER_EXCEPTION "OTHER EXCEPTION"

#endif  //_COMMON_DEF_H_
