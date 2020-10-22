//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#include "TSolDBQuery.h"
#include "Common.h"
#include "TStrFunc.h"
#include "debug_new.h"

//ZMP:467366   
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

SQLROWSETSIZE   iRow=0;

int DBError(RETCODE rc,SQLHENV henv,SQLHDBC hdbc,SQLHSTMT hstmt,ostringstream &ostrOut)
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
        char    sql_state[SQL_MAX_MESSAGE_LENGTH];
        SDWORD  native_error;
        char    err_msg[SQL_MAX_MESSAGE_LENGTH];
        SWORD   msg_len;

        RETCODE rc_err=SQLError(henv,hdbc,hstmt,(SQLCHAR*)sql_state,&native_error,
            (SQLCHAR*)err_msg,(SWORD)sizeof(err_msg),&msg_len);

        ostrOut<<"SQL_STATE   ="<<sql_state<<endl
            <<"NATIVE_ERROR="<<native_error<<endl
            <<"ERR_MSG     ="<<err_msg<<endl
            <<"MEG_LEN     ="<<msg_len<<endl;
    }

    return -1;

}

/*类TOdbcDBException定义部分*/
TSolDBException::TSolDBException(const char *sSql, const char* sFormat, ...)
{
    va_list args;

    memset(m_sErrMsg,0,sizeof(m_sErrMsg));
    //memset(m_sErrSql,0,sizeof(m_sErrSql)); //基类已做
    
    m_lErrCode=0;
    va_start(args,sFormat); 
    //ZMP:467366 
    vsnprintf(m_sErrMsg, sizeof(m_sErrMsg), sFormat,args);
    m_sErrMsg[sizeof(m_sErrMsg)-1] = '\0';
    va_end(args);
    m_sErrMsg[sizeof(m_sErrMsg)-1]='\0';    

    strncpy(m_sErrSql,sSql,MAX_SQLSTMT_LENGTH);
    m_sErrSql[MAX_SQLSTMT_LENGTH]='\0';

}

/*类TOdbcDBDatabase定义部分*/
TSolDBDatabase::TSolDBDatabase()
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
    CheckError("TSolDBDatabase::SQLAllocEnv");

    m_pTOdbcDBQuery=NULL;
}

TSolDBDatabase::~TSolDBDatabase()
{
    //int iRet=0;//ZMP:305593,变量未用到

    try   //析构时,如果有异常也不处理.
    {
        if(m_pTOdbcDBQuery!=NULL)
        {
            m_pTOdbcDBQuery->Close();
            DELETE(m_pTOdbcDBQuery);
        }

        //释放连接
        Disconnect();

        if(henv!=NULL)
        {
            //FREE ENV
            rc=SQLFreeEnv(henv);
            CheckError("TSolDBDatabase::SQLFreeEnv");
            henv=NULL;
        }
    }
    catch(TSolDBException &oe)
    {
        cout<<"TSolDBDatabase::~TSolDBDatabase() TSolDBException:" << oe.GetErrMsg() << endl;
    }
    catch(...)
    {
        //throw TSolDBException("","TSolDBDatabase::~TSolDBDatabase() catch ... Exception.\n");
        cout<<"TSolDBDatabase::~TSolDBDatabase() catch ... Exception"<<endl;
    }
    
}

void TSolDBDatabase::SetLogin(const char *sUser,const char *sPassword,const char *sServerName)throw (TSolDBException)
{
    m_strUser=sUser;
    m_strPassword=sPassword;
    m_strServerName=sServerName;

    return ;   
}

bool TSolDBDatabase::Connect(bool bIsAutoCommit) throw (TSolDBException)
{
    //int iRet=0;//ZMP:305593,变量未用到

    m_iConnectFlag=0;
    m_bAutoCommitFlag=bIsAutoCommit;
    //ALLOCATE CONNECT
    rc=SQLAllocConnect(henv,&hdbc);
    CheckError("TSolDBDatabase::SQLAllocConnect");
    //CONNECT
    rc=SQLConnect(hdbc,(SQLCHAR*)m_strServerName.c_str(),SQL_NTS,(SQLCHAR*)m_strUser.c_str(),SQL_NTS,(SQLCHAR*)m_strPassword.c_str(),SQL_NTS);
    CheckError("TSolDBDatabase::SQLConnect");
    //设置不默认提交
    if(m_bAutoCommitFlag)
    {	
    	rc=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_ON);	
    }
    else
    {		
    	rc=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF);
    }
    CheckError("TSolDBDatabase::SQLSetConnectOption");

    //获取游标在COMMIT的属性
    //UDWORD fFuncs;
    //SQLGetInfo(hdbc,SQL_CURSOR_COMMIT_BEHAVIOR,
    //  (PTR)&fFuncs,sizeof(fFuncs),NULL);
    //if(fFuncs&SQL_CB_CLOSE)
    //{
    //  cout<<"CLOSE"<<endl;
    //}
    //if(fFuncs&SQL_CB_DELETE)
    //{
    //  cout<<"DELETE"<<endl;
    //}
    //if(fFuncs&SQL_CB_PRESERVE)
    //{
    //  cout<<"PRESERVE"<<endl;
    //}

    //设置连接标识
    m_iConnectFlag=1;

    //连接上就立刻提交内存数据库2008-1-14 10:47AM zhangyu
    Commit();
    return true;
}

bool TSolDBDatabase::Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit) throw (TSolDBException)
{
    SetLogin(sUser,sPassword,sServerName);

    return Connect(bIsAutoCommit);
}

int TSolDBDatabase::Disconnect()  throw(TSolDBException)
{
    int iRet=0;

    if(hdbc!=NULL)
    {
        if(m_iConnectFlag==1)
        {
            //默认回退
            Rollback();

            //DISCONNECT
            rc=SQLDisconnect(hdbc);
            CheckError("TSolDBDatabase::SQLDisconnect");
            m_iConnectFlag=0;
        }

        //FREE CONNECT
        rc=SQLFreeConnect(hdbc);
        CheckError("TSolDBDatabase::SQLFreeConnect");

        hdbc=NULL;
    }

    return iRet;
}

//数据库开启事务
void TSolDBDatabase::TransBegin() 
{
    return;
}


//数据库提交
void TSolDBDatabase::Commit() 
{

		if(!m_bAutoCommitFlag)
		{
    	rc=SQLTransact(henv,hdbc,SQL_COMMIT);
    	CheckError("TSolDBDatabase::SQLTransact");
  	}
    return;
}

