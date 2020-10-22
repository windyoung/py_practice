//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/*TSem.cpp*/
/*信号量操作底层函数封装定义*/
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "TSem.h"
#include "TStrFunc.h"
#include "CommonDef.h"
#include "TFileOper.h"
#include "debug_new.h"

using namespace std;

//ZMP:467374    
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

int TSem::iErrCode=0;

/*public类函数*/
/*根据iCreateID生成信号量,设置信号量为iValue,返回句柄到*pSemIDorHandle*/
int TSem::SemCreate(int iCreateID,int iValue,int *pSemIDorHandle )
{
    int iRet=0;
    int iSemID=-1;
    TSem::iErrCode=0;
	
	#ifdef _ASSERT_PARAM
		if( NULL==pSemIDorHandle )
		{
			return ERROR_SEM_PARAMETER;
		}
	#endif

    if((iCreateID<=0)||(iValue<=0))
    {
        return ERROR_SEM_PARAMETER;
    }
    iSemID=iCreateID;

#ifdef  _WIN32

    void    *pHandle=NULL;
    char    sSemID[MAX_SEM_ID_SIZE]={0}; //357132 for cppcheck
    //ZMP:467374
    snprintf(sSemID, sizeof(sSemID), STR_FORMAT_SEM_ID,iSemID%SEM_ID_MOD);
    sSemID[sizeof(sSemID)-1]='\0';

    /*检测信号是否存在*/
    pHandle=OpenSemaphore(SEMAPHORE_ALL_ACCESS,FALSE,sSemID);
    if(pHandle!=NULL)
    {//存在则报错
        CloseHandle(pHandle);
        pHandle=NULL;
        //iSemID=-1; //351951 for cppcheck
        iRet=ERROR_SEM_EXIST;
    }
    else
    {//不存在则生成
         pHandle=CreateSemaphore(NULL,iValue,iValue,sSemID);
         if(pHandle==NULL)
         {
            //iSemID=-1; //351951 for cppcheck
            iRet=ERROR_SEM_CREATE;
         }
    }
    
    /*备注:_WIN32下sizeof(void*)=sizeof(int)*/
    *pSemIDorHandle=((int)pHandle);

    iErrCode=GetLastError();
    

#else   //_UNIX

    key_t   key=-1;
    char    *sEnv=NULL;
    sEnv=getenv(SEM_FTOK_PATH_ENV);
    if(sEnv!=NULL)
    {
        errno=0;
        key=TFileOper::Ftok(sEnv,iSemID);
        if(errno!=0)
        {
            return ERROR_SEM_FTOK;
        }
    }
    else
    {
        key=TFileOper::Ftok(SEM_FTOK_PATH,iSemID);
    }
    
    iSemID=semget(key,1,0666|IPC_CREAT|IPC_EXCL);
    if(iSemID<0)
    {
        iRet=ERROR_SEM_CREATE;
    }
    else
    {//创建成功设置初始值
        union semun semopts;
        semopts.val=iValue;    
        iRet=semctl(iSemID,0,SETVAL,semopts);
        if(iRet!=0)
        {//设置值不成功则删除信号量并报错
            SemDestroy(iSemID);
            iSemID=-1;
            iRet=ERROR_SEM_SET_VALUE;
        }
    }

    *pSemIDorHandle=iSemID;
    iErrCode=errno;

#endif  // _WIN32 _UNIX

    return iRet;
}

