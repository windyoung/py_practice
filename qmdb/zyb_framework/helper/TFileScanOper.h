//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
//将与原来的文件基本操作相分离,这个文件只是做文件扫描操作

#ifndef __T_FILE_SCAN_OPER__
#define __T_FILE_SCAN_OPER__

#include "DataStruct.h"
#include "TFileOper.h"

class TFileInfo : public TBaseObject
{
public:
    char PathFileName[512];             //文件全名，加路径
    char FileName[128];                  //简单文件名，不加路径
    long FileSize;                       //文件大小，字节
    char LastModify[DATE_TIME_SIZE];     //最后修改时间
    bool IsDir;                          //是否是目录
    bool CanWrite;                       //是否可写
    bool CanExecute;                     //是否可执行
public:
    virtual string ToString();
    virtual int CompareKey(const TBaseObject *pObject) const;
    virtual bool CompareKeyByName(const TBaseObject *pObject) const;
};    

#define MAX_FILTER_NUM          10
#define MAX_REVERSE_FILTER_NUM  10

class TFileScanOper {
public:
    //设置一条文件名过滤规则
    void SetFilter(const char * filter);
    //设置扫描的路径
    void SetPath(const char * sPath);
    //扫描目录 filenum是一次加载的文件数目，默认值为-1
    void ScanPath(int filenum = -1);
    //指向第一个文件
    void SetTopFile(void);
    //得到下一个文件信息
    TFileInfo * GetNextFile();
    //获得扫描到的文件的数量
    int GetFileListSize(void);
    //清空文件列表
    void ClearFileList(void);
    //打印文件
    void PrintFileList(void);
    //设置排序标志
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
    //文件名过滤规则数组
    string m_FilterArray[MAX_FILTER_NUM];
    //文件名过滤规则数量
    int m_iFilterNum;
    //反向文件名过滤规则数组
    string m_ReverseFilterArray[MAX_REVERSE_FILTER_NUM];
    //反向文件名过滤规则数量
    int m_iReverseFilterNum;
    //存储扫描到的文件的链表
    TBaseList *m_pFileItemList;
private:
    //扫描的路径
    string m_sScanPath;
    TBaseListItem *m_pItem;
    bool m_bSortFlag;
    bool m_bSortNameFlag;//向下兼容，新增按文件名排序
};
#endif

