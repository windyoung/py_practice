//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
//TStrFunc.cpp
#include "TStrFunc.h"
#include "debug_new.h"
#include "Common.h"
#include "TException.h"
#include "TSplit.h"
#include <stdarg.h>


//ZMP:467374    
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

unsigned char sLower[256] =     {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                                21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                                31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                                41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
                                51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
                                61, 62, 63, 64, 97, 98, 99, 100,101,102,
                                103,104,105,106,107,108,109,110,111,112,
                                113,114,115,116,117,118,119,120,121,122,
                                91, 92, 93, 94, 95, 96, 97, 98, 99, 100,
                                101,102,103,104,105,106,107,108,109,110,
                                111,112,113,114,115,116,117,118,119,120,
                                121,122,123,124,125,126,127,128,129,130,
                                131,132,133,134,135,136,137,138,139,140,
                                141,142,143,144,145,146,147,148,149,150,
                                151,152,153,154,155,156,157,158,159,160,
                                161,162,163,164,165,166,167,168,169,170,
                                171,172,173,174,175,176,177,178,179,180,
                                181,182,183,184,185,186,187,188,189,190,
                                191,192,193,194,195,196,197,198,199,200,
                                201,202,203,204,205,206,207,208,209,210,
                                211,212,213,214,215,216,217,218,219,220,
                                221,222,223,224,225,226,227,228,229,230,
                                231,232,233,234,235,236,237,238,239,240,
                                241,242,243,244,245,246,247,248,249,250,
                                251,252,253,254,255};
//315428 begin
bool TStrFunc::MatchString(const char * srcStr, const char * sNameRule, bool bMatchCase /* = true */)
{
    const char * pchSrc = srcStr;
    const char * lpszMatch = sNameRule;
    if(pchSrc == NULL || lpszMatch == NULL)
        return false;

    if(lpszMatch[0] == 0)//Is a empty string
    {
        if(pchSrc[0] == 0)
            return true;
        else
            return false;
    }

    int i = 0, j = 0;

    //���ɱȽ�����ʱԴ�ַ���'szSource'
    char* szSource =
        new char[ (j = (int)(strlen(pchSrc)+1)) ];

    if( bMatchCase )
    {    
        //ZMP:467374
        strncpy(szSource, pchSrc, (size_t)(j-1));
        szSource[j-1] = '\0';
    }
    else
    {    //Lowercase 'pchSrc' to 'szSource'
        i = 0;
        while(pchSrc[i])
        {    if(pchSrc[i] >= 'A' && pchSrc[i] <= 'Z')
        szSource[i] = (char)(pchSrc[i] - 'A' + 'a');
        else
            szSource[i] = pchSrc[i];

        i++;
        }
        szSource[i] = 0;
    }

    //���ɱȽ�����ʱƥ���ַ���'szMatcher'
    char* szMatcher = new char[strlen(lpszMatch)+1];

    //��lpszMatch���������ġ�*������һ����*�����Ƶ�szMatcher��
    i = j = 0;
    while(lpszMatch[i])
    {
        szMatcher[j++] = (char)((!bMatchCase) ?
            ( (lpszMatch[i] >= 'A' && lpszMatch[i] <= 'Z') ?//Lowercase lpszMatch[i] to szMatcher[j]
            lpszMatch[i] - 'A' + 'a' :lpszMatch[i]) :lpszMatch[i]);//Copy lpszMatch[i] to szMatcher[j]
        //Merge '*'
        if(lpszMatch[i] == '*')
            while(lpszMatch[++i] == '*');
        else
            i++;
    }
    szMatcher[j] = 0;

    //��ʼ����ƥ����
    bool bIsMatched = MatchCheck(szMatcher,szSource);

    DELETE_A(szSource);
    DELETE_A(szMatcher);
    
    return bIsMatched;
}

