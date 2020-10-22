//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#ifndef _DBINTERFACE_H
#define _DBINTERFACE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Common.h"
#include "TException.h"

#ifdef HP_UNIX
//  #pragma pack 8
#endif
 
#ifdef WIN32
#pragma warning(disable: 4290)
#pragma warning(disable: 4267)
#endif

const   int PREFETCH_ROWS = 200;                      				//预先提取n行数据到缓冲区,减少网络流量
const   int MAX_STRING_VALUE_LENGTH = 255;            				//返回的字符串最大的列、返回过程参数长度
const   int MAX_LOB_BUFFER_LENGTH = 1024;             				//LOB数据缓冲区的最大空间
const   int MAX_PARAMS_COUNT = 220;                   				//参数最大数目,ZMP:330592,值太小参考框架1.1改成220
const   int LOB_FLUSH_BUFFER_SIZE = 400*1024;         				//LOB数据积累到此量时，写入数据库

//异常使用
const   int MAX_ERRMSG_LENGTH = 4096;                 				//错误信息的最大长度
const   int MAX_SQLSTMT_LENGTH = 4096*10;                			//出现错误的SQL语句长度

//error message definination:
const   char* const ERR_GENERAL = "General Error: %s"; 			//throw TDBException("TDBQuery(TDBDatabase &db)", ERR_GENERAL, "Can not declare a TDBQuery when the database is not connected");
const   char* const ERR_INDEX_OUT_OF_BOUND = "%s";    				//throw TDBException(fSqlStmt , ERR_INDEX_OUT_OF_BOUND, "field index out of bound when call Field(i)");   
const   char* const ERR_DB_INIT = "OCI: OCI handles init fail in TDatabase constructor: @line:%d";
const   char* const ERR_SET_LOGIN = "OCI: You can only set login infomation on disconnect status: line %d";
const   char* const ERR_CONNECT_NO_LOGIN_INFO = "No login information provided before Connect(), call SetLogin first, line:%d";
const   char* const ERR_NO_DATASET = "OCI: function:%s , Result Dataset is on Bof/Eof. field:%s"; //throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "asBlobBuffer()", name);
const   char* const ERR_DATA_TYPE_CONVERT = "Data type convertion error: field:%s data type:%d can not be access by %s"; //throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "asLobBuffer()");
const   char* const ERR_NOMORE_MEMORY_BE_ALLOCATED = "no more memory can be allocate when :%s, source code:%d"; //throw TDBException(fParentQuery->fSqlStmt, ERR_NOMORE_MEMORY_BE_ALLOCATED, "asBlobBuffer()", __LINE__);
const   char* const ERR_FILE_IO = "%s: can not open file:%s"; //throw TDBException(fParentQuery->fSqlStmt, ERR_FILE_IO, "LoadFromFile()", fileName);
const   char* const ERR_OPEN_FILE = "open file failed:%s, errno:%d, error_msg:%s";
const   char* const ERR_MEM_BUFFER_IO = "asBlobWriter() error: read from file to buffer, field:%s, file:%s, @line:%d"; //throw TDBException(fParentQuery->fSqlStmt, ERR_MEM_BUFFER_IO, name, fileName, __LINE__);
const   char* const ERR_DATA_TYPE_NOT_SUPPORT = "field:%s, datatype:%d not yet supported"; //, pCurrField->name,innerDataType);
const   char* const ERR_PARAM_NOT_EXISTS = "param:%s does not exists."; //throw TDBException(fSqlStmt, ERR_PARAM_NO_EXISTS, paramName, "check spelling error");
const   char* const ERR_FIELD_NOT_EXISTS = "field:%s does not exists.";
const   char* const ERR_INVALID_METHOD_CALL = "%s: invalid call method:%s";
const   char* const ERR_CAPABILITY_NOT_YET_SUPPORT = "capability not support yet:%s"; //例如参数个数超越范围
const   char* const ERR_READ_PARAM_DATA = "read parameter value data type error, parameter name:%s, method:%s";

