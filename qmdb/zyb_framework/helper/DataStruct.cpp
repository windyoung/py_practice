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
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <ctime>

#include "DataStruct.h"
#include "TException.h"
#include "TDateTimeFunc.h"
#include "debug_new.h"

using namespace std;

//ZMP:467370   
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

//=======================
//  TBaseList 实现部分
//=======================
TBaseList::TBaseList()
{
    pHead = pTail = pCurr = NULL;
    lListSize = 0;
    bSetList = false;
    bCloneFlag = false;
    pUpdateHead = NULL;
    pSortTab = NULL;
    m_bSortNameFlag = false;
}

TBaseList::TBaseList(bool bClone)
{
    bCloneFlag = bClone;
    pHead = pTail = pCurr = NULL;
    lListSize = 0;
    bSetList = false;
    pUpdateHead = NULL;
    pSortTab = NULL;
    m_bSortNameFlag = false;
}

TBaseList::~TBaseList()
{ 
    if(!bSetList)
        Clear();

    DELETE_A(pSortTab);
}

void TBaseList::SetSortNameFlag(bool bSortNameFlag )
{
    m_bSortNameFlag = bSortNameFlag;
}

void TBaseList::Reset()
{
    pHead = pTail = pCurr = NULL;
    lListSize = 0;
    pUpdateHead = NULL;
}

llong TBaseList::GetSize(void)
{
    return lListSize;
}

TBaseListItem * TBaseList::GetFirstItem()
{
    pCurr = pHead;
    if (pHead)
        return pHead;
    return NULL;
}

TBaseListItem * TBaseList::GetNextItem(TBaseListItem * pHeadItem)
{
    if(!pHeadItem) 
        return GetFirstItem();
    if(pCurr)
    {
        pCurr = pCurr->next;
        return pCurr;
    }
    return NULL;
}

TBaseListItem * TBaseList::GetLastItem()
{
    if (pTail)
        return pTail;
    return NULL;
}

TBaseListItem * TBaseList::AddFirst(TBaseObject *data, size_t n)
{  
    if(!data)
        return NULL;

    TBaseListItem *pItem;
    pItem = NewItem(data,n);

    if (!pItem) 
        return NULL;

    if (lListSize==0)
        pTail = pItem;
    else
        pItem->next = pHead;

    pHead = pCurr = pItem;
    if(pUpdateHead)
        *pUpdateHead = pHead;

    lListSize++;

    return pItem;
}

TBaseListItem * TBaseList::Add(TBaseObject *data, size_t n)
{  
    if(!data)
        return NULL;

    TBaseListItem *pItem;
    pItem = NewItem(data,n);

    if (!pItem) 
        return NULL;

    if (lListSize==0)
    {
        pHead = pItem;
        if(pUpdateHead)
            *pUpdateHead = pHead;
    }
    else 
    {
        while(pTail->next)
        {
            lListSize++;
            pTail = pTail->next;            
        }
        pTail->next = pItem;    
    }

    pTail = pCurr = pItem;
    lListSize++;

    return pItem;
}

TBaseListItem * TBaseList::Insert(TBaseListItem * item,TBaseObject * data, size_t n)
{
    if (item == NULL)
    {
        return AddFirst(data,n);
    }
    TBaseListItem * newitem = NewItem(data,n);
    if (!newitem)
    {
        return NULL;
    }
    newitem->next = item->next;
    item->next = newitem;
    ++lListSize;
    return newitem;
}

bool TBaseList::AddList(TBaseListItem *pItem)
{
    if(pItem)
    {
        if (lListSize==0)
        {
            pTail = pHead = pItem;
            if(pUpdateHead)
                *pUpdateHead = pHead;    
            lListSize++;
        }
        else
        {
            while(pTail->next)
            {
                lListSize++;
                pTail = pTail->next;            
            }

            pTail->next = pItem;

        }        

        while(pTail->next)
        {
            lListSize++;
            pTail = pTail->next;            
        }

        pCurr = pTail;

        //清除排序数据
        ClearSort();
    }

    return pItem != NULL;
}

void TBaseList::Clear(const bool bDelDataFlag)
{
    TBaseListItem *pCur=pHead,*pTemp=pHead; //357111 for cppcheck
    //pCurr = pTemp = pHead; //357111 for cppcheck 初始化已做  
    while(pCur)
    {
        pTemp = pCur;
        pCur = pCur->next;
        DeleteItem(pTemp,bDelDataFlag);
    }
    pHead = pTail = NULL;

    if(pUpdateHead)
        *pUpdateHead=NULL;

    lListSize = 0;
}

bool TBaseList::Delete(TBaseObject *data,const bool bDelDataFlag)
{
    TBaseListItem *pCur=NULL,*pTemp=NULL; //357111 for cppcheck
    bool bRetFlag = false;

    pCur = pTemp = pHead;
    while(pCur&&!bRetFlag)
    {
        if (Compare(pCur->data,data)==0)
        {
            bRetFlag = true;
            continue;
        }
        pTemp = pCur;
        pCur = pCur->next;
    }

    if (bRetFlag)
    {
        if (pTemp == pCur)
            pTemp = pCur==NULL ? NULL : pCur->next;
        else
            pTemp->next = pCur ==NULL ? NULL : pCur->next;

        if((pTemp!=NULL)&&pTemp->next==NULL) pTail = pTemp;
        if(pCur==pHead) 
        {
            pHead = pTemp;
            //UR:207563
            if(pUpdateHead)
            {
                *pUpdateHead = pHead;
            }
        }
        DeleteItem(pCur,bDelDataFlag);
        lListSize--;
    }

    return bRetFlag;
}

TBaseListItem * TBaseList::NewItem(TBaseObject *data, size_t n)
{
    TBaseListItem *item = new TBaseListItem;
    if (item==NULL)
        throw TException("TBaseList.NewItem() : new TBaseListItem fail.");

    item->data = NULL;
    if(bCloneFlag) 
    {
        if(n>0)
        {
            void *tempdata;
            tempdata = new char[n];
            if (tempdata == NULL) 
                throw TException("TBaseList.NewItem() : malloc data space fail.");

            memset(tempdata,0,n);
            memcpy(tempdata, data, n);
            item->data = (TBaseObject *) tempdata;
        }
        else
        {
            throw TException("TBaseList.NewItem(CloneFlag=True) : Input Size = 0 ");
        }
    } 
    else
        item->data = (TBaseObject *)data;

    item->next = NULL;

    //清除排序数据
    ClearSort();

    return item;
}

