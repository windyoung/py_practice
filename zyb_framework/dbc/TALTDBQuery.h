#ifndef _TALTDBQUERY_H_
#define _TALTDBQUERY_H_

#include "TDBInterface.h"
#include <sqlcli.h>
#include <ses.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

#undef min
#undef max

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include <limits.h>
#include <errno.h>
#include <time.h>
#include <string>
#include <iostream>
#include <sstream>


#ifdef _ALT

#define SQLLEN          SQLINTEGER
#define SQLULEN         SQLUINTEGER
#define SQLSETPOSIROW   SQLUSMALLINT

typedef SQLULEN         SQLROWCOUNT;
typedef SQLULEN         SQLROWSETSIZE;
typedef SQLULEN         SQLTRANSID;
typedef SQLLEN          SQLROWOFFSET;

#endif

#ifdef HP_UNIX
//  #pragma pack 8
#endif

#define SIZE_DB_ERR_MSG                 4096                //数据库操作错误信息长度
#define SIZE_DB_ERR_SQL                 4096                //数据库操作错误SQL存储长度

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

#define ISOLATION_SERIALIZABLE          0
#define ISOLATION_READ_COMMITTED        1

#define CHARLEN_MIN 					64
#define CHARLEN_MIDDLE 					512
#define CHARLEN_MAX 					4096
#define CHARLEN_XXX 					32000 

#define BLOBLEN_MIN 					2048
#define BLOBLEN_MIDDLE 					10240
#define BLOBLEN_MAX 					32000
#define BLOBLEN_XXX 					32000



#define _TTDB_CONN_STRING_              "ALTDB"

using namespace std;

extern int DBError(RETCODE rc,HENV henv,HDBC hdbc,HSTMT hstmt,ostringstream &ostrOut);


class TOdbcDBException;
class TOdbcDBDatabase;
class TOdbcDBQuery;
class TOdbcDBField;
class TOdbcDBParam;
class TException;

class TOdbcDBException: public TDBExcpInterface
{
public:    
    TOdbcDBException(const char *sSql, const char* sFormat, ...);
	virtual ~TOdbcDBException() {}
    virtual char * ToString() const;

private:
	static	int   m_aErrCode[100];
	static	int   m_iErrCodeNum;
};


class TOdbcDBField : public TDBFieldInterface
{
    friend class TOdbcDBQuery;
public:
    virtual ~TOdbcDBField();
    bool    isNULL();
    char*   AsString() throw (TOdbcDBException);
    time_t  AsTimeT() throw (TOdbcDBException);
	double  AsFloat() throw (TOdbcDBException);
    long long   AsInteger() throw (TOdbcDBException);
    //返回日期的各个部分
    void    AsDateTime(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException);
	//YYYYMMDDHHMISS
	char*   AsDateTimeString() throw (TOdbcDBException);    						
	//输出BLOB/BINARY字段
	char*   AsBlobBuffer(int &iBufferLen);		
    const char * AsName();
	//0字符 1整形 2浮点 3 日期
    int DataType(void);																			
    void ClearDataBuf() {}
public:
	SQLBIGINT  dDataValue;											//用于AsFloat()使用
	SQLINTEGER sdwDataValue;										//用于SQL_INTEGER数据存放

	
private:
    TOdbcDBField();
    void ClearInfo();

    time_t AsDateTime_t() throw (TOdbcDBException);
    int AsDateTime_Detail(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException);

    TOdbcDBQuery *m_pTOdbcDBQuery;								//相关QUERY指针
    string strFieldName;										//选择列名称
    int iIndex;													//从1开始,参数索引在SQL语句从左至右
    SQLSMALLINT ssiSqlDataType;									//数据类型
    SQLULEN  sulPrecision;										//精度
    SQLSMALLINT ssiScale;										//度量
    SQLSMALLINT ssiNullable;									//是否可空
    char *sData;												//选择列值缓冲区
    SQLLEN slDataLen;											//选择列值缓冲区字节长度
    SQLSMALLINT ssiDataType;									//选择列值缓冲区数据类型
    SQLLEN* pslValueLen;										//选择列实际返回值长度缓冲区

    char sDataValue[MAX_COLUMN_DATA];							//用于AsSring()使用
    
    long long llDataValue;										//用于AsInteger()使用,注意SQL_SMALLINT对应SWORD,SQL_INTEGER对应SDWORD
    char sDateTime[SIZE_DATE_TIME];								//用于AsDateTimeSring()使用
    
    SWORD swDataValue;											//用于SQL_SAMLLINT数据存放

    int iRowsCount;
    int iMaxRowsCount;
};

