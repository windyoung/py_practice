//==================================
//      
//     QMDB接口适配层(zmp1021118)
//     初期只提供对qmdb接口的性能统计，用于后台业务访问qmdb时进行性能监控
//
//==================================

#ifndef __QMDB_INTERFACE_ADAPTER_H_
#define __QMDB_INTERFACE_ADAPTER_H_

#include <cstddef>

class TMdbDatabase;
class TMdbQuery;
class TMdbField;

class TQmdbQueryAdapter;

class TQmdbDBAdapter
{
public:
    TQmdbDBAdapter();
    ~TQmdbDBAdapter();
    void SetLogin(const char *sUser,const char *sPassword,const char *sServerName);
    bool Connect(bool bIsAutoCommit=false);
    bool Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit=false);
    int Disconnect();
    void Commit() ;
    void Rollback() ;
    bool IsConnect() ;
    TQmdbQueryAdapter *CreateDBQuery();
private:
    TQmdbDBAdapter(TQmdbDBAdapter & adapter);
    TMdbDatabase * m_pMdbDatabase;
};

class TQmdbQueryAdapter
{
public:
    TQmdbQueryAdapter(TMdbQuery * pMdbQuery);
    ~TQmdbQueryAdapter();
    void Close();
    void CloseSQL();
    void SetSQL(const char *sSqlStatement,int iPreFetchRows=-1);
    void SetSQL(const char *sSqlStatement,int iSqlFlag,int iPreFetchRows);
    void Open(int prefetchRows=0);
    void GetValue(void *pStruct,int* Column);
    bool Next();
    bool Commit();
    bool Execute(int iExecuteRows=-1);
    bool Rollback();
    int  RowsAffected();
    TMdbField& Field(const char *sFieldName);
    
    void SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused = false);
    void SetParameter(const char *sParamName, const char cParamValue, bool isOutput = false);
    void SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused = false);
    void SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused = false);
    void SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused = false);
    void SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused = false);
    void SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused = false);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(const char *sParamName);
    
    void SetParamArray(const char *sParamName,char **asParamValue,int iInterval,
                       int iElementSize,int iArraySize,bool bOutput=false,bool * bNullArr = NULL);
    void SetParamArray(const char *sParamName,int *aiParamValue,int iInterval,
                       int iArraySize,bool bOutput=false,bool * bNullArr = NULL);
    void SetParamArray(const char *sParamName,long *alParamValue,int iInterval,
                       int iArraySize,bool bOutput=false,bool * bNullArr = NULL);
    void SetParamArray(const char *sParamName,double *adParamValue,int iInterval,
                       int iArraySize,bool bOutput=false,bool * bNullArr = NULL);
    void SetParamArray(const char *sParamName,long long *allParamValue,int iInterval,
                       int iArraySize,bool bOutput=false,bool * bNullArr = NULL);
    void SetBlobParamArray(const char *sParamName,char *sParamValue,int iBufferLen,
                           int iArraySize,bool bOutput=false,bool * bNullArr = NULL);

private:
    TMdbQuery * m_pMdbQuery;
};

#endif

