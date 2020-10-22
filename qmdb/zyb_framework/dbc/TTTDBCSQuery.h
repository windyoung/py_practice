//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#ifndef _TTTDBCSQUERY_H_
#define _TTTDBCSQUERY_H_

#include "TDBInterface.h"

#ifdef _QM
 #include "TQMDBCSQuery.h"
#else

#include "TTTDBQuery.h"

#ifdef WIN32
#include <windows.h>
#else
//#include <sqlunix.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sql.h>

#include <timesten.h>

#include <sqltypes.h>
#include <sqlext.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <string>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#pragma warning(disable:4290)       																								//声明函数部分throw (异常类)告警
#pragma warning(disable:4267)       																								//size_t和int告警 
#pragma warning(disable:4996)       																								//声明否决
#endif

#ifdef HP_UNIX
//  #pragma pack 8
#endif

#define TIMESTEN_HOME_PATH_ENV          "TIMESTEN_HOME"     //获取timesten路径的环境变量
#define TIMESTEN_CS_LIB_NAME            "libttclient.so"     //timesten的cs库名称

#define _TTDBCS_CONN_STRING_              "TTDBCS"

/*CS模式的函数*/
typedef SQLRETURN (*CS_SQLAllocEnv)(SQLHENV* phenv) ;                  //SQLAllocEnv
typedef SQLRETURN (*CS_SQLFreeEnv)(SQLHENV henv);                      //SQLFreeEnv
typedef SQLRETURN (*CS_SQLAllocConnect)(SQLHENV henv, SQLHDBC* phdbc); //SQLAllocConnect
typedef SQLRETURN (*CS_SQLConnect)(
  SQLHDBC        hdbc,
  SQLCHAR*       szDSN,
  SQLSMALLINT    cbDSN,
  SQLCHAR*       szUID,
  SQLSMALLINT    cbUID,
  SQLCHAR*       szAuthStr,
  SQLSMALLINT    cbAuthStr);                                           //SQLConnect
typedef SQLRETURN (*CS_SQLSetConnectOption)(
  SQLHDBC        hdbc,
  SQLUSMALLINT   fOption,
  SQLULEN        vParam);                                              //SQLSetConnectOption
typedef SQLRETURN (*CS_SQLDisconnect)(
  SQLHDBC        hdbc);                                                //SQLDisconnect
typedef SQLRETURN (*CS_SQLFreeConnect)(
  SQLHDBC        hdbc);                                                //SQLFreeConnect
typedef SQLRETURN (*CS_SQLTransact)(
  SQLHENV        henv,
  SQLHDBC        hdbc,
  SQLUSMALLINT   fType);                                               //SQLTransact
typedef SQLRETURN (*CS_SQLAllocStmt)(
  SQLHDBC        hdbc,
  SQLHSTMT*      phstmt);                                              //SQLAllocStmt
typedef SQLRETURN (*CS_SQLError)(
  SQLHENV        henv,
  SQLHDBC        hdbc,
  SQLHSTMT       hstmt,
  SQLCHAR*       szSqlState,
  SQLINTEGER*    pfNativeError,
  SQLCHAR*       szErrorMsg,
  SQLSMALLINT    cbErrorMsgMax,
  SQLSMALLINT*   pcbErrorMsg);                                         //SQLError
typedef SQLRETURN (*CS_SQLFreeStmt)(
  SQLHSTMT       hstmt,
  SQLUSMALLINT   fOption);                                             //SQLFreeStmt
typedef SQLRETURN (*CS_SQLPrepare)(
  SQLHSTMT       hstmt,
  SQLCHAR*       szSqlStr,
  SQLINTEGER     cbSqlStr);                                            //SQLPrepare
typedef SQLRETURN (*CS_SQLNumParams)(
  SQLHSTMT        hstmt,
  SQLSMALLINT*    pcpar);                                              //SQLNumParams
typedef SQLRETURN (*CS_SQLDescribeParam)(
  SQLHSTMT        hstmt,
  SQLUSMALLINT    ipar,
  SQLSMALLINT*    pfSqlType,
  SQLULEN*        pcbColDef,
  SQLSMALLINT*    pibScale,
  SQLSMALLINT*    pfNullable);                                         //SQLDescribeParam
typedef SQLRETURN (*CS_SQLNumResultCols)(
  SQLHSTMT       hstmt,
  SQLSMALLINT*   pccol);                                               //SQLNumResultCols
