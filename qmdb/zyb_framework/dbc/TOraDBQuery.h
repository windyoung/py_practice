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

//�쳣ʹ��
const   int MAX_ERR_CAT_LENGTH = 50;                  				//������೤��

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
    TDBException(sword errNumb, OCIError *err,const char *cat,const char *sql);//ִ��OCI���������Ĵ���
    TDBException(const char *sql, const char* errFormat, ...);
public:
    virtual char * ToString() const;
private:
    char errCategory[MAX_ERR_CAT_LENGTH+1];  //�������
};


class TDBParam  : public TDBParamInterface
{
    friend class TDBQuery;
public:
    TDBParam();
    virtual ~TDBParam();
    int     AsInteger()  throw (TDBException);   //��ȡ���ز���ֵ
    double  AsFloat()    throw (TDBException);
    long    AsLong()     throw (TDBException);
    char*   AsString()   throw (TDBException);
    bool    isNULL()     throw (TDBException);
private:
	  void ClearInfo(){};
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

class TDBField : public TDBFieldInterface
{
    friend class TDBQuery;
public: 
    virtual ~TDBField(); 
    bool    isNULL();           																//��fetch�����и��е������Ƿ�Ϊ��
    char   *AsString() throw(TDBException);
    double  AsFloat() throw (TDBException);
    long long   AsInteger() throw (TDBException);
  //blob �� clob����
	void        AsLobString(char* const buf, unsigned int maxLength, unsigned int& bufLength) throw (TDBException);	
    //Blob����
    void        AsBlobFile(const char *fileName) throw (TDBException);          	//��ȡ��file��
    void        AsBlobBuffer(unsigned char* &buf, unsigned int *bufLength) throw (TDBException);    //���浽������,�������Ĵ�С�Զ������������ػ�������С*bufLength.
    char*       AsBlobBuffer(int &iBufferLen) throw (TDBException);														//���BLOB�ֶ�
    //���ڴ���
    char       *AsDateTimeString() throw (TDBException);   																						//�������͵�����HH:MM:DD HH24:MI��ʽ��ȡ,ʹ��asString()��ȡ����������������ΪHH:MM:DD
    void        AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBException); //�������ڵĸ�������
    time_t      AsTimeT() throw (TDBException);
    void        LoadFromFile(const char *fileName) throw (TDBException);         //д�뵽blob��
    void        LoadFromBuffer(unsigned char *buf, unsigned int bufLength) throw (TDBException);    //��LOB�������û������������    
    void        ClearDataBuf();																																																		//�α�ѭ����ȡָ���������ݺ�,δ����ϴεĻ���

public:
    char *GetFieldName();    																			//��ȡ����
    long GetFieldType();     																			//��ȡ��������
    long GetFieldSize();     																			//��ȡ�д�С
    int  GetFieldPrecision();     														//��ȡ�о���
private:
    //���ڴ���
    void    AsDateTimeInternal(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBException); //�������ڵĸ�������,û��������У�飬ֻ���ڲ�����                    

    int AsDateTimeInternalNew(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBException);

    char    *name;              																//�ֶ�����
    long    size;               																//���ݳ���
    long    type;               																//�������� in(INT_TYPE,FLOAT_TYPE,DATE_TYPE,STRING_TYPE,ROWID_TYPE)
    int     precision;          																//��ֵ�ܳ���
    int     scale;              																//��ֵ��С�������
    bool    nullable;           																//�ֶζ���ʱ�Ƿ�����Ϊ��ֵ--Ϊ�˺���������һ��
    TDBQuery *fParentQuery;                 				//ָ���Field�����ڵ�Query
    TDBField();
    //���ݻ�����,�ֱ�Ϊ�ַ���������������������ռ�
    ub1 fStrBuffer[MAX_STRING_VALUE_LENGTH];    //���ڱ���ת��Ϊ�ַ������ֵ
    ub1 *fDataBuf;                      								//�ڷ����ֶ�ʱ���ÿռ�max(���е���󳤶�,MAX_STRING_VALUE_LENGTH), ��Destructor���ͷ�
    OCILobLocator *hBlob;               								//֧��LOB
    sb2 *fDataIndicator;                								//��defineByPos��ʹ�ã�������fetchʱ�쿴�Ƿ����ֶ�ֵ���ء��ֶ�ֵ�Ƿ񱻽ض�;valueIsNULL, isTruncated���ݴ�ֵ��ý��
                                        								
