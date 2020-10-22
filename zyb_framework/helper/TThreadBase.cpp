//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/****************************************************************************************
*@Copyrights  2007�����������Ͼ�����������޹�˾ ������ CCB��Ŀ��
*@                   All rights reserved.
*@Name��	    TThreadBase.h		
*@Description�� �̻߳���		
*@Author:		li.shugang
*@Date��	    2008��11��29��
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
* ��������	:  SetThreadInfo()
* ��������	:  �����̵߳������Ϣ    
* ����		:  parg ��������   
* ����		:  stack ����ջ�ռ��С 
* ���		:  ��
* ����ֵ	:  ��
* ����		:  li.shugang
*******************************************************************************/
void TThreadBase::SetThreadInfo(LPVOID parg, UINT32 stack)
{
     m_lpVoid = parg;
     m_iStack = stack;
}


/******************************************************************************
* ��������	:  Run()
* ��������	:  �����̣߳����嶯����Ҫ������д    
* ����		:  ��    
* ���		:  UINT32* phandle����Windows���淵�صľ���������Linux/Unix�������֮
* ����ֵ	:  �ɹ�����0������������󣬷���-1
* ����		:  li.shugang
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
* ��������	:  GetTID()
* ��������	:  ��ȡ�̵߳�ID    
* ����		:  ��    
* ���		:  ��
* ����ֵ	:  �̵߳�ID������������󣬷���-1
* ����		:  li.shugang
*******************************************************************************/
int TThreadBase::GetTID()
{
     return (int)m_iTID;
}


/******************************************************************************
* ��������	:  KillThread()
* ��������	:  ɱ���߳�    
* ����		:  ��
* ���		:  ��
* ����ֵ	:  ����ɹ�=0������������󣬷���-1
* ����		:  li.shugang
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

