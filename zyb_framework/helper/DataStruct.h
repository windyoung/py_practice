//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/*********************************************************************
* 版权所有： 中兴软创
* 文件名称： DataStruct
* 内容摘要： 底层数据结构
* 作    者： Liu.QiQuan
* 完成日期： 2004/10/07
* 
* 修改记录：
*    修改日期：
*    修 改 人：
*    修改内容： 
**********************************************************************/
#ifndef _DATA_STRUCT_H_
#define _DATA_STRUCT_H_

#include <string>
#include "Common.h"

using namespace std;

//链表结点的信息
class TBaseListItem : public TBaseObject {
public:
    TBaseObject *data;//数据信息
    TBaseListItem *next;//下一结点指针
};

#define KEY_COUNT 10
//键树节点结构的信息
//因为键树比较大，所以在节点上设置数据变化情况
class TKeyTreeNode : public TBaseObject {
public:
    TBaseListItem *headitem;//List类的头指针
    TKeyTreeNode * subkey[KEY_COUNT];//记录子节点的链
	  TKeyTreeNode * lastkey; //上一个节点
    int treelevel;
	  //int listnum; // 链表上的记录个数
public:
    TKeyTreeNode();
};

//哈希表
class THashTable {
public:
    TBaseObject *Data;
    TBaseListItem *Head;
public:
    THashTable();
    virtual ~THashTable();
};
//typedef TBaseListItem * THashTable;

//单链表类
class TBaseList {
public :
    TBaseList();
    TBaseList(bool bClone);//bClone是否深度拷贝数据
    virtual ~TBaseList();
public:
    TBaseListItem * GetFirstItem();//得到头部结点
    TBaseListItem * GetNextItem(TBaseListItem * pHeadItem);//得到下一个结点(第一次传入头结点)
    TBaseListItem * GetLastItem();//得到尾部结点
public:
    //得到List的长度
    llong GetSize(void);
    void Reset();//复位
    bool SetList(TBaseListItem * & headitem);//导入List,传入头结点指针，如果没有执行Refresh方法，只能用AddFirst来增加反向链表,因为尾指针和长度还没有定义    
    void SetCloneFlag(bool value) { bCloneFlag = value;}
    void IsNotSetList() { bSetList = false; pUpdateHead=NULL;}
    TBaseListItem ** GetSortArray(void);        
    void Refresh();//以头结点为中心，找出尾部结点指针、算出长度
    void SetSortNameFlag(bool bSortNameFlag=false);
public:
    TBaseListItem * AddFirst(TBaseObject *data, size_t n = 0);//在头部增加数据
    TBaseListItem * Add(TBaseObject *data, size_t n = 0);//在尾部后增加数据    
    TBaseListItem * Insert(TBaseListItem * item,TBaseObject * data, size_t n=0); // 在item后面插入数据，注：item必须存在于该链表中
    bool AddList(TBaseListItem *pItem);//增加一个列表

    bool Delete(TBaseObject *data,const bool bDelDataFlag);//用类默认的指定比较方法，删除结点信息
    TBaseObject * Seek(TBaseObject *data);//查找结点信息(需重载Compare函数)
    TBaseObject * Bisearch(TBaseObject *data);//查找结点信息(需重载Compare函数)
    bool Sort(const bool bDelDupItem=false);//排序链表(默认：升序)   
    void Clear(const bool bDelDataFlag=false);//清除所有结点