typedef SQLRETURN (*CS_SQLSetStmtOption)(
  SQLHSTMT       hstmt,
  SQLUSMALLINT   fOption,
  SQLROWCOUNT    vParam);                                              //SQLSetStmtOption
typedef SQLRETURN (*CS_SQLDescribeCol)(
  SQLHSTMT       hstmt,
  SQLUSMALLINT   icol,
  SQLCHAR*       szColName,
  SQLSMALLINT    cbColNameMax,
  SQLSMALLINT*   pcbColName,
  SQLSMALLINT*   pfSqlType,
  SQLULEN*       pcbColDef,
  SQLSMALLINT*   pibScale,
  SQLSMALLINT*   pfNullable);                                           //SQLDescribeCol
typedef SQLRETURN (*CS_SQLBindCol)(
  SQLHSTMT       hstmt,
  SQLUSMALLINT   icol,
  SQLSMALLINT    fCType,
  SQLPOINTER     rgbValue,
  SQLLEN         cbValueMax,
  SQLLEN*        pcbValue);                                             //SQLBindCol
typedef SQLRETURN (*CS_SQLParamOptions)(
  SQLHSTMT        hstmt,
  SQLROWSETSIZE   crow,
  SQLROWSETSIZE*  pirow);                                               //SQLParamOptions
typedef SQLRETURN (*CS_SQLExecute)(
  SQLHSTMT       hstmt);                                                //SQLExecute
typedef SQLRETURN (*CS_SQLFetch)(
  SQLHSTMT       hstmt);                                                //SQLFetch

#ifdef TT_1122
typedef SQLRETURN (*CS_SQLExtendedFetch)(
  SQLHSTMT        hstmt,
  SQLUSMALLINT    fFetchType,
  SQLLEN          irow,
  SQLULEN*        pcrow,
  SQLUSMALLINT*   rgfRowStatus);
#else
typedef SQLRETURN (*CS_SQLExtendedFetch)(
  SQLHSTMT        hstmt,
  SQLUSMALLINT    fFetchType,
  SQLROWOFFSET    irow,
  SQLROWSETSIZE*  pcrow,
  SQLUSMALLINT*   rgfRowStatus);                                        //SQLExtendedFetch
#endif

typedef SQLRETURN (*CS_SQLRowCount)(
  SQLHSTMT       hstmt,
  SQLLEN*        pcrow);                                                //SQLRowCount
typedef SQLRETURN (*CS_SQLBindParameter)(
  SQLHSTMT     hstmt,
  SQLUSMALLINT ipar,
  SQLSMALLINT  fParamType,
  SQLSMALLINT  fCType,
  SQLSMALLINT  fSqlType,
  SQLULEN      cbColDef,
  SQLSMALLINT  ibScale,
  SQLPOINTER   rgbValue,
  SQLLEN       cbValueMax,
  SQLLEN*      pcbValue);                                               //SQLBindParameter

/*CS模式的函数*/

using namespace std;

extern int CS_DBError(RETCODE rc,HENV henv,HDBC hdbc,HSTMT hstmt,ostringstream &ostrOut);

class TOdbcDBCSDatabase;
class TOdbcDBCSQuery;
class TOdbcDBCSField;
class TOdbcDBCSParam;
class TException;
class SOHelper;

class TTTCSInterface
{
public:
    TTTCSInterface(void);
    virtual ~TTTCSInterface(void);
    TTTCSInterface(const TTTCSInterface& oTTTCSInterface);
public:
    //CS函数
    SOHelper *              m_pSoHelper;
    CS_SQLAllocEnv          cs_SQLAllocEnv;
    CS_SQLFreeEnv           cs_SQLFreeEnv;
    CS_SQLAllocConnect      cs_SQLAllocConnect;
    CS_SQLConnect           cs_SQLConnect;
    CS_SQLSetConnectOption  cs_SQLSetConnectOption;
    CS_SQLDisconnect        cs_SQLDisconnect;
    CS_SQLFreeConnect       cs_SQLFreeConnect;
    CS_SQLTransact          cs_SQLTransact;
    CS_SQLAllocStmt         cs_SQLAllocStmt;
    CS_SQLError             cs_SQLError;
    CS_SQLFreeStmt          cs_SQLFreeStmt;
    CS_SQLPrepare           cs_SQLPrepare;
    CS_SQLNumParams         cs_SQLNumParams;

