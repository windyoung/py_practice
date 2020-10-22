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
    //rename�����ڿ��ļ�ϵͳ����ʱ����ʧ�ܣ����Ա���ʹ��mvϵͳ���ø�����
    //ʹ��MVϵͳ������ɸ���������ͬһ���ļ�ϵͳ�е��ļ��������˷�����
    //sPathName �ļ�����Ŀ¼��������
    static bool SameFileSystem(const char * sPathName1, const char * sPathName2);
    //������һ���ļ�
    static bool Rename(const char * srcFileName, const char * desFileName, bool bSameFileSystem=true);
    //ɾ��һ���ļ�
    static bool  Remove(const char * sFileName);
    //ɾ��һ�����ļ���
    static bool  Rmdir(const char * sPath);
    //�½�һ��Ŀ¼
    static bool  Mkdir(const char * sPath);
    //����·�����������Ķ���Ŀ¼
    static bool  MkFullDir(const char * sPath);
    //�õ�����·�����ļ���
    static const char * GetFileName(const char * sPathFileName);
    //�ж�һ���ļ�����·���Ƿ����
    static bool IsExist(const char * sFile);
    //����һ���ļ�
    static bool Duplicate(const char * srcFileName, const char * desFileName);
    // ��  �� �� // ����Ŀ¼���ļ���ƴװȫ·��
    static bool FillPath(const char *pFolder, const char *pFile, char *buffer, int bufferLen);
    //��ȡ�ļ���С
    static long GetFileSize(const char * sFile);

    static time_t GetModTime(const  char* sFileName);
    
    /******************************************************************************
	* ��������	:  LockFile()
	* ��������	:  ��ס�ļ�pszFile    
	* ����		:  pszFile������ס���ļ���
	* ���		:  ��
	* ����ֵ	:  �ɹ�����0��ʧ�ܷ�������ֵ
	* ����		:  li.shugang
	*******************************************************************************/
	static int LockFile(const char* pszFile);	
	
    //UNIX ƽ̨���滻ftok
#if defined(__OS_WINDOWS__)
#elif defined(__OS_UNIX__)
    //�滻ftok����·����ID��ȡIPCΨһ��ʶ
    static key_t Ftok(const char *sFilePath,int iID);
#endif 

protected:        
    static int GetFileStat(const  char* sFileName, struct stat& buf );
    static void JudgeDesFileIsExist(const char *DesFileName, char* tempDesFileName);
};

#endif //__T_FILE_OPER__
