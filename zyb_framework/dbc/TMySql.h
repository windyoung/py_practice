#ifndef _ZX_DBCMYSQL_H_
#define _ZX_DBCMYSQL_H_

#include<time.h>
#include "MySqlType.h"
#include "BillLog.h"
#include "SOHelper.h"
#include "TDBInterface.h"

#ifdef OS_WINDOWS
  #pragma warning (disable: 4514 4786)
  #pragma warning( push, 3 )
#else
  #include <unistd.h>
#endif


#define pMySqlInstance  TMySqlFactory::GetMySqlInstance()
#define       _MYSQL_CONN_STRING_        "MYSQL"

class TMySqlException: public TDBExcpInterface
{
public:
    virtual ~TMySqlException() {}
    TMySqlException(const char *sSql, const char* sFormat, ...);
};

/*------一批函数指针---------*/
 #if !defined(__WIN__)
    #define STDCALL
 #else
    #define STDCALL __stdcall
 #endif
typedef MYSQL* STDCALL (* MySqlInit)(MYSQL *mysql);
typedef void STDCALL (*MySqlClose)(MYSQL *sock);
typedef my_bool STDCALL (*MySqlCommit)(MYSQL * mysql);
typedef my_bool STDCALL (*MySqlRollBack)(MYSQL * mysql);
typedef my_bool STDCALL (*MySqlAutoCommit)(MYSQL * mysql, my_bool auto_mode);
typedef MYSQL*  STDCALL (*MySqlRealConnect)(MYSQL *mysql, const char *host,
                                            const char *user,
                                            const char *passwd,
                                            const char *db,
                                            unsigned int port,
                                            const char *unix_socket,
                                            unsigned long clientflag);
                                            
typedef unsigned int STDCALL (*MySqlErrno)(MYSQL *mysql);
typedef const char * STDCALL (*MySqlError)(MYSQL *mysql);

typedef MYSQL_FIELD *STDCALL (*MySqlFetchFieldDirect)(MYSQL_RES *res,
                                                      unsigned int fieldnr);

typedef MYSQL_STMT * STDCALL (*MySqlStmtInit)(MYSQL *mysql);
typedef int STDCALL (*MySqlStmtPrepare)(MYSQL_STMT *stmt,
                                        const char *query,
                                        unsigned long length);
typedef int STDCALL (*MySqlStmtExecute)(MYSQL_STMT *stmt);

typedef int STDCALL (*MySqlStmtFetch)(MYSQL_STMT *stmt);
typedef int STDCALL (*MySqlStmtFetchColumn)(MYSQL_STMT *stmt,
                                            MYSQL_BIND *bind_arg,
                                            unsigned int column,
                                            unsigned long offset);
typedef int STDCALL (*MySqlStmtStoreResult)(MYSQL_STMT *stmt);
typedef MYSQL_RES *STDCALL (*MySqlStmtResultMetadata)(MYSQL_STMT *stmt);
typedef unsigned long STDCALL (*MySqlStmtParamCount)(MYSQL_STMT * stmt);
typedef my_bool STDCALL (*MySqlStmtAttrSet)(MYSQL_STMT *stmt,
                                            enum enum_stmt_attr_type attr_type,
                                            const void *attr);
typedef my_bool STDCALL (*MySqlStmtAttrGet)(MYSQL_STMT *stmt,
                                            enum enum_stmt_attr_type attr_type,
                                            void *attr);
typedef my_bool STDCALL (*MySqlStmtBindParam)(MYSQL_STMT * stmt, MYSQL_BIND * bnd);
typedef my_bool STDCALL (*MySqlStmtBindResult)(MYSQL_STMT * stmt, MYSQL_BIND * bnd);
typedef my_bool STDCALL (*MySqlStmtClose)(MYSQL_STMT * stmt);
typedef my_bool STDCALL (*MySqlStmtFreeResult)(MYSQL_STMT *stmt);
typedef unsigned int STDCALL (*MySqlStmtErrno)(MYSQL_STMT * stmt);
typedef const char* STDCALL (*MySqlStmtError)(MYSQL_STMT * stmt);

