//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
//TMathFunc.h

#ifndef __T_MATH_FUNC_H__
#define __T_MATH_FUNC_H__

#include <string.h>
#include <string>
#include <cstring>
#include "CommonDef.h"
using namespace std;

//size_t转换到int，可能丢失数据
#ifdef __OS_WINDOWS__
#pragma warning(disable:4267)
#endif

class TMathFunc
{
public:
    /*整数除法,商向上取整*/
    static int DivCeil(int iDividend,int iDivisor);

    /*整数除法,商向下取整*/
    static int DivFloor(int iDividend,int iDivisor);

    //long long 相除向上取整
    static long long LLDivCeil(long long llDividend,long long llDivisor);

    //long long,相除向下取整
    static long long LLDivFloor(long long llDividend,long long llDivisor);

    /*整数比较*/
    static int IntComp(int iIn1,int iIn2);

    /*long long 类型比较*/
    static int IntComp(long long llIn1,long long llIn2);
};

#define FUNC_MATH TMathFunc

#endif //__T_MATH_FUNC_H__
