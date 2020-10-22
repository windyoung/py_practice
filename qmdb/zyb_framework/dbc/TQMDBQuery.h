//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#ifndef _TQMDBQUERY_H_
#define _TQMDBQUERY_H_

#include "Interface/mdbQuery.h"
#include "TDBInterface.h"

#ifdef WIN32
#include <windows.h>
#else
//#include <sqlunix.h>
#endif

#include <string.h>
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
#pragma warning(disable:4290)       																								//������������throw (�쳣��)�澯
#pragma warning(disable:4267)       																								//size_t��int�澯 
#pragma warning(disable:4996)       																								//�������
#endif

#ifdef HP_UNIX
//  #pragma pack 8
#endif
#define DEFAULT_PREFETCH_ROWS           0                   //Ĭ��FETCH��¼��
#define MAX_SQL_SIZE                    4096                //ִ��SQL�����󳤶�
#define MAX_COLUMN_COUNT                64                  //����ѯ����,��������
#define MAX_COLUMN_NAME                 64                  //���������С
#define MAX_COLUMN_DATA                 512                 //�����ֵ��С
#define DEFAULT_PREFETCH_ROWS           0                   //Ĭ��FETCH��¼��
#define DEFAULT_EXECUTE_ROWS            0                   //Ĭ��EXCUTE��¼��

#define SIZE_TIMESTAMP                  25                  //SQL_TIMESTAMPʱ���ʽ,YYYY-MM-DD HH:MI:SS.MSEC
#define SIZE_DATE_TIME                  15                  //YYYYMMDDHHMISS

#define ODBC_FUNC_BASE                  1000
#define QUERY_BEFORE_OPEN               ODBC_FUNC_BASE+10   //��ѯOPEN֮ǰ״̬
#define QUERY_AFTER_OPEN                ODBC_FUNC_BASE+11   //��ѯOPEN֮��״̬
#define QUERY_FETCH_BOF                 ODBC_FUNC_BASE+12   //��ѯFETCH��ֵ״̬
#define QUERY_FETCH_EOF                 ODBC_FUNC_BASE+13   //��ѯFETCH��ֵ״̬

/* Isolation. ���뼶��0Ϊserializable��1ΪRead-Committed */
#define ISOLATION_SERIALIZABLE             0
#define ISOLATION_READ_COMMITTED           1

//�쳣ʹ��
//const   int MAX_ERR_CAT_LENGTH = 50;                  				//������೤��
#define _TTDB_CONN_STRING_              "QMDB"
using namespace std;

/* classes defined in this file: */
class TOdbcDBException;
class TOdbcDBDatabase;
class TOdbcDBQuery;
class TOdbcDBField;
//class TQMDBParam;
class TException;

namespace TQMDB
{
    extern int iRestartFlag;         //�������̱�־��Ĭ��Ϊ0����Ҫ������Ϊ1ʱ����
}

extern "C" int GetRestartFlag(void);

class TOdbcDBException: public TDBExcpInterface
{
public:
    virtual ~TOdbcDBException() {}
    TOdbcDBException(){}
    TOdbcDBException(const char *sSql, const char* sFormat, ...);
    virtual char * ToString() const;
private:
    void ReadConfig(void);
    void SetRestartFlag();//ZMP:1509456
private:
    static  bool  m_bReadFlag;
    static  int   m_aErrCode[100];
    static  int   m_iErrCodeNum;
};

//ZMP:305013,��ʱ����
class TOdbcDBParam : public TDBParamInterface
{
    friend class TOdbcDBQuery;
public:
    virtual ~TOdbcDBParam();
private:
    TOdbcDBParam();
    void ClearInfo();
};

/*
class TQMDBParam  
{
    friend class TQMDBQuery;
public:
    TQMDBParam();
    virtual ~TQMDBParam();
    int     AsInteger()  throw (TQMDBException);   //��ȡ���ز���ֵ
    double  AsFloat()    throw (TQMDBException);
    long    AsLong()     throw (TQMDBException);
    char*   AsString()   throw (TQMDBException);
    bool    isNULL()     throw (TQMDBException);
private:
    char    *name;
    ub2     dataType;
    int     intValue;
    double  dblValue;         																		//�洢����������ֵ(������)
    long    longValue;        																		//long���ݻ�����
    char   * stringValue;     																		//�ַ������ػ�����
    int    * intArray;        																		//INT����
    double * dblArray;        																		//DOUBLE����
    long   * longArray;       																		//LONG����
    long long * llongArray;   																		//LONG LONG����
    char ** stringArray;      																		//STRING����
    int   stringSize;         																		//STRING�����е�string��С
    bool  fIsOutput;          																		//�Ƿ����������.Ĭ�����������
    sb2   indicator;          																		//�ڷ���ֵʱ���Ƿ�Ϊ��
    OCIBind  *hBind;
};
*/