    ub2   fReturnDataLen;               								//��ȡ����ʱ���ص���ʵ����
    ub2 fInternalType;                  								//Oracle�ڲ���������
    ub2 fRequestType;                   								//�ڶ�ȡ����ʱ���������������
    OCIDefine *hDefine;                 								//���ڶ�ȡ����Ϣ
                                        								
    long lDataBufLen;                   								//��¼fDataBufʵ�ʳ���,�г�*����Ԫ�ظ���
    char *sBlob;																																//���BLOB�ֶ�ֵ
                                        								
    																			                 								
    OCILobLocator **ahBlob;																					//��ȡѡ��������ΪBLOB���ֶ�ֵ
    int iBlobCount;

	char intStr[128];
};

class TDBQuery: public TDBQueryInterface
{
    friend class TDBField;
public:
    //��Ҫ����
    void Close();                                                   //�ر�SQL��䣬��׼��������һ��sql���
    void CloseSQL() {}                                              //����odbc
    void SetSQL(const char *inSqlstmt,int iPreFetchRows=0) throw (TDBException);        //����Sqlstatement
    void Open(int prefetchRows=PREFETCH_ROWS) throw (TDBException); //��SQL SELECT��䷵�ؽ����
    bool Next() throw (TDBException);                               //�ƶ�����һ����¼
    bool Execute(int iters=1) throw (TDBException);                 //ִ�з�SELECT���,û�з��ؽ����
    bool Commit();                                                  //�����ύ
    bool Rollback();                                                //����ع�
    int  RowsAffected() { return (int)fTotalRowsFetched;};               //DELETE/UPDATE/INSERT����޸ĵļ�¼��Ŀ
    int  GetSQLCode() { return fErrorNo;};                          //����Oracleִ�н������
    bool Eof(void) { return fLastRecordFlag;}

    //������Ϣ���              
    int FieldCount() ;                                              //�ܹ��м�����
    TDBField& Field(int index)  throw (TDBException)    ;           //���ص�i������Ϣ
    TDBField& Field(const char *fieldName) throw (TDBException);    //��������(���ִ�Сд)��������Ϣ; ����ʹ��Field(int i)��ø��ߵ�Ч��
    /*begin zmp 841469 */
    /*ֱ�ӻ�ȡֵ*/
    void GetValue(void *pStruct,int* Column)throw (TDBException);
    void SetUpdateMDBFlag(bool flag)throw (TDBException);
    bool GetUpdateMDBFlag()throw (TDBException);
    /*end zmp 841469 */

    bool IsFieldExist(const char *fieldName);
    //�������Ϣ���
    int ParamCount() ;
    bool IsParamExist(const char *paramName);
    TDBParam& Param(int index) throw (TDBException);                //���ص�i������Ϣ
    TDBParam& Param(const char *paramName) throw (TDBException);    //��������(���ִ�Сд)��������Ϣ; ����ʹ��Field(int i)��ø��ߵ�Ч��
    //���������ò���ֵ
    void SetParameter(const char *paramName, const char* paramValue, bool isOutput = false) throw (TDBException);   
    void SetParameter(const char *paramName, const char paramValue, bool isOutput = false) throw (TDBException);    //add by fu.wenjun@20041125
    void SetParameter(const char *paramName, int paramValue, bool isOutput = false) throw (TDBException); 
    void SetParameter(const char *paramName, double paramValue, bool isOutput = false) throw (TDBException);
    void SetParameter(const char *paramName, long paramValue, bool isOutput = false) throw (TDBException);
    void SetParameter(const char *paramName, long long paramValue, bool isOutput = false) throw (TDBException);
    void SetParameter(const char *paramName,const char* paramValue,int iBufferLen,bool isOutput = false) throw (TDBException);//���ڴ���BLOB/BINARY�����ֶ�
    void SetParameterNULL(const char *paramName) throw (TDBException);

	void SetParameter(int iParamIndex,const char* sParamValue) throw (TDBExcpInterface){};
    void SetParameter(int iParamIndex,int iParamValue) throw (TDBExcpInterface){}; 
    void SetParameter(int iParamIndex,long lParamValue) throw (TDBExcpInterface){};
    void SetParameter(int iParamIndex,double dParamValue) throw (TDBExcpInterface){};
    void SetParameter(int iParamIndex,long long llParamValue) throw (TDBExcpInterface){};
    void SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TDBExcpInterface){};//���ڴ���BLOB/BINARY�����ֶ�

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

    /*��ע:����int iArraySize=0Ϊ�˼���Informix���ֵ�ͬ������,OCI���ֲ������κδ���*/
    //�������
    void SetParamArray(const char *paramName, char     ** paramValue, int iStructSize, int iStrSize ,int iArraySize=0,bool isOutput = false) throw (TDBException);  
    void SetParamArray(const char *paramName, int       * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBException); 
    void SetParamArray(const char *paramName, double    * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBException);
    void SetParamArray(const char *paramName, long      * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBException);
    void SetParamArray(const char *paramName, long long * paramValue, int iStructSize,               int iArraySize=0,bool isOutput = false) throw (TDBException);
    void SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize=0,bool isOutput=false) throw (TDBException);//���ڴ���BLOB/BINARY�����ֶ��������

    //constructor & destructor
    TDBQuery(TDBDatabase *oradb) throw (TDBException);
    TDBQuery(TDBDatabase *oradb,TDBSession *session) throw (TDBException);
    virtual ~TDBQuery();
    static void SetBillLog(TBillLog *pBillLog);
    virtual long long GetSequenceByName(const char * sName)throw (TDBExcpInterface); //ZMP_559430
