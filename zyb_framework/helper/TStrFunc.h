//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
//TStrFunc.h
/*********************************************************************
* 版权所有 (C)2004, 中兴软创（ZTESOFT）。
* 
* 文件名称： // TStrFunc.h
* 文件标识： // 
* 内容摘要： // 提供对日期时间的各种操作函数
* 其它说明： // 
* 当前版本： // V1.0
* 作    者： // FU.WENJUN
* 完成日期： // 20041202
**********************************************************************/
#ifndef __T_STRING_FUNC_H__
#define __T_STRING_FUNC_H__

#include <string.h>
#include <string>
#include <cstring>
#include "CommonDef.h"
using namespace std;

//size_t转换到int，可能丢失数据
#ifdef __OS_WINDOWS__
#pragma warning(disable:4267)
#endif

extern unsigned char sLower[256];

#define MAX_32BIT_NUMBER   2147483647
#define MIN_32BIT_NUMBER  -2147483648
#define MAX_64BIT_NUMBER   9223372036854775807
#define MIN_64BIT_NUMBER  -9223372036854775808

class TStrFunc
{
public:
    /**********************************************************************
    *  功  能 ： // 判断源字符串是否与规则相匹配
    * 输入参数： // srcStr      ：源字符串
    *            // sNameRule   ：规则字符串，可以用通配符'*'和'?'，'*'表示多字通配，'?'表示单字通配
    *            // bMatchCase  ：是否区分大小写，默认为区分大小写
    * 输出参数： // 无
    * 返 回 值： // true表示匹配，false表示不匹配
    ***********************************************************************/
    static bool     MatchString(const char * srcStr, const char * sNameRule, bool bMatchCase = true);

    /**********************************************************************
    *  功  能 ： // 在pchSrc中查找字符串lpszFind，lpszFind中可以包含通配字符‘?’
    * 输入参数： // pchSrc      ：源字符串
    *            // lpszFind    ：需要查找的字符串，可以包含通配符'?'，'?'表示单字匹配
    *            // nStart      ：查找的起始位置，默认为从0开始查找
    * 输出参数： // 无
    * 返 回 值： // 找到则返回匹配位置，否则返回-1
    ***********************************************************************/
    static int      FindString(const char* pchSrc, const char* lpszFind, int nStart = 0);

public:
    /**********************************************************************
    *  功  能 ： // 删除字符串两边的填充字符
    * 输入参数： // src      ：源字符串
    *            // filechar ：填充的字符
    * 输出参数： // src      ：被删除填充字符后的字符串
    * 返 回 值： // 无
    ***********************************************************************/
    static void     Trim(char *src,char fillchar = ' ');
    /**********************************************************************
    *  功  能 ： // 删除字符串左边的填充字符
    * 输入参数： // src      ：源字符串
    *            // filechar ：填充的字符
    * 输出参数： // src      ：被删除填充字符后的字符串
    * 返 回 值： // 无
    ***********************************************************************/
    static void     LeftTrim(char *src,char fillchar = ' ');
    /**********************************************************************
    *  功  能 ： // 删除字符串右边的填充字符
    * 输入参数： // src      ：源字符串
    *            // filechar ：填充的字符
    * 输出参数： // src      ：被删除填充字符后的字符串
    * 返 回 值： // 无
    ***********************************************************************/
    static void     RightTrim(char *src,char fillchar = ' ');

    /**********************************************************************
    *  功  能 ： // 删除字符串两边的填充字符
    * 输入参数： // src      ：源字符串
    *            // filechar ：填充的字符数组(多个字符需要删除)
    *            // number   ：填充的字符数组的长度
    * 输出参数： // src      ：被删除填充字符后的字符串
    * 返 回 值： // 无
    ***********************************************************************/
    static void     Trim(char *src,char* fillchar,int number);
    /**********************************************************************
    *  功  能 ： // 连续删除字符串左边的填充字符，如果遇到非填充字符即停止
    * 输入参数： // src      ：源字符串
    *            // filechar ：填充的字符数组(多个字符需要删除)
    *            // number   ：填充的字符数组的长度
    * 输出参数： // src      ：被删除填充字符后的字符串
    * 返 回 值： // 无
    ***********************************************************************/
    static void     LeftTrim(char *src,char* fillchar,int number);
    /**********************************************************************
    *  功  能 ： // 连续删除字符串右边的填充字符，如果遇到非填充字符即停止
    * 输入参数： // src      ：源字符串
    *            // filechar ：填充的字符数组(多个字符需要删除)
    *            // number   ：填充的字符数组的长度
    * 输出参数： // src      ：被删除填充字符后的字符串
    * 返 回 值： // 无
    ***********************************************************************/
    static void     RightTrim(char *src,char* fillchar,int number);

public:
    /**********************************************************************
    *  功  能 ： // 忽略大小写比较字符串大小
    * 输入参数： // sFirst      ：源字符串
    *            // sSecond     ：目标字符串
    * 输出参数： // 无
    * 返 回 值： // =0  表示相等，
    *            // <0 表示源字符串小于目标字符串
    *            // >0 表示源字符串大于目标字符串
    ***********************************************************************/
    static int      StrNoCaseCmp(const char *sFirst,const char *sSecond);

