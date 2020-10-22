//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/*TShm.cpp*/
/*共享内存操作底层函数封装定义*/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include "TShm.h"
#include "TStrFunc.h"
#include "CommonDef.h"
#include "TFileOper.h"
#include "debug_new.h"

using namespace std;

//ZMP:467374    
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

int TShm::iErrCode=0;

/*public类函数*/
/*创建共享内存*/
/*
iCreateID为共享内存创建标识,必须>0
iSize为共享内存大小,必须>0
pShmIDorHandle为句柄,或者为指向返回内存ID的指针
函数返回0表示成功,非0表示失败
*/
int TShm::ShmCreate(int iCreateID,size_t iSize,int *pShmIDorHandle, bool bIsExcel)
{
    int iRet=0;
    int iShmID=-1;

    TShm::iErrCode=0;

	#ifdef _ASSERT_PARAM
		if( NULL==pShmIDorHandle )
		{
			return ERROR_SHM_PARAMETER;
		}
	#endif

    //modify by shi.hongsong for zmp[747732]BF TShm::ShmCreate第二个参数类型错误 
    //if((iCreateID<=0)||(iSize<=0))
    if((iCreateID<=0)||(iSize<=(size_t)0))
    {
        return ERROR_SHM_PARAMETER;
    }
    iShmID=iCreateID;

#ifdef  _WIN32

    void    *pHandle=NULL;
    char    sShmID[MAX_SHM_ID_SIZE]={0}; //357133 for cppcheck
    //ZMP:467374
    snprintf(sShmID, sizeof(sShmID), STR_FORMAT_SHM_ID,iShmID%SHM_ID_MOD);
    sShmID[sizeof(sShmID)-1]='\0';

    /*检测共享内存是否存在*/
    pHandle=OpenFileMapping(FILE_MAP_READ,FALSE,sShmID);
    if(pHandle!=NULL)
    {//存在则报错
        CloseHandle(pHandle);
        pHandle=NULL;
        //iShmID=-1; //351951 for cppcheck
        iRet=ERROR_SHM_EXIST;
    }
    else
    {//不存在则生成
        pHandle=CreateFileMapping((HANDLE)0xFFFFFFFF,//-1(INVALID_HANDLE_VALUE)
            NULL,PAGE_READWRITE,0x00,iSize,sShmID);
        if(pHandle==NULL)
        {
            //iShmID=-1; //351951 for cppcheck
            iRet=ERROR_SHM_CREATE;
        }
    }

    /*备注:_WIN32下sizeof(void*)=sizeof(int)*/
    *pShmIDorHandle=(int)pHandle;

    iErrCode=GetLastError();


#else   //_UNIX
    errno=0;
    key_t   key=-1;
    char    *sEnv=NULL;
    sEnv=getenv(SHM_FTOK_PATH_ENV);
    if(sEnv!=NULL)
    {
        errno=0;
        key=TFileOper::Ftok(sEnv,iShmID);
        if(errno!=0)
        {
            iErrCode=errno;
            return ERROR_SHM_FTOK;
        }
    }
    else
    {    
        errno=0;
        key=TFileOper::Ftok(SHM_FTOK_PATH,iShmID);
        if(errno!=0)
        {
            iErrCode=errno;
            return ERROR_SHM_FTOK;
        }
    }
	
	if(bIsExcel)
	{
    	iShmID=shmget(key, iSize,0666|IPC_CREAT|IPC_EXCL);
    }
    else
    {
    	iShmID=shmget(key, iSize,0666|IPC_CREAT);
    }
    
    if(iShmID<0)
    {
        iRet=ERROR_SHM_CREATE;
    }

    *pShmIDorHandle=iShmID;
    iErrCode=errno;

#endif  // _WIN32 _UNIX

    return iRet;
}

