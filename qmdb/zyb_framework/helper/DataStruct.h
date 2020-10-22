//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/*********************************************************************
* ��Ȩ���У� ������
* �ļ����ƣ� DataStruct
* ����ժҪ�� �ײ����ݽṹ
* ��    �ߣ� Liu.QiQuan
* ������ڣ� 2004/10/07
* 
* �޸ļ�¼��
*    �޸����ڣ�
*    �� �� �ˣ�
*    �޸����ݣ� 
**********************************************************************/
#ifndef _DATA_STRUCT_H_
#define _DATA_STRUCT_H_

#include <string>
#include "Common.h"

using namespace std;

//���������Ϣ
class TBaseListItem : public TBaseObject {
public:
    TBaseObject *data;//������Ϣ
    TBaseListItem *next;//��һ���ָ��
};

#define KEY_COUNT 10
//�����ڵ�ṹ����Ϣ
//��Ϊ�����Ƚϴ������ڽڵ����������ݱ仯���
class TKeyTreeNode : public TBaseObject {
public:
    TBaseListItem *headitem;//List���ͷָ��
    TKeyTreeNode * subkey[KEY_COUNT];//��¼�ӽڵ����
	  TKeyTreeNode * lastkey; //��һ���ڵ�
    int treelevel;
	  //int listnum; // �����ϵļ�¼����
public:
    TKeyTreeNode();
};

//��ϣ��
class THashTable {
public:
    TBaseObject *Data;
    TBaseListItem *Head;
public:
    THashTable();
    virtual ~THashTable();
};
//typedef TBaseListItem * THashTable;

//��������
class TBaseList {
public :
    TBaseList();
    TBaseList(bool bClone);//bClone�Ƿ���ȿ�������
    virtual ~TBaseList();
public:
    TBaseListItem * GetFirstItem();//�õ�ͷ�����
    TBaseListItem * GetNextItem(TBaseListItem * pHeadItem);//�õ���һ�����(��һ�δ���ͷ���)
    TBaseListItem * GetLastItem();//�õ�β�����
public:
    //�õ�List�ĳ���
    llong GetSize(void);
    void Reset();//��λ
    bool SetList(TBaseListItem * & headitem);//����List,����ͷ���ָ�룬���û��ִ��Refresh������ֻ����AddFirst�����ӷ�������,��Ϊβָ��ͳ��Ȼ�û�ж���    
    void SetCloneFlag(bool value) { bCloneFlag = value;}
    void IsNotSetList() { bSetList = false; pUpdateHead=NULL;}
    TBaseListItem ** GetSortArray(void);        
    void Refresh();//��ͷ���Ϊ���ģ��ҳ�β�����ָ�롢�������
    void SetSortNameFlag(bool bSortNameFlag=false);
public:
    TBaseListItem * AddFirst(TBaseObject *data, size_t n = 0);//��ͷ����������
    TBaseListItem * Add(TBaseObject *data, size_t n = 0);//��β������������    
    TBaseListItem * Insert(TBaseListItem * item,TBaseObject * data, size_t n=0); // ��item����������ݣ�ע��item��������ڸ�������
    bool AddList(TBaseListItem *pItem);//����һ���б�

    bool Delete(TBaseObject *data,const bool bDelDataFlag);//����Ĭ�ϵ�ָ���ȽϷ�����ɾ�������Ϣ
    TBaseObject * Seek(TBaseObject *data);//���ҽ����Ϣ(������Compare����)
    TBaseObject * Bisearch(TBaseObject *data);//���ҽ����Ϣ(������Compare����)
    bool Sort(const bool bDelDupItem=false);//��������(Ĭ�ϣ�����)   
    void Clear(const bool bDelDataFlag=false);//������н��

