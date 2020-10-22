//CHECKED_VERSION=FR1|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TMUTEX.h
// Author: Li.ShuGang
// Date: 2008/10/28
// Description: ����
////////////////////////////////////////////////

#ifndef __FRAMEWORK_MUTEX_H_
#define __FRAMEWORK_MUTEX_H_



/* *************************************************************
TMutex ��QMDB����ඨ��������������߸ĳ�TMUTEX
�ļ����ĳ�: Mutex.h Mutex.cpp
*************************************************************  */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>


//�������������ģ����ǵ��Ʒ�ҵ����ص㣬�ݲ������������͵���
class TMUTEX //: public TBaseMutex
{
public:
	TMUTEX(bool bFlag = false);  //����ڹ����ʱ��ͳ�ʼ��bFlag=true
	~TMUTEX();

public:
    int Create();             //������
    int Destroy();            //������

    int Lock(bool bFlag);     //����
    int UnLock(bool bFlag);   //����
    
    const char* GetErrMsg(int iErrno); //���溯���ķ�0����ֵ���Ǵ���ģ�����ͨ������ӿ�����ȡ����ԭ��
	   	
private:
	pthread_mutex_t     mutex;
	pthread_mutexattr_t mattr;
	bool bIsCreate;
};


#endif //__MEMORY_DATABASE_MUTEX_H_