    /**********************************************************************
    *  功  能 ： // 将字符串中的环境变量替换，环境变量的形式为 $(环境变量名)
    * 输入参数： // src      ：源字符串
    * 输出参数： // 无
    * 返 回 值： // 已经被替换了环境变量的string
    ***********************************************************************/
    static string   ReplaceEnv(const char * str);

    /**********************************************************************
    *  功  能 ： // 将一个字符串中的某个子串替换
    * 输入参数： // sOrgSrc     ：源字符串
    *            // sSubStr     ：源字符串中需要替换的子串
    *            // sReplaceStr ：替换成的字符串
    * 输出参数： // 无
    * 返 回 值： // 替换后的string
    ***********************************************************************/
    static string   ReplaceSubStr(const char * sOrgSrc, const char * sSubStr, const char * sReplaceStr);

    /**********************************************************************
    *  功  能 ： // 将一个字符串中的某个子串替换
    * 输入参数： // sOrgSrc     ：源字符串
    *            // sSubStr     ：源字符串中需要替换的子串
    *            // sReplaceStr ：替换成的字符串
    *            // sOutStr     ：返回结果
    *            // bSensitive  ：是否区分大小写
    * 输出参数： // 无
    * 返 回 值： // 替换后的string
    ***********************************************************************/
    static char * ReplaceSubStr(char * sOrgStr, const char * sSubStr, const char * sReplaceStr, char *sOutStr, bool bSensitive = true);

public:
    /**********************************************************************
    *  功  能 ： // 判断该字符串是否是 YYYYMMDDHHMMSS格式的日期时间型字符串
    * 输入参数： // dt      ：源字符串
    * 输出参数： // 无
    * 返 回 值： // true    ：该字符串是YYYYMMDDHHMMSS格式的字符串
    *            // false   ：该字符串不是YYYYMMDDHHMMSS格式的字符串
    ***********************************************************************/
    static bool     IsDateTimeStr(const char * dt);
    /**********************************************************************
    *  功  能 ： // 判断该字符串是否是 HHMMSS格式的日期时间型字符串
    * 输入参数： // sTime   ：源字符串
    * 输出参数： // 无
    * 返 回 值： // true    ：该字符串是HHMMSS格式的字符串
    *            // false   ：该字符串不是HHMMSS格式的字符串
    ***********************************************************************/
    static bool     IsTimeStr(const char * sTime);
    /**********************************************************************
    *  功  能 ： // 判断该字符串是否是 YYYYMMDD格式的日期时间型字符串
    * 输入参数： // sDate   ：源字符串
    * 输出参数： // 无
    * 返 回 值： // true    ：该字符串是HHMMSS格式的字符串
    *            // false   ：该字符串不是HHMMSS格式的字符串
    ***********************************************************************/
    static bool     IsDateStr(const char * sDate);
    /**********************************************************************
    *  功  能 ： // 判断该字符串是否全是数字
    * 输入参数： // inStr   ：源字符串
    * 输出参数： // 无
    * 返 回 值： // true    ：该字符串全是数字
    *            // false   ：该字符串不是数字
    ***********************************************************************/
    static bool     IsNumStr(const char * inStr);

    /**********************************************************************
    函    数: static bool atoiEx(const char * pBuf,int size,int &result);   
    功    能: 按指定长度转换一个字符串到整型
    输入参数: pBuf - 要转换的字符串
              size - 字符串长度
              result -  输出转换的结果
    全局变量: 无
    返回结果:true - 成功 false - 失败
    ************************************************************************/
    static bool atoiEx(const char * pBuf,int size,int &result);

    /**********************************************************************
    函    数: static void strncpyEx(char * desc,char * source,int size,char endtag=' ');
    功    能: 和strncpy功能相同，但是遇到endtag字符时会置'\0',并返回
    输入参数: desc -   字符串
              source - 字符串
              size   - 字符串长度
              endtag - 字符串结束符
    全局变量: 无
    返回结果:true - 成功 false - 失败
    ************************************************************************/
    static void strncpyEx(char * desc,char * source,int size,char endtag=' ');