    CS_SQLDescribeParam     cs_SQLDescribeParam;
    CS_SQLNumResultCols     cs_SQLNumResultCols;
    CS_SQLSetStmtOption     cs_SQLSetStmtOption;
    CS_SQLDescribeCol       cs_SQLDescribeCol;
    CS_SQLBindCol           cs_SQLBindCol;
    CS_SQLParamOptions      cs_SQLParamOptions;
    CS_SQLExecute           cs_SQLExecute;
    CS_SQLFetch             cs_SQLFetch;
    CS_SQLExtendedFetch     cs_SQLExtendedFetch;
    CS_SQLRowCount          cs_SQLRowCount;
    CS_SQLBindParameter     cs_SQLBindParameter;
};

class TTTCSFactory
{
public:
    static TTTCSInterface* GetTTInstance(void);
private:
    static TTTCSInterface *m_pInstance;
};

class TOdbcDBCSField : public TDBFieldInterface
{
    friend class TOdbcDBCSQuery;

public:
    virtual ~TOdbcDBCSField();
    bool    isNULL();
    char*   AsString() throw (TOdbcDBException);

////////////////////////////////////////////////////////////////////////////////
//138555 begin
    time_t AsTimeT() throw (TOdbcDBException);
//138555 end
////////////////////////////////////////////////////////////////////////////////

    double  AsFloat() throw (TOdbcDBException);
    long long   AsInteger() throw (TOdbcDBException);
    //返回日期的各个部分
    void    AsDateTime(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException);
    char*   AsDateTimeString() throw (TOdbcDBException);    						//YYYYMMDDHHMISS
    char* AsBlobBuffer(int &iBufferLen);		//输出BLOB/BINARY字段
    const char * AsName();
    int DataType(void);																			//0字符 1整形 2浮点 3 日期
    void ClearDataBuf() {}
private:
    TOdbcDBCSField();
    void ClearInfo();

    time_t AsDateTime_t() throw (TOdbcDBException);
    int AsDateTime_Detail(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException);

    TOdbcDBCSQuery *m_pTOdbcDBQuery;								//相关QUERY指针
    string strFieldName;																		//选择列名称
    int iIndex;																											//从1开始,参数索引在SQL语句从左至右
    SQLSMALLINT ssiSqlDataType;											//数据类型
    SQLULEN  sulPrecision;																//精度
    SQLSMALLINT ssiScale;																	//度量
    SQLSMALLINT ssiNullable;														//是否可空
    char *sData;																										//选择列值缓冲区
    SQLLEN slDataLen;																					//选择列值缓冲区字节长度
    SQLSMALLINT ssiDataType;														//选择列值缓冲区数据类型
    SQLLEN* pslValueLen;																		//选择列实际返回值长度缓冲区

    char sDataValue[MAX_COLUMN_DATA];					//用于AsSring()使用
    double dDataValue;																				//用于AsFloat()使用
    long long llDataValue;																//用于AsInteger()使用,注意SQL_SMALLINT对应SWORD,SQL_INTEGER对应SDWORD
    char sDateTime[SIZE_DATE_TIME];							//用于AsDateTimeSring()使用
    SDWORD sdwDataValue;																		//用于SQL_INTEGER数据存放
    SWORD swDataValue;																				//用于SQL_SAMLLINT数据存放

    int iRowsCount;
    int iMaxRowsCount;
};

//备注:参数类型不实现输出类型参数功能
class TOdbcDBCSParam : public TDBParamInterface
{
    friend class TOdbcDBCSQuery;
public:
    virtual ~TOdbcDBCSParam();
 //   char* GetName();

private:
    TOdbcDBCSParam();
    void ClearInfo();

    TOdbcDBCSQuery *m_pTOdbcDBQuery;								//相关QUERY指针
    string strParamName;																		//参数名称
    int iIndex;																											//从1开始,参数索引在SQL语句从左至右
    SQLSMALLINT ssiSqlDataType;											//数据类型
    SQLULEN  sulPrecision;																//精度
    SQLSMALLINT ssiScale;																	//度量
    SQLSMALLINT ssiNullable;														//是否可空
    void *pData;																										//参数值缓冲区
    SQLLEN slDataLen;																					//选择列值缓冲区字节长度
    SQLSMALLINT ssiDataType;														//选择列值缓冲区数据类型
    int iElementCount;																				//参数元素个数,判断是否参数赋值条件为>0赋过值
    long long llElementSize;														//参数元素大小

    int iData;
    double dData;
    SQLLEN llValueLen;

