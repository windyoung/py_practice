//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: BillLog.cpp
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: ��־��
////////////////////////////////////////////////

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
#include "BillLog.h"
#include "TException.h"
#include "TDateTimeFunc.h"
#include "TFileOper.h"
#include "TStrFunc.h"
#include "debug_new.h"

#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#include <io.h>
#include <direct.h>
#endif

using namespace std;

#define LOG_TBillLog    	"ZSmart-Public-52100"
#define LOG_TBillLog_INT    52100
#define LEVEL_NAME_LENGTH   15    //ZMP:467369 ,�ȼ����Ƴ���

//94046 begin
int g_argc;
char **g_argv;
//end

#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

//�Ƿ���Ҫ����
bool TBillLog::IsTrackFlag = false;
bool TBillLog::IsBigEndian = false;
llong TBillLog::lLogSize = 30*1024*1024 ; //Ĭ��30M

//----------------------------------------------------------
//127501 begin
long TBillLog::m_lEstiLogSize = 0;
long TBillLog::m_lEstiFatalSize = 0;
//end
//----------------------------------------------------------

//900693
#define THREAD_LOG_LINE_CNT 8192     //�߳�д��־��������
#define THREAD_LOG_LINE_SIZE 4096   //�߳�д��־���г���

//////////////////////////////////////////////////////////////////////////
void WriteLog(const char *sFileName,const char * fmt, ...)
{
    if (TStrFunc::IsEmpty(sFileName))
    {
        cout << "WriteLog() : FileName [" << sFileName << "] is Empty!" << endl;
        return;
    }

    FILE *fp= fopen(sFileName, "a+");
    if (fp == NULL)
    {
        int iError = errno;
        cout << "WriteLog() : FileName [" << sFileName << "] can't open! errno [" << iError << "], error_msg [" << strerror(iError) << "]" << endl;
        return;
    }

    char sDate[30]={0},sInfo[15]={0}; //357111 for cppcheck
    char sLine[10240]={0}; //357111 for cppcheck

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    TBillLog::GetLevelName(0,sInfo);

    va_list ap;
    va_start (ap,fmt);
    try
    {
        vsnprintf(sLine, sizeof(sLine), fmt, ap); 
    }
    catch(...)
    {
    //      printf("Write Buffer Overflow Or Format Paramter Error.\n");
    //      cout << "FormatString : " <<fmt << endl;
    }


    //printf("%s > %s", sInfo,sLine); 
    //BillLog::CheckAndBackup(sFileName, fp);//ZMP:305593,��CheckAndBackup������ֲ������
    //ZMP:305593,begin-----------------------------------------
    char     sFileNameOld[255]= {0};//ZMP:477242
    llong    filesize = -1 ;
    int iRetCode =0;
    bool bIsExist = false;
        
#if defined(__OS_UNIX__)
    iRetCode = access(sFileName, F_OK);
#else
    iRetCode = access(sFileName, 0x00);
#endif
    bIsExist = iRetCode == 0;

    if(!bIsExist)
    {   
        fp = fopen(sFileName, "a+");
        if (NULL == fp)
        {
            int iError = errno;
            throw TException("WriteLog : Open file [%s] fail during rebuild it .errno [%d], error_msg [%s]\n",
                sFileName, iError, strerror(iError));
        }
    }  
    //memset(sFileNameOld,0, 255);

    filesize = TBillLog::getEstiLogSize();

    if (filesize != -1)
    {
        if (filesize >= TBillLog::lLogSize)  
        {
            //���������
            //ZMP:467369
            snprintf(sFileNameOld, sizeof(sFileNameOld), "%s.old", sFileName);
            sFileNameOld[sizeof(sFileNameOld)-1] = '\0';
            fclose(fp);
            fp = NULL;  //357111 for cppcheck
            TFileOper::Remove(sFileNameOld);

            //����־�ļ�������
            TFileOper::Rename(sFileName,sFileNameOld);

            TFileOper::Remove(sFileName);
            TBillLog::setEstiLogSize(0);
#ifndef _WIN32
            int  filehandle = -1 ;
            filehandle = open(sFileName,O_CREAT|O_EXCL,0666);
            if ( filehandle != -1 ) 
            {
                close( filehandle );
                chmod(sFileName, 0666);
            }
#endif
            fp = fopen(sFileName,"a+");
            if(NULL == fp)
            {
                int iError = errno;
                throw TException("Check And Backup Log File: Open file [%s] fail. errno [%d], error_msg [%s]",
                    sFileName, iError, strerror(iError));
            }
        } 
    }
    //ZMP:305593,end--------------------------------------------

    long lPid = getpid();//zmp:971164
    int iSize = fprintf (fp, "%s %s [%ld] > CCBS:%s", sDate,sInfo,lPid,sLine); //zmp:971164
    fflush(fp);
    //----------------------------------------------------------
    //127501 begin
    //int iSize1 = (int)(strlen(sDate)+1+strlen(sInfo)+2+9+strlen(sLine));//ZMP:477242
    if(iSize > 0)
    {
        TBillLog::incEstiLogSize(iSize);
    }
        
    //end
    //----------------------------------------------------------
    fclose(fp);
    fp = NULL; //357111 for cppcheck
    va_end (ap);
   
}

TBillLog::TBillLog() : TThreadBase()
{
    char * sEnv=NULL; //357111 for cppcheck

    //�����ж������ֽ���
    int iEndianTest=0x01020304;
    char * cEndian=(char * )&iEndianTest;
    if( * cEndian == 0x01 )
    IsBigEndian=true;
    else
    IsBigEndian=false;

    m_fp = NULL;

    //143237 begin
    m_ulog_base_fp = NULL;
    m_fatal_base_fp = NULL;
    //143237 end

    //94046 begin
    m_fpFatal = NULL;
    m_FileNameFatal[0] = '\0';
    //end

    //ZMP:305593
    m_bShowConsole =false;
    memset(m_sLogTemp,0,sizeof(m_sLogTemp));

    m_FileName[0] = '\0';
    m_AppName[0] = '\0';
    m_LogPath[0] ='\0';
    m_MsgList = "";
    m_LastMsg = "";
    m_Monitor=NULL;
    m_bMsgFlag = false;
    m_iLogLevel = 0;
    m_FileNameFatalTrack[0] ='\0';
    m_FileNameTrack[0] ='\0';
    m_fpTrack = NULL;
    m_fpFatalTrack = NULL;
    //ȡ��־����release�汾Ĭ��0
#ifdef _DEBUG
    sEnv = getenv("BILL_LOG_LEVEL");
    if ( sEnv == NULL)
    LogLevel =0;
    else 
    LogLevel = atoi ( sEnv);
#else
    LogLevel =0;
#endif
    OneTrackLogLevel = 0;

    //ȡ��־·��
    sEnv = getenv("BILL_LOG_PATH");
    //ZMP:467369 
    strncpy (m_LogPath, ".", sizeof(m_LogPath)-1);
    m_LogPath[sizeof(m_LogPath)-1] = '\0';
    if ( sEnv != NULL && IsExist(sEnv))
    {
        strncpy (m_LogPath,sEnv, sizeof(m_LogPath)-1);
        m_LogPath[sizeof(m_LogPath)-1] = '\0';
    }
    //ȥ�����һ���ָ���
#if defined(__OS_UNIX__)
    if ( m_LogPath [strlen(m_LogPath)-1] == '/')
    m_LogPath [strlen(m_LogPath)-1] = '\0';
#else
    if ( m_LogPath [strlen(m_LogPath)-1] == '\\')
    m_LogPath [strlen(m_LogPath)-1] = '\0';
#endif
    m_bOutToFileFlag = true;

    m_bUseThread = true; //Ĭ��ʹ�õ������߳�д��־
    m_pThreadLogBuf = NULL;
    m_bThreadRunning = false;
    m_bThreadEnded = false;
    m_iThreadLogToBufCnt = 0;
    m_iThreadLogToFileCnt = 0;
}

