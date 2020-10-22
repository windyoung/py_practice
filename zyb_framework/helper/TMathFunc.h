//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
//TMathFunc.h

#ifndef __T_MATH_FUNC_H__
#define __T_MATH_FUNC_H__

#include <string.h>
#include <string>
#include <cstring>
#include "CommonDef.h"
using namespace std;

//size_tת����int�����ܶ�ʧ����
#ifdef __OS_WINDOWS__
#pragma warning(disable:4267)
#endif

class TMathFunc
{
public:
    /*��������,������ȡ��*/
    static int DivCeil(int iDividend,int iDivisor);

    /*��������,������ȡ��*/
    static int DivFloor(int iDividend,int iDivisor);

    //long long �������ȡ��
    static long long LLDivCeil(long long llDividend,long long llDivisor);

    //long long,�������ȡ��
    static long long LLDivFloor(long long llDividend,long long llDivisor);

    /*�����Ƚ�*/
    static int IntComp(int iIn1,int iIn2);

    /*long long ���ͱȽ�*/
    static int IntComp(long long llIn1,long long llIn2);
};

#define FUNC_MATH TMathFunc

#endif //__T_MATH_FUNC_H__