    SQLLEN *allValueLen;																		//参数值实际长度,用于SetParameter()
    bool bSetValue;																							//设置过参数值
};

class TOdbcDBCSQuery: public TDBQueryInterface
{
    friend class TOdbcDBCSField;
    friend class TOdbcDBCSParam;
public:
    TOdbcDBCSQuery(TOdbcDBCSDatabase *pTOdbcDBCSDatabase);
    virtual ~TOdbcDBCSQuery();

    //关闭SQL语句，以准备接收下一个sql语句
    void Close() ;
    void CloseSQL(); //Close the cursor associated with the SELECT statement
    //设置要执行的SQL
    void SetSQL(const char *sSqlStatement,int iPreFetchRows=DEFAULT_PREFETCH_ROWS) throw (TOdbcDBException);
    //打开SQL SELECT语句返回结果集,iPreFetchRows无实际作用
    void Open(int prefetchRows=0) throw (TOdbcDBException);  //参数为了兼容接口而添加
    //移动到下一个记录
    bool Next()throw (TOdbcDBException);
    //执行SQL
    bool Execute(int iExecuteRows=DEFAULT_EXECUTE_ROWS) throw (TOdbcDBException);
    //事务开启
    bool TransBegin();
    //事务提交
    bool Commit() ;
    //事务回滚
    bool Rollback() ;
    //DELETE/UPDATE/INSERT语句修改的记录数目,SELECT语句目前Next之后的记录数
    int RowsAffected();
	bool Eof(void) { return m_iQueryState == QUERY_FETCH_EOF;}

    //与列相关的操作
    int FieldCount();																											//获取列个数
    TOdbcDBCSField& Field(int iIndex) throw (TOdbcDBException);												//根据索引获取第i个列实例,从0开始
    TOdbcDBCSField& Field(const char *sFieldName) throw (TOdbcDBException);//根据列名获取列实例

    //与参数相关的操作
    int ParamCount();																											//获取参数个数
    TOdbcDBCSParam& Param(int iIndex);												//根据索引获取第i个参数实例,从0开始
    TOdbcDBCSParam& Param(const char*sParamName); //根据参数名获取参数实例
    /*begin zmp 841469 */
    /*直接获取值*/
    void GetValue(void *pStruct,int* Column)throw (TOdbcDBException);
    void SetUpdateMDBFlag(bool flag)throw (TOdbcDBException);
    bool GetUpdateMDBFlag()throw (TOdbcDBException);
    static bool m_bUpdateMDBFlag;/*判断数据库是否允许进行更新*/
    /*end zmp 841469 */

