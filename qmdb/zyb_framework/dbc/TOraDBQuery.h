//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#ifndef _TORAQUERY_H
#define _TORAQUERY_H

#include "Common.h"
#include "BillLog.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <oci.h>
#include <oratypes.h>
#include <ocidfn.h>
#include <ocidem.h>
#include <ociapr.h>
#include <oci.h>

#include "TDBInterface.h"

#ifdef WIN32
#pragma warning(disable: 4290)
#pragma warning(disable: 4267)
#endif

#ifdef HP_UNIX
//  #pragma pack 8
#endif

//异常使用
const   int MAX_ERR_CAT_LENGTH = 50;                  				//错误分类长度

#define _ORADB_CONN_STRING_              "ORADB"

/* classes defined in this file: */
class TDBException;
class TDBDatabase;
class TDBQuery;
class TDBField;
class TDBParam;
class TDBSession;
 

class TDBException: public TDBExcpInterface
{
public:
    virtual ~TDBException() {}
    TDBException(sword errNumb, OCIError *err,const char *cat,const char *sql);//执行OCI函数发生的错误
    TDBException(const char *sql, const char* errFormat, ...);
public:
    virtual char * ToString() const;
private:
    char errCategory[MAX_ERR_CAT_LENGTH+1];  //错误分类
};


class TDBParam  : public TDBParamInterface
{
    friend class TDBQuery;
public:
    TDBParam();
    virtual ~TDBParam();
    int     AsInteger()  throw (TDBException);   //读取返回参数值
    double  AsFloat()    throw (TDBException);
    long    AsLong()     throw (TDBException);
    char*   AsString()   throw (TDBException);
    bool    isNULL()     throw (TDBException);
private:
	  void ClearInfo(){};
    char    *name;
    ub2     dataType;
    int     intValue;
    double  dblValue;         																		//存储输入和输出的值(缓冲区)
    long    longValue;        																		//long数据缓冲区
    char   * stringValue;     																		//字符串返回缓冲区
    int    * intArray;        																		//INT数组
    double * dblArray;        																		//DOUBLE数组
    long   * longArray;       																		//LONG数组
    long long * llongArray;   																		//LONG LONG数组
    char ** stringArray;      																		//STRING数组
    int   stringSize;         																		//STRING数组中的string大小
    bool  fIsOutput;          																		//是否是输出参数.默认是输入参数
    sb2   indicator;          																		//在返回值时候是否为空
    OCIBind  *hBind;
};

class TDBField : public TDBFieldInterface
{
    friend class TDBQuery;
public: 
    virtual ~TDBField(); 
    bool    isNULL();           																//在fetch过程中该列的数据是否为空
    char   *AsString() throw(TDBException);
    double  AsFloat() throw (TDBException);
    long long   AsInteger() throw (TDBException);
  //blob 和 clob处理
	void        AsLobString(char* const buf, unsigned int maxLength, unsigned int& bufLength) throw (TDBException);	
    //Blob处理
    void        AsBlobFile(const char *fileName) throw (TDBException);          	//读取到file中
    void        AsBlobBuffer(unsigned char* &buf, unsigned int *bufLength) throw (TDBException);    //保存到缓冲区,缓冲区的大小自动创建，并返回缓冲区大小*bufLength.
    char*       AsBlobBuffer(int &iBufferLen) throw (TDBException);														//输出BLOB字段
    //日期处理
    char       *AsDateTimeString() throw (TDBException);   																						//把日期型的列以HH:MM:DD HH24:MI格式读取,使用asString()读取的日期型数据类型为HH:MM:DD
    void        AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBException); //返回日期的各个部分
    time_t      AsTimeT() throw (TDBException);
    void        LoadFromFile(const char *fileName) throw (TDBException);         //写入到blob中
    void        LoadFromBuffer(unsigned char *buf, unsigned int bufLength) throw (TDBException);    //把LOB的内容用缓冲区内容替代    
    void        ClearDataBuf();																																																		//游标循环获取指定数量数据后,未清空上次的缓存