void TBaseList::DeleteData(TBaseObject *data)
{
    /*
    if(bCloneFlag && TBaseObject::Assigned(data))
        if (bCloneFlag)
        {
            delete [] (char *)data;
            data = NULL;  //357111 for cppcheck
            //DELETE_A(data);
        }
        else
            DELETE(data);
    */
    //ZMP:480975 
    if(bCloneFlag && TBaseObject::Assigned(data))
    {
        delete [] (char *)data;
        //data = NULL;
    }
}


void TBaseList::DeleteItem(TBaseListItem * item,const bool bDelDataFlag)
{
    if (TBaseObject::Assigned(item))
    {
        if(bDelDataFlag)
        {
            if(bCloneFlag)
            {
            	  delete [] (char *)item->data;
                //DELETE_A(item->data);
            }
            else if (TBaseObject::Assigned(item->data))
            {
            	  delete item->data;
                //DELETE(item->data);
            }
        }
        else
            DeleteData(item->data);

        item->data = NULL;
        DELETE(item);
        //清除排序数据
        ClearSort();
    } 
}

TBaseObject * TBaseList::Seek(TBaseObject *data)
{
    TBaseListItem *pCur = pHead;
    while(pCur)
    {
        if (0 == Compare(pCur->data,data))
        {
            return pCur->data;
        }
        pCur = pCur->next;
    }
    return NULL;
}

//查找结点信息(需重载Compare函数)
TBaseObject * TBaseList::Bisearch(TBaseObject *data)
{
    long low=0,mid,high,iDiff;//ZMP:305593,变量未用到,iPos=0
    TBaseObject *pRefData=NULL;

    high= lListSize - 1;
    while(low<=high && pRefData==NULL)
    {
        mid=(low+high)/2;

        iDiff = Compare(pSortTab[mid]->data,data);

        if(iDiff==0)
            pRefData = (TBaseObject *)pSortTab[mid]->data;
        else if(iDiff<0) 
            low=mid+1;
        else 
            high=mid-1;
    }

    return pRefData;    
}

TBaseListItem ** TBaseList::GetSortArray(void)
{
    if(pSortTab==NULL)
    {
        Sort();
    }
    return pSortTab;
}

int CompareKeyEx(TBaseObject *ta, TBaseObject *tb)
{
    if(ta==NULL || tb==NULL)
        throw TException("CompareKeyEx() : Input Object Is NULL.");
    TBaseObject **pta = (TBaseObject **)ta;
    TBaseObject **ptb = (TBaseObject **)tb;
    return (*ptb)->CompareKey(*pta);
}

int TBaseList::creat_heap(int l,int m)
{
    int i,j;
    TBaseListItem *swap=NULL; //357111 for cppcheck
    i=l; j=2*i;
    swap = pSortTab[i];
    while(j<=m)
    {
        //加入按文件名排序
        if(m_bSortNameFlag)
        {
          if (j<m && CompareKeyByName(pSortTab[j+1]->data,pSortTab[j]->data))
          { 
              j++;
          }
          if (CompareKeyByName(pSortTab[j]->data,swap->data))
          {
              pSortTab[i] = pSortTab[j];
              i=j;
              j=2*i;
          }
          else j=m+1;
       }
       else
       {
          if(j<m && CompareKey(pSortTab[j+1]->data,pSortTab[j]->data) > 0)
          { 
              j++;
          }
          if (CompareKey(pSortTab[j]->data,swap->data) > 0)
          {
              pSortTab[i] = pSortTab[j];
              i=j;
              j=2*i;
          }
          else j=m+1;
       }
    }
    pSortTab[i]=swap;
    return i;
}

void TBaseList::heap_sort(int n)
{
    unsigned long int compare=0,move=0;
    TBaseListItem *swap=NULL; //357111 for cppcheck
    int i;
    int a;
    for(i=n/2;i>=0;i--) 
        a=creat_heap(i,n);
    compare++;
    move++;

    for(i=n;i>=1;i--)
    {
        swap=pSortTab[i];
        pSortTab[i]=pSortTab[0];
        pSortTab[0]=swap;
        a=creat_heap(0,i-1);
        compare+=(unsigned long int)a;//ZMP:477242
        move+=(unsigned long int)a;
    }
}

bool TBaseList::Sort(const bool bDelDupItem)
{
    bool bRetFlag = false;

    //清除排序数据
    ClearSort();

    lListSize = GetListSize(pHead);
    if( lListSize <= 0 )
        return true;

    pSortTab = new TBaseListItem * [lListSize];

    try
    {
        TBaseListItem *pItem = NULL; int i=0;
        //ZMP:305593,style校正
        pItem = GetNextItem(pItem);
        while( pItem != NULL )
        {
            pSortTab[i++] = pItem;
            pItem = GetNextItem(pItem);
        }

        if (lListSize > 1)
        {
            heap_sort((int)(lListSize-1));
        }
        
        RefreshSort(bDelDupItem);

        bRetFlag = true;
    }
    catch(TException &ex)
    {
        cout << "TBaseList::Sort() : " << ex.GetErrMsg() << endl;
        bRetFlag = false;
    }
    catch(...)
    {
        cout << "TBaseList::Sort() : Unknow Exception" << endl;
        bRetFlag = false;
    }

    if(pUpdateHead)
    {
        *pUpdateHead = pHead;
    }

    return bRetFlag;
}

void TBaseList::RefreshSort(const bool bDelDupItem)
{
    TBaseListItem *pPreItem=NULL,*pCurItem=NULL; //357111 for cppcheck
    int iNewListSize=0;
    //pPreItem=pCurItem = NULL;  //357111 for cppcheck 初始化已做

    for(int i=0; i< lListSize; i++)
    {
        if(pPreItem==NULL)
        {
            pCurItem = pPreItem = pSortTab[i];
        }
        else
        {
            pPreItem = pCurItem;
            pCurItem = pSortTab[i];

            if(bDelDupItem && CompareKey(pPreItem->data,pCurItem->data)==0)
            {
                DELETE(pSortTab[i]);
                pCurItem = pPreItem;
            }
        }

        if(pSortTab[i]==NULL)
        {
            continue;
        }

        //构造新的数据结构
        iNewListSize++;
        pCurItem->next = NULL;
        if( iNewListSize == 1)
            pHead = pCurItem;
        else
            pPreItem->next = pCurItem;
        pTail = pCurItem;
    }
    lListSize = iNewListSize;
}

void TBaseList::QuickSort(const int begin,const int end)
{
    int mid;

    if ( begin < end )
    {
        mid = PartiTion(begin,end,pSortTab[begin]);
        QuickSort(begin,mid);
        QuickSort(mid+1,end);
    }
}

