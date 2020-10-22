//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#include "TFileScanOper.h"
#include "sys/stat.h"
#include "TStrFunc.h"

#ifdef __OS_WINDOWS__
#include "io.h"
#include "direct.h"
#endif

#if defined(__OS_UNIX__) || defined(_BORLANDC) 
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#endif
#include "debug_new.h"
#include "TDateTimeFunc.h"
#include <errno.h>
#include "Common.h"
#include "debug_new.h"

//ZMP:467373     
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

string TFileInfo::ToString()
{
    char sLineText[1024]={0}; //357131 for cppcheck
    char sBeginTime[30]={0}; //357131 for cppcheck
    //ZMP:467373
    strncpy(sBeginTime,FUNC_DT::TimeToString(FUNC_DT::StringToTime(LastModify),true),sizeof(sBeginTime)-1);
    sBeginTime[sizeof(sBeginTime)-1] = '\0';
    snprintf(sLineText, sizeof(sLineText), "%s %s %6ld %s%s %-14s",sBeginTime,(IsDir?"<DIR> ":"<FILE>"),FileSize,(CanWrite?"W":"."),(CanExecute?"X":"."),FileName);
    sLineText[sizeof(sLineText)-1] = '\0';
    string sLine = sLineText;
    return sLine;
}

int TFileInfo::CompareKey(const TBaseObject *pObject) const
{
    TFileInfo* pObject2 = (TFileInfo*)pObject;
    long iSecond = TDateTimeFunc::GetDiffTime(pObject2->LastModify,LastModify);

    return iSecond == 0? 0:(iSecond >0?  1:-1);
}
//�������ļ�������ӿ�    
bool TFileInfo::CompareKeyByName(const TBaseObject *pObject) const
{
    TFileInfo* pObject2 = (TFileInfo*)pObject;   
    return (strcmp(pObject2->FileName,FileName) > 0)? true : false;
}

TFileScanOper::TFileScanOper(bool bSortFlag)
{
    //�ļ����˹�������
    m_iFilterNum = 0;
    //��������ļ�������
    m_iReverseFilterNum = 0;
    //��������ڵ㷽ʽΪ����
    //m_pFileItemList = new TBaseList(true);
    CreateFileList();
    //ZMP:305593
    m_pItem = NULL;
    //�����־
    m_bSortFlag = bSortFlag;
    m_bSortNameFlag = false;
}
//351951 begin
void TFileScanOper::CreateFileList()
{
     m_pFileItemList = new TBaseList(true);
}
//351951 end

/*
351951 begin
TFileScanOper::TFileScanOper(const TFileScanOper& oTFileScanOper)
{
    m_iFilterNum = oTFileScanOper.m_iFilterNum;
    m_iReverseFilterNum = oTFileScanOper.m_iReverseFilterNum;
    m_pFileItemList = new TBaseList(true);
    TBaseListItem *pItem = oTFileScanOper.GetNextItem(GetFirstItem());
    while(pItem)
    {
        m_pFileItemList->AddList(pItem);
        pItem = oTFileScanOper.GetNextItem(GetFirstItem());
    }
    TBaseListItem *pItem
    m_pFileItemList.l AddList(TBaseListItem *pItem)
    m_pItem = oTFileScanOper.m_pItem;
}
 //351951 end
*/
TFileScanOper::~TFileScanOper()
{
    if(m_pFileItemList)
    {
        //liyong modify 20060921
        m_pFileItemList->Clear(true);
        DELETE(m_pFileItemList);
    }
}

//����ɨ���·��
void TFileScanOper::SetPath(const char * sPath)
{
    m_sScanPath = sPath;
}

int TFileScanOper::GetFileListSize(void)
{
    return (int)m_pFileItemList->GetSize();
}

void TFileScanOper::SetFilter(const char * filter)
{
    //ZMP:324291
    if( filter != NULL)
    {
        if (filter[0] != '!') 
        {
            //���뵽m_pFilterArray��
            if(m_iFilterNum < MAX_FILTER_NUM)
                m_FilterArray[m_iFilterNum++] = filter;
        }
        else
        {
            //���뵽m_pReverseFilterArray��
            if(m_iReverseFilterNum < MAX_REVERSE_FILTER_NUM)
                m_ReverseFilterArray[m_iReverseFilterNum++] = filter+1;
        }
    }
    
}

void TFileScanOper::ClearFileList(void)
{
    m_pFileItemList->Clear();
}