/*建立共享内存连接*/
/*
iCreateID为共享内存创建标识
sBuffer为连接后内存的指针
pShmIDorHandle为返回的共享内存标识或者句柄的地址
函数返回0表示成功,非0表示失败
*/
int TShm::ShmAttach(int iCreateID,char* &sBuffer,int *pShmIDorHandle)
{
    int iRet=0;
    int iShmID=-1;
    char *sBufferTemp=NULL;
    TShm::iErrCode=0;

    if((iCreateID<=0)
        ||(sBuffer!=NULL)
        ||(pShmIDorHandle==NULL)
        )
    {
        return ERROR_SHM_PARAMETER;
    }
    iShmID=iCreateID;

#ifdef  _WIN32

    void    *pHandle=NULL;
    char    sShmID[MAX_SHM_ID_SIZE]={0}; //357133 for cppcheck
    //ZMP:467374 
    snprintf(sShmID, sizeof(sShmID), STR_FORMAT_SHM_ID,iShmID%SHM_ID_MOD);
    sShmID[sizeof(sShmID)-1]='\0';

    /*检测共享内存是否存在*/
    pHandle=OpenFileMapping(FILE_MAP_WRITE,FALSE,sShmID);
    if(pHandle==NULL)
    {//不存在则报错
        iRet=ERROR_SHM_NOT_EXIST;
    }
    else
    {
        sBufferTemp=(char*)MapViewOfFile(pHandle,FILE_MAP_WRITE,0,0,0);
        if(sBufferTemp==NULL)
        {
            iRet=ERROR_SHM_ATTACH;
        }
    }

    (*pShmIDorHandle)=(int)pHandle;

    iErrCode=GetLastError();

#else   //_UNIX

    errno=0;
    key_t   key=-1;
    char    *sEnv=NULL;
    sEnv=getenv(SHM_FTOK_PATH_ENV);
    if(sEnv!=NULL)
    {
        errno=0;
        key=TFileOper::Ftok(sEnv,iShmID);
        if(errno!=0)
        {
            iErrCode=errno;
            return ERROR_SHM_FTOK;
        }
    }
    else
    {
        errno=0;
        key=TFileOper::Ftok(SHM_FTOK_PATH,iShmID);
        if(errno!=0)
        {
            iErrCode=errno;
            return ERROR_SHM_FTOK;
        }
    }
    iShmID=-1;

    /*获取共享内存标识ID*/
    iShmID=shmget(key,0,0666);
    if(iShmID<0)
    {//不存在则报错
        iRet=ERROR_SHM_NOT_EXIST;
    }
    else
    {
        #ifdef _SUN
            sBufferTemp=(char*)shmat(iShmID,0,SHM_SHARE_MMU);
        #else
            sBufferTemp=(char*)shmat(iShmID,0,0);
        #endif
        if(sBufferTemp==(char*)-1)
        {
            iRet=ERROR_SHM_ATTACH;
        }
    }

    *pShmIDorHandle=iShmID;
    iErrCode=errno;

#endif  // _WIN32 _UNIX

    sBuffer=sBufferTemp;

    return iRet;
}

/*建立共享内存连接*/
/*
iShmIDorHandle为共享内存标识指针
sBuffer为连接后内存的指针
函数返回0表示成功,非0表示失败
*/
int TShm::ShmAttach(int iShmIDorHandle,char* &sBuffer)
{
    int iRet=0;
    char *sBufferTemp=NULL;
    TShm::iErrCode=0;

    if((iShmIDorHandle<=0)
        ||(sBuffer!=NULL)
        )
    {
        return ERROR_SHM_PARAMETER;
    }

#ifdef  _WIN32

    sBufferTemp=(char*)MapViewOfFile((HANDLE)(iShmIDorHandle),FILE_MAP_WRITE,0,0,0);
    if(sBufferTemp==NULL)
    {
        iRet=ERROR_SHM_ATTACH;
    }

    iErrCode=GetLastError();

#else   //_UNIX

    errno=0;
    sBufferTemp=(char*)shmat(iShmIDorHandle,0,0);
    if(sBufferTemp==(char*)-1)
    {
        iRet=ERROR_SHM_ATTACH;
    }

    iErrCode=errno;

#endif  // _WIN32 _UNIX

    sBuffer=sBufferTemp;

    return iRet;
}

/*断开共享内存连接*/
/*
sBuffer为指向共享内存的指针
函数返回0表示成功,非0表示失败
*/
int TShm::ShmDetach(char *sBuffer)
{
    int iRet=0;
    TShm::iErrCode=0;

    if(sBuffer==NULL)
    {
        return ERROR_SHM_PARAMETER;
    }

#ifdef  _WIN32

    if(UnmapViewOfFile((void*)(sBuffer))==FALSE)
    {/*释放映射失败*/
        iRet=ERROR_SHM_DETACH;
    }

    iErrCode=GetLastError();

#else   //_UNIX

    errno=0;
    iRet=shmdt(sBuffer);
    if(iRet!=0)
    {
        iRet=ERROR_SHM_DETACH;
    }

    iErrCode=errno;

#endif  // _WIN32 _UNIX

    return iRet;
}