int TBaseList::PartiTion(const int begin,const int end,TBaseListItem *pItem)
{
    int i,j;

    i = begin-1;
    j = end +1;

    while(1)
    {
        do {
            j--;
        } while(CompareKey(pSortTab[j]->data,pItem->data) > 0 );

        do {
            i++;
        } while(CompareKey(pSortTab[i]->data,pItem->data) < 0 );

        if( i < j )
        {
            //swap(pSortTab[i],pSortTab[j]);
            TBaseListItem *sTemp;
            sTemp = pSortTab[i];
            pSortTab[i] = pSortTab[j];
            pSortTab[j] = sTemp;
        }
        else
            return j;
    }
}

int TBaseList::Compare(TBaseObject *data1, TBaseObject *data2)
{
    if(data1==NULL || data2==NULL)
        throw TException("TBaseList::Compare() : Input Object Is NULL.");
    return data2->Compare(data1);
}

int TBaseList::CompareKey(TBaseObject *data1, TBaseObject *data2)
{
    if(data1==NULL || data2==NULL)
        throw TException("TBaseList::Compare() : Input Object Is NULL.");
    return data2->CompareKey(data1);
}

bool TBaseList::CompareKeyByName(TBaseObject *data1, TBaseObject *data2)
{
    if(data1==NULL || data2==NULL)
        throw TException("TBaseList::Compare() : Input Object Is NULL.");
    return data2->CompareKeyByName(data1);
}

void TBaseList::PrintList(void (*PrintDataFunc) (TBaseObject *data,const int level),const int level)
{
    TBaseListItem *pCur= pHead;
    while (pCur)
    {
        if(PrintDataFunc)
            (*PrintDataFunc)(pCur->data,level);
        else
            PrintData(pCur->data,level);

        pCur = pCur->next;
    }
}

//清除排序数据
void TBaseList::ClearSort(void)
{
    //清除排序表格
    DELETE_A(pSortTab);
}

void TBaseList::PrintData(TBaseObject *data,const int level)
{
    TBaseObject *pObjData = (TBaseObject *)data;
    if(pObjData)
    {
        char sLineHead[120] = {0};//ZMP:477242
        //memset(sLineHead,0,sizeof(sLineHead));
        memset(sLineHead,32,(size_t)(level*2));//ZMP:477242

        try
        {
            cout << sLineHead <<pObjData->ToString() << endl;
        }
        catch(...)
        {
            cout << "Print Data Isn't TBaseObject Type," << endl;
        }
    }
}

bool TBaseList::SetList(TBaseListItem * & headitem)
{
    pUpdateHead = &headitem;
    pTail = pHead = pCurr = headitem;
    lListSize = headitem?1:0;
    bSetList = true;
    return !headitem;
}

void TBaseList::Refresh()
{
    TBaseListItem *pCur=pHead;
    lListSize=0;
    while(pCur)
    {
        pTail = pCur;
        pCur = pCur->next;
        lListSize++;
    }
}

//取链表的大小
int TBaseList::GetListSize(TBaseListItem *pHead)
{
    TBaseList ListCtrl(false);
    ListCtrl.SetList(pHead);
    ListCtrl.Refresh();
    return (int)ListCtrl.GetSize();//ZMP:477242
}

//输出列表结点信息
string TBaseList::ListToString(TBaseListItem *pHead)
{
    ostringstream sHeadText;
    string sLineText;

    TBaseList ListCtrl(false);
    ListCtrl.SetList(pHead);
    ListCtrl.Refresh();

    sHeadText << "LinkInfo Size = [" << ListCtrl.GetSize() << "]" << endl;
    TBaseListItem *pItem = NULL;
    //ZMP:305593,style校正
    pItem = ListCtrl.GetNextItem(pItem);
    while(pItem != NULL)
    {
        TBaseObject *Object = (TBaseObject *)pItem->data;
        if(Object)
            sLineText = Object->ToString();
        sHeadText << sLineText << endl;
        pItem = ListCtrl.GetNextItem(pItem);
    }

    return sHeadText.str();
}

//=======================
//  TKeyTree 实现部分
//=======================

//对输入的Key的每个字节进行数字键解码
static char keyarray[256];
TKeyTreeNode::TKeyTreeNode()
{
    headitem = NULL;
    lastkey=NULL;    
    treelevel = -1;
    //listnum=0;    
    for (int i = 0; i < KEY_COUNT; i++)
    {
        subkey[i] = NULL;
    }
}

TKeyTree::TKeyTree(bool bClone)
{
    InitKeyInfo();
    pKeyTop = NewNode(0);
    //pListCtrl = new TBaseList(bClone);
    CreateCtrlList(bClone); //351951 for cppcheck
    bListFlag=true;
    
    //pListCtrlData = new TBaseList(false);
    CreateCtrlDataList();//351951 for cppcheck
    pKeyData = NULL;    
}

TKeyTree::TKeyTree(TBaseList *pCtrl)
{
    bListFlag = false;
    pListCtrl = pCtrl;
    InitKeyInfo();
    pKeyTop = NewNode(0);
    
    //pListCtrlData = new TBaseList(false);
    CreateCtrlDataList(); //351951 for cppcheck
    pKeyData = NULL;    
}
// 351951 begin
void TKeyTree::CreateCtrlList(bool bClone)
{
    pListCtrl = new TBaseList(bClone);
}
void TKeyTree::CreateCtrlDataList()
{
    pListCtrlData = new TBaseList(false);
}
// 351951 end

TKeyTree::~TKeyTree()
{
    if (pKeyData)
    {
        pListCtrlData->SetList(pKeyData);
        pListCtrlData->Clear(false);
        DELETE(pKeyData);
    }
    DELETE(pListCtrlData);
  
    if(pKeyTop)
        DeleteNode(pKeyTop);
    if(bListFlag && pListCtrl)
        DELETE(pListCtrl);
}

void TKeyTree::InitKeyInfo()
{
    for (int c = 0; c < 256; c++) 
    {
        if (isdigit (c))
            keyarray[c] = (char)(c - '0');//ZMP:477242
        else
            keyarray[c] = (char)(c % 10);
    }
}

TKeyTreeNode * TKeyTree::GetKeyTop()
{
    return pKeyTop;
}

TBaseListItem * TKeyTree::GetKeyData()
{
    return pKeyData;
}

TKeyTreeNode * TKeyTree::NewNode(const int up_level)
{
    TKeyTreeNode *node = new TKeyTreeNode();
    if (node==NULL)
        throw TException("TKeyTree.NewNode() : new TKeyTreeNode fail.");

    if (node)
    {
        node->treelevel = up_level+1;
    }
    return node;
}

void TKeyTree::Clear(const bool bDelDataFlag)
{
    DeleteNode(pKeyTop,bDelDataFlag);
    pKeyTop = NULL;
}

