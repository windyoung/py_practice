//CHECKED_VERSION=FR1|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TMUTEX.h
// Author: Li.ShuGang
// Date: 2008/10/28
// Description: 锁类
////////////////////////////////////////////////

#ifndef __FRAMEWORK_MUTEX_H_
#define __FRAMEWORK_MUTEX_H_



/* *************************************************************
TMutex 与QMDB里的类定义重名，所以这边改成TMUTEX
文件名改成: Mutex.h Mutex.cpp
*************************************************************  */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>


//互斥锁是阻塞的，考虑到计费业务的特点，暂不设置其它类型的锁
class TMUTEX //: public TBaseMutex
{
public:
	TMUTEX(bool bFlag = false);  //如果在构造的时候就初始化bFlag=true
	~TMUTEX();

public:
    int Create();             //创建锁
    int Destroy();            //销毁锁

    int Lock(bool bFlag);     //加锁
    int UnLock(bool bFlag);   //解锁
    
    const char* GetErrMsg(int iErrno); //上面函数的非0返回值都是错误的，可以通过这个接口来获取错误原因
	   	
private:
	pthread_mutex_t     mutex;
	pthread_mutexattr_t mattr;
	bool bIsCreate;
};


#endif //__MEMORY_DATABASE_MUTEX_H_