    void PrintList(void (*PrintDataFunc)(TBaseObject *data,const int level),const int level=0);//��ӡList��Ϣ
public:
    static int GetListSize(TBaseListItem *pHead);//ȡ����Ĵ�С
    static string ListToString(TBaseListItem *pHead);//����б�����Ϣ
protected:
    virtual TBaseListItem * NewItem(TBaseObject *data, size_t n = 0);//����һ����㣬�洢��Ϣ
    virtual void DeleteData(TBaseObject *data);
    virtual void DeleteItem(TBaseListItem *item,const bool bDelDataFlag=false);//������
    virtual int Compare(TBaseObject *data1, TBaseObject *data2);//data2Ϊ�Ƚϴ����
    virtual int CompareKey(TBaseObject *data1, TBaseObject *data2);//data2Ϊ�Ƚϴ����
    virtual void PrintData(TBaseObject *data,const int level);
    virtual void ClearSort(void);//�����������
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

//������
class TKeyTree{
public:
    TKeyTree(bool bClone=false);//bClone�Ƿ���ȿ�������
    TKeyTree(TBaseList *pCtrl);
    virtual ~TKeyTree();
public:
    TKeyTreeNode * GetKeyTop();
    TBaseListItem * GetKeyData();    
    TKeyTreeNode * Add(const char *KeyName,TBaseObject *data, size_t n =0);//����һ��ֵ
   
    TBaseObject * Seek(const char *KeyName, TBaseObject *data, const bool isfindlist = true);//���ӷǵݹ��ѯ Seek KeyName��data���ݵ�ֵ
    TBaseObject * Find(const char *KeyName, TBaseObject *data, const bool isfindlist = true);//ԭ�ݹ�Seek ����ΪFind    
    
    void PrintKeyTree(void (*PrintData) (TBaseObject *data,const int level));//��ӡTKeyTree��Ϣ
    void Clear(const bool bDelDataFlag=false);
protected:
    virtual TKeyTreeNode * NewNode(const int up_level);//��������Ϣ    
    virtual void DeleteNode(TKeyTreeNode *node,const bool bDelDataFlag=false);
    TBaseObject * Seek(TKeyTreeNode *pTreeNode, const char *KeyName,TBaseObject *data,const bool isfindlist=true);//��ѯKeyName��data���ݵ�ֵ    
    void PrintKeyNode(TKeyTreeNode *node,void (*PrintData) (TBaseObject *data,const int level),const int level, const int key);//��ӡ���
    void PrintList(TBaseListItem *item, void (*PrintData) (TBaseObject *data,const int level),const int level);//��ӡ�б�
    //��ʼ��pListCtrl
    void CreateCtrlList(bool bClone); //351951 for cppcheck
    //��ʼ��pListCtrlData 
    void CreateCtrlDataList(); //351951 for cppcheck
private:
    void InitKeyInfo();    
    TBaseList *pListCtrl;
    bool bListFlag;
    TKeyTreeNode * pKeyTop;
    TBaseList *pListCtrlData;
    TBaseListItem *pKeyData;    
};


//��ϣ�б� ���������ٷ��ѷ�
class THashList{
public:
    THashList();
    virtual ~THashList();
    THashTable * Add(const char *KeyName, TBaseObject *Data);//���ӽ����Ϣ
    bool Delete(const char *KeyName,TBaseObject *Data,const bool DelDataFlag=false);//����Ĭ�ϵ�ָ���ȽϷ�����ɾ�������Ϣ
    TBaseObject * Seek(const char *KeyName,TBaseObject *Data);//��ѯKeyName��data���ݵ�ֵ
    void Clear(const bool DelDataFlag=false);//������н��
    void SetHashNum(const int Number,const bool DelDataFlag=false);//���ù�ϣ���С
    void PrintHashList(void (*PrintData) (TBaseObject *data,const int level));//��ӡTHashList��Ϣ
    int GetHashCount(){return iHashNum;}
    THashTable ** GetHashList(){return pHashList;}
    virtual int GetKeyCode(const char *key);
private:
    //��ֹ��������
    THashList(const THashList &other) {}
    //THashList(const THashList &other) = delete;   //supported in c++11
    