//ƥ����,ZMP:315428 ,Ȧ���Ӷȴ����ع�
bool TStrFunc::MatchCheck(char *szMatcher,char *szSource)
{
    
    int nMatchOffset = 0;
    int nSourOffset  = 0;
    bool bIsMatched = true;
    bool bIsTrue = false;
    
    while(szMatcher[nMatchOffset])
    {
        if(szMatcher[nMatchOffset] == '*')
        {
            if(szMatcher[nMatchOffset+1] == 0)
            {    //szMatcher[nMatchOffset]�����һ���ַ�

                bIsMatched = true;
                break;
            }
            else
            {    //szMatcher[nMatchOffset+1]ֻ����'?'����ͨ�ַ�

                int nSubOffset = nMatchOffset+1;

                while(szMatcher[nSubOffset])
                {    
                    if(szMatcher[nSubOffset] == '*')
                        break;
                    nSubOffset++;
                }

                if( strlen(szSource+nSourOffset) <size_t(nSubOffset-nMatchOffset-1) )
                {    //Դ�ַ���ʣ�µĳ���С��ƥ�䴮ʣ��Ҫ�󳤶�
                    bIsMatched = false; //�ж���ƥ��
                    break;            //�˳�
                }

                if(!szMatcher[nSubOffset])//nSubOffset is point to ender of 'szMatcher'
                {    //���ʣ�²����ַ��Ƿ�һһƥ��

                    nSubOffset--;
                    int nTempSourOffset = (int)strlen(szSource)-1;
                    //�Ӻ���ǰ����ƥ��
                    while(szMatcher[nSubOffset] != '*')
                    {
/*                         if(szMatcher[nSubOffset] == '?')
                            ;
                        else */
                        if(szMatcher[nSubOffset] != '?') //314406 
                        {    
                            if(szMatcher[nSubOffset] != szSource[nTempSourOffset])
                            {    
                                bIsMatched = false;
                                break;
                            }
                        }
                        nSubOffset--;
                        nTempSourOffset--;
                    }
                    break;
                }
                else//szMatcher[nSubOffset] == '*'
                {    
                    nSubOffset -= nMatchOffset;
                    char* szTempFinder = new char[nSubOffset];
                    nSubOffset--;
                    memcpy(szTempFinder, szMatcher+nMatchOffset+1, (size_t)nSubOffset);
                    szTempFinder[nSubOffset] = 0;

                    int nPos = FindString(szSource+nSourOffset, szTempFinder, 0);
                    DELETE_A(szTempFinder);

                    if(nPos != -1)//��'szSource+nSourOffset'���ҵ�szTempFinder
                    {    
                        nMatchOffset += nSubOffset;
                        nSourOffset += (nPos+nSubOffset-1);
                    }
                    else
                    {    
                        bIsMatched = false;
                        break;
                    }
                }
            }
        }        //end of "if(szMatcher[nMatchOffset] == '*')"
        else if(szMatcher[nMatchOffset] == '?')
        {
            if(!szSource[nSourOffset])
            {    
                bIsMatched = false;
                break;
            }
            bIsTrue = (!szMatcher[nMatchOffset+1] && szSource[nSourOffset+1]);
            if(bIsTrue)
            {    //���szMatcher[nMatchOffset]�����һ���ַ���
                //��szSource[nSourOffset]�������һ���ַ�
                bIsMatched = false;
                break;
            }
            nMatchOffset++;
            nSourOffset++;
        }
        else//szMatcher[nMatchOffset]Ϊ�����ַ�
        {
            bIsTrue = (szSource[nSourOffset] != szMatcher[nMatchOffset]) 
                      ||(!szMatcher[nMatchOffset+1] && szSource[nSourOffset+1]);
            if(bIsTrue)
            {
                bIsMatched = false;
                break;
            }
            nMatchOffset++;
            nSourOffset++;
        }
    }
    return bIsMatched;
}
// 315428  end

int  TStrFunc::FindString(const char* pchSrc, const char* lpszFind, int nStart /* = 0 */)
{
    //    ASSERT(pchSrc && lpszFind && nStart >= 0);
    if(pchSrc == NULL || lpszFind == NULL || nStart < 0)
        return -1;

    int m = (int)strlen(pchSrc);
    int n = (int)strlen(lpszFind);

    if( nStart+n > m )
        return -1;

    if(n == 0)
        return nStart;

    //KMP�㷨
    int* next = new int[n];
    //�õ������ַ�����next����
    {    n--;

    int j, k;
    j = 0;
    k = -1;
    next[0] = -1;

    while(j < n)
    {    if(k == -1 || lpszFind[k] == '?' || lpszFind[j] == lpszFind[k])
    {    j++;
    k++;
    next[j] = k;
    }
    else
        k = next[k];
    }

    n++;
    }

    int i = nStart, j = 0;
    while(i < m && j < n)
    {
        if(j == -1 || lpszFind[j] == '?' || pchSrc[i] == lpszFind[j])
        {    i++;
        j++;
        }
        else
            j = next[j];
    }
    DELETE_A(next);
    
    if(j >= n)
        return i-n;
    else
        return -1;
}

void TStrFunc::LeftTrim(char *src,char fillchar)
{
  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if(IsNullStr(src))
    {
        return;               
    }
#endif
  
    char *pTemp;
    pTemp = src;
    while(!(*pTemp=='\0'))
    {
        if (*pTemp!=fillchar)
            break;
        else
            pTemp++;
    }
    //strcpy(src,pTemp);
    if(pTemp>src)
    {
#ifdef _LINUX
        memmove(src, pTemp, strlen(src)-(size_t)(pTemp-src)+1); 
#else
        //ZMP:467374
        int iSrcLen = (int)strlen(src);
        strncpy(src,pTemp,iSrcLen);
        src[iSrcLen] = '\0';
#endif
    }


}

void TStrFunc::RightTrim(char *src,char fillchar)
{  
    char *pTemp=NULL; //357133 for cppcheck
    if (IsNullStr(src) || *src=='\0')
        return;
    pTemp = src+strlen(src)-1;
    while(!(*pTemp=='\0'))
    {
        if (*pTemp!=fillchar)
            break;
        else
            pTemp--;
    }
    src[pTemp-src+1] = '\0';
}

void TStrFunc::Trim(char *src,char fillchar)
{
    RightTrim(src,fillchar);
    LeftTrim(src,fillchar);
}

