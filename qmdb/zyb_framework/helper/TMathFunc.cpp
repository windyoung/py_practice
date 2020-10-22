//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
//TMathFunc.cpp
#include "TMathFunc.h"
#include "debug_new.h"


/*
�������,������ȡ��,������̾�+1
iDividendΪ������
iDivisorΪ����
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

/*�������,������ȡ��,�������*/
int TMathFunc::DivFloor(int iDividend,int iDivisor)
{
    if(iDivisor==0)
    {
        return 0;
    }
    return (iDividend/iDivisor);
}

//long long �������ȡ��
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

//long long,�������ȡ��
long long TMathFunc::LLDivFloor(long long llDividend,long long llDivisor)
{
    if(llDivisor==0)
    {
        return 0;
    }
    return (llDividend/llDivisor);
}

/*
�����Ƚ�,�Ƚ�iIn1��iIn2�Ĵ�С
�������� 0 ��ʾiIn1=iIn2
1 ��ʾiIn1>iIn2
-1��ʾiIn1<iIn2
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
long long ���ͱȽ�,�Ƚ�llIn1��llIn2�Ĵ�С
�������� 0 ��ʾllIn1=llIn2
1 ��ʾllIn1>llIn2
-1��ʾllIn1<llIn2
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
