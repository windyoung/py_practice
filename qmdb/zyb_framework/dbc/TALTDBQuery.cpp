//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#include "TALTDBQuery.h"
#include "Common.h"
#include "TStrFunc.h"
#include "debug_new.h"
#include "TSplit.h"
#include "TStrFunc.h"
#include "ReadIni.h"

#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

SQLROWSETSIZE   iRow=0;
bool TOdbcDBQuery::m_bUpdateMDBFlag=true;/*判断数据库是否允许进行更新*/

int DBError(RETCODE rc,HENV henv,HDBC hdbc,HSTMT hstmt,ostringstream &ostrOut)
{
    ostrOut<<endl;
    switch(rc)
    {
    case SQL_SUCCESS:
        {
            return 0;
        }
    case SQL_SUCCESS_WITH_INFO:
        {
            ostrOut<<"RET_CODE=SQL_SUCCESS_WITH_INFO"<<endl;
            break;
        }
    case SQL_ERROR:
        {
            ostrOut<<"RET_CODE=SQL_ERROR"<<endl;    
            break;
        }
    case SQL_INVALID_HANDLE:
        {
            ostrOut<<"RET_CODE=SQL_INVALID_HANDLE"<<endl;   
            break;
        }
    default:
        {
            ostrOut<<"RET_CODE="<<rc<<endl; 
            break;
        }
    }//end of switch(rc);

    if((rc==SQL_SUCCESS_WITH_INFO)
        ||(rc==SQL_ERROR)
        )
    {
        char    sql_state[SQL_MAX_MESSAGE_LENGTH] = {0};//ZMP:477242
        SDWORD  native_error;
        char    err_msg[SQL_MAX_MESSAGE_LENGTH] = {0};//ZMP:477242
        SWORD   msg_len;

        /*RETCODE rc_err=*/
		SQLError(henv,hdbc,hstmt,(SQLCHAR*)sql_state,&native_error,
            (SQLCHAR*)err_msg,(SWORD)sizeof(err_msg),&msg_len);

        ostrOut<<"SQL_STATE   ="<<sql_state<<endl
            <<"NATIVE_ERROR="<<native_error<<endl
            <<"ERR_MSG     ="<<err_msg<<endl
            <<"MEG_LEN     ="<<msg_len<<endl;
    }
    return -1;
}

TOdbcDBException::TOdbcDBException(const char *sSql, const char* sFormat, ...)
{
    va_list args;

    memset(m_sErrMsg,0,sizeof(m_sErrMsg));
	memset(m_sErrSql,0,sizeof(m_sErrSql));
    m_lErrCode=0;

    va_start(args,sFormat); 
    vsnprintf(m_sErrMsg, sizeof(m_sErrMsg), sFormat, args);
    m_sErrMsg[sizeof(m_sErrMsg)-1] = '\0';
    va_end(args);
    m_sErrMsg[sizeof(m_sErrMsg)-1]='\0';    

    if( sSql )
    {
        strncpy(m_sErrSql,sSql,MAX_SQLSTMT_LENGTH);
    }
    else
    {
        strncpy(m_sErrSql,"sql is empty.",MAX_SQLSTMT_LENGTH);
    }
    m_sErrSql[MAX_SQLSTMT_LENGTH]='\0';
}

char * TOdbcDBException::ToString() const
{
    static char sBuf[1024] = {0};//ZMP:477242
    sBuf[0] = 0;

    snprintf(sBuf, sizeof(sBuf), "TOdbcDBException:\n    ErrMsg=%s\n    ErrSql=%s\n", GetErrMsg(), GetErrSql());
    return sBuf;
}

/*类TOdbcDBDatabase定义部分*/
TOdbcDBDatabase::TOdbcDBDatabase()
{
    m_strUser="";
    m_strPassword="";
    m_strServerName="";
    m_iConnectFlag=0;
    m_bAutoCommitFlag=false;
    henv=NULL;
    hdbc=NULL;
    rc=0;

    //ALLOCATE ENV
    rc=SQLAllocEnv(&henv);
    CheckError("TOdbcDBDatabase::SQLAllocEnv");

    m_pTOdbcDBQuery=NULL;
	m_stmtCommit = SQL_NULL_HSTMT; //zl add for altibase fast commit
	m_stmtRollback = SQL_NULL_HSTMT; //zl add for altibase fast commit
}

TOdbcDBDatabase::~TOdbcDBDatabase()
{
    try   //析构时,如果有异常也不处理.
    {
        if(m_pTOdbcDBQuery!=NULL)
        {
            m_pTOdbcDBQuery->Close();
            delete m_pTOdbcDBQuery;
			m_pTOdbcDBQuery = NULL;
        }

        //释放连接
        Disconnect();

        if(henv!=NULL)
        {
            //FREE ENV
            rc=SQLFreeEnv(henv);
            CheckError("TOdbcDBDatabase::SQLFreeEnv");
            henv=NULL;
        }
		m_stmtCommit = SQL_NULL_HSTMT; 
		m_stmtRollback = SQL_NULL_HSTMT; 
    }
    catch(TOdbcDBException &oe)
    {
        cout<<"TOdbcDBDatabase::~TOdbcDBDatabase() TOdbcDBException:" << oe.GetErrMsg() << endl;
    }
    catch(...)
    {
        cout<<"TOdbcDBDatabase::~TOdbcDBDatabase() catch ... Exception"<<endl;
    }
    
}

void TOdbcDBDatabase::SetLogin(const char *sUser,const char *sPassword,const char *sServerName)throw (TOdbcDBException)
{
    m_strUser=sUser;
    m_strPassword=sPassword;
    m_strServerName=sServerName;
	
    return ;   
}

bool TOdbcDBDatabase::Connect(bool bIsAutoCommit) throw (TOdbcDBException)
{
	m_iConnectFlag=0;
	m_bAutoCommitFlag=bIsAutoCommit;
	//ALLOCATE CONNECT
	rc=SQLAllocConnect(henv,&hdbc);
	CheckError("TOdbcDBDatabase::SQLAllocConnect");

	char NLS[20] = "US7ASCII";   // NLS_USE ( KO16KSC5601, US7ASCII )
	char ConnStr[1024] = "";

	//本地连接
	sprintf(ConnStr,"DSN=127.0.0.1;UID=%s;PWD=%s;CONNTYPE=%d;NLS_USE=%s",
			m_strUser.c_str(), m_strPassword.c_str(), 2, NLS);

	printf("connect string: %s\n",ConnStr);

	//"DSN=127.0.0.1;UID=ccb;PWD=ccb;CONNTYPE=3;NLS_USE=US7ASCII" 
	rc=SQLDriverConnect( hdbc, NULL,(SQLCHAR *)ConnStr , SQL_NTS,
						  NULL, 0, NULL,
						 SQL_DRIVER_NOPROMPT );

	CheckError("TOdbcDBDatabase::SQLConnect");

	//设置不默认提交
	if(m_bAutoCommitFlag)
	{	 
		rc=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_ON);	  
	}
	else
	{		 
		rc=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF);
	}
	
	CheckError("TOdbcDBDatabase::SQLSetConnectOption");

	//设置连接标识
	m_iConnectFlag=1;

	//zl add for altibase fast commit and rollback begin
	rc = SQLAllocStmt(hdbc, &m_stmtCommit); 
	CheckError("TOdbcDBDatabase::Connect m_stmtCommit");
	rc = SQLPrepare(m_stmtCommit, (SQLCHAR *)"commit", SQL_NTS);
	CheckError("TOdbcDBDatabase::Connect commit");
	
	rc = SQLAllocStmt(hdbc, &m_stmtRollback); 
	CheckError("TOdbcDBDatabase::Connect m_stmtRollback");
	rc = SQLPrepare(m_stmtRollback, (SQLCHAR *)"rollback;", SQL_NTS);
	CheckError("TOdbcDBDatabase::Connect rollback");
	
	//连接上就立刻提交内存数据库2008-1-14 10:47AM zhangyu	
	Commit();

	return true;
}


bool TOdbcDBDatabase::Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit) throw (TOdbcDBException)
{
    SetLogin(sUser,sPassword,sServerName);

    return Connect(bIsAutoCommit);
}

int TOdbcDBDatabase::Disconnect()  throw(TOdbcDBException)
{
    int iRet=0;
	
    if(hdbc!=NULL)
    {
        if(m_iConnectFlag==1)
        {
            //默认回退
            Rollback();

			rc = SQLFreeStmt(m_stmtCommit, SQL_DROP );
		    CheckError("TOdbcDBDatabase::SQLDisconnect m_stmtCommit");

			rc = SQLFreeStmt(m_stmtRollback, SQL_DROP );
			CheckError("TOdbcDBDatabase::SQLDisconnect m_stmtRollback");
			
            rc=SQLDisconnect(hdbc);
            CheckError("TOdbcDBDatabase::SQLDisconnect");
            m_iConnectFlag=0;
        }

        //FREE CONNECT
        rc=SQLFreeConnect(hdbc);
        CheckError("TOdbcDBDatabase::SQLFreeConnect");

        hdbc=NULL;
    }
	
    return iRet;
}

//数据库开启事务
void TOdbcDBDatabase::TransBegin() 
{
    return;
}


//数据库提交
void TOdbcDBDatabase::Commit() 
{
    if(!m_bAutoCommitFlag)
    {
        rc = SQLExecute(m_stmtCommit);
        CheckError("TOdbcDBDatabase::Commit m_stmtCommit");
  	}	
    return;
}

//数据库回滚
void TOdbcDBDatabase::Rollback() 
{
    rc = SQLExecute(m_stmtRollback);
	CheckError("TOdbcDBDatabase::Rollback c_stmtRollback ");
	
    return;
}

//获取数据连接信息
void TOdbcDBDatabase::GetDataInfo(HENV &henv,HDBC &hdbc)
{
    henv=this->henv;
    hdbc=this->hdbc;
    return;
}


bool TOdbcDBDatabase::IsConnect()  
{
    bool bConnectFlag=false;

    if(m_iConnectFlag==1)
    {
        bConnectFlag=true;

        try
        {
            if(m_pTOdbcDBQuery==NULL)
            {
                m_pTOdbcDBQuery=new TOdbcDBQuery(this);
            }

            m_pTOdbcDBQuery->Close();
            m_pTOdbcDBQuery->SetSQL("SELECT 1 FROM DUAL");
            m_pTOdbcDBQuery->Open();
            m_pTOdbcDBQuery->Close();
        }
        catch(TOdbcDBException &oe)
        {
            bConnectFlag = false;   //这边认为有异常那一定是数据库出了问题.
            cout<<"TOdbcDBDatabase::IsConnect() TOdbcDBException:" << oe.GetErrMsg() << endl;
        }
        catch(...)
        {
            throw TOdbcDBException("","TOdbcDBDatabase::IsConnect() catch ... Exception.\n");
        }
    }
    
    return bConnectFlag;
}

TOdbcDBQuery * TOdbcDBDatabase::CreateDBQuery() throw (TOdbcDBException)
{
    return new TOdbcDBQuery(this);
}

//private函数
void TOdbcDBDatabase::CheckError(const char* sSql) throw (TOdbcDBException)
{
    static ostringstream ostrErr;
    int iRet=0;
    ostrErr.str("");

    if((rc!=SQL_SUCCESS)
        &&(rc!=SQL_SUCCESS_WITH_INFO)
        )
    {
        iRet=DBError(rc,henv,hdbc,SQL_NULL_HSTMT,ostrErr);
    }

    if(iRet!=0)
    {
        throw TOdbcDBException(sSql,"Application Error Message=%s",ostrErr.str().c_str());
    }
}

