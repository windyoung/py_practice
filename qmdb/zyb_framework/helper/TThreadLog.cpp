//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TThreadLog.cpp
// Author: Li.ShuGang
// Date: 2008/11/12
// Description: 线程安全的日志类
////////////////////////////////////////////////
/*
* $History: TThreadLog.hcpp $
 * 
 * *****************  Version 1.0  ***************** 
*/
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <string>
#include <sstream>
#include <errno.h>
#include <sys/stat.h>
#include "TThreadLog.h"
#include "TException.h"
#include "TDateTimeFunc.h"
#include "Common.h"
#include "debug_new.h"


#ifdef  _WIN32
    #include <time.h>
    #include <windows.h>
#else
    #include <unistd.h>
#endif  //_UNIX


#ifndef WIN32
  #include <unistd.h>
#else
  #include <process.h>
  #include <io.h>
  #include <direct.h>
#endif

using namespace std;

//ZMP:467375     
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

#define  LEVEL_NAME_LENGTH  15 //日志等级名称长度


TThreadLog* volatile TThreadLog::m_pInstance = NULL;
TMUTEX TThreadLog::m_tMutex(true);


TTLInfo::TTLInfo()
{
	iThreadID = -1;
	m_fp      = NULL;
	m_iNowLogLevel = 0;
	
	memset(sInfo, 0, sizeof(sInfo));
	memset(sPath, 0, sizeof(sPath));
	memset(m_pszFileName, 0, sizeof(m_pszFileName));
    //ZMP:305593
    memset(m_pszLogTemp, 0, sizeof(m_pszLogTemp));
	
	bTraceFlag = false;
}


TTLInfo::~TTLInfo()
{

}


void TTLInfo::Clear()
{
	iThreadID = -1;
	m_fp      = NULL;
	m_iNowLogLevel = 0;
	
	memset(sInfo, 0, sizeof(sInfo));
	memset(sPath, 0, sizeof(sPath));
	memset(m_pszFileName, 0, sizeof(m_pszFileName));
}


TThreadLog* TThreadLog::GetInstance()
{
	//注意这里采用Double Checked-Locking
	if(m_pInstance == NULL)
	{
		m_tMutex.Lock(true);
		if(m_pInstance == NULL)
			m_pInstance = new TThreadLog();	
		m_tMutex.UnLock(true);
	}
	
	return m_pInstance;
}


TThreadLog::TThreadLog()
{
	//变量初始化
    m_bShowConsole = false;

    m_iLogLevel = 0;  //当前日志的级别
    m_iLogSize  = 50; //设定LOG文件最大的大小，单位为M，如果超出此大小，则备份文件
    
    //取日志路径
    //char *sEnv = getenv("BILL_LOG_PATH");//ZMP:305593,变量未用到
    
    //获取App.config路径
	char sAppCfgName[256];
	memset(sAppCfgName, 0, sizeof(sAppCfgName));
    //ZMP:467375
	snprintf(sAppCfgName, sizeof(sAppCfgName), "%s/etc/App.config", getenv("HOME"));
    sAppCfgName[sizeof(sAppCfgName)-1] = '\0';

    m_pIni = new TReadIni(sAppCfgName);   
    if(m_pIni == NULL)
    {
    	throw TException("TThreadLog::TThreadLog() : Can't Read File[%s].", sAppCfgName);	
    }
    
    //获取日志目录
    char sLogPath[MAX_PATH_NAME_LEN];
    memset(sLogPath, 0, sizeof(sLogPath));
    //ZMP:467375
	snprintf(sLogPath, sizeof(sLogPath), "%s/log", getenv("HOME"));
    sLogPath[sizeof(sLogPath)-1] = '\0';
    m_pIni->ReadString("COMMON", "LOG_PATH", sLogPath, sLogPath);
    
    for(int i=0; i<MAX_THREAD_COUNTS; ++i)
    {
    	m_ptInfo[i] = new TTLInfo();
    	
    	m_ptInfo[i]->Clear();
        //ZMP:467375
        strncpy(m_ptInfo[i]->sPath, sLogPath, sizeof(m_ptInfo[i]->sPath)-1);
        m_ptInfo[i]->sPath[sizeof(m_ptInfo[i]->sPath)-1] = '\0';
		//printf("LOG_PATH=[%s].\n", sLogPath);	    
		
	    //去除最后一个分隔符
	#ifndef _WIN32
	    if (m_ptInfo[i]->sPath[strlen(m_ptInfo[i]->sPath)-1] == '/')
	        m_ptInfo[i]->sPath[strlen(m_ptInfo[i]->sPath)-1] = '\0';
	#else
	    if (m_ptInfo[i]->sPath[strlen(m_ptInfo[i]->sPath)-1] == '\\')
	        m_ptInfo[i]->sPath[strlen(m_ptInfo[i]->sPath)-1] = '\0';
	#endif
    }
}


