//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#include<stdio.h>
#ifndef _WIN32
#include <dlfcn.h>
#include <string.h>
#endif
#include "SOHelper.h"
#include "debug_new.h"

SOHelper::SOHelper()
{
    m_handle=NULL;
    lastSymbolHandle=NULL;
    //ZMP:305593
#ifdef _WIN32
    memset(m_LastErrMsg, 0, sizeof(m_LastErrMsg));
#endif
    memset(m_sLibName, 0, sizeof(m_sLibName) );
}

SOHelper::~SOHelper()
{
    CloseLibrary();
}

const char * SOHelper::GetLastError ()
{
#ifdef _WIN32
    LPVOID lpMsgBuf=NULL; //357111 for cppcheck
    DWORD dw = ::GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
    wsprintf(m_LastErrMsg, 
        "OS error %d: %s", 
        dw, lpMsgBuf); 

    MessageBox(NULL, m_LastErrMsg, "Error", MB_OK); 
    LocalFree(lpMsgBuf);
    return m_LastErrMsg;
#else
    return dlerror();
#endif
}

const char * SOHelper::GetLibName()
{
    return m_sLibName;
}
#ifdef _WIN32
HINSTANCE SOHelper::getHandle()
{
    return m_handle;
}

void  SOHelper::setHandle(HINSTANCE handle)
{
    m_handle=handle;
}

#else
void * SOHelper::getHandle()
{
    return m_handle;
}

void  SOHelper::setHandle(void * handle)
{
    m_handle=handle;
}
#endif
void  SOHelper::CloseLibrary()
{
    if(m_handle!=NULL)
    {
#ifdef _WIN32
        FreeLibrary(m_handle); 
#else
        dlclose(m_handle);
#endif
        m_handle=NULL;
    }
}


bool SOHelper::OpenLibrary(const char * FileName, int iRangeMode/* = RTLD_LOCAL*/)
{
#ifdef _WIN32
    m_handle=LoadLibrary(TEXT(FileName)); 
#else
    #ifdef _ASSERT_PARAM
       if(FileName==NULL)
           return false;
    #endif

    // If the value of FileName is NULL, a value for the main application is returned,but not null.
    m_handle=dlopen(FileName, RTLD_NOW | iRangeMode);
#endif
    if( FileName )
    {
        strncpy( m_sLibName, FileName,sizeof(m_sLibName) );
    }
    return m_handle!=NULL;
}

void * SOHelper::LoadSymbol(const char * SymbolName)
{
    lastSymbolHandle=NULL;
    if(m_handle)
    {
#ifdef _WIN32
        lastSymbolHandle=GetProcAddress(m_handle,TEXT(SymbolName)); 
#else
        lastSymbolHandle=dlsym(m_handle, SymbolName);

        //如果没有找到该符号，侧修正符号名（按C++规则进行）
        if(lastSymbolHandle==NULL && strlen(SymbolName)<=120)
        {    
            char interalSymbolName[128]={0}; //357111 for cppcheck
#ifdef OS_LINUX
            //ZMP:467373 
            strncpy(interalSymbolName,"_Z8", sizeof(interalSymbolName)-1);
            strcat(interalSymbolName,SymbolName);
            strcat(interalSymbolName,"v");
#else
            //ZMP:467373 
            strncpy(interalSymbolName,SymbolName, sizeof(interalSymbolName)-1);
            interalSymbolName[sizeof(interalSymbolName)-1] = '\0';
            strcat(interalSymbolName,"__Fv");
#endif
            lastSymbolHandle=dlsym(m_handle, interalSymbolName);
        }
#endif
    }
    return lastSymbolHandle;
}