TBillLog::~TBillLog()
{
    End();
}

bool TBillLog::IsExist(const char *sFileName)
{
    int iRetCode =0;
#if defined(__OS_UNIX__)
    iRetCode = access(sFileName, F_OK);
#else
    iRetCode = access(sFileName, 0x00);
#endif
    return iRetCode == 0;
}

void TBillLog::Start(const char * sAppName,const int iLevel,const bool bFlag,bool bOutToFileFlag)
{
    char sTemp[DATE_TIME_SIZE]={0}; //357111 for cppcheck
    FUNC_DT::GetCurrentTimeStr(sTemp);

    const char *pLogName=TFileOper::GetFileName(sAppName);
    if(strcmp(pLogName,sAppName) !=0 )
    {
        //�����ȫ·����������ǰ׺��ʹ�ô����ȫ·���ļ���
        strncpy(m_FileName,sAppName,sizeof(m_FileName));

        //94046 begin
        //�����ļ���
        if(pLogName != NULL && strlen(pLogName) > 0)
        {
            //ZMP:467369 
            strncpy(m_AppName,"_", sizeof(m_AppName)-1);
            m_AppName[sizeof(m_AppName)-1] = '\0';
            strcat(m_AppName,pLogName);
        }
        char sArg[6] = {0};//ZMP:477242
        //memset(sArg,0x00,sizeof(sArg));
        if( g_argc > 1 )
        	strncpy(sArg,g_argv[1],5);
        #if defined(__OS_UNIX__)
        //ZMP:467369 
        snprintf(m_FileNameFatal, sizeof(m_FileNameFatal), "%s/FATAL%s[%05d]%s%s"     ,m_LogPath,sTemp+4,getpid(),m_AppName,sArg);
        #else
        snprintf(m_FileNameFatal, sizeof(m_FileNameFatal), "%s\\FATAL%s[%05d]%s%s.txt",m_LogPath,sTemp+4,getpid(),m_AppName,sArg);
        m_FileNameFatal[sizeof(m_FileNameFatal)-1] = '\0';
        #endif
        //end

        #if defined(__OS_UNIX__)
        snprintf(m_FileName, sizeof(m_FileName),"%s/ULOG%s[%05d]%s"     ,m_LogPath,sTemp,getpid(),m_AppName);
        #else
        snprintf(m_FileName, sizeof(m_FileName), "%s\\ULOG%s[%05d]%s.txt",m_LogPath,sTemp,getpid(),m_AppName);
        m_FileName[sizeof(m_FileName)-1] = '\0';
        #endif
    }
    else
    {
        //�����ļ���
        if(sAppName&&strlen(sAppName)>0)
        {
            //ZMP:467369 
            strncpy(m_AppName,"_", sizeof(m_AppName)-1);
            m_AppName[sizeof(m_AppName)-1] = '\0';
            strcat(m_AppName,sAppName);
        }

        //94046 begin
        char sArg[6]= {0};//ZMP:477242
        //memset(sArg,0x00,sizeof(sArg));
        if( g_argc > 1 )
        	strncpy(sArg,g_argv[1],5);
        #if defined(__OS_UNIX__)
        //ZMP:467369 
        snprintf(m_FileNameFatal, sizeof(m_FileNameFatal), "%s/FATAL%s[%05d]%s%s"     ,m_LogPath,sTemp+4,getpid(),m_AppName,sArg);
        #else
        snprintf(m_FileNameFatal, sizeof(m_FileNameFatal), "%s\\FATAL%s[%05d]%s%s.txt",m_LogPath,sTemp+4,getpid(),m_AppName,sArg);
        m_FileNameFatal[sizeof(m_FileNameFatal)-1] = '\0';
        #endif
        //end

        #if defined(__OS_UNIX__)
        snprintf(m_FileName, sizeof(m_FileName),"%s/ULOG%s[%05d]%s"     ,m_LogPath,sTemp,getpid(),m_AppName);
        #else
        snprintf(m_FileName, sizeof(m_FileName), "%s\\ULOG%s[%05d]%s.txt",m_LogPath,sTemp,getpid(),m_AppName);
        m_FileName[sizeof(m_FileName)-1] = '\0';
        #endif
    }

    //��������
    LogLevel = iLevel;
    m_bShowConsole = bFlag;
    OneTrackLogLevel = 0;
    //�Ƿ�д��־�ļ�
    m_bOutToFileFlag = bOutToFileFlag;
    if(m_bOutToFileFlag) 
    {
        CreateLogFile(sAppName,iLevel);
        if(m_bUseThread)
        {
            StartThread();
        }
    }
    else//��д��־�ļ�
    {
        cout << "TBillLog::Start() : billlog not output to file!"<<endl;
        if (m_fp!=NULL) 
        {
            fclose(m_fp);
            m_fp = NULL;
        }

        //94046 begin
        if (m_fpFatal!=NULL) 
        {
            fclose(m_fpFatal);
            m_fpFatal = NULL;
        }
        //end
    }    
}

void TBillLog::End()
{
    if(m_bUseThread)
    {
        StopThread();
    }
    
    CloseLogFile();
}

void TBillLog::CreateLogFile(const char * sAppName,const int iLevel)
{
    char sTemp[DATE_TIME_SIZE] = {0};
    FUNC_DT::GetCurrentTimeStr(sTemp);
    //�����ļ���
    m_fp = fopen(m_FileName, "a+");
    if (NULL == m_fp)
    {
        int iError = errno;
        throw TException("TBillLog::Start(%s) : Open file [%s] fail. errno [%d], error_msg [%s]",
            sAppName, m_FileName, iError, strerror(iError));
    }

    //143237 begin
    m_ulog_base_fp = m_fp;
    //143237 end

    cout << "TBillLog::Start() : FILE_NAME=(" << m_FileName << ") LOG_LEVEL=[" << iLevel << "] SHOW_CONSOLE=["<<BOOL_STR[m_bShowConsole] <<"]" <<endl;
    fprintf(m_fp, "===================== < %s - %s [%d]> =====================\n", sTemp,sAppName,iLevel);
    //----------------------------------------------------------
    //127501 begin
    char sTempNew[20]= {0};//ZMP:477242
    //memset(sTempNew,0x00,sizeof(sTempNew));
    //ZMP:467369 
    snprintf(sTempNew, sizeof(sTempNew), "%d",LogLevel);
    sTempNew[sizeof(sTempNew)-1] = '\0';
    m_lEstiLogSize = GetFileSize(m_FileName);
    m_lEstiLogSize += (long)strlen("===================== <  -  []> =====================\n")+(long)strlen(sTemp)+(long)strlen(sAppName)+(long)strlen(sTempNew);
    //end
    //----------------------------------------------------------
    //94046 begin
    //�����ļ���
    m_fpFatal = fopen(m_FileNameFatal, "a+");
    if (NULL == m_fpFatal)
    {
        int iError = errno;
        throw TException("TBillLog::Start(%s) : Open file [%s] fail. errno [%d], error_msg [%s]",
            sAppName, m_FileNameFatal, iError, strerror(iError));
    }

    //143237 begin
    m_fatal_base_fp = m_fpFatal;
    //143237 end

    cout << "TBillLog::Start() : FILE_NAME=(" << m_FileNameFatal << ") LOG_LEVEL=[" << iLevel << "] SHOW_CONSOLE=["<<BOOL_STR[m_bShowConsole] <<"]" <<endl;
    fprintf(m_fpFatal, "===================== < %s - %s [%d]> =====================\n", sTemp,sAppName,iLevel);

    m_lEstiFatalSize = GetFileSize(m_FileNameFatal);
    m_lEstiFatalSize += 50;
}