void TStrFunc::LeftTrim(char *src,char* fillchar,int number)
{  
  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if(IsNullStr(src)||IsNullStr(fillchar))
    {
        return;                
    }
#endif
    char *pTemp = src;
    int i=0;
    int iIndex=number;
    int iSrcLen = (int)strlen(src);
    for(i=0;i<iIndex && pTemp[0] != '\0';)
    {
        if (pTemp[0]==fillchar[i])
        {
            pTemp++;
            i=0;
        }
        else
        {
            i++;
        }
    }

    if(pTemp>src)
    {
        #ifdef _LINUX
            int iSize = (int)strlen(pTemp);
            char sTmpStr[iSize + 1];
            memset(sTmpStr,0,(size_t)(iSize +1));
            //ZMP:467374
            strncpy(sTmpStr, pTemp, (size_t)iSize);
            sTmpStr[iSize] = '\0';
            strncpy(src,sTmpStr,(size_t)iSrcLen);
            src[iSrcLen] = '\0';
        #else
            strncpy(src,pTemp,iSrcLen);
            src[iSrcLen] = '\0';
        #endif
        
    }
}

void TStrFunc::RightTrim(char *src,char* fillchar,int number)
{
  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if(IsNullStr(fillchar))
    {
        throw TException("TStrFunc::RightTrim:Input parameter is NULL");                
    }
#endif   
  
    char *pTemp=NULL;    
    if (IsNullStr(src) || src[0]=='\0')
    {
        return;
    }
    pTemp = src+strlen(src)-1;

    int i=0;
    int iIndex=number;

    for(i=0;i<iIndex && pTemp >= src;)
    {
        if (pTemp[0]==fillchar[i])
        {
            pTemp--;
            i=0;
        }
        else
        {
            i++;
        }
    }

    src[pTemp-src+1] = '\0';
}
void TStrFunc::Trim(char *src,char* fillchar,int number)
{
    RightTrim(src,fillchar,number);
    LeftTrim(src,fillchar,number);
}

int  TStrFunc::StrNoCaseCmp(const char *sFirst,const char *sSecond)
{
	//begin ur:176096
	if(NULL == sFirst && NULL == sSecond)
	{
		return 0;
	}
#ifdef  _WIN32
    /*WINDOWS*/
	return ( NULL == sFirst ) ? stricmp( "", sSecond ) : ( NULL == sSecond ? 
		stricmp( sFirst, "" ) : stricmp( sFirst, sSecond ) );
#else
	/*UNIX*/
	return ( NULL == sFirst ) ? strcasecmp( "", sSecond ) : ( NULL == sSecond ? 
		strcasecmp( sFirst, "" ) : strcasecmp( sFirst, sSecond ) );
	//end
#endif
}

string TStrFunc::ReplaceEnv(const char * str)
{
#ifdef _ASSERT_PARAM
	if(NULL == str)
	{
		throw TException("TStrFunc::ReplaceEnv:Input parameter is NULL");
	}
#endif
    size_t spos, epos;
    string rtn = str;

    spos = rtn.find("$(");
    if( spos == string::npos )                       //���û�л����������õĻ�������
        return str;
    epos = rtn.find(")", spos + 2);
    if( epos == string::npos )
        return str;
    string env = rtn.substr(spos + 2, epos - spos - 2);
    size_t replaceLen = env.length() + 3;              //Ҫ�滻�ĳ���
    char * ptr = getenv(env.c_str());
    if( ptr == NULL )                               //��ʵ�ʵı���ֵ���浽env��
        env = "";
    else
        env = ptr;
    rtn.replace(spos, replaceLen, env);
    return ReplaceEnv(rtn.c_str());
}

string TStrFunc::ReplaceSubStr(const char * sOrgSrc, const char * sSubStr, const char * sReplaceStr)
{
  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if(IsNullStr(sOrgSrc) || IsNullStr(sSubStr))
    {
        throw TException("TStrFunc::ReplaceSubStr:Input parameter is NULL");                
    }
#endif  
    //ZMP:324395 
    if(NULL == sReplaceStr)
    {
        sReplaceStr = "";
    }
    string rtn = sOrgSrc;
    int spos;
    spos = (int)rtn.find(sSubStr);
    if( spos == (int)string::npos )                       //
        return rtn;
    string left = rtn.substr(0,(size_t)spos);
    string right = rtn.substr((size_t)spos + strlen(sSubStr));
    return left + sReplaceStr + ReplaceSubStr(right.c_str(), sSubStr, sReplaceStr);
}

char * TStrFunc::ReplaceSubStr(char * sOrgStr, const char * sSubStr, const char * sReplaceStr, char *sOutStr, bool bsensitive)
{
  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if(IsNullStr(sOrgStr) || IsNullStr(sSubStr)||IsNullStr(sOutStr))
    {
        throw TException("TStrFunc::ReplaceSubStr:Input parameter is NULL");                
    }
#endif   
    //ZMP:324415 
    if( NULL == sReplaceStr)
    {
        sReplaceStr = "";
    }
    int offset = 0;
    int retlen = 0;
    char *p1 = sOrgStr;
    char *p2 = sOutStr;
    char *pRep = (char*)sReplaceStr;

    while(*p1)
    {
        if (bsensitive)
        {
            if (*p1 == sSubStr[offset])
                offset++;
            else
                offset=0;
        }
        else
        {
            if (sLower[(int)(*p1)] == sLower[(int)sSubStr[offset]]) 
                offset++;
            else
                offset=0;
        }

        *p2++ = *p1++;
        retlen++;

        if (sSubStr[offset] == 0)
        {
            retlen -= offset;
            p2 -= offset;
            pRep = (char*)sReplaceStr;
            while(*pRep)
            {
                *p2++ = *pRep++;
                retlen++;
            }
            offset=0;
        }
    }
    *p2++ = 0;

    return sOutStr;
}