private:
    char *fSqlStmt;                         							//����open��Select��䣬���Է������
    ub2 fStmtType;                          							//***ub2!!! ����sqlstmt������:SELECT/UPDATE/DELETE/INSERT/CREATE/BEGIN/ALTER...
    bool fActivated;                        							//�Ƿ��Ѿ����ڴ�״̬���ڵ���OCIStmtPrepare�ɹ���ΪTrue
    bool fLastRecordFlag;
    unsigned    fFetched;                   							//0..prefetchRows
    unsigned    fPrefetchRows;              							//1.. 
    unsigned    fCurrRow;                   							//0..fetched */
    unsigned    fTotalRowsFetched;          							//rows fetched from the start
                                            							
    int fFieldCount;                        							//�ֶθ���
    TDBDatabase *db;                        							//��query�����ĸ�Dabase,��Constructor�д���
    TDBField *fieldList;                    							//���ڲ�����������ֶ���Ϣ
    void GetFieldsDef() throw (TDBException);      //����ֶ���Ϣ,��Ϊ�ֶη���ȡֵ�Ļ�����

    TDBParam *ParamByName(const char *paramName) throw (TDBException);//���ڲ�ʹ�ã�ֱ�ӷ��ز�����ָ��

////////////////////////////////////////////////////////////////////////////////
//141877 begin
    TDBParam *ParamByName(int paramName) throw (TDBException);//���ڲ�ʹ�ã�ֱ�ӷ��ز�����ָ��
//141877 end
////////////////////////////////////////////////////////////////////////////////

    void CheckError(const char* sSql=NULL) throw (TDBException);        //�����жϵ�ǰ������Ƿ���ȷִ�У�����д�����Ѵ�����Ϣ����errMsg;

    int fParamCount;                        							//��������
    TDBParam *paramList;                											//���в������õ���Ϣ
    void GetParamsDef() throw (TDBException);      //��setSQLʱ���ò�����Ϣ
    int nTransTimes;                        							//�Ƿ�����ִ�й�Execute()����������Ա���ع�.
    static TBillLog *gpBillLog;
  
    OCIStmt *hStmt;                     											//���ڷ���sql����handle
    OCIError *hErr;                     											//������
    sword fErrorNo;                     											//�����
    bool fEof;                              							//��Fetchʱ���Ѿ��ﵽ���һ����¼,��ֹ�Ѿ��������һ����¼����fetch�����Ĵ���
    bool fBof;                              							//��Open()ʱ��ΪTrue,��Next()���������Ϊfalse;�����ж��û��Ƿ���Դӻ������ж�ȡ��ֵ,�ò�����δ���
    bool fOpened;                           							//���ݼ��Ƿ��    
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
    bool IsConnect() throw (TDBException);						//�������ݿ��Ƿ���������
    const char* GetProvider() {return _ORADB_CONN_STRING_;}
    TDBQuery *CreateDBQuery() throw (TDBExcpInterface);
private:
    OCISession *hUser;
    OCISvcCtx *hDBSvc;      																				//���ڵ�¼���ӷ�����      
    char *usr, *pwd, *tns;  																				//��¼��
    bool fConnected;        																				//��Connect���Ƿ����ӳɹ�
    sword fErrorNo;         																				//�����
    void CheckError(const char* sSql=NULL) throw (TDBException);     //�����жϵ�ǰ������Ƿ���ȷִ�У�����д�����Ѵ�����Ϣ����errMsg;
    OCIError *hDBErr;
    OCIEnv *hEnv;
    OCIServer *hSvr;
    TDBQuery *m_pTDBQuery;																						//���ڲ������ݿ��Ƿ���������
    char m_sErrIgnore[255];

};



//�ⲿ������,ȡ������
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