void TBillLog::CloseLogFile()
{
    if (m_fp)
    {
        char sTemp[DATE_TIME_SIZE]={0}; //357111 for cppcheck
        FUNC_DT::GetCurrentTimeStr(sTemp);

        //143237 begin
        if( m_fp == m_ulog_base_fp )
        {
            //143237 end

            fprintf(m_fp, "===================== < %s > =====================\n", sTemp);
            fclose(m_fp);
            m_fp = NULL;	//357111 for cppcheck

            //143237 begin
        }
         m_ulog_base_fp = NULL;
         //143237 end

         //----------------------------------------------------------
         //127501 begin
         m_lEstiLogSize += (long)strlen("===================== <  > =====================\n")+(long)strlen(sTemp);
         //end
         //----------------------------------------------------------
         m_fp = NULL;
    }

    //94046 begin
    if (m_fpFatal)
    {
        char sTemp[DATE_TIME_SIZE]={0}; //357111 for cppcheck
        FUNC_DT::GetCurrentTimeStr(sTemp);

        //143237 begin
        if( m_fpFatal == m_fatal_base_fp )
        {
            //143237 end

            fprintf(m_fpFatal, "===================== < %s > =====================\n", sTemp);
            fclose(m_fpFatal);
            m_fpFatal = NULL;  //357111 for cppcheck
        //143237 begin
        }
        m_fatal_base_fp = NULL;
        //143237 end

        m_lEstiFatalSize += (long)strlen("===================== <  > =====================\n")+(long)strlen(sTemp);

        //m_fpFatal = NULL; //357111 for cppcheck Ų��fclose()����
    }
    //end
    
    //�����˳�ʱ����µ��������
    if (m_fpTrack)
    {
        fclose(m_fpTrack);
        m_fpTrack = NULL;    
    }
    if (m_fpFatalTrack)
    {
        fclose(m_fpFatalTrack);
        m_fpFatalTrack = NULL;          
    }
}

void TBillLog::SetLogLevel(const int iLevel)
{
  LogLevel = iLevel;
}

void TBillLog::SetOneTrackLogLevel(const int iLevel)
{
    OneTrackLogLevel = iLevel;    
}

void TBillLog::SetNowLevel(const int iLevel)
{
    m_iLogLevel = iLevel;
}

int TBillLog::GetLogLevel()
{
    return LogLevel;
}

void TBillLog::SetLogSize(const int iLogSize)
{
    lLogSize = iLogSize*1024*1024;
}

int TBillLog::GetLogSize()
{
    return (int)(lLogSize/(1024*1024)) ;//ZMP:477242
}

FILE ** TBillLog::GetLogFile(void)
{
    return &m_fp;
}

//94046 begin
FILE * TBillLog::GetFatalFile(void)
{
    return m_fpFatal;
}
//end

void TBillLog::SetShowConsole(const bool bFlag)
{
    m_bShowConsole = bFlag;
}

void TBillLog::SetMsgFlag(const bool bFlag)
{
    m_bMsgFlag = bFlag;
}

void  TBillLog::SetLogPath(const char *strLogPath)
{
    //ZMP:467369 
    strncpy(m_LogPath, strLogPath, sizeof(m_LogPath)-1);
    m_LogPath[sizeof(m_LogPath)-1] = '\0';
}

const char *TBillLog::GetLogPath()
{
    return m_LogPath;
}

void TBillLog::GetLevelName(const int iLogLevel,char sInfo[])
{
#ifdef _ASSERT_PARAM
	if(NULL == sInfo)
	{
		throw TException("TBillLog::GetLevelName:sInfo is NULL");
	}
#endif  
  
    switch(iLogLevel)
    {
    //ZMP:467369
    case LOG_NORMAL     : strncpy(sInfo,"[NORMAL]", LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    case LOG_DETAIL     : strncpy(sInfo,"[DETAIL]", LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    case LOG_DEBUG      : strncpy(sInfo,"[ DEBUG]", LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    case WARNING_NORMAL : strncpy(sInfo,"[ WARN ]", LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    case WARNING_FATAL  : strncpy(sInfo,"[ ERROR]", LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    case FATAL          : strncpy(sInfo,"[ FATAL]", LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    default             : strncpy(sInfo,"[ (N/A)]", LEVEL_NAME_LENGTH-1);sInfo[LEVEL_NAME_LENGTH-1] = '\0';break;
    }
}

void TBillLog::Write(const char *sFileName,int iLineNumber,const int iLogLevel,const int iLogCode,const char * fmt, ...)
{
    va_list ap;
    va_start (ap,fmt);
    try
    {
        vsnprintf(m_sLogTemp, sizeof(m_sLogTemp), fmt, ap); 
        SetLastMessages(m_sLogTemp);
    }
    catch(...)
    {
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"Write Buffer Overflow Or Format Paramter Error.\n");
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"FormatString : %s \n",fmt==NULL?"NULL":fmt);
    }
    va_end (ap);

    if(iLogLevel == 0 || m_bShowConsole)
    {
        printf("%s",m_sLogTemp);
    }


    if(m_bUseThread)
    {
        unsigned long iIndex = m_iThreadLogToBufCnt%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s",m_sLogTemp);
            ++m_iThreadLogToBufCnt;
        }
    }
    else
    {
        if (m_fp)
        {
            try
            {
                CheckAndBackup(m_FileName, m_fp, m_ulog_base_fp, 1 );

                //143237 begin
                if( m_fp && ( m_fp == m_ulog_base_fp ) )
                {
                    //143237 end

                    fprintf(m_fp,"%s",m_sLogTemp);
                    fflush(m_fp);

                //143237 begin
                }
                //143237 end

                //----------------------------------------------------------
                //127501 begin
                 m_lEstiLogSize += (long)strlen(m_sLogTemp);
                //end
                //----------------------------------------------------------
            }
            catch(...)
            {
                Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"logfile[%s] is not exist  rebuild it failed.\n",m_FileName);
            }
        }
    }
}

void TBillLog::Log(const char *sFileName,int iLineNumber,const int iLogLevel,const int iLogCode, const char * fmt, ...)
{
    char sDate[30]={0},sInfo[15]={0}; //357111 for cppcheck

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    GetLevelName(iLogLevel,sInfo);

    va_list ap;
    va_start (ap,fmt);
    try
    {
        vsnprintf(m_sLogTemp, sizeof(m_sLogTemp), fmt, ap); 

        if(m_Monitor && iLogCode!=LOG_PROC_CA && iLogLevel==LOG_NORMAL)
        {
            m_Monitor->WriteLog(iLogLevel,iLogCode,m_sLogTemp);
        }

        SetLastMessages(m_sLogTemp);
    }
    catch(...)
    {
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"Write Buffer Overflow Or Format Paramter Error.\n");
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"FormatString : %s \n",fmt==NULL?"NULL":fmt);
    }
    va_end (ap);

    if(iLogLevel == 0 || m_bShowConsole)
    {
        printf("%s > %s", sInfo,m_sLogTemp); 
    }

    if(m_bUseThread)
    {
        unsigned long iIndex = m_iThreadLogToBufCnt%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            if( iLogCode != -1 )
            {
                snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s %s > CCBS-%06d:%s",sDate,sInfo,iLogCode,m_sLogTemp);
    	    }
    	    else
    	    {
                snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s %s > CCBS:%s",sDate,sInfo,m_sLogTemp);	
    		} 
            ++m_iThreadLogToBufCnt;
        }
    }
    else
    {
        if (m_fp)
        {
            CheckAndBackup(m_FileName, m_fp, m_ulog_base_fp, 1 );
            if( iLogCode != -1 )
            {
       
                //143237 begin
                if( m_fp && ( m_fp == m_ulog_base_fp ) )
                {
                //143237 end

                    fprintf (m_fp, "%s %s > CCBS-%06d:%s", sDate,sInfo,iLogCode,m_sLogTemp); 
                    fflush(m_fp);

                //143237 begin
                }
                //143237 end
       
                //----------------------------------------------------------
                //127501 begin
                m_lEstiLogSize += (long)strlen(sDate)+1+(long)strlen(sInfo)+8+6+1+(long)strlen(m_sLogTemp);
                //end
                //---------------------------------------------------------- 
            }
            else
            {
                //143237 begin
                if( m_fp && ( m_fp == m_ulog_base_fp ) )
                {
                //143237 end

                    fprintf (m_fp, "%s %s > CCBS:%s", sDate,sInfo,m_sLogTemp);      		
                    fflush(m_fp);
       
                //143237 begin
                }
                //143237 end
       
                //----------------------------------------------------------
                //127501 begin
                m_lEstiLogSize += (long)strlen(sDate)+1+(long)strlen(sInfo)+8+(long)strlen(m_sLogTemp);
                //end
                //---------------------------------------------------------- 
            }
        }
    }
}