void TKeyTree::DeleteNode(TKeyTreeNode * node,const bool bDelDataFlag)
{
    if(TBaseObject::Assigned(node))
    {
        for(int i=0;i<KEY_COUNT;i++)
        {
            if(node->subkey[i]!=NULL)
                DeleteNode(node->subkey[i],bDelDataFlag);
            node->subkey[i] = NULL;
        }
        
        node->lastkey= NULL;
        pListCtrl->SetList(node->headitem);
        pListCtrl->Clear(bDelDataFlag);

        DELETE(node);
    }
}

TKeyTreeNode * TKeyTree::Add(const char *KeyName,TBaseObject *data, size_t n)
{
    TKeyTreeNode *pCurNode = pKeyTop;
    for (int key,i = 0; i < (int)strlen(KeyName) ; i++) 
    {
        key = (int) keyarray[(unsigned char )KeyName[i]];

        if (pCurNode->subkey[key] == NULL)
        {
            pCurNode->subkey[key] = NewNode(pCurNode->treelevel);
            pCurNode->subkey[key]->lastkey=pCurNode;            
        }
        
        pCurNode = pCurNode->subkey[key];
    }
    
    //pCurNode->listnum++;
    pListCtrl->SetList(pCurNode->headitem);
    pListCtrl->AddFirst(data,n);//ZMP:376171 

    pListCtrlData->SetList(pKeyData);
    pKeyData = pListCtrlData->AddFirst(data,n);
    
    return pCurNode;
}

TBaseObject * TKeyTree::Seek(TKeyTreeNode *pTreeNode, const char *KeyName,TBaseObject *data,const bool isfindlist)
{
    TBaseObject *pObject = NULL;

    TKeyTreeNode *pCurNode = pTreeNode;

    if(pCurNode && strlen(KeyName)>0)
    {
        int key = (int) keyarray[(unsigned char )KeyName[0]];

        if (pCurNode->subkey[key] != NULL)
        {
            pObject = Seek(pCurNode->subkey[key],KeyName+1,data,isfindlist);
        }       
    }
    //ZMP:305593
    if((pObject==NULL)&& (NULL != pCurNode))
    {
        if(isfindlist)
        {
            pListCtrl->SetList(pCurNode->headitem);
            pObject = pListCtrl->Seek(data);
        }
        else
        {
            pObject = pCurNode;
        }
    }

    return pObject;
}


// 增加函数Seek 不使用递归调用2008.08.20
TBaseObject * TKeyTree::Seek(const char *KeyName, TBaseObject *data, const bool isfindlist)
{
    TBaseObject *pObject = NULL;
    TKeyTreeNode *pCurNode = pKeyTop;
	int key;
    size_t iLen=strlen(KeyName);//ZMP:477242

	for (size_t i = 0; i < iLen; i++)
	{
        key = (int) keyarray[(unsigned char )KeyName[i]];
        if (pCurNode->subkey[key] != NULL)
        	pCurNode=pCurNode->subkey[key];
		else
			break;
	}
	
    if (isfindlist)
    {
    	while(pObject==NULL && pCurNode)//类似37601
		{
			//if(pCurNode->listnum>0)
			//{
				pListCtrl->SetList(pCurNode->headitem);
				pObject = pListCtrl->Seek(data);
				if(pObject!=NULL)
					return pObject;
			//}
			pCurNode=pCurNode->lastkey;
		}
    }
    else
    {
        pObject = pCurNode;
    }

    return pObject;
}

//查询KeyName和data数据的值, 原来的Seek 改名为Find 
TBaseObject * TKeyTree::Find(const char *KeyName, TBaseObject *data, const bool isfindlist)
{
    return Seek(pKeyTop,KeyName,data,isfindlist);
}

void TKeyTree::PrintKeyTree(void (*PrintKeyData) (TBaseObject *data,const int level))
{
    if (pKeyTop)
    {
        for (int i = 0; i < KEY_COUNT; i++)
        {
            PrintKeyNode(pKeyTop->subkey[i], (*PrintKeyData), 0, i);
        }
    }    
}

void TKeyTree::PrintKeyNode(TKeyTreeNode *node,void (*PrintKeyData) (TBaseObject *data,const int level),const int level, const int key)
{
    if (node) 
    {
        cout.width(level*2+2);
        cout << "L=" << level << ",K=" << key << endl;
        for (int i = 0; i < KEY_COUNT; i++) 
            PrintKeyNode(node->subkey[i], PrintKeyData, level + 1, i);
        PrintList(node->headitem, (*PrintKeyData),level);
    }
}

void TKeyTree::PrintList(TBaseListItem *item, void (*PrintKeyData) (TBaseObject *data,const int level),const int level)
{
    if(item)
    {
        pListCtrl->SetList(item);
        pListCtrl->PrintList((*PrintKeyData),level);
    }
}

//=======================
//  THashList 实现部分
//=======================
THashTable::THashTable()
{
    Data = NULL;
    Head = NULL;
}
THashTable::~THashTable()
{

}
THashList::THashList()
{
    iHashNum = 256;
    InitHashTable();
    pListCtrl = new TBaseList(false);
}

THashList::~THashList()
{
    Clear();
    for(int i=0;i<iHashNum;i++)
    {
        DELETE(pHashList[i]);
    }
    DELETE_A(pHashList);
    DELETE(pListCtrl);
}

void THashList::InitHashTable()
{
    if(iHashNum>0)
    {
        pHashList = new THashTable *[iHashNum];
        for(int i=0;i<iHashNum;i++)
        {
            pHashList[i] = new THashTable();
        }
    }
    else
    {
        pHashList = NULL;
    }
}

THashTable * THashList::Add(const char *KeyName, TBaseObject *Data)
{
    int hval = GetKeyCode(KeyName); 

    if(NULL != pHashList[hval]->Data)
    {
        pListCtrl->SetList(pHashList[hval]->Head);
        pListCtrl->AddFirst(Data);
    }
    else
    {
        pHashList[hval]->Data = Data;
    }

    return pHashList[hval];
}

THashTable * THashList::Seek(const char *KeyName)
{
    int hval = GetKeyCode(KeyName); 
    return pHashList[hval];
}

TBaseObject * THashList::Seek(const char *KeyName, TBaseObject *Data)
{
    THashTable *pTable = Seek(KeyName);
    if (NULL != pTable)
    {
        //compare with the first data
        if (pTable->Data != NULL && pTable->Data->Compare(Data) == 0)
        {
            return pTable->Data;
        }

        //compare with the other data
        if (NULL != pTable->Head)
        {
            pListCtrl->SetList(pTable->Head);
            return pListCtrl->Seek(Data);
        }
    } 
    return NULL;
}

