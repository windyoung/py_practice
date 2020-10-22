//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: BillLog.h
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: 日志类
////////////////////////////////////////////////

#ifndef __BILL_LOG_H_
#define __BILL_LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <string>
#include <sstream>
#include "Common.h"
#include "TException.h"
#include "TThreadBase.h"
using namespace std;

#define  LOG_NORMAL       0     //正常日志, 无论如何都记录(处理文件(批量)级)
#define  LOG_DETAIL       1     //详细日志(记录级)
#define  LOG_DEBUG        2     //一般调试日志, 输出函数调用过程
#define  WARNING_NORMAL 100     //一般告警(数据错误)
#define  WARNING_FATAL  200     //严重告警(系统资源不足, 程序断言失败)
#define  FATAL 300

//不判断debug、release、日志级别 必然会输出的日志NORMAL，WARN，ERROR，FATAL
#define ADD_NORMAL   if(gpBillLog!=NULL) gpBillLog->SetNowLevel(LOG_NORMAL),       gpBillLog->LogNoCode
#define ADD_WARN     if(gpBillLog!=NULL) gpBillLog->SetNowLevel(WARNING_NORMAL),   gpBillLog->Warning
#define ADD_ERROR    if(gpBillLog!=NULL) gpBillLog->SetNowLevel(WARNING_FATAL),    gpBillLog->Warning
#define ADD_FATAL    if(gpBillLog!=NULL) gpBillLog->SetNowLevel(FATAL),            gpBillLog->Fatal

//ZMP:1073352  性能优化日志输出控制
//此处添加条件需要慎重考虑，不能过分影响日志判断逻辑，否则会影响打印性能
//release生产模式，取消IsOneTrackFlag单号码控制 统一用IsTrackFlag来控制，减少判断次数数
//release生产模式，支持-l(离线),ostool(在线) 40012、40015、40011开启或关闭全局/单号码日志
//方便日常开发测试，debug编译下支持配置文件生效日志级别
#ifdef _DEBUG
#define ADD_TRACE    if(gpBillLog!=NULL&&0<gpBillLog->LogLevel) gpBillLog->SetNowLevel(LOG_DETAIL),gpBillLog->LogNoCode   
#define ADD_DEBUG_   if(gpBillLog!=NULL&&1<gpBillLog->LogLevel) gpBillLog->SetNowLevel(LOG_DEBUG),gpBillLog->LogNoCode
#else
#define ADD_TRACE    if(TBillLog::IsTrackFlag&&gpBillLog!=NULL&&0<gpBillLog->LogLevel) gpBillLog->SetNowLevel(LOG_DETAIL),gpBillLog->LogNoCode   
#define ADD_DEBUG_   if(TBillLog::IsTrackFlag&&gpBillLog!=NULL&&1<gpBillLog->LogLevel) gpBillLog->SetNowLevel(LOG_DEBUG),gpBillLog->LogNoCode
#endif

//是否使用日志
#ifndef _DEBUG

#define IS_DEBUG(x) if(false) 

#else

//是否使用日志
#define IS_DEBUG(x)   if( (gpBillLog!=NULL) && (TBillLog::IsTrackFlag || (x) <= gpBillLog->LogLevel ) ) 

#endif
//不区分release和debug只根据日志级别判断
#define IS_PRINT(x)   if( (gpBillLog!=NULL) && ( (x) <= gpBillLog->LogLevel ) ) 

class TBillClientBase {   
public :
    /*写日志*/
    virtual void WriteLog(int iLogLevel,int iLogType,char *sLogMessage){};
    //94913 begin
    /*写日志*/
    virtual void WriteLog(int iLogLevel,const char *iLogType,char *sLogMessage){};
    //end
    /*写警告*/
    virtual void WriteAlarm(int iAlarmLevel,int iAlarmType,const char *sAlarmMessage){};
    //94913 begin
    /*写警告*/
    virtual void WriteAlarm(int iAlarmLevel,const char *iAlarmType,const char *sAlarmMessage){};
    //end
};

//系统日志类
class TBillLog : public TThreadBase
{
public:
    TBillLog();
    virtual ~TBillLog();
public:
    void Start(const char * sAppName,const int iLevel,const bool bFlag, bool bOutToFileFlag = true);
    void End(void);
public:
    void Write(const char *sFileName,int iLineNumber,const int iLogLevel,const int iLogCode,const char * fmt, ...);
    void Log(const char *sFileName,int iLineNumber,const int iLogLevel, const int iLogCode, const char * fmt, ...);
    void Write(const int iLogCode,const char * fmt, ...);
    void Log(const int iLogCode, const char * fmt, ...);
    void Log(const char * fmt, ...);

    //为实现日志编码唯一性，重载以下函数: begin UR:75275
    void Log(const char * sPrimaryModuleCode, const char * sSubModuleCode, const int iLogCode, const char * fmt, ...);
    void Log(const char *sFileName,int iLineNumber,const int iLogLevel, const char * sPrimaryModuleCode, const char * sSubModuleCode,const int iLogCode, const char * fmt, ...);
    //为实现日志编码唯一性，重载以下函数: end UR:75275
public:
    void        SetMonitor(TBillClientBase *pMonitor) {m_Monitor=pMonitor;}
    void        SetLogLevel(const int iLevel);
    void        SetOneTrackLogLevel(const int iLevel);
    void        SetLogSize(const int iLogSize);     //设置为M,比如设置5,就为5*1024*1025
    void        SetNowLevel(const int iLevel);
    void        SetShowConsole(const bool bFlag);
    void        SetMsgFlag(const bool bFlag);
    void        SetLogPath(const char *strLogPath);
    const char *GetLogPath();
    const char *GetFileName();