/*根据iCreateID获取信号量标识或者句柄*pSemIDorHandle*/
int TSem::SemOpen(int iCreateID,int *pSemIDorHandle)
{
    int iRet=0;
    int iSemID=-1;
    TSem::iErrCode=0;

	#ifdef _ASSERT_PARAM
		if( NULL==pSemIDorHandle )
		{
			return ERROR_SEM_PARAMETER;
		}
	#endif	
	
	if(iCreateID<=0)
    {
        return ERROR_SEM_PARAMETER;
    }
    iSemID=iCreateID;

#ifdef  _WIN32

    void    *pHandle=NULL;
    char    sSemID[MAX_SEM_ID_SIZE]={0}; //357132 for cppcheck
    //ZMP:467374
    snprintf(sSemID, sizeof(sSemID), STR_FORMAT_SEM_ID,iSemID%SEM_ID_MOD);
    sSemID[sizeof(sSemID)-1]='\0';
    
    /*检测信号是否存在*/
    pHandle=OpenSemaphore(SEMAPHORE_ALL_ACCESS,FALSE,sSemID);
    if(pHandle==NULL)
    {//不存在则报错
        //iSemID=-1; // 351951 for cppcheck 
        iRet=ERROR_SEM_OPEN;
    }
    
    /*备注:_WIN32下sizeof(void*)=sizeof(int)*/
    *pSemIDorHandle=(int)pHandle;
    iErrCode=GetLastError();

#else   //_UNIX

    key_t   key=-1;
    char    *sEnv=NULL;
    sEnv=getenv(SEM_FTOK_PATH_ENV);
    if(sEnv!=NULL)
    {
        errno=0;
        key=TFileOper::Ftok(sEnv,iSemID);
        if(errno!=0)
        {
            return ERROR_SEM_FTOK;
        }
    }
    else
    {
        key=TFileOper::Ftok(SEM_FTOK_PATH,iSemID);
    }
    
    iSemID=semget(key,1,0666);
    if(iSemID<0)
    {
        iRet=ERROR_SEM_OPEN;
    }

    *pSemIDorHandle=iSemID;
    iErrCode=errno;

#endif  // _WIN32 _UNIX

    return iRet;
}

/*删除iSemIDorHandle对应的信号量*/
int TSem::SemDestroy(int iSemIDorHandle)
{
    int iRet=0;
    TSem::iErrCode=0;
    
    if(iSemIDorHandle<=0)
    {
        return ERROR_SEM_PARAMETER;
    }

#ifdef  _WIN32

    if(CloseHandle((HANDLE)iSemIDorHandle)==FALSE)
    {
        iRet=ERROR_SEM_DESTROY;
    }
    iErrCode=GetLastError();

#else   //_UNIX
   
    iRet=semctl(iSemIDorHandle,0,IPC_RMID,0);
    if(iRet!=0)
    {
        iRet=ERROR_SEM_DESTROY;
    }
    iErrCode=errno;

#endif  // _WIN32 _UNIX

    return iRet;
}

/*信号量P操作,减少信号量值ABS(iSemValue)*/
int TSem::SemP(int iSemIDorHandle,int iSemValue)
{
    int iRet=0;
    TSem::iErrCode=0;
    
    if((iSemIDorHandle<=0)
        ||(iSemValue==0)
        )
    {
        return ERROR_SEM_PARAMETER;
    }

#ifdef  _WIN32

    int iCount=0;
    for(iCount=0;iCount<ABS(iSemValue);iCount++)
    {
        iRet=WaitForSingleObject((HANDLE)iSemIDorHandle,1000*60*60);//等待信号量毫秒数
        if(iRet!=0)
        {
            iErrCode=GetLastError();

            if(iCount>0)
            {//释放已经占用的信号量
                SemV(iSemIDorHandle,iCount);
            }
            iRet=ERROR_SEM_P_OPERATION;
            break;
        }
    }

#else   //_UNIX

    struct sembuf buf_p;
    
    buf_p.sem_num=0;
    buf_p.sem_op=(short)(-ABS(iSemValue));   //减少信号量的值
    buf_p.sem_flg=SEM_UNDO;
    
    iRet=semop(iSemIDorHandle,&buf_p,1);
    if(iRet!=0)
    {
        iRet=ERROR_SEM_P_OPERATION;
    }
    iErrCode=errno;

#endif  // _WIN32 _UNIX

    return iRet;
}

