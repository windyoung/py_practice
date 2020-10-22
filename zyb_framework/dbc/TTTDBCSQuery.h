//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#ifndef _TTTDBCSQUERY_H_
#define _TTTDBCSQUERY_H_

#include "TDBInterface.h"

#ifdef _QM
 #include "TQMDBCSQuery.h"
#else

#include "TTTDBQuery.h"

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

#define TIMESTEN_HOME_PATH_ENV          "TIMESTEN_HOME"     //��ȡtimesten·���Ļ�������
#define TIMESTEN_CS_LIB_NAME            "libttclient.so"     //timesten��cs������

#define _TTDBCS_CONN_STRING_              "TTDBCS"

/*CSģʽ�ĺ���*/
typedef SQLRETURN (*CS_SQLAllocEnv)(SQLHENV* phenv) ;                  //SQLAllocEnv
typedef SQLRETURN (*CS_SQLFreeEnv)(SQLHENV henv);                      //SQLFreeEnv
typedef SQLRETURN (*CS_SQLAllocConnect)(SQLHENV henv, SQLHDBC* phdbc); //SQLAllocConnect
typedef SQLRETURN (*CS_SQLConnect)(
  SQLHDBC        hdbc,
  SQLCHAR*       szDSN,
  SQLSMALLINT    cbDSN,
  SQLCHAR*       szUID,
  SQLSMALLINT    cbUID,
  SQLCHAR*       szAuthStr,
  SQLSMALLINT    cbAuthStr);                                           //SQLConnect
typedef SQLRETURN (*CS_SQLSetConnectOption)(
  SQLHDBC        hdbc,
  SQLUSMALLINT   fOption,
  SQLULEN        vParam);                                              //SQLSetConnectOption
typedef SQLRETURN (*CS_SQLDisconnect)(
  SQLHDBC        hdbc);                                                //SQLDisconnect
typedef SQLRETURN (*CS_SQLFreeConnect)(
  SQLHDBC        hdbc);                                                //SQLFreeConnect
typedef SQLRETURN (*CS_SQLTransact)(
  SQLHENV        henv,
  SQLHDBC        hdbc,
  SQLUSMALLINT   fType);                                               //SQLTransact
typedef SQLRETURN (*CS_SQLAllocStmt)(
  SQLHDBC        hdbc,
  SQLHSTMT*      phstmt);                                              //SQLAllocStmt
typedef SQLRETURN (*CS_SQLError)(
  SQLHENV        henv,
  SQLHDBC        hdbc,
  SQLHSTMT       hstmt,
  SQLCHAR*       szSqlState,
  SQLINTEGER*    pfNativeError,
  SQLCHAR*       szErrorMsg,
  SQLSMALLINT    cbErrorMsgMax,
  SQLSMALLINT*   pcbErrorMsg);                                         //SQLError
typedef SQLRETURN (*CS_SQLFreeStmt)(
  SQLHSTMT       hstmt,
  SQLUSMALLINT   fOption);                                             //SQLFreeStmt
typedef SQLRETURN (*CS_SQLPrepare)(
  SQLHSTMT       hstmt,
  SQLCHAR*       szSqlStr,
  SQLINTEGER     cbSqlStr);                                            //SQLPrepare
typedef SQLRETURN (*CS_SQLNumParams)(
  SQLHSTMT        hstmt,
  SQLSMALLINT*    pcpar);                                              //SQLNumParams
typedef SQLRETURN (*CS_SQLDescribeParam)(
  SQLHSTMT        hstmt,
  SQLUSMALLINT    ipar,
  SQLSMALLINT*    pfSqlType,
  SQLULEN*        pcbColDef,
  SQLSMALLINT*    pibScale,
  SQLSMALLINT*    pfNullable);                                         //SQLDescribeParam
typedef SQLRETURN (*CS_SQLNumResultCols)(
  SQLHSTMT       hstmt,
  SQLSMALLINT*   pccol);                                               //SQLNumResultCols
typedef SQLRETURN (*CS_SQLSetStmtOption)(
  SQLHSTMT       hstmt,
  SQLUSMALLINT   fOption,
  SQLROWCOUNT    vParam);                                              //SQLSetStmtOption
typedef SQLRETURN (*CS_SQLDescribeCol)(
  SQLHSTMT       hstmt,
  SQLUSMALLINT   icol,
  SQLCHAR*       szColName,
  SQLSMALLINT    cbColNameMax,
  SQLSMALLINT*   pcbColName,
  SQLSMALLINT*   pfSqlType,
  SQLULEN*       pcbColDef,
  SQLSMALLINT*   pibScale,
  SQLSMALLINT*   pfNullable);                                           //SQLDescribeCol
typedef SQLRETURN (*CS_SQLBindCol)(
  SQLHSTMT       hstmt,
  SQLUSMALLINT   icol,
  SQLSMALLINT    fCType,
  SQLPOINTER     rgbValue,
  SQLLEN         cbValueMax,
  SQLLEN*        pcbValue);                                             //SQLBindCol
