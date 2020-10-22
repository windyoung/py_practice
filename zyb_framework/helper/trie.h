//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#ifndef __TRIE_H_
#define __TRIE_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define FOUND 1
#define NOTFOUND 0

class TRIE
{
public:
    TRIE *pNext[10];
    void  *pData[10];

public:
    virtual ~TRIE();
    TRIE();
} ;

typedef  void (*PtrAssignFunExt)(void **,void *);

class TrieControlHead
{
public:
    TRIE *pNode;
    static    PtrAssignFunExt pAssignExt;

public:    
    int  InsertTrieExt(char key[16],void *pValue);
    ///根就键值查找，找到返回 1 ，没有找到返回 0
    int SearchTrie(char key[17],void **pptCur);
    //返回pptCur指针的地址
    int SearchTrieEx(char key[17],void **&pptCur);
    static void InitGlobalAntiTableExt();
    virtual ~TrieControlHead();
    TrieControlHead(TRIE*);
};

#endif