bool TStrFunc::IsDateTimeStr(const char * dt)
{
    //201197 begin
    if( NULL == dt )
    {
        return false;
    }
    //201197 end
    
    if (strlen(dt) != 14)
        return false;
    char tmp[10]={0}; //357133 for cppcheck
    memcpy(tmp,dt,8);
    tmp[8] = '\0';
    if (!IsDateStr(tmp))
        return false;
    memcpy(tmp,dt+8,6);
    tmp[6]= '\0';
    if (!IsTimeStr(tmp))
        return false;
    return true;
}

bool TStrFunc::IsDateStr(const char * pchString)
{
    //201197 begin
    if( NULL == pchString )
    {
        return false;
    }
    //201197 end
    
    int num;
    char tmp[10]={0}; //357133 for cppcheck
    int iYear,iMonth,i;

    if(strlen(pchString) != 8)
        return false;
    int iLen = (int)strlen(pchString);
    for (i = 0; i < iLen; i++)
    {
        bool bFlag = ( (pchString[i] < '0') || (pchString[i] > '9') );//315428 
        if ( bFlag )
            return false;
    }

    strncpy(tmp , pchString , 4);
    tmp[4] = 0;
    num = atoi(tmp);
    iYear = num;
    bool bYearFlag =( (num < 1900) || (num > 2099) ) ; //315428 
    if( bYearFlag )
        return false;
    strncpy(tmp , pchString + 4 , 2);
    tmp[2] = 0;
    num = atoi(tmp);
    iMonth = num;
    bool bMonFlag =( (num < 1) || (num > 12) ); //315428 
    if( bMonFlag )
        return false;
    strncpy(tmp , pchString + 6 , 2);
    tmp[2] = 0;
    num = atoi(tmp);
    bool bDayFlag = ((num < 1) || (num > 31)) ;  //315428 
    if( bDayFlag )
        return false;
    switch(iMonth)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        if((num < 1) || (num > 31))
            return false;
        break;
    case 2:
        {
            bool bLeapYearFlag = ( (iYear%4==0&&iYear%100!=0)||iYear%400==0 ) ; //315428 
            if( bLeapYearFlag ) //judge leap year
            {
                bool bFebFlagDays = ( (num < 1) || (num > 29) ); //315428 
                if( bFebFlagDays )
                    return false;
            }
            else
            {
                bool bFebFlagDays =( (num < 1) || (num > 28) ) ;//315428 
                if( bFebFlagDays )
                    return false;
            }
            break;
        }
    default :
        {
            bool bFlag = ( (num < 1) || (num > 30) ); //315428 
            if( bFlag )
                return false;
        }
    }
    return true;
}
bool TStrFunc::IsTimeStr(const char *pchString)
{
    //201197 begin
    if( NULL == pchString )
    {
        return false;
    }
    //201197 end
    
    int num,i;
    char tmp[10]={0}; //357133 for cppcheck

    if(strlen(pchString) != 6)
        return false;
    int iLen = (int)strlen(pchString);
    for (i=0;i<iLen;i++)
    {
        if ((pchString[i]<'0')||(pchString[i]>'9'))
            return false;
    }
    strncpy(tmp , pchString , 2);
    tmp[2] = 0;
    num = atoi(tmp);
    if((num < 0) || (num > 23))
        return false;
    strncpy(tmp , pchString + 2 , 2);
    tmp[2] = 0;
    num = atoi(tmp);
    if((num < 0) || (num > 59))
        return false;
    strncpy(tmp , pchString + 4 , 2);
    tmp[2] = 0;
    num = atoi(tmp);
    if((num < 0) || (num > 59))
        return false;

    return true;
}

bool TStrFunc::IsNumStr(const char * in_string)
{
    //ZMP:324433 
    if(NULL == in_string)
    {
        return false;
    }
  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if('\0'== *in_string)
        return false;
#endif    
  
    //if( in_string == NULL )
    //    return false;

    if( *in_string  == '-' || *in_string  == '+')
    {
        ++in_string;
        if (*in_string == '\0')
            return false;
    }

    while( *in_string != '\0' )
    {
        if( !isdigit(*in_string) )
            return false;
        ++in_string;
    }
    return true;
}

bool TStrFunc::atoiEx(const char * pBuf,int size,int &result)
{
    
    char buf[64]={0}; //357133 for cppcheck
    if(size > 60 || size < 1)
        return false;
    strncpy(buf,pBuf,(size_t)size);
    buf[size]=0;
    if(! TStrFunc::IsNumStr(buf))
        return false;

  //��������ַ����Ƿ�Խ��int�Ľ���
#ifdef _ASSERT_PARAM
    long lTemp = atoll(pBuf);
    if(lTemp < MIN_32BIT_NUMBER || lTemp > MAX_32BIT_NUMBER )
        return false;
#endif 
        
    result=atoi(buf);
    return true;
}


void TStrFunc::strncpyEx(char * desc,char * source,int size,char endtag)
{
  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if(IsNullStr(desc))
    {
        throw TException("TStrFunc::strncpyEx:Input parameter is NULL");                
    }
#endif   
  
    for (int i=0;i<size && *source ;i++,desc++,source++)
    {
        *desc=*source;
        if(*desc == endtag)
        {
            *desc=0;
            break;
        }
    }
}

