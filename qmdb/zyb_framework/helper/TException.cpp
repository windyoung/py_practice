//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#include <stdarg.h>
#include <stdio.h>
#include "TException.h"
#include "debug_new.h"

TException::TException()
{
    m_strErrMsg="Unknown Exception";
}

TException::TException(const char * fmt, ...)
{
    va_list ap;
    va_start (ap,fmt);

    try{
        char sLine[10240]={0}; //357131 for cppcheck
        //ZMP:467370 
        vsnprintf(sLine, sizeof(sLine), fmt,ap);
		sLine[sizeof(sLine)-1] = '\0';
//164830 begin
        va_start(ap,fmt);       
//164830 end

        m_strErrMsg = sLine;
        vprintf(fmt,ap);
        printf("\n\n");
    }
    catch(...)
    {
        printf("Write Buffer Overflow. [%s]\n",fmt);
    }
    va_end (ap);
}

TException::TException(const string strErrMsg)
{
    printf("%s\n\n",strErrMsg.c_str());
    m_strErrMsg=strErrMsg;
}

char* TException::GetErrMsg() const
{
    return ((char*)m_strErrMsg.c_str());
}

char* TException::ToString() const
{
    //return (char*)(("TException:" + m_strErrMsg).c_str());
    
//151264 begin 
    static string str="";
    str="TException:"+m_strErrMsg;
    return (char*)(str.c_str());
//151264 end
}

TException::~TException()
{
   
}

TException::TException(int iLine, const char *sFileName, const char * fmt, ...)
{
    va_list ap;
    va_start (ap,fmt);

    try{
        char sLine[10240]={0}; //355672 for cppcheck
        //ZMP:462008 
        vsnprintf(sLine, sizeof(sLine), fmt, ap);
        sLine[sizeof(sLine)-1] = '\0';

        char stemp[80]={0}; //355672 for cppcheck
        //ZMP:462008
        snprintf(stemp, sizeof(stemp), "[%20s:%5d] ",sFileName,iLine);
        stemp[sizeof(stemp)-1] = '\0';
        printf("%s ",stemp);
        m_strErrMsg = stemp;
        m_strErrMsg = m_strErrMsg + sLine;
        
        va_start (ap,fmt);
        vprintf(fmt,ap);
        printf("\n\n");
    }
    catch(...)
    {
        printf("Write Buffer Overflow. [%s]\n",fmt);
    }
    va_end (ap);
}


