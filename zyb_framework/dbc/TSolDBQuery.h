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
#pragma warning(disable:4290)       																								//������������throw (�쳣��)�澯
#pragma warning(disable:4267)       																								//size_t��int�澯 
#pragma warning(disable:4996)       																								//�������
#endif

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
    //�������ڵĸ�������
    void    AsDateTime(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TSolDBException);
    char*   AsDateTimeString() throw (TSolDBException);    						//YYYYMMDDHHMISS
    char* AsBlobBuffer(int &iBufferLen);		//���BLOB/BINARY�ֶ�
    const char * AsName();
    int DataType(void);																			//0�ַ� 1���� 2���� 3 ����
    void ClearDataBuf() {}
private:
    TSolDBField();
    void ClearInfo();

    TSolDBQuery *m_pTOdbcDBQuery;								//���QUERYָ��
    string strFieldName;																		//ѡ��������
    int iIndex;																											//��1��ʼ,����������SQL����������
    SQLSMALLINT ssiSqlDataType;											//��������
    SQLULEN  sulPrecision;																//����
    SQLSMALLINT ssiScale;																	//����
    SQLSMALLINT ssiNullable;														//�Ƿ�ɿ�
    char *sData;																										//ѡ����ֵ������
    SQLLEN slDataLen;																					//ѡ����ֵ�������ֽڳ���
    SQLSMALLINT ssiDataType;														//ѡ����ֵ��������������
    SQLLEN* pslValueLen;																		//ѡ����ʵ�ʷ���ֵ���Ȼ�����

    char sDataValue[MAX_COLUMN_DATA];					//����AsSring()ʹ��
    double dDataValue;																				//����AsFloat()ʹ��
    long long llDataValue;																//����AsInteger()ʹ��,ע��SQL_SMALLINT��ӦSWORD,SQL_INTEGER��ӦSDWORD
    char sDateTime[SIZE_DATE_TIME];							//����AsDateTimeSring()ʹ��
    SDWORD sdwDataValue;																		//����SQL_INTEGER���ݴ��
    SWORD swDataValue;																				//����SQL_SAMLLINT���ݴ��

    int iRowsCount;
    int iMaxRowsCount;
};

//��ע:�������Ͳ�ʵ��������Ͳ�������
class TSolDBParam 
{
    friend class TSolDBQuery;
public:
    virtual ~TSolDBParam();
    char* GetName();

private:
    TSolDBParam();
    void ClearInfo();

    TSolDBQuery *m_pTOdbcDBQuery;								//���QUERYָ��
    string strParamName;																		//��������
    int iIndex;																											//��1��ʼ,����������SQL����������
    SQLSMALLINT ssiSqlDataType;											//��������
    SQLULEN  sulPrecision;																//����
    SQLSMALLINT ssiScale;																	//����
    SQLSMALLINT ssiNullable;														//�Ƿ�ɿ�
    void *pData;																										//����ֵ������
    SQLLEN slDataLen;																					//ѡ����ֵ�������ֽڳ���
    SQLSMALLINT ssiDataType;														//ѡ����ֵ��������������
    int iElementCount;																				//����Ԫ�ظ���,�ж��Ƿ������ֵ����Ϊ>0����ֵ
    long long llElementSize;														//����Ԫ�ش�С

    int iData;
    double dData;
    SQLLEN llValueLen;

    SQLLEN *allValueLen;																		//����ֵʵ�ʳ���,����SetParameter()
    bool bSetValue;																							//���ù�����ֵ
};

class TSolDBQuery: public TDBQueryInterface
{
    friend class TSolDBField;
    friend class TSolDBParam;
public:
    TSolDBQuery(TSolDBDatabase *pTOdbcDBDatabase);
    virtual ~TSolDBQuery();

    //�ر�SQL��䣬��׼��������һ��sql���
    void Close() ;
    void CloseSQL(); //Close the cursor associated with the SELECT statement
    //����Ҫִ�е�SQL
    void SetSQL(const char *sSqlStatement,int iPreFetchRows=DEFAULT_PREFETCH_ROWS) throw (TSolDBException);
    //��SQL SELECT��䷵�ؽ����,iPreFetchRows��ʵ������
    void Open(int prefetchRows=0) throw (TSolDBException);  //����Ϊ�˼��ݽӿڶ����
    //�ƶ�����һ����¼
    bool Next()throw (TSolDBException);
    //ִ��SQL
    bool Execute(int iExecuteRows=DEFAULT_EXECUTE_ROWS) throw (TSolDBException);
    //������
    bool TransBegin();
    //�����ύ
    bool Commit() ;
    //����ع�
    bool Rollback() ;
    //DELETE/UPDATE/INSERT����޸ĵļ�¼��Ŀ,SELECT���ĿǰNext֮��ļ�¼��
    int RowsAffected();

    //������صĲ���
    int FieldCount();																											//��ȡ�и���
    TSolDBField& Field(int iIndex) throw (TSolDBException);												//����������ȡ��i����ʵ��,��0��ʼ
    TSolDBField& Field(const char *sFieldName) throw (TSolDBException);//����������ȡ��ʵ��
    /*begin zmp 841469 */
    /*ֱ�ӻ�ȡֵ*/
    void GetValue(void *pStruct,int* Column)throw (TSolDBException);
    void SetUpdateMDBFlag(bool flag)throw (TSolDBException);
    bool GetUpdateMDBFlag()throw (TSolDBException);
    /*end zmp 841469 */

    //�������صĲ���
    int ParamCount();																											//��ȡ��������
    TSolDBParam& Param(int iIndex);												//����������ȡ��i������ʵ��,��0��ʼ
    TSolDBParam& Param(const char*sParamName); //���ݲ�������ȡ����ʵ��

    //���ò���ֵ
    void SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused = false) throw (TSolDBException);
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TDBExcpInterface) {}; //δʵ��,��Ϊ����
    
    void SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused = false) throw (TSolDBException); 
    void SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused = false) throw (TSolDBException);
    void SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused = false) throw (TSolDBException);
    void SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused = false) throw (TSolDBException);
    void SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused = false) throw (TSolDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(const char *sParamName) throw (TSolDBException);     //���ò���Ϊ��

    //��
    void SetParameter(int iParamIndex,const char* sParamValue) throw (TSolDBException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TSolDBException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TSolDBException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TSolDBException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TSolDBException);
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TSolDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(int iParamIndex) throw (TSolDBException);     //���ò���Ϊ��


    //�����������ֵ
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
        int iArraySize,bool bOutput=false) throw (TSolDBException);//���ڴ���BLOB/BINARY�����ֶ��������

private:
    //������ݿ����,�д��쳣
    void CheckError(const char* sSql=NULL) throw (TSolDBException);
    //����SQL,�滻:����Ϊ?��ʽ
    void ParseSql(char* sSqlStatement);
    //����ѡ���еĻ�����(SQL_DATA����,����,����)
    void GetFieldBufferInfo(TSolDBField &oTOdbcDBField,int &iRowsCount);
    //���ݲ�����sParamName,������ֵ���iValueFlag(0��ʾ��ֵ,1��ʾδ��ֵ,2��ʾ����),��������������������0��ʼ��ȡָ������
    TSolDBParam* ParamByCondition(const char *sParamName,int iValueFlag=2,int iIndex=0);

    void SetParameter(TSolDBParam *pTOdbcDBParam,const char* sParamValue) throw (TSolDBException);
    void SetParameter(TSolDBParam *pTOdbcDBParam,int iParamValue) throw (TSolDBException); 
    void SetParameter(TSolDBParam *pTOdbcDBParam,long lParamValue) throw (TSolDBException);
    void SetParameter(TSolDBParam *pTOdbcDBParam,double dParamValue) throw (TSolDBException);
    void SetParameter(TSolDBParam *pTOdbcDBParam,long long llParamValue) throw (TSolDBException);
    void SetParameter(TSolDBParam *pTOdbcDBParam,const char* sParamValue,int iBufferLen) throw (TSolDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(TSolDBParam *pTOdbcDBParam) throw (TSolDBException);     //���ò���Ϊ��

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

    char    m_sSql[MAX_SQL_SIZE];										//Ҫִ�е�SQL���
    RETCODE rc;
    SQLHENV    henv;
    SQLHDBC    hdbc;
    SQLHSTMT   hstmt;
    TSolDBDatabase *m_pTOdbcDBDatabase;
    int m_iParamCount;																					//����ʵ�ʸ���
    TSolDBParam    *m_pTOdbcDBParamArray;	//������Ϣ������ַ
    int m_iFieldCount;																					//ѡ����ʵ�ʸ���
    TSolDBField    *m_pTOdbcDBFieldArray;	//ѡ������Ϣ������ַ
    int m_iQueryState;
    int m_iParamElementCount;														//����ֵ����
    long m_lAffectRows;																				//�����漰�ļ�¼��

    int m_iPreFetchRows;																			//SELECT���λ�ȡ�α�����
    SQLROWSETSIZE m_iFetchedRows;										//ʵ�����λ�ȡ�ļ�¼��
    SQLUSMALLINT *aRowsStatus;													//���λ�ȡ��¼��״ֵ̬����ָ��
    int m_iRowIndex;																							//NEXT�ļ�¼����

    char m_sLongLong[SIZE_LONG_LONG];						//long long������ַ�����

};

class TSolDBDatabase: public TDBInterface
{
    friend class TSolDBQuery;
public:
    TSolDBDatabase();
    virtual ~TSolDBDatabase();

    void SetLogin(const char *sUser,const char *sPassword,const char *sServerName) throw (TSolDBException);       //���õ�½��Ϣ
    bool Connect(bool bIsAutoCommit=false) throw (TSolDBException);                            //�������ݿ�
    bool Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit=false) throw (TSolDBException);       //�������ݿ�
    int Disconnect() throw(TSolDBException);                               	//�Ͽ����ݿ�����
    void TransBegin();                               																								//��������
    void Commit() ;
    void Rollback() ;
    bool IsNullConnect()
    { 
        return m_iConnectFlag==0;
    };//�Ƿ��ǿյ�odbc����

    void GetDataInfo(SQLHENV &henv,SQLHDBC &hdbc);													//��ȡ����������Ϣ
    bool IsConnect() ;																																			//�������ݿ��Ƿ���������
    const char* GetProvider() {return _TSOLDB_CONN_STRING_;}
    TSolDBQuery *CreateDBQuery() throw (TSolDBException);
private:
    //������ݿ����,�д��쳣
    void CheckError(const char* sSql=NULL) throw (TSolDBException);

    string m_strUser;																																				//���ݿ��û���
    string m_strPassword;																																//���ݿ��û�����
    string m_strServerName;																														//���ݿ���
    int m_iConnectFlag;          																								//���ݿ��½��־,0��ʾû������,1��ʾ�Ѿ�����
    bool   m_bAutoCommitFlag;																												//�Զ��ύ��־��True�Զ��ύ��false���Զ��ύ
    RETCODE rc;
    SQLHENV    henv;
    SQLHDBC    hdbc;
    TSolDBQuery *m_pTOdbcDBQuery;																						 //���ڲ������ݿ��Ƿ���������
};
 
#endif  //_TSOLDBQUERY_H_