//const define:
const char* const NULL_STRING = "";
const int NULL_NUMBER = 0;

const int INSTANCEDB_RB = 0; // 连接RB库
const int INSTANCEDB_CC = 1; // 连接CC库


#define SIZE_LONG_LONG 64	 //LONG LONG最大长度

/* classes defined in this file: */
class TDBQueryInterface;
class TDBExcpInterface;
class TDBInterface;
class TDBFieldInterface;
class TDBSessionInterface;
class TDBParamInterface;

class TDBExcpInterface: public TException
{
public:
    TDBExcpInterface() 
    {
        m_sErrSql = new char [MAX_SQLSTMT_LENGTH+1];
        //ZMP:305593,在子类中进行了初始化--需斟酌
        memset(m_sErrMsg,0,sizeof(m_sErrMsg));
        memset(m_sErrSql,0,MAX_SQLSTMT_LENGTH+1);
        m_lErrCode = 0;
    }
    //拷贝构造
    TDBExcpInterface(const TDBExcpInterface &oTDBExcpInterface) 
    {
        memset(m_sErrMsg,0,sizeof(m_sErrMsg));
        strncpy(m_sErrMsg,oTDBExcpInterface.m_sErrMsg,sizeof(m_sErrMsg)-1);
        m_sErrMsg[sizeof(m_sErrMsg)-1]='\0';
        
        size_t iLen=strlen(oTDBExcpInterface.m_sErrSql);        
        m_sErrSql = new char [iLen+1];
        memset(m_sErrSql,0,iLen+1);
        strncpy(m_sErrSql,oTDBExcpInterface.m_sErrSql,iLen);
        m_sErrSql[iLen]='\0';
       
        m_lErrCode = oTDBExcpInterface.m_lErrCode;
    }
    //赋值构造
    TDBExcpInterface& operator =(const TDBExcpInterface &oTDBExcpInterface) 
    {
        if(this == &oTDBExcpInterface)
        {
            return *this;
        }
        memset(m_sErrMsg,0,sizeof(m_sErrMsg));
        strncpy(m_sErrMsg,oTDBExcpInterface.m_sErrMsg,sizeof(m_sErrMsg)-1);
        m_sErrMsg[sizeof(m_sErrMsg)-1]='\0';
        
        DELETE_A(m_sErrSql);
        size_t iLen=strlen(oTDBExcpInterface.m_sErrSql);        
        m_sErrSql = new char [iLen+1];
        memset(m_sErrSql,0,iLen+1);
        strncpy(m_sErrSql,oTDBExcpInterface.m_sErrSql,iLen);
        m_sErrSql[iLen]='\0';
        
        m_lErrCode = oTDBExcpInterface.m_lErrCode;
        return *this;
    }
    virtual ~TDBExcpInterface() 
    {
        if( m_sErrSql )
        {
            delete []m_sErrSql;
            m_sErrSql = NULL;
        }
    }
    virtual char *GetErrMsg() const {return((char*)m_sErrMsg);}
    virtual char *GetErrSql() const {return((char*)m_sErrSql);}
protected:    
    char m_sErrMsg[MAX_ERRMSG_LENGTH+1];        			//错误信息
    char* m_sErrSql;           	//发生错误的sql语句
    int m_lErrCode;                                   //错误号
};

class TDBInterface
{
public:
    TDBInterface() throw (TDBExcpInterface) {};
    virtual ~TDBInterface(){};
    virtual void SetLogin(const char *user, const char *password, const char *tnsString) throw (TDBExcpInterface)=0;
    virtual bool Connect(bool bIsAutoCommit=false) throw (TDBExcpInterface)=0;
    virtual void CheckError(const char* sSql=NULL) throw (TDBExcpInterface)=0;
    virtual bool Connect(const char *usr, const char *pwd, const char *tns,bool bIsAutoCommit=false) throw (TDBExcpInterface)=0;
    virtual int  Disconnect() throw (TDBExcpInterface)=0;
    virtual void Commit()=0;    
    virtual void Rollback()=0;
    virtual bool IsConnect()=0;																		//测试数据库是否连接正常
    virtual const char* GetProvider()=0;
    virtual TDBQueryInterface *CreateDBQuery() throw (TDBExcpInterface)=0;

    //计费框架中用于识别连接的数据库实例类型: RB库或CC库
    virtual int  GetInstanceType(){return INSTANCEDB_RB;}
    virtual void SetInstanceType(int iConnectDBType){ }
};

class TDBQueryInterface
{
public:
    TDBQueryInterface() {};
    virtual ~TDBQueryInterface() {}
    //主要功能
    virtual void Close()=0;                                                   				//关闭SQL语句，以准备接收下一个sql语句
    virtual void CloseSQL()=0;    
    virtual void SetSQL(const char *inSqlstmt,int iPreFetchRows=0) throw (TDBExcpInterface)=0;        //设置Sqlstatement
    virtual void Open(int prefetchRows=PREFETCH_ROWS) throw (TDBExcpInterface)=0; //打开SQL SELECT语句返回结果集
    virtual bool Next() throw (TDBExcpInterface)=0;                               //移动到下一个记录
    virtual bool Execute(int iters=1) throw (TDBExcpInterface)=0;                 //执行非SELECT语句,没有返回结果集
    virtual bool Commit()=0;                                                  				//事务提交
    virtual bool Rollback()=0;                                                				//事务回滚
    virtual int  RowsAffected()=0;               																				//DELETE/UPDATE/INSERT语句修改的记录数目
    virtual bool TransBegin(){return true;};                                                            //事务开启
    virtual int  ParamCount(){return 0;};	
    /*begin zmp 841469 */
    /*直接获取值*/
    virtual void GetValue(void *pStruct,int* Column)=0;
    virtual void SetUpdateMDBFlag(bool flag)=0;
    virtual bool GetUpdateMDBFlag()=0;
    /*end zmp 841469 */
    
    //ZMP:305013,去掉不必要的宏
    //#ifndef _QM																											      //获取参数个数
    virtual TDBParamInterface& Param(int iIndex)=0;												                  //根据索引获取第i个参数实例,从0开始
    virtual TDBParamInterface& Param(const char*sParamName)=0;  
    //#endif                            //根据参数名获取参数实例
    //暂不开放//virtual int  GetSQLCode()=0;                          											//返回Oracle执行结果代码
    virtual void CheckError(const char* sSql=NULL) throw (TDBExcpInterface)=0;  
    //#ifndef _QM 
    virtual bool Eof(void)=0;
	//#endif
    //与列信息相关              
    virtual int FieldCount()=0;                                                         //总共有几个列
    virtual TDBFieldInterface& Field(int index)  throw (TDBExcpInterface)=0;            //返回第i个列信息
    virtual TDBFieldInterface& Field(const char *fieldName) throw (TDBExcpInterface)=0; //根据列名(不分大小写)返回列信息; 建议使用Field(int i)获得更高的效率
    
    
    //以下是设置参数值
    virtual void SetParameter(const char *paramName, const char* paramValue, bool isOutput = false) throw (TDBExcpInterface)=0;   
    virtual void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TDBExcpInterface)=0;    //add by fu.wenjun@20041125
    
    virtual void SetParameter(const char *paramName, int paramValue, bool isOutput = false) throw (TDBExcpInterface)=0; 
    virtual void SetParameter(const char *paramName, double paramValue, bool isOutput = false) throw (TDBExcpInterface)=0;
    virtual void SetParameter(const char *paramName, long paramValue, bool isOutput = false) throw (TDBExcpInterface)=0;
    virtual void SetParameter(const char *paramName, long long paramValue, bool isOutput = false) throw (TDBExcpInterface)=0;
    
    
    virtual void SetParameter(const char *paramName,const char* paramValue,int iBufferLen,bool isOutput = false) throw (TDBExcpInterface)=0;//用于传入BLOB/BINARY类型字段
    virtual void SetParameterNULL(const char *paramName) throw (TDBExcpInterface)=0;

    /*备注:增加int iArraySize=0为了兼容Informix部分的同名函数,OCI部分不进行任何处理*/
    //数组操作
    virtual void SetParamArray(const char *paramName, char     ** paramValue, int iStructSize, int iStrSize ,int iArraySize=0,bool isOutput = false) throw (TDBExcpInterface)=0;  
    virtual void SetParamArray(const char *paramName, int       * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBExcpInterface)=0; 
    virtual void SetParamArray(const char *paramName, double    * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBExcpInterface)=0;
    virtual void SetParamArray(const char *paramName, long      * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBExcpInterface)=0;
    virtual void SetParamArray(const char *paramName, long long * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBExcpInterface)=0;
    virtual void SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize=0,bool isOutput=false) throw (TDBExcpInterface)=0;//用于传入BLOB/BINARY类型字段数组参数
    
    virtual void SetParameter(int iParamIndex,const char* sParamValue) throw (TDBExcpInterface){};
    virtual void SetParameter(int iParamIndex,int iParamValue) throw (TDBExcpInterface){}; 
    virtual void SetParameter(int iParamIndex,long lParamValue) throw (TDBExcpInterface){};
    virtual void SetParameter(int iParamIndex,double dParamValue) throw (TDBExcpInterface){};
    virtual void SetParameter(int iParamIndex,long long llParamValue) throw (TDBExcpInterface){};
    virtual void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TDBExcpInterface){};//用于传入BLOB/BINARY类型字段
    virtual void SetParameterNULL(int iParamIndex) throw (TDBExcpInterface){};     //设置参数为空

    virtual void SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused = false) throw (TDBExcpInterface){};
    virtual void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TDBExcpInterface){};
    
    //获取序列值
    virtual long long GetSequenceByName(const char * sName)throw (TDBExcpInterface) =0; //ZMP_559430

};