//备注:参数类型不实现输出类型参数功能
class TOdbcDBParam : public TDBParamInterface
{
    friend class TOdbcDBQuery;
public:
    virtual ~TOdbcDBParam();
    char* GetName();

private:
    TOdbcDBParam();
    void ClearInfo();

    TOdbcDBQuery *m_pTOdbcDBQuery;								//相关QUERY指针
    string strParamName;										//参数名称
    int iIndex;													//从1开始,参数索引在SQL语句从左至右
    SQLSMALLINT ssiSqlDataType;									//数据类型
    SQLULEN  sulPrecision;										//精度
    SQLSMALLINT ssiScale;										//度量
    SQLSMALLINT ssiNullable;									//是否可空
    void *pData;												//参数值缓冲区
    SQLLEN slDataLen;											//选择列值缓冲区字节长度
    SQLSMALLINT ssiDataType;									//选择列值缓冲区数据类型
    int iElementCount;											//参数元素个数,判断是否参数赋值条件为>0赋过值
    long long llElementSize;									//参数元素大小

    int iData;
    double dData;
    SQLLEN llValueLen;

    SQLLEN *allValueLen;										//参数值实际长度,用于SetParameter()
    bool bSetValue;												//设置过参数值
	bool bIsNull;
  	bool bIsBindRightTpye; //绑定过正确的值
};

class TOdbcDBQuery: public TDBQueryInterface
{
    friend class TOdbcDBField;
    friend class TOdbcDBParam;
public:
    TOdbcDBQuery(TOdbcDBDatabase *pTOdbcDBDatabase);
    virtual ~TOdbcDBQuery();

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
    //获取列个数
    int FieldCount();																											
    //根据索引获取第i个列实例,从0开始
    TOdbcDBField& Field(int iIndex) throw (TOdbcDBException);
	//根据列名获取列实例
	TOdbcDBField& Field(const char *sFieldName) throw (TOdbcDBException);
    /*begin zmp 841469 */
    /*直接获取值*/
    void GetValue(void *pStruct,int* Column)throw (TOdbcDBException);
    void SetUpdateMDBFlag(bool flag)throw (TOdbcDBException);
    bool GetUpdateMDBFlag()throw (TOdbcDBException);
    static bool m_bUpdateMDBFlag;/*判断数据库是否允许进行更新*/
    /*end zmp 841469 */

    //与参数相关的操作    
    //获取参数个数
    int ParamCount();
	//根据索引获取第i个参数实例,从0开始
    TOdbcDBParam& Param(int iIndex);    
	//根据参数名获取参数实例
    TOdbcDBParam& Param(const char*sParamName); 
    //设置参数值
    void SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
	//未实现,仅为兼容
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TDBExcpInterface) {};    
    void SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused = false) throw (TOdbcDBException); 
    void SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
	//用于传入BLOB/BINARY类型字段
    void SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused = false) throw (TOdbcDBException);
	//设置参数为空
	void SetParameterNULL(const char *sParamName) throw (TOdbcDBException);     

//138555 begin
    void SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
//138555 end

//141877 begin
    void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException);
//141877 end

    //简化
    void SetParameter(int iParamIndex,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TOdbcDBException);
	//用于传入BLOB/BINARY类型字段
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);
	//设置参数为空
	void SetParameterNULL(int iParamIndex) throw (TOdbcDBException);     


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
	//用于传入BLOB/BINARY类型字段数组参数
    void SetBlobParamArray(const char *sParamName,char *sParamValue,int iBufferLen,	
    	int iArraySize,bool bOutput=false) throw (TOdbcDBException);
	
	/* set param value FOR AB4 */
	void setParam(int iParamIndex, SQLSMALLINT value);    

	void setParam(int iParamIndex, unsigned char value);

	void setParam(int iParamIndex, int value);

	void setParam(int iParamIndex, long value);

	void setParam(int iParamIndex, long long value);

	void setParam(int iParamIndex, float value);

	void setParam(int iParamIndex, double value);
	//#ifndef WIN32
	void setParam(int iParamIndex, SQLCHAR* valueP);
	//#endif

	//void setParam(int iParamIndex, unsigned char* valueP);/*altibase的定义中,typedef unsigned char SQLCHAR;

	void setParam(int iParamIndex, const char* valueP);

	void setParam(int iParamIndex, const void* valueP, int byteLength);

	void setParam(int iParamIndex, TIMESTAMP_STRUCT &ts);

	void setParam(int iParamIndex, DATE_STRUCT &dm);

	void setParam(int iParamIndex, TIME_STRUCT &dm);

	void setParam_Int(int iParamIndex, SQLINTEGER value);

	void setParam_Long(int iParamIndex, SQLBIGINT value);
