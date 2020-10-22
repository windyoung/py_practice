//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/*TShm.h*/
/*共享内存操作底层函数封装声明*/
#ifndef _TSHM_H_
#define _TSHM_H_

/*备注:_WIN32下考虑将HANDLE转换成long型后再转换到int型*/

#ifdef  _WIN32

#pragma warning(disable:4312)           //unsigned int 或者int 转换到HANDLE
#pragma warning(disable:4311)           //void*转换到long

#include <windows.h>

#else   //_UNIX

#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>
#include <signal.h>

#endif  //_WIN32 _UNIX

#include "Common.h"
#include "TException.h"

#ifdef HP_UNIX
//  #pragma pack 8
#endif

#define STR_FORMAT_SHM_ID               "SHM%09d"           //WIN32下用于生成标识名称
#define SHM_ID_MOD                      10000000000         //用于生成标识取模
#define MAX_SHM_ID_SIZE                 64                  //SHM_ID字符串格式的长度
#define SHM_FTOK_PATH_ENV               "CCB_INSTANCE_HOME" //UNIX下用于fotk()函数的环境变量路径
#define SHM_FTOK_PATH                   "/"                 //UNIX下用于ftok()函数
#define MEM_TAB_IPC_KEY                 0x1119
#define SHARE_TABLE_NEW                 3721
#define SHARE_TABLE_SGA                 "_TAB_SGA"

#define SHM_BASE                        100                 //共享内存操作信息起始常量

#define ERROR_SHM_PARAMETER             SHM_BASE+1          //参数错误
#define ERROR_SHM_CREATE                SHM_BASE+2          //共享内存创建失败
#define ERROR_SHM_ATTACH                SHM_BASE+3          //共享内存连接失败
#define ERROR_SHM_DETACH                SHM_BASE+4          //共享内存断开失败
#define ERROR_SHM_DESTROY               SHM_BASE+5          //共享内存删除失败
#define ERROR_SHM_EXIST                 SHM_BASE+6          //共享内存已经存在
#define ERROR_SHM_NOT_EXIST             SHM_BASE+7          //共享内存不存在
#define ERROR_SHM_FTOK                  SHM_BASE+8          //FTOK失败

class TShm
{
    public:
        TShm();
        virtual ~TShm();

    /*public类函数*/
        /*根据iCreateID,iSize生成共享内存,返回句柄到*pShmIDorHandle*/
        static int ShmCreate(int iCreateID,size_t iSize,int *pShmIDorHandle, bool bIsExcel=true);

        /*连接iCreateID对应的共享内存,设置sBuffer为连接后的指针值,返回共享内存标识或者句柄*pShmIDorHandle*/
        static int ShmAttach(int iCreateID,char* &sBuffer,int *pShmIDorHandle);

        /*连接iShmIDorHandle对应的共享内存,设置sBuffer为连接后的指针值*/
        static int ShmAttach(int iShmIDorHandle,char* &sBuffer);

        /*断开*psBuffer对应的共享内存连接*/
        static int ShmDetach(char *sBuffer);

        /*删除IDorHandle对应的共享内存*/
        static int ShmDestroy(int iIDorHandle);
    
        static int GetErrorCode();
        
        static const char* GetErrMsgByCode(int iErrCode,int iDetailCode=0);

    private:
        /*备注:对象私有变量,一个对象只能针对一个内存映射*/
/*
        char    m_sShmID[MAX_SHM_ID_SIZE];                  //共享内存标识ID字符串形式
        int     m_iShmID;                                   //共享内存标识ID
        void    *m_pHandle;                                 //_WIN32下共享内存句柄
        char    *m_sBuffer;                                 //共享内存连接后的字符指针
*/        
    /*private类变量*/
        static int iErrCode;                                //系统错误编码
};

class TShareMemInfo {
public:
    unsigned long Size;
    int iCurrMemTableKey;
public:
    TShareMemInfo(void);
    void InitMemData(size_t ASize, int ACurrMemTableKey=-1);
    unsigned char * GetMemData(void);
    int * GetCurrMemTableKey();
};

class TSharedMem : public TBaseObject {
public:
    TSharedMem(void);
    virtual ~TSharedMem(void);
    static void SetKeyName(const char *AName,char *OKeyName);
public:
    unsigned char *  Create(const char *AMemName,size_t ASize);
    unsigned char *  Attach(const char *AMemName);
    void Detach(void);
    void Destroy(void);
public:
    long GetSize(void);
    unsigned char * GetBuffer(void);
    int * GetCurrMemTableKey();
#ifdef _WIN32
    HANDLE GetHanedle(void);
#else
    int GetHanedle(void);
#endif    
private:
    TShareMemInfo *FMemInfo;
    char FShareMemName[255];
#ifdef _WIN32
    HANDLE FHandle;
#else
    int FHandle;
#endif
    void *FFileView;
    int FLastError;
};


#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif  //_TSHM_H_
