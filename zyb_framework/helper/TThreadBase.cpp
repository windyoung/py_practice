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
#include "TThreadBase.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifndef _WIN32    
    #include <unistd.h>
    #include <stdlib.h>    
    #include <pthread.h>
    #include <sys/types.h>
    #include <sys/time.h>
    #include <sys/select.h>
#else
    #include <windows.h>
#endif
#include "debug_new.h"

void* TThreadBase::agent(void* p)
{
	TThreadBase* pThread = (TThreadBase*)p;
	pThread->svc();
	return 0;
}


TThreadBase::TThreadBase()
{
     m_iTID = 0;      //pthread_t is an unsigned integer type.
     m_lpVoid = this;  
     m_iStack = 0;
}

TThreadBase::~TThreadBase()
{
    
}
    
/******************************************************************************
* 函数名称	:  SetThreadInfo()
* 函数描述	:  设置线程的相关信息    
* 输入		:  parg 函数参数   
* 输入		:  stack 函数栈空间大小 
* 输出		:  无
* 返回值	:  无
* 作者		:  li.shugang
*******************************************************************************/
void TThreadBase::SetThreadInfo(LPVOID parg, UINT32 stack)
{
     m_lpVoid = parg;
     m_iStack = stack;
}


/******************************************************************************
* 函数名称	:  Run()
* 函数描述	:  启动线程，具体动作需要子类重写    
* 输入		:  无    
* 输出		:  UINT32* phandle，在Windows下面返回的句柄，如果在Linux/Unix下面忽略之
* 返回值	:  成功返回0，如果发生错误，返回-1
* 作者		:  li.shugang
*******************************************************************************/
int TThreadBase::Run(UINT32* phandle)
{
	int hthread;    
	
#ifdef _WIN32
	hthread = (int)CreateThread(NULL, m_iStack, (LPTHREAD_START_ROUTINE)agent, m_lpVoid, 0, &m_iTID);
	
	if(hthread != 0 && phandle) 
	{
		*phandle = (UINT32)hthread;
    }
#else
	//pthread_t      dwThreadId;
	pthread_attr_t thread_attr;
	
	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setscope(&thread_attr, PTHREAD_SCOPE_SYSTEM);
	if(m_iStack > 0)
		pthread_attr_setstacksize(&thread_attr, m_iStack);
		
	if (0 != pthread_create(&m_iTID, &thread_attr, agent, m_lpVoid)) 
	{
		hthread  = 0;
	} 
	else 
	{
		hthread  = 1;
	}
    pthread_attr_destroy(&thread_attr);
#endif

	if(hthread == 0)
		return 0;
	else
		return 1;    
}


/******************************************************************************
* 函数名称	:  GetTID()
* 函数描述	:  获取线程的ID    
* 输入		:  无    
* 输出		:  无
* 返回值	:  线程的ID，如果发生错误，返回-1
* 作者		:  li.shugang
*******************************************************************************/
int TThreadBase::GetTID()
{
     return (int)m_iTID;
}


/******************************************************************************
* 函数名称	:  KillThread()
* 函数描述	:  杀死线程    
* 输入		:  无
* 输出		:  无
* 返回值	:  如果成功=0，如果发生错误，返回-1
* 作者		:  li.shugang
*******************************************************************************/
int TThreadBase::KillThread()
{
    if(m_iTID == 0)
        return -1;
        
	int iRet = 0;

#ifdef _WIN32
    iRet = TerminateThread((HANDLE)m_iTID, 0);
#else
    iRet = pthread_cancel(m_iTID);
#endif

	return iRet;    
}

