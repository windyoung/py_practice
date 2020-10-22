//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TDateTimeFunc.h
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: 日期处理类
////////////////////////////////////////////////

#ifndef __T_DATETIME_FUNC_H__
#define __T_DATETIME_FUNC_H__

#include <time.h>


class TDateTimeFunc {
public:
    /**********************************************************************
    *  功  能 ： // 将字符串YYYYMMDDHHMMSS时间的形式转换为系统的time_t型
    * 输入参数： // sTime  ：时间的字符串YYYYMMDDHHMMSS形式
    * 输出参数： // 无
    * 返 回 值： // time_t形式的时间表示
    ***********************************************************************/
    static time_t StringToTime(const char sTime[]);

    /**********************************************************************
    *  功  能 ： // 将字符串YYYYMMDDHHMMSS时间的形式转换为系统的time_t型
    * 输入参数： // sTime  ：时间的字符串YYYYMMDDHHMMSS形式
    * 输出参数： // 无
    * 返 回 值： // time_t形式的时间表示(无时区，无夏令时)
    ***********************************************************************/
    static time_t StringToTimeNodst(const char sTime[]);

    /**********************************************************************
    *  功  能 ： // 得到当前时间的YYYYMMDDHHMMSS字符串形式
    * 输入参数： // 无
    * 输出参数： // 无
    * 返 回 值： // 当前时间的YYYYMMDDHHMMSS字符串形式
    ***********************************************************************/
    static const char * GetCurrentTimeStr();
    
    /**********************************************************************
    *  功  能 ： // 得到当前时间的YYYYMMDDHHMMSS字符串形式同时拷贝到入参sTime
    * 输入参数： // sTime
    * 输出参数： // sTime
    * 返 回 值： // 当前时间的YYYYMMDDHHMMSS字符串形式同时拷贝到入参sTime
    ***********************************************************************/
    static const char * GetCurrentTimeStr(char sTime[]);
    
    /**********************************************************************
    *  功  能 ： // 得到当前时间的YYYYMMDDHHMMSS字符串形式同时拷贝长度iLen到入参sTime
    * 输入参数： // sTime,iLen
    * 输出参数： // sTime
    * 返 回 值： // 当前时间的YYYYMMDDHHMMSS字符串形式同时拷贝长度iLen到入参sTime
    ***********************************************************************/
    static const char * GetCurrentTimeStr(char sTime[],size_t iLen);

    /**********************************************************************
    *  功  能 ： // 得到当前时间的YYYYMMDDHHMMSS或YYYY-MM-DD HH:MI:SS字符串形式同时拷贝长度iLen到入参sTime
    * 输入参数： // sTime,iLen,bLongFlag时间格式
    * 输出参数： // sTime
    * 返 回 值： // 当前时间的YYYYMMDDHHMMSS或YYYY-MM-DD HH:MI:SS字符串形式同时拷贝长度iLen到入参sTime
    ***********************************************************************/
    static const char * GetCurrentTimeStr(char sTime[],size_t iLen, bool bLongFlag);

    /**********************************************************************
    *  功  能 ： // 时间(YYYYMMDDHHMMSS)字符串和时长相加，返回结果字符串
    * 输入参数： // start_time  ：YYYYMMDDHHMMSS形式的字符串
    * 输出参数： // iSeconds         ：时长
    * 返 回 值： // 相加后的结果字符串，如果返回NULL则说明输入参数不合法
    * 注意    ： // 没有时区与夏日时问题
    ***********************************************************************/
    static const char * GetTimeAddSeconds(const char sTime[],long iSeconds,bool bSysFlag=false);
    /**********************************************************************
    *  功  能 ： // 时间(YYYYMMDDHHMMSS)字符串和时长相减，返回结果字符串
    * 输入参数： // start_time  ：YYYYMMDDHHMMSS形式的字符串
    * 输出参数： // iSeconds         ：时长
    * 返 回 值： // 相加后的结果字符串，如果返回NULL则说明输入参数不合法
    * 注意    ： // 没有时区与夏日时问题  ADDED BY SUNY 
    ***********************************************************************/
    static void GetTimeSubSeconds(char  sDateTime[],long iSeconds);

    /**********************************************************************
    *  功  能 ： // 睡眠函数（封装了操作系统的差异）
    * 输入参数： // sec  ：程序休息的时长
    * 输出参数： // 无
    * 返 回 值： // 无
    ***********************************************************************/
    static void Sleep(const int iSeconds);
    
