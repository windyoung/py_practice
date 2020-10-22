//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: SOHelper.h
// Author: Zhangyu
// Date: 2006/12/7
// Description: 动态库助手类
////////////////////////////////////////////////
#ifndef __SO_HELPER_H__
#define __SO_HELPER_H__

#ifdef _WIN32
#include <windows.h> 
#else
#include <dlfcn.h>
#endif

class SOHelper
{
private:
#ifdef _WIN32
    HINSTANCE m_handle;         //动态库句柄 Windows
    TCHAR m_LastErrMsg[128];    //出错信息缓冲区 Windows
#else
    void * m_handle;            //动态库句柄 Unix/Linux
#endif
    void * lastSymbolHandle;    //最后找到的函数地址

public:
    SOHelper();         
    ~SOHelper();
public:    

#ifdef _WIN32
    HINSTANCE getHandle();              //获取动态库句柄 Windows
    void setHandle(HINSTANCE handle);   //设置动态库句柄 Windows
#else
    void * getHandle();                 //获取动态库句柄 UnixLinux
    void setHandle(void * handle);      //设置动态库句柄 UnixLinux
#endif

    /**
     * @brief 打开一个动态库
     * @param FileName [in] 动态库文件名
     * @param iRangMode [in] comes from dlfcn.h
     *      RTLD_GLOBAL: Symbols in this dlopen'ed obj are visible to other dlopen'ed objs.
     *      RTLD_LOCAL : Symbols in this dlopen'ed obj are not visible to other dlopen'ed objs
     * @return  如果返回失败可以使用GetLastError获取出错信息
     */
    bool OpenLibrary(const char * FileName, int iRangeMode = RTLD_LOCAL);

    /**********************************************************************
    函    数: void * LoadSymbol(const char * SymbolName)
    功    能: 加载动态库中的一个函数
    输入参数: SymbolName -- 函数名 
    返回结果: 其它 - 函数地址  NULL - 失败
              如果返回失败可以使用GetLastError获取出错信息
    ************************************************************************/
    void * LoadSymbol(const char * SymbolName);

    /**********************************************************************
    函    数: void   CloseLibrary()
    功    能: 关闭一个已经打开的动态库
    输入参数: 无
    返回结果: 无
    ************************************************************************/
    void   CloseLibrary();

    /**********************************************************************
    函    数: const char * const GetLastError() 
    功    能: 获取出错信息
    输入参数: 无 
    返回结果: 出错的描述信息
    ************************************************************************/
    const char * GetLastError() ;
    
    /**********************************************************************
    函    数: const char * const GetLibName() 
    功    能: 返回动态库的名称
    输入参数: 无 
    返回结果: 返回动态库的名称
    ************************************************************************/    
    const char * GetLibName();
private:
    char m_sLibName[1024];
};

#endif