//数据库回滚
void TSolDBDatabase::Rollback() 
{
    rc=SQLTransact(henv,hdbc,SQL_ROLLBACK);
    CheckError("TSolDBDatabase::SQLTransact");
    return;
}

//获取数据连接信息
void TSolDBDatabase::GetDataInfo(SQLHENV &henv,SQLHDBC &hdbc)
{
    henv=this->henv;
    hdbc=this->hdbc;
    return;
}


bool TSolDBDatabase::IsConnect()  
{
    bool bConnectFlag=false;

    if(m_iConnectFlag==1)
    {
        bConnectFlag=true;

        try
        {
            if(m_pTOdbcDBQuery==NULL)
            {
                m_pTOdbcDBQuery=new TSolDBQuery(this);
            }

            m_pTOdbcDBQuery->Close();
            m_pTOdbcDBQuery->SetSQL("SELECT 1 FROM DUAL");
            m_pTOdbcDBQuery->Open();
            /*if(m_pTOdbcDBQuery->Next())
            {
            }*/
            m_pTOdbcDBQuery->Close();
        }
        catch(TSolDBException &oe)
        {
            //if((strstr(oe.GetErrMsg(),"08001")!=NULL)
            //    ||(strstr(oe.GetErrMsg(),"08S01")!=NULL)
            //    )
            //{
            //    bConnectFlag=false;
            //}
            //else
            //{
            //    throw oe;
            //}
            bConnectFlag = false;   //这边认为有异常那一定是数据库出了问题.
            cout<<"TSolDBDatabase::IsConnect() TSolDBException:" << oe.GetErrMsg() << endl;
        }
        catch(...)
        {
            throw TSolDBException("","TSolDBDatabase::IsConnect() catch ... Exception.\n");
        }
    }
    
    return bConnectFlag;
}

TSolDBQuery * TSolDBDatabase::CreateDBQuery() throw (TSolDBException)
{
    return new TSolDBQuery(this);
}


//private函数
void TSolDBDatabase::CheckError(const char* sSql) throw (TSolDBException)
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
        throw TSolDBException(sSql,"Application Error Message=%s",ostrErr.str().c_str());
    }
}

/*类TOdbcDBField定义部分*/
TSolDBField::TSolDBField()
{
    sData=NULL;
    pslValueLen=NULL;
    iMaxRowsCount=0;
    ClearInfo();
}

TSolDBField::~TSolDBField()
{
    ClearInfo();
}

bool TSolDBField::isNULL()
{
    if(pslValueLen[m_pTOdbcDBQuery->m_iRowIndex]==-1)
    {
        return true;
    }
    return false;
}

char* TSolDBField::AsString() throw (TSolDBException)
{
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TSolDBException("TSolDBField::AsString","Application Error Message=Data set not opened!");
    }

    memset(sDataValue,0,sizeof(sDataValue));
    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
        {
            return sData;
            //break;//ZMP:305593,多余，去掉
        }
    case SQL_INTEGER:
        {
            SDWORD value=*((SDWORD*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            //ZMP:467366 
            snprintf(sDataValue, sizeof(sDataValue), "%ld", value);
            sDataValue[sizeof(sDataValue)-1] = '\0';
            break;
        }
    case SQL_SMALLINT:
        {
            SWORD value=*((SWORD*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            //ZMP:467366 
            snprintf(sDataValue, sizeof(sDataValue), "%ld", value);
            sDataValue[sizeof(sDataValue)-1] = '\0';
            break;
        }
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_BIGINT:
        {
            //ZMP:467366 
            snprintf(sDataValue, sizeof(sDataValue), "%.4lf",*(static_cast<double *>((void *)sData)+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex)));//ZMP:305593
            sDataValue[sizeof(sDataValue)-1] = '\0';
            char *pos = strstr(sDataValue,".0000");
            if(pos!=NULL) *pos='\0';
            break;
        }
    case SQL_TIMESTAMP:
    case SQL_TIME:
    case SQL_DATE:
        {
            return AsDateTimeString();
            //break;//ZMP:305593,多余，去掉
        }
    default:
        {
            throw TSolDBException("TSolDBField::AsString","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,多余，去掉
        }
    }

    return sDataValue;
}

int TSolDBField::DataType(void)
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
        break;
    case SQL_TIMESTAMP:
    case SQL_TIME:
    case SQL_DATE:
        return 3;
        break;
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
        return 4;
        break;
    default:
        {
            throw TSolDBException("TSolDBField::AsString","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,多余，去掉
        }
    }
    return -1;
}

double TSolDBField::AsFloat() throw (TSolDBException)
{
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TSolDBException("TSolDBField::AsFloat","Application Error Message=Data set not opened!");
    }


    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
        {
            return atof(sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            //break;//ZMP:305593,多余，去掉
        }
    case SQL_INTEGER:
        {
            return *((SDWORD*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            //break;//ZMP:305593,多余，去掉
        }
    case SQL_SMALLINT:
        {
            return *((SWORD*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            //break;//ZMP:305593,多余，去掉
        }
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_BIGINT:
        {
            return *(static_cast<double *>((void *)sData)+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));//ZMP:305593
            //break;//ZMP:305593,多余，去掉
        }
    case SQL_TIMESTAMP:
    case SQL_TIME:
    case SQL_DATE:
        {
            /*不能转换成DOUBLE类型*/
            throw TSolDBException("TSolDBField::AsFloat","Application Error Message=SQL_DATA_TYPE(%ld) can not convert!",ssiSqlDataType);
            //break;//ZMP:305593,多余，去掉
        }
    default:
        {
            throw TSolDBException("TSolDBField::AsFloat","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,多余，去掉
        }
    }
    dDataValue=0;
    return dDataValue;
}

long long TSolDBField::AsInteger() throw (TSolDBException)
{
    llDataValue=(long long)this->AsFloat();
    return llDataValue;
}

void TSolDBField::AsDateTime(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TSolDBException)
{
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TSolDBException("TSolDBField::AsDateTime","Application Error Message=Data set not opened!");
    }

    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
    case SQL_INTEGER:
    case SQL_SMALLINT:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_FLOAT:
    case SQL_DOUBLE:
        {
            throw TSolDBException("TSolDBField::AsDateTime","Application Error Message=SQL_DATA_TYPE(%ld) can not convert!",ssiSqlDataType);
            //break;//ZMP:305593,多余，去掉
        }
    case SQL_TIMESTAMP:
    case SQL_TIME:
    case SQL_DATE:
        {
            bool bNumber=false;
            int iSegIndex=0;
            char *sCur=sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);
            int iCount=slDataLen;
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
    default:
        {
            throw TSolDBException("TSolDBField::AsDateTime","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,多余，去掉
        }
    }
    return;
}

//转换时间格式为YYYYMMDDHHMISS
char* TSolDBField::AsDateTimeString() throw (TSolDBException)
{
    int iYear=0;
    int iMonth=0;
    int iDay=0;
    int iHour=0;
    int iMinute=0;
    int iSecond=0;

    this->AsDateTime(iYear,iMonth,iDay,iHour,iMinute,iSecond);
    memset(sDateTime,0,sizeof(sDateTime));
    //ZMP:467366 
    snprintf(sDateTime, sizeof(sDateTime), "%04d%02d%02d%02d%02d%02d",iYear,iMonth,iDay,iHour,iMinute,iSecond);
    sDateTime[sizeof(sDateTime)-1] = '\n';
    return sDateTime;
}

char* TSolDBField::AsBlobBuffer(int &iBufferLen)
{
    char *sBlobBuffer=NULL;

    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TSolDBException("TSolDBField::AsBlobBuffer","Application Error Message=Data set not opened!");
    }

    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
    case SQL_INTEGER:
    case SQL_SMALLINT:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_TIMESTAMP:
    case SQL_TIME:
    case SQL_DATE:
        {
            throw TSolDBException("TSolDBField::AsDateTime","Application Error Message=SQL_DATA_TYPE(%ld) can not convert!",ssiSqlDataType);
            //break;//ZMP:305593,多余，去掉
        }
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
        {
            iBufferLen=(int)pslValueLen[(m_pTOdbcDBQuery->m_iRowIndex)];
            sBlobBuffer=sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);
            break;
        }
    default:
        {
            throw TSolDBException("TSolDBField::AsDateTime","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,多余，去掉
        }
    }
    return sBlobBuffer;
}

const char * TSolDBField::AsName()
{
    return strFieldName.c_str();
}


//private函数
void TSolDBField::ClearInfo()
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
        ||(iMaxRowsCount>1)
        )
        )
    {
        DELETE(sData);
    }
    slDataLen=-1;
    ssiDataType=0;

    DELETE(pslValueLen);

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