    /**********************************************************************
    *  功  能 ： // 睡眠函数（封装了操作系统的差异）
    * 输入参数： // iMicroSeconds  ：程序休息的时长（毫秒）
    * 输出参数： // 无
    * 返 回 值： // 无
    ***********************************************************************/
    static void MSleep(const int iMicroSeconds);

    /**********************************************************************
    *  功  能 ： // 将系统的time_t形按Local时间转换为字符串YYYYMMDDHHMMSS时间的形式
    * 输入参数： // tTime  ：time_t形式的时间表示 bLongFlag 是否长日期格式,iPtrIdx指定使用哪个缓冲区
    * 输出参数： // 无 
    * 返 回 值： // 时间的字符串YYYYMMDDHHMMSS形式
    ***********************************************************************/
    static const char * TimeToString(const time_t tTime);
    static const char * TimeToString(const time_t tTime,const bool bLongFlag);
    static const char * TimeToString(const time_t tTime,const bool bLongFlag,int iPtrIdx);
    
    /**********************************************************************
    *  功  能 ： // 将系统的time_t形按GMT时间转换为字符串YYYYMMDDHHMMSS时间的形式
    * 输入参数： // tTime  ：time_t形式的时间表示 bLongFlag 是否长日期格式
    * 输出参数： // 无 
    * 返 回 值： // 时间的字符串YYYYMMDDHHMMSS形式
    ***********************************************************************/    
    static const char * GMTimeToString(const time_t tTime,const bool bLongFlag=false);

    /**********************************************************************
    *  功  能 ： // 将字符串YYYYMMDDHHMMSS时间的形式转换为GMT时间的time_t型
    * 输入参数： // sTime  ：时间的字符串YYYYMMDDHHMMSS形式
    * 输出参数： // 无
    * 返 回 值： // time_t形式的时间表示
    ***********************************************************************/    
    static time_t StringToGMTime(const char sTime[]);
    
    /**********************************************************************
    *  功  能 ： // 两个时间字符串YYYYMMDDHHMMSS相减的得到的时长秒
    * 输入参数： // sTime1,sTime2
    * 输出参数： // 无
    * 返 回 值： // sTime1 - sTime2 的秒数
    ***********************************************************************/
    static long GetDiffTime(const char sTime1[], const char sTime2[]);

    /**********************************************************************
    *  功  能 ： // 两个时间字符串YYYYMMDDHHMMSS相减的得到的天数 忽略时分秒
    * 输入参数： // sTime1,sTime2
    * 输出参数： // 无
    * 返 回 值： // sTime1 - sTime2 的秒数
    ***********************************************************************/
    static long GetDiffDay(const char sTime1[], const char sTime2[]);
    /**********************************************************************
    *  功  能 ： // 两个时间字符串YYYYMMDDHHMMSS相减的得到的天数 考虑时分秒
    * 输入参数： // sTime1,sTime2
    * 输出参数： // 无
    * 返 回 值： // sTime1 - sTime2 的秒数
    ***********************************************************************/
    static long GetDiffDayWithSeconds(const char sTime1[], const char sTime2[]);

    /**********************************************************************
    *  功  能 ： // 两个时间字符串YYYYMMDDHHMMSS相减的得到的天数,POST（欧洲）PRO租费特用，其他现场如要使用，请规整sTime1,sTime2
    * 输入参数： // sTime1,sTime2
    * 输出参数： // 无
    * 返 回 值： // sTime1 - sTime2 的秒数
    ***********************************************************************/
    static long Daydiff(const char sTime1[], const char sTime2[]);

    /**********************************************************************
    *  功  能 ： // 将字符串YYYYMMDDHHMMSS时间加一天
    * 输入参数： // sDate iDay
    * 输出参数： // sDate
    * 返 回 值： // sDate + iDay 的字符串
    ***********************************************************************/
    static void AddDay(char sDate[],long iDays);

    /**********************************************************************
    *  功  能 ： // 将字符串YYYYMMDDHHMMSS时间加一天
    * 输入参数： // sDate
    * 输出参数： // sDate
    * 返 回 值： // sDate + 1 的字符串
    ***********************************************************************/
    static void AddDate(char sDate[]);

    /**********************************************************************
    *  功  能 ： // 将字符串YYYYMMDDHHMMSS时间减一天
    * 输入参数： // sDate
    * 输出参数： // sDate
    * 返 回 值： // sDate + 1 的字符串
    ***********************************************************************/
    static void SubDate(char sDate[]);

