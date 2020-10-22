//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#ifndef _TSOLDBQUERY_H_
#define _TSOLDBQUERY_H_

#include "TDBInterface.h"

#ifdef WIN32
#include <windows.h>
#else
//#include <sqlunix.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <solidodbc3.h>
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

#define MAX_SQL_SIZE                    4096                //执行SQL语句最大长度
#define MAX_COLUMN_COUNT                64                  //最大查询列数,参数个数
#define MAX_COLUMN_NAME                 64                  //最大列名大小
#define MAX_COLUMN_DATA                 512                 //最大列值大小
#define DEFAULT_PREFETCH_ROWS           0                   //默认FETCH记录数
#define DEFAULT_EXECUTE_ROWS            0                   //默认EXCUTE记录数

#define SIZE_TIMESTAMP                  25                  //SQL_TIMESTAMP时间格式,YYYY-MM-DD HH:MI:SS.MSEC
#define SIZE_DATE_TIME                  15                  //YYYYMMDDHHMISS

#define ODBC_FUNC_BASE                  1000
#define QUERY_BEFORE_OPEN               ODBC_FUNC_BASE+10   //查询OPEN之前状态
#define QUERY_AFTER_OPEN                ODBC_FUNC_BASE+11   //查询OPEN之后状态
#define QUERY_FETCH_BOF                 ODBC_FUNC_BASE+12   //查询FETCH有值状态
#define QUERY_FETCH_EOF                 ODBC_FUNC_BASE+13   //查询FETCH无值状态


#define _TSOLDB_CONN_STRING_              "SOLDB"

using namespace std;

extern int DBError(RETCODE rc,SQLHENV henv,SQLHDBC hdbc,SQLHSTMT hstmt,ostringstream &ostrOut);

class TSolDBException;
class TSolDBDatabase;
class TSolDBQuery;
class TSolDBField;
class TSolDBParam;
class TException;

class TSolDBException: public TDBExcpInterface
{
public:
    virtual ~TSolDBException() {}
    TSolDBException(const char *sSql, const char* sFormat, ...);
};


class TSolDBField : public TDBFieldInterface
{
    friend class TSolDBQuery;

public:
    virtual ~TSolDBField();
    bool    isNULL();
    char*   AsString() throw (TSolDBException);
    double  AsFloat() throw (TSolDBException);
    long long   AsInteger() throw (TSolDBException);
    //返回日期的各个部分
    void    AsDateTime(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TSolDBException);
    char*   AsDateTimeString() throw (TSolDBException);    						//YYYYMMDDHHMISS
    char* AsBlobBuffer(int &iBufferLen);		//输出BLOB/BINARY字段
    const char * AsName();
    int DataType(void);																			//0字符 1整形 2浮点 3 日期
    void ClearDataBuf() {}
private:
    TSolDBField();
    void ClearInfo();

    TSolDBQuery *m_pTOdbcDBQuery;								//相关QUERY指针
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
class TSolDBParam 
{
    friend class TSolDBQuery;
public:
    virtual ~TSolDBParam();
    char* GetName();

private:
    TSolDBParam();
    void ClearInfo();

    TSolDBQuery *m_pTOdbcDBQuery;								//相关QUERY指针
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

class TSolDBQuery: public TDBQueryInterface
{
    friend class TSolDBField;
    friend class TSolDBParam;
public:
    TSolDBQuery(TSolDBDatabase *pTOdbcDBDatabase);
    virtual ~TSolDBQuery();

    //关闭SQL语句，以准备接收下一个sql语句
    void Close() ;
    void CloseSQL(); //Close the cursor associated with the SELECT statement
    //设置要执行的SQL
    void SetSQL(const char *sSqlStatement,int iPreFetchRows=DEFAULT_PREFETCH_ROWS) throw (TSolDBException);
    //打开SQL SELECT语句返回结果集,iPreFetchRows无实际作用
    void Open(int prefetchRows=0) throw (TSolDBException);  //参数为了兼容接口而添加
    //移动到下一个记录
    bool Next()throw (TSolDBException);
    //执行SQL
    bool Execute(int iExecuteRows=DEFAULT_EXECUTE_ROWS) throw (TSolDBException);
    //事务开启
    bool TransBegin();
    //事务提交
    bool Commit() ;
    //事务回滚
    bool Rollback() ;
    //DELETE/UPDATE/INSERT语句修改的记录数目,SELECT语句目前Next之后的记录数
    int RowsAffected();

    //与列相关的操作
    int FieldCount();																											//获取列个数
    TSolDBField& Field(int iIndex) throw (TSolDBException);												//根据索引获取第i个列实例,从0开始
    TSolDBField& Field(const char *sFieldName) throw (TSolDBException);//根据列名获取列实例
    /*begin zmp 841469 */
    /*直接获取值*/
    void GetValue(void *pStruct,int* Column)throw (TSolDBException);
    void SetUpdateMDBFlag(bool flag)throw (TSolDBException);
    bool GetUpdateMDBFlag()throw (TSolDBException);
    /*end zmp 841469 */

    //与参数相关的操作
    int ParamCount();																											//获取参数个数
    TSolDBParam& Param(int iIndex);												//根据索引获取第i个参数实例,从0开始
    TSolDBParam& Param(const char*sParamName); //根据参数名获取参数实例

    //设置参数值
    void SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused = false) throw (TSolDBException);
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TDBExcpInterface) {}; //未实现,仅为兼容
    