    void PrintList(void (*PrintDataFunc)(TBaseObject *data,const int level),const int level=0);//打印List信息
public:
    static int GetListSize(TBaseListItem *pHead);//取链表的大小
    static string ListToString(TBaseListItem *pHead);//输出列表结点信息
protected:
    virtual TBaseListItem * NewItem(TBaseObject *data, size_t n = 0);//分配一个结点，存储信息
    virtual void DeleteData(TBaseObject *data);
    virtual void DeleteItem(TBaseListItem *item,const bool bDelDataFlag=false);//清除结点
    virtual int Compare(TBaseObject *data1, TBaseObject *data2);//data2为比较传入的
    virtual int CompareKey(TBaseObject *data1, TBaseObject *data2);//data2为比较传入的
    virtual void PrintData(TBaseObject *data,const int level);
    virtual void ClearSort(void);//清除排序数据
    virtual bool CompareKeyByName(TBaseObject *data1, TBaseObject *data2);
private:
    void QuickSort(const int begin,const int end);
    int PartiTion(const int begin,const int end,TBaseListItem *pItem);
    void RefreshSort(const bool bDelDupItem);
    int creat_heap(int m,int n);
    void heap_sort(int n);
private :
    TBaseListItem * pHead;
    TBaseListItem * pTail;
    TBaseListItem * pCurr;

    TBaseListItem ** pUpdateHead;
    TBaseListItem ** pSortTab;

    long lListSize;
    bool bCloneFlag,bSetList;
    long m_bSortNameFlag;
};

//键树类
class TKeyTree{
public:
    TKeyTree(bool bClone=false);//bClone是否深度拷贝数据
    TKeyTree(TBaseList *pCtrl);
    virtual ~TKeyTree();
public:
    TKeyTreeNode * GetKeyTop();
    TBaseListItem * GetKeyData();    
    TKeyTreeNode * Add(const char *KeyName,TBaseObject *data, size_t n =0);//增加一键值
   
    TBaseObject * Seek(const char *KeyName, TBaseObject *data, const bool isfindlist = true);//增加非递归查询 Seek KeyName和data数据的值
    TBaseObject * Find(const char *KeyName, TBaseObject *data, const bool isfindlist = true);//原递归Seek 改名为Find    
    
    void PrintKeyTree(void (*PrintData) (TBaseObject *data,const int level));//打印TKeyTree信息
    void Clear(const bool bDelDataFlag=false);
protected:
    virtual TKeyTreeNode * NewNode(const int up_level);//分配结点信息    
    virtual void DeleteNode(TKeyTreeNode *node,const bool bDelDataFlag=false);
    TBaseObject * Seek(TKeyTreeNode *pTreeNode, const char *KeyName,TBaseObject *data,const bool isfindlist=true);//查询KeyName和data数据的值    
    void PrintKeyNode(TKeyTreeNode *node,void (*PrintData) (TBaseObject *data,const int level),const int level, const int key);//打印结点
    void PrintList(TBaseListItem *item, void (*PrintData) (TBaseObject *data,const int level),const int level);//打印列表
    //初始化pListCtrl
    void CreateCtrlList(bool bClone); //351951 for cppcheck
    //初始化pListCtrlData 
    void CreateCtrlDataList(); //351951 for cppcheck
private:
    void InitKeyInfo();    
    TBaseList *pListCtrl;
    bool bListFlag;
    TKeyTreeNode * pKeyTop;
    TBaseList *pListCtrlData;
    TBaseListItem *pKeyData;    
};


//哈希列表 采用线性再分裂法
class THashList{
public:
    THashList();
    virtual ~THashList();
    THashTable * Add(const char *KeyName, TBaseObject *Data);//增加结点信息
    bool Delete(const char *KeyName,TBaseObject *Data,const bool DelDataFlag=false);//用类默认的指定比较方法，删除结点信息
    TBaseObject * Seek(const char *KeyName,TBaseObject *Data);//查询KeyName和data数据的值
    void Clear(const bool DelDataFlag=false);//清除所有结点
    void SetHashNum(const int Number,const bool DelDataFlag=false);//配置哈希表大小
    void PrintHashList(void (*PrintData) (TBaseObject *data,const int level));//打印THashList信息
    int GetHashCount(){return iHashNum;}
    THashTable ** GetHashList(){return pHashList;}
    virtual int GetKeyCode(const char *key);
private:
    //阻止拷贝构造
    THashList(const THashList &other) {}
    //THashList(const THashList &other) = delete;   //supported in c++11
    
    //阻止赋值拷贝
    THashList& operator=(const THashList &other) { return *this; }
    //THashList& operator=(const THashList &other) = delete; //supported in c++11