//用类默认的指定比较方法，删除结点信息
bool THashList::Delete(const char *KeyName,TBaseObject *Data,const bool DelDataFlag)
{
    THashTable *pTable = Seek(KeyName);
    if (NULL != pTable)
    {
        //first data with current hash
        if (NULL != pTable->Data && pTable->Data->Compare(Data) == 0) {
            if (DelDataFlag) {
                DELETE(pTable->Data);
            }
            pTable->Data = NULL;
            return true;
        }

        //other data with current hash
        if (NULL != pTable->Head) {
            pListCtrl->SetList(pTable->Head);
            return pListCtrl->Delete(Data, DelDataFlag);
        }
    } 
    
    return false;
}

void THashList::Clear(const bool DelDataFlag)
{
    for(int i=0;i<iHashNum;i++)
    {
        if(DelDataFlag)
        {
            DELETE(pHashList[i]->Data);
        }
        pListCtrl->SetList(pHashList[i]->Head);
        pListCtrl->Clear(DelDataFlag);

        pHashList[i]->Data = NULL;
        pHashList[i]->Head = NULL;
    }
}

void THashList::SetHashNum(const int Number,const bool DelDataFlag)
{
    Clear(DelDataFlag);
    for(int i=0;i<iHashNum;i++)
    {
        DELETE(pHashList[i]);
    }
    DELETE_A(pHashList);
    iHashNum = Number;
    InitHashTable();
}

void THashList::PrintHashList(void (*PrintData) (TBaseObject *data,const int level))
{
    long iCount = 0;
    for(int i=0;i<iHashNum;i++)
    {
        if (NULL == pHashList[i]->Data && NULL == pHashList[i]->Head)
            continue;

        pListCtrl->SetList(pHashList[i]->Head);
        pListCtrl->Refresh();
        iCount = pListCtrl->GetSize() + (NULL != pHashList[i]->Data ? 1 : 0);
        printf("K=%.3d C=%ld\n", i, iCount);
        if (NULL != pHashList[i]->Data)
        {
            printf("    %s\n", pHashList[i]->Data->ToString().c_str());
        }
        pListCtrl->PrintList((*PrintData), 4);
    }    
}

int THashList::GetKeyCode(const char *KeyName)
{
    //int hval=0;
    //const char *ptr = KeyName;
    //char c;

    //for (int i = 1; c=*ptr++; i++)
    //    hval += c*i; /* ascii char times its 1-based index */

    //return (hval%iHashNum);
    
    //UR:208565
    const char* pKey = KeyName; 
    unsigned int hash_key = 0;
    unsigned int g; 
    while (*pKey != 0) 
    { 
        hash_key = (hash_key << 4) + *pKey++; 
        if ((g = (hash_key & 0xF0000000))) 
        { 
            hash_key = hash_key ^ (g >> 24); 
            hash_key = hash_key ^ g; 
         } 
    } 
    return (int)((long)hash_key % iHashNum); 
}

//////////////////////////////////////////////////////////////////////////
//
//  平衡结点类
//
//////////////////////////////////////////////////////////////////////////
TAvlNode::TAvlNode()
{
    RightNode = NULL;
    LeftNode  = NULL;
    Balance   = 0;    /* 叶子节点等高平衡 */
    Data      = NULL;
}

TAvlNode::TAvlNode(const TBaseObject *pData)
{
    RightNode = NULL;
    LeftNode  = NULL;
    Balance   = 0;    /* 叶子节点等高平衡 */
    Data      = (TBaseObject *)pData;
}

TAvlNode::~TAvlNode()
{

}

int TAvlNode::Compare(int iKey) const
{
  throw TException("TAvlNode::Compare(int iKey) : Not Write Code.");
}

int TAvlNode::Compare(int iKey1, int iKey2) const
{
  throw TException("TAvlNode::Compare(int iKey1, int iKey2) : Not Write Code.");
}

int TAvlNode::Compare(const TBaseObject *pObject) const
{
    return pObject->Compare(Data);
}

//输出对象信息
string TAvlNode::ToString()
{
    ostringstream sLineText;
    sLineText << "TAvlNode ValueList :" << endl;
    sLineText << "  BALANCE = [" << Balance << "] " << endl;
    sLineText << "  DATA = [" << Data->ToString() << "] " << endl;
    sLineText << "  LEFT_NODE = ["  << (LeftNode ?"T":"F") << "] " << endl;
    sLineText << "  RIGHT_NODE = [" << (RightNode?"T":"F") << "] " << endl;
    return sLineText.str();
}

//////////////////////////////////////////////////////////////////////////
//
//  平衡二叉树类
//
//////////////////////////////////////////////////////////////////////////
TAvlTree::TAvlTree(const unsigned int iStep)
{
    m_pRootNode = NULL;
    m_iStep = (int)iStep;//ZMP:477242
    //ZMP:305593
    m_bAvlFlag = 0;
    m_iAvlHeight = 0;
    m_bDelDataFlag = false;
    m_pNewNode = NULL;
}

TAvlTree::~TAvlTree()
{
    if(m_pRootNode!=NULL)
    {
        ClearData(m_pRootNode,false);
    }
}

// 插入例程
TAvlNode * TAvlTree::Add(const TBaseObject *pData)
{
    m_pNewNode = NULL;
    m_pRootNode = InsertRight(m_pRootNode,pData);
    if (m_bAvlFlag != 0)
    {
        m_iAvlHeight += m_bAvlFlag;
    }

    return m_pNewNode;
}

//删除例程
bool TAvlTree::Delete(const TBaseObject *pData,const bool bDelDataFlag)
{
    m_bDelDataFlag = bDelDataFlag;
    m_pRootNode = DeleteRight(m_pRootNode,pData);
    if (m_bAvlFlag != 0)
    {
        m_iAvlHeight += m_bAvlFlag;
    }
    return true;
}

//二叉树查找例程
TAvlNode * TAvlTree::Seek(const TBaseObject *pData)
{
    //定义根结点
    TAvlNode *pCurrNode = m_pRootNode;

    while(pCurrNode != NULL)
    {
        int iKeyValue = pCurrNode->Compare(pData);

        if (iKeyValue > 0)
            pCurrNode = pCurrNode->LeftNode;
        else if (iKeyValue < 0)
            pCurrNode = pCurrNode->RightNode;
        else
            return pCurrNode;
    }
    return NULL;
}


//清除数据
void TAvlTree::Clear(const bool bDelDataFlag)
{
    ClearData(m_pRootNode,bDelDataFlag);
}

