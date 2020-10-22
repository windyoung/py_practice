//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#include "trie.h"
#include <stdio.h>
#include "Common.h"
#include "debug_new.h"
#ifdef WIN32
    #pragma warning(disable:4267)
#endif

int aigCharAntiExt[256];
int aigCharAnti[256];
int igTrieNodeCnt=0;

PtrAssignFunExt TrieControlHead::pAssignExt=NULL;

void TrieControlHead::InitGlobalAntiTableExt()
{
    int c;
    for(c=0;c<=255;c++)
        if(isdigit(c))
        {
            aigCharAntiExt[c]=c-'0';
            aigCharAnti[c]=c-'0';
        }
        else 
        {
            aigCharAntiExt[c]=c%10;
            aigCharAnti[c]=c%10;
        }
}

int TrieControlHead::InsertTrieExt(char key[16],void *pValue)
{
    int i,offset,iStringLen_1;
    TRIE *ptCur=this->pNode; //357132 for cppcheck
    TRIE *ptPre=this->pNode; //357132 for cppcheck
    //void *ptPreLkHead=NULL;

    //ptCur=ptPre=this->pNode;
    iStringLen_1=(int)strlen(key)-1;//ZMP:477242

    for(i=0;i<iStringLen_1;i++)
    {
        offset=aigCharAntiExt[(unsigned char )key[i]];

        //if(ptPre->pData[offset]!=NULL) 
        //    ptPreLkHead=ptPre->pData[offset];

        if((ptCur=ptPre->pNext[offset])==NULL)
        {
            ptCur=new TRIE();        
            ptPre->pNext[offset]=ptCur;
        }
        ptPre=ptCur;          
    }

    offset=aigCharAntiExt[(unsigned char )key[i]];
    
    //¸´ÖÆ¿½±´
    (*pAssignExt)(&(ptCur->pData[offset]),pValue);

    return 0;
}

int TrieControlHead::SearchTrie(char key[17],void **pptCur)
{
    unsigned char *pts=NULL; //357132 for cppcheck
    int  iStringLen,offset;
    TRIE *ptCur=NULL,*ptPre=NULL; //357132 for cppcheck
    pts=(unsigned char *)(&key[0]);

    if((iStringLen=(int)strlen(key))==0)
        return NOTFOUND;

    *pptCur=NULL;
    ptCur=ptPre=this->pNode;

    while(iStringLen-->0)
    {
        if(ptCur==NULL)  
            break;
        offset=aigCharAntiExt[*pts++];
        //if(ptCur->pData[offset]!=NULL)
        //    *pptCur=ptCur->pData[offset];
        ptPre=ptCur;
        ptCur=ptCur->pNext[offset];
        
    }
    if (iStringLen == -1)
        *pptCur=ptPre->pData[offset];

    return (*pptCur==NULL)?NOTFOUND:FOUND;         
}

int TrieControlHead::SearchTrieEx(char key[17],void **&pptCur)
{
    unsigned char *pts=NULL; 
    int  iStringLen,offset;
    TRIE *ptCur=NULL,*ptPre=NULL;  //357132 for cppcheck
    pts=(unsigned char *)(&key[0]);

    if((iStringLen=(int)strlen(key))==0)
        return NOTFOUND;

    pptCur=NULL;
    ptCur=ptPre=this->pNode;

    while(iStringLen-->0)
    {
        if(ptCur==NULL)  
            break;
        offset=aigCharAntiExt[*pts++];
        //if(ptCur->pData[offset]!=NULL)
        //    *pptCur=ptCur->pData[offset];
        ptPre=ptCur;
        ptCur=ptCur->pNext[offset];

    }
    if (iStringLen == -1)
        pptCur=&ptPre->pData[offset];

    return (pptCur==NULL)?NOTFOUND:FOUND;         
}

TRIE::TRIE()
{
    igTrieNodeCnt++;

    for(int i=0;i<10;i++)
    {
        pNext[i]=NULL;
        pData[i]=NULL;
    }
}




TRIE::~TRIE()
{
    for(int i=0;i<10;i++)
    {
        if(pNext[i]!=NULL)
            DELETE(pNext[i]);
    }
    igTrieNodeCnt--;    
}

TrieControlHead::~TrieControlHead()
{
//    if(pNode!=NULL)
//        delete pNode;
}

TrieControlHead::TrieControlHead(TRIE * phead)
{
    pNode=phead;
}