TThreadLog::~TThreadLog()
{
    End();

    DELETE(m_pIni);
    
    for(int i=0; i<MAX_THREAD_COUNTS; ++i)
    {
        DELETE(m_ptInfo[i]);
    }
}


bool TThreadLog::IsExist(const char *sFileName)
{
    int iRetCode =0;
#ifndef _WIN32
    iRetCode = access(sFileName, F_OK);
#else
    iRetCode = access(sFileName, 0x00);
#endif
    return iRetCode == 0;
}

int TThreadLog::GetID() 
{
#ifdef _WIN32
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}



void TThreadLog::Start(const char* sPathName, const char *sAppName, int iLevel, bool bFlag)
{
	int iThreadID = GetID();
	if(iThreadID >= MAX_THREAD_COUNTS)
	{
		throw TException("TThreadLog::Start(%s) : iThreadID[%d] >= MAX_THREAD_COUNTS[%d].", sAppName, iThreadID, MAX_THREAD_COUNTS);	
	}
	
	m_ptInfo[iThreadID]->iThreadID = iThreadID;
	strncpy(m_ptInfo[iThreadID]->sInfo, sAppName, sizeof(m_ptInfo[iThreadID]->sInfo)-1);
	if(sPathName != NULL)
		strncpy(m_ptInfo[iThreadID]->sPath, sPathName, sizeof(m_ptInfo[iThreadID]->sPath)-1);
	
    char sTemp[64]={0}; //357131 for cppcheck
    GetCurrentTimeStr(sTemp);

#ifndef _WIN32
    //ZMP:467375
	snprintf(m_ptInfo[iThreadID]->m_pszFileName, sizeof(m_ptInfo[iThreadID]->m_pszFileName), "%s/ULOG_%s_%s[%05d-%d]", m_ptInfo[iThreadID]->sPath, sAppName, sTemp, getpid(), GetID());
    m_ptInfo[iThreadID]->m_pszFileName[sizeof(m_ptInfo[iThreadID]->m_pszFileName)-1] = '\0';
#else
	snprintf(m_ptInfo[iThreadID]->m_pszFileName, sizeof(m_ptInfo[iThreadID]->m_pszFileName), "%s\\ULOG_%s_%s[%05d-%d].txt", m_ptInfo[iThreadID]->sPath, sAppName, sTemp, getpid(), GetID());
#endif
	//printf("TThreadLog::Start() %s.\n", m_ptInfo[iThreadID]->m_pszFileName);
    //其他配置
    m_iLogLevel    = iLevel;
    m_bShowConsole = bFlag;

    //生成文件名
    m_ptInfo[iThreadID]->m_fp = fopen(m_ptInfo[iThreadID]->m_pszFileName, "a+");
    if(NULL == m_ptInfo[iThreadID]->m_fp)
    {
        int iError = errno;
        throw TException("TThreadLog::Start(%s) : Open file [%s] fail. errno [%d], error_msg [%s]",
            sAppName, m_ptInfo[iThreadID]->m_pszFileName, iError, strerror(iError));
    }

    //cout << "\n[NORMAL] > TThreadLog::Start() : FILE_NAME=(" << m_ptInfo[iThreadID]->m_pszFileName << ") LOG_LEVEL=[" << m_iLogLevel << "] SHOW_CONSOLE=["<<m_bShowConsole <<"]" <<endl;
    fprintf(m_ptInfo[iThreadID]->m_fp, "===================== < %s - %s [%d]> =====================\n", sTemp, sAppName, m_iLogLevel);
}