/*类TOdbcDBField定义部分*/
TOdbcDBField::TOdbcDBField()
{
    sData=NULL;
    pslValueLen=NULL;
    iMaxRowsCount=0;
    ClearInfo();
}

TOdbcDBField::~TOdbcDBField()
{
    ClearInfo();
}

bool TOdbcDBField::isNULL()
{
    if(pslValueLen[m_pTOdbcDBQuery->m_iRowIndex]==-1)
    {
        return true;
    }
    return false;
}

char* TOdbcDBField::AsString() throw (TOdbcDBException)
{
	//printf("AsString() : FiledName[%s]  FiledDataType[%d]------ \n",AsName(),ssiSqlDataType);
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBField::AsString","Application Error Message=Data set not opened!");
    }

    memset(sDataValue,0,sizeof(sDataValue));
    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
	case SQL_BINARY:
	case SQL_BLOB:
        {
			return sData;
        }
    case SQL_INTEGER:
        {
            SQLINTEGER value=*((SQLINTEGER*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            snprintf(sDataValue, sizeof(sDataValue), "%d", value);
            sDataValue[sizeof(sDataValue)-1] = '\0';
            break;
        }
    case SQL_SMALLINT:
        {
            short int value=*((short int*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            snprintf(sDataValue, sizeof(sDataValue), "%d", value);
            sDataValue[sizeof(sDataValue)-1] = '\0';
            break;
        }
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_BIGINT:
        {
            snprintf(sDataValue, sizeof(sDataValue), "%.4lf",*(static_cast<double *>((void *)sData)+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex)));//ZMP:305593
            sDataValue[sizeof(sDataValue)-1] = '\0';
            char *pos = strstr(sDataValue,".0000");
            if(pos!=NULL) *pos='\0';
            break;
        }
    case SQL_TIMESTAMP:
    case SQL_TIME:
	case SQL_TYPE_TIMESTAMP:
    case SQL_DATETIME:
        {
			snprintf(sDataValue, sizeof(sDataValue), "%s", AsDateTimeString());
            sDataValue[sizeof(sDataValue)-1] = '\0';
            break ;
        }
    default:
        {
            throw TOdbcDBException("TOdbcDBField::AsString","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
        }
    }
    return sDataValue;
}

int TOdbcDBField::DataType(void)
{
    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
        return 0;
    case SQL_INTEGER:
    case SQL_SMALLINT:
    case SQL_BIGINT:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
        return 1;
    case SQL_FLOAT:
    case SQL_DOUBLE:
        return 2;
    case SQL_TIMESTAMP:
    case SQL_TIME:
    case SQL_TYPE_TIMESTAMP:
        return 3;
    case SQL_BINARY:
	case SQL_BLOB:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
        return 4;
    default:
        {
            throw TOdbcDBException("TOdbcDBField::AsString","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
        }
    }
    return -1;
}

double TOdbcDBField::AsFloat() throw (TOdbcDBException)
{
	//printf("AsFloat() : FiledName[%s]  FiledDataType[%d]------ \n",AsName(),ssiSqlDataType);
	
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBField::AsFloat","Application Error Message=Data set not opened!");
    }

	//dDataValue = 0;

    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
        {
            dDataValue = atof(sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
			break;
        }
    case SQL_INTEGER:
        {
            dDataValue = *((SQLINTEGER*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
			break;
        }
    case SQL_SMALLINT:
        {
            dDataValue = *((short int*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
			break;
        }
    case SQL_DECIMAL:
    case SQL_NUMERIC:
		{         
            dDataValue = *((SQLBIGINT*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
			break;
        }
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_BIGINT:
        {
			//printf("--dDataValue[%ld]\n",dDataValue);
			//printf("--slDataLen[%d]\n",slDataLen);
			//printf("--m_iRowIndex[%d]\n",m_pTOdbcDBQuery->m_iRowIndex);
            dDataValue = *((SQLBIGINT*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
			//printf("--dDataValue[%ld]--\n",dDataValue);
			break;
        }
    default:
        {
            throw TOdbcDBException("TOdbcDBField::AsFloat","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
        }
    }
	
    return dDataValue;
}

long long TOdbcDBField::AsInteger() throw (TOdbcDBException)
{
	//printf("AsInterger() : FiledName[%s]  FiledDataType[%d]------ \n",AsName(),ssiSqlDataType);
	
	if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {
        throw TOdbcDBException("TOdbcDBField::AsInteger","Application Error Message=Data set not opened!");
    }
	llDataValue=(long long)this->AsFloat();
    return llDataValue;
}

void TOdbcDBField::AsDateTime(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException)
{
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {
        throw TOdbcDBException("TOdbcDBField::AsDateTime","Application Error Message=Data set not opened!");
    }
	
    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_TIMESTAMP:
    case SQL_TIME:
        {
            bool bNumber=false;
            int iSegIndex=0;
            char *sCur=sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);
            int iCount=(int)slDataLen;
            iYear=0;
            iMonth=0;
            iDay=0;
            iHour=0;
            iMinute=0;
            iSecond=0;

            while((iCount>0)&&((*sCur)!='\0'))
            {
                iCount--;
                if(((*sCur)>='0')&&((*sCur)<='9'))
                {
                    bNumber=true;
                    switch(iSegIndex)
                    {
                    case 0:
                        {
                            iYear=iYear*10+(*sCur)-'0';
                            break;
                        }
                    case 1:
                        {
                            iMonth=iMonth*10+(*sCur)-'0';
                            break;
                        }
                    case 2:
                        {
                            iDay=iDay*10+(*sCur)-'0';
                            break;
                        }
                    case 3:
                        {
                            iHour=iHour*10+(*sCur)-'0';
                            break;
                        }
                    case 4:
                        {
                            iMinute=iMinute*10+(*sCur)-'0';
                            break;
                        }
                    case 5:
                        {
                            iSecond=iSecond*10+(*sCur)-'0';
                            break;
                        }
                    }//end of switch(iSegIndex);
                }
                else
                {
                    if(bNumber==true)
                    {
                        bNumber=false;
                        iSegIndex++;
                    }
                }

                sCur++;
            }//end of while((*sCur)!='\0')
            break;

        }
	case SQL_TYPE_TIMESTAMP:
	case SQL_DATETIME:	
         {          
			char *sCur=sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);
			iYear=0;
			iMonth=0;
			iDay=0;
			iHour=0;
			iMinute=0;
			iSecond=0;

			iYear = ((SQL_TIMESTAMP_STRUCT*)sCur)->year;
			iMonth = ((SQL_TIMESTAMP_STRUCT*)sCur)->month;
			iDay = ((SQL_TIMESTAMP_STRUCT*)sCur)->day;
			iHour = ((SQL_TIMESTAMP_STRUCT*)sCur)->hour;
			iMinute = ((SQL_TIMESTAMP_STRUCT*)sCur)->minute;
			iSecond = ((SQL_TIMESTAMP_STRUCT*)sCur)->second;

			break;
        }
    default:
        {
            throw TOdbcDBException("TOdbcDBField::AsDateTime","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
		}
    }
    return;
}

//转换时间格式为YYYYMMDDHHMISS
char* TOdbcDBField::AsDateTimeString() throw (TOdbcDBException)
{
	//printf("AsDateTimeString() : FiledName[%s]  FiledDataType[%d]------ \n",AsName(),ssiSqlDataType);
	if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {
        throw TOdbcDBException("TOdbcDBField::AsDateTimeString","Application Error Message=Data set not opened!");
    }
	
    int iYear=0;
    int iMonth=0;
    int iDay=0;
    int iHour=0;
    int iMinute=0;
    int iSecond=0;

	this->AsDateTime(iYear,iMonth,iDay,iHour,iMinute,iSecond);
    memset(sDateTime,0,sizeof(sDateTime));
 
    snprintf(sDateTime, sizeof(sDateTime), "%04d%02d%02d%02d%02d%02d",iYear,iMonth,iDay,iHour,iMinute,iSecond);
    sDateTime[sizeof(sDateTime)-1] = '\0';

	return sDateTime;
}

char* TOdbcDBField::AsBlobBuffer(int &iBufferLen)
{
	//printf("AsBlobBuffer() : FiledName[%s]  FiledDataType[%d]------ \n",AsName(),ssiSqlDataType);
    char *sBlobBuffer=NULL;

    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {
        throw TOdbcDBException("TOdbcDBField::AsBlobBuffer","Application Error Message=Data set not opened!");
    }

	switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_BINARY:
	case SQL_BLOB:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
        {
            iBufferLen=(int)pslValueLen[(m_pTOdbcDBQuery->m_iRowIndex)];
            sBlobBuffer=sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);
            break;
        }
    default:
        {
            throw TOdbcDBException("TOdbcDBField::AsBlobBuffer","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
        }
    }
    return sBlobBuffer;
}

const char * TOdbcDBField::AsName()
{
    return strFieldName.c_str();
}


//private函数
void TOdbcDBField::ClearInfo()
{
    m_pTOdbcDBQuery=NULL;
    strFieldName="";
    iIndex=-1;
    ssiSqlDataType=0;
    sulPrecision=0;
    ssiScale=0;
    ssiNullable=0;

    if((sData!=NULL)
        &&
        ((ssiDataType==SQL_C_CHAR)
        ||(ssiDataType==SQL_C_BINARY)
        ||(ssiDataType==SQL_C_TYPE_TIMESTAMP)
        ||(iMaxRowsCount>1)
        )
        )
    {
        DELETE(sData);
    }
    slDataLen=-1;
    ssiDataType=0;

    DELETE_A(pslValueLen);

    memset(sDataValue,0,sizeof(sDataValue));
    dDataValue=0;
    llDataValue=0;
    sdwDataValue=0;
    swDataValue=0;
    memset(sDateTime,0,sizeof(sDateTime));

    iRowsCount=0;
    iMaxRowsCount=0;
    return;
}

time_t TOdbcDBField::AsTimeT() throw (TOdbcDBException)
{
	//printf("AsTimeT() : FiledName[%s]  FiledDataType[%d]------ \n",AsName(),ssiSqlDataType);
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {
        throw TOdbcDBException("TOdbcDBField::AsTimeT","Application Error Message=Data set not opened!");
    }
	
    time_t tValue = 0;

    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_VARCHAR:
    case SQL_TIMESTAMP:
    case SQL_TIME:
	case SQL_DATE:
	case SQL_TYPE_TIMESTAMP:
        {
            tValue = AsDateTime_t();
            break;
        }
    default:
        {
            throw TOdbcDBException("TOdbcDBField::AsTimeT","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
        }
    }
    return tValue;
}

time_t TOdbcDBField::AsDateTime_t() throw (TOdbcDBException)
{
	//printf("AsDateTime_t() : FiledName[%s]  FiledDataType[%d]------ \n",AsName(),ssiSqlDataType);
	if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {
        throw TOdbcDBException("TOdbcDBField::AsDateTime_Detail","Application Error Message=Data set not opened!");
    }
	
    int iYear=0;
    int iMonth=0;
    int iDay=0;
    int iHour=0;
    int iMinute=0;
    int iSecond=0;

    int iResult=0;
    iResult = this->AsDateTime_Detail(iYear,iMonth,iDay,iHour,iMinute,iSecond);
    if( -1 == iResult )
      return -1;

    struct tm s;
    s.tm_year=iYear-1900;
    s.tm_mon=iMonth-1;
    s.tm_mday=iDay;
    s.tm_hour=iHour;
    s.tm_min=iMinute;
    s.tm_sec=iSecond;
    s.tm_isdst=(int)-1;    //::daylight 是否启动日光节约时间。＝0为不启动。屏蔽夏令时对程序时间的影响。

    return mktime(&s);
}

int TOdbcDBField::AsDateTime_Detail(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException)
{
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {
        throw TOdbcDBException("TOdbcDBField::AsDateTime_Detail","Application Error Message=Data set not opened!");
    }

    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_VARCHAR:
        {
            char *sTemp = NULL;
            sTemp = sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);
            size_t iLen=strlen(sTemp);//ZMP:477242
    
            if((iLen!=14)&&(iLen!=8)&&(iLen != 0))
                throw TOdbcDBException("TOdbcDBField::AsDateTime_Detail","Application Error Message=FieldLength is error!");

            if(iLen == 0)
            {
                return -1;
            }

            char sTempYear[5]={0},sTempMon[3]={0},sTempMDay[3]={0},sTempHour[3]={0},sTempMin[3]={0},sTempSec[3]={0}; //356853 for cppcheck
            strncpy(sTempYear,sTemp,4);
            //sTempYear[4]=0; //356853, 初始化时已做,此处没必要了.
            sTempMon[0]=sTemp[4];
            sTempMon[1]=sTemp[5];
            //sTempMon[2]=0; //356853, 初始化时已做,此处没必要了.
            sTempMDay[0]=sTemp[6]; 
            sTempMDay[1]=sTemp[7]; 
            //sTempMDay[2]=0; //356853, 初始化时已做,此处没必要了.

            // 支持8位
            if(8 == iLen)
            {
                sTempHour[0] = '0';
                sTempHour[1] = '0';
                //sTempHour[2]=0; //356853, 初始化时已做,此处没必要了.

                sTempMin[0]= '0';
                sTempMin[1]= '0';
                //sTempMin[2]=0; //356853, 初始化时已做,此处没必要了.

                sTempSec[0]= '0';
                sTempSec[1]= '0';
                //sTempSec[2]=0; //356853, 初始化时已做,此处没必要了.
            }
            else  // 14位
            {
                sTempHour[0]=sTemp[8];
                sTempHour[1]=sTemp[9]; 
                //sTempHour[2]=0; //356853, 初始化时已做,此处没必要了.

                sTempMin[0]=sTemp[10];
                sTempMin[1]=sTemp[11]; 
                //sTempMin[2]=0; //356853, 初始化时已做,此处没必要了.

                sTempSec[0]=sTemp[12];
                sTempSec[1]=sTemp[13]; 
                //sTempSec[2]=0; //356853, 初始化时已做,此处没必要了.
            }

            iYear=atoi(sTempYear);
            iMonth=atoi(sTempMon);
            iDay=atoi(sTempMDay);
            iHour=atoi(sTempHour);
            iMinute=atoi(sTempMin);
            iSecond=atoi(sTempSec);
            break;
        }
    case SQL_TIMESTAMP:
    case SQL_TIME:
        {
            bool bNumber=false;
            int iSegIndex=0;
            char *sCur=sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);

            if( !strcmp(sCur,"") )
              return -1;

            int iCount=(int)slDataLen;//ZMP:477242
            iYear=0;
            iMonth=0;
            iDay=0;
            iHour=0;
            iMinute=0;
            iSecond=0;

            while((iCount>0)&&((*sCur)!='\0'))
            {
                iCount--;
                if(((*sCur)>='0')&&((*sCur)<='9'))
                {
                    bNumber=true;
                    switch(iSegIndex)
                    {
                    case 0:
                        {
                            iYear=iYear*10+(*sCur)-'0';
                            break;
                        }
                    case 1:
                        {
                            iMonth=iMonth*10+(*sCur)-'0';
                            break;
                        }
                    case 2:
                        {
                            iDay=iDay*10+(*sCur)-'0';
                            break;
                        }
                    case 3:
                        {
                            iHour=iHour*10+(*sCur)-'0';
                            break;
                        }
                    case 4:
                        {
                            iMinute=iMinute*10+(*sCur)-'0';
                            break;
                        }
                    case 5:
                        {
                            iSecond=iSecond*10+(*sCur)-'0';
                            break;
                        }
                    }//end of switch(iSegIndex);
                }
                else
                {
                    if(bNumber==true)
                    {
                        bNumber=false;
                        iSegIndex++;
                    }
                }

                sCur++;
            }//end of while((*sCur)!='\0')
            break;

        }
	case SQL_TYPE_TIMESTAMP:
	case SQL_DATETIME:	
        {          
			char *sCur=sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);
			iYear=0;
			iMonth=0;
			iDay=0;
			iHour=0;
			iMinute=0;
			iSecond=0;

			iYear = ((SQL_TIMESTAMP_STRUCT*)sCur)->year;
			iMonth = ((SQL_TIMESTAMP_STRUCT*)sCur)->month;
			iDay = ((SQL_TIMESTAMP_STRUCT*)sCur)->day;
			iHour = ((SQL_TIMESTAMP_STRUCT*)sCur)->hour;
			iMinute = ((SQL_TIMESTAMP_STRUCT*)sCur)->minute;
			iSecond = ((SQL_TIMESTAMP_STRUCT*)sCur)->second;

			break;
        }
    default:
        {
            throw TOdbcDBException("TOdbcDBField::AsDateTime_Detail","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
        }
    }
    return 1;
}


/*类TOdbcDBParam定义部分*/
TOdbcDBParam::TOdbcDBParam()
{
    pData=NULL;
    allValueLen=NULL;
    ClearInfo();
}

TOdbcDBParam::~TOdbcDBParam()
{
    ClearInfo();
}

//private函数
void TOdbcDBParam::ClearInfo()
{
    m_pTOdbcDBQuery=NULL;
    strParamName="";
    iIndex=-1;
    ssiSqlDataType=0;
    sulPrecision=0;
    ssiScale=0;
    ssiNullable=0;
    if(pData != NULL)
    {
        delete [](char*)pData;
        pData = NULL;
    }
    slDataLen=-1;
    ssiDataType=0;
    iElementCount=-1;
    DELETE_A(allValueLen);    
    bSetValue=false;
    return;
}

/*类TOdbcDBQuery定义部分*/
TOdbcDBQuery::TOdbcDBQuery(TOdbcDBDatabase *pTOdbcDBDatabase)
{
    henv=NULL;
    hdbc=NULL;
    hstmt=NULL;
    rc=0;
	isEof = true; 
    isPrepare = false;
    if(pTOdbcDBDatabase==NULL)
    {
        throw TOdbcDBException("TOdbcDBQuery::TOdbcDBQuery","Application Error Message=Parameter Error!");
    }

    if(pTOdbcDBDatabase->m_iConnectFlag!=1)
    {
        throw TOdbcDBException("TOdbcDBQuery::TOdbcDBQuery","Application Error Message=Database not connected!");
    }

    pTOdbcDBDatabase->GetDataInfo(henv,hdbc);
    m_pTOdbcDBDatabase=pTOdbcDBDatabase;

    //ALLOCATE STMT
    rc=SQLAllocStmt(hdbc,&hstmt);
    CheckError("TOdbcDBQuery::SQLAllocStmt");

    m_iParamCount=0;
    m_pTOdbcDBParamArray=new TOdbcDBParam[MAX_COLUMN_COUNT];
    if(m_pTOdbcDBParamArray==NULL)
    {
        throw TOdbcDBException("TOdbcDBQuery::TOdbcDBQuery","Application Error Message=Malloc error!");
    }

    m_iFieldCount=0;
    m_pTOdbcDBFieldArray=new TOdbcDBField[MAX_COLUMN_COUNT];
    if(m_pTOdbcDBFieldArray==NULL)
    {
        throw TOdbcDBException("TOdbcDBQuery::TOdbcDBQuery","Application Error Message=Malloc error!");
    }

    m_iQueryState=QUERY_BEFORE_OPEN;
    aRowsStatus=NULL;
    m_iRowIndex=0;
    m_iPreFetchRows=0;
    m_iFetchedRows=0;    

	bSetStmtAttr = false;
	
    m_sSql = NULL;
    m_iPrevprefetchRows = 0;
}

TOdbcDBQuery::~TOdbcDBQuery()
{
    if(hstmt!=NULL && (NULL != m_pTOdbcDBDatabase->hdbc)) //342452
    {
        Close();
        rc=SQLFreeStmt(hstmt,SQL_DROP);
        CheckError(m_sSql);
        hstmt=NULL;
        m_iPrevprefetchRows = 0;
    }
    henv=NULL;
    hdbc=NULL;
	isEof = true; 
    isPrepare = false;
    m_pTOdbcDBDatabase=NULL;
	
    DELETE_A(m_sSql);
    DELETE_A(m_pTOdbcDBFieldArray);
    DELETE_A(m_pTOdbcDBParamArray);
    DELETE_A(aRowsStatus);
}

void TOdbcDBQuery::Close()
{
    if( NULL != m_pTOdbcDBDatabase->hdbc ) 
    {
        rc=SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
        CheckError(m_sSql);

	    rc=SQLFreeStmt(hstmt,SQL_UNBIND);
	    CheckError(m_sSql);
		
	    //关闭执行的SQL
	    rc=SQLFreeStmt(hstmt,SQL_CLOSE);
	    CheckError(m_sSql);
    }

    for(int iFieldIndex=0;iFieldIndex<m_iFieldCount;iFieldIndex++)
    {
        m_pTOdbcDBFieldArray[iFieldIndex].ClearInfo();
    }

    for(int iParamIndex=0;iParamIndex<m_iParamCount;iParamIndex++)
    {
        m_pTOdbcDBParamArray[iParamIndex].ClearInfo();
    }


    DELETE_A(aRowsStatus);

    m_iFieldCount=0;
    m_iParamCount=0;
    DELETE_A(m_sSql);
    m_iQueryState=QUERY_BEFORE_OPEN;
    m_lAffectRows=0;
    m_iPreFetchRows=0;

    m_iRowIndex=0;
    m_iFetchedRows=0;

	bSetStmtAttr = false;
    isEof = true; 
    isPrepare = false;
    return;
}

void TOdbcDBQuery::CloseSQL()
{
	rc = SQLFreeStmt(hstmt, SQL_CLOSE);
	CheckError(m_sSql);
	isEof = true;
	
	m_iQueryState=QUERY_BEFORE_OPEN;
    m_iPreFetchRows=0;
    m_iRowIndex=0;
    m_iFetchedRows=0;
}

void TOdbcDBQuery::SetSQL(const char *sSqlStatement,int iPreFetchRows) throw (TOdbcDBException)
{
    char sFieldName[MAX_COLUMN_NAME]="";
    int iNameLen=0;
    if(sSqlStatement==NULL)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetSQL","Application Error Message=Parameter Error!");
    }

	if (isPrepare)
    {
        return;
    }
    DELETE_A(m_sSql);
    size_t iLen=strlen(sSqlStatement);
    m_sSql=new char[iLen+1];
    if(m_sSql==NULL)
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetSQL","no more memory can be allocate!");
    }
    //memset(m_sSql,0x00,iLen+1);
	strncpy(m_sSql,sSqlStatement,iLen);
    m_sSql[iLen]='\0';
    //需要替换:参数格式到
    ParseSql(m_sSql);

    rc=SQLPrepare(hstmt,(SQLCHAR *)m_sSql,SQL_NTS);
    CheckError(m_sSql);
	
	isPrepare = true;
    //NUMBER PARAM;
    int iParamCount=0;
    short int FAR siParamCount=0;
	
    rc=SQLNumParams(hstmt,(short int FAR*)&siParamCount);
	iParamCount=siParamCount;
    CheckError(m_sSql);
    if(iParamCount!=m_iParamCount)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetSQL","Application Error Message=ParseSql() Error!");
    }

    for(int iParamIndex=1;iParamIndex<=m_iParamCount;iParamIndex++)
    {
        rc=SQLDescribeParam(hstmt,(unsigned short)iParamIndex,
            (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType),
            (SQLULEN*)&(m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision),
            (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiScale),
            (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable));
        CheckError(m_sSql);
    }

    //NUMBER FIELD;
    int iFieldCount=0;
    short int FAR siFieldCount=0;
    rc=SQLNumResultCols(hstmt,(short int FAR*)&siFieldCount);
    iFieldCount=siFieldCount;
    CheckError(m_sSql);

    if(iFieldCount>MAX_COLUMN_COUNT)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetSQL","Application Error Message=Count of field beyond limit(%ld)",MAX_COLUMN_COUNT);
    }
    m_iFieldCount=iFieldCount;
    isEof = true;
	
    //new position
    if(iPreFetchRows>1)
    {
        SQLSetStmtOption(hstmt,SQL_CONCURRENCY,SQL_CONCUR_READ_ONLY);
        SQLSetStmtOption(hstmt,SQL_CURSOR_TYPE,SQL_CURSOR_KEYSET_DRIVEN);
        SQLSetStmtOption(hstmt,SQL_ROWSET_SIZE,(SQLULEN)iPreFetchRows);
    }
	
    for(int iFieldIndex=1;iFieldIndex<=m_iFieldCount;iFieldIndex++)
    {
        iNameLen=0;

        //获取各个选择列信息
        rc=SQLDescribeCol(hstmt,(unsigned short)iFieldIndex,(SQLCHAR*)sFieldName,MAX_COLUMN_NAME,
            (SQLSMALLINT *)&iNameLen,
            (SQLSMALLINT *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].ssiSqlDataType),
            (SQLULEN *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].sulPrecision),
            (SQLSMALLINT *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].ssiScale),
            (SQLSMALLINT *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].ssiNullable));
        CheckError(m_sSql);
		/*
		printf("after SQLDescribeCol Field[%d].ssiSqlDataType = %d  Field[%d].sulPrecision = %d\n",
			iFieldIndex,m_pTOdbcDBFieldArray[iFieldIndex-1].ssiSqlDataType,
			iFieldIndex,m_pTOdbcDBFieldArray[iFieldIndex-1].sulPrecision);
		*/
		if(m_pTOdbcDBFieldArray[iFieldIndex-1].ssiSqlDataType == SQL_BLOB)
		{
			m_pTOdbcDBFieldArray[iFieldIndex-1].sulPrecision = 4000;
		}
		/*
		printf("after SQLDescribeCol Field[%d].ssiSqlDataType = %d  Field[%d].sulPrecision = %d\n",
			iFieldIndex,m_pTOdbcDBFieldArray[iFieldIndex-1].ssiSqlDataType,
			iFieldIndex,m_pTOdbcDBFieldArray[iFieldIndex-1].sulPrecision);
		*/
		m_pTOdbcDBFieldArray[iFieldIndex-1].m_pTOdbcDBQuery=this;
        m_pTOdbcDBFieldArray[iFieldIndex-1].strFieldName=sFieldName;
        m_pTOdbcDBFieldArray[iFieldIndex-1].iIndex=iFieldIndex-1;

        //分配各个选择列缓冲区
        GetFieldBufferInfo(m_pTOdbcDBFieldArray[iFieldIndex-1],iPreFetchRows);
        
        //绑定各个选择列缓冲区
        rc=SQLBindCol(hstmt,(unsigned short)iFieldIndex,
            (SQLSMALLINT)m_pTOdbcDBFieldArray[iFieldIndex-1].ssiDataType,
            (SQLPOINTER)m_pTOdbcDBFieldArray[iFieldIndex-1].sData,
            (SQLLEN)m_pTOdbcDBFieldArray[iFieldIndex-1].slDataLen,
            (SQLLEN*)m_pTOdbcDBFieldArray[iFieldIndex-1].pslValueLen);
        CheckError(m_sSql);

    }//end of for(遍历处理各个选择列)
    
    if(m_iPreFetchRows<iPreFetchRows)
    {
        DELETE_A(aRowsStatus);
        aRowsStatus=new SQLUSMALLINT[iPreFetchRows];
    }

    m_iPreFetchRows=iPreFetchRows;    
    return;
}