void TBillLog::Write(const int iLogCode,const char * fmt, ...)
{
    static char sLogTemp[10240]={0};
    sLogTemp[0] = '\0';
    va_list ap;
    va_start (ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap); 
    Write(NULL,-1,m_iLogLevel,iLogCode,"%s",sLogTemp);
    va_end (ap);
}

void TBillLog::Log(const int iLogCode, const char * fmt, ...)
{
    static char sLogTemp[10240]={0}; 
    sLogTemp[0] = '\0';
    va_list ap;
    va_start (ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap); 
    Log(NULL,-1,m_iLogLevel,iLogCode,"%s",sLogTemp);
    va_end (ap);
}

void TBillLog::Log(const char * fmt, ...)
{
    static char sLogTemp[10240]={0}; 
    sLogTemp[0] = '\0';
    va_list ap;
    va_start (ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap);
    Log(NULL,-1,m_iLogLevel,-1,"%s",sLogTemp);
    va_end (ap);
}

void TBillLog::Log(const char *sFileName,int iLineNumber,const int iLogLevel, const char * sPrimaryModuleCode, const char * sSubModuleCode,const int iLogCode, const char * fmt, ...)
{
    char sDate[30]={0},sInfo[15]={0}; //357111 for cppcheck

    char sWholeLogCode[60]={0}; //357111 for cppcheck

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    GetLevelName(iLogLevel,sInfo);

    #if defined(WIN32) || defined(_WIN32) 
    sWholeLogCode[sizeof(sWholeLogCode)-1]='\0';
    #endif

    snprintf(sWholeLogCode,sizeof(sWholeLogCode)-1,"%s-%s-%06d",sPrimaryModuleCode,sSubModuleCode,iLogCode);

    va_list ap;
    va_start (ap,fmt);
    try
    {
        vsnprintf(m_sLogTemp, sizeof(m_sLogTemp), fmt, ap); 

        if(m_Monitor && iLogCode!=LOG_PROC_CA && iLogLevel==LOG_NORMAL)
        {
            m_Monitor->WriteLog(iLogLevel,iLogCode,m_sLogTemp);
        }

        SetLastMessages(m_sLogTemp);
    }
    catch(...)
    {
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"Write Buffer Overflow Or Format Paramter Error.\n");
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"FormatString : %s \n",fmt==NULL?"NULL":fmt);
    }
    va_end (ap);

    if(iLogLevel == 0 || m_bShowConsole)
    {
        printf("%s > %s", sInfo,m_sLogTemp); 
    }

    if (m_bUseThread)
    {
        unsigned long iIndex = m_iThreadLogToBufCnt%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s %s > %s : %s",sDate,sInfo,sWholeLogCode,m_sLogTemp);
            ++m_iThreadLogToBufCnt;
        }
    }
    else
    {
        if (m_fp)
        {
            CheckAndBackup(m_FileName, m_fp, m_ulog_base_fp, 1 );
        
            //143237 begin
            if( m_fp && ( m_fp == m_ulog_base_fp ) )
            {
            //143237 end
        
                fprintf (m_fp, "%s %s > %s : %s", sDate,sInfo,sWholeLogCode,m_sLogTemp); 
                fflush(m_fp);
        
            //143237 begin
            }
            //143237 end
        
            //----------------------------------------------------------
            //127501 begin
             m_lEstiLogSize += (long)strlen(sDate)+1+(long)strlen(sInfo)+3+(long)strlen(sWholeLogCode)+3+(long)strlen(m_sLogTemp);
            //end
            //----------------------------------------------------------
        }
    }
}

void TBillLog::Log(const char * sPrimaryModuleCode, const char * sSubModuleCode, const int iLogCode, const char * fmt, ...)
{
    static char sLogTemp[10240]={0}; //357111 for cppcheck
    sLogTemp[0] = '\0';
    va_list ap;
    va_start (ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap); 
    Log(NULL,-1,m_iLogLevel,sPrimaryModuleCode,sSubModuleCode,iLogCode,"%s",sLogTemp);
    va_end (ap);
}

void TBillLog::SetLastMessages(const char *sLastMessage)
{
/* Ӱ���ȶ��ԣ�ע��
  m_LastMsg = sLastMessage;

  if(m_bMsgFlag)
  {
    llong iLength = m_MsgList.length();

    //����־�۵�5Mʱ�����Զ��ر��ۼƹ��ܡ�
    if(iLength>5*1024*1024)
    {
      m_MsgList = "";
      m_bMsgFlag = false;
      Warning((const char *)__FILE__,__LINE__,WARNING_FATAL,LOG_TBillLog,"Write [%d] LastMessage Overflow.\n",iLength);
    }

    m_MsgList = m_MsgList + m_LastMsg;
  }
 */
}

void TBillLog::ClearMessage()
{
    m_LastMsg = "";
    m_MsgList = "";
}

//ȡ�������־�б�
const char * TBillLog::GetLastMessage()
{
    try
    {
        return m_LastMsg.c_str();
    }
    catch(...)
    {
        return NULL;
    }
}

//ȡ����־�б�
const char *TBillLog::GetMessages()
{
    try
    {
        return m_MsgList.c_str();
    }
    catch(...)
    {
        return NULL;
    }
}

//ȡ���ļ���
const char *TBillLog::GetFileName()
{
    return m_FileName;
}

//94046 begin
//ȡ���ļ���
const char *TBillLog::GetFatalFileName()
{
    return m_FileNameFatal;
}
//end

llong TBillLog::GetFileSize(char * sFullPathFileName)
{
    struct stat sbuf;
    if (stat(sFullPathFileName, &sbuf) == 0)
        return sbuf.st_size;
    else 
        return -1;
}

