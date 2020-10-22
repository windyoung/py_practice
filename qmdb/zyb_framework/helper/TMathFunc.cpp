//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
//TMathFunc.cpp
#include "TMathFunc.h"
#include "debug_new.h"


/*
整数相除,商向上取整,有余就商就+1
iDividend为被除数
iDivisor为除数
*/
int TMathFunc::DivCeil(int iDividend,int iDivisor)
{
    if(iDivisor==0)
    {
        return 0;
    }
    if((iDividend%iDivisor)>0)
    {
        return (iDividend/iDivisor+1);
    }
    else
    {
        return (iDividend/iDivisor);
    }
}

/*整数相除,商向下取整,有余忽略*/
int TMathFunc::DivFloor(int iDividend,int iDivisor)
{
    if(iDivisor==0)
    {
        return 0;
    }
    return (iDividend/iDivisor);
}

//long long 相除向上取整
long long TMathFunc::LLDivCeil(long long llDividend,long long llDivisor)
{
    if(llDivisor==0)
    {
        return 0;
    }
    if((llDividend%llDivisor)>0)
    {
        return (llDividend/llDivisor+1);
    }
    else
    {
        return (llDividend/llDivisor);
    }
}

//long long,相除向下取整
long long TMathFunc::LLDivFloor(long long llDividend,long long llDivisor)
{
    if(llDivisor==0)
    {
        return 0;
    }
    return (llDividend/llDivisor);
}

/*
整数比较,比较iIn1和iIn2的大小
函数返回 0 表示iIn1=iIn2
1 表示iIn1>iIn2
-1表示iIn1<iIn2
*/
int TMathFunc::IntComp(int iIn1,int iIn2)
{
    int iRet=0;

    if(iIn1>iIn2)
    {
        iRet=1;
    }
    else if(iIn1<iIn2)
    {
        iRet=-1;
    }
    else//if(iIn1==iIn2)
    {
        iRet=0;
    }

    return iRet;
}

/*
long long 类型比较,比较llIn1和llIn2的大小
函数返回 0 表示llIn1=llIn2
1 表示llIn1>llIn2
-1表示llIn1<llIn2
*/
int TMathFunc::IntComp(long long llIn1,long long llIn2)
{
    int iRet=0;

    if(llIn1>llIn2)
    {
        iRet=1;
    }
    else if(llIn1<llIn2)
    {
        iRet=-1;
    }
    else//if(llIn1==llIn2)
    {
        iRet=0;
    }

    return iRet;
}
