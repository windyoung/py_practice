//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#ifndef _TTTDBQUERY_H_
#define _TTTDBQUERY_H_

#include "TDBInterface.h"

#ifdef _QM
	#include "TQMDBQuery.h"
#else

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
#pragma warning(disable:4290)       																								//������������throw (�쳣��)�澯
#pragma warning(disable:4267)       																								//size_t��int�澯 
#pragma warning(disable:4996)       																								//�������
#endif

#ifdef HP_UNIX
//  #pragma pack 8
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

/* Isolation. ���뼶��0Ϊserializable��1ΪRead-Committed */
#define ISOLATION_SERIALIZABLE             0
#define ISOLATION_READ_COMMITTED           1


#define _TTDB_CONN_STRING_              "TTDB"

using namespace std;

extern int DBError(RETCODE rc,HENV henv,HDBC hdbc,HSTMT hstmt,ostringstream &ostrOut);

namespace TTTDB
{
	extern int bRestartFlag;         //�������̱�־��Ĭ��Ϊ0����Ҫ������Ϊ1ʱ���� 
}

extern "C" int GetRestartFlag(void);


class TOdbcDBException;
class TOdbcDBDatabase;
class TOdbcDBQuery;
class TOdbcDBField;
class TOdbcDBParam;
class TException;

class TOdbcDBException: public TDBExcpInterface
{
public:
    virtual ~TOdbcDBException() {}
    TOdbcDBException(const char *sSql, const char* sFormat, ...);
public:
    virtual char * ToString() const;
private:
	void ReadConfig(void);
	void SetRestartFlag();
private:
	static	bool  m_bReadFlag;
	static	int   m_aErrCode[100];
	static	int   m_iErrCodeNum;
    static       char m_sStateCode[100][32];
    static       int  m_iStateCodeNum;
};


class TOdbcDBField : public TDBFieldInterface
{
    friend class TOdbcDBQuery;

public:
    virtual ~TOdbcDBField();
    bool    isNULL();
    char*   AsString() throw (TOdbcDBException);

////////////////////////////////////////////////////////////////////////////////
//138555 begin
    time_t AsTimeT() throw (TOdbcDBException);
//138555 end
////////////////////////////////////////////////////////////////////////////////

    double  AsFloat() throw (TOdbcDBException);
    long long   AsInteger() throw (TOdbcDBException);
    //�������ڵĸ�������
    void    AsDateTime(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException);
    char*   AsDateTimeString() throw (TOdbcDBException);    						//YYYYMMDDHHMISS
    char* AsBlobBuffer(int &iBufferLen);		//���BLOB/BINARY�ֶ�
    const char * AsName();
    int DataType(void);																			//0�ַ� 1���� 2���� 3 ����
    void ClearDataBuf() {}
private:
    TOdbcDBField();
    void ClearInfo();

    time_t AsDateTime_t() throw (TOdbcDBException);
    int AsDateTime_Detail(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException);

    TOdbcDBQuery *m_pTOdbcDBQuery;								//���QUERYָ��
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
class TOdbcDBParam : public TDBParamInterface
{
    friend class TOdbcDBQuery;
public:
    virtual ~TOdbcDBParam();
    char* GetName();

private:
    TOdbcDBParam();
    void ClearInfo();

    TOdbcDBQuery *m_pTOdbcDBQuery;								//���QUERYָ��
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

class TOdbcDBQuery: public TDBQueryInterface
{
    friend class TOdbcDBField;
    friend class TOdbcDBParam;
public:
    TOdbcDBQuery(TOdbcDBDatabase *pTOdbcDBDatabase);
    virtual ~TOdbcDBQuery();

    //�ر�SQL��䣬��׼��������һ��sql���
    void Close() ;
    void CloseSQL(); //Close the cursor associated with the SELECT statement
    //����Ҫִ�е�SQL
    void SetSQL(const char *sSqlStatement,int iPreFetchRows=DEFAULT_PREFETCH_ROWS) throw (TOdbcDBException);
    //��SQL SELECT��䷵�ؽ����,iPreFetchRows��ʵ������
    void Open(int prefetchRows=0) throw (TOdbcDBException);  //����Ϊ�˼��ݽӿڶ����
    //�ƶ�����һ����¼
    bool Next()throw (TOdbcDBException);
    //ִ��SQL
    bool Execute(int iExecuteRows=DEFAULT_EXECUTE_ROWS) throw (TOdbcDBException);
    //������
    bool TransBegin();
    //�����ύ
    bool Commit() ;
    //����ع�
    bool Rollback() ;
    //DELETE/UPDATE/INSERT����޸ĵļ�¼��Ŀ,SELECT���ĿǰNext֮��ļ�¼��
    int RowsAffected();
	bool Eof(void) { return m_iQueryState == QUERY_FETCH_EOF;}

    //������صĲ���
    int FieldCount();																											//��ȡ�и���
    TOdbcDBField& Field(int iIndex) throw (TOdbcDBException);												//����������ȡ��i����ʵ��,��0��ʼ
    TOdbcDBField& Field(const char *sFieldName) throw (TOdbcDBException);//����������ȡ��ʵ��
    /*begin zmp 841469 */
    /*ֱ�ӻ�ȡֵ*/
    void GetValue(void *pStruct,int* Column)throw (TOdbcDBException);
    void SetUpdateMDBFlag(bool flag)throw (TOdbcDBException);
    bool GetUpdateMDBFlag()throw (TOdbcDBException);
    static bool m_bUpdateMDBFlag;/*�ж����ݿ��Ƿ�������и���*/
    /*end zmp 841469 */

    //�������صĲ���
    int ParamCount();																											//��ȡ��������
    TOdbcDBParam& Param(int iIndex);												//����������ȡ��i������ʵ��,��0��ʼ
    TOdbcDBParam& Param(const char*sParamName); //���ݲ�������ȡ����ʵ��

    //���ò���ֵ
    void SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TDBExcpInterface) {}; //δʵ��,��Ϊ����
    
    void SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused = false) throw (TOdbcDBException); 
    void SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused = false) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(const char *sParamName) throw (TOdbcDBException);     //���ò���Ϊ��

////////////////////////////////////////////////////////////////////////////////
//138555 begin
    void SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
//138555 end
////////////////////////////////////////////////////////////////////////////////

//141877 begin
    void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException);
//141877 end

    //��
    void SetParameter(int iParamIndex,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(int iParamIndex) throw (TOdbcDBException);     //���ò���Ϊ��


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

private:
    //������ݿ����,�д��쳣
    void CheckError(const char* sSql=NULL) throw (TOdbcDBException);
    //����SQL,�滻:����Ϊ?��ʽ
    void ParseSql(char* sSqlStatement);
    //����ѡ���еĻ�����(SQL_DATA����,����,����)
    void GetFieldBufferInfo(TOdbcDBField &oTOdbcDBField,int &iRowsCount);
    //���ݲ�����sParamName,������ֵ���iValueFlag(0��ʾ��ֵ,1��ʾδ��ֵ,2��ʾ����),��������������������0��ʼ��ȡָ������
    TOdbcDBParam* ParamByCondition(const char *sParamName,int iValueFlag=2,int iIndex=0);

    void SetParameter(TOdbcDBParam *pTOdbcDBParam,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBParam *pTOdbcDBParam,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(TOdbcDBParam *pTOdbcDBParam,long lParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBParam *pTOdbcDBParam,double dParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBParam *pTOdbcDBParam,long long llParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBParam *pTOdbcDBParam,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(TOdbcDBParam *pTOdbcDBParam) throw (TOdbcDBException);     //���ò���Ϊ��

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

    //char    m_sSql[MAX_SQL_SIZE];										//Ҫִ�е�SQL���
    char*   m_sSql;
    RETCODE rc;
    HENV    henv;
    HDBC    hdbc;
    HSTMT   hstmt;
    TOdbcDBDatabase *m_pTOdbcDBDatabase;
    int m_iParamCount;																					//����ʵ�ʸ���
    TOdbcDBParam    *m_pTOdbcDBParamArray;	//������Ϣ������ַ
    int m_iFieldCount;																					//ѡ����ʵ�ʸ���
    TOdbcDBField    *m_pTOdbcDBFieldArray;	//ѡ������Ϣ������ַ
    int m_iQueryState;
    int m_iParamElementCount;														//����ֵ����
    long m_lAffectRows;																				//�����漰�ļ�¼��

    int m_iPreFetchRows;									   //SELECT���λ�ȡ�α�����
#ifdef TT_1122
    SQLULEN  m_iFetchedRows;	                               //ʵ�����λ�ȡ�ļ�¼��
#else    
    SQLROWSETSIZE m_iFetchedRows;							   //ʵ�����λ�ȡ�ļ�¼��
#endif    
    SQLUSMALLINT *aRowsStatus;								   //���λ�ȡ��¼��״ֵ̬����ָ��
    int m_iRowIndex;																							//NEXT�ļ�¼����

    char m_sLongLong[SIZE_LONG_LONG];						//long long������ַ�����

////////////////////////////////////////////////////////////////////////////////
//133994 begin
    int m_iPrevprefetchRows;
//133994 end
////////////////////////////////////////////////////////////////////////////////

};

class TOdbcDBDatabase: public TDBInterface
{
    friend class TOdbcDBQuery;
public:
    TOdbcDBDatabase();
    virtual ~TOdbcDBDatabase();

    void SetLogin(const char *sUser,const char *sPassword,const char *sServerName) throw (TOdbcDBException);       //���õ�½��Ϣ
    bool Connect(bool bIsAutoCommit=false) throw (TOdbcDBException);                            //�������ݿ�
    bool Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit=false) throw (TOdbcDBException);       //�������ݿ�
    int Disconnect() throw(TOdbcDBException);                               	//�Ͽ����ݿ�����
    void TransBegin();                               																								//��������
    void Commit() ;
    void Rollback() ;
    bool IsNullConnect()
    { 
        return m_iConnectFlag==0;
    };//�Ƿ��ǿյ�odbc����

    void GetDataInfo(HENV &env,HDBC &dbc);													//��ȡ����������Ϣ
    bool IsConnect() ;																								//�������ݿ��Ƿ���������
    int  SetIsolationLevel(int  IsolationLevel);                      //���ø��뼶��
    const char* GetProvider() {return _TTDB_CONN_STRING_;}
    TOdbcDBQuery *CreateDBQuery() throw (TOdbcDBException);
private:
    //������ݿ����,�д��쳣
    void CheckError(const char* sSql=NULL) throw (TOdbcDBException);

    string m_strUser;																																				//���ݿ��û���
    string m_strPassword;																																//���ݿ��û�����
    string m_strServerName;																														//���ݿ���
    int m_iConnectFlag;          																								//���ݿ��½��־,0��ʾû������,1��ʾ�Ѿ�����
    bool   m_bAutoCommitFlag;																												//�Զ��ύ��־��True�Զ��ύ��false���Զ��ύ
    RETCODE rc;
    HENV    henv;
    HDBC    hdbc;
    TOdbcDBQuery *m_pTOdbcDBQuery;																						 //���ڲ������ݿ��Ƿ���������
};

#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif  //_TTTDBQUERY_H_

#endif