/*释放共享内存*/
/*

函数返回0表示成功,非0表示失败
*/
int TShm::ShmDestroy(int iIDorHandle)
{
    int iRet=0;
    TShm::iErrCode=0;

    if(iIDorHandle<=0)
    {
        return ERROR_SHM_PARAMETER;
    }

#ifdef  _WIN32

    if(CloseHandle((HANDLE)iIDorHandle)==FALSE)
    {
        iRet=ERROR_SHM_DESTROY;
    }
    iErrCode=GetLastError();

#else   //_UNIX

    errno=0;
    iRet=shmctl(iIDorHandle,IPC_RMID,0);
    if(iRet!=0)
    {
        iRet=ERROR_SHM_DESTROY;
    }
    iErrCode=errno;

#endif  // _WIN32 _UNIX

    return iRet;
}


int TShm::GetErrorCode()
{
    return TShm::iErrCode;
}

const char* TShm::GetErrMsgByCode(int iErrorCode,int iDetailCode)
{
    static string strErrMsg="";

    strErrMsg="TShm Message : ";

    switch (iErrorCode)
    {
    case ERROR_SHM_PARAMETER:
        {
            strErrMsg=strErrMsg+"Shm Input parameter error";
            break;
        }
    case ERROR_SHM_CREATE:
        {
            strErrMsg=strErrMsg+"Shm Create action error";
            break;
        }
    case ERROR_SHM_ATTACH:
        {
            strErrMsg=strErrMsg+"Shm Attach action error";
            break;
        }
    case ERROR_SHM_DETACH:
        {
            strErrMsg=strErrMsg+"Shm Detach action error";
            break;
        }
    case ERROR_SHM_DESTROY:
        {
            strErrMsg=strErrMsg+"Shm Destroy action error";
            break;
        }
    case ERROR_SHM_EXIST:
        {
            strErrMsg=strErrMsg+"Shm has existed";
            break;
        }
    case ERROR_SHM_NOT_EXIST:
        {
            strErrMsg=strErrMsg+"Shm not exist";
            break;
        }
    case ERROR_SHM_FTOK:
        {
            strErrMsg=strErrMsg+"Unix ftok() error";
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
        strErrMsg=strErrMsg+"(Code="+FUNC_STR::IntToChar(iDetailCode)+", Message="+strerror(iDetailCode)+")";

#endif  //_UNIX
    }

    return strErrMsg.c_str();
}


/*public函数*/
TShm::TShm()
{
}

TShm::~TShm()
{
}

//////////////////////////////////////////////////////////////////////////

TShareMemInfo::TShareMemInfo(void)
{
    Size = 0;
    iCurrMemTableKey = -1;
}

void TShareMemInfo::InitMemData(size_t ASize,int ACurrMemTableKey)
{
    Size = ASize;
    iCurrMemTableKey = ACurrMemTableKey;
}

unsigned char * TShareMemInfo::GetMemData(void)
{
    return (unsigned char *)this + sizeof(Size) + sizeof(iCurrMemTableKey);
}

int * TShareMemInfo::GetCurrMemTableKey()
{
    return &(this->iCurrMemTableKey);
}

TSharedMem::TSharedMem(void)
{
#ifdef _WIN32
    FHandle = NULL;
#else
    FHandle = -1;
#endif
    FMemInfo = NULL;
    //ZMP:305593
    memset(FShareMemName, 0, sizeof(FShareMemName));
    FFileView = NULL;
    FLastError = 0;

}

TSharedMem::~TSharedMem(void)
{

}

unsigned char *  TSharedMem::Create(const char *AMemName,size_t ASize)
{
	#ifdef _ASSERT_PARAM  
	if( ASize < 1 ) //判断参数的合法性
	{
		throw TException("Parameter ASize is wrong! ASize value=%zd", ASize);
	}
	#endif
	//printf("TSharedMem::Create(MemName=[%s],Size=%d)\n",AMemName,ASize);

    strncpy(FShareMemName,AMemName,sizeof(FShareMemName)-1);
    FShareMemName[sizeof(FShareMemName)-1]='\0';

#ifdef  _WIN32
    FHandle=OpenFileMapping(FILE_MAP_READ,false,FShareMemName);
    if(FHandle!=NULL)
    {
        throw TException("TSharedMem::Create() : Already Create Share Memory. Name=[%s],ErrorNo=[%d]",FShareMemName,GetLastError());
    }
    else
    {
        //ZMP:394011 
        SECURITY_ATTRIBUTES SecAttr;
        SECURITY_DESCRIPTOR SecDesc;
        if (InitializeSecurityDescriptor(&SecDesc, SECURITY_DESCRIPTOR_REVISION))
        {
            if (SetSecurityDescriptorDacl(&SecDesc, TRUE, (PACL)0,FALSE))
            {
                SecAttr.nLength = sizeof(SecAttr);
                SecAttr.lpSecurityDescriptor = &SecDesc;
                SecAttr.bInheritHandle = TRUE;
            }
            else
            {
                throw TException("TSharedMem:: Create():SetSecurityDescriptorDacl error ! Name = [%s],ErrorNo = [%d].\n", FShareMemName,GetLastError());
            }
        } 
        else 
        {
            throw TException("TSharedMem:: Create():InitializeSecurityDescriptor error !Name = [%s],ErrorNo = [%d].\n",FShareMemName,GetLastError());
        }
        FHandle = CreateFileMapping(INVALID_HANDLE_VALUE,&SecAttr,PAGE_READWRITE,0,ASize + sizeof(unsigned long) + sizeof(int),FShareMemName);      
        if(FHandle==NULL)
        {
            throw TException("TSharedMem::Create() : CreateFileMapping(Name=%s) RETURN=[NULL],ErrorNo=[%d]",FShareMemName,GetLastError());
        }

        FFileView = MapViewOfFile(FHandle,FILE_MAP_WRITE,0,0,0);   
        if(FFileView==NULL)
        {
            throw TException("TSharedMem::Create() : MapViewOfFile(Name=%s) RETURN=[NULL],ErrorNo=[%d]",FShareMemName,GetLastError());
        }

        FMemInfo = (TShareMemInfo *)FFileView; 
        FMemInfo->InitMemData(ASize);
    }   
#else  
    key_t iNameKey = ftok(FShareMemName,MEM_TAB_IPC_KEY);

    FHandle=shmget(iNameKey,0,0666);
    if(FHandle>=0)
    {
        throw TException("TSharedMem::Create() : Already Create Share Memory. Name=[%s] Key=[%d]",FShareMemName,FHandle);
    }
    else
    {
        FHandle=shmget(iNameKey, ASize + sizeof(unsigned long) + sizeof(int),0666|IPC_CREAT|IPC_EXCL);

        if(FHandle<0)
        {
            throw TException("TSharedMem::Create() : shmget(Name=%s,Key=%d) RETURN=[NULL]",FShareMemName,FHandle);
        }

        FFileView=shmat(FHandle,0,0);
        if(FFileView==(char*)-1)
        {
            FMemInfo = NULL;
            FFileView = NULL;
            shmctl(FHandle,IPC_RMID,0);
            FHandle = -1;
            throw TException("TSharedMem::Create() : shmat(Name=%s,Key=%d) RETURN=[NULL]",FShareMemName,FHandle);
        }

        FMemInfo = (TShareMemInfo *)FFileView; 
        FMemInfo->InitMemData(ASize);
    }
#endif
    return FMemInfo->GetMemData();
}

unsigned char *  TSharedMem::Attach(const char *AMemName)
{
    //printf("TSharedMem::Attach(MemName=[%s])\n",AMemName);

    strncpy(FShareMemName,AMemName,sizeof(FShareMemName)-1);    
    FShareMemName[sizeof(FShareMemName)-1]='\0';

#ifdef  _WIN32
    FHandle=OpenFileMapping(FILE_MAP_WRITE,false,FShareMemName);

    if(FHandle!=NULL)
    {
        FFileView = MapViewOfFile(FHandle,FILE_MAP_WRITE,0,0,0);   
        if(FFileView==NULL)
        {
            throw TException("TSharedMem::Attach() : MapViewOfFile(Name=%s) RETURN=[NULL],ErrorNo=[%d]",FShareMemName,GetLastError());
        }
        FMemInfo = (TShareMemInfo *)FFileView; 
    }
    else
    {
        return NULL;
    }   
#else  
    int iNameKey = ftok(FShareMemName,MEM_TAB_IPC_KEY);

    FHandle=shmget(iNameKey,0,0666);

    if(FHandle>=0)
    {
        FFileView=shmat(FHandle,0,0);
        if(FFileView==(char*)-1)
        {
            FMemInfo = NULL;
            FFileView = NULL;
            throw TException("TSharedMem::Attach() : shmat(Name=%s,Key=%d) RETURN=[NULL]",FShareMemName,FHandle);
        }
        FMemInfo = (TShareMemInfo *)FFileView; 
    }
    else
    {
        return NULL;
    }
#endif
    return FMemInfo->GetMemData();
}

void TSharedMem::Detach(void)
{
    //printf("TSharedMem::Detach(MemName=[%s])\n",FShareMemName);
#ifdef  _WIN32
    if(FFileView!=NULL)
    {
        UnmapViewOfFile(FFileView);
        FLastError=GetLastError();
        FFileView = NULL;
        FMemInfo = NULL;
    }
#else  
    if(FFileView!=NULL)
    {
#ifndef _SUN
        shmdt(FFileView);
#else
        shmdt((char*)FFileView);
#endif
        FLastError=errno;
        FFileView = NULL;
        FMemInfo = NULL;
    }
#endif 
}

void TSharedMem::Destroy(void)
{
    //printf("TSharedMem::Destroy(MemName=[%s])\n",FShareMemName);
#ifdef  _WIN32
    if(FFileView!=NULL)
    {
        UnmapViewOfFile(FFileView);
        FLastError=GetLastError();
        FFileView = NULL;
        FMemInfo = NULL;
    }

    if(FHandle!=NULL)
    {
        CloseHandle(FHandle);
        FLastError=GetLastError();
        FHandle = NULL;
    }
#else  
    if(FFileView!=NULL)
    {
#ifndef _SUN
        shmdt(FFileView);
#else
        shmdt((char*)FFileView);
#endif
        FLastError=errno;
        FFileView = NULL;
        FMemInfo = NULL;
    }

    if(FHandle>0)
    {
        shmctl(FHandle,IPC_RMID,0);
        FLastError=errno;
        FHandle = -1;
    }
#endif 
}

long TSharedMem::GetSize(void)
{
    return FMemInfo ? (long)FMemInfo->Size : 0;

}

unsigned char * TSharedMem::GetBuffer(void)
{
    return FMemInfo ? FMemInfo->GetMemData() : NULL;
}

int * TSharedMem::GetCurrMemTableKey()
{
    return FMemInfo ? FMemInfo->GetCurrMemTableKey() : NULL;
}       

#ifdef _WIN32
HANDLE TSharedMem::GetHanedle(void)
{
    return FHandle;
}
#else
int TSharedMem::GetHanedle(void)
{
    return FHandle;
}
#endif 

void TSharedMem::SetKeyName(const char *AName,char *OKeyName)
{
   
#ifdef _ASSERT_PARAM  
	if( NULL==OKeyName )
	{			
		throw TException("Parameter is wrong,OKeyName mustn't bu NULL");
	}
#endif
   //char *sEnv=NULL;//ZMP:305593,变量未用到
   //sEnv = getenv(SHM_FTOK_PATH_ENV);

#ifdef  _WIN32
    //if(sEnv!=NULL)
    //{
    //    strncpy(OKeyName,sEnv,200);
    //}
    //else
    {
        //ZMP:467374 
        strncpy(OKeyName,"ALL",200);
    }
    strcat(OKeyName,AName);
#else
    //ZMP:305593
    char *sEnv = getenv(SHM_FTOK_PATH_ENV);
    if(sEnv!=NULL)
    {
        strncpy(OKeyName,sEnv,200);
        if(!TFileOper::IsExist(OKeyName))
        {
            TFileOper::Mkdir(OKeyName);
        }
    }
    else
    {
        //ZMP:467374 
        strncpy(OKeyName,".",200);
    }

    if(OKeyName[strlen(OKeyName)-1]!=PATH_DELIMITATED_CHAR)
    {
        strcat(OKeyName,PATH_DELIMITATED);
    }
    strcat(OKeyName,"tkey");
    if(!TFileOper::IsExist(OKeyName))
    {
        TFileOper::Mkdir(OKeyName);
    }

    if(OKeyName[strlen(OKeyName)-1]!=PATH_DELIMITATED_CHAR)
    {
        strcat(OKeyName,PATH_DELIMITATED);
    }

    strcat(OKeyName,AName);
    if(!TFileOper::IsExist(OKeyName))
    {
        TFileOper::Mkdir(OKeyName);
    }
#endif
}
