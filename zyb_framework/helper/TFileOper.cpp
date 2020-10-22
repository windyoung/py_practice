//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TFileOper.cpp
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: 
////////////////////////////////////////////////
/*
* $History: TFileOper.cpp $
*/

#include "TStrFunc.h"
#include "TFileOper.h"
#include "sys/stat.h"
#include <assert.h>
#include <signal.h>
typedef void (*sighandler_t)(int);

#ifdef __OS_WINDOWS__
#include "io.h"
#include "direct.h"
#endif
#define ASSERT assert

#if defined(__OS_UNIX__) || defined(_BORLANDC) 
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <errno.h>
#endif

#include "TSplit.h"
#include "debug_new.h"


//ZMP:467370   
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

#define MAX_FILE_NUMBER 10000000
const int   FILE_NAME_LEN = 1024;

bool TFileOper::SameFileSystem(const char * sPathName1, const char * sPathName2)
{
    struct stat StatBuf1;
    struct stat StatBuf2;

    stat(sPathName1, &StatBuf1);
    stat(sPathName2, &StatBuf2);
    return (StatBuf1.st_dev == StatBuf2.st_dev);
}

bool TFileOper::Rename(const char * srcFileName, const char * desFileName, bool bSameFileSystem)
{
    if (!IsExist(srcFileName))
    {
        //UR:207595
        if(srcFileName != NULL)
        {
            cout << "SrcFile [" << srcFileName << "]" << "Not Exist." << endl; 
        }
        return false;
    }
    static char tempDesFileName[FILE_NAME_LEN]; 
    tempDesFileName[0] = '\0';
    JudgeDesFileIsExist(desFileName, tempDesFileName);

#if defined(__OS_WINDOWS__)
    int iRet = ::rename(srcFileName, tempDesFileName);
     //357131 for cppcheck 增加对返回值的判断
    if( iRet )
    {
        return false;
    }
#else
    if (bSameFileSystem)
    {
        int iRet = rename(srcFileName, tempDesFileName);
        return (0 == iRet);
    }
    else
    {
        //用系统命令处理改名操作
        char sCmdLine[1024]={0}; //357131 for cppcheck
        //ZMP:467370 
        snprintf(sCmdLine, sizeof(sCmdLine), "mv %s %s",srcFileName, tempDesFileName);
        sCmdLine[sizeof(sCmdLine)-1] = '\0';
        sighandler_t old_handler;               //zmp 853284
        old_handler = signal(SIGCLD, SIG_DFL);  //zmp 853284
        int iRet = system(sCmdLine);
        signal(SIGCLD, old_handler);            //zmp 853284
        //357131 for cppcheck 增加对返回值的判断,ZMP:413757 
        ++iRet;
    }

#endif

    return !IsExist(srcFileName);
}

void TFileOper::JudgeDesFileIsExist(const char *DesFileName, char* tempDesFileName)
{
    if (IsExist(DesFileName))
    {
        cout << "DesFile [" << DesFileName << "]" << "Exist." << endl; 
        bool bExistDesFile = true;
        while (bExistDesFile)
        {
            static llong iFileIndex = 0;
            time_t iCurrentTime = time(NULL);
            //.DFN_当前时间time_t格式_PID_seq
            snprintf(tempDesFileName,FILE_NAME_LEN,"%s.DFN_%ld_%ld_%ld",DesFileName,(llong)iCurrentTime,(llong)getpid(),iFileIndex);
            if (!IsExist(tempDesFileName))
            {
                bExistDesFile = false;
            }
            iFileIndex++;
            if (iFileIndex > MAX_FILE_NUMBER)
            {
                iFileIndex = 0;
            }
        }
        cout << "now use [" << tempDesFileName << "]" << " to replace [" << DesFileName << "]" << endl; 
    }
    else
    {
        memccpy(tempDesFileName, DesFileName, 0, FILE_NAME_LEN-1);
        tempDesFileName[FILE_NAME_LEN - 1] = '\0';
    }
    return;
}

bool TFileOper::Remove(const char * sFileName)
{
    return (0 == ::remove(sFileName));
}

bool TFileOper::Rmdir(const char * sPath)
{
    return (0 == ::rmdir(sPath));
}