public:
    char *GetFieldName();    																			//获取列名
    long GetFieldType();     																			//获取列名类型
    long GetFieldSize();     																			//获取列大小
    int  GetFieldPrecision();     														//获取列精度
private:
    //日期处理
    void    AsDateTimeInternal(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBException); //返回日期的各个部分,没有作其他校验，只是内部调用                    

    int AsDateTimeInternalNew(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBException);

    char    *name;              																//字段名称
    long    size;               																//数据长度
    long    type;               																//数据类型 in(INT_TYPE,FLOAT_TYPE,DATE_TYPE,STRING_TYPE,ROWID_TYPE)
    int     precision;          																//数值总长度
    int     scale;              																//数值中小数点个数
    bool    nullable;           																//字段定义时是否允许为空值--为了和其他的相一致
    TDBQuery *fParentQuery;                 				//指向该Field所属于的Query
    TDBField();
    //数据缓冲区,分别为字符串、整数、浮点数分配空间
    ub1 fStrBuffer[MAX_STRING_VALUE_LENGTH];    //用于保存转换为字符串后的值
    ub1 *fDataBuf;                      								//在分析字段时候获得空间max(该列的最大长度,MAX_STRING_VALUE_LENGTH), 在Destructor中释放
    OCILobLocator *hBlob;               								//支持LOB
    sb2 *fDataIndicator;                								//在defineByPos中使用，用于在fetch时察看是否有字段值返回、字段值是否被截断;valueIsNULL, isTruncated根据此值获得结果
                                        								
    ub2   fReturnDataLen;               								//读取数据时返回的真实长度
    ub2 fInternalType;                  								//Oracle内部数据类型
    ub2 fRequestType;                   								//在读取数据时候的请求数据类型
    OCIDefine *hDefine;                 								//用于读取列信息
                                        								
    long lDataBufLen;                   								//记录fDataBuf实际长度,列长*数组元素个数
    char *sBlob;																																//存放BLOB字段值
                                        								
    																			                 								
    OCILobLocator **ahBlob;																					//获取选择列类型为BLOB的字段值
    int iBlobCount;

	char intStr[128];
};

class TDBQuery: public TDBQueryInterface
{
    friend class TDBField;
public:
    //主要功能
    void Close();                                                   //关闭SQL语句，以准备接收下一个sql语句
    void CloseSQL() {}                                              //兼容odbc
    void SetSQL(const char *inSqlstmt,int iPreFetchRows=0) throw (TDBException);        //设置Sqlstatement
    void Open(int prefetchRows=PREFETCH_ROWS) throw (TDBException); //打开SQL SELECT语句返回结果集
    bool Next() throw (TDBException);                               //移动到下一个记录
    bool Execute(int iters=1) throw (TDBException);                 //执行非SELECT语句,没有返回结果集
    bool Commit();                                                  //事务提交
    bool Rollback();                                                //事务回滚
    int  RowsAffected() { return (int)fTotalRowsFetched;};               //DELETE/UPDATE/INSERT语句修改的记录数目
    int  GetSQLCode() { return fErrorNo;};                          //返回Oracle执行结果代码
    bool Eof(void) { return fLastRecordFlag;}

    //与列信息相关              
    int FieldCount() ;                                              //总共有几个列
    TDBField& Field(int index)  throw (TDBException)    ;           //返回第i个列信息
    TDBField& Field(const char *fieldName) throw (TDBException);    //根据列名(不分大小写)返回列信息; 建议使用Field(int i)获得更高的效率
    /*begin zmp 841469 */
    /*直接获取值*/
    void GetValue(void *pStruct,int* Column)throw (TDBException);
    void SetUpdateMDBFlag(bool flag)throw (TDBException);
    bool GetUpdateMDBFlag()throw (TDBException);
    /*end zmp 841469 */