typedef SQLRETURN (*CS_SQLParamOptions)(
  SQLHSTMT        hstmt,
  SQLROWSETSIZE   crow,
  SQLROWSETSIZE*  pirow);                                               //SQLParamOptions
typedef SQLRETURN (*CS_SQLExecute)(
  SQLHSTMT       hstmt);                                                //SQLExecute
typedef SQLRETURN (*CS_SQLFetch)(
  SQLHSTMT       hstmt);                                                //SQLFetch

#ifdef TT_1122
typedef SQLRETURN (*CS_SQLExtendedFetch)(
  SQLHSTMT        hstmt,
  SQLUSMALLINT    fFetchType,
  SQLLEN          irow,
  SQLULEN*        pcrow,
  SQLUSMALLINT*   rgfRowStatus);
#else
typedef SQLRETURN (*CS_SQLExtendedFetch)(
  SQLHSTMT        hstmt,
  SQLUSMALLINT    fFetchType,
  SQLROWOFFSET    irow,
  SQLROWSETSIZE*  pcrow,
  SQLUSMALLINT*   rgfRowStatus);                                        //SQLExtendedFetch
#endif

typedef SQLRETURN (*CS_SQLRowCount)(
  SQLHSTMT       hstmt,
  SQLLEN*        pcrow);                                                //SQLRowCount
typedef SQLRETURN (*CS_SQLBindParameter)(
  SQLHSTMT     hstmt,
  SQLUSMALLINT ipar,
  SQLSMALLINT  fParamType,
  SQLSMALLINT  fCType,
  SQLSMALLINT  fSqlType,
  SQLULEN      cbColDef,
  SQLSMALLINT  ibScale,
  SQLPOINTER   rgbValue,
  SQLLEN       cbValueMax,
  SQLLEN*      pcbValue);                                               //SQLBindParameter

/*CSģʽ�ĺ���*/

using namespace std;

extern int CS_DBError(RETCODE rc,HENV henv,HDBC hdbc,HSTMT hstmt,ostringstream &ostrOut);

class TOdbcDBCSDatabase;
class TOdbcDBCSQuery;
class TOdbcDBCSField;
class TOdbcDBCSParam;
class TException;
class SOHelper;

class TTTCSInterface
{
public:
    TTTCSInterface(void);
    virtual ~TTTCSInterface(void);
    TTTCSInterface(const TTTCSInterface& oTTTCSInterface);
public:
    //CS����
    SOHelper *              m_pSoHelper;
    CS_SQLAllocEnv          cs_SQLAllocEnv;
    CS_SQLFreeEnv           cs_SQLFreeEnv;
    CS_SQLAllocConnect      cs_SQLAllocConnect;
    CS_SQLConnect           cs_SQLConnect;
    CS_SQLSetConnectOption  cs_SQLSetConnectOption;
    CS_SQLDisconnect        cs_SQLDisconnect;
    CS_SQLFreeConnect       cs_SQLFreeConnect;
    CS_SQLTransact          cs_SQLTransact;
    CS_SQLAllocStmt         cs_SQLAllocStmt;
    CS_SQLError             cs_SQLError;
    CS_SQLFreeStmt          cs_SQLFreeStmt;
    CS_SQLPrepare           cs_SQLPrepare;
    CS_SQLNumParams         cs_SQLNumParams;

    CS_SQLDescribeParam     cs_SQLDescribeParam;
    CS_SQLNumResultCols     cs_SQLNumResultCols;
    CS_SQLSetStmtOption     cs_SQLSetStmtOption;
    CS_SQLDescribeCol       cs_SQLDescribeCol;
    CS_SQLBindCol           cs_SQLBindCol;
    CS_SQLParamOptions      cs_SQLParamOptions;
    CS_SQLExecute           cs_SQLExecute;
    CS_SQLFetch             cs_SQLFetch;
    CS_SQLExtendedFetch     cs_SQLExtendedFetch;
    CS_SQLRowCount          cs_SQLRowCount;
    CS_SQLBindParameter     cs_SQLBindParameter;
};

class TTTCSFactory
{
public:
    static TTTCSInterface* GetTTInstance(void);
private:
    static TTTCSInterface *m_pInstance;
};

class TOdbcDBCSField : public TDBFieldInterface
{
    friend class TOdbcDBCSQuery;

public:
    virtual ~TOdbcDBCSField();
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
    TOdbcDBCSField();
    void ClearInfo();

    time_t AsDateTime_t() throw (TOdbcDBException);
    int AsDateTime_Detail(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException);

    TOdbcDBCSQuery *m_pTOdbcDBQuery;								//���QUERYָ��
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
class TOdbcDBCSParam : public TDBParamInterface
{
    friend class TOdbcDBCSQuery;
public:
    virtual ~TOdbcDBCSParam();
 //   char* GetName();

private:
    TOdbcDBCSParam();
    void ClearInfo();

    TOdbcDBCSQuery *m_pTOdbcDBQuery;								//���QUERYָ��
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

class TOdbcDBCSQuery: public TDBQueryInterface
{
    friend class TOdbcDBCSField;
    friend class TOdbcDBCSParam;
public:
    TOdbcDBCSQuery(TOdbcDBCSDatabase *pTOdbcDBCSDatabase);
    virtual ~TOdbcDBCSQuery();

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
    TOdbcDBCSField& Field(int iIndex) throw (TOdbcDBException);												//����������ȡ��i����ʵ��,��0��ʼ
    TOdbcDBCSField& Field(const char *sFieldName) throw (TOdbcDBException);//����������ȡ��ʵ��

    //�������صĲ���
    int ParamCount();																											//��ȡ��������
    TOdbcDBCSParam& Param(int iIndex);												//����������ȡ��i������ʵ��,��0��ʼ
    TOdbcDBCSParam& Param(const char*sParamName); //���ݲ�������ȡ����ʵ��
    /*begin zmp 841469 */
    /*ֱ�ӻ�ȡֵ*/
    void GetValue(void *pStruct,int* Column)throw (TOdbcDBException);
    void SetUpdateMDBFlag(bool flag)throw (TOdbcDBException);
    bool GetUpdateMDBFlag()throw (TOdbcDBException);
    static bool m_bUpdateMDBFlag;/*�ж����ݿ��Ƿ�������и���*/
    /*end zmp 841469 */

    //���ò���ֵ
    void SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TDBExcpInterface) {}; //δʵ��,��Ϊ����
    
    void SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused = false) throw (TOdbcDBException); 
    void SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
    void SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused = false) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(const char *sParamName) throw (TOdbcDBException);     //���ò���Ϊ��

    //��
    void SetParameter(int iParamIndex,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(int iParamIndex,long lParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,double dParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,long long llParamValue) throw (TOdbcDBException);
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(int iParamIndex) throw (TOdbcDBException);     //���ò���Ϊ��

////////////////////////////////////////////////////////////////////////////////
//138555 begin
    void SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused = false) throw (TOdbcDBException);
//138555 end
////////////////////////////////////////////////////////////////////////////////

//141877 begin
    void SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException);
//141877 end

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
    void GetFieldBufferInfo(TOdbcDBCSField &oTOdbcDBField,int &iRowsCount);
    //���ݲ�����sParamName,������ֵ���iValueFlag(0��ʾ��ֵ,1��ʾδ��ֵ,2��ʾ����),��������������������0��ʼ��ȡָ������
    TOdbcDBCSParam* ParamByCondition(const char *sParamName,int iValueFlag=2,int iIndex=0);

    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,const char* sParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,int iParamValue) throw (TOdbcDBException); 
    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,long lParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,double dParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,long long llParamValue) throw (TOdbcDBException);
    void SetParameter(TOdbcDBCSParam *pTOdbcDBParam,const char* sParamValue,int iBufferLen) throw (TOdbcDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(TOdbcDBCSParam *pTOdbcDBParam) throw (TOdbcDBException);     //���ò���Ϊ��
    //ZMP_559430
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
    char*    m_sSql;
    RETCODE rc;
    HENV    henv;
    HDBC    hdbc;
    HSTMT   hstmt;
    TOdbcDBCSDatabase *m_pTOdbcDBDatabase;
    int m_iParamCount;																					//����ʵ�ʸ���
    TOdbcDBCSParam    *m_pTOdbcDBParamArray;	//������Ϣ������ַ
    int m_iFieldCount;																					//ѡ����ʵ�ʸ���
    TOdbcDBCSField    *m_pTOdbcDBFieldArray;	//ѡ������Ϣ������ַ
    int m_iQueryState;
    int m_iParamElementCount;														//����ֵ����
    long m_lAffectRows;																				//�����漰�ļ�¼��

    int m_iPreFetchRows;																			//SELECT���λ�ȡ�α�����
#ifdef TT_1122
    SQLULEN  m_iFetchedRows;										//ʵ�����λ�ȡ�ļ�¼��
#else
    SQLROWSETSIZE m_iFetchedRows;									//ʵ�����λ�ȡ�ļ�¼��
#endif    
    SQLUSMALLINT *aRowsStatus;													//���λ�ȡ��¼��״ֵ̬����ָ��
    int m_iRowIndex;																							//NEXT�ļ�¼����

    char m_sLongLong[SIZE_LONG_LONG];						//long long������ַ�����

////////////////////////////////////////////////////////////////////////////////
//133994 begin
    int m_iPrevprefetchRows;
//133994 end
////////////////////////////////////////////////////////////////////////////////

};

class TOdbcDBCSDatabase: public TDBInterface
{
    friend class TOdbcDBCSQuery;
public:
    TOdbcDBCSDatabase();
    virtual ~TOdbcDBCSDatabase();

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

    void GetDataInfo(HENV &henv,HDBC &hdbc);													//��ȡ����������Ϣ
    bool IsConnect() ;																																			//�������ݿ��Ƿ���������
    const char* GetProvider() {return _TTDB_CONN_STRING_;}
    TOdbcDBCSQuery *CreateDBQuery() throw (TOdbcDBException);
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
    TOdbcDBCSQuery *m_pTOdbcDBQuery;																						 //���ڲ������ݿ��Ƿ���������
};

#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif  //_TTTDBCSQUERY_H_

#endif