typedef my_ulonglong STDCALL (*MySqlStmtNumRows)(MYSQL_STMT *stmt);
typedef my_ulonglong STDCALL (*MySqlStmtAffectedRows)(MYSQL_STMT *stmt);
typedef unsigned int STDCALL (*MySqlStmtFieldCount)(MYSQL_STMT *stmt);
typedef MYSQL_FIELD *STDCALL (*MySqlFetchFieldDirect)(MYSQL_RES *res, unsigned int fieldnr);
typedef void    STDCALL (*MySqlFreeResult)(MYSQL_RES *result);
typedef unsigned long * STDCALL (*MySqlFetchLengths)(MYSQL_RES *result);


const char* const MYSQL_HOME_PATH_ENV    =   "MYSQL_HOME" ;       //获取MYSQL路径的环境变量,主要是获取mysqlclient库
#ifndef OS_WINDOWS
        const char* const MYSQL_LIB_NAME   =  "libmysqlclient.so" ;  //MYSQL库名称
#else
        const char* const MYSQL_LIB_NAME   =  "libmysqlclient.dll" ;  //MYSQL库名称
#endif

class TMySqlInterface
{
public:
    TMySqlInterface(void);
    virtual ~TMySqlInterface(void);
public:
    SOHelper *               m_pSoHelper;
    MySqlInit                fpMySqlInit;
    MySqlClose               fpMySqlClose;
    MySqlCommit              fpMySqlCommit;
    MySqlRollBack            fpMySqlRollBack;
    MySqlAutoCommit          fpMySqlAutoCommit;
    MySqlRealConnect         fpMySqlRealConnect;
    MySqlErrno               fpMySqlErrno;
    MySqlError               fpMySqlError;
    MySqlStmtInit            fpMySqlStmtInit;
    MySqlStmtPrepare         fpMySqlStmtPrepare;
    MySqlStmtExecute         fpMySqlStmtExecute;
    MySqlStmtFetch           fpMySqlStmtFetch;
    MySqlStmtFetchColumn     fpMySqlStmtFetchColumn;
    MySqlStmtStoreResult     fpMySqlStmtStoreResult;
    MySqlStmtResultMetadata  fpMySqlStmtResultMetadata;
    MySqlStmtParamCount      fpMySqlStmtParamCount;
    MySqlStmtAttrSet         fpMySqlStmtAttrSet;
    MySqlStmtAttrGet         fpMySqlStmtAttrGet;
    MySqlStmtBindParam       fpMySqlStmtBindParam;
    MySqlStmtBindResult      fpMySqlStmtBindResult;
    MySqlStmtClose           fpMySqlStmtClose;
    MySqlStmtFreeResult      fpMySqlStmtFreeResult;
    MySqlStmtErrno           fpMySqlStmtErrno;
    MySqlStmtError           fpMySqlStmtError;
    MySqlStmtNumRows         fpMySqlStmtNumRows;
    MySqlStmtAffectedRows    fpMySqlStmtAffectedRows;
    MySqlStmtFieldCount      fpMySqlStmtFieldCount;
    MySqlFetchFieldDirect    fpMySqlFetchFieldDirect;
    MySqlFreeResult          fpMySqlFreeResult;
    MySqlFetchLengths        fpMySqlFetchLengths;
};

class TMySqlFactory
{
public:
    static TMySqlInterface* GetMySqlInstance(void);
private:
    static TMySqlInterface *m_pInstance;
};

class TMySqlDatabase;
class TMySqlParam;
class TMySqlField;
class TMySqlQuery;
const int MAX_FIELD_COUNT = 256;

class TMySqlParam : public TDBParamInterface
{
   friend class TMySqlQuery;      
public:
    TMySqlParam();
    virtual ~TMySqlParam();
public:
    void    ClearInfo() ;
public:
    //char    m_cCharBuf;
    //enum    enum_field_types m_uiBufferType;
    //long    m_lBufferLen;
    char    m_bNullIndictor;      //标是空
    //my_bool m_bIsnull;           //用于设置绑定空值
public:
    char*   m_pName;              //参数名称
    //int     m_BufType;            //缓冲区类型
    