    //94046 begin
    const char *GetFatalFileName();
    //end

    int         GetLogLevel();
    int         GetLogSize() ;                     //如果为LogSize:5*1024*1025 那返回就为5
    FILE       **GetLogFile(void);

    //94046 begin
    FILE       *GetFatalFile(void);
    //end

    const char *GetMessages();//取出日志列表
    const char *GetLastMessage();//取出最后日志列表
    void        ClearMessage();
public:
    static llong GetFileSize(char * sFullPathFileName);
    static bool CheckAndBackup(const char *sFullPathFileName , FILE *& fp);

    //143237 begin
    static bool CheckAndBackup(const char *sFullPathFileName , FILE *& fp, FILE *& fp2, int iFlag );
    //143237 end

    static void GetLevelName(const int iLogLevel,char sInfo[]);
    static bool IsExist(const char *sFileName);
private:  
    void SetLastMessages(const char *sLastMessage);
 
public:
    int  LogLevel;    
    int  TrackLogLevel;
    int  OneTrackLogLevel;

private:
    char m_AppName[80];
    char m_FileName[256];
    char m_LogPath[256];
    bool m_bShowConsole;
    bool m_bOutToFileFlag;
    FILE *m_fp;

    //143237 begin
    FILE *m_ulog_base_fp;
    FILE *m_fatal_base_fp;
    //143237 end

    //94046 begin
    FILE *m_fpFatal;
    char m_FileNameFatal[256];
    //end

    TBillClientBase *m_Monitor;
    string m_MsgList;
    string m_LastMsg;
    bool m_bMsgFlag;
    char m_sLogTemp[10240];
    int  m_iLogLevel;//当前日志的级别
    //----------------------------------------------------------
    //127501 begin
    static long m_lEstiLogSize;//日志文件估算大小
    static long m_lEstiFatalSize;//错误文件估算大小
    //end
    //----------------------------------------------------------
    //单号码单独日志
    char m_FileNameFatalTrack[256];
    char m_FileNameTrack[256];
    
    FILE *m_fpTrack;
    FILE *m_fpFatalTrack;
public:
    //----------------------------------------------------------
    //127501 begin
    static long getEstiLogSize();//得到日志文件估算大小
    static void incEstiLogSize(int iSize);//增加日志文件估算大小
    //end
    //----------------------------------------------------------

    static void setEstiLogSize(int iSize);//ZMP:305593,设置日志文件估算大小
    //是否需要跟踪
    static bool IsTrackFlag;
    //static bool IsOneTrackFlag; //单号码跟踪标记
    static bool IsBigEndian;     //表示当前主机是否是高字节在前    
    static long lLogSize ;//设定LOG文件最大的大小，单位为字节，如果超出此大小，则备份到.old文件

    //94913 begin
    void LogNoCode(const char * fmt, ...);
    void Write(const char *sFileName,int iLineNumber,const int iLogLevel,const char *iLogCode,const char * fmt, ...);
    void Log(const char *sFileName,int iLineNumber,const int iLogLevel, const char *iLogCode, const char * fmt, ...);
    void Warning(const char *sFileName,int iLineNumber,const int iWarningLevel, const char *iWarningCode, const char * fmt, ...);
    void Warning(const char *iWarningCode, const char * fmt, ...);

    void Write(const char *iLogCode,const char * fmt, ...);

    void Warning(const char *sFileName,int iLineNumber,const int iWarningLevel, const int iWarningCode, const char * fmt, ...);
    void Warning(const int iWarningCode, const char * fmt, ...);

    void Log(const char * sPrimaryModuleCode, const char * sSubModuleCode, const char *iLogCode, const char * fmt, ...);
    void Log(const char *sFileName,int iLineNumber,const int iLogLevel, const char * sPrimaryModuleCode, const char * sSubModuleCode,const char *iLogCode, const char * fmt, ...);

    void Fatal(const char *sFileName,int iLineNumber,const int iWarningLevel, const char *iWarningCode, const char * fmt, ...);
    void Fatal(const char *iWarningCode, const char * fmt, ...);
    //end
    void CreateTraceLogFile(const char *sFileName);
    void CloseTraceLogFile();
    
    void LogTrack(const char *sFileName,const int iLogLevel, const char * fmt, ...);    
    void FatalTrack(const char *sFileName,const int iLogLevel, const char * fmt, ...);
    
    //900693 单独线程写日志begin
public:
    virtual int svc();
    void SetUseThread(bool bUseThread);
private:
    void CreateLogFile(const char * sAppName,const int iLevel);
    void CloseLogFile();
    void StartThread();
    void StopThread();
    
private:
    bool m_bUseThread;
    bool m_bThreadRunning;    //线程运行中
    bool m_bThreadEnded;      //线程已停止
    
    char ** m_pThreadLogBuf;  //线程写日志的缓冲区
    unsigned long m_iThreadLogToBufCnt; //写到缓冲区的总量
    unsigned long m_iThreadLogToFileCnt; //从缓冲区输出到文件的总量
    //900693 单独线程写日志end
};

//写日志函数
void WriteLog(const char *sFileName,const char * fmt, ...);

#endif