//清除数据
void TAvlTree::ClearData(TAvlNode *pNode,const bool bDelDataFlag)
{
    TAvlNode *pCurrNode = pNode;

    if (pNode == NULL)
    {
        //定义根结点
        pCurrNode = m_pRootNode;
    }

    if(TBaseObject::Assigned(pCurrNode))
    {
        if(pCurrNode->LeftNode != NULL)
        {
            ClearData(pCurrNode->LeftNode,bDelDataFlag);
            pCurrNode->LeftNode = NULL;
        }

        if(bDelDataFlag && TBaseObject::Assigned(pCurrNode->Data))
        {
            DELETE(pCurrNode->Data);
        }

        if(pCurrNode->RightNode != NULL)
        {
            ClearData(pCurrNode->RightNode,bDelDataFlag);
            pCurrNode->RightNode = NULL;
        }

        //UR:207563
        if(pCurrNode == m_pRootNode)
        {
            m_pRootNode = NULL;
        }
        
        DELETE(pCurrNode);
    }
    
}

void TAvlTree::PrintData(TAvlNode *pNode,const int iLevel)
{
    TAvlNode *pCurrNode = pNode;

    if (pNode == NULL)
    {
        //定义根结点
        pCurrNode = m_pRootNode;
    }

    if(pCurrNode != NULL)
    {
        if(pCurrNode->LeftNode != NULL)
        {
            PrintData(pCurrNode->LeftNode,iLevel+1);
        }
		
		#ifdef _ASSERT_PARAM
			if(NULL == pCurrNode->Data)
			{
				cout<<"Data is null"<<endl;//输出提示信息
			}
			else
			{
				printf("LEVEL=[%2d],BALANCE=[%2d],TEXT=[%s]\n", iLevel,pCurrNode->Balance,pCurrNode->Data->ToString().c_str());
			}
		#else
			 printf("LEVEL=[%2d],BALANCE=[%2d],TEXT=[%s]\n", iLevel,pCurrNode->Balance,pCurrNode->Data->ToString().c_str());
		#endif

        //printf("LEVEL=[%2d],BALANCE=[%2d],TEXT=[%s]\n", iLevel,pCurrNode->Balance,pCurrNode->Data->ToString().c_str());

        if(pCurrNode->RightNode != NULL)
        {
            PrintData(pCurrNode->RightNode,iLevel+1);
        }
    }

}

//新增结点
TAvlNode * TAvlTree::NewNode(const TBaseObject *pData)
{
    return m_pNewNode = new TAvlNode(pData);
}   

//插入左结点
TAvlNode * TAvlTree::InsertRight(TAvlNode *pParentNode,const TBaseObject *pData)
{
    int iBalance;

    //当前节点为空
    if (pParentNode == NULL)
    {
        pParentNode = NewNode(pData); /* 建立新节点 */
        m_bAvlFlag = 1; /* 高度增加 */
        return pParentNode;
    }

    iBalance = pParentNode->Balance;

    int iKeyValue = pParentNode->Compare(pData);

    if (iKeyValue > 0)
    {
        pParentNode->LeftNode = InsertRight(pParentNode->LeftNode,pData);
        pParentNode->Balance -= m_bAvlFlag; /* 左子树高度增加 */
    } 
    else if (iKeyValue < 0)
    {
        pParentNode->RightNode = InsertRight(pParentNode->RightNode,pData);
        pParentNode->Balance   += m_bAvlFlag; /* 右子树高度增加 */
    }
    else
    { 
        //键已经在表中 
        m_bAvlFlag = 0;
    }

    if (m_bAvlFlag != 0) 
    {
        pParentNode = BalanceTree(pParentNode,iBalance);
    }

    return pParentNode;
}

TAvlNode * TAvlTree::DeleteRight(TAvlNode *pParentNode,const TBaseObject *pData)
{
    TAvlNode *pNode=NULL; //357111 for cppcheck
    TBaseObject *pTempData=NULL; //357111 for cppcheck
    int iBalance;

    //当前节点为空
    if(pParentNode == NULL) 
    { 
        m_bAvlFlag = 0;
        return pParentNode;
    }

    iBalance = pParentNode->Balance;

    int iKeyValue = pParentNode->Compare(pData);

    // 键找到
    if (iKeyValue == 0) 
    {    
        //有一个子树为空
        if (pParentNode->LeftNode == NULL) 
        {
            pNode = pParentNode;
            pParentNode = pParentNode->RightNode;
            FreeNode(pNode);
            m_bAvlFlag = -1; /* 高度减少 */
            return pParentNode; 
        }
        else if (pParentNode->RightNode == NULL) 
        {
            pNode = pParentNode;
            pParentNode = pParentNode->LeftNode;
            FreeNode(pNode);
            m_bAvlFlag = -1; /* 高度减少 */
            return pParentNode; 
        } 
        else 
            //没有一个子树为空 
        {        
            if(pParentNode->Balance<0) 
            {
                /* 找到前驱节点 */
                pNode = pParentNode->LeftNode; 
                while (pNode->RightNode != NULL)
                    pNode = pNode->RightNode;

                /* 交换数据区 */
                pTempData = pParentNode->Data;
                pParentNode->Data = pNode->Data;
                pNode->Data = pTempData;

                pParentNode->LeftNode = DeleteRight(pParentNode->LeftNode,pNode->Data);
                pParentNode->Balance -= m_bAvlFlag; /* 左子树高度减少 */
            } 
            else {
                /* 找到后继节点 */
                pNode = pParentNode->RightNode; 
                while (pNode->LeftNode != NULL)
                    pNode = pNode->LeftNode;

                /* 交换数据区 */
                pTempData = pParentNode->Data;
                pParentNode->Data = pNode->Data;
                pNode->Data = pTempData;

                pParentNode->RightNode = DeleteRight(pParentNode->RightNode,pNode->Data);
                pParentNode->Balance += m_bAvlFlag; /* 右子树高度减少 */
            }
        }
    }        
    /* 当前节点不是要删除的，继续查找 */
    else if(iKeyValue < 0 ) 
    {
        pParentNode->RightNode = DeleteRight(pParentNode->RightNode,pData);
        pParentNode->Balance  += m_bAvlFlag; /* 右子树高度减少 */
    }
    else 
    {
        pParentNode->LeftNode  = DeleteRight(pParentNode->LeftNode,pData);
        pParentNode->Balance  -= m_bAvlFlag; /* 左子树高度减少 */
    }

    if (ABS(m_bAvlFlag) >= m_iStep)
    {
        pParentNode = BalanceTree(pParentNode,iBalance);
    }

    return pParentNode;
}