void TOdbcDBQuery::Open(int prefetchRows) throw (TOdbcDBException)
{
    m_lAffectRows=0;
    if((m_iQueryState!=QUERY_BEFORE_OPEN)
        &&(m_iQueryState!=QUERY_FETCH_EOF)
        &&(m_iQueryState!=QUERY_FETCH_BOF)
        &&(m_iQueryState!=QUERY_AFTER_OPEN)
        )
    {   
        //判断执行状态
        //应用修改,不可能到达此处
        throw TOdbcDBException("TOdbcDBQuery::Open","Application Error Message=Data set not closed!");
    }

    if (!isEof)
    {
        //不是CLOSE后调用
        rc=SQLFreeStmt(hstmt,SQL_CLOSE);
        CheckError(m_sSql);
        isEof = true;
    }

    if(m_iFieldCount<=0)
    {
        throw TOdbcDBException("TOdbcDBQuery::Open","Application Error Message=SQL is not for SELECT!");
    }

    if(m_iParamCount>0)
    {
    	//存在参数情况下,校验
        for(int iParamIndex=1;iParamIndex<=m_iParamCount;iParamIndex++)
        {
            if(m_pTOdbcDBParamArray[iParamIndex-1].iElementCount<=0)
            {
                throw TOdbcDBException("TOdbcDBQuery::Open","Application Error Message=Param(%d) not be set!",iParamIndex);
            }
        }//end of for(遍历所有参数,校验是否已经赋值)
    }

    //回复设置参数数组元素个数
    iRow=0;    
    //执行SQL
    rc=SQLExecute(hstmt);
    CheckError(m_sSql);

	isEof = false;	
    m_iQueryState=QUERY_AFTER_OPEN;

    return;
}