/*信号量V操作,增加信号量值ABS(iSemValue)*/
int TSem::SemV(int iSemIDorHandle,int iSemValue)
{
    int iRet=0;
    TSem::iErrCode=0;

    if((iSemIDorHandle<=0)
        ||(iSemValue==0)
      )
    {
        return ERROR_SEM_PARAMETER;
    }

#ifdef  _WIN32

    if(ReleaseSemaphore((HANDLE)iSemIDorHandle,ABS(iSemValue),NULL)==FALSE)
    {
        iRet=ERROR_SEM_V_OPERATION;
    }
    iErrCode=GetLastError();

#else   //_UNIX

    struct sembuf buf_p;
    
    buf_p.sem_num=0;
    buf_p.sem_op=(short)ABS(iSemValue);//增加信号量的值 ZMP:477242
    buf_p.sem_flg=SEM_UNDO;
    
    iRet=semop(iSemIDorHandle,&buf_p,1);
    if(iRet!=0)
    {
        iRet=ERROR_SEM_V_OPERATION;
    }
    iErrCode=errno;

#endif  // _WIN32 _UNIX

    return iRet;
}

int TSem::GetErrorCode()
{
    return TSem::iErrCode;
}

const char* TSem::GetErrMsgByCode(int iErrorCode,int iDetailCode)
{
    static string  strErrMsg="";

    strErrMsg="TSem::ErrMsg=";

    switch (iErrorCode)
    {
        case ERROR_SEM_PARAMETER:
        {
            strErrMsg=strErrMsg+"Sem Input parameter error";
            break;
        }
        case ERROR_SEM_CREATE:
        {
            strErrMsg=strErrMsg+"Sem Create action error";
            break;
        }
        case ERROR_SEM_OPEN:
        {
            strErrMsg=strErrMsg+"Sem Open action error";
            break;
        }
        case ERROR_SEM_SET_VALUE:
        {
            strErrMsg=strErrMsg+"Sem Set value action error";
            break;
        }
        case ERROR_SEM_DESTROY:
        {
            strErrMsg=strErrMsg+"Sem Destroy action error";
            break;
        }
        case ERROR_SEM_P_OPERATION:
        {
            strErrMsg=strErrMsg+"Sem P action error";
            break;
        }
        case ERROR_SEM_V_OPERATION:
        {
            strErrMsg=strErrMsg+"Sem V action error";
            break;
        }
        case ERROR_SEM_EXIST:
        {
            strErrMsg=strErrMsg+"Sem has existed";
            break;
        }
        case ERROR_SEM_FTOK:
        {
            strErrMsg=strErrMsg+"Unix ftok() error";
            break;
        }
        case ERROR_SEM_GETVAL:
        {
            strErrMsg=strErrMsg+"Unix semctl(GETVAL) error";
            break;
        }
        default:
        {
            strErrMsg="";
            break;
        }
    }//end of swtich(iErrCode)

    if(iDetailCode!=0)
    {
#ifdef  _WIN32
#else
        strErrMsg=strErrMsg+"(DetailCode="+FUNC_STR::IntToChar(iDetailCode)+
                    ",DetailMsg="+strerror(iDetailCode)+")";
#endif  //_UNIX
    }

    return strErrMsg.c_str();

}

/*public函数*/
TSem::TSem()
{
}

TSem::~TSem()
{
}


//查看指定信号量的值
int TSem::SemGetValue(int iSemIDorHandle,int &iValue)
{
    int iRet=0;
    TSem::iErrCode=0;
    iValue=-1;

    if(iSemIDorHandle<=0)
    {
        return ERROR_SEM_PARAMETER;
    }

#ifdef  _WIN32
#else    //UNIX
    errno=0;
    union semun semopts;
    semopts.val=0;
    iRet=semctl(iSemIDorHandle,0,GETVAL,semopts);
    
    if(iRet<0)
    {
        iRet=ERROR_SEM_GETVAL;
    }
    else
    {
        iValue=iRet;
        iRet=0;
    }

    iErrCode=errno;
#endif //UNIX

    return iRet;
}

