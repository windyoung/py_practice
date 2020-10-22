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

const   int PREFETCH_ROWS = 200;                      				//Ԥ����ȡn�����ݵ�������,������������
const   int MAX_STRING_VALUE_LENGTH = 255;            				//���ص��ַ��������С����ع��̲�������
const   int MAX_LOB_BUFFER_LENGTH = 1024;             				//LOB���ݻ����������ռ�
const   int MAX_PARAMS_COUNT = 220;                   				//���������Ŀ,ZMP:330592,ֵ̫С�ο����1.1�ĳ�220
const   int LOB_FLUSH_BUFFER_SIZE = 400*1024;         				//LOB���ݻ��۵�����ʱ��д�����ݿ�

//�쳣ʹ��
const   int MAX_ERRMSG_LENGTH = 4096;                 				//������Ϣ����󳤶�
const   int MAX_SQLSTMT_LENGTH = 4096*10;                			//���ִ����SQL��䳤��

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
const   char* const ERR_CAPABILITY_NOT_YET_SUPPORT = "capability not support yet:%s"; //�������������Խ��Χ
const   char* const ERR_READ_PARAM_DATA = "read parameter value data type error, parameter name:%s, method:%s";

//const define:
const char* const NULL_STRING = "";
const int NULL_NUMBER = 0;

const int INSTANCEDB_RB = 0; // ����RB��
const int INSTANCEDB_CC = 1; // ����CC��


#define SIZE_LONG_LONG 64	 //LONG LONG��󳤶�

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
        //ZMP:305593,�������н����˳�ʼ��--������
        memset(m_sErrMsg,0,sizeof(m_sErrMsg));
        memset(m_sErrSql,0,MAX_SQLSTMT_LENGTH+1);
        m_lErrCode = 0;
    }
    //��������
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
    //��ֵ����
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
    char m_sErrMsg[MAX_ERRMSG_LENGTH+1];        			//������Ϣ
    char* m_sErrSql;           	//���������sql���
    int m_lErrCode;                                   //�����
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
    virtual bool IsConnect()=0;																		//�������ݿ��Ƿ���������
    virtual const char* GetProvider()=0;
    virtual TDBQueryInterface *CreateDBQuery() throw (TDBExcpInterface)=0;

    //�Ʒѿ��������ʶ�����ӵ����ݿ�ʵ������: RB���CC��
    virtual int  GetInstanceType(){return INSTANCEDB_RB;}
    virtual void SetInstanceType(int iConnectDBType){ }
};

class TDBQueryInterface
{
public:
    TDBQueryInterface() {};
    virtual ~TDBQueryInterface() {}
    //��Ҫ����
    virtual void Close()=0;                                                   				//�ر�SQL��䣬��׼��������һ��sql���
    virtual void CloseSQL()=0;    
    virtual void SetSQL(const char *inSqlstmt,int iPreFetchRows=0) throw (TDBExcpInterface)=0;        //����Sqlstatement
    virtual void Open(int prefetchRows=PREFETCH_ROWS) throw (TDBExcpInterface)=0; //��SQL SELECT��䷵�ؽ����
    virtual bool Next() throw (TDBExcpInterface)=0;                               //�ƶ�����һ����¼
    virtual bool Execute(int iters=1) throw (TDBExcpInterface)=0;                 //ִ�з�SELECT���,û�з��ؽ����
    virtual bool Commit()=0;                                                  				//�����ύ
    virtual bool Rollback()=0;                                                				//����ع�
    virtual int  RowsAffected()=0;               																				//DELETE/UPDATE/INSERT����޸ĵļ�¼��Ŀ
    virtual bool TransBegin(){return true;};                                                            //������
    virtual int  ParamCount(){return 0;};	
    /*begin zmp 841469 */
    /*ֱ�ӻ�ȡֵ*/
    virtual void GetValue(void *pStruct,int* Column)=0;
    virtual void SetUpdateMDBFlag(bool flag)=0;
    virtual bool GetUpdateMDBFlag()=0;
    /*end zmp 841469 */
    
