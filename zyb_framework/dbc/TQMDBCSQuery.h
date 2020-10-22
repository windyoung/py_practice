//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#ifndef _TQMDB_CS_QUERY_H_
#define _TQMDB_CS_QUERY_H_

#include "Interface/mdbQuery.h"
#include "TDBInterface.h"
#include "TQMDBQuery.h"

#ifdef WIN32
#include <windows.h>
#else
//#include <sqlunix.h>
#endif

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
#ifdef _WIN32
#pragma warning(disable:4290)       																								//声明函数部分throw (异常类)告警
#pragma warning(disable:4267)       																								//size_t和int告警 
#pragma warning(disable:4996)       																								//声明否决
#endif

#ifdef HP_UNIX
//  #pragma pack 8
#endif
/*
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
*/

#define _TTDBCS_CONN_STRING_              "QMDBCS"

using namespace std;

//extern int DBError(RETCODE rc,HENV henv,HDBC hdbc,HSTMT hstmt,ostringstream &ostrOut);

class TOdbcDBCSDatabase;
class TOdbcDBCSQuery;
class TOdbcDBCSField;
class TOdbcDBCSParam;
class TException;


//class TOdbcDBException;
//class TOdbcDBDatabase;
//class TOdbcDBQuery;
//class TOdbcDBField;
// //class TOdbcDBParam;
//class TException;
/*
class TOdbcDBException: public TDBExcpInterface
{
public:
    virtual ~TOdbcDBException() {}
    TOdbcDBException(){}
    TOdbcDBException(const char *sSql, const char* sFormat, ...);
};
*/

//-------------------------------------------------------------
class TOdbcDBCSField : public TDBFieldInterface
{
    friend class TOdbcDBCSQuery;
public: 
    //TOdbcDBCSField();
    virtual ~TOdbcDBCSField(); 
    bool    isNULL();           																//在fetch过程中该列的数据是否为空
    char   *AsString() throw(TOdbcDBException);
    double  AsFloat() throw (TOdbcDBException);
    long long   AsInteger() throw (TOdbcDBException);
    //Blob处理
    char*       AsBlobBuffer(int &iBufferLen) throw (TOdbcDBException);														//输出BLOB字段
    //日期处理
    char       *AsDateTimeString() throw (TOdbcDBException);   																						//把日期型的列以HH:MM:DD HH24:MI格式读取,使用asString()读取的日期型数据类型为HH:MM:DD
    void        AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TOdbcDBException); //返回日期的各个部分
    void       SetTMdbField(const TMdbClientField& field);
    
    //time_t      AsTimeT() throw (TOdbcDBException);
    //void        LoadFromFile(const char *fileName) throw (TOdbcDBException);         //写入到blob中
    //void        LoadFromBuffer(unsigned char *buf, unsigned int bufLength) throw (TOdbcDBException);    //把LOB的内容用缓冲区内容替代    
    void        ClearDataBuf();																																																		//游标循环获取指定数量数据后,未清空上次的缓存
    const char * AsName();
    int DataType(void);			
  	//ZMP:305920 begin
    time_t AsTimeT() throw (TOdbcDBException);													
private:
	TMdbClientField m_MdbField;
	
	TOdbcDBCSField();
    void ClearInfo();
private:
    char sBlobBuf[1024*4];
   
};

class TOdbcDBCSQuery: public TDBQueryInterface
{
    friend class TOdbcDBCSField;
public:
    //主要功能
    
    TOdbcDBCSQuery(TOdbcDBCSDatabase *pTOdbcDBCSDatabase) throw (TOdbcDBException);
    virtual ~TOdbcDBCSQuery();
    
    void Close();                                                   //关闭SQL语句，以准备接收下一个sql语句
    void CloseSQL();    
    //设置同步属性
    void SetRepAttr(bool bWriteBuf, bool bRepBuf);                                          //兼容odbc
    void SetSQL(const char *sSqlStatement,int iPreFetchRows=0) throw (TOdbcDBException);        //设置Sqlstatement
    void Open(int prefetchRows=PREFETCH_ROWS) throw (TOdbcDBException); //打开SQL SELECT语句返回结果集
    bool Next() throw (TOdbcDBException);                               //移动到下一个记录
    bool Execute(int iExecuteRows=1) throw (TOdbcDBException);                 //执行非SELECT语句,没有返回结果集
    bool TransBegin();
    bool Commit();                                                  //事务提交
    bool Rollback();                                                //事务回滚
    //dongchun begin 没有实现，先暂时注释掉
    int  RowsAffected() ;                        //DELETE/UPDATE/INSERT语句修改的记录数目
    bool Eof(void);
    //dongchun end
    //int  GetSQLCode() ;                          //返回Oracle执行结果代码
	void CheckError(const char* sSql=NULL) throw (TOdbcDBException);        //用于判断当前的语句是否正确执行，如果有错误则把错误信息放入errMsg;
	bool IsFieldExist(const char *fieldName);