    /**********************************************************************
    *  功  能 ： // 将字符串YYYYMMDDHHMMSS时间的周数
    * 输入参数： // sDate
    * 输出参数： // 
    * 返 回 值： // 周几
    ***********************************************************************/
    static int  GetWeek(const char sDate[]);

    /**********************************************************************
    *  功  能 ： // 返回time_t时间对应的周数
    * 输入参数： // time_t
    * 输出参数： // 
    * 返 回 值： // 周几
    ***********************************************************************/
    static int  GetWeek(const time_t tm);

    /**********************************************************************
    *  功  能 ： // 将字符串YYYYMMDDHHMMSS时间转换成GM时间，返回周数
    * 输入参数： // sDate
    * 输出参数： // 
    * 返 回 值： // 周几
    ***********************************************************************/
    static int  GetGMWeek(const char sDate[]);

    /**********************************************************************
    *  功  能 ： // 返回time_t时间对应的周数
    * 输入参数： // time_t
    * 输出参数： // 
    * 返 回 值： // 周几
    ***********************************************************************/
    static int  GetGMWeek(const time_t tm);

    /**********************************************************************
    *  功  能 ：当前日期为本年度的多少天  //ADDED BY SUNY
    * 输入参数： // 月,天,是否闰年
    * 输出参数： //
    * 返 回 值： // 天数
    ***********************************************************************/
    static int  GetCurrentDays(const int iMonth,const int iDay, bool bIsLeapYear);

    /* 功能：本年度的总天数
     * 入参：年份
     * 返回值：天数
     */
    static int GetTotalDaysOfYear(int iYear);
    /**********************************************************************
    *  功  能 ：当前日期为本年度(平年)的多少天  
    * 输入参数： // 月,天
    * 返 回 值： // 天数
    ***********************************************************************/
    static int  GetCurrentDaysForCommonYear(const int iMonth,const int iDay);
   /**********************************************************************
    *  功  能 ：当前日期为本年度(闰年)的多少天
    * 输入参数： // 月,天
    * 返 回 值： // 天数
    ***********************************************************************/
    static int  GetCurrentDaysForLeapYear(const int iMonth,const int iDay);

    //取代 SetMonthDay 函数
    static void SetMonthAndDay(int iYear, int iDays, int &iMonth, int &iDate);
    /**********************************************************************
    *  功  能 ：算出本年度天数的日期  //ADDED BY SUNY
    * 输入参数： //天数, 月,天,是否闰年
    * 输出参数： // 月,日
    * 返 回 值： //
    ***********************************************************************/
    static void SetMonthDay(int &iMonth, int &iDay, int idays, bool bIsLeapYear);
    
    /**********************************************************************
    *  功  能 ：算出本年度(平年)天数的日期
    * 输入参数： //天数, 月,天
    * 输出参数： // 月,日
    * 返 回 值： //
    ***********************************************************************/
    static void SetMonthDayForCommonYear(int &iMonth,int &iDay,int idays);
    
    /**********************************************************************
    *  功  能 ：算出本年度天数(润年)的日期 
    * 输入参数： //天数, 月,天,
    * 输出参数： // 月,日
    * 返 回 值： //
    ***********************************************************************/
    static void SetMonthDayForLeapYear(int &iMonth,int &iDay,int idays);

    /**********************************************************************
    *  功  能 ： // 获取输入日期的下iMonths个月的日期
    * 输入参数： // sDate 参考日期,iMonths 延后的月数
    * 输出参数： // 
    * 返 回 值： // 周几
    ***********************************************************************/
    static void AddMonths(char sDate[],long iMonths);

    /**********************************************************************
    *  功  能 ： // 获取输入两个日期之间月数差
    * 输入参数： // sTime1时间1 sTime2时间2
    * 输出参数： // 
    * 返 回 值： // sTime1 - sTime2
    ***********************************************************************/
    static long GetDiffMon(const char sTime1[], const char sTime2[]);

    /**********************************************************************
    *  功  能 ： // 获取输入日期的下 iYears 年的日期
    * 输入参数： // sDate 参考日期,iYears 延后的年数
    * 输出参数： // 
    * 返 回 值： // 周几
    ***********************************************************************/
    static void AddYears(char sDate[],long iYears);


    /*判断指定日期，是否在两个日期之间真*/
    static bool BetweenDate(const char *dInDate,const char *dBeginDate,const char *dEndDate);