class TOdbcDBField : public TDBFieldInterface
{
    friend class TOdbcDBQuery;
public: 
    TOdbcDBField();
    virtual ~TOdbcDBField(); 
    bool    isNULL();           																//��fetch�����и��е������Ƿ�Ϊ��
    char   *AsString() throw(TOdbcDBException);
    double  AsFloat() throw (TOdbcDBException);
    long long   AsInteger() throw (TOdbcDBException);
    //Blob����
    //void        AsBlobBuffer(unsigned char* &buf, unsigned int *bufLength) throw (TQMDBException);    //���浽������,�������Ĵ�С�Զ������������ػ�������С*bufLength.
    char*       AsBlobBuffer(int &iBufferLen) throw (TOdbcDBException);														//���BLOB�ֶ�
    //���ڴ���
    char       *AsDateTimeString() throw (TOdbcDBException);   																						//�������͵�����HH:MM:DD HH24:MI��ʽ��ȡ,ʹ��asString()��ȡ����������������ΪHH:MM:DD
    void        AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TOdbcDBException); //�������ڵĸ�������
    void       SetTMdbField(TMdbField* field);
    
    //time_t      AsTimeT() throw (TQMDBException);
    //void        LoadFromFile(const char *fileName) throw (TQMDBException);         //д�뵽blob��
    //void        LoadFromBuffer(unsigned char *buf, unsigned int bufLength) throw (TQMDBException);    //��LOB�������û������������    
    void        ClearDataBuf();																																																		//�α�ѭ����ȡָ���������ݺ�,δ����ϴεĻ���
  														//��ȡ�о���
    //ZMP:305920 begin
    time_t AsTimeT() throw (TOdbcDBException);
    //ZMP:305920 begin
private:
	TMdbField* m_MdbField;
    char sBlobBuf[1024*4];
   
};

class TOdbcDBQuery: public TDBQueryInterface
{
    friend class TOdbcDBField;
public:
    //��Ҫ����
    //constructor & destructor
    TOdbcDBQuery(TOdbcDBDatabase *pTMdbDatabase) throw (TOdbcDBException);
    virtual ~TOdbcDBQuery();
    
    void Close();                                                   //�ر�SQL��䣬��׼��������һ��sql���
    void CloseSQL() {}    
    //����ͬ������
    void SetRepAttr(bool bWriteBuf, bool bRepBuf);                                          //����odbc
    void SetSQL(const char *inSqlstmt,int iPreFetchRows=0) throw (TOdbcDBException);        //����Sqlstatement
    void Open(int prefetchRows=PREFETCH_ROWS) throw (TOdbcDBException); //��SQL SELECT��䷵�ؽ����
    bool Next() throw (TOdbcDBException);                               //�ƶ�����һ����¼
    bool Execute(int iters=1) throw (TOdbcDBException);                 //ִ�з�SELECT���,û�з��ؽ����
    bool Commit();                                                  //�����ύ
    bool Rollback();                                                //����ع�
    int  RowsAffected() ;                        //DELETE/UPDATE/INSERT����޸ĵļ�¼��Ŀ
    //int  GetSQLCode() ;                          //����Oracleִ�н������
	void CheckError(const char* sSql=NULL) throw (TOdbcDBException);        //�����жϵ�ǰ������Ƿ���ȷִ�У�����д�����Ѵ�����Ϣ����errMsg;
	bool Eof(void);
    //������Ϣ���              
    int FieldCount() ; //�ܹ��м�����
    
    //ZMP:305013 begin
    TOdbcDBParam& Param(int iIndex);	//����������ȡ��i������ʵ��,��0��ʼ
    TOdbcDBParam& Param(const char*sParamName); //���ݲ�������ȡ����ʵ��
    //ZMP:305013 end
    
