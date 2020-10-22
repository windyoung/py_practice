//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/*TSem.h*/
/*�ź��������ײ㺯����װ����*/
#ifndef _TSEM_H_
#define _TSEM_H_


/*��ע:_WIN32�¿��ǽ�HANDLEת����long�ͺ���ת����int��*/

#ifdef  _WIN32

#pragma warning(disable:4312)           //unsigned int ����int ת����HANDLE
#pragma warning(disable:4311)           //void*ת����long

#include <windows.h>

#else   //_UNIX

#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>

/*�ź���������Ϣ�ṹ*/
union semun
{
    int val;
    struct semid_ds *buf;
    ushort *array;
};

/*��ֹarg��ͻ*/
/*
} arg;
*/

#endif  //_WIN32 _UNIX

#define STR_FORMAT_SEM_ID               "SEM%09d"           //WIN32���������ɱ�ʶ����
#define SEM_ID_MOD                      10000000000         //�������ɱ�ʶȡģ
#define MAX_SEM_ID_SIZE                 64                  //SEM_ID�ַ�����ʽ�ĳ���
#define SEM_FTOK_PATH_ENV               "CCB_INSTANCE_HOME" //UNIX������fotk()�����Ļ�������·��
#define SEM_FTOK_PATH                   "/"                 //UNIX������ftok()����
#define MAX_SEM_ERROR_MSG_SIZE          1024                //�ź����������Ϣ����󳤶�

#define SEM_BASE                        200                 //�ź���������Ϣ��ʼ����

#define ERROR_SEM_PARAMETER             SEM_BASE+1          //��������
#define ERROR_SEM_CREATE                SEM_BASE+2          //�ź�������ʧ��
#define ERROR_SEM_OPEN                  SEM_BASE+3          //�ź�����ȡ�źű�ʶ����ʧ��
#define ERROR_SEM_SET_VALUE             SEM_BASE+4          //�ź��������ź�ֵʧ��
#define ERROR_SEM_P_OPERATION           SEM_BASE+5          //�ź���P����ʧ��
#define ERROR_SEM_V_OPERATION           SEM_BASE+6          //�ź���V����ʧ��
#define ERROR_SEM_DESTROY               SEM_BASE+7          //�ź���ɾ��ʧ��
#define ERROR_SEM_EXIST                 SEM_BASE+8          //�ź����Ѿ�����
#define ERROR_SEM_FTOK                  SEM_BASE+9          //FTOKʧ��
#define ERROR_SEM_GETVAL                SEM_BASE+10         //�ź�����ȡֵʧ��

class TSem
{
    public:
        TSem();
        virtual ~TSem();

    /*public�ຯ��*/
        /*����iCreateID�����ź���,�����ź���ΪiValue,���ؾ����*pSemIDorHandle*/
        static int SemCreate(int iCreateID,int iValue,int *pSemIDorHandle);
        
        /*����iCreateID��ȡ�ź�����ʶ���߾��*pSemIDorHandle*/
        static int SemOpen(int iCreateID,int *pSemIDorHandle);

        /*ɾ��iSemIDorHandle��Ӧ���ź���*/
        static int SemDestroy(int iSemIDorHandle);
        
        /*�ź���P����,�����ź���ֵabs(iSemValue)*/
        static int SemP(int iSemIDorHandle,int iSemValue=1);

        /*�ź���V����,�����ź���ֵabs(iSemValue)*/
        static int SemV(int iSemIDorHandle,int iSemValue=1);

        //�鿴ָ���ź�����ֵ
           static int SemGetValue(int iSemIDorHandle,int &iValue);

        static int GetErrorCode();

        static const char* GetErrMsgByCode(int iErrCode,int iDetailCode=0);
    
    private:
        static int iErrCode;                                //ϵͳ�������
};

#endif  //_TSEM_H_