    bool IsFieldExist(const char *fieldName);
    //与参数信息相关
    int ParamCount() ;
    bool IsParamExist(const char *paramName);
    TDBParam& Param(int index) throw (TDBException);                //返回第i个列信息
    TDBParam& Param(const char *paramName) throw (TDBException);    //根据列名(不分大小写)返回列信息; 建议使用Field(int i)获得更高的效率
    //以下是设置参数值
    void SetParameter(const char *paramName, const char* paramValue, bool isOutput = false) throw (TDBException);   
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TDBException);    //add by fu.wenjun@20041125
    void SetParameter(const char *paramName, int paramValue, bool isOutput = false) throw (TDBException); 
    void SetParameter(const char *paramName, double paramValue, bool isOutput = false) throw (TDBException);
    void SetParameter(const char *paramName, long paramValue, bool isOutput = false) throw (TDBException);
    void SetParameter(const char *paramName, long long paramValue, bool isOutput = false) throw (TDBException);
    void SetParameter(const char *paramName,const char* paramValue,int iBufferLen,bool isOutput = false) throw (TDBException);//用于传入BLOB/BINARY类型字段
    void SetParameterNULL(const char *paramName) throw (TDBException);

	void SetParameter(int iParamIndex,const char* sParamValue) throw (TDBExcpInterface){};
    void SetParameter(int iParamIndex,int iParamValue) throw (TDBExcpInterface){}; 
    void SetParameter(int iParamIndex,long lParamValue) throw (TDBExcpInterface){};
    void SetParameter(int iParamIndex,double dParamValue) throw (TDBExcpInterface){};
    void SetParameter(int iParamIndex,long long llParamValue) throw (TDBExcpInterface){};
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TDBExcpInterface){};//用于传入BLOB/BINARY类型字段

////////////////////////////////////////////////////////////////////////////////
//138555 begin
    void SetParameterTime_t(const char *paramName, time_t tparamValue, bool isOutput = false) throw (TDBException);   
//138555 end
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//141877 begin
    void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TDBException);
    void SetParameterNULL(int paramName) throw (TDBException);
//141877 end
////////////////////////////////////////////////////////////////////////////////

    /*备注:增加int iArraySize=0为了兼容Informix部分的同名函数,OCI部分不进行任何处理*/
    //数组操作
    void SetParamArray(const char *paramName, char     ** paramValue, int iStructSize, int iStrSize ,int iArraySize=0,bool isOutput = false) throw (TDBException);  
    void SetParamArray(const char *paramName, int       * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBException); 
    void SetParamArray(const char *paramName, double    * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBException);
    void SetParamArray(const char *paramName, long      * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBException);
    void SetParamArray(const char *paramName, long long * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBException);
    void SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize=0,bool isOutput=false) throw (TDBException);//用于传入BLOB/BINARY类型字段数组参数

    //constructor & destructor
    TDBQuery(TDBDatabase *oradb) throw (TDBException);
    TDBQuery(TDBDatabase *oradb,TDBSession *session) throw (TDBException);
    virtual ~TDBQuery();
    static void SetBillLog(TBillLog *pBillLog);
    virtual long long GetSequenceByName(const char * sName)throw (TDBExcpInterface); //ZMP_559430
private:
    char *fSqlStmt;                         							//保存open的Select语句，可以方便调试
    ub2 fStmtType;                          							//***ub2!!! 保存sqlstmt的类型:SELECT/UPDATE/DELETE/INSERT/CREATE/BEGIN/ALTER...
    bool fActivated;                        							//是否已经处于打开状态，在调用OCIStmtPrepare成功后为True
    bool fLastRecordFlag;
    unsigned    fFetched;                   							//0..prefetchRows
    unsigned    fPrefetchRows;              							//1.. 
    unsigned    fCurrRow;                   							//0..fetched */
    unsigned    fTotalRowsFetched;          							//rows fetched from the start
                                            							
    int fFieldCount;                        							//字段个数
    TDBDatabase *db;                        							//此query属于哪个Dabase,在Constructor中创建
    TDBField *fieldList;                    							//在内部保存的所有字段信息
    void GetFieldsDef() throw (TDBException);      //获得字段信息,并为字段分配取值的缓冲区

    TDBParam *ParamByName(const char *paramName) throw (TDBException);//在内部使用，直接返回参数的指针

