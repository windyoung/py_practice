//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/*
* debug_new.h  1.7 2003/07/03
*
* Header file for checking leakage by operator new
*
* By Wu Yongwei
*
*/

#ifndef _DEBUG_NEW_H
#define _DEBUG_NEW_H


    #ifndef NO_DEBUG_NEW

	#include <stdio.h>

    void* operator new(size_t ASize, const char* AFile, int ALine);
    void* operator new[](size_t ASize, const char* AFile, int ALine);
    void operator delete(void* pointer);
    void operator delete[](void* pointer);

    bool CheckHashNew(FILE *fp=NULL);
    extern char TestAddress[120];
    extern size_t TestSize;
    extern bool AutoCheckFlag;

    #ifdef _BORLAND_C_
        #define new DEBUG_NEW
        #define DEBUG_NEW new(__FILE__, __LINE__)
    #endif

    #endif //NO_DEBUG_NEW
#endif // _DEBUG_NEW_H