    int     m_iIntBuf;            //存储输入的缓冲区
    double  m_dblBuf;               //存储输入的缓冲区
    long    m_lLongBuf;           //long数据缓冲区
    long long m_llBuf;            //long long 数据缓冲区
    char*   m_StrBuf;               //字符串返回缓冲区
    //std::string  m_StrBlb;             //Blob 数据缓冲区
    
    int*    m_IntArray;           //INT数组
    double* m_DBLArray;           //DOUBLE数组
    long*   m_LongArray;          //LONG数组
    //char**  m_StrArray;            //STRING数组    
    long long* m_LLArray;         //LONG LONG数组
//    string*  m_BlbArray;          // Blog 数组
    //unsigned int m_iElemNum;      //数组元素个数    
    unsigned int m_iParamIndex;   //记录本参数是第几个参数
};

class TMySqlField : public TDBFieldInterface
{
    friend class TMySqlQuery;
public:
    TMySqlField();
    virtual ~TMySqlField();
public:
    bool   isNULL();
    char *  AsString() throw (TMySqlException);
    double  AsFloat() throw(TMySqlException);
    long long  AsInteger() throw(TMySqlException);
    time_t  AsTimeT()throw (TMySqlException);
    char*   AsBlobBuffer(int &iBufferLen) ;
    char   *AsDateTimeString() throw (TMySqlException);       
    void   AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TMySqlException);
    char*   GetFieldName();
    long    GetFieldType();
    void    ClearInfo();
protected:
    void    ClearDataBuf() ; // 释放m_pDataBuf内存
protected:
    void*   m_pDataBuf;
    long    m_lBufLen;
    char    m_bIsNull;
    TMySqlQuery* m_pParentQuery;
    char*   m_pName;       //字段名称,在实现中要new出来，大小通过获取字段长度来搞
    long    m_lType;       //数据类型
    bool    m_bNullable;   //字段定义是否允许为空值
    char    m_StrBuffer[MAX_STRING_VALUE_LENGTH];
    char*   m_sBlob;       //存放BLOB字段值
    unsigned long  m_StrLen; //当字段为字符串类型或BLOB类型时，字段值的实际长度。
    //int     m_precision; //字段的宽度
    //int     m_scale; //小数点的个数
    
};

class TMySqlQuery : public TDBQueryInterface
{
    friend class TMySqlField;
public:
    TMySqlQuery( TMySqlDatabase* pTMySqlDb);
    virtual ~TMySqlQuery();
public:
    void   Close();
    void   SetSQL(const char *sSqlstmt,int iPreFetchRows=1) throw(TMySqlException) ;
    void   CloseSQL() {  }
    void   Open(int iPrefetchRows=1) throw(TMySqlException);
    bool   Next() throw(TMySqlException);
    bool   Execute(int iters=1) throw(TMySqlException);
    int    RowsAffected();           
    bool   IsFieldExist(const char *sFieldName);
    int    ParamCount();
    TMySqlParam& Param(int iIndex);                                                         
    TMySqlParam& Param(const char*sParamName);
    bool   Eof();
    int    FieldCount();
    bool   Commit();
    bool   Rollback();
    TMySqlField&  Field(int index) throw(TMySqlException) ;
    TMySqlField&  Field(const char *sFieldName) throw(TMySqlException) ;
    /*begin zmp 841469 */
    /*直接获取值*/
    void GetValue(void *pStruct,int* Column)throw (TMySqlException);
    void SetUpdateMDBFlag(bool flag)throw (TMySqlException);
    bool GetUpdateMDBFlag()throw (TMySqlException);
    /*end zmp 841469 */
public:
    void SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused = false) throw (TMySqlException);
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TMySqlException) ;

    void SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused = false) throw (TMySqlException); 
    void SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused = false) throw (TMySqlException);
    void SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused = false) throw (TMySqlException);
    void SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused = false) throw (TMySqlException);
    void SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused = false) throw (TMySqlException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(const char *sParamName) throw (TMySqlException);     //设置参数为空
    void SetParameterTime_t(const char *sParamName,time_t tParamValue,bool isOutput_Unused = false) throw (TMySqlException);

    //简化
    void SetParameter(int iParamIndex,const char* sParamValue) throw (TMySqlException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TMySqlException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TMySqlException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TMySqlException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TMySqlException);
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TMySqlException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(int iParamIndex) throw (TMySqlException);     //设置参数为空
    void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TMySqlException);    
    
    //批量操作,MySql不支持
     void SetParamArray(const char *paramName, char  ** paramValue, int iStructSize, int iStrSize ,int iArraySize=0,bool isOutput = false) throw (TMySqlException);  
    void SetParamArray(const char *paramName, int  * paramValue, int iStructSize,  int iArraySize=0,bool isOutput = false) throw (TMySqlException); 
    void SetParamArray(const char *paramName, double * paramValue, int iStructSize,   int iArraySize=0,bool isOutput = false) throw (TMySqlException);
    void SetParamArray(const char *paramName, long  * paramValue, int iStructSize,  int iArraySize=0,bool isOutput = false) throw (TMySqlException);
    void SetParamArray(const char *paramName, long long * paramValue, int iStructSize,  int iArraySize=0,bool isOutput = false) throw (TMySqlException);
    void SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize=0,bool isOutput=false) throw (TMySqlException);//用于传入BLOB/BINARY类型字段数组参数
     //ZMP_559430
    virtual long long GetSequenceByName(const char * sName)throw (TMySqlException);
    
    static void SetBillLog(TBillLog *pBillLog); //578112
