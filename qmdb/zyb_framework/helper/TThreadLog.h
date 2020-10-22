//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TThreadLog.h
// Author: Li.ShuGang
// Date: 2008/11/12
// Description: �̰߳�ȫ����־��
////////////////////////////////////////////////
/*
* $History: TThreadLog.h $
 * 
 * *****************  Version 1.0  ***************** 
*/

#ifndef __THREAD_SAFE_LOG_H__
#define __THREAD_SAFE_LOG_H__

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <string>
#include <sstream>
#include "Mutex.h"
#include "ReadIni.h"
using namespace std;

#define  TLOG_NORMAL       0     //������־, ������ζ���¼(�����ļ�(����)��)
#define  TLOG_FLOW         1     //��ϸ��־(��¼��)
#define  TLOG_FUNC         2     //һ�������־, ����������ù���
#define  TLOG_DETAIL       3     //��ϸ���Լ�¼, �������ʱ����

#define  TLOG_WARNING 100     //һ��澯(���ݴ���)
#define  TLOG_FATAL   200     //���ظ澯(ϵͳ��Դ����, �������ʧ��)

#define MAX_THREAD_COUNTS 32
#define MAX_BUSINESS_NAME_LEN 32
#define MAX_PATH_NAME_LEN 256
#define MAX_LOG_BUFFER_LEN 10240


class TTLInfo
{
public:
	TTLInfo();
	~TTLInfo();
	
	void Clear();
	
public:
	int iThreadID;
	int  m_iNowLogLevel;
	
	bool bTraceFlag;
	
	char sInfo[MAX_BUSINESS_NAME_LEN];
	char sPath[MAX_PATH_NAME_LEN];	
	char m_pszFileName[MAX_PATH_NAME_LEN];
	char m_pszLogTemp[MAX_LOG_BUFFER_LEN];
	
	FILE *m_fp;
};


//ϵͳ��־��
class TThreadLog 
{
public:
	static TThreadLog* GetInstance();
    void Release();
    
public:
    void Start(const char* sPathName, const char * sAppName, int iLevel, bool bFlag);
    void End();
    
    void Log(const char * fmt, ...);	
    //void Log(const char* pszFileName, int iLineNumber, const char * fmt, ...);	
	
    void SetLogLevel(int iLevel);
    void SetNowLogLevel(int iLevel);
    void SetLogSize(int iLogSize);
    void SetShowConsole(bool bFlag);
    void SetTrace(bool bFlag);
    void SetTraceInfo(const char* pszInfo);

    int  GetLogLevel();
    int  GetLogSize() ;
    FILE *GetLogFile(void);

private:
	TThreadLog();
	TThreadLog(const TThreadLog& log) { }
    ~TThreadLog();    
    
    long GetFileSize(char * sFullPathFileName);    
    bool CheckAndBackup(const char *sFullPathFileName , FILE*& fp);
    bool IsExist(const char *sFileName);
    int  GetID();
    void GetLevelName(int iLogLevel, char sInfo[]);
    
    FILE* GetFile();
    
    const char *GetCurrentTimeStr(char sTime[]=NULL,size_t iLen=0,const bool bLongFlag=false);

private:
    bool m_bShowConsole;  //�Ƿ�����Ļ��ӡ  
    int  m_iLogLevel;     //�趨����־�ļ���
    int  m_iLogSize;      //�趨LOG�ļ����Ĵ�С����λΪM����������˴�С���򱸷��ļ�
    TReadIni *m_pIni;     //���ö�ȡ
    
    static TThreadLog* volatile m_pInstance;      //����ʵ��
    static TMUTEX m_tMutex;
    
    TTLInfo *m_ptInfo[MAX_THREAD_COUNTS];  //ÿ���̵߳���Ϣ
};



//�Ƿ�ʹ����־,TADD == Thread Add
#ifndef _DEBUG
    #define TIS_DEBUG(x) if(false)
#else
	#define TIS_DEBUG(x) if(true)
#endif
    
#define TRACE_START(x) TThreadLog::GetInstance()->IsTrackFlag = (x) 
#define TADD_START(sPathName,sAppName,iLevel,bFlag) TThreadLog::GetInstance()->Start(sPathName, sAppName, iLevel, bFlag)
#define TADD_SET_LOG_LEVEL(iLevel) TThreadLog::GetInstance()->SetLogLevel(iLevel)
        	
#define TADD_ERROR TThreadLog::GetInstance()->SetNowLogLevel(TLOG_FATAL), TThreadLog::GetInstance()->Log   
				
#define TADD_WARNING TThreadLog::GetInstance()->SetNowLogLevel(TLOG_WARNING), TThreadLog::GetInstance()->Log

#define TADD_NORMAL if(0 <= TThreadLog::GetInstance()->GetLogLevel()) \
		TThreadLog::GetInstance()->SetNowLogLevel(0),TThreadLog::GetInstance()->Log   

#define TADD_FLOW  if(1 <= TThreadLog::GetInstance()->GetLogLevel()) \
		TThreadLog::GetInstance()->SetNowLogLevel(1),TThreadLog::GetInstance()->Log   
	
#define TADD_FUNC if(2 <= TThreadLog::GetInstance()->GetLogLevel()) \
		TThreadLog::GetInstance()->SetNowLogLevel(2),TThreadLog::GetInstance()->Log

#define TADD_DETAIL if(3 <= TThreadLog::GetInstance()->GetLogLevel())  \
		TThreadLog::GetInstance()->SetNowLogLevel(3),TThreadLog::GetInstance()->Log

#define IS_LOG(x) if(x <= TThreadLog::GetInstance()->GetLogLevel())

/*				
#define TADD_FLOW  if( (1<=TThreadLog::GetInstance()->GetLogLevel()) || \
	(TThreadLog::GetInstance()->IsTrackFlag && 1<=TThreadLog::GetInstance()->GetTrackLogLevel()) ) \
		TThreadLog::GetInstance()->SetNowLogLevel(1),TThreadLog::GetInstance()->Log   
	
#define TADD_FUNC if((2<=TThreadLog::GetInstance()->GetLogLevel()) || \
	(TThreadLog::GetInstance()->IsTrackFlag && 2<=TThreadLog::GetInstance()->GetTrackLogLevel()) ) \
		TThreadLog::GetInstance()->SetNowLogLevel(2),TThreadLog::GetInstance()->Log

#define TADD_DETAIL if((3<=TThreadLog::GetInstance()->GetLogLevel()) || \
	(TThreadLog::GetInstance()->IsTrackFlag && 3<=TThreadLog::GetInstance()->GetTrackLogLevel()) ) \
		TThreadLog::GetInstance()->SetNowLogLevel(3),TThreadLog::GetInstance()->Log
*/

#endif //__THREAD_SAFE_LOG_H__