    /**********************************************************************
    *  功         能 ：  判断指定日期，是否在两个日期之间真
    * 输入参数： dInDate : 指定time_t 时间值;
                                   dBeginDate : 比较的开始时间time_t 时间值;
                                   dEndDate : 比较的结束时间time_t 时间值
    * 输出参数： 无
    * 返 回    值： bool : true 在开始时间和结束时间之间
                                            false不在开始时间和结束时间之间
    ***********************************************************************/
    static bool BetweenDate(time_t dInDate,time_t  dBeginDate,time_t dEndDate);

    /*公用函数,获取当前日期时间*/
    static void GetNowDateTime(char *sDateTime,int iDateTimeFlag);

    //比较YYYYMMDD部分，忽略MMhhSS部分，当iTime1小于iTime2则返回-1，等于则返回0，大于则返回1
    static int CompareDate(time_t iTime1, time_t iTime2);

    //去掉iTime中的HHmmSS部分后，返回对应的time_t值，也就是获取YYYYMMDD部分的time_t值
    static time_t GetDate(time_t iTime);

    /**********************************************************************
    *  功         能 ：  获取输入时间的下iMonths个月的时间值
    * 输入参数： iTime : time_t 时间值;
                                   iMonths : 月数
    * 输出参数： iTime
    * 返 回    值： 输入时间的下iMonths个月的时间值
    ***********************************************************************/
    static time_t AddMonths(time_t iTime, long iMonths);

    /**********************************************************************
    *  功         能 ：  获取time_t时间值的下 iYears 年的时间值
    * 输入参数： iTime : time_t 时间值;
                                   iYears : 年数
    * 输出参数： iTime
    * 返 回    值： 下 iYears 年的时间值
    ***********************************************************************/
    static time_t AddYears(time_t iTime,long iYears);

        /**********************************************************************
    *  功         能 ： 将时间加iDays天
    * 输入参数： iTime: 时间,
                                   iDays: 天数
    * 输出参数： 无
    * 返 回    值： iTime + iDay天
    ***********************************************************************/
    static time_t AddDays(time_t iTime,long iDays);

    /**********************************************************************
    *  功         能 ： 两个time_t 值时间相差的时差
    * 输入参数： iTime1, iTime2
    * 输出参数： 无
    * 返 回    值：  iTime1 - iTime2 的时差
    ***********************************************************************/
    static long AdjustTimeZone(time_t iTime1, time_t iTime2);

    /**********************************************************************
    *  功         能 ： 两个time_t 值时间相减的得到的天数
    * 输入参数： iTime1, iTime2
    * 输出参数： 无
    * 返 回    值：  iTime1 - iTime2 的天数
    ***********************************************************************/
    static long GetDiffDay(time_t iTime1, time_t iTime2);

    /**********************************************************************
    *  功         能 ：  获取两个time_t时间之间月数差
    * 输入参数：  iTime1 : time_t 时间1
                                    iTime2 : time_t 时间2
    * 输出参数：  无
    * 返 回    值：  iTime1 - iTime2 月数差
    ***********************************************************************/
    static long GetDiffMonth(time_t iTime1, time_t iTime2);

    /**********************************************************************
    *  功         能 ：  获取两个time_t时间之间月数差 不带日期
    * 输入参数：  iTime1 : time_t 时间1
                                    iTime2 : time_t 时间2
    * 输出参数：  无
    * 返 回    值：  iTime1 - iTime2 月数差
    ***********************************************************************/
    static long GetDiffMonthWithOutDays(time_t iTime1, time_t iTime2);
public:
    /**********************************************************************
    *  功  能 ： 将字符串拆分成年月日时分秒等数字，记录在record里面
    * 输入参数： // sTime 表示日期的字符串, 
    * 输出参数： // record : 记录 年 月 日 时 分 秒
    * 返 回 值： // 无
    ***********************************************************************/
    static void SplitString(const char sTime[],int record[]);

    /**********************************************************************
    *  功  能 ： 判断年份是否为闰年
    * 输入参数： // int year
    * 输出参数： // 
    * 返 回 值： // bool
    ***********************************************************************/
    static bool IsLeapYear(int iYear);
    