protected:
    void GetFieldsDef( ) throw (TMySqlException);
    void CheckError(const char* sInfo=NULL)throw (TMySqlException);
    TMySqlParam *ParamByName(const char *sParamName) ; 
    TMySqlParam* ParamByIndex(int iParamIndex);
    bool IsParamExist(const char *sParamName);
    void SetPrefetch( int iPreFetchRows ) throw (TMySqlException);
private:
    void   ParseSQL() ;
private:
    static TBillLog *gpBillLog; //578112
    //连接句柄
    MYSQL* m_pHandleConn;

    //MySql预处理句柄
    MYSQL_STMT* m_pHandleStmt;

    //用于存放结果集
    MYSQL_RES* m_pResult;
    
    TMySqlDatabase* m_pTMySqlDb;
    
    //参数列表
    TMySqlParam* m_pTParmList;
    int m_iParamCount;//参数数量
    
    //字段列表
    TMySqlField* m_pTFieldList;
    int m_iFieldNum;  //字段数量

    MYSQL_BIND    aParamBind[MAX_FIELD_COUNT];
    MYSQL_BIND    aFieldBind[MAX_FIELD_COUNT];
    bool m_bEofRow ;  //标识是否到达记录的最后一行
    int m_iRowAffected; //执行 增,删,改,影响的记录数
protected:
    char *m_pSqlStmt; 

private:
    unsigned  long  m_ulPrefetchRows;            //1..
};

class TMySqlDatabase  : public TDBInterface
{
    friend class TMySqlQuery;
public:
    TMySqlDatabase() ;
    virtual ~TMySqlDatabase();
public:                     
    bool Connect(bool bAutoCommit=false ) throw (TMySqlException);
    int Disconnect() throw (TMySqlException);
    bool IsConnect()throw (TMySqlException);
    void Commit();
    void Rollback();
    const char* GetProvider() {return _MYSQL_CONN_STRING_;}
    TMySqlQuery* CreateDBQuery()throw (TDBExcpInterface);
    
    void SetLogin(const char *sUser, const char *sPassword, const char *sTnsString) throw (TMySqlException);
    bool Connect(const char *usr, const char *pwd, const char *tns, bool bUnused=false) throw (TMySqlException);
    void CheckError(const char* sInfo=NULL) throw (TMySqlException);
    void SetServer(const char *pHost ="localhost", unsigned int uiPort =3306,char *sUnixSocket= NULL,unsigned long ulClientFlag=0) ;

protected:
    char m_sUsr[64];  //数据库用户名
    char m_sPwd[64];  //数据库密码
    char m_sTns[64];  //数据库实例名
private:
    MYSQL* m_pHandleConn;
    bool   m_bConnFlag;
    char * m_pHost;
    unsigned int m_uiPort ;
    char * m_pUnixSocket;
    unsigned long m_ulClientFlag; //client_flag的值通常为0
    TMySqlQuery*  m_pTMySqlQuery;
    bool m_bAutoCommit;//属性，是否自动提交
};
#endif