/**********************************************************************
*  ��  �� �� // ��Դ������һ���������й����ַ������ڶ����������е�ÿ���ַ�
* ��������� // strSrc,strSeparate
* ��������� // ��
* �� �� ֵ�� // ���˵��ָ��ַ����ַ���
***********************************************************************/
char * TStrFunc::FilterSeparate(const char *strSrc,const char *strSeparate)
{
#ifdef _ASSERT_PARAM
	if(NULL == strSrc)
	{
		throw TException("TStrFunc::FilterSeparate:Input parameter is NULL");
	}
#endif
    int strStartPos;
    int strLen;
    char cSeparate;

    static char strBuff[1024] = {0};//ZMP:477242
    static char strTarget[1024] = {0};//ZMP:477242
    memset(strBuff,0,sizeof(strBuff));
    memset(strTarget,0,sizeof(strTarget));
    //ZMP:467374
    strncpy(strBuff,strSrc,sizeof(strBuff)-1);
    strBuff[sizeof(strBuff)-1] = '\0';
    //ZMP:305593
    cSeparate=*strSeparate++;
    while(cSeparate != '\0')
    {
        int i=0;
        //ZMP:467374
        strncpy(strTarget,"", sizeof(strTarget)-1);
        while(strBuff[i] != '\0')
        {
            strLen = 0;
            strStartPos = i;
            while((strBuff[i] != cSeparate)&&(strBuff[i] !='\0'))
            {
                i++;
                strLen++;
            }

            if( strLen > 0 )
                strncat(strTarget,strBuff+strStartPos,(size_t)strLen);

            if (strBuff[i] == cSeparate)
                i++;
        }
        //ZMP:467374 
        strncpy(strBuff,strTarget, sizeof(strBuff)-1);
        strBuff[sizeof(strBuff)-1] = '\0';
        cSeparate=*strSeparate++;
    }

    return strTarget;    
}


char * TStrFunc::FormatChar(char *sLine)
{
    //201197 begin
    if( NULL == sLine )
    {
        return NULL;
    }
    //201197 end
    
    char *pStrPos=sLine;
    while(*pStrPos)
    {
        if(*pStrPos=='\r')
            *pStrPos = '\0';
        if(*pStrPos=='\n')
            *pStrPos = '\0';
        if(*pStrPos == '\0')
            break;
        pStrPos++;
    }
    return sLine;
}

int TStrFunc::StrToInt(const char *sSrc)
{
    int    iReturn=0;
    int iPos=0;

  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if((NULL == sSrc) || (0 == strcmp(sSrc,"")))
    {
        return iReturn;  
    }
    
    if(TStrFunc::IsNumStr(sSrc))
    {
        long lTemp = atoll(sSrc);
        if(lTemp < MIN_32BIT_NUMBER || lTemp > MAX_32BIT_NUMBER )
        {
            //Խ��
            throw TException("TStrFunc::StrToInt:Input sSrc is out of bound");                
        }
    }
    else
    {
        //������ȫ������
        throw TException("TStrFunc::StrToInt:Input sSrc is not number");          
    }

    int vsZero = 1;
    if('-'==sSrc[0])
    {
        ++sSrc;
        vsZero = -1;
    }
    else if('+'==sSrc[0])
    {
        ++sSrc;  
    }
      
#endif 

    if(sSrc!=NULL)
    {
        while(sSrc[iPos]!='\0')
        {
            iReturn=iReturn*10+sSrc[iPos]-'0';
            iPos++;
        }//end of while(sSrc[iPos]!='\0')
    }

#ifdef _ASSERT_PARAM
    //vsZero��������ǰ���������
    iReturn =  vsZero* iReturn;  
#endif  
   
    return iReturn;
}

long long TStrFunc::StrToLLong(const char *sSrc,bool bSignFlag)
{  
    long long    llReturn=0;
    int iPos=0;
    int iSignFlag = 1;

  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if((NULL == sSrc) || (0 == strcmp(sSrc,"")))
    {
         return llReturn; 
    }

    int allLength = strlen(sSrc); //�������ŵ��ܳ���
    int vsZero; //1Ϊ�� ��-1Ϊ��, 0Ϊ���Ϸ�
    int length; //�����ַ�����
     //357133 for cppcheck
    char outStr[20]={0}; //��¼ȥ������λ���ַ���
    char *pStr = outStr; //ָ����ַ�����ָ��
    vsZero = IsNumString(sSrc,pStr,length);

    if(0 == vsZero)
    {
        //������ȫ������
        throw TException("TStrFunc::StrToLLong:Input sSrc is not number");      
    }

    if(false == bSignFlag)
    {
        //Ĭ�ϵĲ������ŵ�
        if(allLength != length)
        {
            //���ô����ŵ�ʱ��������ˡ�
            //������ȫ������
            throw TException("TStrFunc::StrToLLong:the bSignFlag is false");                         
        }        
    }

    if(length>19)
    {
        //long����Խ��
        throw TException("TStrFunc::StrToLLong:Input parameter is too long");                
    }
    if(19 == length)
    {
        //int�ļ��޳���
        if(IsOutofLLong(outStr,vsZero))
        {
            //int����Խ��
            throw TException("TStrFunc::StrToLLong:Input parameter is outofbound");                           
        }  
    }

    //sSrc�ع鵽��ǰ��
   // sSrc = sSrc - allLength;        
#endif  

    if(sSrc!=NULL)
    {
        if (bSignFlag)
        {
            if (sSrc[iPos] == '+')
                iPos++;
            else if (sSrc[iPos] == '-')
                iSignFlag = -1,iPos++;
        }
                
        while(sSrc[iPos]!='\0')
        {
            llReturn=llReturn*10+sSrc[iPos]-'0';
            iPos++;
        }//end of while(sSrc[iPos]!='\0')
    }
    
    llReturn = llReturn*iSignFlag;
    
    return llReturn;
}

