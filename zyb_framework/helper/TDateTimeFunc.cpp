//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TDateTimeFunc.cpp
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: ���ڴ�����
////////////////////////////////////////////////


#include <string.h>
#include <stdlib.h>
#include <stdio.h>




#ifdef  _WIN32
    #include <time.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/time.h>
#endif  //_UNIX

#include "CommonDef.h"
#include "TException.h"
#include "TDateTimeFunc.h"
#include "debug_new.h"
#include "ReadIni.h"

//ZMP:467372     
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif


int TDateTimeFunc::agiMonthC[]={31,28,31,30,31,30, 31,31,30,31,30,31};/*UsedbyAddDate*/

//=====================================================
//129457 begin:
//#ifdef _IMP_DATE
long TDateTimeFunc::TIME_DIFFERENCE = 0;
bool TDateTimeFunc::bFirst = true; 
bool TDateTimeFunc::m_bReadconfigFlag = false; 
bool TDateTimeFunc::m_bIsDstUsed = false; 
//#endif
//end!
//=====================================================


/**********************************************************************
*  ��  �� �� // ���ַ���YYYYMMDDHHMMSSʱ�����ʽת��Ϊϵͳ��time_t��
* ��������� // sTime  ��ʱ����ַ���YYYYMMDDHHMMSS��ʽ
* ��������� // ��
* �� �� ֵ�� // time_t��ʽ��ʱ���ʾ(��ʱ����������ʱ)
***********************************************************************/
time_t TDateTimeFunc::StringToTime(const char sTime[])
{
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif
    static time_t tTimeCache = 0;
    static char sTimeCache[DATE_TIME_SIZE]={0};

    if(sTime[0]=='\0')
    {
        return -1;
    }

    size_t iLen=strlen(sTime);//ZMP:477242
    if((iLen!=14)&&(iLen!=8))
        throw TException("StringToTime(Time=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime);

    if(strcmp(sTime,sTimeCache)==0)
    {
        return tTimeCache;
    }


#ifdef _IMP_DATE
    //ʹ��localtime���Ը���::daylight��ʹ���ϵͳʱ������һ��
    time_t tCurrent = 0;
    const time_t dt = tCurrent + TIME_DIFFERENCE ;
    gmtime(&dt);
   
#else
    //ʹ��localtime���Ը���::daylight��ʹ���ϵͳʱ������һ��
    time_t tCurrent = 0;		
    localtime(&tCurrent); //ȡ�õ�ǰʱ���tmֵ
#endif 

    static char sTempYear[5]={0},sTempMon[3]={0},sTempMDay[3]={0},sTempHour[3]={0},sTempMin[3]={0},sTempSec[3]={0}; 
    strncpy(sTempYear,sTime,4);sTempYear[4]=0;
    sTempMon[0]=sTime[4];
    sTempMon[1]=sTime[5];
    sTempMDay[0]=sTime[6]; sTempMon[2]=0;
    sTempMDay[1]=sTime[7]; sTempMDay[2]=0;

    // ֧��8λ
    if(8 == iLen)
    {
        sTempHour[0] = '0';
        sTempHour[1] = '0';
        sTempHour[2]=0;

        sTempMin[0]= '0';
        sTempMin[1]= '0';
        sTempMin[2]=0;

        sTempSec[0]= '0';
        sTempSec[1]= '0';
        sTempSec[2]=0;
    }
    else  // 14λ
    {
        sTempHour[0]=sTime[8];
        sTempHour[1]=sTime[9]; sTempHour[2]=0;

        sTempMin[0]=sTime[10];
        sTempMin[1]=sTime[11]; sTempMin[2]=0;

        sTempSec[0]=sTime[12];
        sTempSec[1]=sTime[13]; sTempSec[2]=0;
    }

    static struct tm s;
    s.tm_year=atoi(sTempYear)-1900;
    s.tm_mon=atoi(sTempMon)-1;
    s.tm_mday=atoi(sTempMDay);
    s.tm_hour=atoi(sTempHour);
    s.tm_min=atoi(sTempMin);
    s.tm_sec=atoi(sTempSec);
    #ifdef __BORLANDC__
    s.tm_isdst=(int)-1;
    #else
    s.tm_isdst=(int)-1;    //::daylight �Ƿ������չ��Լʱ�䡣��0Ϊ����������������ʱ�Գ���ʱ���Ӱ�졣
    #endif

    strncpy(sTimeCache,sTime,sizeof(sTimeCache)-1);
    sTimeCache[sizeof(sTimeCache)-1]='\0';
    tTimeCache = mktime(&s);
    
    return tTimeCache;
}

/**********************************************************************
*  ��  �� �� // ���ַ���YYYYMMDDHHMMSSʱ�����ʽת��Ϊϵͳ��time_t��
* ��������� // sTime  ��ʱ����ַ���YYYYMMDDHHMMSS��ʽ
* ��������� // ��
* �� �� ֵ�� // time_t��ʽ��ʱ���ʾ(��ʱ����������ʱ)
***********************************************************************/
time_t TDateTimeFunc::StringToTimeNodst(const char sTime[])
{
    size_t iLen=strlen(sTime);//ZMP:477242
    if((iLen!=14)&&(iLen!=8)&&(iLen != 0 ))
        throw TException("StringToTimeNodst(Time=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime);

    if(iLen == 0)
    {
        return -1;
    }
 //====================================================
//140185 begin:
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif
//140185 end!
//====================================================/ 
    
//====================================================
//140185 begin:
#ifdef _IMP_DATE
    //ʹ��localtime���Ը���::daylight��ʹ���ϵͳʱ������һ��
    time_t tCurrent = 0;
    const time_t dt = tCurrent + TIME_DIFFERENCE ;
    gmtime(&dt);
   
#else
    //ʹ��localtime���Ը���::daylight��ʹ���ϵͳʱ������һ��
    time_t tCurrent = 0;        
    localtime(&tCurrent); //ȡ�õ�ǰʱ���tmֵ
#endif 
//140185 end!
//====================================================/ 

    static char sTempYear[5]={0},sTempMon[3]={0},sTempMDay[3]={0},sTempHour[3]={0},sTempMin[3]={0},sTempSec[3]={0};
    strncpy(sTempYear,sTime,4);sTempYear[4]=0;
    sTempMon[0]=sTime[4];
    sTempMon[1]=sTime[5];
    sTempMDay[0]=sTime[6]; sTempMon[2]=0;
    sTempMDay[1]=sTime[7]; sTempMDay[2]=0;

    // ֧��8λ
    if(8 == iLen)
    {
        sTempHour[0] = '0';
        sTempHour[1] = '0';
        sTempHour[2]=0;

        sTempMin[0]= '0';
        sTempMin[1]= '0';
        sTempMin[2]=0;

        sTempSec[0]= '0';
        sTempSec[1]= '0';
        sTempSec[2]=0;
    }
    else  // 14λ
    {
        sTempHour[0]=sTime[8];
        sTempHour[1]=sTime[9]; sTempHour[2]=0;

        sTempMin[0]=sTime[10];
        sTempMin[1]=sTime[11]; sTempMin[2]=0;

        sTempSec[0]=sTime[12];
        sTempSec[1]=sTime[13]; sTempSec[2]=0;
    }
    

    static struct tm s;
    s.tm_year=atoi(sTempYear)-1900;
    s.tm_mon=atoi(sTempMon)-1;
    s.tm_mday=atoi(sTempMDay);
    s.tm_hour=atoi(sTempHour);
    s.tm_min=atoi(sTempMin);
    s.tm_sec=atoi(sTempSec);
    s.tm_isdst=(int)0;    //::daylight �Ƿ������չ��Լʱ�䡣��0Ϊ����������������ʱ�Գ���ʱ���Ӱ�졣
    return mktime(&s);
}


const char * TDateTimeFunc::GetCurrentTimeStr()
{
    static time_t tCurrTimeCache=0;
    static char sCurtime[DATE_TIME_SIZE]={0};
    time_t tCurrent;
    time(&tCurrent); //ȡ�õ�ǰʱ���time_tֵ
    if(tCurrTimeCache==tCurrent)
    {
        return sCurtime;
    }
    
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
    const time_t dt = tCurrent + TIME_DIFFERENCE ;
    struct tm *tm_Cur = gmtime(&dt);
#else		
    struct tm *tm_Cur = localtime(&tCurrent); //ȡ�õ�ǰʱ���tmֵ�����̰߳�ȫ
#endif 
    timeToStdStr(sCurtime,sizeof(sCurtime),tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
    tCurrTimeCache = tCurrent;

    return sCurtime;
}

const char * TDateTimeFunc::GetCurrentTimeStr(char sTime[])
{
    const char *pRet=GetCurrentTimeStr();
    if(sTime!=NULL)
    {
        strncpy(sTime,pRet,DATE_TIME_SIZE-1);
        sTime[DATE_TIME_SIZE-1] = '\0';
    }
    return pRet;
}

const char * TDateTimeFunc::GetCurrentTimeStr(char sTime[],size_t iLen)
{
    const char *pRet=GetCurrentTimeStr();
    if(sTime!=NULL)
    {
        if(iLen>0)
        {
            strncpy(sTime,pRet,iLen-1);
            sTime[iLen-1]='\0';
        }
        else
        {
            strncpy(sTime,pRet,DATE_TIME_SIZE-1);
            sTime[DATE_TIME_SIZE-1] = '\0';
        }
    }
    return pRet;
}

const char * TDateTimeFunc::GetCurrentTimeStr(char sTime[],size_t iLen,bool bLongFlag)
{
    if(!bLongFlag)
    {
        return GetCurrentTimeStr(sTime,iLen);
    }

    static time_t tCurrTimeCache=0;
    static char sCurtime[32]={0};
    time_t tCurrent;
    time(&tCurrent); //ȡ�õ�ǰʱ���time_tֵ
    
    if(tCurrTimeCache!=tCurrent)
    {
#ifdef _IMP_DATE
        if(bFirst)
        {
            getTimeDifference();
            bFirst = false;
        }
    
        const time_t dt = tCurrent + TIME_DIFFERENCE ;
        struct tm *tm_Cur = gmtime(&dt);
#else		
        struct tm *tm_Cur = localtime(&tCurrent); //ȡ�õ�ǰʱ���tmֵ
#endif 
    
        timeToStr2(sCurtime,tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
        tCurrTimeCache = tCurrent;
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
            int iLength = bLongFlag ? 19:14;
            strncpy(sTime,sCurtime,(size_t)iLength);
            sTime[iLength] = '\0';
        }
    }

    return sCurtime;
}


/**********************************************************************
*  ��  �� �� // ʱ��(YYYYMMDDHHMMSS)�ַ�����ʱ����ӣ����ؽ���ַ���
* ��������� // start_time  ��YYYYMMDDHHMMSS��ʽ���ַ���
* ��������� // iSeconds         ��ʱ��
* �� �� ֵ�� // ��Ӻ�Ľ���ַ������������NULL��˵������������Ϸ�
* ע��    �� // û��ʱ��������ʱ����
***********************************************************************/
const char * TDateTimeFunc::GetTimeAddSeconds(const char sTime[],long iSeconds,bool bSysFlag)
{
    if (strlen(sTime) != DATE_TIME_SIZE-1)
        throw TException("GetTimeAddSeconds(Time=%s) : FormatString Is 'YYYYMMDDHHMMSS'.", sTime);

    static char sCurtime[DATE_TIME_SIZE] = { 0 };
    //static char sAntiCrashBuf[1024];//�˴���Ϊ�ݴ���ƣ�����յ������쳣�����⡣//ZMP:305593,����δ�õ�
    
    //�����ַ���������
    sCurtime[DATE_TIME_SIZE-1] = '\0';
    if (iSeconds == 0)
    {
        strncpy(sCurtime, sTime, sizeof(sCurtime) - 1);
        return sCurtime;
    }

    if (bSysFlag) 
    {
        time_t count_second = StringToTime(sTime);
        count_second = count_second + iSeconds;
        strncpy(sCurtime, TimeToString(count_second, false), sizeof(sCurtime) - 1);
    } 
    else 
    {  
        memset(sCurtime, 0, sizeof(sCurtime));

        int iIncrSec = (int)(ABS(iSeconds) % 60);//ZMP:477242
        int iIncrMin = (int)((ABS(iSeconds) / 60) % 60);
        int iIncrHour = (int)((ABS(iSeconds) / 3600) % 24);
        int iIncrDay = (int)(ABS(iSeconds) / 86400);

        int iOriginSec = 10 * (sTime[12] - '0') + sTime[13] - '0';
        int iOriginMin = 10 * (sTime[10] - '0') + sTime[11] - '0';
        int iOriginHour = 10 * (sTime[8] - '0') + sTime[9] - '0';

        if (iSeconds < 0)
        {
            int iTmpDd = -1;
            iOriginHour += 24 - 1;
            iOriginMin += 60 - 1;
            iOriginSec += 60;

            /*�ۼ�����*/
            iOriginSec -= iIncrSec;

            /*��->�� ����*/
            iOriginMin += iOriginSec / 60;
            iOriginSec %= 60;

            /*�ۼƷ�����*/
            iOriginMin -= iIncrMin;

            /*��->Сʱ ����*/
            iOriginHour += iOriginMin / 60;
            iOriginMin %= 60;

            /*�ۼ�Сʱ*/
            iOriginHour -= iIncrHour;

            /*Сʱ->��*/
            iTmpDd += iOriginHour / 24;
            iOriginHour %= 24;

            iTmpDd  -= iIncrDay;

            iIncrHour = iOriginHour;
            iIncrMin = iOriginMin;
            iIncrSec = iOriginSec;
            iIncrDay = iTmpDd;
        }
        else
        {
            /*�ۼ�����*/
            iIncrSec += iOriginSec;

            /*��->�� ����*/
            iIncrMin += iIncrSec / 60;
            iIncrSec %= 60;

            /*�ۼƷ�����*/
            iIncrMin += iOriginMin;

            /*��->Сʱ ����*/
            iIncrHour += iIncrMin / 60;
            iIncrMin %= 60;

            /*�ۼ�Сʱ*/

            iIncrHour += iOriginHour;

            /*Сʱ->��*/
            iIncrDay += iIncrHour / 24;
            iIncrHour %= 24;
        }

//=====================================================
//129457 begin:
#ifdef _IMP_DATE
        timeToStr3(sCurtime, sTime, iIncrHour, iIncrMin, iIncrSec);
#else
        snprintf(sCurtime, sizeof(sCurtime), "%.8s%02d%02d%02d", sTime, iIncrHour, iIncrMin, iIncrSec);
#endif 
//129457 end!
//=====================================================/  

        //��������
        AddDay(sCurtime, iIncrDay);
    }

    return sCurtime;
}

/**********************************************************************
*  ��  �� �� // ʱ��(YYYYMMDDHHMMSS)�ַ�����ʱ����������ؽ���ַ���
* ��������� // start_time  ��YYYYMMDDHHMMSS��ʽ���ַ���
* ��������� // iSeconds         ��ʱ��
* �� �� ֵ�� // ��Ӻ�Ľ���ַ������������NULL��˵������������Ϸ�
* ע��    �� // û��ʱ��������ʱ���� ADDED BY SUNY
 ***********************************************************************/
void TDateTimeFunc::GetTimeSubSeconds(char  sDateTime[],long iSeconds)
{
    if(strlen(sDateTime)!=14||iSeconds<0)
        throw TException("GetTimeAddSeconds(Time=%s) : FormatString Is 'YYYYMMDDHHMMSS',iSeconds[%ld].",sDateTime,iSeconds);

    int  iTempMon,iTempDay,iTempYear,iHour,iMinute,iSecond;
    long  iTotDays=0,iRemainSecs=0,iCurrTotSecs=0;     //iCurrTotSecsΪ���������
    iTotDays      = iSeconds/86400;
    iRemainSecs = iSeconds%86400;
    
    if(iTotDays>0) AddDay(sDateTime,-iTotDays);
    
    iTempYear = (sDateTime[0]-'0')*1000+(sDateTime[1]-'0')*100+(sDateTime[2]-'0')*10+(sDateTime[3]-'0');
    iTempMon  = (sDateTime[4]-'0')*10 +(sDateTime[5]-'0');
    iTempDay  = (sDateTime[6]-'0')*10 +(sDateTime[7]-'0');    
    iHour          =  (sDateTime[8]-'0')*10 +(sDateTime[9]-'0');    
    iMinute          =  (sDateTime[10]-'0')*10 +(sDateTime[11]-'0');    
    iSecond          =  (sDateTime[12]-'0')*10 +(sDateTime[13]-'0');        
    iCurrTotSecs    =  iHour*3600+iMinute*60+iSecond;
    iCurrTotSecs    =  iCurrTotSecs-iRemainSecs;
    
    if(iCurrTotSecs<0) 
    {
            AddDay(sDateTime,-1);
        iTempYear = (sDateTime[0]-'0')*1000+(sDateTime[1]-'0')*100+(sDateTime[2]-'0')*10+(sDateTime[3]-'0');
        iTempMon  = (sDateTime[4]-'0')*10 +(sDateTime[5]-'0');
        iTempDay  = (sDateTime[6]-'0')*10 +(sDateTime[7]-'0');    
        iHour          =  (sDateTime[8]-'0')*10 +(sDateTime[9]-'0');    
        iMinute          =  (sDateTime[10]-'0')*10 +(sDateTime[11]-'0');    
        iSecond          =  (sDateTime[12]-'0')*10 +(sDateTime[13]-'0');        
        iCurrTotSecs +=  86400;        
    }

    iHour = (int)(iCurrTotSecs/3600);//ZMP:477242
    iCurrTotSecs = iCurrTotSecs%3600;
    iMinute = (int)(iCurrTotSecs/60);
    iSecond = (int)(iCurrTotSecs%60);

    
    sDateTime[0] = (char)(iTempYear/1000+'0');
    sDateTime[1] = (char)((iTempYear%1000)/100+'0');
    sDateTime[2] = (char)((iTempYear%100)/10+'0');
    sDateTime[3] = (char)(iTempYear%10+'0');

    sDateTime[4] = (char)(iTempMon/10+'0');
    sDateTime[5] = (char)(iTempMon%10+'0');
    
    sDateTime[6] = (char)(iTempDay/10+'0');
    sDateTime[7] = (char)(iTempDay%10+'0');

    sDateTime[8] = (char)(iHour/10+'0');
    sDateTime[9] = (char)(iHour%10+'0');
    
    sDateTime[10] = (char)(iMinute/10+'0');
    sDateTime[11] = (char)(iMinute%10+'0');
    
    sDateTime[12] = (char)(iSecond/10+'0');
    sDateTime[13] = (char)(iSecond%10+'0');
    
}


/**********************************************************************
*  ��  �� �� // ˯�ߺ�������װ�˲���ϵͳ�Ĳ��죩
* ��������� // sec  ��������Ϣ��ʱ��
* ��������� // ��
* �� �� ֵ�� // ��
***********************************************************************/
void  TDateTimeFunc::Sleep(const int iSeconds)
{
#ifdef _ASSERT_PARAM
  if(iSeconds<0)
  {
    throw TException("TDateTimeFunc::Sleep():iSeconds is error");    
  }
#endif 

#ifdef _WIN32
    ::Sleep(iSeconds*1000);
#else
    ::sleep((unsigned int)iSeconds);
#endif
}


/**********************************************************************
*  ��  �� �� // ˯�ߺ�������װ�˲���ϵͳ�Ĳ��죩
* ��������� // iMicroSeconds  ��������Ϣ��ʱ�������룩
* ��������� // ��
* �� �� ֵ�� // ��
***********************************************************************/
void  TDateTimeFunc::MSleep(const int iMicroSeconds)
{
  //����ַ���ʱ���Ƿ�Ϸ�
#ifdef _ASSERT_PARAM
  if(iMicroSeconds<0)
  {
    throw TException("TDateTimeFunc::MSleep():iSeconds is error");    
  }
#endif

#ifdef _WIN32
    ::Sleep(iMicroSeconds);
#else
    struct timeval tval;
    tval.tv_sec=iMicroSeconds/1000;
    tval.tv_usec=(iMicroSeconds*1000)%1000000;
    select(0,NULL,NULL,NULL,&tval);
#endif
}

const char * TDateTimeFunc::TimeToString(time_t tTime)
{
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif

    static time_t tTimeCache=0;
    static char sCurtime[DATE_TIME_SIZE] ={0};
    if(tTimeCache==tTime)
    {
        return sCurtime;
    }


#ifdef _IMP_DATE
    const time_t dt = tTime + TIME_DIFFERENCE ;
    struct tm *tm_Cur = gmtime(&dt);
#else
    struct tm *tm_Cur = localtime(&tTime);
    
#endif 
    tTimeCache=tTime;
    timeToStdStr(sCurtime,sizeof(sCurtime),tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);

    return sCurtime;
}


const char * TDateTimeFunc::TimeToString(time_t tTime,bool bLongFlag)
{
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif

    if(!bLongFlag)
    {
        return TimeToString(tTime);
    }

    static time_t tTimeCache=0;
    static char sCurtime[32] ={0};
    if(tTimeCache==tTime)
    {
        return sCurtime;
    }

#ifdef _IMP_DATE
    const time_t dt = tTime + TIME_DIFFERENCE ;
    struct tm *tm_Cur = gmtime(&dt);
#else
    struct tm *tm_Cur = localtime(&tTime);
#endif 

    tTimeCache=tTime;
    timeToStr2(sCurtime,tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);  

    return sCurtime;
}

const char * TDateTimeFunc::TimeToString(time_t tTime,bool bLongFlag,int iPtrIdx)
{
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif

    static char sCurtime[40] ={0}; 
    static char sCurtime1[40]={0}; 
    static char sCurtime2[40]={0}; 
    static char sCurtime3[40]={0}; 
    static char sCurtime4[40]={0}; 
    static char sCurtime5[40]={0}; 
    static char sCurtime6[40]={0}; 
    static char sCurtime7[40]={0}; 

    char *p_sCurtime=NULL;
    switch (iPtrIdx)
    {
    case 0:
        p_sCurtime=sCurtime;
        break;
    case 1:
        p_sCurtime=sCurtime1;
        break;
    case 2:
        p_sCurtime=sCurtime2;
        break;
    case 3:
        p_sCurtime=sCurtime3;
        break;
    case 4:
        p_sCurtime=sCurtime4;
        break;
    case 5:
        p_sCurtime=sCurtime5;
        break;
    case 6:
        p_sCurtime=sCurtime6;
        break;
    case 7:
        p_sCurtime=sCurtime7;
        break;
    default:
        throw TException("TimeToString: iPtrIdx Is 0<=iPtrIdx<=7");
    }

#ifdef _IMP_DATE
    const time_t dt = tTime + TIME_DIFFERENCE ;
    struct tm *tm_Cur = gmtime(&dt);
#else
    struct tm *tm_Cur = localtime(&tTime);
#endif 

    if(bLongFlag)
    {
        timeToStr2(p_sCurtime,tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);  
    }
    else
    {
        timeToStdStr(p_sCurtime,sizeof(sCurtime),tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
    }

    return p_sCurtime;
}

/**********************************************************************
*  ��  �� �� // ����ʱ���ַ���YYYYMMDDHHMMSS����ĵõ���ʱ����
* ��������� // sTime1,sTime2
* ��������� // ��
* �� �� ֵ�� // sTime1 - sTime2 ������
***********************************************************************/
long TDateTimeFunc::GetDiffTime(const char sTime1[], const char sTime2[])
{
    if(strlen(sTime1)!=14||strlen(sTime2)!=14)
        throw TException("GetDiffTime(Time1=%s,Time2=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime1,sTime2);

    return (long)difftime(StringToTime(sTime1),StringToTime(sTime2));
}

/**********************************************************************
*  ��  �� �� // ����ʱ���ַ���YYYYMMDDHHMMSS����ĵõ������� ����ʱ����
* ��������� // sTime1,sTime2
* ��������� // ��
* �� �� ֵ�� // sTime1 - sTime2 ������
***********************************************************************/
long TDateTimeFunc::GetDiffDay(const char sTime1[], const char sTime2[])
{
    if(strlen(sTime1)!=14||strlen(sTime2)!=14)
        throw TException("GetDiffDay(Time1=%s,Time2=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime1,sTime2);

    char sDayTime1[15]={0};
    char sDayTime2[15]={0};
    strncpy(sDayTime1, sTime1, 8);
    strncpy(sDayTime2, sTime2, 8);
    memset(sDayTime1+8, '0',6);
    memset(sDayTime2+8, '0',6);
    return (long)(difftime(StringToTimeNodst(sDayTime1),StringToTimeNodst(sDayTime2))/86400);
}

/**********************************************************************
*  ��  �� �� // ����ʱ���ַ���YYYYMMDDHHMMSS����ĵõ������� ����ʱ����
* ��������� // sTime1,sTime2
* ��������� // ��
* �� �� ֵ�� // sTime1 - sTime2 ������
***********************************************************************/
long TDateTimeFunc::GetDiffDayWithSeconds(const char sTime1[], const char sTime2[])
{
    if(strlen(sTime1)!=14||strlen(sTime2)!=14)
        throw TException("GetDiffDayWithSeconds(Time1=%s,Time2=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime1,sTime2);
    return (long)(difftime(StringToTime(sTime1),StringToTime(sTime2))/86400);
}

/**********************************************************************
*  ��  �� �� // ����ʱ���ַ���YYYYMMDDHHMMSS����ĵõ�������,POST��ŷ�ޣ�PRO������ã������ֳ���Ҫʹ�ã������sTime1,sTime2
* ��������� // sTime1,sTime2
* ��������� // ��
* �� �� ֵ�� // sTime1 - sTime2 ������
***********************************************************************/
long TDateTimeFunc::Daydiff(const char sTime1[], const char sTime2[])
{
    if(strlen(sTime1)!=14||strlen(sTime2)!=14)
        throw TException("Daydiff(Time1=%s,Time2=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime1,sTime2);

    struct tm t;
    time_t t1 =0;
    time_t t2 =0;
    char tmp[10] ={0};
    int hhmiss =0;

    memcpy(tmp, sTime1, 4); 
    tmp[4] = 0; 
    t.tm_year = atoi(tmp) - 1900;
        
    memcpy(tmp, sTime1 + 4, 2); 
    tmp[2] = 0; 
    t.tm_mon = atoi(tmp) - 1;
        
    memcpy(tmp, sTime1 + 6, 2); 
    tmp[2] = 0; 
    t.tm_mday = atoi(tmp);
    t.tm_hour = 0;
    t.tm_min  = 0;
    t.tm_sec  = 0;
    t.tm_isdst = 0;      
    t1 = mktime(&t);        

    memcpy(tmp, sTime2, 4);    
    tmp[4] = 0; 
    t.tm_year = atoi(tmp) - 1900;
        
    memcpy(tmp, sTime2 + 4, 2);  
    tmp[2] = 0; 
    t.tm_mon = atoi(tmp) - 1;
        
    memcpy(tmp, sTime2 + 6, 2);  
    tmp[2] = 0; 
    t.tm_mday = atoi(tmp);
        
    memcpy(tmp, sTime2 + 8, 6);  
    tmp[6] = 0; 
    hhmiss = atoi(tmp);
        
    t2 = mktime(&t);

    if (hhmiss==0) 
    {
        return (long)difftime(t2,t1)/86400 ;
    } 
    else 
    {
        return (long)difftime(t2,t1)/86400 + 1;
    }        
    return 0; 
}


/**********************************************************************
*  ��  �� �� // ���ַ���YYYYMMDDHHMMSSʱ���һ��
* ��������� // sDate iDay
* ��������� // sDate
* �� �� ֵ�� // sDate + iDay ���ַ���
***********************************************************************/
void TDateTimeFunc::AddDay(char sDate[],long iDays)
{
    if (strlen(sDate) != 14)
        throw TException("SubDate(Date=%s) : FormatString Is 'YYYYMMDDHHMMSS'.", sDate);

    int iTempYear = (sDate[0]-'0')*1000+(sDate[1]-'0')*100+(sDate[2]-'0')*10+(sDate[3]-'0');
    int iTempMon  = (sDate[4]-'0')*10 +(sDate[5]-'0');
    int iTempDay  = (sDate[6]-'0')*10 +(sDate[7]-'0');
    /*���괦��*/
    /*��ǰ����Ϊ����ȵĶ�����*/
    const int iCurrentDay=GetCurrentDays(iTempMon,iTempDay, IsLeapYear(iTempYear));
    long lRemainDays = iDays + iCurrentDay;
    if (lRemainDays > 0)
    {
        while (lRemainDays > GetTotalDaysOfYear(iTempYear))
        {
            lRemainDays -= GetTotalDaysOfYear(iTempYear);
            ++iTempYear;
        }
    }
    else
    {
        while (lRemainDays <= 0)
        {
            --iTempYear;
            lRemainDays += GetTotalDaysOfYear(iTempYear);
        }
    }

    SetMonthAndDay(iTempYear, static_cast<int>(lRemainDays), iTempMon, iTempDay);

    snprintf(sDate, 15, "%04d%02d%02d%.6s", iTempYear, iTempMon, iTempDay, sDate + DATE_SIZE);
}

/**********************************************************************
*  ��  �� �� // ���ַ���YYYYMMDDHHMMSSʱ���һ��
* ��������� // sDate
* ��������� // sDate
* �� �� ֵ�� // sDate + 1 ���ַ���
***********************************************************************/
void TDateTimeFunc::SubDate(char sDate[])
{
     if(strlen(sDate)!=14)
        throw TException("SubDate(Date=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sDate);

    //char sTempMon[3],sTempDay[3],sTempYear[5];
    int  iTempMon,iTempDay,iTempYear,iMonthDays;

    /*strncpy(sTempYear,sDate,4); sTempYear[4]=0;
    strncpy(sTempMon,&sDate[4],2); sTempMon[2]=0;
    strncpy(sTempDay,&sDate[6],2); sTempDay[2]=0;
    iTempYear=atoi(sTempYear);
    iTempMon=atoi(sTempMon);
    iTempDay=atoi(sTempDay);
    */
    
    iTempYear = (sDate[0]-'0')*1000+(sDate[1]-'0')*100+(sDate[2]-'0')*10+(sDate[3]-'0');
    iTempMon  = (sDate[4]-'0')*10 +(sDate[5]-'0');
    iTempDay  = (sDate[6]-'0')*10 +(sDate[7]-'0');

    iMonthDays = agiMonthC [iTempMon-1];

    /*���괦��*/
    if ( iTempMon ==2 && ( (iTempYear %4 == 0) && ( iTempYear%100 != 0 || iTempYear%400 ==0)) )
        iMonthDays++;

    if(iTempDay==1)
    {
        if(iTempMon==1)
        {
            iTempMon=12;
            iTempYear-=1;
        }
        else
            iTempMon-=1;

        iTempDay = agiMonthC [iTempMon-1];
        /*���괦��*/
        if ( iTempMon ==2 && ( (iTempYear %4 == 0) && ( iTempYear%100 != 0 || iTempYear%400 ==0)) )
            iTempDay++;
    }
    else iTempDay-=1;

    //sprintf(sDate,"%04d%02d%02d%s",iTempYear,iTempMon,iTempDay,&sDate[8]);
    sDate[0] = (char)(iTempYear/1000+'0');//ZMP:477242
    sDate[1] = (char)((iTempYear%1000)/100+'0');
    sDate[2] = (char)((iTempYear%100)/10+'0');
    sDate[3] = (char)(iTempYear%10+'0');

    sDate[4] = (char)(iTempMon/10+'0');
    sDate[5] = (char)(iTempMon%10+'0');
    
    sDate[6] = (char)(iTempDay/10+'0');
    sDate[7] = (char)(iTempDay%10+'0');
    
}

/**********************************************************************
*  ��  �� �� // ���ַ���YYYYMMDDHHMMSSʱ���һ��
* ��������� // sDate
* ��������� // sDate
* �� �� ֵ�� // sDate + 1 ���ַ���
***********************************************************************/
void TDateTimeFunc::AddDate(char sDate[])
{
    if(strlen(sDate)!=14)
        throw TException("AddDate(Date=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sDate);

    //char sTempMon[3],sTempDay[3],sTempYear[5];
    int  iTempMon,iTempDay,iTempYear,iMonthDays;
   /*
    strncpy(sTempYear,sDate,4); sTempYear[4]=0;
    strncpy(sTempMon,&sDate[4],2); sTempMon[2]=0;
    strncpy(sTempDay,&sDate[6],2); sTempDay[2]=0;
    iTempYear=atoi(sTempYear);
    iTempMon=atoi(sTempMon);
    iTempDay=atoi(sTempDay);
   */
    iTempYear = (sDate[0]-'0')*1000+(sDate[1]-'0')*100+(sDate[2]-'0')*10+(sDate[3]-'0');
    iTempMon  = (sDate[4]-'0')*10 +(sDate[5]-'0');
    iTempDay  = (sDate[6]-'0')*10 +(sDate[7]-'0');


    iMonthDays = agiMonthC [iTempMon-1];

    /*���괦��*/
    if ( iTempMon ==2 && ( (iTempYear %4 == 0) && ( iTempYear%100 != 0 || iTempYear%400 ==0)) )
        iMonthDays++;

    if(iTempDay>=iMonthDays)
    {
        iTempDay=1;
        if(iTempMon>=12)
        {
            iTempMon=1;
            iTempYear+=1;
        }
        else iTempMon+=1;
    }
    else iTempDay+=1;

    //sprintf(sDate,"%04d%02d%02d%s",iTempYear,iTempMon,iTempDay,&sDate[8]);
    sDate[0] = (char)(iTempYear/1000+'0');//ZMP:477242
    sDate[1] = (char)((iTempYear%1000)/100+'0');
    sDate[2] = (char)((iTempYear%100)/10+'0');
    sDate[3] = (char)(iTempYear%10+'0');

    sDate[4] = (char)(iTempMon/10+'0');
    sDate[5] = (char)(iTempMon%10+'0');
    
    sDate[6] = (char)(iTempDay/10+'0');
    sDate[7] = (char)(iTempDay%10+'0');

}

/**********************************************************************
*  ��  �� �� // ���ַ���YYYYMMDDHHMMSSʱ�������
* ��������� // sDate
* ��������� // 
* �� �� ֵ�� // �ܼ�
***********************************************************************/
int TDateTimeFunc::GetWeek(const char sDate[])
{
//====================================================
//138842 begin:
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif
//138842 end!
//====================================================/   

    time_t tm = StringToTime(sDate);
    
//====================================================
//138842 begin:
#ifdef _IMP_DATE
    const time_t dt = tm + TIME_DIFFERENCE ;
    struct tm *tm_Cur = gmtime(&dt); 
#else	
    struct tm *tm_Cur = localtime(&tm); //ȡ�õ�ǰʱ���tmֵ
#endif 
//138842 end!
//====================================================/

    return tm_Cur->tm_wday;
}

/**********************************************************************
*  ��  �� �� // ����time_tʱ���Ӧ������
* ��������� // time_t
* ��������� // 
* �� �� ֵ�� // �ܼ�
***********************************************************************/
int TDateTimeFunc::GetWeek(const time_t tm)
{
//====================================================
//138842 begin:
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif
//138842 end!
//====================================================/ 
  
//====================================================
//138842 begin:
#ifdef _IMP_DATE
    const time_t dt = tm + TIME_DIFFERENCE ;
    struct tm *tm_Cur = gmtime(&dt); 
#else	
    struct tm *tm_Cur = localtime(&tm); //ȡ�õ�ǰʱ���tmֵ
#endif 
//138842 end!
//====================================================/
  
  return tm_Cur->tm_wday;
}

/**********************************************************************
*  ��  �� �� // ���ַ���YYYYMMDDHHMMSSʱ�������
* ��������� // sDate
* ��������� // 
* �� �� ֵ�� // �ܼ�
***********************************************************************/
int TDateTimeFunc::GetGMWeek(const char sDate[])
{
  time_t tm = StringToGMTime(sDate);
  struct tm *tm_Cur = gmtime(&tm); //ȡ�õ�ǰʱ���tmֵ
  return tm_Cur->tm_wday;
}

/**********************************************************************
*  ��  �� �� // ����time_tʱ���Ӧ������
* ��������� // time_t
* ��������� // 
* �� �� ֵ�� // �ܼ�
***********************************************************************/
int TDateTimeFunc::GetGMWeek(const time_t tm)
{
  struct tm *tm_Cur = gmtime(&tm); //ȡ�õ�ǰʱ���tmֵ
  return tm_Cur->tm_wday;
}

/**********************************************************************
*  ��  �� ����ǰ����Ϊ����ȵĶ�����
* ��������� // ��,��
* ��������� //
* �� �� ֵ�� // ����
***********************************************************************/
/*
int  TDateTimeFunc::GetCurrentDays(const int iMonth,const int iDay,int iTag)
{

    //����ַ���ʱ���Ƿ�Ϸ�
#ifdef _ASSERT_PARAM
  if((iMonth>12)||(iMonth<1))
  {
    //�·�Խ��
    throw TException("GetCurrentDays():montherror: [%d]'.",iMonth);
  }

  int month_day[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if( iDay < 1 )
  {
    throw TException("GetCurrentDays():dateerror:[%d]",iDay);
  }
  if( (2 == iMonth)
   && iTag  )
  {
    month_day[1] += 1;
  }
  if( iDay > month_day[iMonth-1] )
  {
    throw TException("CheckYearString():dateerror:[%d]",iDay);
  }
#endif

    if((iDay>31)||(iDay<0))
        throw TException("Input parameter iDay Error [%d]'.",iDay);

    int iTempMon,icurrentdays=0;
    iTempMon=iMonth;

        switch(iTempMon)
        {
            case 1:
            //icurrentdays = 31;
            break;
         case 2:
            icurrentdays = 31;
            break;
          case 3:
             if(iTag)
                 icurrentdays = 60;     //31+29
            else
                icurrentdays = 59;     //31+28
            break;
          case 4:
             if(iTag)
                 icurrentdays = 91;     //31+29+31
            else
                icurrentdays = 90;     //31+28+31
            break;
          case 5:
             if(iTag)
                 icurrentdays = 121;     //31+29+31+30
            else
                icurrentdays = 120;
            break;
          case 6:
             if(iTag)
                 icurrentdays = 152;     //31+29+31+30+31
            else
                icurrentdays = 151;
            break;
          case 7:
             if(iTag)
                 icurrentdays = 182;    //31+29+31+30+31+30
            else
                icurrentdays = 181;
            break;
          case 8:
             if(iTag)
                 icurrentdays = 213;    //31+29+31+30+31+30+31
            else
                icurrentdays = 212;
            break;
          case 9:
             if(iTag)
                 icurrentdays = 244;     //31+29+31+30+31+30+31+31
            else
                icurrentdays = 243;
            break;
          case 10:
             if(iTag)
                 icurrentdays = 274;    //31+29+31+30+31+30+31+31+30
            else
                icurrentdays = 273;
            break;
          case 11:
             if(iTag)
                 icurrentdays = 305;    //31+29+31+30+31+30+31+31+30+31
            else
                icurrentdays = 304;
            break;
          case 12:
             if(iTag)
                 icurrentdays = 335;     //31+29+31+30+31+30+31+31+30+31+30
            else
                icurrentdays = 334;
            break;
           default:
             throw TException("MONTH ERROR:%d\n",iTempMon);
        }

     return icurrentdays+iDay;

}
*/
int  TDateTimeFunc::GetCurrentDays(const int iMonth,const int iDay, bool bIsLeapYear)
{

    //����ַ���ʱ���Ƿ�Ϸ�
#ifdef _ASSERT_PARAM
  if((iMonth>12)||(iMonth<1))
  {
    //�·�Խ��
    throw TException("GetCurrentDays():montherror: [%d]'.",iMonth);
  }

  int month_day[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if( iDay < 1 )
  {
    throw TException("GetCurrentDays():dateerror:[%d]",iDay);
  }
  if( (2 == iMonth)
   && bIsLeapYear)
  {
    month_day[1] += 1;
  }
  if( iDay > month_day[iMonth-1] )
  {
    throw TException("CheckYearString():dateerror:[%d]",iDay);
  }
#endif

    if((iDay>31)||(iDay<0))
        throw TException("Input parameter iDay Error [%d]'.",iDay);

    int icurrentdays=0;
    if (bIsLeapYear)
    {
        icurrentdays = GetCurrentDaysForLeapYear(iMonth, iDay);
    }
    else
    {
        icurrentdays = GetCurrentDaysForCommonYear(iMonth, iDay);
    }
    return icurrentdays+iDay;
}

int TDateTimeFunc::GetTotalDaysOfYear(int iYear)
{
    return IsLeapYear(iYear) ? 366 : 365;
}


int  TDateTimeFunc::GetCurrentDaysForCommonYear(const int iMonth,const int iDay)
{
    int icurrentdays = 0;
    switch(iMonth)
    {
      case 1:
        break;
      case 2:
        icurrentdays = 31;
        break;
      case 3:
            icurrentdays = 59;     //31+28
            break;
      case 4:
            icurrentdays = 90;     //31+28+31
            break;
      case 5:
            icurrentdays = 120;
            break;
      case 6:
            icurrentdays = 151;
            break;
      case 7:
            icurrentdays = 181;
            break;
      case 8:
            icurrentdays = 212;
            break;
      case 9:
            icurrentdays = 243;
            break;
      case 10:
            icurrentdays = 273;
            break;
      case 11:
            icurrentdays = 304;
            break;
      case 12:
            icurrentdays = 334;
            break;
      default:
         throw TException("MONTH ERROR:%d\n",iMonth);
    }
    return icurrentdays;
}

int  TDateTimeFunc::GetCurrentDaysForLeapYear(const int iMonth,const int iDay)
{
    int icurrentdays = 0;
    switch(iMonth)
    {
      case 1:
        break;
      case 2:
        icurrentdays = 31;
        break;
      case 3:
            icurrentdays = 60;     //31+29
            break;
      case 4:
            icurrentdays = 91;     //31+29+31
            break;
      case 5:
            icurrentdays = 121;     //31+29+31+30
            break;
      case 6:
            icurrentdays = 152;     //31+29+31+30+31
            break;
      case 7:
            icurrentdays = 182;    //31+29+31+30+31+30
            break;
      case 8:
            icurrentdays = 213;    //31+29+31+30+31+30+31
            break;
      case 9:
            icurrentdays = 244;     //31+29+31+30+31+30+31+31
            break;
      case 10:
            icurrentdays = 274;    //31+29+31+30+31+30+31+31+30
            break;
      case 11:
            icurrentdays = 305;    //31+29+31+30+31+30+31+31+30+31
            break;
      case 12:
            icurrentdays = 335;     //31+29+31+30+31+30+31+31+30+31+30
            break;
      default:
         throw TException("MONTH ERROR:%d\n",iMonth);
    }
    return icurrentdays;
}

void TDateTimeFunc::SetMonthAndDay(int iYear, int iDays, int &iMonth, int &iDate)
{
    //����ַ���ʱ���Ƿ�Ϸ�
#ifdef _ASSERT_PARAM
    if ((iDays < 1))
        throw TException("Input parameter idays Error [%d]'.", iDays);
#endif

    const int iTotalDays = GetTotalDaysOfYear(iYear);
    if (iDays > iTotalDays || (iDays < 0))
        throw TException("Input parameter idays Error [%d]'.", iDays);

    static const int iDaysInCommonYear[12] = {
        0,
        31,
        31 + 28,
        31 + 28 + 31,
        31 + 28 + 31 + 30,
        31 + 28 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 };
    static const int iDaysInLeapYear[12] = {
        0,
        31,
        31 + 29,
        31 + 29 + 31,
        31 + 29 + 31 + 30,
        31 + 29 + 31 + 30 + 31,
        31 + 29 + 31 + 30 + 31 + 30,
        31 + 29 + 31 + 30 + 31 + 30 + 31,
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 };

    const int *pDays = IsLeapYear(iYear) ? iDaysInLeapYear : iDaysInCommonYear;
    for (int i = 11; i >= 0; --i)
    {
        if (iDays > pDays[i])
        {
            iMonth = i + 1;
            iDate = iDays - pDays[i];
            break;
        }
    }
    if (iDate == 0)
    {
        iDate = 1;
    }
}

/**********************************************************************
*  ��  �� ��������������������  //ADDED BY SUNY
* ��������� //����, ��,��,�Ƿ�����
* ��������� // ��,��
* �� �� ֵ�� //
***********************************************************************/
/*
void  TDateTimeFunc::SetMonthDay(int &iMonth,int &iDay,int idays,int iTag)
{

   //����ַ���ʱ���Ƿ�Ϸ�
    #ifdef _ASSERT_PARAM
    if((idays<1))
        throw TException("Input parameter idays Error [%d]'.",idays);
    #endif

    if((iTag&&idays>366)||(!iTag&&idays>365)||(idays<0))
        throw TException("Input parameter idays Error [%d]'.",idays);

    if(iTag)
    {
        if(idays>335)   //31+29+31+30+31+30+31+31+30+31+30
            {
                iMonth = 12;
                iDay = idays-335;
            }
        else
        if(idays>305)   //31+29+31+30+31+30+31+31+30+31
            {
                iMonth = 11;
                iDay = idays-305;
            }
        else
        if(idays>274)  //(31+29+31+30+31+30+31+31+30)
            {
                iMonth = 10;
                iDay = idays-274;
            }
        else
        if(idays>244)  //31+29+31+30+31+30+31+31
            {
                iMonth = 9;
                iDay = idays-244;
            }
        else
        if(idays>213)  //(31+29+31+30+31+30+31)
            {
                iMonth = 8;
                iDay = idays-213;
            }
        else
        if(idays>182)  //31+29+31+30+31+30
            {
                iMonth = 7;
                iDay = idays-(31+29+31+30+31+30);
            }
        else
        if(idays>152)  //31+29+31+30+31
            {
                iMonth = 6;
                iDay = idays-152;
            }
        else
        if(idays>121)  //31+29+31+30
            {
                iMonth = 5;
                iDay = idays-121;
            }
        else
        if(idays>91)   //31+29+31
            {
                iMonth = 4;
                iDay = idays-91;
            }
        else
        if(idays>60)  //31+29
            {
                iMonth = 3;
                iDay = idays-60;
            }
        else
        if(idays>31)
            {
                iMonth = 2;
                iDay = idays-31;
            }
        else
        if(idays>=0)
            {
                iMonth = 1;
                iDay = idays;
                if(iDay==0) iDay=1;
            }
    }
    else
    {
        if(idays>334)              //31+28+31+30+31+30+31+31+30+31+30
            {
                iMonth = 12;
                iDay = idays-334;
            }
        else
        if(idays>304)                 //31+28+31+30+31+30+31+31+30+31
            {
                iMonth = 11;
                iDay = idays-304;
            }
        else
        if(idays>273)                    //31+28+31+30+31+30+31+31+30
            {
                iMonth = 10;
                iDay = idays-273;
            }
        else
        if(idays>243)                     //31+28+31+30+31+30+31+31
            {
                iMonth = 9;
                iDay = idays-243;
            }
        else
        if(idays>212)                          //31+28+31+30+31+30+31
            {
                iMonth = 8;
                iDay = idays-212;
            }
        else
        if(idays>181)                             //31+28+31+30+31+30
            {
                iMonth = 7;
                iDay = idays-181;
            }
        else
        if(idays>151)                               //31+28+31+30+31
            {
                iMonth = 6;
                iDay = idays-151;
            }
        else
        if(idays>120)                                  //31+28+31+30
            {
                iMonth = 5;
                iDay = idays-120;
            }
        else
        if(idays>90)                                     //31+28+31
            {
                iMonth = 4;
                iDay = idays-90;
            }
        else
        if(idays>59)                                         //31+28
            {
                iMonth = 3;
                iDay = idays-59;
            }
        else
        if(idays>31)
            {
                iMonth = 2;
                iDay = idays-31;
            }
        else
        if(idays>=0)
            {
                iMonth = 1;
                iDay = idays;
                if(iDay==0) iDay=1;
            }
    }
}
*/

//315426 begin
void  TDateTimeFunc::SetMonthDay(int &iMonth,int &iDay,int idays, bool bIsLeapYear)
{

   //����ַ���ʱ���Ƿ�Ϸ�
    #ifdef _ASSERT_PARAM
    if((idays<1))
        throw TException("Input parameter idays Error [%d]'.",idays);
    #endif

    if((bIsLeapYear&&idays>366)||(!bIsLeapYear&&idays>365)||(idays<0))
        throw TException("Input parameter idays Error [%d]'.",idays);

    if(bIsLeapYear)
    {
        SetMonthDayForLeapYear(iMonth,iDay,idays) ;
    }
    else
    {
        SetMonthDayForCommonYear(iMonth,iDay,idays);
    }
}
void TDateTimeFunc::SetMonthDayForLeapYear(int &iMonth,int &iDay,int idays)
{
    if(idays>335)   //31+29+31+30+31+30+31+31+30+31+30
        {
            iMonth = 12;
            iDay = idays-335;
        }
    else
    if(idays>305)   //31+29+31+30+31+30+31+31+30+31
        {
            iMonth = 11;
            iDay = idays-305;
        }
    else
    if(idays>274)  //(31+29+31+30+31+30+31+31+30)
        {
            iMonth = 10;
            iDay = idays-274;
        }
    else
    if(idays>244)  //31+29+31+30+31+30+31+31
        {
            iMonth = 9;
            iDay = idays-244;
        }
    else
    if(idays>213)  //(31+29+31+30+31+30+31)
        {
            iMonth = 8;
            iDay = idays-213;
        }
    else
    if(idays>182)  //31+29+31+30+31+30
        {
            iMonth = 7;
            iDay = idays-(31+29+31+30+31+30);
        }
    else
    if(idays>152)  //31+29+31+30+31
        {
            iMonth = 6;
            iDay = idays-152;
        }
    else
    if(idays>121)  //31+29+31+30
        {
            iMonth = 5;
            iDay = idays-121;
        }
    else
    if(idays>91)   //31+29+31
        {
            iMonth = 4;
            iDay = idays-91;
        }
    else
    if(idays>60)  //31+29
        {
            iMonth = 3;
            iDay = idays-60;
        }
    else
    if(idays>31)
        {
            iMonth = 2;
            iDay = idays-31;
        }
    else
    if(idays>=0)
        {
            iMonth = 1;
            iDay = idays;
            if(iDay==0) iDay=1;
        }
}

void TDateTimeFunc::SetMonthDayForCommonYear(int &iMonth,int &iDay,int idays)
{
    if(idays>334)              //31+28+31+30+31+30+31+31+30+31+30
        {
            iMonth = 12;
            iDay = idays-334;
        }
    else
    if(idays>304)                 //31+28+31+30+31+30+31+31+30+31
        {
            iMonth = 11;
            iDay = idays-304;
        }
    else
    if(idays>273)                    //31+28+31+30+31+30+31+31+30
        {
            iMonth = 10;
            iDay = idays-273;
        }
    else
    if(idays>243)                     //31+28+31+30+31+30+31+31
        {
            iMonth = 9;
            iDay = idays-243;
        }
    else
    if(idays>212)                          //31+28+31+30+31+30+31
        {
            iMonth = 8;
            iDay = idays-212;
        }
    else
    if(idays>181)                             //31+28+31+30+31+30
        {
            iMonth = 7;
            iDay = idays-181;
        }
    else
    if(idays>151)                               //31+28+31+30+31
        {
            iMonth = 6;
            iDay = idays-151;
        }
    else
    if(idays>120)                                  //31+28+31+30
        {
            iMonth = 5;
            iDay = idays-120;
        }
    else
    if(idays>90)                                     //31+28+31
        {
            iMonth = 4;
            iDay = idays-90;
        }
    else
    if(idays>59)                                         //31+28
        {
            iMonth = 3;
            iDay = idays-59;
        }
    else
    if(idays>31)
        {
            iMonth = 2;
            iDay = idays-31;
        }
    else
    if(idays>=0)
        {
            iMonth = 1;
            iDay = idays;
            if(iDay==0) iDay=1;
        }
}
//315426 end
const char * TDateTimeFunc::GMTimeToString(const time_t tTime,const bool bLongFlag)
{
    static char sCurtime[32]={0}; 
    struct tm * tm_Cur;

    sCurtime[31]=0;
    tm_Cur = gmtime(&tTime);
    if(bLongFlag)
    {
#ifdef _IMP_DATE
        timeToStr2(sCurtime,tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);  
#else
        //ZMP:467372
        timeToStr2(sCurtime, tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
#endif 
    }
    else
    {
#ifdef _IMP_DATE
        timeToStdStr(sCurtime,sizeof(sCurtime),tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);  
#else
        //ZMP:467372
        timeToStdStr(sCurtime, sizeof(sCurtime), tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
#endif 
    }

    return sCurtime;
}

/**********************************************************************
*  ��  �� �� // ���ַ���YYYYMMDDHHMMSSʱ�����ʽת��Ϊϵͳ��time_t��
* ��������� // sTime  ��ʱ����ַ���YYYYMMDDHHMMSS��ʽ
* ��������� // ��
* �� �� ֵ�� // time_t��ʽ��ʱ���ʾ(��ʱ����������ʱ)
***********************************************************************/
time_t TDateTimeFunc::StringToGMTime(const char sTime[])
{
  size_t iLen=strlen(sTime);//ZMP:477242

  if((iLen!=14)&&(iLen!=8)&&(iLen != 0))
    throw TException("StringToGMTime(Time=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime);

  if(iLen == 0)
  {
    return -1;
  }

  static int iOffset = (int)((12 * 3600) - StringToTime("19700101120000"));
  static char sTempYear[5]={0},sTempMon[3]={0},sTempMDay[3]={0},sTempHour[3]={0},sTempMin[3]={0},sTempSec[3]={0}; 
  strncpy(sTempYear,sTime,4);sTempYear[4]=0;
  sTempMon[0]=sTime[4];
  sTempMon[1]=sTime[5];
  sTempMDay[0]=sTime[6]; sTempMon[2]=0;
  sTempMDay[1]=sTime[7]; sTempMDay[2]=0;

  // ֧��8λ
  if(8 == iLen)
  {
    sTempHour[0] = '0';
    sTempHour[1] = '0';
    sTempHour[2]=0;

    sTempMin[0]= '0';
    sTempMin[1]= '0';
    sTempMin[2]=0;

    sTempSec[0]= '0';
    sTempSec[1]= '0';
    sTempSec[2]=0;
  }
  else  // 14λ
  {
    sTempHour[0]=sTime[8];
    sTempHour[1]=sTime[9]; sTempHour[2]=0;

    sTempMin[0]=sTime[10];
    sTempMin[1]=sTime[11]; sTempMin[2]=0;

    sTempSec[0]=sTime[12];
    sTempSec[1]=sTime[13]; sTempSec[2]=0;
  }

  static struct tm s;
  s.tm_year=atoi(sTempYear)-1900;
  s.tm_mon=atoi(sTempMon)-1;
  s.tm_mday=atoi(sTempMDay);
  s.tm_hour=atoi(sTempHour);
  s.tm_min=atoi(sTempMin);
  s.tm_sec=atoi(sTempSec);
  s.tm_isdst=(int)0;    //::daylight �Ƿ������չ��Լʱ�䡣��0Ϊ����������������ʱ�Գ���ʱ���Ӱ�졣

  return mktime(&s) + iOffset;
}



/**********************************************************************
*  ��  �� �� // ��ȡ�������ڵ���iMonths���µ�����
* ��������� // sDate �ο�����,iMonths �Ӻ������
* ��������� // 
* �� �� ֵ�� // void
***********************************************************************/
void TDateTimeFunc::AddMonths(char sDate[],long iMonths)
{
    if(strlen(sDate)!=14)
        throw TException("AddMonths(Date=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sDate);

    int month_day[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int d1 = 0, m1 = 0, y1 = 0;
    sscanf( sDate, "%04d%02d%02d", &y1, &m1, &d1);

    //����ת������
    int iCurMonth = y1*12 + m1;
    iCurMonth += (int)iMonths;

    y1 = iCurMonth/12;
    m1 = iCurMonth%12;
    // m1 = (m1==0)?12:m1;
    if(0 == m1)
    {
        m1 = 12;
        y1 -= 1;
    }

    if ( (y1 %4 == 0) && ( y1%100 != 0 || y1%400 ==0) )
        month_day[1] = 29;

    if(m1==2)
    {
        d1 = d1>month_day[1]?month_day[1]:d1;
    }
    else if(d1==31 && month_day[m1-1]==30)
    {
        d1=30;
    }

//=====================================================
//129457 begin:
#ifdef _IMP_DATE
    timeToStr4(sDate,y1,m1,d1);
#else
    //ZMP:467372
    snprintf( sDate, 15, "%04d%02d%02d000000", y1, m1, d1);
    sDate[14] = '\0';
#endif 
//129457 end!
//=====================================================/		
}

/**********************************************************************
*  ��  �� �� // ��ȡ������������֮��������
* ��������� // sTime1ʱ��1 sTime2ʱ��2
* ��������� // 
* �� �� ֵ�� // sTime1 - sTime2
***********************************************************************/
long TDateTimeFunc::GetDiffMon(const char sTime1[], const char sTime2[])
{
    if(strlen(sTime1)!=14||strlen(sTime2)!=14)
        throw TException("GetDiffMon(Time1=%s,Time2=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime1,sTime2);

    int d1 = 0, m1 = 0, y1 = 0;
    sscanf( sTime1, "%04d%02d%02d", &y1, &m1, &d1 );

    int d2 = 0, m2 = 0, y2 = 0;
    sscanf( sTime2, "%04d%02d%02d", &y2, &m2, &d2 );

    return (y1-y2)*12 + m1 - m2 +((d1-d2)>=0?1:0);
}

/**********************************************************************
*  ��  �� �� // ��ȡ�������ڵ��� iYears �������
* ��������� // sDate �ο�����,iYears �Ӻ������
* ��������� // 
* �� �� ֵ�� // void
***********************************************************************/
void TDateTimeFunc::AddYears(char sDate[],long iYears)
{
    if (strlen(sDate)!=14)
        throw TException("AddYears(Date=%s) : FormatString Is 'YYYYMMDDHHMMSS'.", sDate);

    int month_day[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int d1 = 0, m1 = 0, y1 = 0, h1 = 0, M1 = 0, s1 = 0;
    sscanf(sDate, "%04d%02d%02d%02d%02d%02d", &y1, &m1, &d1, &h1, &M1, &s1);

    y1 += (int)iYears;

    if ( (y1 % 4 == 0) && (y1 % 100 != 0 || y1 % 400 == 0) )
        month_day[1] = 29;

    if (m1==2)
    {
        d1 = d1>month_day[1]?month_day[1]:d1;
    }
    
    timeToStdStr(sDate, 15, y1, m1, d1, h1, M1, s1);
}




//////////////////////////////////////////////////////////////////////////
/*�ж�ָ�����ڣ��Ƿ�����������֮����*/
bool TDateTimeFunc::BetweenDate(const char *dInDate,const char *dBeginDate,const char *dEndDate)
{
    //����ַ���ʱ���Ƿ�Ϸ�
    #ifdef _ASSERT_PARAM
        if(NULL == dInDate)
        {
            throw TException("BetweenDate():dInDate is NULL");           
        } 
    #endif 

		if(!dBeginDate)
    {
        return strcmp(dInDate,dEndDate)<0;
    }
    else if(!dEndDate)
    {
        return strcmp(dInDate,dBeginDate)>=0;
    }
    else
    {
        return strcmp(dInDate,dBeginDate)>=0 && strcmp(dInDate,dEndDate)<0;
    }
}


/**********************************************************************
*  ��         �� ��  �ж�ָ�����ڣ��Ƿ�����������֮����
* ��������� dInDate : ָ��time_t ʱ��ֵ;
                                dBeginDate : �ȽϵĿ�ʼʱ��time_t ʱ��ֵ;
                                dEndDate : �ȽϵĽ���ʱ��time_t ʱ��ֵ
* ��������� ��
* �� ��    ֵ�� bool : true �ڿ�ʼʱ��ͽ���ʱ��֮��
                                        false���ڿ�ʼʱ��ͽ���ʱ��֮��
***********************************************************************/
bool TDateTimeFunc::BetweenDate(time_t dInDate,time_t  dBeginDate,time_t dEndDate)
{
    return ( dInDate >= dBeginDate  && dInDate < dEndDate );
}

/*��ȡ��ǰ����ʱ��
sDateTime     Ϊ��������ʱ���ַ�����ʽΪYYYYMMDDHHMISS
iDateTimeFlag Ϊ����ʱ���ʶ
Ϊ0 ���ص�ǰ����ʱ��
Ϊ1 ���ص�ǰ����
Ϊ2 ���ص�ǰʱ��
*/
void TDateTimeFunc::GetNowDateTime(char *sDateTime,int iDateTimeFlag)
{
    //����ַ���ʱ���Ƿ�Ϸ�
    #ifdef _ASSERT_PARAM
    if(NULL==sDateTime)
    {
         throw TException("sDateTime is NULL");    
    }        
    if((iDateTimeFlag>2)||(iDateTimeFlag<0))
    {
         throw TException("iDateTimeFlag is error");        
    }
    #endif 

//=====================================================
//129457 begin:
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif
//129457 end!
//=====================================================/ 

    time_t timeNow;
    struct tm *ptmNow;

    time(&timeNow);

//=====================================================
//129457 begin:
#ifdef _IMP_DATE
    const time_t dt = timeNow + TIME_DIFFERENCE ;
    ptmNow = gmtime(&dt);
    if(iDateTimeFlag==0)
    {   /*����ʱ��*/
        timeToStdStr(sDateTime,15,ptmNow->tm_year+1900,ptmNow->tm_mon+1,ptmNow->tm_mday,
            ptmNow->tm_hour,ptmNow->tm_min,ptmNow->tm_sec); 
    }
    else if(iDateTimeFlag==1)
    {   /*����*/
        timeToStr5(sDateTime,ptmNow->tm_year+1900,ptmNow->tm_mon+1,ptmNow->tm_mday);  
    }
    else if(iDateTimeFlag==2)
    {   /*ʱ��*/
        timeToStr6(sDateTime,ptmNow->tm_hour,ptmNow->tm_min,ptmNow->tm_sec);
    }     
#else		
    ptmNow=localtime(&timeNow);

    if(iDateTimeFlag==0)
    {   /*����ʱ��*/
        //ZMP:467372
        timeToStdStr(sDateTime, 15, ptmNow->tm_year+1900,ptmNow->tm_mon+1,ptmNow->tm_mday,
            ptmNow->tm_hour,ptmNow->tm_min,ptmNow->tm_sec);
        sDateTime[14] = '\0';
    }
    else if(iDateTimeFlag==1)
    {   /*����*/
        //ZMP:467372
        snprintf(sDateTime, 9, "%04d%02d%02d",ptmNow->tm_year+1900,ptmNow->tm_mon+1,ptmNow->tm_mday);
        sDateTime[8] = '\0';
    }
    else if(iDateTimeFlag==2)
    {   /*ʱ��*/
        //ZMP:467372
        snprintf(sDateTime, 7, "%02d%02d%02d",ptmNow->tm_hour,ptmNow->tm_min,ptmNow->tm_sec);
        sDateTime[6] = '\0';
    }
#endif 
//129457 end!
//=====================================================/
}

//�Ƚ�YYYYMMDD���֣�����MMhhSS���֣�С���򷵻�-1�������򷵻�0�������򷵻�1
int TDateTimeFunc::CompareDate(time_t iTime1, time_t iTime2)
{
#ifdef _IMP_DATE
    //ֱ���õ�ʱ���
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif

#ifdef _IMP_DATE
    const time_t dt1 = iTime1 + TIME_DIFFERENCE ;
    const time_t dt2 = iTime2 + TIME_DIFFERENCE ;
	struct tm tm_date1;
	struct tm tm_date2;
	gmtime_r(&dt1,&tm_date1);
	gmtime_r(&dt2,&tm_date2);
#else
	struct tm tm_date1;
	struct tm tm_date2;
	localtime_r(&iTime1,&tm_date1);
	localtime_r(&iTime2,&tm_date2);
#endif

    int time1 = (tm_date1.tm_year+1900)*10000 + (tm_date1.tm_mon+1)*100 + (tm_date1.tm_mday);
	int time2 = (tm_date2.tm_year+1900)*10000 + (tm_date2.tm_mon+1)*100 + (tm_date2.tm_mday);

	if(time1 == time2)
	{
		return 0;
	}else if(time1 > time2)
	{
		return 1;
	}else
	{
		return -1;
	}
}

//ȥ��iTime�е�HHmmSS���ֺ󣬷��ض�Ӧ��time_tֵ��Ҳ���ǻ�ȡYYYYMMDD���ֵ�time_tֵ
time_t  TDateTimeFunc::GetDate(time_t iTime)
{
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif

	struct tm tm_date;

#ifdef _IMP_DATE
    const time_t dt = iTime + TIME_DIFFERENCE ;
	gmtime_r(&dt,&tm_date);
#else
    localtime_r(&iTime,&tm_date);
#endif

	tm_date.tm_hour=0;
	tm_date.tm_min=0;
	tm_date.tm_sec=0;
	return mktime(&tm_date);
}

/**********************************************************************
*  ��         �� ��  ��ȡ����ʱ�����iMonths���µ�ʱ��ֵ
* ��������� iTime : time_t ʱ��ֵ;
                               iMonths : ����
* ��������� iTime
* �� ��    ֵ�� ����ʱ�����iMonths���µ�ʱ��ֵ
***********************************************************************/
/*
time_t TDateTimeFunc::AddMonths(time_t& iTime, long iMonths)
{
    char* sTime = (char*)TimeToString(iTime);
    AddMonths(sTime, iMonths);
    return (iTime = StringToTime(sTime));
}
*/

//150175 begin
time_t TDateTimeFunc::AddMonths(time_t iTime, long iMonths)
{
    char sTime[DATE_TIME_SIZE] = {'\0'};
    TimeToString(sTime, DATE_TIME_SIZE, iTime);
    AddMonths(sTime, iMonths);
    return StringToTime(sTime);
}
//150175 end
/**********************************************************************
*  ��         �� ��  ��ȡtime_tʱ��ֵ���� iYears ���ʱ��ֵ
* ��������� iTime : time_t ʱ��ֵ;
                               iYears : ����
* ��������� iTime
* �� ��    ֵ�� �� iYears ���ʱ��ֵ
***********************************************************************/
time_t TDateTimeFunc::AddYears(time_t iTime,long iYears)
{
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif

#ifdef _IMP_DATE
    const time_t dt = iTime + TIME_DIFFERENCE ;
    struct tm * pTm = gmtime(&dt);
#else
    struct tm * pTm = localtime(&iTime);
#endif

    pTm->tm_year += (int)iYears;

    int year = 1900+pTm->tm_year;
    int month_day[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if ( (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0) )
        month_day[1] = 29;

    if (pTm->tm_mon+1==2)
    {
        pTm->tm_mday = (pTm->tm_mday > month_day[1])?month_day[1]:pTm->tm_mday;
    }
    //return (iTime=mktime(pTm));
	
//150175 begin
    return mktime(pTm);
//150175 begin
}

/**********************************************************************
*  ��         �� �� ��ʱ���iDays��
* ��������� iTime: ʱ��,
                               iDays: ����
* ��������� ��
* �� ��    ֵ�� iTime + iDay��
***********************************************************************/
/*
time_t TDateTimeFunc::AddDays(time_t& iTime,long iDays)
{
    return (iTime += iDays*86400);
}
*/
//150175 begin
time_t TDateTimeFunc::AddDays(time_t iTime,long iDays)
{
    time_t TimeDifference = AdjustTimeZone(iTime,iTime + iDays*86400);   
    return (iTime + iDays*86400 + TimeDifference);

}
//150175 end

/**********************************************************************
*  ��         �� �� ���㶬����ʱ�л���������
* ��������� iTime1, iTime2
* ��������� ��
* �� ��    ֵ��  TimeDifference
***********************************************************************/
long TDateTimeFunc::AdjustTimeZone(time_t iTime1, time_t iTime2)
{
    long TimeDifference = 0;
    long GmTime1 = 0;
    long GmTime2 = 0;
    struct tm DestLocal = *localtime(&iTime1);   //��time_t ת��Ϊ ����ʱ��
    struct tm DestGm    = *gmtime   (&iTime1);   //��time_t ת��Ϊ UTCʱ��
    DestGm.tm_isdst =-1;    
    struct tm OriLocal = *localtime(&iTime2);
    struct tm OritGm   = *gmtime   (&iTime2);   
    OritGm.tm_isdst =-1;    
  
    GmTime1 =  mktime(&DestLocal) - mktime(&DestGm);
    GmTime2 =  mktime(&OriLocal) - mktime(&OritGm);   
    TimeDifference =  GmTime1 - GmTime2;

    return TimeDifference;
}


/**********************************************************************
*  ��         �� �� ����time_t ֵʱ������ĵõ�������
* ��������� iTime1, iTime2
* ��������� ��
* �� ��    ֵ��  iTime1 - iTime2 ������
***********************************************************************/
long TDateTimeFunc::GetDiffDay(time_t iTime1, time_t iTime2)
{
/*
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
*/
    if(iTime1==iTime2)
    {
        return 0L;
    }
    //zmp:1431973
    ReadConfig();

    if(m_bIsDstUsed)
    {
        return (long)(difftime(iTime1,iTime2)/86400);
    }
    else
    {
    getTimeDifference(); 
    iTime1 = iTime1 + TIME_DIFFERENCE;
    iTime2 = iTime2 + TIME_DIFFERENCE;
    }

    iTime1 = iTime1 - iTime1%86400;//ȡ����
    iTime2 = iTime2 - iTime2%86400;//ȡ����

    return (long)(difftime(iTime1,iTime2)/86400);//ZMP:477242
}

void TDateTimeFunc::ReadConfig(void)
{
    //��App.config�ж�ȡ�������б�ֻ��ȡһ��
    if(!m_bReadconfigFlag)
    {
        m_bReadconfigFlag = true;
        char sTemp[32] = {0};
        char sAppCfgName[256] = "";
        const char * theEnv=getenv("HOME");
        if(theEnv==NULL)
        {
            strncpy(sAppCfgName,"App.config", sizeof(sAppCfgName)-1);
            sAppCfgName[sizeof(sAppCfgName)-1] = '\0';
        }
        else
        {
            snprintf(sAppCfgName, sizeof(sAppCfgName), "%s/etc/App.config",theEnv);
        }
        TReadIni ReadIni(sAppCfgName); 
        ReadIni.ReadString("COMMON","IS_DST_USED",sTemp, "N");
        m_bIsDstUsed = sTemp[0] == 'Y';
    }
}


/**********************************************************************
*  ��         �� ��  ��ȡ����time_tʱ��֮��������
* ���������  iTime1 : time_t ʱ��1
                                iTime2 : time_t ʱ��2
* ���������  ��
* �� ��    ֵ��  iTime1 - iTime2 ������
***********************************************************************/
long TDateTimeFunc::GetDiffMonth(time_t iTime1, time_t iTime2)
{
#ifdef _IMP_DATE
    if(bFirst)
    {
        getTimeDifference();
        bFirst = false;
    }
#endif

    int y1 = 0, m1 = 0, d1 = 0;//ZMP:305593,����δ�õ� h1 = 0, M1 = 0, s1 = 0;
    int y2 = 0, m2 = 0, d2 = 0;//ZMP:305593,����δ�õ� h2 = 0, M2 = 0, s2 = 0;

    struct tm * pTm;

#ifdef _IMP_DATE
    time_t dt = iTime1 + TIME_DIFFERENCE ;
    pTm = gmtime(&dt);
#else
    pTm = localtime(&iTime1);
#endif
    
    y1 = pTm->tm_year+1900;
    m1 = pTm->tm_mon+1;
    d1 = pTm->tm_mday;
    //h1 = pTm->tm_hour;//ZMP:305593,����δ�õ�
    //M1 = pTm->tm_min; //ZMP:305593,����δ�õ�
    //s1 = pTm->tm_sec; //ZMP:305593,����δ�õ�

#ifdef _IMP_DATE
    dt = iTime2 + TIME_DIFFERENCE ;
    pTm = gmtime(&dt);
#else
    pTm = localtime(&iTime2);
#endif

    y2 = pTm->tm_year+1900;
    m2 = pTm->tm_mon+1;
    d2 = pTm->tm_mday;
    //h2 = pTm->tm_hour;//ZMP:305593,����δ�õ�
    //M2 = pTm->tm_min;//ZMP:305593,����δ�õ�
    //s2 = pTm->tm_sec;//ZMP:305593,����δ�õ�

    return (y1-y2)*12 + m1 - m2 +((d1-d2)>0?1:0);//ZMP:474932 
}


long TDateTimeFunc::GetDiffMonthWithOutDays(time_t iTime1, time_t iTime2)
{
#ifdef _IMP_DATE
        if(bFirst)
        {
            getTimeDifference();
            bFirst = false;
         }
#endif

        int y1 = 0, m1 = 0, d1 = 0;//ZMP:305593,����δ�õ� h1 = 0, M1 = 0, s1 = 0;
        int y2 = 0, m2 = 0, d2 = 0;//ZMP:305593,����δ�õ� h2 = 0, M2 = 0, s2 = 0;

        struct tm * pTm;

#ifdef _IMP_DATE
        time_t dt = iTime1 + TIME_DIFFERENCE ;
        pTm = gmtime(&dt);
#else
        pTm = localtime(&iTime1);
#endif

        y1 = pTm->tm_year+1900;
        m1 = pTm->tm_mon+1;
        d1 = pTm->tm_mday;

#ifdef _IMP_DATE
        dt = iTime2 + TIME_DIFFERENCE ;
        pTm = gmtime(&dt);
#else
        pTm = localtime(&iTime2);
#endif

        y2 = pTm->tm_year+1900;
        m2 = pTm->tm_mon+1;
        d2 = pTm->tm_mday;
        
        return (y1-y2)*12 + m1 - m2 ;//ZMP:474932 
}

void TDateTimeFunc::SplitString(const char sTime[],int record[])
{
  // long Remainder;
  long long time = atoll(sTime);
  //�õ����
  record[0] = (int)(time/10000000000);//ZMP:477242
  time = time%10000000000;
  //�õ��·�
  record[1] = (int)(time/100000000);
  time = time%100000000;
  //�õ�����
  record[2] = (int)(time/1000000);
  time = time%1000000;  
  //�õ�Сʱ
  record[3] = (int)(time/10000);
  time = time%10000; 
  //�õ�����
  record[4] = (int)(time/100);
  time = time%100; 
  //�õ���
  record[5] = (int)(time);                    
}

bool TDateTimeFunc::IsLeapYear(int iYear)
{
    return (iYear % 4 == 0 && (iYear % 100 != 0 || iYear % 400 == 0));
}

void TDateTimeFunc::CheckYearString(const char sTime[])
{
  if(sTime == NULL)
  {
    throw TException("CheckYearString():sTime is NULL");    
  }
  
  int record[6]={0,0,0,0,0,0}; 
  char str[15] = {0};//ZMP:477242
  //memset(str,0x00,sizeof(str));
  if(strlen(sTime) == 8)
  {
    //���ڰ�λ�Ĵ���
    strcat(str,sTime);
    strcat(str,"000000");
    SplitString(str,record);        
  }
  else
  {
    SplitString(sTime,record);        
  }        
  
  
  if((record[3]>23)||(record[3]<0))
  {
    //СʱԽ�� 
    throw TException("CheckYearString():hourerror:[%d]'.",record[3]);   
  }
  
  if((record[4]>59)||(record[4]<0))
  {
    //����Խ�� 
    throw TException("CheckYearString():Minuteerror: [%d]'.",record[4]);   
  }
  
  if((record[5]>59)||(record[5]<0))
  {
    //����Խ�� 
    throw TException("CheckYearString():Seconderror: [%d]'.",record[5]);   
  } 
  
  if((record[1]>12)||(record[1]<1))
  {
    //�·�Խ�� 
    throw TException("CheckYearString():montherror: [%d]'.",record[1]);   
  }  
  
  int month_day[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if( record[2] < 1 )
  {
    throw TException("CheckYearString():dateerror:[%d]",record[2]);    
  }
  if( (2 == record[1]) 
   && IsLeapYear(record[0]) )
  {
    month_day[1] += 1;    
  }
  if( record[2] > month_day[record[1]-1] )
  {
    throw TException("CheckYearString():dateerror:[%d]",record[2]);      
  }      
}

//=====================================================
//129457 begin:
//#ifdef _IMP_DATE
/**********************************************************************
*  ��         �� �� �õ�ʱ��
* ��������� ��
* ��������� ��
* �� ��    ֵ�� ��
***********************************************************************/    
void TDateTimeFunc::getTimeDifference()
{
#ifndef _WIN32
	char sGmtTime[40] = {0};//ZMP:477242
	char sLocalTime[40] = {0};
	//memset(sGmtTime,0x00,sizeof(sGmtTime));
	//memset(sLocalTime,0x00,sizeof(sLocalTime));
	time_t dtGmtTime,dtLocalTime;
	struct tm *tmpGmtTime,*tmpLocalTime;
	struct tm tmLocalTime;
	struct timeval tpTime;
	gettimeofday(&tpTime,NULL);	
	dtLocalTime = dtGmtTime = tpTime.tv_sec;
	tmpGmtTime = gmtime(&dtGmtTime);//UTCʱ��
    timeToStdStr(sGmtTime,sizeof(sGmtTime),tmpGmtTime->tm_year+1900,tmpGmtTime->tm_mon+1,tmpGmtTime->tm_mday,tmpGmtTime->tm_hour,tmpGmtTime->tm_min,tmpGmtTime->tm_sec); 
	tmpLocalTime = localtime_r(&dtLocalTime,&tmLocalTime);//����ʱ��
	timeToStdStr(sLocalTime,sizeof(sLocalTime),tmpLocalTime->tm_year+1900,tmpLocalTime->tm_mon+1,tmpLocalTime->tm_mday,tmpLocalTime->tm_hour,tmpLocalTime->tm_min,tmpLocalTime->tm_sec);
	TIME_DIFFERENCE = GetDiffTime_p(sLocalTime,sGmtTime);
#else
    time_t nTime = time(NULL);//��ȡ��ǰ������ʱ��
    struct tm tmLocal = *localtime(&nTime);
    struct tm tmGm    = *gmtime   (&nTime);//��time_t ת��Ϊ UTCʱ��
    TIME_DIFFERENCE = mktime(&tmLocal)-mktime(&tmGm);
#endif	
}

/**********************************************************************
*��    �ܣ���time_t��ʽʱ��ת�ɱ�׼��YYYYMMDDHH24MISS��ʽʱ�䣬������sprintfʮ��������strftime
*�������:����������ʱ���룬sCurtimeΪҪ���ص��ַ����飬uSize��sizeof(sCurtime)
*�������:YYYYMMDDHH24MISS��ʽʱ�䱣�浽sCurtime
*�� �� ֵ:��
***********************************************************************/ 
void TDateTimeFunc::timeToStdStr(char sCurtime[],size_t uSize,int year,int mon,int day, int hour, int min, int sec)
{
    if(uSize > 14)
    {
        sCurtime[0]=(char)('0'+year/1000);
        sCurtime[1]=(char)('0'+(year%1000)/100);
        sCurtime[2]=(char)('0'+(year%100)/10);
        sCurtime[3]=(char)('0'+year%10);
       
        sCurtime[4]=(char)('0'+mon/10);
        sCurtime[5]=(char)('0'+mon%10);
      
        sCurtime[6]=(char)('0'+day/10);
        sCurtime[7]=(char)('0'+day%10);
       
        sCurtime[8]=(char)('0'+hour/10);
        sCurtime[9]=(char)('0'+hour%10);
    
        sCurtime[10]=(char)('0'+min/10);        
        sCurtime[11]=(char)('0'+min%10);
        
        sCurtime[12]=(char)('0'+sec/10);
        sCurtime[13]=(char)('0'+sec%10);
        sCurtime[14]= 0; 
    }
}

//-----------------------------------------------------
//��    �ܣ��������sprintf(sCurtime,"%04d-%02d-%02d %02d:%02d:%02d",...)
//�������:
//�������:
//�� �� ֵ:
//-----------------------------------------------------
void TDateTimeFunc::timeToStr2(char *sCurtime,int year,int mon,int day, int hour, int min, int sec)
{
    sCurtime[0]=(char)('0'+year/1000);//ZMP:477242
    sCurtime[1]=(char)('0'+(year%1000)/100);
    sCurtime[2]=(char)('0'+(year%100)/10);
    sCurtime[3]=(char)('0'+year%10);
    sCurtime[4]='-';        
    sCurtime[5]=(char)('0'+mon/10);
    sCurtime[6]=(char)('0'+mon%10);
    sCurtime[7]='-';        
    sCurtime[8]=(char)('0'+day/10);
    sCurtime[9]=(char)('0'+day%10);
    sCurtime[10]=' ';        
    sCurtime[11]=(char)('0'+hour/10);
    sCurtime[12]=(char)('0'+hour%10);
    sCurtime[13]=':';
    sCurtime[14]=(char)('0'+min/10);        
    sCurtime[15]=(char)('0'+min%10);
    sCurtime[16]=':';        
    sCurtime[17]=(char)('0'+sec/10);
    sCurtime[18]=(char)('0'+sec%10);
    sCurtime[19]= 0; 
}

//-----------------------------------------------------
//��    �ܣ��������sprintf(sCurtime,"%s%02d%02d%02d",...)
//�������: strռ8λ���������ա�
//�������:
//�� �� ֵ:
//-----------------------------------------------------
void TDateTimeFunc::timeToStr3(char *sCurtime,const char* str, int hour, int min, int sec)
{

    strncpy(sCurtime,str,8);
    sCurtime[8]=(char)('0'+hour/10);//ZMP:477242
    sCurtime[9]=(char)('0'+hour%10);

    sCurtime[10]=(char)('0'+min/10);        
    sCurtime[11]=(char)('0'+min%10);
    
    sCurtime[12]=(char)('0'+sec/10);
    sCurtime[13]=(char)('0'+sec%10);
    sCurtime[14]= 0; 
}

//-----------------------------------------------------
//��    �ܣ��������sprintf( sDate, "%04d%02d%02d000000",...)
//�������:
//�������:
//�� �� ֵ:
//-----------------------------------------------------
void TDateTimeFunc::timeToStr4(char *sCurtime,int year,int mon,int day)
{
    sCurtime[0]=(char)('0'+year/1000);//ZMP:477242
    sCurtime[1]=(char)('0'+(year%1000)/100);
    sCurtime[2]=(char)('0'+(year%100)/10);
    sCurtime[3]=(char)('0'+year%10);
   
    sCurtime[4]=(char)('0'+mon/10);
    sCurtime[5]=(char)('0'+mon%10);
  
    sCurtime[6]=(char)('0'+day/10);
    sCurtime[7]=(char)('0'+day%10);
   
    sCurtime[8]='0';
    sCurtime[9]='0';

    sCurtime[10]='0';        
    sCurtime[11]='0';
    
    sCurtime[12]='0';
    sCurtime[13]='0';
    sCurtime[14]= 0;
}

//-----------------------------------------------------
//��    �ܣ��������sprintf(sDateTime,"%04d%02d%02d",...)
//�������:
//�������:
//�� �� ֵ:
//-----------------------------------------------------
void TDateTimeFunc::timeToStr5(char *sCurtime,int year,int mon,int day)
{
    sCurtime[0]=(char)('0'+year/1000);//ZMP:477242
    sCurtime[1]=(char)('0'+(year%1000)/100);
    sCurtime[2]=(char)('0'+(year%100)/10);
    sCurtime[3]=(char)('0'+year%10);
   
    sCurtime[4]=(char)('0'+mon/10);
    sCurtime[5]=(char)('0'+mon%10);
  
    sCurtime[6]=(char)('0'+day/10);
    sCurtime[7]=(char)('0'+day%10);
   
    sCurtime[8]= 0;
} 

//-----------------------------------------------------
//��    �ܣ��������sprintf(sDateTime,"%02d%02d%02d",...)
//�������: ʱ����
//�������:
//�� �� ֵ:
//-----------------------------------------------------
void TDateTimeFunc::timeToStr6(char *sCurtime,int hour,int min,int sec)
{
    sCurtime[0]=(char)('0'+hour/10);//ZMP:477242
    sCurtime[1]=(char)('0'+hour%10);

    sCurtime[2]=(char)('0'+min/10);        
    sCurtime[3]=(char)('0'+min%10);
    
    sCurtime[4]=(char)('0'+sec/10);
    sCurtime[5]=(char)('0'+sec%10);
    
    sCurtime[6]=0;    
} 

//#endif
//end!
//=====================================================


long TDateTimeFunc::GetDiffTime_p(const char sTime1[], const char sTime2[])
{
    if(strlen(sTime1)!=14||strlen(sTime2)!=14)
        throw TException("GetDiffTime_p(Time1=%s,Time2=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime1,sTime2);

    return (long)difftime(StringToTime_p(sTime1),StringToTime_p(sTime2));
}

time_t TDateTimeFunc::StringToTime_p(const char sTime[])
{
    size_t iLen=strlen(sTime);//ZMP:477242
    
    if((iLen!=14)&&(iLen!=8)&&(iLen != 0))
        throw TException("StringToTime(Time=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime);

    if(iLen == 0)
    {
        return -1;
    }

    //����ַ���ʱ���Ƿ�Ϸ�
    #ifdef _ASSERT_PARAM
        CheckYearString(sTime);
    #endif 

    //ʹ��localtime���Ը���::daylight��ʹ���ϵͳʱ������һ��
    time_t tCurrent = 0;
    localtime(&tCurrent);

    static char sTempYear[5]={0},sTempMon[3]={0},sTempMDay[3]={0},sTempHour[3]={0},sTempMin[3]={0},sTempSec[3]={0}; 
    strncpy(sTempYear,sTime,4);sTempYear[4]=0;
    sTempMon[0]=sTime[4];
    sTempMon[1]=sTime[5];
    sTempMDay[0]=sTime[6]; sTempMon[2]=0;
    sTempMDay[1]=sTime[7]; sTempMDay[2]=0;

    // ֧��8λ
    if(8 == iLen)
    {
        sTempHour[0] = '0';
        sTempHour[1] = '0';
        sTempHour[2]=0;

        sTempMin[0]= '0';
        sTempMin[1]= '0';
        sTempMin[2]=0;

        sTempSec[0]= '0';
        sTempSec[1]= '0';
        sTempSec[2]=0;
    }
    else  // 14λ
    {
        sTempHour[0]=sTime[8];
        sTempHour[1]=sTime[9]; sTempHour[2]=0;

        sTempMin[0]=sTime[10];
        sTempMin[1]=sTime[11]; sTempMin[2]=0;

        sTempSec[0]=sTime[12];
        sTempSec[1]=sTime[13]; sTempSec[2]=0;
    }

    static struct tm s;
    s.tm_year=atoi(sTempYear)-1900;
    s.tm_mon=atoi(sTempMon)-1;
    s.tm_mday=atoi(sTempMDay);
    s.tm_hour=atoi(sTempHour);
    s.tm_min=atoi(sTempMin);
    s.tm_sec=atoi(sTempSec);
    #ifdef __BORLANDC__
    s.tm_isdst=(int)-1;
    #else
    s.tm_isdst=(int)-1;    //::daylight �Ƿ������չ��Լʱ�䡣��0Ϊ����������������ʱ�Գ���ʱ���Ӱ�졣
    #endif

    return mktime(&s);
}

/**********************************************************************
*  ��  �� �� // ���ַ���YYYYMMDDHHMMSSʱ�����ʽת��Ϊϵͳ��time_t��(�̰߳�ȫ)
* ��������� // sTime  ��ʱ����ַ���YYYYMMDDHHMMSS��ʽ
* ��������� // ��
* �� �� ֵ�� // time_t��ʽ��ʱ���ʾ
***********************************************************************/
time_t TDateTimeFunc::StringToTimeEx(const char sTime[])
{  
    size_t iLen=strlen(sTime);//ZMP:477242
    if((iLen!=14)&&(iLen!=8)&&(iLen != 0 ))
        throw TException("StringToTime(Time=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime);

    if(iLen == 0)
    {
        return -1;
    }
    
    //ʹ��localtime���Ը���::daylight��ʹ���ϵͳʱ������һ��
    struct tm tCurTm;
    time_t tCurrent = 0;	
    localtime_r(&tCurrent,&tCurTm); //ȡ�õ�ǰʱ���tmֵ

    char sTempYear[5]={0},sTempMon[3]={0},sTempMDay[3]={0},sTempHour[3]={0},sTempMin[3]={0},sTempSec[3]={0}; 
    strncpy(sTempYear,sTime,4);sTempYear[4]=0;
    sTempMon[0]=sTime[4];
    sTempMon[1]=sTime[5];
    sTempMDay[0]=sTime[6]; sTempMon[2]=0;
    sTempMDay[1]=sTime[7]; sTempMDay[2]=0;

    // ֧��8λ
    if(8 == iLen)
    {
        sTempHour[0] = '0';
        sTempHour[1] = '0';
        sTempHour[2]=0;

        sTempMin[0]= '0';
        sTempMin[1]= '0';
        sTempMin[2]=0;

        sTempSec[0]= '0';
        sTempSec[1]= '0';
        sTempSec[2]=0;
    }
    else  // 14λ
    {
        sTempHour[0]=sTime[8];
        sTempHour[1]=sTime[9]; sTempHour[2]=0;

        sTempMin[0]=sTime[10];
        sTempMin[1]=sTime[11]; sTempMin[2]=0;

        sTempSec[0]=sTime[12];
        sTempSec[1]=sTime[13]; sTempSec[2]=0;
    }
    
    struct tm s;
    s.tm_year=atoi(sTempYear)-1900;
    s.tm_mon=atoi(sTempMon)-1;
    s.tm_mday=atoi(sTempMDay);
    s.tm_hour=atoi(sTempHour);
    s.tm_min=atoi(sTempMin);
    s.tm_sec=atoi(sTempSec);
    #ifdef __BORLANDC__
    s.tm_isdst=(int)-1;
    #else
    s.tm_isdst=(int)-1;    //::daylight �Ƿ������չ��Լʱ�䡣��0Ϊ����������������ʱ�Գ���ʱ���Ӱ�졣
    #endif

    return mktime(&s);
}


/**********************************************************************
*  ��  �� �� �õ���ǰʱ���YYYYMMDDHHMMSS�ַ�����ʽ�����߳���ʹ�ã�
* ��������� ��
* ��������� ��
* �� �� ֵ�� ��ǰʱ���YYYYMMDDHHMMSS�ַ�����ʽ
***********************************************************************/
void TDateTimeFunc::GetCurrentTimeStr2(char sTime[],size_t iLen,const bool bLongFlag)
{
    time_t tCurrent;
    struct tm tm_Cur;

    //��鴫������Ƿ���ȷ
    if(NULL==sTime)
    {
        throw TException("GetCurrentTimeStr2() : sTime is Null !\n");
    }

    time(&tCurrent); //ȡ�õ�ǰʱ���time_tֵ
    localtime_r(&tCurrent,&tm_Cur); //ȡ�õ�ǰʱ���tmֵ
    if(bLongFlag)
    {
        if (iLen<20)
        {
            throw TException("GetCurrentTimeStr2() : iLen[%zd] is too small !\n", iLen);
        }
        //ZMP:467372
        snprintf(sTime, iLen, "%04d-%02d-%02d %02d:%02d:%02d",tm_Cur.tm_year+1900,tm_Cur.tm_mon+1,tm_Cur.tm_mday,tm_Cur.tm_hour,tm_Cur.tm_min,tm_Cur.tm_sec);
    }
    else
    {
        if (iLen<15)
        {
            throw TException("GetCurrentTimeStr2() : iLen[%zd] is too small !\n", iLen);
        }
        //ZMP:467372
        timeToStdStr(sTime, iLen,tm_Cur.tm_year+1900,tm_Cur.tm_mon+1,tm_Cur.tm_mday,tm_Cur.tm_hour,tm_Cur.tm_min,tm_Cur.tm_sec);
    }
    sTime[iLen-1]='\0';

    return;
}


/**********************************************************************
*  ��  �� ��  ʱ��(YYYYMMDDHHMMSS)�ַ�����ʱ����ӣ����ؽ���ַ���(�̰߳�ȫ)
* ��������� start_time  ��YYYYMMDDHHMMSS��ʽ���ַ���
* ��������� iSeconds         ��ʱ��
* �� �� ֵ��  ��Ӻ�Ľ���ַ������������NULL��˵������������Ϸ�
* ע��    ��     û��ʱ��������ʱ����
***********************************************************************/
const char * TDateTimeFunc::GetTimeAddSecondsEx(const char sTime[],long iSeconds,char sCurtime[],bool bSysFlag)
{
    if(strlen(sTime)!=14)
        throw TException("GetTimeAddSeconds(Time=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime);
    
    //�����ַ���������
    sCurtime[14]=0;
    if(iSeconds==0)
    {
        strncpy(sCurtime,sTime,sizeof(sCurtime)-1);
        return sCurtime;
    }

    if(bSysFlag) 
    {
        time_t count_second  = StringToTimeEx(sTime);
        count_second = count_second+iSeconds;
        TimeToStringEx(count_second,sCurtime);
        //strncpy(sCurtime,TimeToString(count_second,false),sizeof(sCurtime)-1);
    } 
    else 
    {  
        memset(sCurtime,0,15);
        int dd,hh,mi,ss; 
        int iTmpSs,iTmpMi,iTmpHh,iTmpDd;   
        char sTmp[5]={0}; 

        ss = (int)(ABS(iSeconds)%60);//ZMP:477242
        mi = (int)((ABS(iSeconds)/60)%60);
        hh = (int)((ABS(iSeconds)/3600)%24);
        dd = (int)(ABS(iSeconds)/86400);

        strncpy (sTmp,sTime+12,2);
        sTmp[2]=0;
        iTmpSs = atoi(sTmp);

        strncpy (sTmp,sTime+10,2);
        sTmp[2]=0;
        iTmpMi = atoi(sTmp);

        strncpy (sTmp,sTime+8,2);
        sTmp[2]=0;
        iTmpHh = atoi(sTmp);

        if(iSeconds<0)
        {
            iTmpDd  =  -1;
            iTmpHh +=24-1;
            iTmpMi +=60-1;
            iTmpSs +=60;

            /*�ۼ�����*/
            iTmpSs -= ss;

            /*��->�� ����*/
            iTmpMi += iTmpSs/60;
            iTmpSs %= 60;

            /*�ۼƷ�����*/
            iTmpMi -= mi;

            /*��->Сʱ ����*/
            iTmpHh += iTmpMi/60;
            iTmpMi %=60;

            /*�ۼ�Сʱ*/

            iTmpHh -= hh;

            /*Сʱ->��*/
            iTmpDd += iTmpHh/24;
            iTmpHh %=24;

            iTmpDd  -= dd;

            hh = iTmpHh;
            mi = iTmpMi;
            ss = iTmpSs;
            dd = iTmpDd;
        }
        else
        {
            /*�ۼ�����*/
            ss += iTmpSs;

            /*��->�� ����*/
            mi += ss/60;
            ss %= 60;

            /*�ۼƷ�����*/
            mi += iTmpMi;

            /*��->Сʱ ����*/
            hh += mi/60;
            mi %=60;

            /*�ۼ�Сʱ*/

            hh += iTmpHh;

            /*Сʱ->��*/
            dd += hh/24;
            hh %=24;
        }

        strncpy(sCurtime,sTime,8);
        //ZMP:305593
        char sTempStr[15] = {0};//ZMP:477242
        //memset(sTempStr, 0, sizeof(sTempStr));
        strncpy(sTempStr,sCurtime,sizeof(sTempStr));
        //ZMP:467372
        snprintf(sCurtime, 15, "%s%02d%02d%02d",sTempStr,hh,mi,ss);
        sCurtime[14] = '\0';

        //��������
        AddDay(sCurtime,dd);
    }

    return sCurtime;
}

/**********************************************************************
*  ��  �� �� ��ϵͳ��time_t��ת��Ϊ�ַ���YYYYMMDDHHMMSSʱ�����ʽ(�̰߳�ȫ)
* ��������� tTime  ��time_t��ʽ��ʱ���ʾ
* ��������� �� 
* �� �� ֵ��  ʱ����ַ���YYYYMMDDHHMMSS��ʽ
***********************************************************************/
const char * TDateTimeFunc::TimeToStringEx(const time_t tTime,char sCurtime[],const bool bLongFlag)
{
    struct tm tm_Cur;
    localtime_r(&tTime,&tm_Cur);
    if(bLongFlag)
    {
        timeToStr2(sCurtime,tm_Cur.tm_year+1900,tm_Cur.tm_mon+1,tm_Cur.tm_mday,tm_Cur.tm_hour,tm_Cur.tm_min,tm_Cur.tm_sec); 
    }
    else
    {
        timeToStdStr(sCurtime,15,tm_Cur.tm_year+1900,tm_Cur.tm_mon+1,tm_Cur.tm_mday,tm_Cur.tm_hour,tm_Cur.tm_min,tm_Cur.tm_sec);  
    }

    return sCurtime;
}


/**********************************************************************
*  ��  �� �� ����ʱ���ַ���YYYYMMDDHHMMSS����ĵõ���ʱ����(�̰߳�ȫ)
* ��������� sTime1,sTime2
* ��������� ��
* �� �� ֵ�� sTime1 - sTime2 ������
***********************************************************************/
long TDateTimeFunc::GetDiffTimeEx(const char sTime1[], const char sTime2[])
{
    if(strlen(sTime1)!=14||strlen(sTime2)!=14)
        throw TException("GetDiffTime(Time1=%s,Time2=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sTime1,sTime2);

    return (long)difftime(StringToTimeEx(sTime1),StringToTimeEx(sTime2));
}


bool TDateTimeFunc::GetCurrentTimeStrNew(char sTime[], size_t iLen)
{
    if(sTime == NULL || iLen < DATE_TIME_SIZE)
    {
        return false;
    }

    static time_t tCurrTimeCache=0;
    static char sCurtime[DATE_TIME_SIZE]={0};
    time_t tCurrent;
    time(&tCurrent); //ȡ�õ�ǰʱ���time_tֵ
    
    if(tCurrTimeCache != tCurrent)
    {
        struct tm *tm_Cur = localtime(&tCurrent); //ȡ�õ�ǰʱ���tmֵ�����̰߳�ȫ
        timeToStdStr(sCurtime,sizeof(sCurtime),tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
        tCurrTimeCache = tCurrent;
    }
    	
    memccpy(sTime, sCurtime, 0, DATE_TIME_SIZE-1);
    sTime[DATE_TIME_SIZE-1] = '\0';

    return true;
}

bool TDateTimeFunc::GetCurrentTimeStrNew(char sTime[], size_t iLen, bool bLongFlag)
{
    if (!bLongFlag)
    {
        return GetCurrentTimeStrNew(sTime, iLen);
    }

    if (sTime == NULL || iLen < 20)
    {
        return false;
    }

    static time_t tCurrTimeCache = 0;
    static char sCurtime[32] = { 0 };
    time_t tCurrent;
    time(&tCurrent); //ȡ�õ�ǰʱ���time_tֵ

    if (tCurrTimeCache != tCurrent)
    {
        struct tm *tm_Cur = localtime(&tCurrent); //ȡ�õ�ǰʱ���tmֵ

        timeToStr2(sCurtime, tm_Cur->tm_year + 1900, tm_Cur->tm_mon + 1, tm_Cur->tm_mday,
            tm_Cur->tm_hour, tm_Cur->tm_min, tm_Cur->tm_sec);
        tCurrTimeCache = tCurrent;
    }

    memccpy(sTime, sCurtime, 0, iLen - 1);
    sTime[iLen - 1] = '\0';

    return true;
}

bool TDateTimeFunc::TimeToString(char sTime[], size_t uSize, time_t tTime)
{
    if (sTime == NULL || uSize < DATE_TIME_SIZE)
    {
        return false;
    }

    static time_t tTimeCache = 0;
    static char sCurtime[DATE_TIME_SIZE] = { 0 };

    if (tTimeCache != tTime)
    {
        struct tm *tm_Cur = localtime(&tTime);
        tTimeCache = tTime;
        timeToStdStr(sCurtime, sizeof(sCurtime), tm_Cur->tm_year + 1900, tm_Cur->tm_mon + 1,
            tm_Cur->tm_mday, tm_Cur->tm_hour, tm_Cur->tm_min, tm_Cur->tm_sec);
    }

    memccpy(sTime, sCurtime, 0, DATE_TIME_SIZE - 1);
    sTime[DATE_TIME_SIZE - 1] = '\0';

    return true;
}

bool TDateTimeFunc::TimeToString(char sTime[], size_t uSize, time_t tTime, bool bLongFlag)
{
    if (!bLongFlag)
    {
        return TimeToString(sTime, uSize, tTime);
    }

    if (sTime == NULL || uSize < 20)
    {
        return false;
    }

    static time_t tTimeCache = 0;
    static char sCurtime[32] = { 0 };

    if (tTimeCache != tTime)
    {
        struct tm *tm_Cur = localtime(&tTime);
        tTimeCache = tTime;
        timeToStr2(sCurtime, tm_Cur->tm_year + 1900, tm_Cur->tm_mon + 1, tm_Cur->tm_mday,
            tm_Cur->tm_hour, tm_Cur->tm_min, tm_Cur->tm_sec);
    }

    memccpy(sTime, sCurtime, 0, uSize - 1);
    sTime[uSize - 1] = '\0';

    return true;
}

bool TDateTimeFunc::GMTimeToString(char sTime[], size_t uSize , time_t tTime, bool bLongFlag)
{
    if(sTime == NULL)
    {
        return false;
    }
    if(bLongFlag && uSize < 20)
    {
        return false;
    }
    else if(!bLongFlag && uSize < DATE_TIME_SIZE)
    {
        return false;
    }
    
    struct tm * tm_Cur;
    tm_Cur = gmtime(&tTime);
    
    if(bLongFlag)
    {
        timeToStr2(sTime, tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
    }
    else
    {
        timeToStdStr(sTime, DATE_TIME_SIZE, tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec); 
    }

    return true;
}

void TDateTimeFunc::AddMonthsWithHMS(char sDate[],long iMonths)
{
    if(strlen(sDate)!=14)
        throw TException("AddMonths(Date=%s) : FormatString Is 'YYYYMMDDHHMMSS'.",sDate);

    int month_day[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int d1 = 0, m1 = 0, y1 = 0, h1 = 0, M1 = 0, s1 = 0;
    sscanf( sDate, "%04d%02d%02d%02d%02d%02d", &y1, &m1, &d1 , &h1, &M1, &s1);

    //����ת������
    int iCurMonth = y1*12 + m1;
    iCurMonth += (int)iMonths;

    y1 = iCurMonth/12;
    m1 = iCurMonth%12;
    // m1 = (m1==0)?12:m1;
    if(0 == m1)
    {
        m1 = 12;
        y1 -= 1;
    }

    if ( (y1 %4 == 0) && ( y1%100 != 0 || y1%400 ==0) )
        month_day[1] = 29;

    if(m1==2)
    {
        d1 = d1>month_day[1]?month_day[1]:d1;
    }
    else if(d1==31 && month_day[m1-1]==30)
    {
        d1=30;
    }

//=====================================================
//129457 begin:
#ifdef _IMP_DATE
    timeToStr2(sDate,y1,m1,d1, h1, M1, s1);
#else
    //ZMP:467372
    snprintf( sDate, 15, "%04d%02d%02d%02d%02d%02d", y1, m1, d1 , h1, M1, s1);
    sDate[14] = '\0';
#endif 
//129457 end!
//=====================================================/		
}

time_t TDateTimeFunc::AddMonthsWithHMS(time_t iTime, long iMonths)
{
    char sTime[DATE_TIME_SIZE] = {'\0'};
    TimeToString(sTime, DATE_TIME_SIZE, iTime);
    AddMonthsWithHMS(sTime, iMonths);
    return StringToTime(sTime);
}


#ifdef _IMP_DATE
	bool GetDstFlag()
	{
		return false;
	}
#endif

int TDateTimeFunc::GetMonthDays(int iYear, int iMonth)
{
    int Monthdays[12] =  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if(iMonth==2 && IsLeapYear(iYear))
    {
        Monthdays[1] += 1;
    }

    return Monthdays[iMonth-1];
}

int TDateTimeFunc::GetMonthDays(time_t iTime)
{
    struct tm *tm_Cur = localtime(&iTime);
    return GetMonthDays(tm_Cur->tm_year+1900, tm_Cur->tm_mon+1);
}


