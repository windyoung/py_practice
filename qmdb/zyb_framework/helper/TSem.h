//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/*TSem.h*/
/*信号量操作底层函数封装声明*/
#ifndef _TSEM_H_
#define _TSEM_H_


/*备注:_WIN32下考虑将HANDLE转换成long型后再转换到int型*/

#ifdef  _WIN32

#pragma warning(disable:4312)           //unsigned int 或者int 转换到HANDLE
#pragma warning(disable:4311)           //void*转换到long

#include <windows.h>

#else   //_UNIX

#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>

/*信号量控制信息结构*/
union semun
{
    int val;
    struct semid_ds *buf;
    ushort *array;
};

/*防止arg冲突*/
/*
} arg;
*/

#endif  //_WIN32 _UNIX

#define STR_FORMAT_SEM_ID               "SEM%09d"           //WIN32下用于生成标识名称
#define SEM_ID_MOD                      10000000000         //用于生成标识取模
#define MAX_SEM_ID_SIZE                 64                  //SEM_ID字符串格式的长度
#define SEM_FTOK_PATH_ENV               "CCB_INSTANCE_HOME" //UNIX下用于fotk()函数的环境变量路径
#define SEM_FTOK_PATH                   "/"                 //UNIX下用于ftok()函数
#define MAX_SEM_ERROR_MSG_SIZE          1024                //信号量类错误信息的最大长度

#define SEM_BASE                        200                 //信号量操作信息起始常量

#define ERROR_SEM_PARAMETER             SEM_BASE+1          //参数错误
#define ERROR_SEM_CREATE                SEM_BASE+2          //信号量创建失败
#define ERROR_SEM_OPEN                  SEM_BASE+3          //信号量获取信号标识或句柄失败
#define ERROR_SEM_SET_VALUE             SEM_BASE+4          //信号量设置信号值失败
#define ERROR_SEM_P_OPERATION           SEM_BASE+5          //信号量P操作失败
#define ERROR_SEM_V_OPERATION           SEM_BASE+6          //信号量V操作失败
#define ERROR_SEM_DESTROY               SEM_BASE+7          //信号量删除失败
#define ERROR_SEM_EXIST                 SEM_BASE+8          //信号量已经存在
#define ERROR_SEM_FTOK                  SEM_BASE+9          //FTOK失败
#define ERROR_SEM_GETVAL                SEM_BASE+10         //信号量获取值失败

class TSem
{
    public:
        TSem();
        virtual ~TSem();

    /*public类函数*/
        /*根据iCreateID生成信号量,设置信号量为iValue,返回句柄到*pSemIDorHandle*/
        static int SemCreate(int iCreateID,int iValue,int *pSemIDorHandle);
        
        /*根据iCreateID获取信号量标识或者句柄*pSemIDorHandle*/
        static int SemOpen(int iCreateID,int *pSemIDorHandle);

        /*删除iSemIDorHandle对应的信号量*/
        static int SemDestroy(int iSemIDorHandle);
        
        /*信号量P操作,减少信号量值abs(iSemValue)*/
        static int SemP(int iSemIDorHandle,int iSemValue=1);

        /*信号量V操作,增加信号量值abs(iSemValue)*/
        static int SemV(int iSemIDorHandle,int iSemValue=1);

        //查看指定信号量的值
           static int SemGetValue(int iSemIDorHandle,int &iValue);

        static int GetErrorCode();

        static const char* GetErrMsgByCode(int iErrCode,int iDetailCode=0);
    
    private:
        static int iErrCode;                                //系统错误编码
};

#endif  //_TSEM_H_