    /**********************************************************************
    *  功  能 ： // 从源串（第一个参数）中过滤字符串（第二个参数）中的每个字符
    * 输入参数： // strSrc,strSeparate
    * 输出参数： // 无
    * 返 回 值： // 过滤掉分割字符的字符串
    ***********************************************************************/
    static char * FilterSeparate(const char *strSrc,const char *strSeparate);

    //////////////////////////////////////////////////////////////////////////
    static char * FormatChar(char *sLine);

    static int StrToInt(const char *sSrc);

    static long long StrToLLong(const char *sSrc,bool bSignFlag = false);

    static llong StrTollong(const char *sSrc);
    static int StrTollong(const char *sSrc, llong &iResult);

    static bool IsEmpty(const char *sSrc);

    /**********************************************************************
    *  功  能 ：十六进制字符串转成十进制整数
    * 输入参数：strSrc
    * 输出参数：无
    * 返 回 值：转换好的整数
    ***********************************************************************/
    static llong StrTollongH(const char * sSrc);//

    /*公用函数,剔除字符串首尾空格,TAB键*/
    static void AllTrim(char *sBuffer);
    
    /*将整型转换成字符串输出*/
    static const char* IntToChar(int iIn);
    
    /*公用函数,将长整型转换成字符串输出*/
    static const char* NumToChar(const long long iIn);

    static void IntToChar(char sOut[], size_t iOutSize, int iIn);
    static void NumToChar(char sOut[], size_t iOutSize, long long iIn);

    /*公用函数,忽略字符大小写比较字符串大小*/
    static int StrNocaseCmp(const char *sFirst,const char *sSecond);
    
    /*使用数组映射的不分大小写字符串比较,返回值同 StrNocaseCmp */
    static int StrCmpNoCase(const char *sFirst,const char *sSecond);    

public:
    //将字符串进行Hash，转换成一个比较散列的值
    static long StrToHash(const char* pszSrc);
           
  //判断字符串是否为空
  static bool IsNullStr(const char* src);
  
  /**********************************************************************
  *  功  能 ： // 判断字符是否全部由数字组成，并对正负号进行处理
  * 输入参数： // inSrc数字字符串
  * 输出参数： // outSrc去掉加减号的数字字符串,  outStr的长度
  * 返 回 值： // 返回0说明不合法，返回1说明为正数，返回-1说明为负数
  ***********************************************************************/
  static int IsNumString(const char * inStr, char* outStr,int &length);

  /**********************************************************************
  *  功  能 ： // 判断int字符是否越界(注：保证位数为10位的情况下判断)
  * 输入参数： // intStr表示数字， flag表示符号(正为1，负为-1)
  * 输出参数： // 
  * 返 回 值： // 越界bool=true, 否则bool=false
  ***********************************************************************/  
  static bool IsOutofInt(const char * intStr, int flag);
  
  /**********************************************************************
  *  功  能 ： // 判断long字符是否越界(注：保证位数为19位的情况下判断)
  * 输入参数： // inStr表示数字， flag表示符号(正为1，负为-1)
  * 输出参数： // 
  * 返 回 值： // 越界bool=true, 否则bool=false
  ***********************************************************************/  
  static bool IsOutofLLong(const char * inStr, int flag); 
  /**********************************************************************
  *  功  能 ： //合法 IPV4 地址的检测
  * 输入参数： // inStr表示IPV4 地址(以点号分隔，共4段，每段值在[0,255]之间即合法)
  * 输出参数： // 
  * 返 回 值： // 合法返回true, 否则返回false
  ***********************************************************************/  
  static bool IsIPV4(const char *intStr);

  /**********************************************************************
  *  功  能 ： //字符串拼接，字符串尾部保留分割符
  * 输入参数： // s-源字符串，iSize-指定最大安全长度，fmt-格式
  * 输出参数： // 
  * 返 回 值： // 合法返回true, 否则返回false
  ***********************************************************************/  
  static int StrCatFMT(char *s, const int iSize, const char *fmt, ...);

  /**********************************************************************
  *  功  能 ： //字符串拼接，字符串尾部去除分割符
  * 输入参数： // s-源字符串，iSize-指定最大安全长度，fmt-格式
  * 输出参数： //
  * 返 回 值： // 合法返回true, 否则返回false
  ***********************************************************************/
  static void  _StrCatFMT(char *s, const int iSize, const char *fmt, ...);

private:
    //ZMP:315428 ,圈复杂度代码重构，MatchString拆分函数，匹配检查
    static bool MatchCheck(char *szMatcher,char *szSource);  

};

#define FUNC_STR TStrFunc

#endif //__T_STRING_FUNC_H__
