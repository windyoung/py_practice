//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: SOHelper.h
// Author: Zhangyu
// Date: 2006/12/7
// Description: ��̬��������
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
    HINSTANCE m_handle;         //��̬���� Windows
    TCHAR m_LastErrMsg[128];    //������Ϣ������ Windows
#else
    void * m_handle;            //��̬���� Unix/Linux
#endif
    void * lastSymbolHandle;    //����ҵ��ĺ�����ַ

public:
    SOHelper();         
    ~SOHelper();
public:    

#ifdef _WIN32
    HINSTANCE getHandle();              //��ȡ��̬���� Windows
    void setHandle(HINSTANCE handle);   //���ö�̬���� Windows
#else
    void * getHandle();                 //��ȡ��̬���� UnixLinux
    void setHandle(void * handle);      //���ö�̬���� UnixLinux
#endif

    /**
     * @brief ��һ����̬��
     * @param FileName [in] ��̬���ļ���
     * @param iRangMode [in] comes from dlfcn.h
     *      RTLD_GLOBAL: Symbols in this dlopen'ed obj are visible to other dlopen'ed objs.
     *      RTLD_LOCAL : Symbols in this dlopen'ed obj are not visible to other dlopen'ed objs
     * @return  �������ʧ�ܿ���ʹ��GetLastError��ȡ������Ϣ
     */
    bool OpenLibrary(const char * FileName, int iRangeMode = RTLD_LOCAL);

    /**********************************************************************
    ��    ��: void * LoadSymbol(const char * SymbolName)
    ��    ��: ���ض�̬���е�һ������
    �������: SymbolName -- ������ 
    ���ؽ��: ���� - ������ַ  NULL - ʧ��
              �������ʧ�ܿ���ʹ��GetLastError��ȡ������Ϣ
    ************************************************************************/
    void * LoadSymbol(const char * SymbolName);

    /**********************************************************************
    ��    ��: void   CloseLibrary()
    ��    ��: �ر�һ���Ѿ��򿪵Ķ�̬��
    �������: ��
    ���ؽ��: ��
    ************************************************************************/
    void   CloseLibrary();

    /**********************************************************************
    ��    ��: const char * const GetLastError() 
    ��    ��: ��ȡ������Ϣ
    �������: �� 
    ���ؽ��: �����������Ϣ
    ************************************************************************/
    const char * GetLastError() ;
    
    /**********************************************************************
    ��    ��: const char * const GetLibName() 
    ��    ��: ���ض�̬�������
    �������: �� 
    ���ؽ��: ���ض�̬�������
    ************************************************************************/    
    const char * GetLibName();
private:
    char m_sLibName[1024];
};

#endif
