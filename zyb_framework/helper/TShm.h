//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/*TShm.h*/
/*�����ڴ�����ײ㺯����װ����*/
#ifndef _TSHM_H_
#define _TSHM_H_

/*��ע:_WIN32�¿��ǽ�HANDLEת����long�ͺ���ת����int��*/

#ifdef  _WIN32

#pragma warning(disable:4312)           //unsigned int ����int ת����HANDLE
#pragma warning(disable:4311)           //void*ת����long

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

#define STR_FORMAT_SHM_ID               "SHM%09d"           //WIN32���������ɱ�ʶ����
#define SHM_ID_MOD                      10000000000         //�������ɱ�ʶȡģ
#define MAX_SHM_ID_SIZE                 64                  //SHM_ID�ַ�����ʽ�ĳ���
#define SHM_FTOK_PATH_ENV               "CCB_INSTANCE_HOME" //UNIX������fotk()�����Ļ�������·��
#define SHM_FTOK_PATH                   "/"                 //UNIX������ftok()����
#define MEM_TAB_IPC_KEY                 0x1119
#define SHARE_TABLE_NEW                 3721
#define SHARE_TABLE_SGA                 "_TAB_SGA"

#define SHM_BASE                        100                 //�����ڴ������Ϣ��ʼ����

#define ERROR_SHM_PARAMETER             SHM_BASE+1          //��������
#define ERROR_SHM_CREATE                SHM_BASE+2          //�����ڴ洴��ʧ��
#define ERROR_SHM_ATTACH                SHM_BASE+3          //�����ڴ�����ʧ��
#define ERROR_SHM_DETACH                SHM_BASE+4          //�����ڴ�Ͽ�ʧ��
#define ERROR_SHM_DESTROY               SHM_BASE+5          //�����ڴ�ɾ��ʧ��
#define ERROR_SHM_EXIST                 SHM_BASE+6          //�����ڴ��Ѿ�����
#define ERROR_SHM_NOT_EXIST             SHM_BASE+7          //�����ڴ治����
#define ERROR_SHM_FTOK                  SHM_BASE+8          //FTOKʧ��

class TShm
{
    public:
        TShm();
        virtual ~TShm();

    /*public�ຯ��*/
        /*����iCreateID,iSize���ɹ����ڴ�,���ؾ����*pShmIDorHandle*/
        static int ShmCreate(int iCreateID,size_t iSize,int *pShmIDorHandle, bool bIsExcel=true);

        /*����iCreateID��Ӧ�Ĺ����ڴ�,����sBufferΪ���Ӻ��ָ��ֵ,���ع����ڴ��ʶ���߾��*pShmIDorHandle*/
        static int ShmAttach(int iCreateID,char* &sBuffer,int *pShmIDorHandle);

        /*����iShmIDorHandle��Ӧ�Ĺ����ڴ�,����sBufferΪ���Ӻ��ָ��ֵ*/
        static int ShmAttach(int iShmIDorHandle,char* &sBuffer);

        /*�Ͽ�*psBuffer��Ӧ�Ĺ����ڴ�����*/
        static int ShmDetach(char *sBuffer);

        /*ɾ��IDorHandle��Ӧ�Ĺ����ڴ�*/
        static int ShmDestroy(int iIDorHandle);
    
        static int GetErrorCode();
        
        static const char* GetErrMsgByCode(int iErrCode,int iDetailCode=0);

    private:
        /*��ע:����˽�б���,һ������ֻ�����һ���ڴ�ӳ��*/
/*
        char    m_sShmID[MAX_SHM_ID_SIZE];                  //�����ڴ��ʶID�ַ�����ʽ
        int     m_iShmID;                                   //�����ڴ��ʶID
        void    *m_pHandle;                                 //_WIN32�¹����ڴ���
        char    *m_sBuffer;                                 //�����ڴ����Ӻ���ַ�ָ��
*/        
    /*private�����*/
        static int iErrCode;                                //ϵͳ�������
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
