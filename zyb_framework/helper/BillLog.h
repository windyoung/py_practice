//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: BillLog.h
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: ��־��
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

#define  LOG_NORMAL       0     //������־, ������ζ���¼(�����ļ�(����)��)
#define  LOG_DETAIL       1     //��ϸ��־(��¼��)
#define  LOG_DEBUG        2     //һ�������־, ����������ù���
#define  WARNING_NORMAL 100     //һ��澯(���ݴ���)
#define  WARNING_FATAL  200     //���ظ澯(ϵͳ��Դ����, �������ʧ��)
#define  FATAL 300

//���ж�debug��release����־���� ��Ȼ���������־NORMAL��WARN��ERROR��FATAL
#define ADD_NORMAL   if(gpBillLog!=NULL) gpBillLog->SetNowLevel(LOG_NORMAL),       gpBillLog->LogNoCode
#define ADD_WARN     if(gpBillLog!=NULL) gpBillLog->SetNowLevel(WARNING_NORMAL),   gpBillLog->Warning
#define ADD_ERROR    if(gpBillLog!=NULL) gpBillLog->SetNowLevel(WARNING_FATAL),    gpBillLog->Warning
#define ADD_FATAL    if(gpBillLog!=NULL) gpBillLog->SetNowLevel(FATAL),            gpBillLog->Fatal

//ZMP:1073352  �����Ż���־�������
//�˴����������Ҫ���ؿ��ǣ����ܹ���Ӱ����־�ж��߼��������Ӱ���ӡ����
//release����ģʽ��ȡ��IsOneTrackFlag��������� ͳһ��IsTrackFlag�����ƣ������жϴ�����
//release����ģʽ��֧��-l(����),ostool(����) 40012��40015��40011������ر�ȫ��/��������־
//�����ճ��������ԣ�debug������֧�������ļ���Ч��־����
#ifdef _DEBUG
#define ADD_TRACE    if(gpBillLog!=NULL&&0<gpBillLog->LogLevel) gpBillLog->SetNowLevel(LOG_DETAIL),gpBillLog->LogNoCode   
#define ADD_DEBUG_   if(gpBillLog!=NULL&&1<gpBillLog->LogLevel) gpBillLog->SetNowLevel(LOG_DEBUG),gpBillLog->LogNoCode
#else
#define ADD_TRACE    if(TBillLog::IsTrackFlag&&gpBillLog!=NULL&&0<gpBillLog->LogLevel) gpBillLog->SetNowLevel(LOG_DETAIL),gpBillLog->LogNoCode   
#define ADD_DEBUG_   if(TBillLog::IsTrackFlag&&gpBillLog!=NULL&&1<gpBillLog->LogLevel) gpBillLog->SetNowLevel(LOG_DEBUG),gpBillLog->LogNoCode
#endif

//�Ƿ�ʹ����־
#ifndef _DEBUG

#define IS_DEBUG(x) if(false) 

#else

//�Ƿ�ʹ����־
#define IS_DEBUG(x)   if( (gpBillLog!=NULL) && (TBillLog::IsTrackFlag || (x) <= gpBillLog->LogLevel ) ) 

#endif
//������release��debugֻ������־�����ж�
#define IS_PRINT(x)   if( (gpBillLog!=NULL) && ( (x) <= gpBillLog->LogLevel ) ) 

class TBillClientBase {   
public :
    /*д��־*/
    virtual void WriteLog(int iLogLevel,int iLogType,char *sLogMessage){};
    //94913 begin
    /*д��־*/
    virtual void WriteLog(int iLogLevel,const char *iLogType,char *sLogMessage){};
    //end
    /*д����*/
    virtual void WriteAlarm(int iAlarmLevel,int iAlarmType,const char *sAlarmMessage){};
    //94913 begin
    /*д����*/
    virtual void WriteAlarm(int iAlarmLevel,const char *iAlarmType,const char *sAlarmMessage){};
    //end
};

//ϵͳ��־��
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

    //Ϊʵ����־����Ψһ�ԣ��������º���: begin UR:75275
    void Log(const char * sPrimaryModuleCode, const char * sSubModuleCode, const int iLogCode, const char * fmt, ...);
    void Log(const char *sFileName,int iLineNumber,const int iLogLevel, const char * sPrimaryModuleCode, const char * sSubModuleCode,const int iLogCode, const char * fmt, ...);
    //Ϊʵ����־����Ψһ�ԣ��������º���: end UR:75275
public:
    void        SetMonitor(TBillClientBase *pMonitor) {m_Monitor=pMonitor;}
    void        SetLogLevel(const int iLevel);
    void        SetOneTrackLogLevel(const int iLevel);
    void        SetLogSize(const int iLogSize);     //����ΪM,��������5,��Ϊ5*1024*1025
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
    int         GetLogSize() ;                     //���ΪLogSize:5*1024*1025 �Ƿ��ؾ�Ϊ5
    FILE       **GetLogFile(void);

    //94046 begin
    FILE       *GetFatalFile(void);
    //end

    const char *GetMessages();//ȡ����־�б�
    const char *GetLastMessage();//ȡ�������־�б�
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
    int  m_iLogLevel;//��ǰ��־�ļ���
    //----------------------------------------------------------
    //127501 begin
    static long m_lEstiLogSize;//��־�ļ������С
    static long m_lEstiFatalSize;//�����ļ������С
    //end
    //----------------------------------------------------------
    //�����뵥����־
    char m_FileNameFatalTrack[256];
    char m_FileNameTrack[256];
    
    FILE *m_fpTrack;
    FILE *m_fpFatalTrack;
public:
    //----------------------------------------------------------
    //127501 begin
    static long getEstiLogSize();//�õ���־�ļ������С
    static void incEstiLogSize(int iSize);//������־�ļ������С
    //end
    //----------------------------------------------------------

    static void setEstiLogSize(int iSize);//ZMP:305593,������־�ļ������С
    //�Ƿ���Ҫ����
    static bool IsTrackFlag;
    //static bool IsOneTrackFlag; //��������ٱ��
    static bool IsBigEndian;     //��ʾ��ǰ�����Ƿ��Ǹ��ֽ���ǰ    
    static long lLogSize ;//�趨LOG�ļ����Ĵ�С����λΪ�ֽڣ���������˴�С���򱸷ݵ�.old�ļ�

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
    
    //900693 �����߳�д��־begin
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
    bool m_bThreadRunning;    //�߳�������
    bool m_bThreadEnded;      //�߳���ֹͣ
    
    char ** m_pThreadLogBuf;  //�߳�д��־�Ļ�����
    unsigned long m_iThreadLogToBufCnt; //д��������������
    unsigned long m_iThreadLogToFileCnt; //�ӻ�����������ļ�������
    //900693 �����߳�д��־end
};

//д��־����
void WriteLog(const char *sFileName,const char * fmt, ...);

#endif