    void SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused = false) throw (TSolDBException); 
    void SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused = false) throw (TSolDBException);
    void SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused = false) throw (TSolDBException);
    void SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused = false) throw (TSolDBException);
    void SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused = false) throw (TSolDBException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(const char *sParamName) throw (TSolDBException);     //设置参数为空

    //简化
    void SetParameter(int iParamIndex,const char* sParamValue) throw (TSolDBException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TSolDBException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TSolDBException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TSolDBException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TSolDBException);
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TSolDBException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(int iParamIndex) throw (TSolDBException);     //设置参数为空


    //设置数组参数值
    void SetParamArray(const char *sParamName,char **asParamValue,int iInterval,
        int iElementSize,int iArraySize,bool bOutput=false) throw (TSolDBException);
    void SetParamArray(const char *sParamName,int *aiParamValue,int iInterval,
        int iArraySize,bool bOutput=false) throw (TSolDBException); 
    void SetParamArray(const char *sParamName,long *alParamValue,int iInterval,
        int iArraySize,bool bOutput=false) throw (TSolDBException);
    void SetParamArray(const char *sParamName,double *adParamValue,int iInterval,
        int iArraySize,bool bOutput=false) throw (TSolDBException);
    void SetParamArray(const char *sParamName,long long *allParamValue,int iInterval,
        int iArraySize,bool bOutput=false) throw (TSolDBException);
    void SetBlobParamArray(const char *sParamName,char *sParamValue,int iBufferLen,
        int iArraySize,bool bOutput=false) throw (TSolDBException);//用于传入BLOB/BINARY类型字段数组参数

private:
    //检查数据库错误,有错报异常
    void CheckError(const char* sSql=NULL) throw (TSolDBException);
    //解析SQL,替换:参数为?格式
    void ParseSql(char* sSqlStatement);
    //分配选择列的缓冲区(SQL_DATA类型,精度,度量)
    void GetFieldBufferInfo(TSolDBField &oTOdbcDBField,int &iRowsCount);
    //根据参数名sParamName,参数赋值情况iValueFlag(0表示赋值,1表示未赋值,2表示忽略),符合条件的索引各数从0开始获取指定参数
    TSolDBParam* ParamByCondition(const char *sParamName,int iValueFlag=2,int iIndex=0);

    void SetParameter(TSolDBParam *pTOdbcDBParam,const char* sParamValue) throw (TSolDBException);
    void SetParameter(TSolDBParam *pTOdbcDBParam,int iParamValue) throw (TSolDBException); 
    void SetParameter(TSolDBParam *pTOdbcDBParam,long lParamValue) throw (TSolDBException);
    void SetParameter(TSolDBParam *pTOdbcDBParam,double dParamValue) throw (TSolDBException);
    void SetParameter(TSolDBParam *pTOdbcDBParam,long long llParamValue) throw (TSolDBException);
    void SetParameter(TSolDBParam *pTOdbcDBParam,const char* sParamValue,int iBufferLen) throw (TSolDBException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(TSolDBParam *pTOdbcDBParam) throw (TSolDBException);     //设置参数为空

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

    char    m_sSql[MAX_SQL_SIZE];										//要执行的SQL语句
    RETCODE rc;
    SQLHENV    henv;
    SQLHDBC    hdbc;
    SQLHSTMT   hstmt;
    TSolDBDatabase *m_pTOdbcDBDatabase;
    int m_iParamCount;																					//参数实际个数
    TSolDBParam    *m_pTOdbcDBParamArray;	//参数信息数组首址
    int m_iFieldCount;																					//选择列实际个数
    TSolDBField    *m_pTOdbcDBFieldArray;	//选择列信息数组首址
    int m_iQueryState;
    int m_iParamElementCount;														//参数值个数
    long m_lAffectRows;																				//操作涉及的记录数

    int m_iPreFetchRows;																			//SELECT批次获取游标数量
    SQLROWSETSIZE m_iFetchedRows;										//实际批次获取的记录数
    SQLUSMALLINT *aRowsStatus;													//批次获取记录的状态值数组指针
    int m_iRowIndex;																							//NEXT的记录索引

    char m_sLongLong[SIZE_LONG_LONG];						//long long存放在字符串中

};

class TSolDBDatabase: public TDBInterface
{
    friend class TSolDBQuery;
public:
    TSolDBDatabase();
    virtual ~TSolDBDatabase();

    void SetLogin(const char *sUser,const char *sPassword,const char *sServerName) throw (TSolDBException);       //设置登陆信息
    bool Connect(bool bIsAutoCommit=false) throw (TSolDBException);                            //连接数据库
    bool Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit=false) throw (TSolDBException);       //连接数据库
    int Disconnect() throw(TSolDBException);                               	//断开数据库连接
    void TransBegin();                               																								//开启事务
    void Commit() ;
    void Rollback() ;
    bool IsNullConnect()
    { 
        return m_iConnectFlag==0;
    };//是否是空的odbc连接

    void GetDataInfo(SQLHENV &henv,SQLHDBC &hdbc);													//获取数据连接信息
    bool IsConnect() ;																																			//测试数据库是否连接正常
    const char* GetProvider() {return _TSOLDB_CONN_STRING_;}
    TSolDBQuery *CreateDBQuery() throw (TSolDBException);
private:
    //检查数据库错误,有错报异常
    void CheckError(const char* sSql=NULL) throw (TSolDBException);

    string m_strUser;																																				//数据库用户名
    string m_strPassword;																																//数据库用户密码
    string m_strServerName;																														//数据库名
    int m_iConnectFlag;          																								//数据库登陆标志,0表示没有连接,1表示已经连接
    bool   m_bAutoCommitFlag;																												//自动提交标志，True自动提交，false不自动提交
    RETCODE rc;
    SQLHENV    henv;
    SQLHDBC    hdbc;
    TSolDBQuery *m_pTOdbcDBQuery;																						 //用于测试数据库是否连接正常
};
 
#endif  //_TSOLDBQUERY_H_