bool TOdbcDBQuery::Next() throw (TOdbcDBException)
{
    if((m_iQueryState!=QUERY_AFTER_OPEN)
        &&(m_iQueryState!=QUERY_FETCH_BOF))
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBQuery::Next","Application Error Message=Data set not opened!");
    }

	m_iQueryState=QUERY_FETCH_BOF;
	
    m_iRowIndex++;
    if(m_iRowIndex<m_iPreFetchRows)
    {
    }
    else
    {
        //清除各选择列列缓冲区数据
        for(int iFieldIndex=1;iFieldIndex<=m_iFieldCount;iFieldIndex++)
        {
            memset(m_pTOdbcDBFieldArray[iFieldIndex-1].sData,0,(size_t)m_pTOdbcDBFieldArray[iFieldIndex-1].iRowsCount*m_pTOdbcDBFieldArray[iFieldIndex-1].slDataLen);
            memset(m_pTOdbcDBFieldArray[iFieldIndex-1].pslValueLen,0,(size_t)m_pTOdbcDBFieldArray[iFieldIndex-1].iRowsCount*sizeof(SQLLEN));
        }

        m_iRowIndex=0;
        m_iFetchedRows=0;
        if(m_iPreFetchRows<=1)
        {
            rc=SQLFetch(hstmt);
            if((rc==SQL_NO_DATA_FOUND)||(rc==SQL_NO_DATA)) //TimesTen for unix无
            {//未找到数据
                m_iQueryState=QUERY_FETCH_EOF;
				isEof = true;
				return false;
            }
			/*
			printf("Field[0] Name=[%s]  sdwDataValue=[%d] \n",m_pTOdbcDBFieldArray[0].AsName(),m_pTOdbcDBFieldArray[0].sdwDataValue);
			printf("Field[0] Name=[%s]  dDataValue=[%ld] \n",m_pTOdbcDBFieldArray[0].AsName(),m_pTOdbcDBFieldArray[0].dDataValue);
			printf("Field[1] Name=[%s]  sdwDataValue=[%d] \n",m_pTOdbcDBFieldArray[1].AsName(),m_pTOdbcDBFieldArray[1].sdwDataValue);
			printf("Field[1] Name=[%s]  dDataValue=[%ld] \n",m_pTOdbcDBFieldArray[1].AsName(),m_pTOdbcDBFieldArray[1].dDataValue);
			*/
			
            CheckError(m_sSql);
            m_iFetchedRows=1;    
        }
        else
        {
        	rc=SQLFetch(hstmt);
            //rc=SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,m_iPreFetchRows,&m_iFetchedRows,aRowsStatus);
			if((rc==SQL_NO_DATA_FOUND)||(rc==SQL_NO_DATA)) 
            {//未找到数据
                m_iQueryState=QUERY_FETCH_EOF;
				isEof = true;
                return false;
            }
            CheckError(m_sSql);
        }
    }
    m_lAffectRows=m_lAffectRows+m_iFetchedRows;
	isEof = false;
    return true;
}

bool TOdbcDBQuery::Execute(int iExecuteRows)throw (TOdbcDBException)
{
    int iMaxParamElementCount=0;

    m_lAffectRows=0;
    if(m_iFieldCount>0)
    {
        throw TOdbcDBException("TOdbcDBQuery::Execute","Application Error Message=SQL can not Execute!");
    }

    if(m_iQueryState!=QUERY_BEFORE_OPEN)
    {
    	//判断执行状态
        throw TOdbcDBException("TOdbcDBQuery::Execute","Application Error Message=Called with error env!");
    }

	if(m_iParamCount>0)
    {
    	//存在参数情况下,校验
        iMaxParamElementCount=m_pTOdbcDBParamArray[0].iElementCount;
        for(int iParamIndex=1;iParamIndex<=m_iParamCount;iParamIndex++)
        {
            if(m_pTOdbcDBParamArray[iParamIndex-1].iElementCount<=0)
            {
                throw TOdbcDBException("TOdbcDBQuery::Execute","Application Error Message=Param(%d) not be set!",iParamIndex);
            }
            else if(m_pTOdbcDBParamArray[iParamIndex-1].iElementCount<iMaxParamElementCount)
            {
                iMaxParamElementCount=m_pTOdbcDBParamArray[iParamIndex-1].iElementCount;
            }
        }//end of for(遍历所有参数,校验是否已经赋值)

		//printf("-----iExecuteRows [%d]---iMaxParamElementCount[%d]---\n",iExecuteRows,iMaxParamElementCount);
        if((iExecuteRows>0)&&(iExecuteRows<iMaxParamElementCount))
        {
            iMaxParamElementCount=iExecuteRows;
        }

        //设置参数数组元素个数
        iRow=0;
        rc=SQLParamOptions(hstmt,(SQLUINTEGER)iMaxParamElementCount,&iRow);
        CheckError(m_sSql);
    }

    //Execute
    rc=SQLExecute(hstmt);
    CheckError(m_sSql);

    if(rc == SQL_NO_DATA || rc == SQL_NO_DATA_FOUND)
    {
        m_lAffectRows=0; 
    }
    else//只在有值的时候取生效数量
    {
		m_lAffectRows=0;

		SQLLEN       row_count;
		rc=SQLRowCount(hstmt, &row_count);
		m_lAffectRows = row_count;

		CheckError(m_sSql);
		//printf("m_lAffectRows = %d\n",row_count);
	}	
	
    return true;
}

bool TOdbcDBQuery::TransBegin()
{
    m_pTOdbcDBDatabase->TransBegin();
    return true;
}

bool TOdbcDBQuery::Commit()
{
    m_pTOdbcDBDatabase->Commit();
    return true;
}

bool TOdbcDBQuery::Rollback()
{
    m_pTOdbcDBDatabase->Rollback();
    return true;
}