/*类TOdbcDBParam定义部分*/
TSolDBParam::TSolDBParam()
{
    pData=NULL;
    allValueLen=NULL;
    ClearInfo();
}

TSolDBParam::~TSolDBParam()
{
    ClearInfo();
}

//private函数
void TSolDBParam::ClearInfo()
{
    m_pTOdbcDBQuery=NULL;
    strParamName="";
    iIndex=-1;
    ssiSqlDataType=0;
    sulPrecision=0;
    ssiScale=0;
    ssiNullable=0;
    DELETE(pData);
    
    slDataLen=-1;
    ssiDataType=0;
    iElementCount=-1;
    DELETE(allValueLen);
    
    bSetValue=false;
    return;
}

/*类TOdbcDBQuery定义部分*/
TSolDBQuery::TSolDBQuery(TSolDBDatabase *pTOdbcDBDatabase)
{
    henv=NULL;
    hdbc=NULL;
    hstmt=NULL;
    rc=0;

    if(pTOdbcDBDatabase==NULL)
    {
        throw TSolDBException("TSolDBQuery::TSolDBQuery","Application Error Message=Parameter Error!");
    }

    if(pTOdbcDBDatabase->m_iConnectFlag!=1)
    {
        throw TSolDBException("TSolDBQuery::TSolDBQuery","Application Error Message=Database not connected!");
    }

    pTOdbcDBDatabase->GetDataInfo(henv,hdbc);
    m_pTOdbcDBDatabase=pTOdbcDBDatabase;

    //ALLOCATE STMT
    rc=SQLAllocStmt(hdbc,&hstmt);
    CheckError("TSolDBQuery::SQLAllocStmt");

    m_iParamCount=0;
    m_pTOdbcDBParamArray=new TSolDBParam[MAX_COLUMN_COUNT];
    if(m_pTOdbcDBParamArray==NULL)
    {
        throw TSolDBException("TSolDBQuery::TSolDBQuery","Application Error Message=Malloc error!");
    }

    m_iFieldCount=0;
    m_pTOdbcDBFieldArray=new TSolDBField[MAX_COLUMN_COUNT];
    if(m_pTOdbcDBFieldArray==NULL)
    {
        throw TSolDBException("TSolDBQuery::TSolDBQuery","Application Error Message=Malloc error!");
    }

    m_iQueryState=QUERY_BEFORE_OPEN;
    aRowsStatus=NULL;
    m_iRowIndex=0;
    m_iPreFetchRows=0;
    m_iFetchedRows=0;    
}

TSolDBQuery::~TSolDBQuery()
{
    if(hstmt!=NULL)
    {
        //关闭执行的SQL
        //rc=SQLFreeStmt(hstmt,SQL_CLOSE);
        //CheckError(m_sSql);
        Close();

        //FREE STMT
        rc=SQLFreeStmt(hstmt,SQL_DROP);
        CheckError(m_sSql);
        hstmt=NULL;
    }
    henv=NULL;
    hdbc=NULL;

    m_pTOdbcDBDatabase=NULL;

    DELETE_A(m_pTOdbcDBFieldArray);
    DELETE_A(m_pTOdbcDBParamArray);
    DELETE_A(aRowsStatus);
}

void TSolDBQuery::Close()
{
    rc=SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    CheckError(m_sSql);

    rc=SQLFreeStmt(hstmt,SQL_UNBIND);
    CheckError(m_sSql);

    //关闭执行的SQL
    rc=SQLFreeStmt(hstmt,SQL_CLOSE);
    CheckError(m_sSql);

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
    memset(m_sSql,0,sizeof(m_sSql));
    m_iQueryState=QUERY_BEFORE_OPEN;
    m_lAffectRows=0;
    m_iPreFetchRows=0;

    m_iRowIndex=0;
    m_iFetchedRows=0;

    return;
}

void TSolDBQuery::CloseSQL()
{
	rc = SQLFreeStmt(hstmt, SQL_CLOSE);
	CheckError(m_sSql);
	 
	m_iQueryState=QUERY_BEFORE_OPEN;
    m_iPreFetchRows=0;
    m_iRowIndex=0;
    m_iFetchedRows=0;
}