//����ļ��Ƿ񳬳����ƣ����ǣ����䱸��Ϊ.old�ļ���������һ���µ��ļ�
//����1Ϊ��־�ļ���������2Ϊ��־�ļ���Ӧ�ľ��
bool TBillLog::CheckAndBackup(const char * sFileName , FILE*& fp)
{
    char     sFileNameOld[255]= {0};//ZMP:477242
    llong    filesize = -1 ;
    //int      filehandle = -1 ;//ZMP:305593
    if(NULL == sFileName)
    {
        //  	printf("TBillLog::CheckAndBackup(),sFileName is NULL \n");
        return false;
    }

    if(!IsExist(sFileName))
    {  	
        fp = fopen(sFileName, "a+");
        if (NULL == fp)
        {
            int iError = errno;
            throw TException("TBillLog::CheckAndBackup : Open file [%s] fail during rebuild it.errno [%d], error_msg [%s] \n",
                sFileName, iError, strerror(iError));
        }
        //	printf("TBillLog::CheckAndBackup : file[%s] is not exist,rebuild it. \n",sFileName);
    }  
    //memset(sFileNameOld,0, 255);

    //----------------------------------------------------------
    //127501 begin
    //filesize = GetFileSize((char*)sFileName);
    filesize = m_lEstiLogSize;
    //end
    //----------------------------------------------------------
    if (filesize == -1) return false;

    if (filesize >= lLogSize)	
    {
        //���������
        //ZMP:467369
        snprintf(sFileNameOld, sizeof(sFileNameOld), "%s.old", sFileName);
        sFileNameOld[sizeof(sFileNameOld)-1] = '\0';
        fclose(fp);
        fp = NULL;  //357111 for cppcheck
        TFileOper::Remove(sFileNameOld);

        //����־�ļ�������
        TFileOper::Rename(sFileName,sFileNameOld);

        TFileOper::Remove(sFileName);
        //----------------------------------------------------------
        //127501 begin
        m_lEstiLogSize = 0;
        //end
        //----------------------------------------------------------
#ifndef _WIN32
        int filehandle = -1 ;//ZMP:305593
        filehandle = open(sFileName,O_CREAT|O_EXCL,0666);
        if ( filehandle != -1 ) 
        {
            close( filehandle );
            chmod(sFileName, 0666);
        }
#endif
        fp = fopen(sFileName,"a+");
        if(NULL == fp)
        {
            int iError = errno;
            throw TException("Check And Backup Log File: Open file [%s] fail.errno [%d], error_msg [%s]",
                sFileName, iError, strerror(iError));
        }
    }
    return true ;
}

//143237 begin
bool TBillLog::CheckAndBackup(const char * sFileName , FILE*& fp, FILE *& fp2, int iFlag )
{
    char     sFileNameOld[255]= {0};//ZMP:477242
    llong    filesize = -1 ;
    //int      filehandle = -1 ;//ZMP:305593
    if(NULL == sFileName)
    {
        //  	printf("TBillLog::CheckAndBackup(),sFileName is NULL \n");
        return false;
    }

    if(!IsExist(sFileName))
    {  	
        fp = fopen(sFileName, "a+");
        if (NULL == fp)
        {
            int iError = errno;
            throw TException("TBillLog::CheckAndBackup : Open file [%s] fail during rebuild it.errno [%d], error_msg [%s]\n",
                sFileName, iError, strerror(iError));
        }

        fp2 = fp;

        //	printf("TBillLog::CheckAndBackup : file[%s] is not exist,rebuild it. \n",sFileName);
    }  
    //memset(sFileNameOld,0, 255);

    //----------------------------------------------------------
    //127501 begin
    //filesize = GetFileSize((char*)sFileName);
    if( 1 == iFlag )
        filesize = m_lEstiLogSize;
    else
        filesize = m_lEstiFatalSize;
    //end
    //----------------------------------------------------------
    if (filesize == -1) return false;

    if (filesize >= lLogSize)	
    {
        //���������
        //ZMP:467369
        snprintf(sFileNameOld, sizeof(sFileNameOld), "%s.old", sFileName);
        sFileNameOld[sizeof(sFileNameOld)-1] = '\0';
        fclose(fp);
        fp = NULL;  //357111 for cppcheck
        TFileOper::Remove(sFileNameOld);

        //����־�ļ�������
        TFileOper::Rename(sFileName,sFileNameOld);

        TFileOper::Remove(sFileName);
        //----------------------------------------------------------
        //127501 begin
        if( 1 == iFlag )
            m_lEstiLogSize = 0;
        else
            m_lEstiFatalSize = 0;
        //end
        //----------------------------------------------------------
#ifndef _WIN32
        int filehandle = -1 ;//ZMP:305593
        filehandle = open(sFileName,O_CREAT|O_EXCL,0666);
        if ( filehandle != -1 ) 
        {
          close( filehandle );
          chmod(sFileName, 0666);
        }
#endif
        fp = fopen(sFileName,"a+");
        if(NULL == fp)
        {
            int iError = errno;
            throw TException("Check And Backup Log File: Open file [%s] fail. errno [%d], error_msg [%s]",
                sFileName, iError, strerror(iError));
        }
        fp2 = fp;
    }
    return true ;
}
//143237 end


//94913 begin
void TBillLog::LogNoCode(const char * fmt, ...)
{
    static char sLogTemp[10240]={0}; //357111 for cppcheck
    sLogTemp[0] = '\0';
    va_list ap;
    va_start (ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap);
    if(TBillLog::IsTrackFlag && OneTrackLogLevel>0)
    {
      LogTrack(NULL,m_iLogLevel,"%s",sLogTemp);
    }
    else
    {
      Log(NULL,-1,m_iLogLevel,-1,"%s",sLogTemp);
    }      
    va_end (ap);   
}

void TBillLog::Write(const char *sFileName,int iLineNumber,const int iLogLevel,const char *iLogCode,const char * fmt, ...)
{
    va_list ap;
    va_start (ap,fmt);
    try
    {
        vsnprintf(m_sLogTemp, sizeof(m_sLogTemp), fmt, ap); 
        SetLastMessages(m_sLogTemp);
    }
    catch(...)
    {
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"Write Buffer Overflow Or Format Paramter Error.\n");
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"FormatString : %s \n",fmt==NULL?"NULL":fmt);
    }
    va_end (ap);

    if(iLogLevel == 0 || m_bShowConsole)
        printf("%s",m_sLogTemp);


    if(m_bUseThread)
    {
        unsigned long iIndex = m_iThreadLogToBufCnt%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s",m_sLogTemp);
            ++m_iThreadLogToBufCnt;
        }
    }
    else
    {
        if (m_fp)
        {
            try
            {
                CheckAndBackup(m_FileName, m_fp, m_ulog_base_fp, 1 );

                //143237 begin
                if( m_fp && ( m_fp == m_ulog_base_fp ) )
                {
                //143237 end

                    fprintf(m_fp,"%s",m_sLogTemp);
                    fflush(m_fp);

                //143237 begin
                }
                //143237 end

                //----------------------------------------------------------
                //127501 begin
                m_lEstiLogSize += (long)strlen(m_sLogTemp);
                //end
                //----------------------------------------------------------
            }
            catch(...)
            {
                Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"logfile[%s] is not exist  rebuild it failed.\n",m_FileName);
            }
        }
    }
}