    /**********************************************************************
    *  功  能 ： 检查年份字符串是否合法
    * 输入参数： // sTime:时间字符串
    * 输出参数： // 
    * 返 回 值： // bool
    ***********************************************************************/
    static void CheckYearString(const char sTime[]);
   
public:
    /* ***********************************************************************************
    *    **********************************************************************************
    *    *************************线程安全的时间函数*****************************
    * *********************************************************************************  */
    
    
    /**********************************************************************
    *  功  能 ： // 将字符串YYYYMMDDHHMMSS时间的形式转换为系统的time_t型(线程安全)
    * 输入参数： // sTime  ：时间的字符串YYYYMMDDHHMMSS形式
    * 输出参数： // 无
    * 返 回 值： // time_t形式的时间表示
    ***********************************************************************/
    static time_t StringToTimeEx(const char sTime[]);
    
    /**********************************************************************
    *  功  能 ： // 得到当前时间的YYYYMMDDHHMMSS字符串形式（多线程下使用）
    * 输入参数： // 无
    * 输出参数： // 无
    * 返 回 值： // 当前时间的YYYYMMDDHHMMSS字符串形式
    ***********************************************************************/
    static void GetCurrentTimeStr2(char sTime[],size_t iLen,const bool bLongFlag=false);
    
    /**********************************************************************
    *  功  能 ：  时间(YYYYMMDDHHMMSS)字符串和时长相加，返回结果字符串(线程安全)
    * 输入参数： start_time  ：YYYYMMDDHHMMSS形式的字符串
    * 输出参数： iSeconds         ：时长
    * 返 回 值：  相加后的结果字符串，如果返回NULL则说明输入参数不合法
    * 注意    ：     没有时区与夏日时问题
    ***********************************************************************/
    static const char * GetTimeAddSecondsEx(const char sTime[],long iSeconds,char sOutTime[],bool bSysFlag=false);

    /**********************************************************************
    *  功  能 ： 将系统的time_t形按Local时间转换为字符串YYYYMMDDHHMMSS时间的形式(线程安全)
    * 输入参数：  tTime  ：time_t形式的时间表示 bLongFlag 是否长日期格式
    * 输出参数：  无 
    * 返 回 值：  时间的字符串YYYYMMDDHHMMSS形式
    ***********************************************************************/
    static const char * TimeToStringEx(const time_t tTime,char sCurtime[],const bool bLongFlag=false);

    /**********************************************************************
    *  功  能 ： 两个时间字符串YYYYMMDDHHMMSS相减的得到的时长秒(线程安全)
    * 输入参数：  sTime1,sTime2
    * 输出参数：  无
    * 返 回 值： sTime1 - sTime2 的秒数
    ***********************************************************************/
    static long GetDiffTimeEx(const char sTime1[], const char sTime2[]);

    /**********************************************************************
    *  功  能 ： 将字符串YYYYMMDDHHMMSS时间加一天(线程安全)
    * 输入参数： sDate iDay
    * 输出参数： sDate
    * 返 回 值    ： sDate + iDay 的字符串
    ***********************************************************************/
    static time_t AddDay(time_t iDate,long iDays);

    /**********************************************************************
    *  功         能 ： 得到时差
    * 输入参数： 无
    * 输出参数： 无
    * 返 回    值： 无
    ***********************************************************************/    
    static void getTimeDifference();
  
    /**********************************************************************
    *  功         能 ： 得到IS_DST_USED配置项
    * 输入参数： 无
    * 输出参数： 无
    * 返 回    值： 无
    ***********************************************************************/    
    static void ReadConfig();

    /**********************************************************************
    *功    能：将参数传入的时间转成标准的YYYYMMDDHH24MISS格式时间，性能是sprintf十倍并优于strftime
    *输入参数:整形年月日时分秒，sCurtime为要返回的字符数组，uSize是sizeof(sCurtime)
    *输出参数:YYYYMMDDHH24MISS格式时间保存到sCurtime
    *返 回 值:无
    ***********************************************************************/ 
    static void timeToStdStr(char sCurtime[],size_t uSize,int year,int mon,int day, int hour, int min, int sec);

    /**********************************************************************
    *  功         能 ：  获取输入时间的下iMonths个月的时间值，使用输入时间的时分秒
    * 输入参数： iTime : time_t 时间值;
                                   iMonths : 月数
    * 输出参数： iTime(带输入时间的时分秒)
    * 返 回    值： 输入时间的下iMonths个月的时间值
    ***********************************************************************/
    static time_t AddMonthsWithHMS(time_t iTime, long iMonths);
    