bool TFileOper::IsExist(const char * sFile)
{
    int rtn = -1;
#if defined(__OS_UNIX__)
    rtn = access(sFile, F_OK);
#elif defined(__OS_WINDOWS__)
    rtn = access(sFile, 0x00);
#endif
    return rtn == 0;
}

bool TFileOper::Mkdir(const char * sPath)
{
    int rtn = -1;
    //UR:207595
    if(NULL == sPath)
    {
        sPath = "";
    }
#if defined(__OS_WINDOWS__)
    rtn = ::mkdir(sPath);
#elif defined(__OS_UNIX__)
    rtn = ::mkdir(sPath, O_CREAT|S_IRWXU|S_IRWXG);
    if (rtn != 0 && errno == EEXIST)
    {
        cout << "Mkdir :" << sPath << ", this path already exists." << endl;
        rtn = 0;
    }
    string sChmod = "chmod -R 777 ";    
    sChmod += sPath;    
    system(sChmod.c_str());
#endif 
    return rtn == 0;
}

bool TFileOper::MkFullDir(const char * sPath)
{
    static TSplit split;
    split.SetSplitter(PATH_DELIMITATED_CHAR);
    char sBuf[256] = {'\0'};

    split.SetString(sPath);
    for(int i=0; i<split.GetCount(); i++)
    {
        strcat(sBuf, split[i]);
        strcat(sBuf, PATH_DELIMITATED);
        if (!IsExist(sBuf))
        {
            if(!Mkdir(sBuf))
                return false;
        }
    }
    return true;
}

bool TFileOper::Duplicate(const char * srcFileName, const char * desFileName)
{
    //判断是否有原文件
    if (!IsExist(srcFileName))
    {
        //UR:207595
        if(srcFileName != NULL)
        {
            cout << "SrcFile [" << srcFileName << "]" << "Not Exist." << endl; 
        }
        return false;
    }

    //如果目标文件存在,先删除目标文件
    if (IsExist(desFileName) && !Remove(desFileName))
    {
        return false;
    }
    char sCmdLine[4096]={0}; //357131 for cppcheck

#if defined(__OS_WINDOWS__)
    //ZMP:467370 
    snprintf(sCmdLine, sizeof(sCmdLine), "copy %s %s",srcFileName, desFileName);
    sCmdLine[sizeof(sCmdLine)-1] = '\0';
#else
    snprintf(sCmdLine, sizeof(sCmdLine), "cp %s %s",srcFileName, desFileName);
#endif       

    sighandler_t old_handler;               //zmp 853284
    old_handler = signal(SIGCLD, SIG_DFL);  //zmp 853284
    int iRet = system(sCmdLine);
    signal(SIGCLD, old_handler);            //zmp 853284
     //357131 for cppcheck,ZMP:413757 
    ++iRet;

    return IsExist(desFileName);
}

const char * TFileOper::GetFileName(const char * sPathFileName)
{
    //UR:207595
    if(NULL == sPathFileName)
    {
        return NULL;
    }
    const char * pos = strrchr(sPathFileName, PATH_DELIMITATED_CHAR );
    if ( pos == NULL )
        return sPathFileName ;
    else
        return pos+1 ;
}

long TFileOper::GetFileSize(const char * sFile)
{
    if (!TFileOper::IsExist(sFile))
    {
        return -1;
    }

    FILE* fp = fopen(sFile, "r");
    if (NULL == fp)
    {
        int iError = errno;
        printf("fopen(%s, %s) failed. errno : %d, reason : %s \n", sFile, "r", iError, strerror(iError));
        return -1;
    }

    int iRet = fseek(fp, 0L, SEEK_END);
     //357131 for cppcheck ,增加对返回值的判断
    if( iRet )
    {
        fclose(fp);
        fp = NULL;
        return -1;
    }
    long len = ftell(fp);//ZMP:477242
    fclose(fp);
    fp = NULL; //357131 for cppcheck
    return len;
}

int TFileOper::GetFileStat(const  char* sFileName, struct stat& buf )
{
    if(NULL == sFileName) return -1;
    if(0 == sFileName[0] ) return -1;
    return stat(sFileName, &buf);
}

time_t TFileOper::GetModTime(const  char* sFileName)
{
    struct stat buf;
    int iRet = TFileOper::GetFileStat(sFileName,buf);
    if(iRet != 0)
    {
        return 0;
    }
    else
    {
        if(((S_IFREG & buf.st_mode) != 0) )
        {
            return buf.st_mtime;
        }
    }
    return 0;
}