    TOdbcDBField& Field(int index)  throw (TOdbcDBException)    ;           //���ص�i������Ϣ
    TOdbcDBField& Field(const char *fieldName) throw (TOdbcDBException);    //��������(���ִ�Сд)��������Ϣ; ����ʹ��Field(int i)��ø��ߵ�Ч��
    /*begin zmp 841469 */
    /*ֱ�ӻ�ȡֵ*/
    void GetValue(void *pStruct,int* Column)throw (TOdbcDBException);
    void SetUpdateMDBFlag(bool flag)throw (TOdbcDBException);
    bool GetUpdateMDBFlag()throw (TOdbcDBException);
    static bool m_bUpdateMDBFlag;/*�ж����ݿ��Ƿ�������и���*/
    /*end zmp 841469 */  
    bool IsFieldExist(const char *fieldName);  
    //�������Ϣ���
    //int ParamCount() ;

    //TQMDBParam& Param(int index) throw (TOdbcDBException);                //���ص�i������Ϣ
    //TQMDBParam& Param(const char *paramName) throw (TOdbcDBException);    //��������(���ִ�Сд)��������Ϣ; ����ʹ��Field(int i)��ø��ߵ�Ч��
    
    //���������ò���ֵ
    void SetParameter(const char *paramName, const char* paramValue, bool isOutput = false) throw (TOdbcDBException);   
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TOdbcDBException);    //add by fu.wenjun@20041125
    void SetParameter(const char *paramName, int paramValue, bool isOutput = false) throw (TOdbcDBException); 
    void SetParameter(const char *paramName, double paramValue, bool isOutput = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName, long paramValue, bool isOutput = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName, long long paramValue, bool isOutput = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName,const char* paramValue,int iBufferLen,bool isOutput = false) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(const char *paramName) throw (TOdbcDBException);

    void SetParameter(int iParamIndex,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(int iParamIndex) throw (TOdbcDBException);     //���ò���Ϊ��

    //ZMP:305920 begin
    void SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException);
    //ZMP:305920 end
    
    /*��ע:����int iArraySize=0Ϊ�˼���Informix���ֵ�ͬ������,OCI���ֲ������κδ���*/
    //�������
    void SetParamArray(const char *paramName, char     ** paramValue, int iStructSize, int iStrSize ,int iArraySize=0,bool isOutput = false) throw (TOdbcDBException);  
    void SetParamArray(const char *paramName, int       * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TOdbcDBException); 
    void SetParamArray(const char *paramName, double    * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TOdbcDBException);
    void SetParamArray(const char *paramName, long      * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TOdbcDBException);
    void SetParamArray(const char *paramName, long long * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TOdbcDBException);
    void SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize=0,bool isOutput=false) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ��������
    virtual long long GetSequenceByName(const char * sName)throw (TOdbcDBException); //ZMP_559430
private:
	TMdbQuery *m_pQuery;
	TOdbcDBField m_tField[64]; //Ĭ�����64���ֶ�,ZMP:331611 

};


class TOdbcDBDatabase: public TDBInterface 
{
    friend class TOdbcDBQuery;
    //friend class TQMDBSession;
    //friend class TDBDirPath;
    friend class TOdbcDBField;
public:
    TOdbcDBDatabase() throw (TOdbcDBException);
    virtual ~TOdbcDBDatabase();
    void SetLogin(const char *user, const char *password, const char *tnsString) throw (TOdbcDBException);
    bool Connect(bool bUnused=false) throw (TOdbcDBException);
    bool Connect(const char *usr, const char *pwd, const char *tns, bool bUnused=false) throw (TOdbcDBException);
    bool Connect( const char* db, const char* server,const char* user, const char* password, unsigned int port = 0 ) throw (TOdbcDBException);
    int Disconnect() throw (TOdbcDBException);
    //�����жϵ�ǰ������Ƿ���ȷִ�У�����д�����Ѵ�����Ϣ����errMsg;
    void CheckError(const char* sSql=NULL) throw (TOdbcDBException);  
    void Commit();
    void Rollback();
    bool IsConnect() throw (TOdbcDBException);						//�������ݿ��Ƿ���������
    const char* GetProvider() {return NULL;}
    TOdbcDBQuery *CreateDBQuery() throw (TDBExcpInterface);
    TMdbDatabase *m_pMdbDatabase;
private:
    
    //TOdbcDBQuery *m_pTQMDBQuery;			
   																			//���ڲ������ݿ��Ƿ���������
};




#ifdef HP_UNIX
//  #pragma pack 4
#endif
 
#endif   //_TOraQuery_h