    //��ֹ��ֵ����
    THashList& operator=(const THashList &other) { return *this; }
    //THashList& operator=(const THashList &other) = delete; //supported in c++11

    void InitHashTable();
    THashTable * Seek(const char *KeyName);//��ѯΪKeyName��ϣList

private:
    int iHashNum;
    TBaseList *pListCtrl;
    THashTable **pHashList;
};


//////////////////////////////////////////////////////////////////////////
//
//  ƽ������
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
    TBaseObject *Data;   // ��ֵ
    int Balance;         // ƽ�����ӣ��ǽڵ���������ĸ߶ȼ�ȥ�������ĸ߶ȵĸ߶Ȳ� 
    TAvlNode *LeftNode, 
             *RightNode; // ָ��������ָ��
};

//////////////////////////////////////////////////////////////////////////
//
//  ƽ���������
//
//////////////////////////////////////////////////////////////////////////
class TAvlTree {
public:    
    TAvlTree(const unsigned int iStep=1);
    virtual ~TAvlTree(void);
public:
    TAvlNode * Add(const TBaseObject *pData);//���������
    bool Delete(const TBaseObject *pData,const bool bDelDataFlag=false);//ɾ�������
    TAvlNode * Seek(const TBaseObject *pData);//���������ҽ��
    void Clear(const bool bDelDataFlag=false);//�������
    void PrintData(TAvlNode *pNode=NULL,const int iLevel=0);//��ӡ����
    static void TestAvlTree(void);//���Զ�����
protected:
    void ClearData(TAvlNode *pNode=NULL,const bool bDelDataFlag=false);//�������
    virtual TAvlNode * NewNode(const TBaseObject *pData);//����һ����㣬�洢��Ϣ
    virtual void FreeNode(TAvlNode *pNode);
    TAvlNode * InsertRight(TAvlNode *pParentNode,const TBaseObject *pData);
    TAvlNode * DeleteRight(TAvlNode *pParentNode,const TBaseObject *pData);
    TAvlNode * BalanceTree(TAvlNode *pNode, int Balance);
    //ZMP:315426,Ȧ���Ӷȴ������ģ�begin
    TAvlNode * BalanceTreeInc(TAvlNode *pNode, int Balance);//����������ʱ��ƽ��
    TAvlNode * BalanceTreeDec(TAvlNode *pNode, int Balance);//����������ʱ��ƽ��
    //ZMP:315426,Ȧ���Ӷȴ������ģ�end
    TAvlNode * LeftRotary(TAvlNode *pNode);
    TAvlNode * RightRotary(TAvlNode *pNode);
private:
    TAvlNode *m_pRootNode;   //ƽ�����Ľ׵ĸ�
    int m_iStep;             //�߶�ƽ�����Ľף���ƽ������Ϊ [-m_iStep,m_iStep] ��ʱ����ƽ���
    int m_bAvlFlag;          //�߶�������־
    int m_iAvlHeight;        //�߶�ƽ�����ĸ߶�
    bool m_bDelDataFlag;     //ɾ������Ƿ��ɾ������
    TAvlNode *m_pNewNode;    //����������
};



//��Component.h������
//ջ,ֻ�ṩllong �� char* ��
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
    bool   Push(llong iData);   //��ջ
    llong  Pop();             //��ջ
    llong  Top();             //��ջ��Ԫ��
    llong  GetData(int iPos); //ȡĳ��λ�õ�ֵ
    bool   Clear();           //���ջ
    bool   Delete(int iPos);  //ɾ��ջ��ĳ��Ԫ��
    bool   Sort(bool Asc=true);            //��ջ��Ԫ�ؽ�������,Ĭ������,��С����
    int    Find(llong iValue);  //����ֵ
private:
    llong  *m_piData;
    int    m_iStackSize;
    int    m_iPos;
};
#endif
