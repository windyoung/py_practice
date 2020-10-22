//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TThreadLog.h
// Author: Li.ShuGang
// Date: 2008/11/12
// Description: 线程安全的日志类
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

#define  TLOG_NORMAL       0     //正常日志, 无论如何都记录(处理文件(批量)级)
#define  TLOG_FLOW         1     //详细日志(记录级)
#define  TLOG_FUNC         2     //一般调试日志, 输出函数调用过程
#define  TLOG_DETAIL       3     //详细调试记录, 输出运行时数据

#define  TLOG_WARNING 100     //一般告警(数据错误)
#define  TLOG_FATAL   200     //严重告警(系统资源不足, 程序断言失败)

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


//系统日志类
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
    bool m_bShowConsole;  //是否在屏幕打印  
    int  m_iLogLevel;     //设定的日志的级别
    int  m_iLogSize;      //设定LOG文件最大的大小，单位为M，如果超出此大小，则备份文件
    TReadIni *m_pIni;     //配置读取
    
    static TThreadLog* volatile m_pInstance;      //单个实例
    static TMUTEX m_tMutex;
    
    TTLInfo *m_ptInfo[MAX_THREAD_COUNTS];  //每个线程的信息
};



//是否使用日志,TADD == Thread Add
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