void TSolDBQuery::SetSQL(const char *sSqlStatement,int iPreFetchRows) throw (TSolDBException)
{
    char sFieldName[MAX_COLUMN_NAME]={0}; //356853 for cppcheck
    int iNameLen=0;
    
    if(sSqlStatement==NULL)
    {
        throw TSolDBException("TSolDBQuery::SetSQL","Application Error Message=Parameter Error!");
    }

    if(strlen(sSqlStatement)>=sizeof(m_sSql))
    {
        throw TSolDBException("TSolDBQuery::SetSQL","Application Error Message=Length of sql is beyond limit(%ld)",sizeof(m_sSql));
    }

    strncpy(m_sSql,sSqlStatement,sizeof(m_sSql)-1);
    m_sSql[sizeof(m_sSql)-1]='\0';

    //需要替换:参数格式到?
    ParseSql(m_sSql);

    //PREPARE
    rc=SQLPrepare(hstmt,(SQLCHAR *)m_sSql,sizeof(m_sSql));
    CheckError(m_sSql);

    //NUMBER PARAM;
    int iParamCount=0;
    SQLSMALLINT siParamCount=0;
    rc=SQLNumParams(hstmt,(SQLSMALLINT*)&siParamCount);
    iParamCount=siParamCount;
    CheckError(m_sSql);

    if(iParamCount!=m_iParamCount)
    {
        throw TSolDBException("TSolDBQuery::SetSQL","Application Error Message=ParseSql() Error!");
    }
    //获取参数信息
    for(int iParamIndex=1;iParamIndex<=m_iParamCount;iParamIndex++)
    {
        rc=SQLDescribeParam(hstmt,iParamIndex,
            (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType),
            (SQLULEN*)&(m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision),
            (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiScale),
            (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable));
        CheckError(m_sSql);
    }

    //NUMBER FIELD;
    int iFieldCount=0;
    SQLSMALLINT siFieldCount=0;
    rc=SQLNumResultCols(hstmt,(SQLSMALLINT*)&siFieldCount);
    iFieldCount=siFieldCount;
    CheckError(m_sSql);

    if(iFieldCount>MAX_COLUMN_COUNT)
    {
        throw TSolDBException("TSolDBQuery::SetSQL","Application Error Message=Count of field beyond limit(%ld)",MAX_COLUMN_COUNT);
    }
    m_iFieldCount=iFieldCount;
    
    //new position
    if(iPreFetchRows>1)
    {
        SQLSetStmtOption(hstmt,SQL_CONCURRENCY,SQL_CONCUR_READ_ONLY);
        SQLSetStmtOption(hstmt,SQL_CURSOR_TYPE,SQL_CURSOR_KEYSET_DRIVEN);
        SQLSetStmtOption(hstmt,SQL_ROWSET_SIZE,iPreFetchRows);
    }
    
    for(int iFieldIndex=1;iFieldIndex<=m_iFieldCount;iFieldIndex++)
    {
        iNameLen=0;

        //获取各个选择列信息
        rc=SQLDescribeCol(hstmt,iFieldIndex,(SQLCHAR*)sFieldName,MAX_COLUMN_NAME,
            (SQLSMALLINT *)&iNameLen,
            (SQLSMALLINT *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].ssiSqlDataType),
            (SQLULEN *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].sulPrecision),
            (SQLSMALLINT *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].ssiScale),
            (SQLSMALLINT *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].ssiNullable));
        CheckError(m_sSql);

        
        m_pTOdbcDBFieldArray[iFieldIndex-1].m_pTOdbcDBQuery=this;
        m_pTOdbcDBFieldArray[iFieldIndex-1].strFieldName=sFieldName;
        m_pTOdbcDBFieldArray[iFieldIndex-1].iIndex=iFieldIndex-1;

        //分配各个选择列缓冲区
        GetFieldBufferInfo(m_pTOdbcDBFieldArray[iFieldIndex-1],iPreFetchRows);
        
        //绑定各个选择列缓冲区
        rc=SQLBindCol(hstmt,iFieldIndex,
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

void TSolDBQuery::Open(int prefetchRows) throw (TSolDBException)
{
    m_lAffectRows=0;
    if((m_iQueryState!=QUERY_BEFORE_OPEN)
        &&(m_iQueryState!=QUERY_FETCH_EOF)
        &&(m_iQueryState!=QUERY_FETCH_BOF)
        &&(m_iQueryState!=QUERY_AFTER_OPEN)
        )
    {//判断执行状态
        //应用修改,不可能到达此处
        throw TSolDBException("TSolDBQuery::Open","Application Error Message=Data set not closed!");
    }

    if(m_iQueryState!=QUERY_BEFORE_OPEN)
    {//不是CLOSE后调用
        rc=SQLFreeStmt(hstmt,SQL_CLOSE);
        CheckError(m_sSql);
    }

    if(m_iFieldCount<=0)
    {
        throw TSolDBException("TSolDBQuery::Open","Application Error Message=SQL is not for SELECT!");
    }

    if(m_iParamCount>0)
    {//存在参数情况下,校验
        for(int iParamIndex=1;iParamIndex<=m_iParamCount;iParamIndex++)
        {
            if(m_pTOdbcDBParamArray[iParamIndex-1].iElementCount<=0)
            {
                throw TSolDBException("TSolDBQuery::Open","Application Error Message=Param(%d) not be set!",iParamIndex);
            }
        }//end of for(遍历所有参数,校验是否已经赋值)
    }

    //回复设置参数数组元素个数
    iRow=0;
    //rc=SQLParamOptions(hstmt,1,&iRow);
    rc=SQLSetStmtAttr(hstmt,1,&iRow,0);
    CheckError(m_sSql);
    
    //执行SQL
    rc=SQLExecute(hstmt);
    CheckError(m_sSql);

    m_iQueryState=QUERY_AFTER_OPEN;
    return;
}

bool TSolDBQuery::Next() throw (TSolDBException)
{
    if((m_iQueryState!=QUERY_AFTER_OPEN)
        &&(m_iQueryState!=QUERY_FETCH_BOF))
    {//判断执行状态
        throw TSolDBException("TSolDBQuery::Next","Application Error Message=Data set not opened!");
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
            memset(m_pTOdbcDBFieldArray[iFieldIndex-1].sData,0,(size_t)m_pTOdbcDBFieldArray[iFieldIndex-1].slDataLen);
            memset(m_pTOdbcDBFieldArray[iFieldIndex-1].pslValueLen,0,sizeof(SQLLEN)*m_pTOdbcDBFieldArray[iFieldIndex-1].iRowsCount);
        }

        m_iRowIndex=0;
        m_iFetchedRows=0;
        if(m_iPreFetchRows<=1)
        {
            rc=SQLFetch(hstmt);
            if((rc==SQL_NO_DATA_FOUND)
                //||(rc==SQL_NO_DATA)TimesTen for unix无
                )
            {//未找到数据
                m_iQueryState=QUERY_FETCH_EOF;
                return false;
            }
            CheckError(m_sSql);
            m_iFetchedRows=1;    
        }
        else
        {
            rc=SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,m_iPreFetchRows,&m_iFetchedRows,aRowsStatus);
            if((rc==SQL_NO_DATA_FOUND)
                //||(rc==SQL_NO_DATA)TimesTen for unix无
                )
            {//未找到数据
                m_iQueryState=QUERY_FETCH_EOF;
                return false;
            }
            CheckError(m_sSql);
        }

    }
    m_lAffectRows=m_lAffectRows+m_iFetchedRows;
    return true;
}

