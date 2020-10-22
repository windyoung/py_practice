//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#ifdef  _WIN32
#pragma warning(disable:4312)           //unsigned int 或者int 转换到HANDLE
#pragma warning(disable:4311)           //void*转换到long
#endif

#define NEW_HASH_SIZE 16384
#ifdef LONG64
    #define NEW_HASH(p) (((unsigned long)(p) >> 8) % NEW_HASH_SIZE)
#else
    #define NEW_HASH(p) (((unsigned     )(p) >> 8) % NEW_HASH_SIZE)
#endif

//ZMP:467375     
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

struct TNewInfo
{
    TNewInfo * Next;
    char       File[255];
    int        Line;
    size_t     Size;
};
static TNewInfo * NewpNewAddrList[NEW_HASH_SIZE];
char TestAddress[120];
size_t TestSize;
bool AutoCheckFlag;

class TAutoCheck {
public:
    TAutoCheck();
    ~TAutoCheck(void);
};

bool CheckHashNew(FILE *fp)
{
    if(AutoCheckFlag==false) return false;
    bool fLeaked = false;
    char sFileLine[400]={0}; //357131 for cppcheck
    for (int i = 0; i < NEW_HASH_SIZE; ++i)
    {
        TNewInfo* pNewAddr = NewpNewAddrList[i];

        if (pNewAddr == NULL) continue;

        fLeaked = true;
        while (pNewAddr)
        {  
           if (0!=pNewAddr->Line)
           {
               snprintf(sFileLine, sizeof(sFileLine), "%s:%d",pNewAddr->File,pNewAddr->Line);
               sFileLine[sizeof(sFileLine)-1] = '\0';
               fprintf(fp?fp:stdout,"Memory block 0x%p size %-5u file %-30s\n",(char*)pNewAddr + sizeof(TNewInfo),pNewAddr->Size,sFileLine);
           }
           pNewAddr = pNewAddr->Next;
        }
    }
    return fLeaked;
}

void* operator new(size_t ASize, const char* AFile, int ALine)
{
    size_t s = ASize + sizeof(TNewInfo);

    TNewInfo* pNewAddr = (TNewInfo *)malloc(s);
    if (pNewAddr == NULL)
    {
        fprintf(stderr, "New:  Out of memory when allocating %u Bytes\n",ASize);
        abort();
    }

    memset(pNewAddr,'*',sizeof(TNewInfo));
    void * APointer = (char*)pNewAddr + sizeof(TNewInfo);
    memset(APointer,0,ASize);

    size_t iHashIndex = NEW_HASH(APointer);

    //结构信息填充
    strncpy(pNewAddr->File, AFile, sizeof(pNewAddr->File)-1);
    pNewAddr->File[sizeof(pNewAddr->File)-1] = '\0';
    pNewAddr->Line = ALine;
    pNewAddr->Size = ASize;
    pNewAddr->Next = NewpNewAddrList[iHashIndex];
    NewpNewAddrList[iHashIndex] = pNewAddr;

    if(ASize==TestSize)
    {
        printf("Memory Allocated Debug:  Allocated  %p (size %u, %s:%d)\n",APointer, ASize, AFile, ALine);
    }
    
    if(strlen(TestAddress)>0)
    {
        char sAddress[120]={0}; //357131 for cppcheck
        //ZMP:467375  
        snprintf(sAddress, sizeof(sAddress), "%p",APointer);
        sAddress[sizeof(sAddress)-1] = '\0';

        if(strcmp(sAddress,TestAddress)==0)
        {
            printf("Memory Allocated Debug:  Allocated  %p (size %u, %s:%d)\n",APointer, ASize, AFile, ALine);
        }
    }

    return APointer;
}

void* operator new[](size_t ASize, const char* AFile, int ALine)
{
    return operator new(ASize, AFile, ALine);
}

void* operator new(size_t ASize)
{
    return operator new(ASize, "<Unknown>", 0);
}

void* operator new[](size_t ASize)
{
    return operator new(ASize);
}

void operator delete(void* APointer)
{
    if (APointer == NULL)
        return;

    size_t iHashIndex = NEW_HASH(APointer);
    TNewInfo* pNewAddr = NewpNewAddrList[iHashIndex];
    TNewInfo* pNewLastAddr = NULL;

    while (pNewAddr)
    {
        if ((char*)pNewAddr + sizeof(TNewInfo) == APointer)
        {
            if (pNewLastAddr == NULL)
                NewpNewAddrList[iHashIndex] = pNewAddr->Next;
            else
                pNewLastAddr->Next = pNewAddr->Next;

            free(pNewAddr);
            pNewAddr = NULL; //357131 for cppcheck

            return;
        }

        pNewLastAddr = pNewAddr;
        pNewAddr = pNewAddr->Next;
    }

    fprintf(stderr, "Delete: Invalid APointer %p\n", APointer);
    //abort();
}

void operator delete[](void* APointer)
{
    operator delete(APointer);
}

TAutoCheck::TAutoCheck(void)
{
    TestSize = -1;
    memset(TestAddress,0,sizeof(TestAddress));
    AutoCheckFlag = true;        
}

TAutoCheck::~TAutoCheck(void)
{
    if(AutoCheckFlag)
    {
        CheckHashNew(NULL);        
    }
}

TAutoCheck AutoCheck;