    void InitHashTable();
    THashTable * Seek(const char *KeyName);//查询为KeyName哈希List

private:
    int iHashNum;
    TBaseList *pListCtrl;
    THashTable **pHashList;
};


//////////////////////////////////////////////////////////////////////////
//
//  平衡结点类
//
//////////////////////////////////////////////////////////////////////////
class TAvlNode : public TBaseObject {    
public:
    TAvlNode();
    TAvlNode(const TBaseObject *pData);
    virtual ~TAvlNode(void);
    virtual int Compare(int iKey) const;
    virtual int Compare(int iKey1, int iKey2) const;
    virtual int Compare(const TBaseObject *pObject) const;
    string ToString(); 
public:
    TBaseObject *Data;   // 键值
    int Balance;         // 平衡因子，是节点的右子树的高度减去左子树的高度的高度差 
    TAvlNode *LeftNode, 
             *RightNode; // 指向子树的指针
};

//////////////////////////////////////////////////////////////////////////
//
//  平衡二叉树类
//
//////////////////////////////////////////////////////////////////////////
class TAvlTree {
public:    
    TAvlTree(const unsigned int iStep=1);
    virtual ~TAvlTree(void);
public:
    TAvlNode * Add(const TBaseObject *pData);//增加树结点
    bool Delete(const TBaseObject *pData,const bool bDelDataFlag=false);//删除树结点
    TAvlNode * Seek(const TBaseObject *pData);//二叉树查找结点
    void Clear(const bool bDelDataFlag=false);//清除数据
    void PrintData(TAvlNode *pNode=NULL,const int iLevel=0);//打印数据
    static void TestAvlTree(void);//测试二叉树
protected:
    void ClearData(TAvlNode *pNode=NULL,const bool bDelDataFlag=false);//清除数据
    virtual TAvlNode * NewNode(const TBaseObject *pData);//分配一个结点，存储信息
    virtual void FreeNode(TAvlNode *pNode);
    TAvlNode * InsertRight(TAvlNode *pParentNode,const TBaseObject *pData);
    TAvlNode * DeleteRight(TAvlNode *pParentNode,const TBaseObject *pData);
    TAvlNode * BalanceTree(TAvlNode *pNode, int Balance);
    //ZMP:315426,圈复杂度代码整改，begin
    TAvlNode * BalanceTreeInc(TAvlNode *pNode, int Balance);//二叉树增高时的平衡
    TAvlNode * BalanceTreeDec(TAvlNode *pNode, int Balance);//二叉树降低时的平衡
    //ZMP:315426,圈复杂度代码整改，end
    TAvlNode * LeftRotary(TAvlNode *pNode);
    TAvlNode * RightRotary(TAvlNode *pNode);
private:
    TAvlNode *m_pRootNode;   //平衡树的阶的根
    int m_iStep;             //高度平衡树的阶，当平衡因子为 [-m_iStep,m_iStep] 的时候是平衡的
    int m_bAvlFlag;          //高度增减标志
    int m_iAvlHeight;        //高度平衡树的高度
    bool m_bDelDataFlag;     //删除结点是否把删除数据
    TAvlNode *m_pNewNode;    //最后新增结点
};



//从Component.h中移入
//栈,只提供llong 与 char* 的
enum STACK_TYPE
{
    STACK_LONG = 1,
    STACK_CHAR = 2
};
const int STACK_SIZE = 20;

class StackInt
{
public:
    StackInt(int iSize = STACK_SIZE);
    StackInt(const StackInt& oStackInt );//351951 for cppcheck
    ~StackInt();
public:
    bool   Push(llong iData);   //入栈
    llong  Pop();             //出栈
    llong  Top();             //读栈顶元素
    llong  GetData(int iPos); //取某个位置的值
    bool   Clear();           //清空栈
    bool   Delete(int iPos);  //删除栈中某个元素
    bool   Sort(bool Asc=true);            //对栈中元素进行排序,默认升序,从小到大
    int    Find(llong iValue);  //查找值
private:
    llong  *m_piData;
    int    m_iStackSize;
    int    m_iPos;
};
#endif