void TBillLog::Log(const char *sFileName,int iLineNumber,const int iLogLevel,const char *iLogCode, const char * fmt, ...)
{
    char sDate[30]={0},sInfo[15]={0}; //357111 for cppcheck

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    GetLevelName(iLogLevel,sInfo);

    va_list ap;
    va_start (ap,fmt);
    try
    {
        vsnprintf(m_sLogTemp, sizeof(m_sLogTemp), fmt, ap); 

    	bool bResult = false;
    	const char *p = NULL;
    	p = strrchr(iLogCode,'-');
    	if( p == NULL )
    		bResult = (atol(iLogCode) != LOG_PROC_CA) ? true : false;
    	else if(strlen(p)>1)
    		bResult = (atol(++p) != LOG_PROC_CA) ? true : false;
    	else
    		bResult = true;

        if(m_Monitor && bResult && iLogLevel==LOG_NORMAL)
        {
            m_Monitor->WriteLog(iLogLevel,iLogCode,m_sLogTemp);
        }

        SetLastMessages(m_sLogTemp);
    }
    catch(...)
    {
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"Write Buffer Overflow Or Format Paramter Error.\n");
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"FormatString : %s \n",fmt==NULL?"NULL":fmt);
    }
    va_end (ap);

    if(iLogLevel == 0 || m_bShowConsole)
    {
        printf("%s > %s", sInfo,m_sLogTemp); 
    }

    if(m_bUseThread)
    {
        unsigned long iIndex = m_iThreadLogToBufCnt%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            if( strcmp(iLogCode,"-1") != 0 )
            {
                snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s %s > CCBS-%s:%s",sDate,sInfo,iLogCode,m_sLogTemp);
            }
            else
            {
                snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s %s > CCBS:%s",sDate,sInfo,m_sLogTemp);
            }
            ++m_iThreadLogToBufCnt;
        }
    }
    else
    {
        if (m_fp)
        {
            CheckAndBackup(m_FileName, m_fp, m_ulog_base_fp, 1 );
            if( strcmp(iLogCode,"-1") )
            {

                //143237 begin
                if( m_fp && ( m_fp == m_ulog_base_fp ) )
                {
                //143237 end

                    fprintf (m_fp, "%s %s > CCBS-%s:%s", sDate,sInfo,iLogCode,m_sLogTemp); 
                    fflush(m_fp);

                //143237 begin
                }
                //143237 end

                //----------------------------------------------------------
                //127501 begin
                m_lEstiLogSize += (long)strlen(sDate)+1+(long)strlen(sInfo)+8+(long)strlen(iLogCode)+1+(long)strlen(m_sLogTemp);
                //end
                //----------------------------------------------------------
            } 
            else
            {
                //143237 begin
                if( m_fp && ( m_fp == m_ulog_base_fp ) )
                {
                //143237 end

                    fprintf (m_fp, "%s %s > CCBS:%s", sDate,sInfo,m_sLogTemp); 
                    fflush(m_fp);

                //143237 begin
                }
                //143237 end

                //----------------------------------------------------------
                //127501 begin
                m_lEstiLogSize += (long)strlen(sDate)+1+(long)strlen(sInfo)+8+(long)strlen(m_sLogTemp);
                //end
                //----------------------------------------------------------				
            }
        }
    }
}

void TBillLog::Warning(const char *sFileName,int iLineNumber,const int iWarningLevel, const char *iWarningCode, const char * fmt, ...)
{
    char sDate[30]={0},sInfo[15]={0}; //357111 for cppcheck

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    GetLevelName(iWarningLevel,sInfo);

    va_list ap;
    va_start (ap,fmt);
    try
    {
        vsnprintf(m_sLogTemp, sizeof(m_sLogTemp), fmt, ap); 

        bool bResult = false;
        const char *p = NULL;
        p = strrchr(iWarningCode, '-');
        if (p == NULL)
            bResult = (atol(iWarningCode) != WARN_PROC_CA) ? true : false;
        else if (strlen(p) > 1)
            bResult = (atol(++p) != WARN_PROC_CA) ? true : false;
        else
            bResult = true;

        if (m_Monitor && bResult)
        {
            m_Monitor->WriteAlarm(iWarningLevel, iWarningCode, m_sLogTemp);
        }

        SetLastMessages(m_sLogTemp);
    }
    catch(...)
    {
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"Write Buffer Overflow Or Format Paramter Error.\n");
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"FormatString : %s \n",fmt==NULL?"NULL":fmt);
    }

    va_end (ap);

    if (m_bUseThread)
    {
        unsigned long iIndex = m_iThreadLogToBufCnt%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s %s > CCBS-%s:%s",sDate,sInfo,iWarningCode,m_sLogTemp);
            ++m_iThreadLogToBufCnt;
        }
    }
    else
    {
        if (m_fp)
        {
            CheckAndBackup(m_FileName, m_fp, m_ulog_base_fp, 1 );

            //143237 begin
            if( m_fp && ( m_fp == m_ulog_base_fp ) )
            {
            //143237 end

                fprintf (m_fp, "%s %s > CCBS-%s:%s", sDate,sInfo,iWarningCode,m_sLogTemp); 
                fflush(m_fp);

            //143237 begin
            }
            //143237 end

            //----------------------------------------------------------
            //127501 begin
            m_lEstiLogSize += (long)strlen(sDate)+1+(long)strlen(sInfo)+8+(long)strlen(iWarningCode)+1+(long)strlen(m_sLogTemp);
            //end
            //----------------------------------------------------------
        }
    }
}

void TBillLog::Warning(const char *sFileName,int iLineNumber,const int iWarningLevel, const int iWarningCode, const char * fmt, ...)
{
    char sDate[30]={0},sInfo[15]={0}; //357111 for cppcheck

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    GetLevelName(iWarningLevel,sInfo);

    va_list ap;
    va_start (ap,fmt);
    try
    {
        vsnprintf(m_sLogTemp, sizeof(m_sLogTemp), fmt, ap); 

        bool bResult = false;

        bResult = (iWarningCode != WARN_PROC_CA) ? true : false;

        if(m_Monitor && bResult)
        {
          	m_Monitor->WriteAlarm(iWarningLevel,iWarningCode,m_sLogTemp);
        }

        SetLastMessages(m_sLogTemp);
    }
    catch(...)
    {
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog_INT,"Write Buffer Overflow Or Format Paramter Error.\n");
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog_INT,"FormatString : %s \n",fmt==NULL?"NULL":fmt);
    }

    va_end (ap);


    if (m_bUseThread)
    {
        unsigned long iIndex = m_iThreadLogToBufCnt%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s %s > CCBS-%d:%s",sDate,sInfo,iWarningCode,m_sLogTemp);
            ++m_iThreadLogToBufCnt;
        }
    }
    else
    {
        if (m_fp)
        {
        	CheckAndBackup(m_FileName, m_fp, m_ulog_base_fp, 1 );

            if( m_fp && ( m_fp == m_ulog_base_fp ) )
            {
        		fprintf (m_fp, "%s %s > CCBS-%d:%s", sDate,sInfo,iWarningCode,m_sLogTemp); 
        		fflush(m_fp);
            }

            m_lEstiLogSize += (long)strlen(sDate)+1+(long)strlen(sInfo)+8+ 5 +1+(long)strlen(m_sLogTemp);
        }
    }
}


void TBillLog::Write(const char *iLogCode,const char * fmt, ...)
{
    static char sLogTemp[10240]={0}; //357111 for cppcheck
    sLogTemp[0] = '\0';
    va_list ap;
    va_start (ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap); 
    Write(NULL,-1,m_iLogLevel,iLogCode,"%s",sLogTemp);
    va_end (ap);
}

void TBillLog::Warning(const char *iWarningCode, const char * fmt, ...)
{
    static char sLogTemp[10240]={0}; //357111 for cppcheck
    sLogTemp[0] = '\0';
    va_list ap;
    va_start (ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap); 
    if(TBillLog::IsTrackFlag && OneTrackLogLevel>0)
    {
      LogTrack(NULL,m_iLogLevel,"%s",sLogTemp);
    }
    else
    {
      Warning(NULL,-1,m_iLogLevel,iWarningCode,"%s",sLogTemp);
    } 
    va_end (ap);    
}

void TBillLog::Warning(const int iWarningCode, const char * fmt, ...)
{
    static char sLogTemp[10240]={0}; //357111 for cppcheck
    sLogTemp[0] = '\0';
    va_list ap;
    va_start (ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap);
    if(TBillLog::IsTrackFlag && OneTrackLogLevel>0)
    {
      LogTrack(NULL,m_iLogLevel,"%s",sLogTemp);
    }
    else
    {
      Warning(NULL,-1,m_iLogLevel,iWarningCode,"%s",sLogTemp);
    }
    va_end (ap);
}