////////////////////////////////////////////////////////////////////////////////
//141877 begin
    TDBParam *ParamByName(int paramName) throw (TDBException);//在内部使用，直接返回参数的指针
//141877 end
////////////////////////////////////////////////////////////////////////////////

    void CheckError(const char* sSql=NULL) throw (TDBException);        //用于判断当前的语句是否正确执行，如果有错误则把错误信息放入errMsg;

    int fParamCount;                        							//参数个数
    TDBParam *paramList;                											//所有参数设置的信息
    void GetParamsDef() throw (TDBException);      //在setSQL时候获得参数信息
    int nTransTimes;                        							//是否曾经执行过Execute()事务操作，以便与回滚.
    static TBillLog *gpBillLog;
  
    OCIStmt *hStmt;                     											//用于分析sql语句的handle
    OCIError *hErr;                     											//错误处理
    sword fErrorNo;                     											//错误号
    bool fEof;                              							//在Fetch时候，已经达到最后一个记录,防止已经读到最后一个记录后，又fetch发生的错误
    bool fBof;                              							//在Open()时候为True,在Next()如果有数据为false;用于判断用户是否可以从缓冲区中读取列值,该部分尚未完成
    bool fOpened;                           							//数据集是否打开    
#ifdef __DEBUG__
    bool bExecuteFlag;
#endif
};


class TDBDatabase: public TDBInterface
{
    friend class TDBQuery;
    friend class TDBSession;
    friend class TDBDirPath;
    friend class TDBField;
public:
    TDBDatabase() throw (TDBException);
    virtual ~TDBDatabase();
    void SetLogin(const char *user, const char *password, const char *tnsString) throw (TDBException);
    bool Connect(bool bUnused=false) throw (TDBException);
    bool Connect(const char *usr, const char *pwd, const char *tns, bool bUnused=false) throw (TDBException);
    int Disconnect() throw (TDBException);
    void Commit();
    void Rollback();
    bool IsConnect() throw (TDBException);						//测试数据库是否连接正常
    const char* GetProvider() {return _ORADB_CONN_STRING_;}
    TDBQuery *CreateDBQuery() throw (TDBExcpInterface);
private:
    OCISession *hUser;
    OCISvcCtx *hDBSvc;      																				//用于登录链接服务器      
    char *usr, *pwd, *tns;  																				//登录名
    bool fConnected;        																				//在Connect中是否连接成功
    sword fErrorNo;         																				//错误号
    void CheckError(const char* sSql=NULL) throw (TDBException);     //用于判断当前的语句是否正确执行，如果有错误则把错误信息放入errMsg;
    OCIError *hDBErr;
    OCIEnv *hEnv;
    OCIServer *hSvr;
    TDBQuery *m_pTDBQuery;																						//用于测试数据库是否连接正常
    char m_sErrIgnore[255];

};



//外部不访问,取消派生
class TDBSession //: public TDBSessionInterface
{   
public:
    TDBSession(TDBDatabase *pDB);
    virtual ~TDBSession();

    void sessionBegin();
    void sessionEnd();

private:

    OCISvcCtx   *m_hSrvCtx;
    OCISession  *m_hSession;
    OCIError    *m_hError;

    bool    m_bActive;

    int m_iErrorNo;

    void CheckError();

    friend  class TDBDirPath;
    friend  class TDBQuery; 
};
 
#ifdef HP_UNIX
//  #pragma pack 4
#endif
 
#endif   //_TOraQuery_h