llong TStrFunc::StrTollong(const char *sSrc)
{  
    llong    llReturn=0;
    int iPos=0;

  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if((NULL == sSrc) || (0 == strcmp(sSrc,"")))
    {
         return llReturn; 
    }

    int allLength = strlen(sSrc); //�������ŵ��ܳ���
    int vsZero; //1Ϊ�� ��-1Ϊ��, 0Ϊ���Ϸ�
    int length; //�ַ�����
     //357133 for cppcheck
    char outStr[20]={0}; //��¼ȥ������λ���ַ���
    char *pStr = outStr; //ָ����ַ�����ָ��
    vsZero = IsNumString(sSrc,pStr,length);
    if(0 == vsZero)
    {
        //������ȫ������
        throw TException("TStrFunc::StrTollong:Input parameter is not right");      
    }
    
    if(length>19)
    {
        //int����Խ��
        throw TException("TStrFunc::StrTollong:Input parameter is outofbound");                
    }
    if(19 == length)
    {
        //int�ļ��޳���
        if(IsOutofLLong(outStr,vsZero))
        {
            //int����Խ��
            throw TException("TStrFunc::StrTollong:Input parameter is outofbound");                           
        }  
    }
    //�˴��Ѿ�û���쳣�ˣ�ָ����������ŵĵ�һ���ַ�
    if(allLength==length+1) 
    {
        ++sSrc;
    }     
#endif  

    if(sSrc!=NULL)
    {
        while(sSrc[iPos]!='\0')
        {
            llReturn=llReturn*10+sSrc[iPos]-'0';
            iPos++;
        }//end of while(sSrc[iPos]!='\0')
    }
    
#ifdef _ASSERT_PARAM
    //vsZero��������ǰ���������
    llReturn =  vsZero* llReturn;  
#endif  
    
    return llReturn;
}


//��������ת��
int TStrFunc::StrTollong(const char *sSrc, llong &iResult)
{  
    iResult=0;
    int iPos=0;
    bool bIsNegtive = false;

  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if((NULL == sSrc) || (0 == strcmp(sSrc,"")))      
    {
        return 0;  
    } 
  
    int vsZero; //1Ϊ�� ��-1Ϊ��, 0Ϊ���Ϸ�
    int length; //�����ַ�����
     //357133 for cppcheck
    char outStr[20]={0}; //��¼ȥ������λ���ַ���
    char *pStr = outStr; //ָ����ַ�����ָ��
    vsZero = IsNumString(sSrc,pStr,length);

    if(0 == vsZero)
    {
        return -1;
    }

    if(length>19)
    {
        return -1;
    }
    if(19 == length)
    {
        //llong�ļ��޳���
        if(IsOutofLLong(outStr,vsZero))
        {
            return -1;
        }  
    }       
#endif  


    if(sSrc!=NULL)
    {
    	//�����������
    	if(sSrc[iPos] == '-')
    	{
    		bIsNegtive = true;
    		++iPos;	
    	}
    	
    	//ת������
        while(sSrc[iPos]!='\0')
        {
        	if(sSrc[iPos] < '0' || sSrc[iPos] > '9')
        	{
        		return -1;
        	}
            iResult = iResult*10 + sSrc[iPos]-'0';
            iPos++;
        }//end of while(sSrc[iPos]!='\0')
        
        if(bIsNegtive)
        {
        	iResult = -iResult;	
        }
    }
    return 0;
}


bool TStrFunc::IsEmpty(const char *sSrc)
{
    if(sSrc==NULL)
    {
        return true;
    }
    return strlen(sSrc) == 0;
}

/**********************************************************************
*  ��  �� ��ʮ�������ַ���ת��ʮ��������
* ���������strSrc
* �����������
* �� �� ֵ��ת���õ�����
***********************************************************************/
llong TStrFunc::StrTollongH(const char * sSrc)
{
    llong iRet = 0;
    char * p = (char *)sSrc;
    char c;

    if (NULL==sSrc)
    {
        return iRet;
    }

    while ( *p!='\0')
    {
        c = *p;
        if ( c>='0'&& c<='9')
        {
            iRet = iRet*16 + (c - '0');
        }
        else if ( c>='A' && c<='F')
        {
            iRet = iRet*16 + (c - 'A' + 10);
        }
        else if( c>='a' && c<='f')
        {
            iRet = iRet*16 + (c - 'a' +10);
        }
        else
        {
            return -1;
        }
        p++;
    }

    return iRet;
}