void TBillLog::Log(const char *sFileName,int iLineNumber,const int iLogLevel, const char * sPrimaryModuleCode, const char * sSubModuleCode,const char *iLogCode, const char * fmt, ...)
{
    char sDate[30]={0},sInfo[15]={0}; //357111 for cppcheck

    char sWholeLogCode[60]={0}; //357111 for cppcheck

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    GetLevelName(iLogLevel,sInfo);

#if defined(WIN32) || defined(_WIN32) 
    sWholeLogCode[sizeof(sWholeLogCode)-1]='\0';
#endif

    snprintf(sWholeLogCode,sizeof(sWholeLogCode)-1,"%s-%s-%s",sPrimaryModuleCode,sSubModuleCode,iLogCode);

    va_list ap;
    va_start (ap,fmt);
    try
    {
        vsnprintf(m_sLogTemp, sizeof(m_sLogTemp), fmt, ap); 

    	bool bResult = false;
    	const char *p = NULL;
    	p = strrchr(iLogCode,'-');
    	if( p == NULL )
    		bResult = (atol(iLogCode) != LOG_PROC_CA) ? true : false;
    	else if(strlen(p)>1)
    		bResult = (atol(++p) != LOG_PROC_CA) ? true : false;
    	else
    		bResult = true;

        if(m_Monitor && bResult && iLogLevel==LOG_NORMAL)
        {
            m_Monitor->WriteLog(iLogLevel,iLogCode,m_sLogTemp);
        }

        SetLastMessages(m_sLogTemp);
    }
    catch(...)
    {
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"Write Buffer Overflow Or Format Paramter Error.\n");
        Warning(sFileName,iLineNumber,WARNING_FATAL,LOG_TBillLog,"FormatString : %s \n",fmt==NULL?"NULL":fmt);
    }
    va_end (ap);

    if(iLogLevel == 0 || m_bShowConsole)
    {
        printf("%s > %s", sInfo,m_sLogTemp); 
    }

    if (m_bUseThread)
    {
        unsigned long iIndex = m_iThreadLogToBufCnt%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s %s > %s : %s",sDate,sInfo,sWholeLogCode,m_sLogTemp);
            ++m_iThreadLogToBufCnt;
        }
    }
    else
    {
        if (m_fp)
        {
            CheckAndBackup(m_FileName, m_fp, m_ulog_base_fp, 1 );

            //143237 begin
            if( m_fp && ( m_fp == m_ulog_base_fp ) )
            {
            //143237 end

                fprintf (m_fp, "%s %s > %s : %s", sDate,sInfo,sWholeLogCode,m_sLogTemp); 
                fflush(m_fp);

            //143237 begin
            }
            //143237 end

            //----------------------------------------------------------
            //126822 begin
            m_lEstiLogSize += (long)strlen(sDate)+1+(long)strlen(sInfo)+3+(long)strlen(sWholeLogCode)+3+(long)strlen(m_sLogTemp);
            //end
            //----------------------------------------------------------
        }
    }
}

void TBillLog::Log(const char * sPrimaryModuleCode, const char * sSubModuleCode, const char *iLogCode, const char * fmt, ...)
{
    static char sLogTemp[10240]={0}; //357111 for cppcheck
    sLogTemp[0] = '\0';
    va_list ap;
    va_start (ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap); 
    Log(NULL,-1,m_iLogLevel,sPrimaryModuleCode,sSubModuleCode,iLogCode,"%s",sLogTemp);
    va_end (ap);
}

void TBillLog::Fatal(const char *sFileName,int iLineNumber,const int iWarningLevel,const char *iWarningCode, const char * fmt, ...)
{
    char sDate[30]={0},sInfo[15]={0}; //357111 for cppcheck

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    GetLevelName(iWarningLevel,sInfo);

    va_list ap;
    va_start (ap,fmt);
    try
    {
        vsnprintf(m_sLogTemp, sizeof(m_sLogTemp), fmt, ap); 

		bool bResult = false;
		const char *p = NULL;
		p = strrchr(iWarningCode,'-');
		if( p == NULL )
			bResult = (atol(iWarningCode) != WARN_PROC_CA) ? true : false;
		else if(strlen(p)>1)
			bResult = (atol(++p) != WARN_PROC_CA) ? true : false;
		else
			bResult = true;

        if(m_Monitor && bResult)
        {
            m_Monitor->WriteAlarm(iWarningLevel,iWarningCode,m_sLogTemp);
        }

        SetLastMessages(m_sLogTemp);
    }
    catch(...)
    {
        Fatal(sFileName,iLineNumber,FATAL,LOG_TBillLog,"Write Buffer Overflow Or Format Paramter Error.\n");
        Fatal(sFileName,iLineNumber,FATAL,LOG_TBillLog,"FormatString : %s \n",fmt==NULL?"NULL":fmt);
    }

    va_end (ap);

    if (m_fpFatal)
    {
        CheckAndBackup(m_FileNameFatal, m_fpFatal, m_fatal_base_fp, 2 );

        //143237 begin
        if( m_fpFatal && ( m_fpFatal == m_fatal_base_fp ) )
        {
        //143237 end

            fprintf (m_fpFatal, "%s %s > CCBS-%s:%s", sDate,sInfo,iWarningCode,m_sLogTemp); 
            fflush(m_fpFatal);

        //143237 begin
        }
        //143237 end

        m_lEstiFatalSize += 100;
    }

    //��fatal��־д��ULOG��
    
    if (m_bUseThread)
    {
        unsigned long iIndex = m_iThreadLogToBufCnt%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            snprintf(m_pThreadLogBuf[iIndex],THREAD_LOG_LINE_SIZE,"1%s %s > CCBS-%s:%s",sDate,sInfo,iWarningCode,m_sLogTemp);
            ++m_iThreadLogToBufCnt;
        }
    }
    else
    {
        if (m_fp)
        {
            CheckAndBackup(m_FileName, m_fp, m_ulog_base_fp, 1 );
            if( m_fp && ( m_fp == m_ulog_base_fp ) )
            {
                fprintf (m_fp, "%s %s > CCBS-%s:%s", sDate,sInfo,iWarningCode,m_sLogTemp); 
                fflush(m_fp);
            }
            m_lEstiLogSize += (long)strlen(sDate)+1+(long)strlen(sInfo)+8+(long)strlen(iWarningCode)+1+(long)strlen(m_sLogTemp);
        }
    }
}

void TBillLog::Fatal(const char *iWarningCode, const char * fmt, ...)
{
    static char sLogTemp[10240]={0}; //357111 for cppcheck
    sLogTemp[0] = '\0';
    va_list ap;
    va_start (ap,fmt);
    vsnprintf(sLogTemp, sizeof(sLogTemp), fmt, ap); 
    if(TBillLog::IsTrackFlag && OneTrackLogLevel>0)
    {
      FatalTrack(NULL,m_iLogLevel,"%s",sLogTemp);
    }
    else
    {
      Fatal(NULL,-1,m_iLogLevel,iWarningCode,"%s",sLogTemp);
    }    
    va_end (ap);
}

//end

//----------------------------------------------------------
//126822 begin
long TBillLog::getEstiLogSize()
{
    return m_lEstiLogSize;
}

void TBillLog::incEstiLogSize(int iSize)
{
    m_lEstiLogSize += iSize;
}
//end
//----------------------------------------------------------

//ZMP:305593
void TBillLog::setEstiLogSize(int iSize)
{
    m_lEstiLogSize = iSize;
}

