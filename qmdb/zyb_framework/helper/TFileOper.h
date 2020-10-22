//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TFileOper.h
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: 
////////////////////////////////////////////////
/*
* $History: TFileOper.h $
*/
#ifndef __T_FILE_OPER__
#define __T_FILE_OPER__
#include "Common.h"

class TFileOper {
public:
    //rename函数在跨文件系统改名时，会失败，所以必须使用mv系统调用改名。
    //使用MV系统调用完成改名。对于同一个文件系统中的文件改名，浪费性能
    //sPathName 文件名，目录名都可以
    static bool SameFileSystem(const char * sPathName1, const char * sPathName2);
    //重命名一个文件
    static bool Rename(const char * srcFileName, const char * desFileName, bool bSameFileSystem=true);
    //删除一个文件
    static bool  Remove(const char * sFileName);
    //删除一个空文件夹
    static bool  Rmdir(const char * sPath);
    //新建一个目录
    static bool  Mkdir(const char * sPath);
    //根据路径创建完整的多重目录
    static bool  MkFullDir(const char * sPath);
    //得到不含路径的文件名
    static const char * GetFileName(const char * sPathFileName);
    //判断一个文件或者路径是否存在
    static bool IsExist(const char * sFile);
    //复制一个文件
    static bool Duplicate(const char * srcFileName, const char * desFileName);
    // 功  能 ： // 根据目录和文件名拼装全路径
    static bool FillPath(const char *pFolder, const char *pFile, char *buffer, int bufferLen);
    //获取文件大小
    static long GetFileSize(const char * sFile);

    static time_t GetModTime(const  char* sFileName);
    
    /******************************************************************************
	* 函数名称	:  LockFile()
	* 函数描述	:  缩住文件pszFile    
	* 输入		:  pszFile，待锁住的文件名
	* 输出		:  无
	* 返回值	:  成功返回0，失败返回其他值
	* 作者		:  li.shugang
	*******************************************************************************/
	static int LockFile(const char* pszFile);	
	
    //UNIX 平台下替换ftok
#if defined(__OS_WINDOWS__)
#elif defined(__OS_UNIX__)
    //替换ftok根据路径和ID获取IPC唯一标识
    static key_t Ftok(const char *sFilePath,int iID);
#endif 

protected:        
    static int GetFileStat(const  char* sFileName, struct stat& buf );
    static void JudgeDesFileIsExist(const char *DesFileName, char* tempDesFileName);
};

#endif //__T_FILE_OPER__