int TOdbcDBQuery::RowsAffected()
{
    return (int)m_lAffectRows;
}

int TOdbcDBQuery::FieldCount()
{
    return m_iFieldCount;
}

TOdbcDBField& TOdbcDBQuery::Field(int iIndex) throw (TOdbcDBException)
{
    if((iIndex<0)
        ||(iIndex>=m_iFieldCount)
        )
    {
        throw TOdbcDBException("TOdbcDBQuery::Field","Application Error Message=Parameter Error!");
    }
    return m_pTOdbcDBFieldArray[iIndex];
}

TOdbcDBField& TOdbcDBQuery::Field(const char *sFieldName) throw (TOdbcDBException)
{
    if((sFieldName==NULL)
        ||(strlen(sFieldName)==0)
        ||(m_iFieldCount<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBQuery::Field","Application Error Message=Parameter Error!");
    }

    for(int iIndex=0;iIndex<m_iFieldCount;iIndex++)
    {
        if(FUNC_STR::StrCmpNoCase(sFieldName,m_pTOdbcDBFieldArray[iIndex].strFieldName.c_str())==0)
        {//名字无大小区分
            return m_pTOdbcDBFieldArray[iIndex];
        }
    }

    throw TOdbcDBException("TOdbcDBQuery::Field","Application Error Message=Field(%s) not exist!",sFieldName);
}

/*begin zmp 841469 */
void TOdbcDBQuery::GetValue(void *pStruct,int* Column)throw (TOdbcDBException)
{
    throw TOdbcDBException("TOdbcDBException::GetValue","Not Support!");
    /*
    char* pStructAddr = (char*)pStruct;
    int i = 0;
    int iFiledCounts = FieldCount();
    char sValue[8192];
    //全表列信息
    for(i = 0; i<iFiledCounts; ++i)
    {
        memset(sValue,0,sizeof(sValue));
        strncpy(sValue,Field(i).AsString(),sizeof(sValue));
        //printf("FIELD[%d] value= [%s] \n",i,sValue);
        //printf("test[%d] columnoffsetof [%d] \n",i,Column[i]);
        //每列对应的filed值拷贝到 pStructAddr+Column[i]
        if(i == iFiledCounts - 1)//最后一列
        {
            //NOTICE:列长度为8，并且值都是数值类型的做转换。最后一列只判断是否是纯数值。
            if(TStrFunc::IsNumStr(sValue))
            {
                long long llValue = TStrFunc::StrToLLong(sValue,true);
                memcpy(pStructAddr+Column[i],(long long*)&llValue ,8);
            }
            else
            {
                memcpy(pStructAddr+Column[i],sValue ,strlen(sValue));
            }
        }
        else
        {
            int iColLen = Column[i+1] - Column[i];
            //NOTICE:列长度为8，并且值都是数值类型的做转换。
            if(iColLen == 8 && TStrFunc::IsNumStr(sValue))
            {
                long long llValue = TStrFunc::StrToLLong(sValue,true);
                memcpy(pStructAddr+Column[i],(long long*)&llValue ,Column[i+1] - Column[i]);
            }
            else
            {
                memcpy(pStructAddr+Column[i],sValue ,Column[i+1] - Column[i]);
            }
        }
    }
    */
}

void TOdbcDBQuery::SetUpdateMDBFlag(bool flag)throw (TOdbcDBException)
{
    TOdbcDBQuery::m_bUpdateMDBFlag=flag;
}
bool TOdbcDBQuery::GetUpdateMDBFlag()throw (TOdbcDBException)
{
    return TOdbcDBQuery::m_bUpdateMDBFlag;
}
/*end zmp 841469 */

int TOdbcDBQuery::ParamCount()
{
    return m_iParamCount;
}

TOdbcDBParam& TOdbcDBQuery::Param(int iIndex)
{
    if((iIndex<0)
        ||(iIndex>=m_iParamCount)
        )
    {
        throw TOdbcDBException("TOdbcDBQuery::Param","Application Error Message=Parameter Error!");
    }
    return m_pTOdbcDBParamArray[iIndex];
}

TOdbcDBParam& TOdbcDBQuery::Param(const char*sParamName)
{
    if((sParamName==NULL)
        ||(strlen(sParamName)==0)
        ||(m_iParamCount<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBQuery::Param","Application Error Message=Parameter Error!");
    }

    return *(ParamByCondition(sParamName));
}

//设置参数值
void TOdbcDBQuery::SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBParam,sParamValue);
    return;
}

void TOdbcDBQuery::SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
    SetParameter(sParamName,(long)iParamValue);
    return;
}

void TOdbcDBQuery::SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBParam,lParamValue);
    return;
}

void TOdbcDBQuery::SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBParam,dParamValue);
    return;
}

void TOdbcDBQuery::SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
#ifdef _WIN32
    //ZMP:467368 
    snprintf(m_sLongLong, sizeof(m_sLongLong), "%I64d",llParamValue);
#else
    snprintf(m_sLongLong, sizeof(m_sLongLong), "%lld",llParamValue);
#endif
	m_sLongLong[sizeof(m_sLongLong)-1] = '\0';
    SetParameter(sParamName,m_sLongLong);
    return;
}

void TOdbcDBQuery::SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBParam,sParamValue,iBufferLen);
    return;
}

void TOdbcDBQuery::SetParameterNULL(const char *sParamName)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
	if (pTOdbcDBParam->ssiDataType != 0)
	{     
		pTOdbcDBParam->llValueLen=SQL_NULL_DATA;
	} 
	else 
	{
        SetParameter(sParamName, (char*)NULL);
	}
    return;
}

void TOdbcDBQuery::SetParameter(int iParamIndex,const char* sParamValue)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,sParamValue);
    return;
}

void TOdbcDBQuery::SetParameter(int iParamIndex,int iParamValue)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,(long)iParamValue);
    return;
}
void TOdbcDBQuery::SetParameter(int iParamIndex,long lParamValue)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,lParamValue);
    return;
}
void TOdbcDBQuery::SetParameter(int iParamIndex,double dParamValue)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,dParamValue);
    return;
}
void TOdbcDBQuery::SetParameter(int iParamIndex,long long llParamValue)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,(double)llParamValue);
    return;
}
void TOdbcDBQuery::SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,sParamValue,iBufferLen);
    return;
}

void TOdbcDBQuery::SetParameterNULL(int iParamIndex)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,(char*)NULL);
    return;
}

//设置数组参数值
void TOdbcDBQuery::SetParamArray(const char *sParamName,char **asParamValue,int iInterval,
                                 int iElementSize,int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if((asParamValue==NULL)
        ||(iElementSize<=0)
        ||(iArraySize<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;
	if (bSetStmtAttr == false){
        SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (void*)iElementCount, 0);
        bSetStmtAttr = true;
    }
	
    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
        )
    {
        delete [](char*)pTOdbcDBParam->pData;
        pTOdbcDBParam->pData = NULL;
        pTOdbcDBParam->bSetValue=false;
    }
    
    if((pTOdbcDBParam->allValueLen!=NULL)
        &&(pTOdbcDBParam->iElementCount<iElementCount)
        )
    {
        delete [] pTOdbcDBParam->allValueLen;
        pTOdbcDBParam->allValueLen = NULL;
        pTOdbcDBParam->bSetValue=false;
    }

    if(pTOdbcDBParam->bSetValue==false)
    {
        if(pTOdbcDBParam->pData==NULL)
        {
            pTOdbcDBParam->pData=new char[iElementSize*iElementCount];
            pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        }
        
        memcpy(pTOdbcDBParam->pData,asParamValue,(size_t)iArraySize);
        pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBParam->llElementSize=iElementSize;
           
        if(pTOdbcDBParam->allValueLen==NULL)
        {
            pTOdbcDBParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        }
    
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=SQL_NTS;
        }
    	/*
        rc=SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
		*/
		rc=SQLBindParameter(hstmt,(unsigned short)pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,SQL_CHAR,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
        CheckError(m_sSql);		
    }
    else
    {
        pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        memcpy(pTOdbcDBParam->pData,asParamValue,(size_t)iArraySize);
        pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBParam->llElementSize=iElementSize;
        memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=SQL_NTS;
        }
    }

    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;
    return;
}

void TOdbcDBQuery::SetParamArray(const char *sParamName,int *aiParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if((aiParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    int iElementSize=sizeof(int);
    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;

	if (bSetStmtAttr == false){
        SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (void*)iElementCount, 0);
        bSetStmtAttr = true;
    }
	
    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
        )
    {
        delete [](char*)pTOdbcDBParam->pData;
        pTOdbcDBParam->pData = NULL;
        pTOdbcDBParam->bSetValue=false;
    }
    
    if((pTOdbcDBParam->allValueLen!=NULL)
        &&(pTOdbcDBParam->iElementCount<iElementCount)
        )
    {
        delete [] pTOdbcDBParam->allValueLen;
        pTOdbcDBParam->allValueLen=NULL;
        pTOdbcDBParam->bSetValue=false;
    }

    if(pTOdbcDBParam->bSetValue==false)
    {
        if(pTOdbcDBParam->pData==NULL)
        {
            pTOdbcDBParam->pData=new int[iElementCount];
            pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        }
        
        pTOdbcDBParam->ssiDataType=SQL_C_SLONG;
        pTOdbcDBParam->llElementSize=iElementSize;
        
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((int*)pTOdbcDBParam->pData)[iIndex]=aiParamValue[iIndex];
        }       
            
        if(pTOdbcDBParam->allValueLen==NULL)
        {
            pTOdbcDBParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        }
    	/*
        rc=SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
		*/
		rc=SQLBindParameter(hstmt, pTOdbcDBParam->iIndex+1, SQL_PARAM_INPUT,
                         SQL_C_SLONG, SQL_INTEGER,0, 0,
                         (int*)pTOdbcDBParam->pData,
                         0,/* For all fixed size C data type, this argument is ignored */
                         NULL);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        pTOdbcDBParam->ssiDataType=SQL_C_SLONG;
        pTOdbcDBParam->llElementSize=iElementSize;
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((int*)pTOdbcDBParam->pData)[iIndex]=aiParamValue[iIndex];
        }       
        memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
    }

    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;    
    return;
}

void TOdbcDBQuery::SetParamArray(const char *sParamName,long *alParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if(sizeof(long)==sizeof(int))
    {
        int *aiParamValue=NULL;

        try
        {
            int iElementCount=iArraySize/iInterval;
            aiParamValue=new int[iElementCount];
            for(int iIndex=0;iIndex<iElementCount;iIndex++)
            {
                aiParamValue[iIndex]=(int)alParamValue[iIndex];
            }
            SetParamArray(sParamName,aiParamValue,(int)sizeof(int),iElementCount*(int)sizeof(int),bOutput);
        }
        catch(TOdbcDBException &e)
        {
            DELETE_A(aiParamValue);
            throw e;
        }

        DELETE_A(aiParamValue);
    }
    else
    {
        long long *allParamValue=NULL;
        try
        {
            int iElementCount=iArraySize/iInterval;
            allParamValue=new long long[iElementCount];
            for(int iIndex=0;iIndex<iElementCount;iIndex++)
            {
                allParamValue[iIndex]=alParamValue[iIndex];
            }
            SetParamArray(sParamName,(long long*)allParamValue,(int)sizeof(long long),iElementCount*(int)(sizeof(long long)),bOutput);
        }
        catch(TOdbcDBException &e)
        {
            DELETE_A(allParamValue);
            throw e;
        }

        DELETE_A(allParamValue);
    }
    return;
}