void TThreadLog::End()
{
	int iThreadID = GetID();
	if(iThreadID >= MAX_THREAD_COUNTS)
	{
		throw TException("TThreadLog::End() : iThreadID[%d] >= MAX_THREAD_COUNTS[%d].", iThreadID, MAX_THREAD_COUNTS);	
	}
	
    if(m_ptInfo[iThreadID]->m_fp)
    {
        char sTemp[64]={0}; //357131 for cppcheck
        GetCurrentTimeStr(sTemp);

        fprintf(m_ptInfo[iThreadID]->m_fp, "===================== < %s > =====================\n", sTemp);
        fclose(m_ptInfo[iThreadID]->m_fp);
        m_ptInfo[iThreadID]->m_fp = NULL;
    }
}

void TThreadLog::SetLogLevel(int iLevel)
{
    m_iLogLevel = iLevel;
}


void TThreadLog::SetNowLogLevel(int iLevel)
{
	int iThreadID = GetID();
	if(iThreadID >= MAX_THREAD_COUNTS)
	{
		throw TException("TThreadLog::SetNowLogLevel() : iThreadID[%d] >= MAX_THREAD_COUNTS[%d].", iThreadID, MAX_THREAD_COUNTS);	
	}
	
    m_ptInfo[iThreadID]->m_iNowLogLevel = iLevel;
}


int TThreadLog::GetLogLevel()
{
    return m_iLogLevel;
}

void TThreadLog::SetLogSize(int iLogSize)
{
    m_iLogSize = iLogSize ;
}

int TThreadLog::GetLogSize()
{
    return m_iLogSize ;
}


void TThreadLog::SetShowConsole(bool bFlag)
{
    m_bShowConsole = bFlag;
}