private:
    //检查数据库错误,有错报异常
    void CheckError(const char* sSql=NULL) throw (TOdbcDBException);
    //解析SQL,替换:参数为?格式
    void ParseSql(char* sSqlStatement);
    //分配选择列的缓冲区(SQL_DATA类型,精度,度量)
    void GetFieldBufferInfo(TOdbcDBField &oTOdbcDBField,int &iRowsCount);
    //根据参数名sParamName,参数赋值情况iValueFlag(0表示赋值,1表示未赋值,2表示忽略),符合条件的索引各数从0开始获取指定参数
    TOdbcDBParam* ParamByCondition(const char *sParamName,int iValueFlag=2,int iIndex=0);

    void SetParameter(TOdbcDBParam *pTOdbcDBParam,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBParam *pTOdbcDBParam,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(TOdbcDBParam *pTOdbcDBParam,long lParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBParam *pTOdbcDBParam,double dParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBParam *pTOdbcDBParam,long long llParamValue) throw (TOdbcDBException);
	//用于传入BLOB/BINARY类型字段
	void SetParameter(TOdbcDBParam *pTOdbcDBParam,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);
	//设置参数为空
	void SetParameterNULL(TOdbcDBParam *pTOdbcDBParam) throw (TOdbcDBException);     

    virtual long long GetSequenceByName(const char * sName)throw (TOdbcDBException);
    
    int StrNocaseCmp(const char *sFirst,const char *sSecond)
    {
        return strcasecmp(sFirst,sSecond);
    }

    char*   m_sSql;
    RETCODE rc;
    HENV    henv;
    HDBC    hdbc;
    HSTMT   hstmt;
    TOdbcDBDatabase *m_pTOdbcDBDatabase;
    int m_iParamCount;										//参数实际个数				
    TOdbcDBParam    *m_pTOdbcDBParamArray;					//参数信息数组首址
    int m_iFieldCount;										//选择列实际个数
    TOdbcDBField    *m_pTOdbcDBFieldArray;					//选择列信息数组首址
    int m_iQueryState;
    int m_iParamElementCount;								//参数值个数
    long m_lAffectRows;										//操作涉及的记录数
	int m_iPrevprefetchRows;
    int m_iPreFetchRows;									//SELECT批次获取游标数量
    SQLUINTEGER m_iFetchedRows;								//实际批次获取的记录数
	SQLUSMALLINT *aRowsStatus;								//批次获取记录的状态值数组指针  
    int m_iRowIndex;										//NEXT的记录索引
	SQLLEN m_iLen; 
    char m_sLongLong[SIZE_LONG_LONG];						//long long存放在字符串中
	bool bSetStmtAttr; 										//用来判断是否执行过 SQLSetStmtAttr
	bool isEof;
	bool isPrepare;
};

class TOdbcDBDatabase: public TDBInterface
{
    friend class TOdbcDBQuery;
public:
    TOdbcDBDatabase();
    virtual ~TOdbcDBDatabase();

    void SetLogin(const char *sUser,const char *sPassword,const char *sServerName) throw (TOdbcDBException);  //设置登陆信息
    bool Connect(bool bIsAutoCommit=false) throw (TOdbcDBException);          		//连接数据库
    bool Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit=false) throw (TOdbcDBException); //连接数据库
    int Disconnect() throw(TOdbcDBException);                               		//断开数据库连接
    void TransBegin();                               								//开启事务
    void Commit() ;
    void Rollback() ;
    bool IsNullConnect()
    { 
        return m_iConnectFlag==0;
    };//是否是空的odbc连接

    void GetDataInfo(HENV &henv,HDBC &hdbc);										//获取数据连接信息
    bool IsConnect() ;																//测试数据库是否连接正常
    const char* GetProvider() {return _TTDB_CONN_STRING_;}
    TOdbcDBQuery *CreateDBQuery() throw (TOdbcDBException);

	//检查数据库错误,有错报异常
    void CheckError(const char* sSql=NULL) throw (TOdbcDBException);

	int m_iConnectFlag;          													//数据库登陆标志,0表示没有连接,1表示已经连接
    bool   m_bAutoCommitFlag;														//自动提交标志，True自动提交，false不自动提交
    RETCODE rc;
    HENV    henv;
    HDBC    hdbc;
    string m_strUser;																//数据库用户名
    string m_strPassword;															//数据库用户密码
    string m_strServerName;															//数据库名   
    
	SQLHSTMT     m_stmtCommit;
	SQLHSTMT     m_stmtRollback;
private:
	TOdbcDBQuery *m_pTOdbcDBQuery;													//用于测试数据库是否连接正常

};

#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif


