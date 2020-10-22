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
#pragma warning(disable:4290)       																								//������������throw (�쳣��)�澯
#pragma warning(disable:4267)       																								//size_t��int�澯 
#pragma warning(disable:4996)       																								//�������
#endif

#ifdef HP_UNIX
//  #pragma pack 8
#endif
/*
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
    bool    isNULL();           																//��fetch�����и��е������Ƿ�Ϊ��
    char   *AsString() throw(TOdbcDBException);
    double  AsFloat() throw (TOdbcDBException);
    long long   AsInteger() throw (TOdbcDBException);
    //Blob����
    char*       AsBlobBuffer(int &iBufferLen) throw (TOdbcDBException);														//���BLOB�ֶ�
    //���ڴ���
    char       *AsDateTimeString() throw (TOdbcDBException);   																						//�������͵�����HH:MM:DD HH24:MI��ʽ��ȡ,ʹ��asString()��ȡ����������������ΪHH:MM:DD
    void        AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TOdbcDBException); //�������ڵĸ�������
    void       SetTMdbField(const TMdbClientField& field);
    
    //time_t      AsTimeT() throw (TOdbcDBException);
    //void        LoadFromFile(const char *fileName) throw (TOdbcDBException);         //д�뵽blob��
    //void        LoadFromBuffer(unsigned char *buf, unsigned int bufLength) throw (TOdbcDBException);    //��LOB�������û������������    
    void        ClearDataBuf();																																																		//�α�ѭ����ȡָ���������ݺ�,δ����ϴεĻ���
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
    //��Ҫ����
    
    TOdbcDBCSQuery(TOdbcDBCSDatabase *pTOdbcDBCSDatabase) throw (TOdbcDBException);
    virtual ~TOdbcDBCSQuery();
    
    void Close();                                                   //�ر�SQL��䣬��׼��������һ��sql���
    void CloseSQL();    
    //����ͬ������
    void SetRepAttr(bool bWriteBuf, bool bRepBuf);                                          //����odbc
    void SetSQL(const char *sSqlStatement,int iPreFetchRows=0) throw (TOdbcDBException);        //����Sqlstatement
    void Open(int prefetchRows=PREFETCH_ROWS) throw (TOdbcDBException); //��SQL SELECT��䷵�ؽ����
    bool Next() throw (TOdbcDBException);                               //�ƶ�����һ����¼
    bool Execute(int iExecuteRows=1) throw (TOdbcDBException);                 //ִ�з�SELECT���,û�з��ؽ����
    bool TransBegin();
    bool Commit();                                                  //�����ύ
    bool Rollback();                                                //����ع�
    //dongchun begin û��ʵ�֣�����ʱע�͵�
    int  RowsAffected() ;                        //DELETE/UPDATE/INSERT����޸ĵļ�¼��Ŀ
    bool Eof(void);
    //dongchun end
    //int  GetSQLCode() ;                          //����Oracleִ�н������
	void CheckError(const char* sSql=NULL) throw (TOdbcDBException);        //�����жϵ�ǰ������Ƿ���ȷִ�У�����д�����Ѵ�����Ϣ����errMsg;
	bool IsFieldExist(const char *fieldName);

    //������Ϣ���              
    int FieldCount() ;  //�ܹ��м�����

    //ZMP:305013 begin
    TOdbcDBParam& Param(int iIndex);	//����������ȡ��i������ʵ��,��0��ʼ
    TOdbcDBParam& Param(const char*sParamName); //���ݲ�������ȡ����ʵ��
    //ZMP:305013 end
    
    TOdbcDBCSField& Field(int iIndex) throw (TOdbcDBException);												//����������ȡ��i����ʵ��,��0��ʼ
    TOdbcDBCSField& Field(const char *sFieldName) throw (TOdbcDBException);//����������ȡ��ʵ��
    /*begin zmp 841469 */
    /*ֱ�ӻ�ȡֵ*/
    void GetValue(void *pStruct,int* Column)throw (TOdbcDBException);
    void SetUpdateMDBFlag(bool flag)throw (TOdbcDBException);
    bool GetUpdateMDBFlag()throw (TOdbcDBException);
    static bool m_bUpdateMDBFlag;/*�ж����ݿ��Ƿ�������и���*/
    /*end zmp 841469 */

    //bool IsFieldExist(const char *fieldName);
    
    //�������Ϣ���
    //dongchun begin û��ʵ�֣�����ʱע�͵�
    //int ParamCount() ;
    //dongchun end
    //TQMDBParam& Param(int index) throw (TOdbcDBException){return NULL;}                //���ص�i������Ϣ
    //TQMDBParam& Param(const char *paramName) throw (TOdbcDBException){return NULL;}    //��������(���ִ�Сд)��������Ϣ; ����ʹ��Field(int i)��ø��ߵ�Ч��
    
    //���������ò���ֵ
    void SetParameter(const char *paramName, const char* paramValue, bool isOutput = false) throw (TOdbcDBException);   
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TOdbcDBException);    //add by fu.wenjun@20041125
    void SetParameter(const char *paramName, int paramValue, bool isOutput = false) throw (TOdbcDBException); 
    void SetParameter(const char *paramName, double paramValue, bool isOutput = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName, long paramValue, bool isOutput = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName, long long paramValue, bool isOutput = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName,const char* paramValue,int iBufferLen,bool isOutput = false) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(const char *paramName) throw (TOdbcDBException);

    //ZMP:305920 begin
    void SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException);
    //ZMP:305920 end
    
    //��
    void SetParameter(int iParamIndex,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(int iParamIndex) throw (TOdbcDBException);     //���ò���Ϊ��


    /*��ע:����int iArraySize=0Ϊ�˼���Informix���ֵ�ͬ������,OCI���ֲ������κδ���*/
    //�������
     //�����������ֵ
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
        int iArraySize,bool bOutput=false) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ��������
    
    //��ȡ����
    virtual long long GetSequenceByName(const char * sName)throw (TOdbcDBException); //ZMP_559430

private:
	TMdbClientQuery *m_pQuery;
	//dongchun ���ﲻ��Ҫ���飬ֻ��Ҫ������Ϳ�����
	TOdbcDBCSField m_tField; //Ĭ�����40���ֶ�

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
    bool IsConnect() throw (TOdbcDBException);						//�������ݿ��Ƿ���������
    bool IsNullConnect();
    
    const char* GetProvider() {return _TTDBCS_CONN_STRING_;}
    TOdbcDBCSQuery *CreateDBQuery() throw (TDBExcpInterface);
    TMdbClientDatabase *m_pMdbDatabase;		
private:
    //�����жϵ�ǰ������Ƿ���ȷִ�У�����д�����Ѵ�����Ϣ����errMsg;
    void CheckError(const char* sSql=NULL) throw (TOdbcDBException);  
    															//���ڲ������ݿ��Ƿ���������
};


//-------------------------------------------------------------


#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif  //_TTTDBQUERY_H_

