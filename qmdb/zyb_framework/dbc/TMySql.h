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

/*------һ������ָ��---------*/
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


const char* const MYSQL_HOME_PATH_ENV    =   "MYSQL_HOME" ;       //��ȡMYSQL·���Ļ�������,��Ҫ�ǻ�ȡmysqlclient��
#ifndef OS_WINDOWS
        const char* const MYSQL_LIB_NAME   =  "libmysqlclient.so" ;  //MYSQL������
#else
        const char* const MYSQL_LIB_NAME   =  "libmysqlclient.dll" ;  //MYSQL������
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
    char    m_bNullIndictor;      //���ǿ�
    //my_bool m_bIsnull;           //�������ð󶨿�ֵ
public:
    char*   m_pName;              //��������
    //int     m_BufType;            //����������
    
    int     m_iIntBuf;            //�洢����Ļ�����
    double  m_dblBuf;               //�洢����Ļ�����
    long    m_lLongBuf;           //long���ݻ�����
    long long m_llBuf;            //long long ���ݻ�����
    char*   m_StrBuf;               //�ַ������ػ�����
    //std::string  m_StrBlb;             //Blob ���ݻ�����
    
    int*    m_IntArray;           //INT����
    double* m_DBLArray;           //DOUBLE����
    long*   m_LongArray;          //LONG����
    //char**  m_StrArray;            //STRING����    
    long long* m_LLArray;         //LONG LONG����
//    string*  m_BlbArray;          // Blog ����
    //unsigned int m_iElemNum;      //����Ԫ�ظ���    
    unsigned int m_iParamIndex;   //��¼�������ǵڼ�������
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
    void    ClearDataBuf() ; // �ͷ�m_pDataBuf�ڴ�
protected:
    void*   m_pDataBuf;
    long    m_lBufLen;
    char    m_bIsNull;
    TMySqlQuery* m_pParentQuery;
    char*   m_pName;       //�ֶ�����,��ʵ����Ҫnew��������Сͨ����ȡ�ֶγ�������
    long    m_lType;       //��������
    bool    m_bNullable;   //�ֶζ����Ƿ�����Ϊ��ֵ
    char    m_StrBuffer[MAX_STRING_VALUE_LENGTH];
    char*   m_sBlob;       //���BLOB�ֶ�ֵ
    unsigned long  m_StrLen; //���ֶ�Ϊ�ַ������ͻ�BLOB����ʱ���ֶ�ֵ��ʵ�ʳ��ȡ�
    //int     m_precision; //�ֶεĿ��
    //int     m_scale; //С����ĸ���
    
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
    /*ֱ�ӻ�ȡֵ*/
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
    void SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused = false) throw (TMySqlException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(const char *sParamName) throw (TMySqlException);     //���ò���Ϊ��
    void SetParameterTime_t(const char *sParamName,time_t tParamValue,bool isOutput_Unused = false) throw (TMySqlException);

    //��
    void SetParameter(int iParamIndex,const char* sParamValue) throw (TMySqlException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TMySqlException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TMySqlException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TMySqlException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TMySqlException);
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TMySqlException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(int iParamIndex) throw (TMySqlException);     //���ò���Ϊ��
    void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TMySqlException);    
    
    //��������,MySql��֧��
     void SetParamArray(const char *paramName, char  ** paramValue, int iStructSize, int iStrSize ,int iArraySize=0,bool isOutput = false) throw (TMySqlException);  
    void SetParamArray(const char *paramName, int  * paramValue, int iStructSize,  int iArraySize=0,bool isOutput = false) throw (TMySqlException); 
    void SetParamArray(const char *paramName, double * paramValue, int iStructSize,   int iArraySize=0,bool isOutput = false) throw (TMySqlException);
    void SetParamArray(const char *paramName, long  * paramValue, int iStructSize,  int iArraySize=0,bool isOutput = false) throw (TMySqlException);
    void SetParamArray(const char *paramName, long long * paramValue, int iStructSize,  int iArraySize=0,bool isOutput = false) throw (TMySqlException);
    void SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize=0,bool isOutput=false) throw (TMySqlException);//���ڴ���BLOB/BINARY�����ֶ��������
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
    //���Ӿ��
    MYSQL* m_pHandleConn;

    //MySqlԤ������
    MYSQL_STMT* m_pHandleStmt;

    //���ڴ�Ž����
    MYSQL_RES* m_pResult;
    
    TMySqlDatabase* m_pTMySqlDb;
    
    //�����б�
    TMySqlParam* m_pTParmList;
    int m_iParamCount;//��������
    
    //�ֶ��б�
    TMySqlField* m_pTFieldList;
    int m_iFieldNum;  //�ֶ�����

    MYSQL_BIND    aParamBind[MAX_FIELD_COUNT];
    MYSQL_BIND    aFieldBind[MAX_FIELD_COUNT];
    bool m_bEofRow ;  //��ʶ�Ƿ񵽴��¼�����һ��
    int m_iRowAffected; //ִ�� ��,ɾ,��,Ӱ��ļ�¼��
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
    char m_sUsr[64];  //���ݿ��û���
    char m_sPwd[64];  //���ݿ�����
    char m_sTns[64];  //���ݿ�ʵ����
private:
    MYSQL* m_pHandleConn;
    bool   m_bConnFlag;
    char * m_pHost;
    unsigned int m_uiPort ;
    char * m_pUnixSocket;
    unsigned long m_ulClientFlag; //client_flag��ֵͨ��Ϊ0
    TMySqlQuery*  m_pTMySqlQuery;
    bool m_bAutoCommit;//���ԣ��Ƿ��Զ��ύ
};
#endif