    /**********************************************************************
    *  功  能 ： // 获取输入日期的下iMonths个月的日期,使用输入时间的时分秒
    * 输入参数： // sDate 参考日期,iMonths 延后的月数
    * 输出参数： // 
    * 返 回 值： // 周几
    ***********************************************************************/
    static void AddMonthsWithHMS(char sDate[],long iMonths);

public:
    /**********************************************************************
    *  功  能 ： // 得到当前时间的YYYYMMDDHHMMSS字符串形式同时拷贝到入参sTime
    * 输入参数： // iLen 指定输出参数sTime的大小，短日期格式时必须大等于15，长日期格式必须大于等于20，如不满足则返回false
    *            // bLongFlag是否是长日期格式
    * 输出参数： // sTime
    * 返 回 值： // 当前时间的YYYYMMDDHHMMSS字符串形式同时拷贝到入参sTime
    ***********************************************************************/
    static bool GetCurrentTimeStrNew(char sTime[], size_t iLen);
    static bool GetCurrentTimeStrNew(char sTime[], size_t iLen, bool bLongFlag);
    
    /**********************************************************************
    *  功  能 ： // 将系统的time_t形按Local时间转换为字符串YYYYMMDDHHMMSS时间的形式
    * 输入参数： // tTime  ：time_t形式的时间表示 bLongFlag 是否长日期格式
    *            // uSize：指定输出参数sTime的大小，短日期格式时必须大等于15，长日期格式必须大于等于20，如不满足则返回false
    * 输出参数： // sTime:      时间的字符串YYYYMMDDHHMMSS形式
    * 返 回 值： // bool True:成功  false:失败
    ***********************************************************************/
    static bool TimeToString(char sTime[], size_t uSize, time_t tTime);
    static bool TimeToString(char sTime[], size_t uSize, time_t tTime, bool bLongFlag);

    /**********************************************************************
    *  功  能 ： // 将系统的time_t形按GMT时间转换为字符串YYYYMMDDHHMMSS时间的形式
    * 输入参数： // tTime  ：time_t形式的时间表示 bLongFlag 是否长日期格式
    * 输出参数： // sTime:      时间的字符串YYYYMMDDHHMMSS形式
    * 返 回 值： // True:成功  false:失败
    ***********************************************************************/    
    static bool GMTimeToString(char sTime[], size_t uSize, time_t tTime, bool bLongFlag);

    static int GetMonthDays(int iYear, int iMonth);

    static int GetMonthDays(time_t iTime);

private:
    //-----------------------------------------------------
    //功    能：用于替代sprintf(sCurtime,"%04d-%02d-%02d %02d:%02d:%02d",...)
    //输入参数:
    //输出参数:
    //返 回 值:
    //-----------------------------------------------------
    static void timeToStr2(char *sCurtime,int year,int mon,int day, int hour, int min, int sec);

    //-----------------------------------------------------
    //功    能：用于替代sprintf(sCurtime,"%s%02d%02d%02d",...)
    //输入参数:
    //输出参数:
    //返 回 值:
    //-----------------------------------------------------
    static void timeToStr3(char *sCurtime,const char* str, int hour, int min, int sec);

    //-----------------------------------------------------
    //功    能：用于替代sprintf( sDate, "%04d%02d%02d000000",...)
    //输入参数:
    //输出参数:
    //返 回 值:
    //-----------------------------------------------------
    static void timeToStr4(char *sCurtime,int year,int mon,int day);

    //-----------------------------------------------------
    //功    能：用于替代sprintf(sDateTime,"%04d%02d%02d",...)
    //输入参数: 年月日
    //输出参数:
    //返 回 值:
    //-----------------------------------------------------
    static void timeToStr5(char *sCurtime,int year,int mon,int day); 

    //-----------------------------------------------------
    //功    能：用于替代sprintf(sDateTime,"%02d%02d%02d",...)
    //输入参数: 时分秒
    //输出参数:
    //返 回 值:
    //-----------------------------------------------------
    static void timeToStr6(char *sCurtime,int hour,int min,int sec);   

private:
    static int agiMonthC[12];
    
    static long TIME_DIFFERENCE;
    static bool bFirst; 
    static bool m_bReadconfigFlag;
    static bool m_bIsDstUsed;
//#endif
//129457 end!
//===================================================== 
public:	
    static long GetDiffTime_p(const char sTime1[], const char sTime2[]);
    static time_t StringToTime_p(const char sTime[]);	
	
};

#define FUNC_DT TDateTimeFunc

#endif //__T_DATETIME_FUNC_H__