bool TSolDBQuery::Execute(int iExecuteRows)throw (TSolDBException)
{
    int iMaxParamElementCount=0;

    m_lAffectRows=0;
    if(m_iFieldCount>0)
    {
        throw TSolDBException("TSolDBQuery::Execute","Application Error Message=SQL can not Execute!");
    }

    if(m_iQueryState!=QUERY_BEFORE_OPEN)
    {//判断执行状态
        throw TSolDBException("TSolDBQuery::Execute","Application Error Message=Called with error env!");
    }

    if(m_iParamCount>0)
    {//存在参数情况下,校验
        iMaxParamElementCount=m_pTOdbcDBParamArray[0].iElementCount;
        for(int iParamIndex=1;iParamIndex<=m_iParamCount;iParamIndex++)
        {
            if(m_pTOdbcDBParamArray[iParamIndex-1].iElementCount<=0)
            {
                throw TSolDBException("TSolDBQuery::Execute","Application Error Message=Param(%d) not be set!",iParamIndex);
            }
            else if(m_pTOdbcDBParamArray[iParamIndex-1].iElementCount<iMaxParamElementCount)
            {
                iMaxParamElementCount=m_pTOdbcDBParamArray[iParamIndex-1].iElementCount;
            }
        }//end of for(遍历所有参数,校验是否已经赋值)

        if((iExecuteRows>0)&&(iExecuteRows<iMaxParamElementCount))
        {
            iMaxParamElementCount=iExecuteRows;
        }

        //设置参数数组元素个数
        iRow=0;
        //rc=SQLParamOptions(hstmt,iMaxParamElementCount,&iRow);
        rc=SQLSetStmtAttr(hstmt,iMaxParamElementCount,&iRow,0);
        CheckError(m_sSql);
    }


    //Execute
    rc=SQLExecute(hstmt);
    CheckError(m_sSql);

    m_lAffectRows=0;
    rc=SQLRowCount(hstmt,(SQLLEN*)&m_lAffectRows);
    CheckError(m_sSql);

    return true;
}

bool TSolDBQuery::TransBegin()
{
    m_pTOdbcDBDatabase->TransBegin();
    return true;
}

bool TSolDBQuery::Commit()
{
    m_pTOdbcDBDatabase->Commit();
    return true;
}

bool TSolDBQuery::Rollback()
{
    m_pTOdbcDBDatabase->Rollback();
    return true;
}

int TSolDBQuery::RowsAffected()
{
    return m_lAffectRows;
}

int TSolDBQuery::FieldCount()
{
    return m_iFieldCount;
}

TSolDBField& TSolDBQuery::Field(int iIndex) throw (TSolDBException)
{
    if((iIndex<0)
        ||(iIndex>=m_iFieldCount)
        )
    {
        throw TSolDBException("TSolDBQuery::Field","Application Error Message=Parameter Error!");
    }
    return m_pTOdbcDBFieldArray[iIndex];
}

TSolDBField& TSolDBQuery::Field(const char *sFieldName) throw (TSolDBException)
{
    if((sFieldName==NULL)
        ||(strlen(sFieldName)==0)
        ||(m_iFieldCount<=0)
        )
    {
        throw TSolDBException("TSolDBQuery::Field","Application Error Message=Parameter Error!");
    }

    for(int iIndex=0;iIndex<m_iFieldCount;iIndex++)
    {
        if(FUNC_STR::StrCmpNoCase(sFieldName,m_pTOdbcDBFieldArray[iIndex].strFieldName.c_str())==0)
        {//名字无大小区分
            return m_pTOdbcDBFieldArray[iIndex];
        }
    }

    throw TSolDBException("TSolDBQuery::Field","Application Error Message=Field(%s) not exist!",sFieldName);
}
/*begin zmp 841469 */
void TSolDBQuery::GetValue(void *pStruct,int* Column)throw (TSolDBException)
{
    throw TSolDBException("TSolDBException::GetValue","Not Support!");
}
void TSolDBQuery::SetUpdateMDBFlag(bool flag)throw (TSolDBException)
{
    throw TSolDBException("TSolDBException::SetUpdateMDBFlag","Not Support!");
}
bool TSolDBQuery::GetUpdateMDBFlag()throw (TSolDBException)
{
    throw TSolDBException("TSolDBException::GetUpdateMDBFlag","Not Support!");
}
/*end zmp 841469 */
int TSolDBQuery::ParamCount()
{
    return m_iParamCount;
}

TSolDBParam& TSolDBQuery::Param(int iIndex)
{
    if((iIndex<0)
        ||(iIndex>=m_iParamCount)
        )
    {
        throw TSolDBException("TSolDBQuery::Param","Application Error Message=Parameter Error!");
    }
    return m_pTOdbcDBParamArray[iIndex];
}

TSolDBParam& TSolDBQuery::Param(const char*sParamName)
{
    if((sParamName==NULL)
        ||(strlen(sParamName)==0)
        ||(m_iParamCount<=0)
        )
    {
        throw TSolDBException("TSolDBQuery::Param","Application Error Message=Parameter Error!");
    }

    return *(ParamByCondition(sParamName));
}

//设置参数值
void TSolDBQuery::SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBParam,sParamValue);
    return;
}

void TSolDBQuery::SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused)throw (TSolDBException)
{
    SetParameter(sParamName,(long)iParamValue);
    return;
}

void TSolDBQuery::SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBParam,lParamValue);
    return;
}

void TSolDBQuery::SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBParam,dParamValue);
    return;
}

void TSolDBQuery::SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused)throw (TSolDBException)
{
#ifdef _WIN32
    //ZMP:467366 
    snprintf(m_sLongLong, sizeof(m_sLongLong), "%I64d", llParamValue);
    m_sLongLong[sizeof(m_sLongLong)-1] = '\0';
#else
    snprintf(m_sLongLong, sizeof(m_sLongLong), "%lld", llParamValue);
#endif
    SetParameter(sParamName,m_sLongLong);
    return;
}

void TSolDBQuery::SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBParam,sParamValue,iBufferLen);
    return;
}

void TSolDBQuery::SetParameterNULL(const char *sParamName)throw (TSolDBException)
{
    SetParameter(sParamName, (char*)NULL);
    return;
}

void TSolDBQuery::SetParameter(int iParamIndex,const char* sParamValue)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,sParamValue);
    return;
}

void TSolDBQuery::SetParameter(int iParamIndex,int iParamValue)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,(long)iParamValue);
    return;
}
void TSolDBQuery::SetParameter(int iParamIndex,long lParamValue)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,lParamValue);
    return;
}
void TSolDBQuery::SetParameter(int iParamIndex,double dParamValue)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,dParamValue);
    return;
}
void TSolDBQuery::SetParameter(int iParamIndex,long long llParamValue)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,(double)llParamValue);
    return;
}
void TSolDBQuery::SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,sParamValue,iBufferLen);
    return;
}

void TSolDBQuery::SetParameterNULL(int iParamIndex)throw (TSolDBException)
{
    TSolDBParam *pTOdbcDBParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBParam,(char*)NULL);
    return;
}

//设置数组参数值
void TSolDBQuery::SetParamArray(const char *sParamName,char **asParamValue,int iInterval,
                                 int iElementSize,int iArraySize,bool bOutput)throw (TSolDBException)
{
    if((asParamValue==NULL)
        ||(iElementSize<=0)
        ||(iArraySize<=0)
        )
    {
        throw TSolDBException("TSolDBQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    TSolDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;

    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
        )
    {
        DELETE(pTOdbcDBParam->pData);
        pTOdbcDBParam->bSetValue=false;
    }
    
    if((pTOdbcDBParam->allValueLen!=NULL)
        &&(pTOdbcDBParam->iElementCount<iElementCount)
        )
    {
        DELETE(pTOdbcDBParam->allValueLen);
        pTOdbcDBParam->bSetValue=false;
    }

    if(pTOdbcDBParam->bSetValue==false)
    {
        if(pTOdbcDBParam->pData==NULL)
        {
            pTOdbcDBParam->pData=new char[iElementSize*iElementCount];
            pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        }
        
        memcpy(pTOdbcDBParam->pData,asParamValue,iArraySize);
        pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBParam->llElementSize=iElementSize;
           
        if(pTOdbcDBParam->allValueLen==NULL)
        {
            pTOdbcDBParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBParam->allValueLen,0,sizeof(SQLLEN)*iElementCount);
        }
    
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=SQL_NTS;
        }
    
        rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        memcpy(pTOdbcDBParam->pData,asParamValue,iArraySize);
        pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBParam->llElementSize=iElementSize;
        memset(pTOdbcDBParam->allValueLen,0,sizeof(SQLLEN)*iElementCount);
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=SQL_NTS;
        }
    }

    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;
    return;
}

void TSolDBQuery::SetParamArray(const char *sParamName,int *aiParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TSolDBException)
{
    if((aiParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TSolDBException("TSolDBQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    int iElementSize=sizeof(int);
    TSolDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;

    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
        )
    {
        DELETE(pTOdbcDBParam->pData);
        pTOdbcDBParam->bSetValue=false;
    }
    
    if((pTOdbcDBParam->allValueLen!=NULL)
        &&(pTOdbcDBParam->iElementCount<iElementCount)
        )
    {
        DELETE(pTOdbcDBParam->allValueLen);
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
            memset(pTOdbcDBParam->allValueLen,0,sizeof(SQLLEN)*iElementCount);
        }
    
        rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
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
        memset(pTOdbcDBParam->allValueLen,0,sizeof(SQLLEN)*iElementCount);
    }

    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;    
    return;
}

void TSolDBQuery::SetParamArray(const char *sParamName,long *alParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TSolDBException)
{
    if(sizeof(long)==sizeof(int))
    {
        int *aiParamValue=NULL;

        try
        {
            int iElementCount=iArraySize/iInterval;
            int *aiParamValue=new int[iElementCount];
            for(int iIndex=0;iIndex<iElementCount;iIndex++)
            {
                aiParamValue[iIndex]=(int)alParamValue[iIndex];
            }
            SetParamArray(sParamName,aiParamValue,sizeof(int),iElementCount*sizeof(int),bOutput);
        }
        catch(TSolDBException &e)
        {
            DELETE(aiParamValue);
            //ZMP:305593
            TSolDBException  ex(e);
            throw ex;
        }

        DELETE(aiParamValue);
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
            SetParamArray(sParamName,(long long*)allParamValue,sizeof(long long),iElementCount*sizeof(long long),bOutput);
        }
        catch(TSolDBException &e)
        {
            DELETE(allParamValue);
            throw e;
        }

        DELETE(allParamValue);
    }

    return;
}

void TSolDBQuery::SetParamArray(const char *sParamName,double *adParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TSolDBException)
{
    if((adParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TSolDBException("TSolDBQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    int iElementSize=sizeof(double);
    TSolDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;

    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
      )
    {
        DELETE(pTOdbcDBParam->pData);
        pTOdbcDBParam->bSetValue=false;
    }
    
    if((pTOdbcDBParam->allValueLen!=NULL)
        &&(pTOdbcDBParam->iElementCount<iElementCount)
        )
    {
        DELETE(pTOdbcDBParam->allValueLen);
        pTOdbcDBParam->bSetValue=false;
    }

    if(pTOdbcDBParam->bSetValue==false)
    {
        if(pTOdbcDBParam->pData==NULL)
        {
            pTOdbcDBParam->pData=new double[iElementCount];
            pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        }

        memcpy(pTOdbcDBParam->pData,adParamValue,iArraySize);
        pTOdbcDBParam->ssiDataType=SQL_C_DOUBLE;
        pTOdbcDBParam->llElementSize=iElementSize;
    
        if(pTOdbcDBParam->allValueLen==NULL)
        {
            pTOdbcDBParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBParam->allValueLen,0,sizeof(SQLLEN)*iElementCount);
        }
    
        rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        memcpy(pTOdbcDBParam->pData,adParamValue,iArraySize);
        pTOdbcDBParam->ssiDataType=SQL_C_DOUBLE;
        pTOdbcDBParam->llElementSize=iElementSize;
        memset(pTOdbcDBParam->allValueLen,0,sizeof(SQLLEN)*iElementCount);
    }
    
    pTOdbcDBParam->iElementCount=iArraySize/iElementSize;
    pTOdbcDBParam->bSetValue=true;
    return;
}

void TSolDBQuery::SetParamArray(const char *sParamName,long long *allParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TSolDBException)
{
    if((allParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TSolDBException("TSolDBQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    
    TSolDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;
    int iElementSize=SIZE_LONG_LONG;

    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
      )
    {
        DELETE(pTOdbcDBParam->pData);
        pTOdbcDBParam->bSetValue=false;
    }
    
     if((pTOdbcDBParam->allValueLen!=NULL)
        &&(pTOdbcDBParam->iElementCount<iElementCount)
        )
    {
        DELETE(pTOdbcDBParam->allValueLen);
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
                //ZMP:467366 
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%I64d", allParamValue[iIndex]);
                m_sLongLong[sizeof(m_sLongLong)-1] = '\0';
            #else
                //ZMP:467366 
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%lld", allParamValue[iIndex]);
            #endif
            memcpy((char*)pTOdbcDBParam->pData+SIZE_LONG_LONG*iIndex,m_sLongLong,sizeof(m_sLongLong));
        }
        
        pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBParam->llElementSize=iElementSize;
       
        if(pTOdbcDBParam->allValueLen==NULL)
        {
            pTOdbcDBParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBParam->allValueLen,0,sizeof(SQLLEN)*iElementCount);
        }
    
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=SQL_NTS;
        }
    
        rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
                
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            #ifdef _WIN32
                //ZMP:467366 
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%I64d", allParamValue[iIndex]);
                m_sLongLong[sizeof(m_sLongLong)-1] = '\0';
            #else
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%lld",allParamValue[iIndex]);
            #endif
            memcpy((char*)pTOdbcDBParam->pData+SIZE_LONG_LONG*iIndex,m_sLongLong,sizeof(m_sLongLong));
        }
        
        pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBParam->llElementSize=iElementSize;
        
        memset(pTOdbcDBParam->allValueLen,0,sizeof(SQLLEN)*iElementCount);
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=SQL_NTS;
        }
    }

    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;
    return;
}

void TSolDBQuery::SetBlobParamArray(const char *sParamName,char *sParamValue,
                                     int iBufferLen,int iArraySize,bool bOutput)throw (TSolDBException)
{
    if((sParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TSolDBException("TSolDBQuery::SetBlobParamArray","Application Error Message=Parameter Error!");
    }

    int iElementSize=iBufferLen;
    TSolDBParam *pTOdbcDBParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iBufferLen;

    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<iElementSize*iElementCount)
      )
    {
        DELETE(pTOdbcDBParam->pData);
        pTOdbcDBParam->bSetValue=false;
    }
    
    if((pTOdbcDBParam->allValueLen!=NULL)
        &&(pTOdbcDBParam->iElementCount<iElementCount)
        )
    {
        DELETE(pTOdbcDBParam->allValueLen);
        pTOdbcDBParam->bSetValue=false;
    }
    
    if(pTOdbcDBParam->bSetValue==false)
    {
        if(pTOdbcDBParam->pData==NULL)
        {
            pTOdbcDBParam->pData=new char[iArraySize];
            pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        }
        
        memcpy(pTOdbcDBParam->pData,sParamValue,iElementSize*iElementCount);
        pTOdbcDBParam->ssiDataType=SQL_C_BINARY;
        pTOdbcDBParam->llElementSize=iElementSize;
        
        if(pTOdbcDBParam->allValueLen==NULL)
        {
            pTOdbcDBParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBParam->allValueLen,0,sizeof(SQLLEN)*iElementCount);
        }
    
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=iElementSize;
        }
    
        rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBParam->pData,(SQLLEN)pTOdbcDBParam->llElementSize,
            (SQLLEN*)pTOdbcDBParam->allValueLen);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBParam->slDataLen=iElementSize*iElementCount;
        memcpy(pTOdbcDBParam->pData,sParamValue,iElementSize*iElementCount);
        pTOdbcDBParam->ssiDataType=SQL_C_BINARY;
        pTOdbcDBParam->llElementSize=iElementSize;
        
        memset(pTOdbcDBParam->allValueLen,0,sizeof(SQLLEN)*iElementCount);
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBParam->allValueLen)[iIndex]=iElementSize;
        }
    }

    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;
    return;
}

//private函数
void TSolDBQuery::CheckError(const char* sSql) throw (TSolDBException)
{
    static ostringstream ostrErr;
    int iRet=0;

    if((rc!=SQL_SUCCESS)
        &&(rc!=SQL_SUCCESS_WITH_INFO)
        )
    {
        ostrErr.str("");
        iRet=DBError(rc,henv,hdbc,hstmt,ostrErr);
    }

    if(iRet!=0)
    {
        throw TSolDBException(sSql,"Application Error Message=%s",ostrErr.str().c_str());
    }
}

void TSolDBQuery::ParseSql(char* sSqlStatement)
{
    int iLen=0;
    int iIndex=0;
    int iParamBegin=0;
    int iParamEnd=0;
    int iQuoteFlag=0;                                       //为0表示无引号,为1表示单引号开始
    char    cCur=0;
    string strParamName="";

    iLen=strlen(sSqlStatement);
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
                throw TSolDBException("TSolDBQuery::ParseSql","Application Error Message=Param name error");
            }
            else
            {
                if(m_iParamCount==MAX_COLUMN_COUNT)
                {
                    throw TSolDBException("TSolDBQuery::ParseSql","Application Error Message=Count of prams beyond limit(%ld)",MAX_COLUMN_COUNT);
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

void TSolDBQuery::GetFieldBufferInfo(TSolDBField &oTOdbcDBField,int &iRowsCount)
{
    void *pBuffer=NULL;
    SQLSMALLINT ssiSqlDataType=oTOdbcDBField.ssiSqlDataType;
    SQLULEN sulPrecision=oTOdbcDBField.sulPrecision;
    SQLSMALLINT ssiScale=oTOdbcDBField.ssiScale;
    SQLSMALLINT ssiDataType=0;
    SQLLEN slDataLen=0;

    if(iRowsCount<=1)
    {
        iRowsCount=1;
    }

    if(oTOdbcDBField.sData!=NULL)
    {
        //if(iRowsCount<=oTOdbcDBField.iMaxRowsCount)
        //{//已经分配足够大小的缓冲区了
        //    return;
        //}
        bool bFlag = ( (oTOdbcDBField.ssiDataType==SQL_C_CHAR)
            ||(oTOdbcDBField.ssiDataType==SQL_C_BINARY)
            ||(oTOdbcDBField.iMaxRowsCount>1) ) ; //315428 
        if( bFlag )
        {
            DELETE(oTOdbcDBField.sData);
        }
        oTOdbcDBField.sData=NULL;
        oTOdbcDBField.iRowsCount=0;
        if(oTOdbcDBField.pslValueLen!=NULL)
        {
            DELETE(oTOdbcDBField.pslValueLen);
        }
    }
    
    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
        {
            slDataLen=sulPrecision+1;
            if(iRowsCount<=1)
            {
                pBuffer=new char[sulPrecision+1];
            }
            else
            {
                pBuffer=new char[(sulPrecision+1)*iRowsCount];
            }
            ssiDataType=SQL_C_CHAR;
            break;
        }
    case SQL_INTEGER:
        {
            slDataLen=sizeof(SDWORD);
            if(iRowsCount<=1)
            {
                pBuffer=&(oTOdbcDBField.sdwDataValue);
            }
            else
            {   
                pBuffer=(SDWORD*)new SDWORD[iRowsCount];
            }
            ssiDataType=SQL_C_LONG;
            break;
        }
    case SQL_SMALLINT:
        {
            slDataLen=sizeof(SWORD);
            if(iRowsCount<=1)
            {
                pBuffer=&(oTOdbcDBField.swDataValue);
            }
            else
            {   
                pBuffer=(SWORD*)new SWORD[iRowsCount];
            }
            ssiDataType=SQL_C_SHORT;
            break;
        }
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_BIGINT:
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
    case SQL_DATE:
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
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
        {//BLOB/BINARY
            slDataLen=sulPrecision+1;
            if(iRowsCount<=1)
            {
                pBuffer=new char[sulPrecision+1];
            }
            else
            {
                pBuffer=new char[(sulPrecision+1)*iRowsCount];
            }
            
            ssiDataType=SQL_C_BINARY;
            break;
        }
    default:
        {
            throw TSolDBException("TSolDBQuery::GetFieldBufferInfo","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,多余，去掉
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
TSolDBParam* TSolDBQuery::ParamByCondition(const char *sParamName,int iValueFlag,int iIndex)
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
        throw TSolDBException("TSolDBQuery::ParamByCondition","Application Error Message=Parameter Error!");
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

    throw TSolDBException("TSolDBQuery::ParamByCondition","Application Error Message=Param(%s) not exist!",sParamName);

    return NULL;
}

//设置参数值
void TSolDBQuery::SetParameter(TSolDBParam *pTOdbcDBParam,const char* sParamValue) throw (TSolDBException)
{
    int iElementCount=1;
    int iLenParamValue = 0;
    if (sParamValue!=NULL)
    {
        iLenParamValue = strlen(sParamValue);
    }
    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<=(SQLLEN)iLenParamValue)
        )
    {
        DELETE(pTOdbcDBParam->pData);
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
                pTOdbcDBParam->slDataLen=strlen(sParamValue)+1;
            }
            memset(pTOdbcDBParam->pData,0,strlen(sParamValue)+1);
            memcpy(pTOdbcDBParam->pData,sParamValue,strlen(sParamValue));
            pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBParam->llValueLen=SQL_NTS;
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
            pTOdbcDBParam->llValueLen=SQL_NULL_DATA;
        }
    
        rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
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
        {
            ((char*)pTOdbcDBParam->pData)[0]='\0';
            pTOdbcDBParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBParam->llValueLen=SQL_NULL_DATA;
        }
    }

    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;
    
    return;
}

void TSolDBQuery::SetParameter(TSolDBParam *pTOdbcDBParam,int iParamValue)throw (TSolDBException)
{
    int iElementCount=1;

    if(pTOdbcDBParam->bSetValue==false)
    {
        pTOdbcDBParam->iData=iParamValue;
        pTOdbcDBParam->slDataLen=sizeof(iParamValue);
        pTOdbcDBParam->ssiDataType=SQL_C_SLONG;
        pTOdbcDBParam->llValueLen=0;
    
        rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)&(pTOdbcDBParam->iData),(SQLLEN)pTOdbcDBParam->slDataLen,
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

void TSolDBQuery::SetParameter(TSolDBParam *pTOdbcDBParam,long lParamValue)throw (TSolDBException)
{
    double dParamValue=lParamValue;
    SetParameter(pTOdbcDBParam,dParamValue);
    return;
}

void TSolDBQuery::SetParameter(TSolDBParam *pTOdbcDBParam,double dParamValue)throw (TSolDBException)
{
    int iElementCount=1;

    if(pTOdbcDBParam->bSetValue==false)
    {
        pTOdbcDBParam->dData=dParamValue;
        pTOdbcDBParam->slDataLen=sizeof(double);
        pTOdbcDBParam->ssiDataType=SQL_C_DOUBLE;
        pTOdbcDBParam->llValueLen=0;
    
        rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBParam->ssiDataType,(SQLSMALLINT)pTOdbcDBParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBParam->sulPrecision,(SQLSMALLINT)pTOdbcDBParam->ssiScale,
            (SQLPOINTER*)&(pTOdbcDBParam->dData),(SQLLEN)pTOdbcDBParam->slDataLen,
            (SQLLEN*)&(pTOdbcDBParam->llValueLen));
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBParam->dData=dParamValue;
    }
    
    pTOdbcDBParam->iElementCount=iElementCount;
    pTOdbcDBParam->bSetValue=true;
    
    return;
}

void TSolDBQuery::SetParameter(TSolDBParam *pTOdbcDBParam,long long llParamValue)throw (TSolDBException)
{
    SetParameter(pTOdbcDBParam,(double)llParamValue);
    return;
}

void TSolDBQuery::SetParameter(TSolDBParam *pTOdbcDBParam,const char* sParamValue,int iBufferLen)throw (TSolDBException)
{
    int iElementCount=1;

    if((pTOdbcDBParam->pData!=NULL)
        &&(pTOdbcDBParam->slDataLen<(SQLLEN)iBufferLen)
        )
    {
        DELETE(pTOdbcDBParam->pData);
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
            memset(pTOdbcDBParam->pData,0,iBufferLen);
            memcpy(pTOdbcDBParam->pData,sParamValue,iBufferLen);
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
    
        rc=SQLBindParameter(hstmt,pTOdbcDBParam->iIndex+1,SQL_PARAM_INPUT,
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
            memset(pTOdbcDBParam->pData,0,iBufferLen);
            memcpy(pTOdbcDBParam->pData,sParamValue,iBufferLen);
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

void TSolDBQuery::SetParameterNULL(TSolDBParam *pTOdbcDBParam) throw (TSolDBException)
{
    SetParameter(pTOdbcDBParam,(char*)NULL);
    return;
}
 
 