//ZMP:315426,圈复杂度代码整改
TAvlNode * TAvlTree::BalanceTree(TAvlNode *pNode, int Balance)
{
    if (m_bAvlFlag >0) 
    {
       return BalanceTreeInc(pNode,Balance);
    }
    else if(m_bAvlFlag <0)
    {   
        return BalanceTreeDec(pNode,Balance);
    }
    else
    {
        return pNode;
    }
}
//ZMP:315426,圈复杂度代码整改，begin
//二叉树增高时的平衡
TAvlNode * TAvlTree::BalanceTreeInc(TAvlNode *pNode, int Balance)
{
    //左子树过高于右子树: 需要右旋
    if(pNode->Balance < -m_iStep ) 
    {
        if(pNode->LeftNode->Balance > 0) 
        {
            if (pNode->LeftNode->Balance > 1 && pNode->LeftNode->RightNode->Balance > 0) 
                pNode->Balance += 1;
            pNode->LeftNode = LeftRotary(pNode->LeftNode);
        }
        pNode = RightRotary(pNode);
        m_bAvlFlag = 0; /* 本节点的高度不增加 */
    }
    //右子树过高于左子树: 需要左旋
    else if(pNode->Balance > m_iStep) 
    {
        if(pNode->RightNode->Balance < 0) 
        {
            if (pNode->RightNode->Balance < -1 && pNode->RightNode->LeftNode->Balance <0)
                pNode->Balance -= 1;
            pNode->RightNode = RightRotary(pNode->RightNode);
        }
        pNode = LeftRotary(pNode);
        m_bAvlFlag = 0; /* 本节点的高度不增加 */
    }
    else if( (Balance > 0 && Balance > pNode->Balance) ||
        (Balance < 0 && Balance < pNode->Balance))
    {
        m_bAvlFlag = 0; /* 本节点的高度不增加 */
    }
    else
    {
        m_bAvlFlag = 1; /* 本节点的高度增加 */
    }
    
    return pNode;
    
}
//二叉树降低时的平衡
TAvlNode * TAvlTree::BalanceTreeDec(TAvlNode *pNode, int Balance)
{
    //左子树过高于右子树: 需要右旋
    if(pNode->Balance < -m_iStep) 
    {
        if(pNode->LeftNode->Balance > 0) 
        {
            if (pNode->LeftNode->Balance > 1 && pNode->LeftNode->RightNode->Balance > 0) 
                pNode->Balance += 1;
            pNode->LeftNode = LeftRotary(pNode->LeftNode);
            m_bAvlFlag = -1;
        }
        else if (pNode->LeftNode->Balance == 0)
            m_bAvlFlag =  0;/* 本节点的高度不减少 */
        else
            m_bAvlFlag = -1;/* 本节点的高度减少 */
        pNode = RightRotary(pNode);
    }
    //右子树过高于左子树: 需要左旋 
    else if(pNode->Balance > m_iStep) 
    {
        if(pNode->RightNode->Balance < 0) 
        { 
            if (pNode->RightNode->Balance < -1 && pNode->RightNode->LeftNode->Balance <0)
                pNode->Balance -= 1;
            pNode->RightNode = RightRotary(pNode->RightNode);
            m_bAvlFlag = -1;
        }
        else if (pNode->RightNode->Balance == 0)
            m_bAvlFlag =  0;/* 本节点的高度不减少 */
        else
            m_bAvlFlag = -1;/* 本节点的高度减少 */
        pNode = LeftRotary(pNode);
    }
    else if( (Balance > 0 && Balance > pNode->Balance) ||
        (Balance < 0 && Balance < pNode->Balance))
    {
        m_bAvlFlag = -1; /* 本节点的高度减少 */
    }
    else
    {
        m_bAvlFlag = 0; /* 本节点的高度不减少 */
    }
    return pNode;
}
//ZMP:315426,圈复杂度代码整改，end

/*
* 左旋例程
*            X              Y
*           / \            / \
*          A   Y    ==>   X   C
*             / \        / \
*            B   C      A   B
*/
TAvlNode * TAvlTree::LeftRotary(TAvlNode *pNode)
{
    TAvlNode *pTempNode=pNode; //357111 for cppcheck
    int x,y;

    //pTempNode = pNode; //357111 初始化已做,此处没必要了.
    pNode = pNode->RightNode;
    pTempNode->RightNode = pNode->LeftNode;  
    pNode->LeftNode = pTempNode;

    x = pTempNode->Balance;
    y = pNode->Balance;
    /* 旋转之前：
    假定 X 的平衡因子是 x, Y 的平衡因子是 y, 
    * 设 A 的高度为 h, 则 Y 的高度为 h+x 
    * 节点 B 高度为 h+x-1-max(y,0); 
    * 节点 C 的高度为 h+x-1+MIN(y,0);
    * 旋转之后：
    * 节点 X 的新平衡因子是 x-1-max(y,0); 
    * 节点 Y 的新平衡因子是 C-(max(A,B)+1) => MIN(C-A-1,C-B-1) 
    *     => MIN(x-2+MIN(y,0),y-1)
    */
    pTempNode->Balance = x-1-MAX(y, 0);
    pNode->Balance     = MIN(x-2+MIN(y, 0), y-1);

    return pNode;
}
/*
* 右旋例程
*            X              Y
*           / \            / \
*          Y   C    ==>   A   X
*         / \                / \
*        A   B              B   C
*/
TAvlNode * TAvlTree::RightRotary(TAvlNode *pNode)
{
    TAvlNode * pTempNode=NULL;  //357111 for cppcheck
    int x,y;

    pTempNode = pNode;
    pNode = pNode->LeftNode;
    pTempNode->LeftNode = pNode->RightNode;
    pNode->RightNode = pTempNode;

    x = pTempNode->Balance;
    y = pNode->Balance;
    /* 旋转之前：
    假定 X 的平衡因子是 x, 节点 Y 的平衡因子是 y, 
    * 设 C 的高度为 h, 则 Y 的高度为 h-x
    * 节点 A 高度为 h-x-1-max(y,0); 
    * 节点 B 的高度为 h-x-1+MIN(y,0);
    * 旋转之后：
    * 节点 X 的新平衡因子是 x+1-MIN(y,0)
    * 节点 Y 的新平衡因子是 max(B,C)+1-A => max(B-A+1,C-A+1) 
    *     => max(y+1,x+2+max(y,0))
    */
    pTempNode->Balance = x+1-MIN(y, 0);
    pNode->Balance     = MAX(x+2+MAX(y, 0), y+1);

    return pNode;
}