/**********************************************************************
*  功  能 ： // 根据目录和文件名拼装全路径
* 输入参数： // 目录和该目录下的文件名
* 输出参数： // buffer:保存该文件全路径的变量
* 返 回 值： // 成功返回true
***********************************************************************/
bool TFileOper::FillPath(const char *pFolder, const char *pFile, char *buffer, int bufferLen)
{
    ASSERT(NULL != pFolder);
    ASSERT(NULL != pFile);

    char *pTmp = (char*)pFolder;
    int len1 = (int)strlen(pTmp);//ZMP:477242
    int len2 = (int)strlen(pFile);
    //防止内存越界
    if ((len1 + len2 + 1) > bufferLen)
    {
        return false;
    }
    //ZMP:467370
    strncpy(buffer, pFolder, (size_t)len1);
    buffer[len1] = '\0';

    //如果目录最后一个不是\或者/，就在后面添加'\'
    if (len1 > 0 && pTmp[len1 - 1] != '/' && pTmp[len1 - 1]  != '\\')
    {
        strcat(buffer, PATH_DELIMITATED);
    }
    strcat(buffer, pFile);
    return true;
}


/******************************************************************************
* 函数名称	:  LockFile()
* 函数描述	:  缩住文件pszFile    
* 输入		:  pszFile，待锁住的文件名
* 输出		:  无
* 返回值	:  成功返回0，失败返回其他值
* 作者		:  li.shugang
*******************************************************************************/
int TFileOper::LockFile(const char* pszFile)
{
	char sFileName[256]={0}; //357131 for cppcheck
	//int	fileQ = -1;//ZMP:305593,变量未用到
	
#ifdef _WIN32
	//拼写出全路径的文件名
	//ZMP:467376    
	snprintf(sFileName, sizeof(sFileName), "C:\\etc\\%s", pszFile);
    sFileName[sizeof(sFileName)-1] = '\0';
	
	//打开文件
	HANDLE hFile = CreateFile(sFileName,     // open Two.txt 
		GENERIC_READ,                 // open for writing 
		0,                            // do not share 
		NULL,                         // no security 
		OPEN_EXISTING,                // open or create 
		FILE_ATTRIBUTE_NORMAL,        // normal file 
		NULL);                        // no attr. template 
	
	if (hFile == INVALID_HANDLE_VALUE) 
	{
		printf("open file:[%s] error, errno:%d\n", sFileName,errno);
		return -2;
	}
	
	//加锁
    int iRet = ::LockFile(hFile, 0, 0, 0, 0); 
	if (!iRet)
	{
		printf("lock file error! errno[%d]\n", errno);
		CloseHandle(hFile);
		return -1;
	}
#else
    int	fileQ = -1;//ZMP:305593,变量未用到
	//拼写出全路径的文件名
	//ZMP:467370
	snprintf(sFileName, sizeof(sFileName), "%s/etc/%s", getenv("HOME"), pszFile);
	sFileName[sizeof(sFileName)-1] = '\0';
	//打开文件
	fileQ = open(sFileName, O_RDWR);
	if(fileQ < 0)	
	{

		return -2;
	}
	
	//锁住文件
	if(lockf(fileQ, F_TLOCK, 0) < 0)	
	{
		return -1;
	}
#endif
	return 0;	
}


#if defined(__OS_WINDOWS__)
#elif defined(__OS_UNIX__)
key_t TFileOper::Ftok(const char *sFilePath,int iID)
{
    bool bRet=false;
    key_t key=-1;
    //UR:207595
    if(sFilePath == NULL)
    {
    	sFilePath = "";
    }
    char sID[16] = {0};
    FUNC_STR::IntToChar(sID, sizeof(sID), iID/64);
    string strFilePath=sFilePath;
    strFilePath=strFilePath + "/CCB_SHM_" + sID;
    bRet=TFileOper::IsExist(strFilePath.c_str());
    if(bRet==false)
    {
        bRet=TFileOper::Mkdir(strFilePath.c_str());
    }
    if(bRet==true)
    {
        //id取1-64
        errno=0;
        key=ftok(strFilePath.c_str(),iID%64+1);
    }

    return key;
}
#endif 