    //与列信息相关              
    int FieldCount() ;  //总共有几个列

    //ZMP:305013 begin
    TOdbcDBParam& Param(int iIndex);	//根据索引获取第i个参数实例,从0开始
    TOdbcDBParam& Param(const char*sParamName); //根据参数名获取参数实例
    //ZMP:305013 end
    
    TOdbcDBCSField& Field(int iIndex) throw (TOdbcDBException);												//根据索引获取第i个列实例,从0开始
    TOdbcDBCSField& Field(const char *sFieldName) throw (TOdbcDBException);//根据列名获取列实例
    /*begin zmp 841469 */
    /*直接获取值*/
    void GetValue(void *pStruct,int* Column)throw (TOdbcDBException);
    void SetUpdateMDBFlag(bool flag)throw (TOdbcDBException);
    bool GetUpdateMDBFlag()throw (TOdbcDBException);
    static bool m_bUpdateMDBFlag;/*判断数据库是否允许进行更新*/
    /*end zmp 841469 */

    //bool IsFieldExist(const char *fieldName);
    
    //与参数信息相关
    //dongchun begin 没有实现，先暂时注释掉
    //int ParamCount() ;
    //dongchun end
    //TQMDBParam& Param(int index) throw (TOdbcDBException){return NULL;}                //返回第i个列信息
    //TQMDBParam& Param(const char *paramName) throw (TOdbcDBException){return NULL;}    //根据列名(不分大小写)返回列信息; 建议使用Field(int i)获得更高的效率
    
    //以下是设置参数值
    void SetParameter(const char *paramName, const char* paramValue, bool isOutput = false) throw (TOdbcDBException);   
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TOdbcDBException);    //add by fu.wenjun@20041125
    void SetParameter(const char *paramName, int paramValue, bool isOutput = false) throw (TOdbcDBException); 
    void SetParameter(const char *paramName, double paramValue, bool isOutput = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName, long paramValue, bool isOutput = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName, long long paramValue, bool isOutput = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName,const char* paramValue,int iBufferLen,bool isOutput = false) throw (TOdbcDBException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(const char *paramName) throw (TOdbcDBException);

    //ZMP:305920 begin
    void SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException);
    //ZMP:305920 end
    
    //简化
    void SetParameter(int iParamIndex,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(int iParamIndex) throw (TOdbcDBException);     //设置参数为空


    /*备注:增加int iArraySize=0为了兼容Informix部分的同名函数,OCI部分不进行任何处理*/
    //数组操作
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
    
    //获取序列
    virtual long long GetSequenceByName(const char * sName)throw (TOdbcDBException); //ZMP_559430

private:
	TMdbClientQuery *m_pQuery;
	//dongchun 这里不需要数组，只需要个对象就可以了
	TOdbcDBCSField m_tField; //默认最多40个字段

};


class TOdbcDBCSDatabase: public TDBInterface 
{
    friend class TOdbcDBCSQuery;
    
public:
    TOdbcDBCSDatabase() throw (TOdbcDBException);
    virtual ~TOdbcDBCSDatabase();
    void SetServer(const char* pszIP,int pszPort) throw (TOdbcDBException);
    void SetLogin(const char *user, const char *password, const char *tnsString) throw (TOdbcDBException);
    bool Connect(bool bIsAutoCommit=false) throw (TOdbcDBException);
    bool Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit=false) throw (TOdbcDBException);
    bool Connect( const char* db, const char* server,const char* user, const char* password, unsigned int port = 0 ) throw (TOdbcDBException);
    int Disconnect() throw (TOdbcDBException);
    void TransBegin();
    void Commit();
    void Rollback();
    bool IsConnect() throw (TOdbcDBException);						//测试数据库是否连接正常
    bool IsNullConnect();
    
    const char* GetProvider() {return _TTDBCS_CONN_STRING_;}
    TOdbcDBCSQuery *CreateDBQuery() throw (TDBExcpInterface);
    TMdbClientDatabase *m_pMdbDatabase;		
private:
    //用于判断当前的语句是否正确执行，如果有错误则把错误信息放入errMsg;
    void CheckError(const char* sSql=NULL) throw (TOdbcDBException);  
    															//用于测试数据库是否连接正常
};


//-------------------------------------------------------------


#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif  //_TTTDBQUERY_H_