/*
�޳��ַ�����β�ո�,TAB��
sBufferΪ��Ҫ�޳��ո��TAB�����ַ�����ַ
*/
void TStrFunc::AllTrim(char *sBuffer)
{
  //��������ַ����ķ�Ϸ�
#ifdef _ASSERT_PARAM
    if(IsNullStr(sBuffer))
    {
        throw TException("TStrFunc::AllTrim:Input parameter is NULL");                
    }
#endif   
  
    long lIndex=0;
    long lFirstCharFlag=1;       //���ַ���־
    long lFirstPos=0;            //���ַ�λ��
    long lEndPos=0;              //β�ַ�λ��

    for(lIndex=0;sBuffer[lIndex]!='\0';lIndex++)
    {
        if( (sBuffer[lIndex]==' ')||
            (sBuffer[lIndex]=='\t') )
        {
            if(lFirstCharFlag==1)
            {
                lFirstPos++;
            }
        }
        else
        {
            lFirstCharFlag=0;
            lEndPos=lIndex;
        }
    }

    for(lIndex=lFirstPos;lIndex<=lEndPos;lIndex++)
    {   //�ַ�ǰ��
        sBuffer[lIndex-lFirstPos]=sBuffer[lIndex];
    }

    sBuffer[lEndPos-lFirstPos+1]='\0';
}


/*
�����ַ���Сд�Ƚ��ַ�����С
��������  0 ��ʾ�ַ������
>0 ��ʾ��һ�����ڵڶ���
<0 ��ʾ�ڶ����ַ������ڵ�һ��
*/
int TStrFunc::StrNocaseCmp(const char *sFirst,const char *sSecond)
{
	//begin ur:176096
	if(NULL == sFirst && NULL == sSecond)
	{
		return 0;
	}
#ifdef  _WIN32
	/*WINDOWS*/
	return ( NULL == sFirst ) ? stricmp( "", sSecond ) : ( NULL == sSecond ? 
		stricmp( sFirst, "" ) : stricmp( sFirst, sSecond ) );
#else
	/*UNIX*/
	return ( NULL == sFirst ) ? strcasecmp( "", sSecond ) : ( NULL == sSecond ? 
		strcasecmp( sFirst, "" ) : strcasecmp( sFirst, sSecond ) );
	//end
#endif
}


/*���ú���,������ת�����ַ������*/
const char* TStrFunc::IntToChar(int iIn)
{
    static char sInt[16]={0}; //357133 for cppcheck
    //ZMP:467374 
    snprintf(sInt, sizeof(sInt), "%d",iIn);
    sInt[sizeof(sInt)-1] = '\0';

    return sInt;
}


/*���ú���,��������ת�����ַ������*/
const char* TStrFunc::NumToChar(const long long iIn)
{
#ifdef _ASSERT_PARAM
    static char sInt[21]={0}; //357133 for cppcheck
#else
    static char sInt[21]={0}; //357133 for cppcheck    
#endif 
    //ZMP:467374   
    snprintf(sInt, sizeof(sInt), "%lld",iIn);
    sInt[sizeof(sInt)-1] = '\0';

    return sInt;
}

void TStrFunc::IntToChar(char sOut[], size_t iOutSize, int iIn)
{
    if(sOut == NULL || iOutSize <=0 )
    {
        return;
    }

    snprintf(sOut, iOutSize, "%d",iIn);
    sOut[iOutSize - 1] = '\0';
}

void TStrFunc::NumToChar(char sOut[], size_t iOutSize, long long iIn)
{
    if(sOut == NULL || iOutSize <=0 )
    {
        return;
    }
    snprintf(sOut, iOutSize, "%lld",iIn);
    sOut[iOutSize - 1] = '\0';
}

/*ʹ������ӳ��Ĳ��ִ�Сд�ַ����Ƚ�,����ֵͬ StrNocaseCmp */
int TStrFunc::StrCmpNoCase(const char *sFirst,const char *sSecond)
{
    //201197 begin
    if(  sFirst ==  NULL &&  sSecond == NULL)
    {
        return 0;
    }
    if(sFirst ==  NULL )
    {
        sFirst = "";
    }
    else if(sSecond == NULL)
    {
        sSecond= "";
    }    
    //201197 end
    
    int result;

    while ((result = sLower[(int)(*sFirst)] - sLower[(int)(*sSecond++)]) == 0)
        if (*sFirst++ == '\0')
            break;

    return result;  
}


//���ַ�������Hash��ת����һ���Ƚ�ɢ�е�ֵ
long TStrFunc::StrToHash(const char* pszSrc)
{
    unsigned long h = 0;
    unsigned long g;

    //ZMP:324433
    if(NULL == pszSrc)
    {
        return 0;
    }
    
    while( *pszSrc )
    {
        h =(unsigned long)((h<< 4) + *pszSrc++);
        g = h & 0xf0000000L;
        
        if( g ) 
            h ^= g >> 24;
            
        h &= ~g;
    }
    
    return (long)h;
}

  //�ж��ַ����Ƿ�Ϊ��,���ַ�������true
bool TStrFunc::IsNullStr(const char* src)
{
    bool isNull = false;
    if(NULL == src)
    {
        isNull = true;
    }    
    return isNull;  
}

  /**********************************************************************
  *  ��  �� �� // �ж��ַ��Ƿ�ȫ����������ɣ����������Ž��д���
  * ��������� // inSrc�����ַ���
  * ��������� // outSrcȥ���Ӽ��ŵ������ַ���,  outStr�ĳ���
  * �� �� ֵ�� // ����0˵�����Ϸ�������1˵��Ϊ����������-1˵��Ϊ����
  ***********************************************************************/
int TStrFunc::IsNumString(const char * inStr, char* outStr,int &length)
{
    int flag = 0;
    length = 0; 
    
    if( inStr == NULL )
        return flag;

    if( '\0' == *inStr)
        return flag;
    
    if( *inStr  == '-')
    {
        ++inStr;
        if (*inStr == '\0')
        {
            return flag;          
        }
        else
        {
            flag = -1;    //Ϊ����
        }
    }
    else if(*inStr  == '+')
    {
        ++inStr;
        if (*inStr == '\0')
        {
            return flag;  //���Ϸ�        
        }
        else
        {
            flag = 1;    //Ϊ����
        }          
    }     
    else
    {
        flag = 1;  
    }

    while( *inStr != '\0' )
    {
        if(*inStr < '0' || *inStr > '9')
        {
            flag = 0;    //���Ϸ�
            return flag;  
        }
        *outStr = *inStr;  //��¼��ȥ���ŵ�����
        ++outStr;   
        ++inStr;    
        ++length;   //��¼��ȥ�������ֵ�λ��
    }
    *outStr = '\0'; //���һ����β�ַ�
    return flag; 
}

  /**********************************************************************
  *  ��  �� �� // �ж�int�ַ��Ƿ�Խ��(ע����֤λ��Ϊ10λ��������ж�)
  * ��������� // intStr��ʾ���֣� flag��ʾ����(��Ϊ1����Ϊ-1)
  * ��������� // 
  * �� �� ֵ�� // Խ��bool=true, ����bool=false
  ***********************************************************************/  
bool TStrFunc::IsOutofInt(const char *intStr, int flag)
{
    bool isOut = true;
    char intMax[10] = {'2','1','4','7','4','8','3','6','4','7'};
    char intMin[10] = {'2','1','4','7','4','8','3','6','4','8'};
    int index = 0;
    if(-1 == flag)
    {
        //����
        while( *intStr != '\0' )
        {
            if(*intStr > intMin[index])
            {
                //Խ����
                return isOut;  
            } 
            ++index; 
            ++intStr;    
        }    
    }

    if(1 == flag)
    {
        //����
        while( *intStr != '\0' )
        {
            if(*intStr > intMax[index])
            {
                //Խ����
                return isOut;  
            }  
            ++index;
            ++intStr;    
        }    
    }
    isOut = false;
    return isOut;      
}

  /**********************************************************************
  *  ��  �� �� // �ж�long�ַ��Ƿ�Խ��(ע����֤λ��Ϊ19λ��������ж�)
               //[-9223372036854775808,9223372036854775807]
  * ��������� // intStr��ʾ���֣� flag��ʾ����(��Ϊ1����Ϊ-1)
  * ��������� // 
  * �� �� ֵ�� // Խ��bool=true, ����bool=false
  ***********************************************************************/  
bool TStrFunc::IsOutofLLong(const char * inStr, int flag)
{
    bool isOut = true;
    char llongMax[19] = {'9','2','2','3','3','7','2','0','3','6'
                      ,'8','5','4','7','7','5','8','0','7'};
    char llongMin[19] = {'9','2','2','3','3','7','2','0','3','6'
                      ,'8','5','4','7','7','5','8','0','8'};
    int index = 0;
    if(-1 == flag)
    {
        //����
        while( *inStr != '\0' )
        {
            if(*inStr > llongMin[index])
            {
                //Խ����
                return isOut;  
            } 
            ++index; 
            ++inStr;    
        }    
    }

    if(1 == flag)
    {
        //����
        while( *inStr != '\0' )
        {
            if(*inStr > llongMax[index])
            {
                //Խ����
                return isOut;  
            }
            ++index;  
            ++inStr;    
        }    
    }
    isOut = false;
    return isOut;   
}
//�Ϸ� IPV4 ��ַ�ļ�⡣
bool TStrFunc::IsIPV4(const char *intStr)
{
    bool bRet = true;
    if(intStr == NULL)
    {
        return false;
    }
    //   �Ե�ŷָ�����4�Σ�ÿ��ֵ��[0,255]֮��
    static TSplit tSplit1;
    tSplit1.SetSplitter('.');
    tSplit1.SetString(intStr);
    int iCount = tSplit1.GetCount();
    if(iCount == 4)
    {
        for (int i = 0; i < iCount; i++)
        {
            if (tSplit1.GetIndex(i) == NULL || atol(tSplit1.GetIndex(i))< 0 || atol(tSplit1.GetIndex(i))>255)
            {
                bRet = false;
                break;
            }
        }
    }
    else
    {
        return false;
    }
    return bRet;
}

int TStrFunc::StrCatFMT(char *s, const int iSize, const char *fmt, ...)//a;b;
{
   va_list ap;
   va_start(ap, fmt);

   int len = vsnprintf(s + strlen(s), iSize - strlen(s), fmt, ap);
   va_end(ap);
   return len;
}

void TStrFunc::_StrCatFMT(char *s, const int iSize, const char *fmt, ...) //a;b 
{
   va_list ap;
   va_start(ap, fmt);

   if (*s == '\0')
      vsnprintf(s, iSize, fmt + 1, ap);
   else
      vsnprintf(s + strlen(s), iSize - strlen(s), fmt, ap);
   va_end(ap);
}