void TThreadLog::GetLevelName(int iLogLevel, char sInfo[])
{
    switch(iLogLevel)
    {
    //ZMP:467375
    case TLOG_FLOW       : strncpy(sInfo,"[ FLOW ]",LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    case TLOG_DETAIL     : strncpy(sInfo,"[DETAIL]",LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    case TLOG_FUNC       : strncpy(sInfo,"[ FUNC ]",LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    case TLOG_WARNING    : strncpy(sInfo,"[ WARN ]",LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    case TLOG_FATAL      : strncpy(sInfo,"[ ERROR]",LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    case TLOG_NORMAL     : strncpy(sInfo,"[NORMAL]",LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    default              : strncpy(sInfo,"[ (N/A)]",LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    }
}

/*
void TThreadLog::Log(const char* pszFileName, int iLineNumber, const char * fmt, ...)
{
	int iThreadID = GetID();
	if(iThreadID >= MAX_THREAD_COUNTS)
	{
		throw TException("TThreadLog::Log() : iThreadID[%d] >= MAX_THREAD_COUNTS[%d].", iThreadID, MAX_THREAD_COUNTS);	
	}
	
    char sDate[30], sInfo[15];

    GetCurrentTimeStr(sDate, sizeof(sDate), true);
    GetLevelName(m_ptInfo[iThreadID]->m_iNowLogLevel, sInfo);

    va_list ap;
    va_start(ap,fmt);
    vsnprintf(m_ptInfo[iThreadID]->m_pszLogTemp, sizeof(m_ptInfo[iThreadID]->m_pszLogTemp), fmt, ap); 
    va_end (ap);

    if(m_ptInfo[iThreadID]->m_iNowLogLevel == 0 || m_bShowConsole)
    {
        printf("====%s > %s\n", sInfo, m_ptInfo[iThreadID]->m_pszLogTemp); 
    }	
    
    if(m_ptInfo[iThreadID]->m_fp)
    {
        CheckAndBackup(m_ptInfo[iThreadID]->m_pszFileName, m_ptInfo[iThreadID]->m_fp);
       	fprintf(m_ptInfo[iThreadID]->m_fp, "%s %s > CCBS[%d]: [%s : %d] %s\n", sDate, sInfo, iThreadID, pszFileName, iLineNumber, m_ptInfo[iThreadID]->m_pszLogTemp);
        fflush(m_ptInfo[iThreadID]->m_fp);
    }
}
*/

void TThreadLog::Log(const char * fmt, ...)
{
	int iThreadID = GetID();
	if(iThreadID >= MAX_THREAD_COUNTS)
	{		
		throw TException("TThreadLog::Log() : iThreadID[%d] >= MAX_THREAD_COUNTS[%d].", iThreadID, MAX_THREAD_COUNTS);	
	}
	
    char sDate[30]={0}, sInfo[15]={0}; //357131 for cppcheck

    GetCurrentTimeStr(sDate, sizeof(sDate), true);
   	
   	//memset(m_ptInfo[iThreadID]->m_pszLogTemp, 0, sizeof(m_ptInfo[iThreadID]->m_pszLogTemp));
    	
    GetLevelName(m_ptInfo[iThreadID]->m_iNowLogLevel, sInfo);
    char sLogTemp[10240]={0}; //357131 for cppcheck
    va_list ap;
    va_start(ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap); 
    va_end (ap);

    if(m_ptInfo[iThreadID]->m_iNowLogLevel == 0 || m_bShowConsole)
    {
        printf("[%d] %s > %s\n", iThreadID, sInfo, sLogTemp); 
    }
	    
    if(m_ptInfo[iThreadID]->m_fp)
    {
        CheckAndBackup(m_ptInfo[iThreadID]->m_pszFileName, m_ptInfo[iThreadID]->m_fp);
       	fprintf(m_ptInfo[iThreadID]->m_fp, "%s %s > CCBS[%d]: %s\n", sDate, sInfo, iThreadID, sLogTemp);
        fflush(m_ptInfo[iThreadID]->m_fp);
    }
}


long TThreadLog::GetFileSize(char * sFullPathFileName)
{
    struct stat sbuf;
    if (stat(sFullPathFileName, &sbuf) == 0)
        return sbuf.st_size;
    else 
        return -1;
}


//检查文件是否超出限制，若是，则将其备份为.old文件，并创建一个新的文件
//参数1为日志文件名，参数2为日志文件对应的句柄
bool TThreadLog::CheckAndBackup(const char * sFileName , FILE*& fp)
{
	return true;
	/*
    char     sFileNameOld[255];
    long     filesize = -1 ;
    int      filehandle = -1 ;
    memset(sFileNameOld,0, 255);
    filesize = GetFileSize((char*)sFileName);

    if (filesize == -1) return false;
    
    if (filesize >= m_iLogSize*1024*1024)	
    {
        //超过最长长度,以当前时间YYYYMMDDHHM24SS表示
        char sTemp[64];
        FUNC_DT::GetCurrentTimeStr(sTemp);
        sprintf(sFileNameOld,"%s.%s", sFileName, sTemp);

        fclose(fp);
        TFileOper::Remove(sFileNameOld);

        //将日志文件重命名
        TFileOper::Rename(sFileName,sFileNameOld);

        TFileOper::Remove(sFileName);

#ifndef _WIN32
        filehandle = open(sFileName,O_CREAT|O_EXCL,0666);
        if ( filehandle != -1 ) 
        {
            close( filehandle );
            chmod(sFileName, 0666);
        }
#endif
        fp = fopen (sFileName,"a+");
        if(fp == NULL)
        {
            throw TException("Check And Backup Log File: Open file [%s] fail.",sFileName);
        }
    }
    return true ;*/
}


const char *TThreadLog::GetCurrentTimeStr(char sTime[], size_t iLen, const bool bLongFlag)
{
    time_t tCurrent;
    struct tm *tm_Cur;
    static char sCurtime[40]={0}; //357131 for cppcheck
    sCurtime[39]=0;
    time(&tCurrent); //取得当前时间的time_t值
    tm_Cur = localtime(&tCurrent); //取得当前时间的tm值
    
    if(bLongFlag)
    {
        //ZMP:467375
        snprintf(sCurtime, sizeof(sCurtime), "%04d-%02d-%02d %02d:%02d:%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
        sCurtime[sizeof(sCurtime)-1] = '\0';
    }
    else
    {
        snprintf(sCurtime, sizeof(sCurtime), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
        sCurtime[sizeof(sCurtime)-1] = '\0';
    }

    if(sTime!=NULL)
    {
        if(iLen>0)
        {
            strncpy(sTime,sCurtime,iLen-1);
            sTime[iLen-1]='\0';
        }
        else
        {
            //ZMP:467375  
            int iLength = bLongFlag? 19:14;
            strncpy(sTime,sCurtime,iLength);
            sTime[iLength] = '\0';
        }
    }

    return sCurtime;
}