void TAvlTree::FreeNode(TAvlNode *pNode)
{
    if(TBaseObject::Assigned(pNode->Data))
    {
        if(m_bDelDataFlag && TBaseObject::Assigned(pNode->Data))
        {
            DELETE(pNode->Data);
        }
        DELETE(pNode);
    }
}


class TAccNbrData : public TBaseObject
{
public:
    TAccNbrData(const char *sData)
    {
        strncpy(sAccNbr,sData,sizeof(sAccNbr)-1);
    }
    TAccNbrData(const llong iData)
    {
        //ZMP:467370 
        snprintf(sAccNbr, sizeof(sAccNbr), "%ld",iData);
        sAccNbr[sizeof(sAccNbr)-1] = '\0';
    }

    int Compare(int iKey) const
    {
      throw TException("TAccNbrData::Compare(int iKey) : Not Write Code.");
    }

    int Compare(int iKey1, int iKey2) const
    {
      throw TException("TAccNbrData::Compare(int iKey1, int iKey2) : Not Write Code.");
    }

    int Compare(const TBaseObject *pObject) const
    {
        llong iNum1,iNum2;
        iNum1 = atol(sAccNbr);
        iNum2 = atol(((TAccNbrData *)pObject)->sAccNbr);
        return (int)(iNum1 - iNum2);
    }
    //输出对象信息
    string ToString()
    {
        string LineText(sAccNbr);
        return LineText;
    }
    char sAccNbr[20];
};


void TAvlTree::TestAvlTree(void)
{
    TAvlTree AvlTree;
    TAccNbrData *pAccNbr=NULL; //357111 for cppcheck
    llong iNumber,iTotal,iFlag;
    char sDate[30]={0}; //357111 for cppcheck

    //随机数种子
    srand( (unsigned)time( NULL ) );

    printf("  ===== 测试平衡二叉树 =====\n");
    printf("请输入测试的数据量(1..10000000):");
    scanf("%ld",&iTotal);

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    printf("开始生成平衡二叉树(%s)...",sDate);
    for(int i=0;i<iTotal;i++)
    {
        iNumber = rand()%2000000;
        pAccNbr = new TAccNbrData(iNumber);
        if(AvlTree.Add(pAccNbr)==NULL)
        {
            DELETE(pAccNbr);
        }
    }
    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    printf(" 完成(%s).\n",sDate);

    printf("是否打印数据(是-1/否-0):");
    scanf("%ld",&iFlag);
    if(iFlag==1)
    {
        AvlTree.PrintData();
    }

    printf("请输入要查找数据(1..2000000):");
    scanf("%ld",&iFlag);
    TAccNbrData FindData(iFlag);
    TAvlNode *pAccNbrNode=NULL; //357111 for cppcheck

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    printf("开始查找数据(%s):%ld ...",sDate,iFlag);
    //ZMP:305593,style校正
    pAccNbrNode = AvlTree.Seek(&FindData);
    if(pAccNbrNode != NULL)
    {
        FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
        printf(" 完成(%s)\n",sDate);

        FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
        printf("开始删除查找的数据(%s) ...",sDate);
        if(AvlTree.Delete(&FindData,true))
        {
            FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
            printf(" 成功(%s)\n",sDate);
        }
        else
        {
            FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
            printf(" 不成功(%s)\n",sDate);
        }
    }
    else
    {
        FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
        printf(" 没找到(%s)！\n",sDate);
    }

    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    printf("开始删除所有的数据(%s) ...",sDate);
    AvlTree.Clear();
    FUNC_DT::GetCurrentTimeStr(sDate,sizeof(sDate),true);
    printf(" 完成(%s)\n",sDate);
}



//以下从Component.cpp中移入
StackInt::StackInt(int iSize)
{
    m_iStackSize = iSize;
    m_iPos = 0;
    if (m_iStackSize > 0)
        m_piData = new llong[m_iStackSize];
    else
        fprintf(stderr,"StackInt Input Param Error,iSize=[%d]\n",iSize);
}
//351951 begin
StackInt::StackInt(const StackInt& oStackInt )
{
    m_iStackSize = oStackInt.m_iStackSize;
    m_iPos = oStackInt.m_iPos;
    m_piData = new llong[oStackInt.m_iStackSize];
    for( int i = 0; i<m_iPos ; ++i)
    {
        m_piData[i] = oStackInt.m_piData[i];
    }
}
//351951 end
StackInt::~StackInt()
{
    if (m_iStackSize > 0)
        DELETE_A(m_piData);
}

bool StackInt::Push(llong iData)   //入栈
{
    bool bRet = true;
    if (m_iPos > m_iStackSize-1)
    {
        bRet = false;
        fprintf(stderr,"StackInt::Push(%ld),m_iPos=[%d],StackSize=[%d] overflow\n",iData,m_iPos,m_iStackSize);
    }
    else
    {
        m_piData[m_iPos++] = iData;
    }
    return bRet;
}

llong  StackInt::Pop()             //出栈
{
    #ifdef _ASSERT_PARAM
	if (m_iPos<=0 )
	{
		//cout<<"Access beyond the stack boundaries!\n";
		return 0;
	}
	#endif
	return m_piData[--m_iPos];
}

llong  StackInt::Top()             //读栈顶元素
{
    #ifdef _ASSERT_PARAM
	if (m_iPos<=0 )
	{
		return 0;
	}
	#endif
    return m_piData[m_iPos-1];
}

llong StackInt::GetData(int iPos) //取某个位置的值
{
    if (iPos <0 || iPos > m_iPos-1)
        return 0;
    else
        return m_piData[iPos];
}

bool StackInt::Clear()           //清空栈
{
    m_iPos = 0;
    return true;
}

bool StackInt::Delete(int iPos)  //删除栈中某个元素
{
    if (iPos <0 || iPos > m_iPos-1)
    {
        return true;
    }
    else
    {
        for (int i=iPos;i<m_iPos-1;i++)
        {
            m_piData[i] = m_piData[i+1];
        }
        m_iPos--;
    }
    return true;
}

bool StackInt::Sort(bool Asc)            //对栈中元素进行排序,默认升序,从小到大
{
    //qsort();
    llong itmp = 0;
    for (int i=0;i<m_iPos;i++)
    {
        for (int j=i+1;j<m_iPos;j++)
        {
            if (m_piData[i] < m_piData[j])
            {
                itmp = m_piData[i];
                m_piData[i] = m_piData[j];
                m_piData[j] = itmp;
            }
        }
    }
    return true;
}

int  StackInt::Find(llong iValue)  //查找值
{
    int iPos = -1;
    for (int i=0;i<m_iPos;i++)
    {
        if (iValue == m_piData[i])
        {
            iPos = i;
            break;
        }
    }
    return iPos;
}

