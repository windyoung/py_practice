//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/****************************************************************************************
*@Copyrights  2007，中兴软创（南京）计算机有限公司 开发部 CCB项目组
*@                   All rights reserved.
*@Name：	    TThreadBase.h		
*@Description： 线程基类		
*@Author:		li.shugang
*@Date：	    2008年11月29日
*@History:
******************************************************************************************/
#ifndef __MINI_DATABASE_THREAD_BASE_H__
#define __MINI_DATABASE_THREAD_BASE_H__

#include "TBaseType.h"
#ifndef _WIN32    
    #include <pthread.h>
#else
    #include <windows.h>
#endif

class TThreadBase
{
public:
    TThreadBase();
    ~TThreadBase(); 
    
     /******************************************************************************
	* 函数名称	:  SetThreadInfo()
	* 函数描述	:  设置线程的相关信息    
	* 输入		:  parg 函数参数   
	* 输入		:  stack 函数栈空间大小,默认为1M 
	* 输出		:  无
	* 返回值	:  无
	* 作者		:  li.shugang
	*******************************************************************************/
    void SetThreadInfo(LPVOID parg, UINT32 stack=1024*1024);
    
    
     /******************************************************************************
	* 函数名称	:  Run()
	* 函数描述	:  启动线程，具体动作需要子类重写    
	* 输入		:  无    
	* 输出		:  UINT32* phandle，在Windows下面返回的句柄，如果在Linux/Unix下面忽略之
	* 返回值	:  成功返回0，如果发生错误，返回-1
	* 作者		:  li.shugang
	*******************************************************************************/
    int  Run(UINT32* phandle);
    
    
    /******************************************************************************
	* 函数名称	:  GetTID()
	* 函数描述	:  获取线程的ID    
	* 输入		:  无    
	* 输出		:  无
	* 返回值	:  线程的ID，如果发生错误，返回-1
	* 作者		:  li.shugang
	*******************************************************************************/
	int GetTID();
	
	
	/******************************************************************************
	* 函数名称	:  KillThread()
	* 函数描述	:  杀死线程    
	* 输入		:  无
	* 输出		:  无
	* 返回值	:  如果成功=0，如果发生错误，返回-1
	* 作者		:  li.shugang
	*******************************************************************************/
	int KillThread();

protected:
    //子类的线程动作
    virtual int svc()=0;
    
    //中间的一个转接函数
    static void* agent(void* p);
    
    pthread_t m_iTID; 
    LPVOID m_lpVoid;
    UINT32 m_iStack;
};

#endif //__MINI_DATABASE_THREAD_BASE_H__