    //设置参数值
    void SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TDBExcpInterface) {}; //未实现,仅为兼容
    
    void SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused = false) throw (TOdbcDBException); 
    void SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused = false) throw (TOdbcDBException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(const char *sParamName) throw (TOdbcDBException);     //设置参数为空

    //简化
    void SetParameter(int iParamIndex,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(int iParamIndex) throw (TOdbcDBException);     //设置参数为空

////////////////////////////////////////////////////////////////////////////////
//138555 begin
    void SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
//138555 end
////////////////////////////////////////////////////////////////////////////////

//141877 begin
    void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException);
//141877 end

    //设置数组参数值
    void SetParamArray(const char *sParamName,char **asParamValue,int iInterval,
        int iElementSize,int iArraySize,bool bOutput=false) throw (TOdbcDBException);
    void SetParamArray(const char *sParamName,int *aiParamValue,int iInterval,
        int iArraySize,bool bOutput=false) throw (TOdbcDBException); 
    void SetParamArray(const char *sParamName,long *alParamValue,int iInterval,
        int iArraySize,bool bOutput=false) throw (TOdbcDBException);
    void SetParamArray(const char *sParamName,double *adParamValue,int iInterval,
        int iArraySize,bool bOutput=false) throw (TOdbcDBException);
    void SetParamArray(const char *sParamName,long long *allParamValue,int iInterval,
        int iArraySize,bool bOutput=false) throw (TOdbcDBException);
    void SetBlobParamArray(const char *sParamName,char *sParamValue,int iBufferLen,
        int iArraySize,bool bOutput=false) throw (TOdbcDBException);//用于传入BLOB/BINARY类型字段数组参数

private:
    //检查数据库错误,有错报异常
    void CheckError(const char* sSql=NULL) throw (TOdbcDBException);
    //解析SQL,替换:参数为?格式
    void ParseSql(char* sSqlStatement);
    //分配选择列的缓冲区(SQL_DATA类型,精度,度量)
    void GetFieldBufferInfo(TOdbcDBCSField &oTOdbcDBField,int &iRowsCount);
    //根据参数名sParamName,参数赋值情况iValueFlag(0表示赋值,1表示未赋值,2表示忽略),符合条件的索引各数从0开始获取指定参数
    TOdbcDBCSParam* ParamByCondition(const char *sParamName,int iValueFlag=2,int iIndex=0);

    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,long lParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,double dParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,long long llParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(TOdbcDBCSParam *pTOdbcDBParam) throw (TOdbcDBException);     //设置参数为空
    //ZMP_559430
    virtual long long GetSequenceByName(const char * sName)throw (TOdbcDBException);
    
    int StrNocaseCmp(const char *sFirst,const char *sSecond)
    {
#ifdef  _WIN32
        /*WINDOWS*/
        return stricmp(sFirst,sSecond);
#else
        /*UNIX*/
        return strcasecmp(sFirst,sSecond);
#endif
    }

    //char    m_sSql[MAX_SQL_SIZE];										//要执行的SQL语句
    char*    m_sSql;
    RETCODE rc;
    HENV    henv;
    HDBC    hdbc;
    HSTMT   hstmt;
    TOdbcDBCSDatabase *m_pTOdbcDBDatabase;
    int m_iParamCount;																					//参数实际个数
    TOdbcDBCSParam    *m_pTOdbcDBParamArray;	//参数信息数组首址
    int m_iFieldCount;																					//选择列实际个数
    TOdbcDBCSField    *m_pTOdbcDBFieldArray;	//选择列信息数组首址
    int m_iQueryState;
    int m_iParamElementCount;														//参数值个数
    long m_lAffectRows;																				//操作涉及的记录数

    int m_iPreFetchRows;																			//SELECT批次获取游标数量
#ifdef TT_1122
    SQLULEN  m_iFetchedRows;										//实际批次获取的记录数
#else
    SQLROWSETSIZE m_iFetchedRows;									//实际批次获取的记录数
#endif    
    SQLUSMALLINT *aRowsStatus;													//批次获取记录的状态值数组指针
    int m_iRowIndex;																							//NEXT的记录索引

    char m_sLongLong[SIZE_LONG_LONG];						//long long存放在字符串中

////////////////////////////////////////////////////////////////////////////////
//133994 begin
    int m_iPrevprefetchRows;
//133994 end
////////////////////////////////////////////////////////////////////////////////

};

class TOdbcDBCSDatabase: public TDBInterface
{
    friend class TOdbcDBCSQuery;
public:
    TOdbcDBCSDatabase();
    virtual ~TOdbcDBCSDatabase();

    void SetLogin(const char *sUser,const char *sPassword,const char *sServerName) throw (TOdbcDBException);       //设置登陆信息
    bool Connect(bool bIsAutoCommit=false) throw (TOdbcDBException);                            //连接数据库
    bool Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit=false) throw (TOdbcDBException);       //连接数据库
    int Disconnect() throw(TOdbcDBException);                               	//断开数据库连接
    void TransBegin();                               																								//开启事务
    void Commit() ;
    void Rollback() ;
    bool IsNullConnect()
    { 
        return m_iConnectFlag==0;
    };//是否是空的odbc连接

    void GetDataInfo(HENV &henv,HDBC &hdbc);													//获取数据连接信息
    bool IsConnect() ;																																			//测试数据库是否连接正常
    const char* GetProvider() {return _TTDB_CONN_STRING_;}
    TOdbcDBCSQuery *CreateDBQuery() throw (TOdbcDBException);
private:
    //检查数据库错误,有错报异常
    void CheckError(const char* sSql=NULL) throw (TOdbcDBException);

    string m_strUser;																																				//数据库用户名
    string m_strPassword;																																//数据库用户密码
    string m_strServerName;																														//数据库名
    int m_iConnectFlag;          																								//数据库登陆标志,0表示没有连接,1表示已经连接
    bool   m_bAutoCommitFlag;																												//自动提交标志，True自动提交，false不自动提交
    RETCODE rc;
    HENV    henv;
    HDBC    hdbc;
    TOdbcDBCSQuery *m_pTOdbcDBQuery;																						 //用于测试数据库是否连接正常
};

#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif  //_TTTDBCSQUERY_H_

#endif


