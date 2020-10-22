#include "TQmdbInterfaceAdapter.h"
#include "Interface/mdbQuery.h"
#include "Common.h"

TQmdbDBAdapter::TQmdbDBAdapter()
{
    try
    {
        m_pMdbDatabase = new TMdbDatabase();
    }
    catch(...)
    {
        throw;
    }
}

TQmdbDBAdapter::TQmdbDBAdapter(TQmdbDBAdapter & adapter)
{
    try
    {
        m_pMdbDatabase = new TMdbDatabase();
    }
    catch(...)
    {
        throw;
    }
}

TQmdbDBAdapter::~TQmdbDBAdapter()
{
    if(m_pMdbDatabase != NULL)
    {
        delete m_pMdbDatabase;
        m_pMdbDatabase = NULL;
    }
}

void TQmdbDBAdapter::SetLogin(const char *sUser,const char *sPassword,const char *sServerName)
{
    try
    {
        m_pMdbDatabase->SetLogin(sUser, sPassword, sServerName);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

bool TQmdbDBAdapter::Connect(bool bIsAutoCommit)
{
    PS_BEGIN(QmdbDBAdapter_Connect1,this);
    try
    {
        bool bRet = m_pMdbDatabase->Connect(bIsAutoCommit);
        PS_END(QmdbDBAdapter_Connect1);
        return bRet;
    }
    catch(...)
    {
        PS_END(QmdbDBAdapter_Connect1);
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

bool TQmdbDBAdapter::Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit)
{
    PS_BEGIN(QmdbDBAdapter_Connect2,this);
    try
    {
        bool bRet = m_pMdbDatabase->Connect(sUser, sPassword, sServerName, bIsAutoCommit);
        PS_END(QmdbDBAdapter_Connect2);
        return bRet;
    }
    catch(...)
    {
        PS_END(QmdbDBAdapter_Connect2);
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

int TQmdbDBAdapter::Disconnect()
{
    PS_BEGIN(QmdbDBAdapter_Disconnect,this);
    try
    {
        int iRet = m_pMdbDatabase->Disconnect();
        PS_END(QmdbDBAdapter_Disconnect);
        return iRet;
    }
    catch(...)
    {
        PS_END(QmdbDBAdapter_Disconnect);
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbDBAdapter::Commit()
{
    PS_BEGIN(QmdbDBAdapter_Commit,this);
    try
    {
        m_pMdbDatabase->Commit();
        PS_END(QmdbDBAdapter_Commit);
    }
    catch(...)
    {
        PS_END(QmdbDBAdapter_Commit);
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbDBAdapter::Rollback()
{
    PS_BEGIN(QmdbDBAdapter_Rollback,this);
    try
    {
        m_pMdbDatabase->Rollback();
        PS_END(QmdbDBAdapter_Rollback);
    }
    catch(...)
    {
        PS_END(QmdbDBAdapter_Rollback);
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

bool TQmdbDBAdapter::IsConnect()
{
    PS_BEGIN(QmdbDBAdapter_IsConnect,this);
    try
    {
        bool bRet =  m_pMdbDatabase->IsConnect();
        PS_END(QmdbDBAdapter_IsConnect);
        return bRet;
    }
    catch(...)
    {
        PS_END(QmdbDBAdapter_IsConnect);
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

TQmdbQueryAdapter *TQmdbDBAdapter::CreateDBQuery()
{
    try
    {
        return new TQmdbQueryAdapter(m_pMdbDatabase->CreateDBQuery());
    }
    catch(...)
    {
        throw;
    }
}


TQmdbQueryAdapter::TQmdbQueryAdapter(TMdbQuery * pMdbQuery)
{
    m_pMdbQuery = pMdbQuery;
}

TQmdbQueryAdapter::~TQmdbQueryAdapter()
{
    if(m_pMdbQuery != NULL)
    {
        delete m_pMdbQuery;
        m_pMdbQuery = NULL;
    }
}

void TQmdbQueryAdapter::CloseSQL()
{
    PS_BEGIN(QmdbQueryAdapter_CloseSQL,this);
    try
    {
        m_pMdbQuery->CloseSQL();
        PS_END(QmdbQueryAdapter_CloseSQL);
    }
    catch(...)
    {
        PS_END(QmdbQueryAdapter_CloseSQL);
        throw; //重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::Close()
{
    PS_BEGIN(QmdbQueryAdapter_Close,this);
    try
    {
        m_pMdbQuery->Close();
        PS_END(QmdbQueryAdapter_Close);
    }
    catch(...)
    {
        PS_END(QmdbQueryAdapter_Close);
        throw; //重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetSQL(const char *sSqlStatement,int iPreFetchRows)
{
    PS_BEGIN(QmdbQueryAdapter_SetSQL1,this);
    try
    {
        m_pMdbQuery->SetSQL(sSqlStatement, iPreFetchRows);
        PS_END(QmdbQueryAdapter_SetSQL1);
    }
    catch(...)
    {
        PS_END(QmdbQueryAdapter_SetSQL1);
        throw; //重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetSQL(const char *sSqlStatement,int iSqlFlag,int iPreFetchRows)
{
    PS_BEGIN(QmdbQueryAdapter_SetSQL2,this);
    try
    {
        m_pMdbQuery->SetSQL(sSqlStatement, iSqlFlag, iPreFetchRows);
        PS_END(QmdbQueryAdapter_SetSQL2);
    }
    catch(...)
    {
        PS_END(QmdbQueryAdapter_SetSQL2);
        throw; //重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::Open(int prefetchRows)
{

#ifdef OS_IBM//AIX上gettimeofday执行较慢,不统计
    m_pMdbQuery->Open(prefetchRows);
#else
    PS_BEGIN(QmdbQueryAdapter_Open,this);
    try
    {
        m_pMdbQuery->Open(prefetchRows);
        PS_END(QmdbQueryAdapter_Open);
    }
    catch(...)
    {
        PS_END(QmdbQueryAdapter_Open);
        throw; //重新抛出异常,目的是保证原有代码结构不变
    }
#endif

}

bool TQmdbQueryAdapter::Next()
{

#ifdef OS_IBM//AIX上gettimeofday执行较慢
    return m_pMdbQuery->Next();
#else
    PS_BEGIN(QmdbQueryAdapter_Next,this);
    try
    {
        bool bRet = m_pMdbQuery->Next();
        PS_END(QmdbQueryAdapter_Next);
        return bRet;
    }
    catch(...)
    {
        PS_END(QmdbQueryAdapter_Next);
        throw; //重新抛出异常,目的是保证原有代码结构不变
    }
#endif

}

bool TQmdbQueryAdapter::Commit()
{
    PS_BEGIN(QmdbQueryAdapter_Commit,this);
    try
    {
        bool bRet = m_pMdbQuery->Commit();
        PS_END(QmdbQueryAdapter_Commit);
        return bRet;
    }
    catch(...)
    {
        PS_END(QmdbQueryAdapter_Commit);
        throw; //重新抛出异常,目的是保证原有代码结构不变
    }
}

bool TQmdbQueryAdapter::Execute(int iExecuteRows)
{
    PS_BEGIN(QmdbQueryAdapter_Execute,this);
    try
    {
        bool bRet = m_pMdbQuery->Execute(iExecuteRows);
        PS_END(QmdbQueryAdapter_Execute);
        return bRet;
    }
    catch(...)
    {
        PS_END(QmdbQueryAdapter_Execute);
        throw; //重新抛出异常,目的是保证原有代码结构不变
    }
}

bool TQmdbQueryAdapter::Rollback()
{
    PS_BEGIN(QmdbQueryAdapter_Rollback,this);
    try
    {
        bool bRet = m_pMdbQuery->Rollback();
        PS_END(QmdbQueryAdapter_Rollback);
        return bRet;
    }
    catch(...)
    {
        PS_END(QmdbQueryAdapter_Rollback);
        throw; //重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::GetValue(void *pStruct,int* Column)
{

#ifdef OS_IBM//AIX上gettimeofday执行较慢
    m_pMdbQuery->GetValue(pStruct,Column);
#else
    PS_BEGIN(QmdbQueryAdapter_GetValue,this);
    try
    {
        m_pMdbQuery->GetValue(pStruct,Column);
        PS_END(QmdbQueryAdapter_GetValue);
    }
    catch(...)
    {
        PS_END(QmdbQueryAdapter_GetValue);
        throw; //重新抛出异常,目的是保证原有代码结构不变
    }
#endif

}

int TQmdbQueryAdapter::RowsAffected()
{
    try
    {
        return m_pMdbQuery->RowsAffected();
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

TMdbField& TQmdbQueryAdapter::Field(const char *sFieldName)
{
    try
    {
        return m_pMdbQuery->Field(sFieldName);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused)
{
    try
    {
        m_pMdbQuery->SetParameter(sParamName, sParamValue, isOutput_Unused);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParameter(const char *sParamName, const char cParamValue, bool isOutput)
{
    try
    {
        m_pMdbQuery->SetParameter(sParamName, cParamValue, isOutput);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused)
{
    try
    {
        m_pMdbQuery->SetParameter(sParamName, iParamValue, isOutput_Unused);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused)
{
    try
    {
        m_pMdbQuery->SetParameter(sParamName, lParamValue, isOutput_Unused);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused)
{
    try
    {
        m_pMdbQuery->SetParameter(sParamName, dParamValue, isOutput_Unused);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused)
{
    try
    {
        m_pMdbQuery->SetParameter(sParamName, llParamValue, isOutput_Unused);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused)
{
    try
    {
        m_pMdbQuery->SetParameter(sParamName, sParamValue, isOutput_Unused);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParameterNULL(const char *sParamName)
{
    try
    {
        m_pMdbQuery->SetParameterNULL(sParamName);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParamArray(const char *sParamName,char **asParamValue,int iInterval,
                   int iElementSize,int iArraySize,bool bOutput,bool * bNullArr)
{
    try
    {
        m_pMdbQuery->SetParamArray(sParamName, asParamValue, iInterval, iElementSize, iArraySize, bOutput, bNullArr);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParamArray(const char *sParamName,int *aiParamValue,int iInterval,
                   int iArraySize,bool bOutput,bool * bNullArr)
{
    try
    {
        m_pMdbQuery->SetParamArray(sParamName, aiParamValue, iInterval, iArraySize, bOutput, bNullArr);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}
void TQmdbQueryAdapter::SetParamArray(const char *sParamName,long *alParamValue,int iInterval,
                   int iArraySize,bool bOutput,bool * bNullArr)
{
    try
    {
        m_pMdbQuery->SetParamArray(sParamName, alParamValue, iInterval, iArraySize, bOutput, bNullArr);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetParamArray(const char *sParamName,double *adParamValue,int iInterval,
                   int iArraySize,bool bOutput,bool * bNullArr)
{
    try
    {
        m_pMdbQuery->SetParamArray(sParamName, adParamValue, iInterval, iArraySize, bOutput, bNullArr);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}
void TQmdbQueryAdapter::SetParamArray(const char *sParamName,long long *allParamValue,int iInterval,
                   int iArraySize,bool bOutput,bool * bNullArr)
{
    try
    {
        m_pMdbQuery->SetParamArray(sParamName, allParamValue, iInterval, iArraySize, bOutput, bNullArr);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

void TQmdbQueryAdapter::SetBlobParamArray(const char *sParamName,char *sParamValue,int iBufferLen,
                       int iArraySize,bool bOutput,bool * bNullArr)
{
    try
    {
        m_pMdbQuery->SetBlobParamArray(sParamName, sParamValue, iBufferLen, iArraySize, bOutput, bNullArr);
    }
    catch(...)
    {
        throw;//重新抛出异常,目的是保证原有代码结构不变
    }
}

