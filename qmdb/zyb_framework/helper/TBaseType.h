//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
/****************************************************************************************
*@Copyrights  2008，中兴软创（南京）计算机有限公司 开发部 CCB项目--C++框架组
*@                   All rights reserved.
*@Name：	    TBaseType.h		
*@Description： 定义基本数据类型
*@Author:		li.shugang
*@Date：	    2008年11月25日
*@History:
******************************************************************************************/
#ifndef __ZXSOFT_BASE_DATE_TYPE_H__
#define __ZXSOFT_BASE_DATE_TYPE_H__


//定义基本数据类型
#ifndef _WIN32
	typedef unsigned int DWORD;
#endif

typedef unsigned char BYTE;
typedef unsigned char BYTE8;
typedef unsigned char UINT8;
typedef signed   char INT8;

typedef unsigned short int WORD;
typedef unsigned short int WORD16;
typedef unsigned short int UINT16;
typedef signed   short int INT16;

typedef unsigned int WORD32;
typedef unsigned int UINT32;
typedef signed   int INT32;

typedef signed  int SLONG32;

#ifndef _WIN32
	typedef int LONG;
#endif

#ifndef _WIN32
	typedef unsigned long long	WORD64;
#ifndef HP_UINT64
    #ifdef __LP64__
        typedef unsigned long           UINT64;
    #else
        typedef unsigned long long      UINT64;
    #endif

    #ifdef HP_UNIX
        #define HP_UINT64 
    #endif
#endif
	typedef signed   long long  INT64;
#else
	typedef UINT64	            WORD64;
#endif

typedef unsigned char*		ULPSTR;
typedef ULPSTR*				LPLPSTR;

typedef void* LPVOID;

#ifndef _WIN32
	typedef void*          HANDLE;
	typedef void           VOID;
	typedef unsigned char* ZX_LPSTR;
#endif


#endif //__ZXSOFT_BASE_DATE_TYPE_H__