void TOdbcDBQuery::SetParamArray(const char *sParamName,double *adParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if((adParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    int iElementSize=sizeof(double);
    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;
	if (bSetStmtAttr == false){
        SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (void*)iElementCount, 0);
        bSetStmtAttr = true;
    }
	
    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
      )
    {
        delete [] (char*)pTOdbcDBParam->pData;
        pTOdbcDBParam->pData = NULL;
        pTOdbcDBParam->bSetValue=false;
    }
    
    if((pTOdbcDBParam->allValueLen!=NULL)
        &&(pTOdbcDBParam->iElementCount<iElementCount)
        )
    {
        delete [] pTOdbcDBParam->allValueLen;
        pTOdbcDBParam->allValueLen = NULL;		
        pTOdbcDBParam->bSetValue=false;
    }

    if(pTOdbcDBParam->bSetValue==false)
    {
        if(pTOdbcDBParam->pData==NULL)
        {
            pTOdbcDBParam->pData=new double[iElementCount];
            pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        }

        memcpy(pTOdbcDBParam->pData,adParamValue,(size_t)iArraySize);
        pTOdbcDBParam->ssiDataType=SQL_C_DOUBLE;
        pTOdbcDBParam->llElementSize=iElementSize;
    
        if(pTOdbcDBParam->allValueLen==NULL)
        {
            pTOdbcDBParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        }

		/*
        rc=SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
        */
        rc=SQLBindParameter(hstmt, pTOdbcDBParam->iIndex+1, SQL_PARAM_INPUT,
                         SQL_C_DOUBLE, SQL_FLOAT,(SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
                         (double *)pTOdbcDBParam->pData,
                         0,/* For all fixed size C data type, this argument is ignored */
                         0);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        memcpy(pTOdbcDBParam->pData,adParamValue,(size_t)iArraySize);
        pTOdbcDBParam->ssiDataType=SQL_C_DOUBLE;
        pTOdbcDBParam->llElementSize=iElementSize;
        memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
    }
    
    pTOdbcDBParam->iElementCount=iArraySize/iElementSize;
    pTOdbcDBParam->bSetValue=true;
    return;
}

void TOdbcDBQuery::SetParamArray(const char *sParamName,long long *allParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if((allParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    
    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;
    int iElementSize=SIZE_LONG_LONG;

	if (bSetStmtAttr == false)
	{
        SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (void*)iElementCount, 0);
        bSetStmtAttr = true;
    }
	
    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
      )
    {
        delete [](char*)pTOdbcDBParam->pData;
        pTOdbcDBParam->pData = NULL;
        pTOdbcDBParam->bSetValue=false;
    }
    
     if((pTOdbcDBParam->allValueLen!=NULL)
        &&(pTOdbcDBParam->iElementCount<iElementCount)
        )
    {
    	delete [] pTOdbcDBParam->allValueLen;
        pTOdbcDBParam->allValueLen = NULL;
        pTOdbcDBParam->bSetValue=false;
    }
    
    if(pTOdbcDBParam->bSetValue==false)
    {
        if(pTOdbcDBParam->pData==NULL)
        {
            pTOdbcDBParam->pData=new char[iElementCount*SIZE_LONG_LONG];
            pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        }
       
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            #ifdef _WIN32
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%I64d",allParamValue[iIndex]);                
            #else
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%lld",allParamValue[iIndex]);
            #endif
			m_sLongLong[sizeof(m_sLongLong)-1] = '\0';
            memcpy((char*)pTOdbcDBParam->pData+SIZE_LONG_LONG*iIndex,m_sLongLong,SIZE_LONG_LONG);
        }
        
        pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBParam->llElementSize=iElementSize;
       
        if(pTOdbcDBParam->allValueLen==NULL)
        {
            pTOdbcDBParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        }
    
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=SQL_NTS;
        }
    	/*
        rc=SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
        */
        rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
            SQL_C_CHAR,SQL_VARCHAR,SIZE_LONG_LONG,0,
            (SQLPOINTER*)pTOdbcDBParam->pData,SIZE_LONG_LONG,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
                
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            #ifdef _WIN32
                snprintf(m_sLongLong, sizeof(m_sLongLong),"%I64d",allParamValue[iIndex]);                
            #else
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%lld",allParamValue[iIndex]);
            #endif
			m_sLongLong[sizeof(m_sLongLong)-1] = '\0';
            memcpy((char*)pTOdbcDBParam->pData+SIZE_LONG_LONG*iIndex,m_sLongLong,sizeof(m_sLongLong));
        }
        
        pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBParam->llElementSize=iElementSize;
        
        memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=SQL_NTS;
        }
    }

    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;
    return;
}
/*
void TOdbcDBQuery::SetBlobParamArray(const char *sParamName,char *sParamValue,
                                     int iBufferLen,int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if((sParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBQuery::SetBlobParamArray","Application Error Message=Parameter Error!");
    }

    int iElementSize=iBufferLen;
    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iBufferLen;

    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
      )
    {
        if(pTOdbcDBParam->pData != NULL)
        {
            delete [](char*)pTOdbcDBParam->pData;
            pTOdbcDBParam->pData = NULL;
        }
        pTOdbcDBParam->bSetValue=false;
    }
    
    if((pTOdbcDBParam->allValueLen!=NULL)
        &&(pTOdbcDBParam->iElementCount<iElementCount)
        )
    {
    	delete [] pTOdbcDBParam->allValueLen;
        pTOdbcDBParam->allValueLen = NULL;
        pTOdbcDBParam->bSetValue=false;
    }
    
    if(pTOdbcDBParam->bSetValue==false)
    {
        if(pTOdbcDBParam->pData==NULL)
        {
            pTOdbcDBParam->pData=new char[iArraySize];
            pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        }
        
        memcpy(pTOdbcDBParam->pData,sParamValue,(size_t)(iElementCount*iElementSize));

		printf("==== pData = %s  ====\n",pTOdbcDBParam->pData);
		printf("==== iElementSize = %d  ====\n",iElementSize);
		printf("==== iArraySize = %d  ====\n",iArraySize);
		printf("==== iElementCount = %d  ====\n",iElementCount);
		
        pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBParam->llElementSize=iElementSize;
        
        if(pTOdbcDBParam->allValueLen==NULL)
        {
            pTOdbcDBParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        }
    
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=iElementSize;
        }
    
        rc=SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
        CheckError(m_sSql);
		printf("===========after BindParam=========\n");
			
		printf("==== ssiSqlDataType = %d  ====\n",pTOdbcDBParam->ssiSqlDataType);
		printf("==== sulPrecision = %d  ====\n",pTOdbcDBParam->sulPrecision);
		printf("==== ssiScale = %d  ====\n",pTOdbcDBParam->ssiScale);
		printf("==== pData = %s  ====\n",pTOdbcDBParam->pData);
		printf("==== llElementSize = %d  ====\n",pTOdbcDBParam->llElementSize);
    }
    else
    {
        pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        memcpy(pTOdbcDBParam->pData,sParamValue,(size_t)(iElementSize*iElementCount));
        pTOdbcDBParam->ssiDataType=SQL_C_BINARY;
        pTOdbcDBParam->llElementSize=iElementSize;
        
        memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=iElementSize;
        }
    }

    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;
    return;
}
*/
void TOdbcDBQuery::SetBlobParamArray(const char *sParamName,char *sParamValue,
									 int iBufferLen,int iArraySize,bool bOutput)throw (TOdbcDBException)
{
	if((sParamValue==NULL)
		||(iArraySize<=0)
		)
	{
		throw TOdbcDBException("TOdbcDBQuery::SetBlobParamArray","Application Error Message=Parameter Error!");
	}

	int iElementSize=iBufferLen;
	TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
	int iElementCount=iArraySize/iBufferLen;

	if((pTOdbcDBParam->pData!=NULL)
		&&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
	  )
	{
		delete [](char*)pTOdbcDBParam->pData;
		pTOdbcDBParam->pData = NULL;
		pTOdbcDBParam->bSetValue=false;
	}
	
	if((pTOdbcDBParam->allValueLen!=NULL)
		&&(pTOdbcDBParam->iElementCount<iElementCount)
		)
	{
		delete [](char*)pTOdbcDBParam->allValueLen;
		pTOdbcDBParam->allValueLen = NULL;
		pTOdbcDBParam->bSetValue=false;
	}
	
	if(pTOdbcDBParam->bSetValue==false)
	{
		if(pTOdbcDBParam->pData==NULL)
		{
			pTOdbcDBParam->pData=new char[iArraySize];
			pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
		}
		
		memcpy(pTOdbcDBParam->pData,sParamValue,(size_t)(iElementCount*iElementSize));
		//pTOdbcDBParam->ssiDataType=SQL_C_BINARY;
		pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
		pTOdbcDBParam->llElementSize=iElementSize;
		
		if(pTOdbcDBParam->allValueLen==NULL)
		{
			pTOdbcDBParam->allValueLen=new SQLLEN[iElementCount];
			memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
		}
	
		for(int iIndex=0;iIndex<iElementCount;iIndex++)
		{
			((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=iElementSize;
		}
	
		rc=SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBParam->iIndex+1),SQL_PARAM_INPUT,
			(SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
			(SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
			(SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
			(SQLLEN*)pTOdbcDBParam->allValueLen);
		CheckError(m_sSql);
	}
	else
	{
		pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
		memcpy(pTOdbcDBParam->pData,sParamValue,(size_t)(iElementSize*iElementCount));
		//pTOdbcDBParam->ssiDataType=SQL_C_BINARY;
		pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
		pTOdbcDBParam->llElementSize=iElementSize;
		
		memset(pTOdbcDBParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
		
		for(int iIndex=0;iIndex<iElementCount;iIndex++)
		{
			((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=iElementSize;
		}
	}

	pTOdbcDBParam->iElementCount=iElementCount;
	pTOdbcDBParam->bSetValue=true;
	return;
}



void TOdbcDBQuery::setParam(int iParamIndex, SQLSMALLINT value)
{
    m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_SMALLINT;
    m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=5;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
    return ;
}

void TOdbcDBQuery::setParam(int iParamIndex, unsigned char value)
{
    m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_CHAR;
    m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=2;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
    return;
}

void TOdbcDBQuery::setParam(int iParamIndex, int value)
{
    setParam_Int(iParamIndex, (SQLINTEGER)value);
}

void TOdbcDBQuery::setParam_Int(int iParamIndex, SQLINTEGER value)
{
    m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_INTEGER;
    m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=10;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
    return;
}

void TOdbcDBQuery::setParam(int iParamIndex, long value)
{ 
    if(sizeof(SQLBIGINT)==sizeof(long))
        setParam_Long(iParamIndex, (SQLBIGINT)value);
    else
        setParam_Long(iParamIndex, (SQLINTEGER)value);        
    return;
}

void TOdbcDBQuery::setParam_Long(int iParamIndex, SQLBIGINT value)
{
    m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_BIGINT;
    m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=19;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
    return;
}

void TOdbcDBQuery::setParam(int iParamIndex,long long value)
{ 
    setParam_Long(iParamIndex, (SQLBIGINT)value);
    return;
}

void TOdbcDBQuery::setParam(int iParamIndex, float value)
{
    m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_FLOAT;
    m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=7;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
    return;
}

void TOdbcDBQuery::setParam(int iParamIndex, double value)
{
    m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_FLOAT;
    m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=15;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
    return;
}

void TOdbcDBQuery::setParam(int iParamIndex, SQLCHAR* valueP)
{
    size_t paramLen = 0;
    if(valueP == NULL)
      paramLen = 0;
    else
      paramLen = strlen((const char *)valueP);

	m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_VARCHAR;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
	
	if (m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision >= paramLen) //测试一下边界
	{
		return;
	}
    if(paramLen<=CHARLEN_MIN)
    {
        m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=CHARLEN_MIN;
    }
    else if(paramLen<=CHARLEN_MIDDLE)
    {
        m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=CHARLEN_MIDDLE;
    }
    else if(paramLen<=CHARLEN_MAX)
    {
        m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=CHARLEN_MAX;
    }
    else if(paramLen<=CHARLEN_XXX)
    {
        m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=CHARLEN_XXX;
    }
    else
    {	
		throw TOdbcDBException("DATA OUT BUFFER!","Application Error DATA-paramLen=[%zd] is OUT MAXLEN=[%ld]",paramLen,CHARLEN_XXX);
    }

    if (m_pTOdbcDBParamArray[iParamIndex-1].pData)
    {
        delete [] (char*)m_pTOdbcDBParamArray[iParamIndex-1].pData;
        m_pTOdbcDBParamArray[iParamIndex-1].pData=NULL;
    }
	
    m_pTOdbcDBParamArray[iParamIndex-1].bSetValue=false;

    return;
}

void TOdbcDBQuery::setParam(int iParamIndex,const char* valueP)
{ 
	setParam(iParamIndex,const_cast<char*>(valueP));
 	return;
}

void TOdbcDBQuery::setParam(int iParamIndex, const void* valueP, int byteLength)
{
    int paramLen = byteLength;

	m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_BINARY;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
	
	if (m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision >= paramLen) //测试一下边界
	{
		return;
	}
    if(paramLen<=BLOBLEN_MIN)
    {
        m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=BLOBLEN_MIN;
    }
    else if(paramLen<=BLOBLEN_MIDDLE)
    {
        m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=BLOBLEN_MIDDLE;
    }
    else if(paramLen<=BLOBLEN_MAX)
    {
        m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=BLOBLEN_MAX;
    }
    else if(paramLen<=BLOBLEN_XXX)
    {
        m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=BLOBLEN_XXX;
    }
    else
    {
        throw TOdbcDBException("DATA OUT BUFFER!","Application Error DATA-paramLen=[%d] is OUT MAXLEN=[%d]",paramLen,BLOBLEN_XXX);
    }

    if (m_pTOdbcDBParamArray[iParamIndex-1].pData)
    {
        delete [] (char*)m_pTOdbcDBParamArray[iParamIndex-1].pData;
        m_pTOdbcDBParamArray[iParamIndex-1].pData=NULL;
    }
    m_pTOdbcDBParamArray[iParamIndex-1].bSetValue=false;

    return;
}

void TOdbcDBQuery::setParam(int iParamIndex, TIMESTAMP_STRUCT &ts)
{

    m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=15;      
    m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_TYPE_TIMESTAMP;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
    return;
}

void TOdbcDBQuery::setParam(int iParamIndex, DATE_STRUCT &dm)
{

    m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=15;      
    m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_TYPE_DATE;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
    return;
}

void TOdbcDBQuery::setParam(int iParamIndex, TIME_STRUCT &dm)
{

    m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision=15;      
	m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType=SQL_TYPE_TIME;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiScale=0;
    m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable=true;
    return;
}

//private函数
void TOdbcDBQuery::CheckError(const char* sSql) throw (TOdbcDBException)
{
    static ostringstream ostrErr;
    int iRet=0;
    ostrErr.str("");

    if((rc!=SQL_SUCCESS)
        &&(rc!=SQL_SUCCESS_WITH_INFO)
		&&(rc!=SQL_NO_DATA)
		&&(rc!=SQL_NO_DATA_FOUND)
        )
    {
        iRet=DBError(rc,henv,hdbc,hstmt,ostrErr);
    }

    if(iRet!=0)
    {
        throw TOdbcDBException(sSql,"Application Error Message=%s",ostrErr.str().c_str());
    }
}

void TOdbcDBQuery::ParseSql(char* sSqlStatement)
{
    int iLen=0;
    int iIndex=0;
    int iParamBegin=0;
    int iParamEnd=0;
    int iQuoteFlag=0;                                       //为0表示无引号,为1表示单引号开始
    char    cCur=0;
    string strParamName="";

    iLen=(int)strlen(sSqlStatement);
    m_iParamCount=0;
    while(iIndex<=iLen)
    {
        cCur=sSqlStatement[iIndex];
        if(cCur=='\'')
        {
            iQuoteFlag=ABS(iQuoteFlag-1);
            iIndex++;
            continue;
        }

        if((cCur==':')&&(iQuoteFlag==0))
        {
            iIndex++;

            /*参数开始,指定参数格式为:字母,数字,下划线*/
            iParamBegin=iIndex;
            iParamEnd=iParamBegin-1;
            while(iIndex<=iLen)
            {
                cCur=sSqlStatement[iIndex];
                if((isalnum(cCur)!=0)||(cCur=='_'))
                {//参数名包含字母,数字,下划线
                    iParamEnd=iIndex;
                    iIndex++;
                }
                else
                {//参数名结束
                    break;
                }
            }//end of while(循环内找参数名)

            if(iParamEnd<iParamBegin)
            {//参数名不正确
                throw TOdbcDBException("TOdbcDBQuery::ParseSql","Application Error Message=Param name error");
            }
            else
            {
                if(m_iParamCount==MAX_COLUMN_COUNT)
                {
                    throw TOdbcDBException("TOdbcDBQuery::ParseSql","Application Error Message=Count of prams beyond limit(%ld)",MAX_COLUMN_COUNT);
                }

                for(int iNameIndex=iParamBegin;iNameIndex<=iParamEnd;iNameIndex++)
                {
                    strParamName=strParamName+sSqlStatement[iNameIndex];
                }

                iParamBegin--;                              //处理:字符
                while(iParamBegin<iParamEnd)
                {//替换:参数名为?
                    sSqlStatement[iParamBegin]=' ';
                    iParamBegin++;
                }
                sSqlStatement[iParamEnd]='?';


                //设置参数名称
                m_pTOdbcDBParamArray[m_iParamCount].strParamName=strParamName;
                m_pTOdbcDBParamArray[m_iParamCount].iIndex=m_iParamCount;
                m_pTOdbcDBParamArray[m_iParamCount].m_pTOdbcDBQuery=this;
                m_iParamCount++;

                iParamBegin=0;
                iParamEnd=0;
                strParamName="";

                //抵消参数名后一个字符
                iIndex--;
            }
            continue;
        }

        iIndex++;
    }//end of while(iIndex<=iLen)

    return;
}

void TOdbcDBQuery::GetFieldBufferInfo(TOdbcDBField &oTOdbcDBField,int &iRowsCount)
{
    void *pBuffer=NULL;
    SQLSMALLINT ssiSqlDataType=oTOdbcDBField.ssiSqlDataType;
    SQLULEN sulPrecision=oTOdbcDBField.sulPrecision;
    //SQLSMALLINT ssiScale=oTOdbcDBField.ssiScale;
    SQLSMALLINT ssiDataType=0;
    SQLLEN slDataLen=0;

    if(iRowsCount<=1)
    {
        iRowsCount=1;
    }

    if(oTOdbcDBField.sData!=NULL)
    {
        bool bFlag = ( (oTOdbcDBField.ssiDataType==SQL_C_CHAR)
            ||(oTOdbcDBField.ssiDataType==SQL_C_BINARY)
            ||(oTOdbcDBField.ssiDataType==SQL_C_TYPE_TIMESTAMP)
            ||(oTOdbcDBField.iMaxRowsCount>1) ); 
        if( bFlag )
        {
            delete oTOdbcDBField.sData;
        }
        oTOdbcDBField.sData=NULL;
        oTOdbcDBField.iRowsCount=0;
        if(oTOdbcDBField.pslValueLen!=NULL)
        {
            DELETE_A(oTOdbcDBField.pslValueLen);//ZMP_591266
        }
    }
    
    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:	
        {
            slDataLen=(SQLLEN)(sulPrecision+1);
            if(iRowsCount<=1)
            {
                pBuffer=new char[sulPrecision+1];
            }
            else
            {
                pBuffer=new char[((int)sulPrecision+1)*iRowsCount];
            }
            ssiDataType=SQL_C_CHAR;
            break;
        }
    case SQL_INTEGER:
        {
            slDataLen=sizeof(SQLINTEGER);
            if(iRowsCount<=1)
            {
                pBuffer=&(oTOdbcDBField.sdwDataValue);
            }
            else
            {   
                pBuffer=(SQLINTEGER*)new SQLINTEGER[iRowsCount];
            }
            ssiDataType=SQL_C_LONG;
            break;
        }
	case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_BIGINT:
        {
            slDataLen=sizeof(SQLBIGINT);
            if(iRowsCount<=1)
            {
                pBuffer=&(oTOdbcDBField.dDataValue);

            }
            else
            {
                pBuffer=(SQLBIGINT*)new SQLBIGINT[iRowsCount];
            }
            ssiDataType=SQL_C_SBIGINT;
            break;
        }
    case SQL_SMALLINT:
        {
            slDataLen=sizeof(short int);
            if(iRowsCount<=1)
            {
                pBuffer=&(oTOdbcDBField.swDataValue);
            }
            else
            {   
                pBuffer=(short int*)new short int[iRowsCount];
            }
            ssiDataType=SQL_C_SHORT;
            break;
        }
    case SQL_DECIMAL:
    case SQL_NUMERIC:
		{       
            slDataLen=sizeof(double); 
            if(iRowsCount<=1)
            { 
                pBuffer=&(oTOdbcDBField.dDataValue);
            }
            else
            {
                pBuffer=(double*)new double[iRowsCount];
            }
            ssiDataType=SQL_C_DOUBLE;
            break;
        }    
    case SQL_TIMESTAMP:
    case SQL_TIME:
        {
            slDataLen=SIZE_TIMESTAMP;
            if(iRowsCount<=1)
            {
                pBuffer=new char[SIZE_TIMESTAMP];
            }
            else
            {
                pBuffer=new char[(SIZE_TIMESTAMP)*iRowsCount];
            }
            
            ssiDataType=SQL_C_CHAR;
            break;
        }
	case SQL_TYPE_TIMESTAMP:
    case SQL_DATETIME:
        {
            slDataLen=sizeof(SQL_TIMESTAMP_STRUCT);
            if(iRowsCount<=1)
            {
                pBuffer=new char[slDataLen];
            }
            else
            {
                pBuffer=new char[(slDataLen)*iRowsCount];
            }
            
            ssiDataType=SQL_C_TYPE_TIMESTAMP;
            break;
        }
	case SQL_BLOB:
    case SQL_BINARY:	
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
	case SQL_BYTES:
        {
            slDataLen=(SQLLEN)(sulPrecision+1);
            if(iRowsCount<=1)
            {
                pBuffer=new char[sulPrecision+1];
            }
            else
            {
                pBuffer=new char[((int)sulPrecision+1)*iRowsCount];
            }
            
            ssiDataType=SQL_C_BINARY;
            break;
        }
    default:
        {
            throw TOdbcDBException("TOdbcDBQuery::GetFieldBufferInfo","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
        }
    }//end of switch(ssiSqlDataType)

    oTOdbcDBField.pslValueLen=(SQLLEN*)new SQLLEN[iRowsCount];
    oTOdbcDBField.iRowsCount=iRowsCount;
    oTOdbcDBField.iMaxRowsCount=iRowsCount;
    oTOdbcDBField.sData=(char*)pBuffer;
    oTOdbcDBField.ssiDataType=ssiDataType;
    oTOdbcDBField.slDataLen=slDataLen;

    return;
}
//默认方式iValueFlag=1,iIndex=0,获取同sParamName的第一个未赋值的参数实例指针
TOdbcDBParam* TOdbcDBQuery::ParamByCondition(const char *sParamName,int iValueFlag,int iIndex)
{
    int iParamIndex=0;
    int iCount=-1;

    if((sParamName==NULL)
        ||(strlen(sParamName)==0)
        ||(iIndex<0)
        ||(iIndex>=m_iParamCount)
        ||(m_iParamCount<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBQuery::ParamByCondition","Application Error Message=Parameter Error!");
    }

    for(iParamIndex=0;iParamIndex<m_iParamCount;iParamIndex++)
    {
        if(FUNC_STR::StrCmpNoCase(sParamName,m_pTOdbcDBParamArray[iParamIndex].strParamName.c_str())==0)
        {//名字无大小区分
            switch(iValueFlag)
            {
            case 0:
                {
                    if(m_pTOdbcDBParamArray[iParamIndex].iElementCount>0)
                    {//已赋值
                        iCount++; 
                    }
                    break;

                }
            case 1:
                {
                    if(m_pTOdbcDBParamArray[iParamIndex].iElementCount<=0)
                    {//未赋值
                        iCount++;
                    }
                    break;
                }
            case 2:
            default:
                {//不考虑赋值
                    iCount++;
                    break;
                }
            }//end of switch(iValueFlag);

            if(iCount==iIndex)
            {//指定条件的第iIndex参数实例
                return &(m_pTOdbcDBParamArray[iParamIndex]);
            }
        }
    }

    throw TOdbcDBException("TOdbcDBQuery::ParamByCondition","Application Error Message=Param(%s) not exist!",sParamName);

    return NULL;
}

//设置参数值
void TOdbcDBQuery::SetParameter(TOdbcDBParam *pTOdbcDBParam,const char* sParamValue) throw (TOdbcDBException)
{
    int iElementCount=1;
    size_t iLenParamValue = 0;//ZMP:477242
    if (sParamValue!=NULL)
    {
        iLenParamValue = strlen(sParamValue);
    }
    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<=(SQLLEN)iLenParamValue)
        )
    {
        delete [](char*)pTOdbcDBParam->pData;
		pTOdbcDBParam->pData = NULL;

        pTOdbcDBParam->slDataLen=0;
        pTOdbcDBParam->bSetValue=false;
    }

    if(pTOdbcDBParam->bSetValue==false)
    {
        pTOdbcDBParam->llValueLen=0;
        if(sParamValue!=NULL)
        {
            if(pTOdbcDBParam->pData==NULL)
            {
                pTOdbcDBParam->pData=new char[strlen(sParamValue)+1];
                pTOdbcDBParam->slDataLen=(SQLLEN)(strlen(sParamValue)+1);
            }
            memset(pTOdbcDBParam->pData,0,strlen(sParamValue)+1);
            memcpy(pTOdbcDBParam->pData,sParamValue,strlen(sParamValue));
            pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBParam->llValueLen=SQL_NTS;
        }
        else
        {//该分支不只处理字符串类型
            if(pTOdbcDBParam->pData==NULL)
            {
                pTOdbcDBParam->pData=new char[sizeof(double)];//这些类型中，sizeof(double)最大
                pTOdbcDBParam->slDataLen=1;
            }
            ((char*)pTOdbcDBParam->pData)[0]='\0';
            pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBParam->llValueLen=SQL_NULL_DATA;
        }
    
        rc=SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->slDataLen,
            (SQLLEN*)&(pTOdbcDBParam->llValueLen));
        CheckError(m_sSql);
    }
    else
    {
        if(sParamValue!=NULL)
        {
            memset(pTOdbcDBParam->pData,0,strlen(sParamValue)+1);
            memcpy(pTOdbcDBParam->pData,sParamValue,strlen(sParamValue));
            pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBParam->llValueLen=SQL_NTS;
        }
        else
        {//这个分支，处理的不一定是字符串类型
            ((char*)pTOdbcDBParam->pData)[0]='\0';
            pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBParam->llValueLen=SQL_NULL_DATA;
        }
    }

    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;
    
    return;
}


void TOdbcDBQuery::SetParameter(TOdbcDBParam *pTOdbcDBParam,int iParamValue)throw (TOdbcDBException)
{
	int iElementCount=1;
	setParam(pTOdbcDBParam->iIndex+1,iParamValue);
	if(pTOdbcDBParam->bSetValue==false)
	{
		pTOdbcDBParam->iData=iParamValue;
		pTOdbcDBParam->slDataLen=sizeof(iParamValue);
		pTOdbcDBParam->ssiDataType=SQL_C_SLONG;
		pTOdbcDBParam->llValueLen=0;

		rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT_OUTPUT,
			(SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
			(SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
			(void *)&(pTOdbcDBParam->iData),(SQLLEN)pTOdbcDBParam->slDataLen,
			(SQLLEN*)&(pTOdbcDBParam->llValueLen));
		CheckError(m_sSql);
	}
	else
	{
		pTOdbcDBParam->iData=iParamValue;
	}
	
	pTOdbcDBParam->iElementCount=iElementCount;
	pTOdbcDBParam->bSetValue=true;
	
	return;
}


void TOdbcDBQuery::SetParameter(TOdbcDBParam *pTOdbcDBParam,long lParamValue)throw (TOdbcDBException)
{
	/*
	double dParamValue=lParamValue;
	SetParameter(pTOdbcDBParam,dParamValue);
	*/
	double dParamValue = 0;
	memcpy(&dParamValue,&lParamValue,sizeof(long));
	
	int iElementCount=1;
	setParam(pTOdbcDBParam->iIndex+1,lParamValue);
	if(pTOdbcDBParam->bSetValue==false || pTOdbcDBParam->ssiDataType != SQL_C_SBIGINT)
	{
		pTOdbcDBParam->dData=dParamValue;
		pTOdbcDBParam->slDataLen=sizeof(long);
		pTOdbcDBParam->ssiDataType=SQL_C_SBIGINT;
		pTOdbcDBParam->llValueLen=0;
		rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT_OUTPUT,
			(SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
			(SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
			(SQLPOINTER*)&(pTOdbcDBParam->dData),(SQLLEN)pTOdbcDBParam->slDataLen,
			(SQLLEN*)&(pTOdbcDBParam->llValueLen));    
		CheckError(m_sSql);
	}
	else
	{
		pTOdbcDBParam->dData=dParamValue;
		pTOdbcDBParam->llValueLen=0;
	}
	
	pTOdbcDBParam->iElementCount=iElementCount;
	pTOdbcDBParam->bSetValue=true;

	return;
}


void TOdbcDBQuery::SetParameter(TOdbcDBParam *pTOdbcDBParam,double dParamValue)throw (TOdbcDBException)
{
	int iElementCount=1;
	setParam(pTOdbcDBParam->iIndex+1,dParamValue);	
	if(pTOdbcDBParam->bSetValue==false || pTOdbcDBParam->ssiDataType != SQL_C_DOUBLE)
	{
		pTOdbcDBParam->dData=dParamValue;
		pTOdbcDBParam->slDataLen=sizeof(double);
		pTOdbcDBParam->ssiDataType=SQL_C_DOUBLE;
		pTOdbcDBParam->llValueLen=0;
		rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT_OUTPUT,
			(SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
			(SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
			(SQLPOINTER*)&(pTOdbcDBParam->dData),(SQLLEN)pTOdbcDBParam->slDataLen,
			(SQLLEN*)&(pTOdbcDBParam->llValueLen));    
		CheckError(m_sSql);

	}
	else
	{
		pTOdbcDBParam->dData=dParamValue;
		//zl add for null
		pTOdbcDBParam->llValueLen=0;
	}
	
	pTOdbcDBParam->iElementCount=iElementCount;
	pTOdbcDBParam->bSetValue=true;
	
	return;
}


void TOdbcDBQuery::SetParameter(TOdbcDBParam *pTOdbcDBParam,long long llParamValue)throw (TOdbcDBException)
{
    SetParameter(pTOdbcDBParam,(double)llParamValue);
    return;
}

void TOdbcDBQuery::SetParameter(TOdbcDBParam *pTOdbcDBParam,const char* sParamValue,int iBufferLen)throw (TOdbcDBException)
{
    int iElementCount=1;

    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<(SQLLEN)iBufferLen)
        )
    {
        delete [] (char*)pTOdbcDBParam->pData;
        pTOdbcDBParam->pData=NULL;
        pTOdbcDBParam->slDataLen=0;
        pTOdbcDBParam->bSetValue=false;
    }

    if(pTOdbcDBParam->bSetValue==false)
    {
        pTOdbcDBParam->llValueLen=0;
        if((sParamValue!=NULL)&&(iBufferLen>0))
        {
            if(pTOdbcDBParam->pData==NULL)
            {
                pTOdbcDBParam->pData=new char[iBufferLen];
                pTOdbcDBParam->slDataLen=iBufferLen;
            }
            memset(pTOdbcDBParam->pData,0,(size_t)iBufferLen);
            memcpy(pTOdbcDBParam->pData,sParamValue,(size_t)iBufferLen);
            pTOdbcDBParam->ssiDataType=SQL_C_BINARY;
            pTOdbcDBParam->llValueLen=iBufferLen;
        }
        else
        {
            if(pTOdbcDBParam->pData==NULL)
            {
                pTOdbcDBParam->pData=new char[1];
                pTOdbcDBParam->slDataLen=1;
            }
            ((char*)pTOdbcDBParam->pData)[0]='\0';
            pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBParam->llValueLen=0;
        }
    
        rc=SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->slDataLen,
            (SQLLEN*)&(pTOdbcDBParam->llValueLen));
        CheckError(m_sSql);
    }
    else
    {
        if((sParamValue!=NULL)&&(iBufferLen>0))
        {
            memset(pTOdbcDBParam->pData,0,(size_t)iBufferLen);
            memcpy(pTOdbcDBParam->pData,sParamValue,(size_t)iBufferLen);
            pTOdbcDBParam->ssiDataType=SQL_C_BINARY;
            pTOdbcDBParam->llValueLen=iBufferLen;
        }
        else
        {
            ((char*)pTOdbcDBParam->pData)[0]='\0';
            pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBParam->llValueLen=0;
        }
    }
    
    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;
    
    return;
}


void TOdbcDBQuery::SetParameterNULL(TOdbcDBParam *pTOdbcDBParam) throw (TOdbcDBException)
{
    SetParameter(pTOdbcDBParam,(char*)NULL);
    return;
}

long long TOdbcDBQuery::GetSequenceByName(const char * sName)throw (TOdbcDBException)
{
    throw TOdbcDBException("TOdbcDBQuery::TOdbcDBQuery"," TOdbcDBQuery doesn't support GetSequenceByName() now. ");
}


void TOdbcDBQuery::SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    char sParamValue[40] = {0};
    struct tm * tm_Cur;
    tm_Cur = localtime(&tParamValue);
    //ZMP:467368 
    snprintf(sParamValue, sizeof(sParamValue), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
    sParamValue[sizeof(sParamValue)-1] = '\0';
    SetParameter(pTOdbcDBParam,sParamValue);
    return;
}

void TOdbcDBQuery::SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException)
{
    TOdbcDBParam *pTOdbcDBParam=&Param(iParamIndex);
    char sParamValue[40] = {0};
    struct tm * tm_Cur;
    tm_Cur = localtime(&tParamValue);
    //ZMP:467368 
    snprintf(sParamValue, sizeof(sParamValue), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
    sParamValue[sizeof(sParamValue)-1] = '\0';
    SetParameter(pTOdbcDBParam,sParamValue);
    return;
}
