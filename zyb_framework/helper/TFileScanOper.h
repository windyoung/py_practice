//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
//����ԭ�����ļ��������������,����ļ�ֻ�����ļ�ɨ�����

#ifndef __T_FILE_SCAN_OPER__
#define __T_FILE_SCAN_OPER__

#include "DataStruct.h"
#include "TFileOper.h"

class TFileInfo : public TBaseObject
{
public:
    char PathFileName[512];             //�ļ�ȫ������·��
    char FileName[128];                  //���ļ���������·��
    long FileSize;                       //�ļ���С���ֽ�
    char LastModify[DATE_TIME_SIZE];     //����޸�ʱ��
    bool IsDir;                          //�Ƿ���Ŀ¼
    bool CanWrite;                       //�Ƿ��д
    bool CanExecute;                     //�Ƿ��ִ��
public:
    virtual string ToString();
    virtual int CompareKey(const TBaseObject *pObject) const;
    virtual bool CompareKeyByName(const TBaseObject *pObject) const;
};    

#define MAX_FILTER_NUM          10
#define MAX_REVERSE_FILTER_NUM  10

class TFileScanOper {
public:
    //����һ���ļ������˹���
    void SetFilter(const char * filter);
    //����ɨ���·��
    void SetPath(const char * sPath);
    //ɨ��Ŀ¼ filenum��һ�μ��ص��ļ���Ŀ��Ĭ��ֵΪ-1
    void ScanPath(int filenum = -1);
    //ָ���һ���ļ�
    void SetTopFile(void);
    //�õ���һ���ļ���Ϣ
    TFileInfo * GetNextFile();
    //���ɨ�赽���ļ�������
    int GetFileListSize(void);
    //����ļ��б�
    void ClearFileList(void);
    //��ӡ�ļ�
    void PrintFileList(void);
    //���������־
    void SetSortFlag(bool bSortFlag );
    void SetSortNameFlag(bool bSortNameFlag = false);
public:
    TFileScanOper(bool bSortFlag = false);
    //TFileScanOper(const TFileScanOper& oTFileScanOper); //351951 For Cppcheck
    virtual ~TFileScanOper();
protected:
    bool IsSuffice(const char *sFileName);
    void CreateFileList(); //351951 For Cppcheck
protected:
    //�ļ������˹�������
    string m_FilterArray[MAX_FILTER_NUM];
    //�ļ������˹�������
    int m_iFilterNum;
    //�����ļ������˹�������
    string m_ReverseFilterArray[MAX_REVERSE_FILTER_NUM];
    //�����ļ������˹�������
    int m_iReverseFilterNum;
    //�洢ɨ�赽���ļ�������
    TBaseList *m_pFileItemList;
private:
    //ɨ���·��
    string m_sScanPath;
    TBaseListItem *m_pItem;
    bool m_bSortFlag;
    bool m_bSortNameFlag;//���¼��ݣ��������ļ�������
};
#endif

