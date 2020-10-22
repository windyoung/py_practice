//CHECKED_VERSION=FR1|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TMutex.cpp
// Author: Li.ShuGang
// Date: 2008/10/28
// Description: 锁类
////////////////////////////////////////////////

#include "Mutex.h"
#include "debug_new.h"



////////////////////////////////////////////////
// 互斥锁类 TMUTEX
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


//创建锁
int TMUTEX::Create()
{	
	//如果已经创建则直接返回
	if(bIsCreate == true)
	{
		return 0;	
	}
	
	//设置互斥锁属性
	int iRet = pthread_mutexattr_init(&mattr);
	if(iRet != 0)
	{
		return iRet;	
	}
	
	//默认：进程间使用	
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
		
	//设置互斥锁
	iRet = pthread_mutex_init(&mutex, &mattr);
	if(iRet != 0)
	{
		return iRet;	
	}
	
	return 0;
}


//销毁锁 
int TMUTEX::Destroy()
{
	//释放互斥锁属性
	int iRet = pthread_mutexattr_destroy(&mattr);
	if(iRet != 0)
	{
		return iRet;	
	}
	
	//释放互斥锁
	iRet = pthread_mutex_destroy(&mutex);
	if(iRet != 0)
	{
		return iRet;	
	}
	
	return 0;
}   
    

//加锁    
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



//解锁   
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


//上面函数的非0返回值都是错误的，可以通过这个接口来获取错误原因
const char* TMUTEX::GetErrMsg(int iErrno)
{
	return strerror(iErrno);	
}