class TDBFieldInterface
{
public: 
    virtual ~TDBFieldInterface() {} 
    virtual bool    isNULL()=0;           										//在fetch过程中该列的数据是否为空
    virtual char   *AsString() throw(TDBExcpInterface)=0;
    virtual double  AsFloat() throw (TDBExcpInterface)=0;
    virtual long long   AsInteger() throw (TDBExcpInterface)=0;
    //#ifndef _QM	//ZMP_496768
    virtual char* AsBlobBuffer(int &iBufferLen)=0;
    //#endif
    //Blob处理
    //暂不开放//virtual void   AsBlobFile(const char *fileName) throw (TDBExcpInterface)=0;       //读取到file中
    //暂不开放//virtual void   AsBlobBuffer(unsigned char* &buf, unsigned int *bufLength) throw (TDBExcpInterface)=0;    //保存到缓冲区,缓冲区的大小自动创建，并返回缓冲区大小*bufLength.
    //暂不开放//virtual char*  AsBlobBuffer(int &iBufferLen) throw (TDBExcpInterface)=0;										 //输出BLOB字段
    //日期处理
    virtual char*		AsDateTimeString() throw (TDBExcpInterface)=0;   			//把日期型的列以HH:MM:DD HH24:MI格式读取,使用asString()读取的日期型数据类型为HH:MM:DD
    virtual void   AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBExcpInterface)=0; //返回日期的各个部分
    //#ifndef _QM	
    virtual time_t AsTimeT() throw (TDBExcpInterface)=0;
    //#endif

    virtual void   ClearDataBuf()=0;																	  	//游标循环获取指定数量数据后,未清空上次的缓存
    virtual int DataType(void){return 0;};
    virtual const char * AsName(){return NULL;};
};

class TDBParamInterface
{
public:
    virtual ~TDBParamInterface() {} ;
  //  virtual char* GetName()=0;
    
private:
    virtual void ClearInfo()=0;
};

//
//class TDBSessionInterface
//{   
//public:
//    //TDBSessionInterface();
//    //virtual ~TDBSessionInterface();
//    virtual void CheckError()=0;
//    virtual void SessionBegin()=0;
//    virtual void SessionEnd()=0;
//    
//};

#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif  //_DBInterface_h