void TFileScanOper::ScanPath(int filenum)
{
    //����ļ��б�
    ClearFileList();

    //int rtn = 0;//ZMP:305593,����δ�õ�
    TFileInfo FileItem;
    struct stat StatBuf;
    m_pItem = NULL;

#if defined(__OS_UNIX__)

    DIR * pDir;
    dirent *pDirent;

    if((pDir = opendir(m_sScanPath.c_str()))) 
    {
        while((pDirent = readdir(pDir)))
        {
            strncpy(FileItem.PathFileName,(m_sScanPath + PATH_DELIMITATED_CHAR + pDirent->d_name).c_str(),sizeof(FileItem.PathFileName)-1);
            stat(FileItem.PathFileName, &StatBuf);

            if( (StatBuf.st_mode & S_IFDIR) == false)//������ļ��Ļ�
            {               
                //�������������ļ�������һ���ļ�
                if(IsSuffice(pDirent->d_name)==false)
                {
                    continue;
                }
            }
            else
            {
                if(pDirent->d_name[0] == '.')
                {
                    continue;
                }
            }

            if(StatBuf.st_mode & S_IWRITE)                      //�Ƿ��д
                FileItem.CanWrite = true;
            else
                FileItem.CanWrite = false;

            if(StatBuf.st_mode & S_IFDIR)                       //�Ƿ���Ŀ¼
                FileItem.IsDir = true;
            else
                FileItem.IsDir = false;

            if(StatBuf.st_mode & S_IEXEC)                       //�Ƿ��ִ��
                FileItem.CanExecute = true;
            else
                FileItem.CanExecute = false;

            FileItem.FileSize = StatBuf.st_size;                //�ļ���С

            strcpy(FileItem.LastModify,FUNC_DT::TimeToString(StatBuf.st_mtime));
            strncpy(FileItem.FileName,pDirent->d_name,sizeof(FileItem.FileName)-1);

            //if(FileItem.IsDir==false)
            //{
            m_pFileItemList->Add(&FileItem,sizeof(FileItem));
            //}
			if((filenum>0) && (m_pFileItemList->GetSize() >= filenum))
			{
				break;
			}
        } //end of while((pDirent = readdir(pDir)) != NULL)
        //�ر�Ŀ¼���
        closedir(pDir);
    }

#elif defined(__OS_WINDOWS__)

    _finddata_t FileStruct;                          //�ļ��ṹ��
    long handle;                                     //�ļ����Ҿ��
    char tmpPath[1024]={0}; //357131 for cppcheck
    //ZMP:467373
    strncpy(tmpPath, m_sScanPath.c_str(), sizeof(tmpPath)-1);
    tmpPath[sizeof(tmpPath)-1] = '\0';
    strcat(tmpPath, "\\*");

    handle=_findfirst(tmpPath,&FileStruct);
    if( handle != -1 ) 
    {
        do
        {
            strncpy(FileItem.PathFileName, (string(m_sScanPath) + PATH_DELIMITATED_CHAR + FileStruct.name).c_str(),sizeof(FileItem.PathFileName) -1);
            stat(FileItem.PathFileName, &StatBuf);

            if( (StatBuf.st_mode & S_IFDIR) == false )  //������ļ��Ļ�
            {                 
                //�������������ļ�������һ���ļ�
                if(IsSuffice(FileStruct.name)==false)
                {
                    continue;
                }
            }
            else
            {
                if(FileStruct.name[0] == '.')
                {
                    continue;
                }
            }

            if(StatBuf.st_mode & S_IWRITE)                      //�Ƿ��д
                FileItem.CanWrite = true;
            else
                FileItem.CanWrite = false;

            if(StatBuf.st_mode & S_IFDIR)                       //�Ƿ���Ŀ¼
                FileItem.IsDir = true;
            else
                FileItem.IsDir = false;

            if(StatBuf.st_mode & S_IEXEC)                       //�Ƿ��ִ��
                FileItem.CanExecute = true;
            else
                FileItem.CanExecute = false;

            FileItem.FileSize = StatBuf.st_size;                //�ļ���С
            //ZMP:467373
            strncpy(FileItem.LastModify,FUNC_DT::TimeToString(StatBuf.st_mtime), sizeof(FileItem.LastModify)-1);
            FileItem.LastModify[sizeof(FileItem.LastModify)-1] = '\0';
            strncpy(FileItem.FileName,FileStruct.name,sizeof(FileItem.FileName)-1);
            m_pFileItemList->Add(&FileItem,sizeof(FileItem));
			if((filenum>0) && (m_pFileItemList->GetSize() >= filenum))
			{
				break;
			}
        }while(_findnext(handle,&FileStruct )!=-1);

//add for 163278 begin
      _findclose(handle);
//end

    }
#endif 

    //���޸�ʱ��Ͱ��ļ�������ֻ��ѡ����һ
    if(m_bSortNameFlag)
    {   
        m_pFileItemList->SetSortNameFlag(m_bSortNameFlag);
        m_pFileItemList->Sort();
    }    
    else if(m_bSortFlag)
    {
        m_pFileItemList->Sort();
    }
}
//��ӡ�ļ�
void TFileScanOper::PrintFileList(void)
{
    m_pItem = NULL;
    TFileInfo *pFileItem=NULL; //357131 for cppcheck
    cout << "Scan Path [" << m_sScanPath << "] File Total :" << m_pFileItemList->GetSize() << endl; 
    //ZMP:305593
    pFileItem=GetNextFile();
    while(pFileItem != NULL)
    {
        cout << pFileItem->ToString()<<endl;
        pFileItem=GetNextFile();
    }
    m_pItem = NULL;
}


void TFileScanOper::SetSortFlag(bool bSortFlag )
{
    m_bSortFlag = bSortFlag;
}

void TFileScanOper::SetSortNameFlag(bool bSortNameFlag )
{
    m_bSortNameFlag = bSortNameFlag;
}

bool TFileScanOper::IsSuffice(const char *sFileName)
{
    bool bFlag = true;
    int i;

    //�Ƿ���Ϲ�����
    for(i=0;i<m_iFilterNum;i++)
    {
        bFlag = FUNC_STR::MatchString(sFileName,m_FilterArray[i].c_str());
        if(bFlag)
        {
            break;
        }
    } 

    if(bFlag==true)
    {
        for(i=0;i<m_iReverseFilterNum;i++)
        {
            if(FUNC_STR::MatchString(sFileName, m_ReverseFilterArray[i].c_str()))
            {
                bFlag=false;
                break;
            }
        }
    }

    return bFlag;
}

//ָ���һ���ļ�
void TFileScanOper::SetTopFile(void)
{
    m_pItem = NULL;
}

TFileInfo * TFileScanOper::GetNextFile()
{
    TFileInfo *pFileItem=NULL;

    m_pItem = m_pFileItemList->GetNextItem(m_pItem);
    if (m_pItem != NULL ) 
    {
        pFileItem = (TFileInfo *)m_pItem->data;
    }

    return pFileItem;
}