//�߳�д��־����
int TBillLog::svc()
{
    m_bThreadEnded= false;
    unsigned long iIndex = 0;
    while(m_bThreadRunning)
    {
        if(m_iThreadLogToFileCnt >= m_iThreadLogToBufCnt)
        {
            FUNC_DT::MSleep(10);
            continue;
        }
        iIndex = m_iThreadLogToFileCnt%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            if(m_pThreadLogBuf[iIndex][0] == '1') //��һ���ַ�Ϊ'1'��ʾ��ǰ����Ҫд��־��Ϊ'0'����Ҫ
            {
                if(m_fp != NULL)
                {
                    CheckAndBackup(m_FileName,m_fp,m_ulog_base_fp,1);
                    if( m_fp && ( m_fp == m_ulog_base_fp ) )
                    {
                      fprintf(m_fp,"%s",m_pThreadLogBuf[iIndex]+1);
                      fflush(m_fp);
                    }
                    m_lEstiLogSize += (long)strlen(m_pThreadLogBuf[iIndex])-1;                    
                }
                m_pThreadLogBuf[iIndex][0] = '0';
            }
            ++m_iThreadLogToFileCnt;
        }


            //д���ļ��е���־����д��BUF�е���־�����Խ�࣬Խ��Ҫ�ӿ촦��
            //ͨ������sleep��ʱ�䳤��������д�ļ����ٶ�

                //��������־���ǵ�����ˣ��������������������
    }

    //ʣ��δд�����������βдһ��
    for(unsigned long i=m_iThreadLogToFileCnt;i<m_iThreadLogToBufCnt;i++)
    {
        
        iIndex = i%THREAD_LOG_LINE_CNT;
        if(m_pThreadLogBuf!=NULL && m_pThreadLogBuf[iIndex]!=NULL)
        {
            if(m_pThreadLogBuf[iIndex][0] == '1')
            {
                if(m_fp != NULL)
                {
                    CheckAndBackup(m_FileName,m_fp,m_ulog_base_fp,1);
                    if( m_fp && ( m_fp == m_ulog_base_fp ) )
                    {
                      fprintf(m_fp,"%s",m_pThreadLogBuf[iIndex]+1);
                      fflush(m_fp);
                    }
                    m_lEstiLogSize += (long)strlen(m_pThreadLogBuf[iIndex])-1;                    
                }
                m_pThreadLogBuf[iIndex][0] = '0';
            }
        }
    }
    m_bThreadEnded= true;
    return 0;
}

void TBillLog::SetUseThread(bool bUseThread)
{
    m_bUseThread = bUseThread;
    //ZMP:1128529
    //ͨ�������ʱ��Ҫ�ж��߳��Ƿ񴴽� ���ر�ʱ��Ҫ�ر��߳�
    if( m_bUseThread&& !m_bThreadRunning)
    {
        StartThread();    
    }
    else if(!m_bUseThread && m_bThreadRunning)
    {
        StopThread();
    }
}

void TBillLog::StartThread()
{
    if(m_bThreadRunning)
    {
        return;
    }
    m_pThreadLogBuf = new(std::nothrow) char*[THREAD_LOG_LINE_CNT];
    if(m_pThreadLogBuf == NULL)
    {
        throw TException("TBillLog::StartThread() : pThreadLogBuf==NULL.");
    }
    for(int i=0; i<THREAD_LOG_LINE_CNT; i++)
    {
        m_pThreadLogBuf[i] = new(std::nothrow) char[THREAD_LOG_LINE_SIZE+1];
        if(m_pThreadLogBuf[i] == NULL)
        {
            for(int j=0; j<i; j++)
            {
                delete []m_pThreadLogBuf[j];
                m_pThreadLogBuf[j] = NULL;
            }
            delete []m_pThreadLogBuf;
            m_pThreadLogBuf = NULL;
            throw TException("TBillLog::StartThread() : pThreadLogBuf[%d]==NULL.", i);
        }
    }
    m_bThreadRunning = true;
    Run(NULL);
}

void TBillLog::StopThread()
{
    if(m_bThreadRunning)
    {
        m_bThreadRunning = false;
        int iRetryCnt=0;
        while(!m_bThreadEnded)//�ȴ��߳��˳�
        {
            FUNC_DT::MSleep(100);
            iRetryCnt++;
            if(iRetryCnt > 20)
            {
                //���Գ���20�Σ�ǿ���˳�
                break;
            }
        }
    }

    if(m_pThreadLogBuf != NULL)
    {
        for(int i=0;i<THREAD_LOG_LINE_CNT;i++)
        {
            if(m_pThreadLogBuf[i]!=NULL)
            {
                delete [](m_pThreadLogBuf[i]);
                m_pThreadLogBuf[i] = NULL;
            }
        }
        delete []m_pThreadLogBuf;
        m_pThreadLogBuf = NULL;
    }
}

//���ٿ�ʼ �����ļ����
void TBillLog::CreateTraceLogFile(const char *sFileName)
{  
    char sTemp[DATE_TIME_SIZE] = {0};
    FUNC_DT::GetCurrentTimeStr(sTemp);
    
    snprintf(m_FileNameFatalTrack, sizeof(m_FileNameFatalTrack), "%s/FATAL%s[%05d]%s",sFileName,sTemp+4,getpid(),m_AppName);  
    snprintf(m_FileNameTrack, sizeof(m_FileNameTrack),"%s/ULOG%s[%05d]%s",sFileName,sTemp,getpid(),m_AppName);   
    cout << "TBillLog::CreateTraceLogFile m_FileNameTrack====[" << m_FileNameTrack <<"]"<<endl;
    cout << "TBillLog::CreateTraceLogFile m_FileNameFatalTrack====[" << m_FileNameFatalTrack <<"]" <<endl;
    
    m_fpTrack = fopen(m_FileNameTrack,"a+");
    if (NULL == m_fpTrack)
    {
        int iError = errno;
        throw TException("TBillLog::CreateTraceLogFile : Open file [%s] fail. errno [%d], error_msg [%s]",
            m_FileNameTrack, iError, strerror(iError));
    }
    m_fpFatalTrack = fopen(m_FileNameFatalTrack,"a+");
    if (NULL == m_fpFatalTrack)
    {
        int iError = errno;
        throw TException("TBillLog::CreateTraceLogFile : Open file [%s] fail.errno [%d], error_msg [%s]",
            m_FileNameFatalTrack, iError, strerror(iError));
    }
}

//���ٽ��� �ر��ļ����
void TBillLog::CloseTraceLogFile()
{
    if (m_fpTrack)
    {
        fclose(m_fpTrack);
        m_fpTrack = NULL;    
    }
    if (m_fpFatalTrack)
    {
        fclose(m_fpFatalTrack);
        m_fpFatalTrack = NULL;          
    }
}

//ZMP 1013165 �˴���־����Ҫ����̫���ӣ���ҵ��������漰������־��ӡ������ɼ���
void TBillLog::LogTrack(const char *sFileName,const int iLogLevel, const char * fmt, ...)
{
    char sDate[30]={0},sInfo[15]={0};  
    static char logstr[10240]= { 0 };
    
    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    GetLevelName(iLogLevel,sInfo);
    va_list argp;
    va_start (argp,fmt);
    vsnprintf(logstr, sizeof(logstr), fmt, argp); 
        
    if (NULL!=m_fpTrack) 
    {
        fprintf(m_fpTrack, "%s %s > CCBS:%s", sDate,sInfo,logstr);
        fflush(m_fpTrack);   
    } 
    va_end (argp);
}

void TBillLog::FatalTrack(const char *sFileName,const int iLogLevel, const char * fmt, ...)
{
    char sDate[30]={0},sInfo[15]={0};  
    static char logstr[10240]= { 0 };
    
    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    GetLevelName(iLogLevel,sInfo);
    va_list argp;
    va_start (argp,fmt);
    vsnprintf(logstr, sizeof(logstr), fmt, argp); 
        
    if (NULL!=m_fpFatalTrack) 
    {
        fprintf(m_fpFatalTrack, "%s %s > CCBS:%s", sDate,sInfo,logstr);
        fflush(m_fpFatalTrack);      
    }  
    va_end (argp);
}