    //ZMP:305013,ȥ������Ҫ�ĺ�
    //#ifndef _QM																											      //��ȡ��������
    virtual TDBParamInterface& Param(int iIndex)=0;												                  //����������ȡ��i������ʵ��,��0��ʼ
    virtual TDBParamInterface& Param(const char*sParamName)=0;  
    //#endif                            //���ݲ�������ȡ����ʵ��
    //�ݲ�����//virtual int  GetSQLCode()=0;                          											//����Oracleִ�н������
    virtual void CheckError(const char* sSql=NULL) throw (TDBExcpInterface)=0;  
    //#ifndef _QM 
    virtual bool Eof(void)=0;
	//#endif
    //������Ϣ���              
    virtual int FieldCount()=0;                                                         //�ܹ��м�����
    virtual TDBFieldInterface& Field(int index)  throw (TDBExcpInterface)=0;            //���ص�i������Ϣ
    virtual TDBFieldInterface& Field(const char *fieldName) throw (TDBExcpInterface)=0; //��������(���ִ�Сд)��������Ϣ; ����ʹ��Field(int i)��ø��ߵ�Ч��
    
    
    //���������ò���ֵ
    virtual void SetParameter(const char *paramName, const char* paramValue, bool isOutput = false) throw (TDBExcpInterface)=0;   
    virtual void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TDBExcpInterface)=0;    //add by fu.wenjun@20041125
    
    virtual void SetParameter(const char *paramName, int paramValue, bool isOutput = false) throw (TDBExcpInterface)=0; 
    virtual void SetParameter(const char *paramName, double paramValue, bool isOutput = false) throw (TDBExcpInterface)=0;
    virtual void SetParameter(const char *paramName, long paramValue, bool isOutput = false) throw (TDBExcpInterface)=0;
    virtual void SetParameter(const char *paramName, long long paramValue, bool isOutput = false) throw (TDBExcpInterface)=0;
    
    
    virtual void SetParameter(const char *paramName,const char* paramValue,int iBufferLen,bool isOutput = false) throw (TDBExcpInterface)=0;//���ڴ���BLOB/BINARY�����ֶ�
    virtual void SetParameterNULL(const char *paramName) throw (TDBExcpInterface)=0;

    /*��ע:����int iArraySize=0Ϊ�˼���Informix���ֵ�ͬ������,OCI���ֲ������κδ���*/
    //�������
    virtual void SetParamArray(const char *paramName, char     ** paramValue, int iStructSize, int iStrSize ,int iArraySize=0,bool isOutput = false) throw (TDBExcpInterface)=0;  
    virtual void SetParamArray(const char *paramName, int       * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBExcpInterface)=0; 
    virtual void SetParamArray(const char *paramName, double    * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBExcpInterface)=0;
    virtual void SetParamArray(const char *paramName, long      * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBExcpInterface)=0;
    virtual void SetParamArray(const char *paramName, long long * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBExcpInterface)=0;
    virtual void SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize=0,bool isOutput=false) throw (TDBExcpInterface)=0;//���ڴ���BLOB/BINARY�����ֶ��������
    
    virtual void SetParameter(int iParamIndex,const char* sParamValue) throw (TDBExcpInterface){};
    virtual void SetParameter(int iParamIndex,int iParamValue) throw (TDBExcpInterface){}; 
    virtual void SetParameter(int iParamIndex,long lParamValue) throw (TDBExcpInterface){};
    virtual void SetParameter(int iParamIndex,double dParamValue) throw (TDBExcpInterface){};
    virtual void SetParameter(int iParamIndex,long long llParamValue) throw (TDBExcpInterface){};
    virtual void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TDBExcpInterface){};//���ڴ���BLOB/BINARY�����ֶ�
    virtual void SetParameterNULL(int iParamIndex) throw (TDBExcpInterface){};     //���ò���Ϊ��

    virtual void SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused = false) throw (TDBExcpInterface){};
    virtual void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TDBExcpInterface){};
    
    //��ȡ����ֵ
    virtual long long GetSequenceByName(const char * sName)throw (TDBExcpInterface) =0; //ZMP_559430

};


class TDBFieldInterface
{
public: 
    virtual ~TDBFieldInterface() {} 
    virtual bool    isNULL()=0;           										//��fetch�����и��е������Ƿ�Ϊ��
    virtual char   *AsString() throw(TDBExcpInterface)=0;
    virtual double  AsFloat() throw (TDBExcpInterface)=0;
    virtual long long   AsInteger() throw (TDBExcpInterface)=0;
    //#ifndef _QM	//ZMP_496768
    virtual char* AsBlobBuffer(int &iBufferLen)=0;
    //#endif
    //Blob����
    //�ݲ�����//virtual void   AsBlobFile(const char *fileName) throw (TDBExcpInterface)=0;       //��ȡ��file��
    //�ݲ�����//virtual void   AsBlobBuffer(unsigned char* &buf, unsigned int *bufLength) throw (TDBExcpInterface)=0;    //���浽������,�������Ĵ�С�Զ������������ػ�������С*bufLength.
    //�ݲ�����//virtual char*  AsBlobBuffer(int &iBufferLen) throw (TDBExcpInterface)=0;										 //���BLOB�ֶ�
    //���ڴ���
    virtual char*		AsDateTimeString() throw (TDBExcpInterface)=0;   			//�������͵�����HH:MM:DD HH24:MI��ʽ��ȡ,ʹ��asString()��ȡ����������������ΪHH:MM:DD
    virtual void   AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBExcpInterface)=0; //�������ڵĸ�������
    //#ifndef _QM	
    virtual time_t AsTimeT() throw (TDBExcpInterface)=0;
    //#endif

    virtual void   ClearDataBuf()=0;																	  	//�α�ѭ����ȡָ���������ݺ�,δ����ϴεĻ���
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


