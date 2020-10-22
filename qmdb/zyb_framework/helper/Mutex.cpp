//CHECKED_VERSION=FR1|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TMutex.cpp
// Author: Li.ShuGang
// Date: 2008/10/28
// Description: ����
////////////////////////////////////////////////

#include "Mutex.h"
#include "debug_new.h"



////////////////////////////////////////////////
// �������� TMUTEX
////////////////////////////////////////////////

TMUTEX::TMUTEX(bool bFlag)
{
	bIsCreate = false;
	
	if(bFlag)
	{
		int iRet = Create();
		if(iRet == 0)
			bIsCreate = true;	
	}
}


TMUTEX::~TMUTEX()
{
	Destroy();
}


//������
int TMUTEX::Create()
{	
	//����Ѿ�������ֱ�ӷ���
	if(bIsCreate == true)
	{
		return 0;	
	}
	
	//���û���������
	int iRet = pthread_mutexattr_init(&mattr);
	if(iRet != 0)
	{
		return iRet;	
	}
	
	//Ĭ�ϣ����̼�ʹ��	
	iRet = pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	if(iRet != 0)
	{
		return iRet;	
	}

#ifdef OS_HP
	iRet =  pthread_mutexattr_setspin_np(&mattr, PTHREAD_MUTEX_SPINONLY_NP);
	if(iRet != 0)
	{
		return iRet;	
	}
#endif
		
	//���û�����
	iRet = pthread_mutex_init(&mutex, &mattr);
	if(iRet != 0)
	{
		return iRet;	
	}
	
	return 0;
}


//������ 
int TMUTEX::Destroy()
{
	//�ͷŻ���������
	int iRet = pthread_mutexattr_destroy(&mattr);
	if(iRet != 0)
	{
		return iRet;	
	}
	
	//�ͷŻ�����
	iRet = pthread_mutex_destroy(&mutex);
	if(iRet != 0)
	{
		return iRet;	
	}
	
	return 0;
}   
    

//����    
int TMUTEX::Lock(bool bFlag)
{
	if(bFlag)
	{
		return pthread_mutex_lock(&mutex); /* acquire the mutex */
	}
	else
	{
		return 0;
	}
}



//����   
int TMUTEX::UnLock(bool bFlag)
{
	if(bFlag)
	{
		return pthread_mutex_unlock(&mutex); /* release the mutex */
	}
	else
	{
		return 0;	
	}	
}


//���溯���ķ�0����ֵ���Ǵ���ģ�����ͨ������ӿ�����ȡ����ԭ��
const char* TMUTEX::GetErrMsg(int iErrno)
{
	return strerror(iErrno);	
}


