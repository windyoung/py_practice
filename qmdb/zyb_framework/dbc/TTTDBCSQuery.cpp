//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#ifdef _QM
	#include "TQMDBCSQuery.cpp"
#else
#include "TTTDBCSQuery.h"
#include "Common.h"

#include "SOHelper.h"
#include "TStrFunc.h"
#include "debug_new.h"
#include "ReadIni.h"

SQLROWSETSIZE   iRow_CS=0;

//ZMP:467367   
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif
bool TOdbcDBCSQuery::m_bUpdateMDBFlag=true;/*判断数据库是否允许进行更新*/

int CS_DBError(RETCODE rc,HENV henv,HDBC hdbc,HSTMT hstmt,ostringstream &ostrOut)
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
        char    err_msg[SQL_MAX_MESSAGE_LENGTH] = {0};
        SWORD   msg_len;

        //RETCODE rc_err=SQLError(henv,hdbc,hstmt,(SQLCHAR*)sql_state,&native_error,
        //    (SQLCHAR*)err_msg,(SWORD)sizeof(err_msg),&msg_len);
        TTTCSFactory::GetTTInstance()->cs_SQLError(henv,hdbc,hstmt,(SQLCHAR*)sql_state,&native_error,
            (SQLCHAR*)err_msg,(SWORD)sizeof(err_msg),&msg_len);

        ostrOut<<"SQL_STATE   ="<<sql_state<<endl
            <<"NATIVE_ERROR="<<native_error<<endl
            <<"ERR_MSG     ="<<err_msg<<endl
            <<"MEG_LEN     ="<<msg_len<<endl;
    }

    return -1;

}

/*类TTTCSInterface定义部分*/
TTTCSInterface::TTTCSInterface(void)
{
    //CS模式相关
    char sItem[512] = {0};
    char *sSOPath = NULL;
    
    
    char sAppCfgName[256] = {0};
    memset(sAppCfgName, 0, sizeof(sAppCfgName));
    const char * theEnv=getenv("HOME");
    if(theEnv==NULL)
    {
        //ZMP:467367 
        strncpy(sAppCfgName,"App.config", sizeof(sAppCfgName)-1);
        sAppCfgName[sizeof(sAppCfgName)-1] = '\0';
    }
    else
    {
#if defined(__OS_WINDOWS__)
        //ZMP:467367 
        snprintf(sAppCfgName, sizeof(sAppCfgName), "%s\\etc\\App.config",theEnv);
        sAppCfgName[sizeof(sAppCfgName)-1] = '\0';
#else
        snprintf(sAppCfgName, sizeof(sAppCfgName), "%s/etc/App.config",theEnv);
#endif
    }

    TReadIni ReadIni(sAppCfgName); 
    sSOPath = ReadIni.ReadString("SO_PATH","TT_CS_LIB",sItem,NULL);
    
    m_pSoHelper = new SOHelper();
    if (sSOPath==NULL)
    {
      sSOPath = getenv(TIMESTEN_HOME_PATH_ENV);
#ifdef WIN32
        //ZMP:467367 
        snprintf(sItem, sizeof(sItem), "%s\\lib\\%s",sSOPath,TIMESTEN_CS_LIB_NAME);
        sItem[sizeof(sItem)-1] = '\0';
#else
        snprintf(sItem, sizeof(sItem), "%s/lib/%s",sSOPath,TIMESTEN_CS_LIB_NAME);
#endif
    }

    if(!m_pSoHelper->OpenLibrary(sItem))
    {
        throw TException("TOdbcDBDatabase::TOdbcDBDatabase() : OpenLibrary %s return False\n", sItem);
    }

    cs_SQLAllocEnv         = (CS_SQLAllocEnv)m_pSoHelper->LoadSymbol("SQLAllocEnv");
    cs_SQLFreeEnv          = (CS_SQLFreeEnv)m_pSoHelper->LoadSymbol("SQLFreeEnv");
    cs_SQLAllocConnect     = (CS_SQLAllocConnect)m_pSoHelper->LoadSymbol("SQLAllocConnect");
    cs_SQLConnect          = (CS_SQLConnect)m_pSoHelper->LoadSymbol("SQLConnect");
    cs_SQLSetConnectOption = (CS_SQLSetConnectOption)m_pSoHelper->LoadSymbol("SQLSetConnectOption");
    cs_SQLDisconnect       = (CS_SQLDisconnect)m_pSoHelper->LoadSymbol("SQLDisconnect");
    cs_SQLFreeConnect      = (CS_SQLFreeConnect)m_pSoHelper->LoadSymbol("SQLFreeConnect");
    cs_SQLTransact         = (CS_SQLTransact)m_pSoHelper->LoadSymbol("SQLTransact");
    cs_SQLAllocStmt        = (CS_SQLAllocStmt)m_pSoHelper->LoadSymbol("SQLAllocStmt");
    cs_SQLError            = (CS_SQLError)m_pSoHelper->LoadSymbol("SQLError");
    cs_SQLFreeStmt         = (CS_SQLFreeStmt)m_pSoHelper->LoadSymbol("SQLFreeStmt");
    cs_SQLPrepare          = (CS_SQLPrepare)m_pSoHelper->LoadSymbol("SQLPrepare");
    cs_SQLNumParams        = (CS_SQLNumParams)m_pSoHelper->LoadSymbol("SQLNumParams");

    cs_SQLDescribeParam    = (CS_SQLDescribeParam)m_pSoHelper->LoadSymbol("SQLDescribeParam");
    cs_SQLNumResultCols    = (CS_SQLNumResultCols)m_pSoHelper->LoadSymbol("SQLNumResultCols");
    cs_SQLSetStmtOption    = (CS_SQLSetStmtOption)m_pSoHelper->LoadSymbol("SQLSetStmtOption");
    cs_SQLDescribeCol      = (CS_SQLDescribeCol)m_pSoHelper->LoadSymbol("SQLDescribeCol");
    cs_SQLBindCol          = (CS_SQLBindCol)m_pSoHelper->LoadSymbol("SQLBindCol");
    cs_SQLParamOptions     = (CS_SQLParamOptions)m_pSoHelper->LoadSymbol("SQLParamOptions");
    cs_SQLExecute          = (CS_SQLExecute)m_pSoHelper->LoadSymbol("SQLExecute");
    cs_SQLFetch            = (CS_SQLFetch)m_pSoHelper->LoadSymbol("SQLFetch");
    cs_SQLExtendedFetch    = (CS_SQLExtendedFetch)m_pSoHelper->LoadSymbol("SQLExtendedFetch");
    cs_SQLRowCount         = (CS_SQLRowCount)m_pSoHelper->LoadSymbol("SQLRowCount");
    cs_SQLBindParameter    = (CS_SQLBindParameter)m_pSoHelper->LoadSymbol("SQLBindParameter");
}
TTTCSInterface::TTTCSInterface(const TTTCSInterface& oTTTCSInterface)
{
        
    m_pSoHelper = new SOHelper();
    const char* sItem = oTTTCSInterface.m_pSoHelper->GetLibName();
    if( !m_pSoHelper->OpenLibrary( sItem ) )
    {
        throw TException("TOdbcDBDatabase::TOdbcDBDatabase() : OpenLibrary %s return False\n", sItem);
    }

    cs_SQLAllocEnv         = (CS_SQLAllocEnv)m_pSoHelper->LoadSymbol("SQLAllocEnv");
    cs_SQLFreeEnv          = (CS_SQLFreeEnv)m_pSoHelper->LoadSymbol("SQLFreeEnv");
    cs_SQLAllocConnect     = (CS_SQLAllocConnect)m_pSoHelper->LoadSymbol("SQLAllocConnect");
    cs_SQLConnect          = (CS_SQLConnect)m_pSoHelper->LoadSymbol("SQLConnect");
    cs_SQLSetConnectOption = (CS_SQLSetConnectOption)m_pSoHelper->LoadSymbol("SQLSetConnectOption");
    cs_SQLDisconnect       = (CS_SQLDisconnect)m_pSoHelper->LoadSymbol("SQLDisconnect");
    cs_SQLFreeConnect      = (CS_SQLFreeConnect)m_pSoHelper->LoadSymbol("SQLFreeConnect");
    cs_SQLTransact         = (CS_SQLTransact)m_pSoHelper->LoadSymbol("SQLTransact");
    cs_SQLAllocStmt        = (CS_SQLAllocStmt)m_pSoHelper->LoadSymbol("SQLAllocStmt");
    cs_SQLError            = (CS_SQLError)m_pSoHelper->LoadSymbol("SQLError");
    cs_SQLFreeStmt         = (CS_SQLFreeStmt)m_pSoHelper->LoadSymbol("SQLFreeStmt");
    cs_SQLPrepare          = (CS_SQLPrepare)m_pSoHelper->LoadSymbol("SQLPrepare");
    cs_SQLNumParams        = (CS_SQLNumParams)m_pSoHelper->LoadSymbol("SQLNumParams");

    cs_SQLDescribeParam    = (CS_SQLDescribeParam)m_pSoHelper->LoadSymbol("SQLDescribeParam");
    cs_SQLNumResultCols    = (CS_SQLNumResultCols)m_pSoHelper->LoadSymbol("SQLNumResultCols");
    cs_SQLSetStmtOption    = (CS_SQLSetStmtOption)m_pSoHelper->LoadSymbol("SQLSetStmtOption");
    cs_SQLDescribeCol      = (CS_SQLDescribeCol)m_pSoHelper->LoadSymbol("SQLDescribeCol");
    cs_SQLBindCol          = (CS_SQLBindCol)m_pSoHelper->LoadSymbol("SQLBindCol");
    cs_SQLParamOptions     = (CS_SQLParamOptions)m_pSoHelper->LoadSymbol("SQLParamOptions");
    cs_SQLExecute          = (CS_SQLExecute)m_pSoHelper->LoadSymbol("SQLExecute");
    cs_SQLFetch            = (CS_SQLFetch)m_pSoHelper->LoadSymbol("SQLFetch");
    cs_SQLExtendedFetch    = (CS_SQLExtendedFetch)m_pSoHelper->LoadSymbol("SQLExtendedFetch");
    cs_SQLRowCount         = (CS_SQLRowCount)m_pSoHelper->LoadSymbol("SQLRowCount");
    cs_SQLBindParameter    = (CS_SQLBindParameter)m_pSoHelper->LoadSymbol("SQLBindParameter");
}

TTTCSInterface::~TTTCSInterface(void)
{
    if(m_pSoHelper)
    {
        m_pSoHelper->CloseLibrary();
        DELETE(m_pSoHelper);
    }

    cs_SQLAllocEnv         = NULL;
    cs_SQLFreeEnv          = NULL;
    cs_SQLAllocConnect     = NULL;
    cs_SQLConnect          = NULL;
    cs_SQLSetConnectOption = NULL;
    cs_SQLDisconnect       = NULL;
    cs_SQLFreeConnect      = NULL;
    cs_SQLTransact         = NULL;
    cs_SQLAllocStmt        = NULL;
    cs_SQLError            = NULL;
    cs_SQLFreeStmt         = NULL;
    cs_SQLPrepare          = NULL;
    cs_SQLNumParams        = NULL;

    cs_SQLDescribeParam    = NULL;
    cs_SQLNumResultCols    = NULL;
    cs_SQLSetStmtOption    = NULL;
    cs_SQLDescribeCol      = NULL;
    cs_SQLBindCol          = NULL;
    cs_SQLParamOptions     = NULL;
    cs_SQLExecute          = NULL;
    cs_SQLFetch            = NULL;
    cs_SQLExtendedFetch    = NULL;
    cs_SQLRowCount         = NULL;
    cs_SQLBindParameter    = NULL;
}

/**/
TTTCSInterface*  TTTCSFactory::m_pInstance = NULL;
TTTCSInterface *TTTCSFactory::GetTTInstance()
{
    if(m_pInstance == NULL)
    {
        m_pInstance = new TTTCSInterface();
        if (m_pInstance==NULL)
        {
            throw TException("TTTCSFactory::GetTTInstance() new TTTCSInterface() Failed.\n");
        }
    }

    return m_pInstance;
}

/*类TOdbcDBDatabase定义部分*/
TOdbcDBCSDatabase::TOdbcDBCSDatabase()
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
    //rc=SQLAllocEnv(&henv);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLAllocEnv(&henv);
    CheckError("TOdbcDBDatabase::SQLAllocEnv");

    m_pTOdbcDBQuery=NULL;
}

TOdbcDBCSDatabase::~TOdbcDBCSDatabase()
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
            //rc=SQLFreeEnv(henv);
            rc=TTTCSFactory::GetTTInstance()->cs_SQLFreeEnv(henv);
            CheckError("TOdbcDBDatabase::SQLFreeEnv");
            henv=NULL;
        }
    }
    catch(TOdbcDBException &oe)
    {
        cout<<"TOdbcDBDatabase::~TOdbcDBDatabase() TOdbcDBException:" << oe.GetErrMsg() << endl;
    }
    catch(...)
    {
        //throw TOdbcDBException("","TOdbcDBDatabase::~TOdbcDBDatabase() catch ... Exception.\n");
        cout<<"TOdbcDBDatabase::~TOdbcDBDatabase() catch ... Exception"<<endl;
    }
    
}

void TOdbcDBCSDatabase::SetLogin(const char *sUser,const char *sPassword,const char *sServerName)throw (TOdbcDBException)
{
    m_strUser=sUser;
    m_strPassword=sPassword;
    m_strServerName=sServerName;

    return ;   
}

bool TOdbcDBCSDatabase::Connect(bool bIsAutoCommit) throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBCSDatabase_Connect,this);
    //int iRet=0;//ZMP:305593,变量未用到

    m_iConnectFlag=0;
    m_bAutoCommitFlag=bIsAutoCommit;
    //ALLOCATE CONNECT
    //rc=SQLAllocConnect(henv,&hdbc);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLAllocConnect(henv,&hdbc);
    CheckError("TOdbcDBDatabase::SQLAllocConnect");

    //CONNECT
    //rc=SQLConnect(hdbc,(SQLCHAR*)m_strServerName.c_str(),SQL_NTS,
    //    (SQLCHAR*)m_strUser.c_str(),SQL_NTS,
    //    (SQLCHAR*)m_strPassword.c_str(),SQL_NTS);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLConnect(hdbc,(SQLCHAR*)m_strServerName.c_str(),SQL_NTS,
        (SQLCHAR*)m_strUser.c_str(),SQL_NTS,
        (SQLCHAR*)m_strPassword.c_str(),SQL_NTS);
    CheckError("TOdbcDBDatabase::SQLConnect");

    //设置不默认提交
    if(m_bAutoCommitFlag)
    {	
    	rc=TTTCSFactory::GetTTInstance()->cs_SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_ON);	
    }
    else
    {		
    	rc=TTTCSFactory::GetTTInstance()->cs_SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF);
    }
    
    CheckError("TOdbcDBDatabase::SQLSetConnectOption");

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
    PS_END(BF_TOdbcDBCSDatabase_Connect);
    return true;
}

bool TOdbcDBCSDatabase::Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit) throw (TOdbcDBException)
{
    SetLogin(sUser,sPassword,sServerName);

    return Connect(bIsAutoCommit);
}

int TOdbcDBCSDatabase::Disconnect()  throw(TOdbcDBException)
{
    int iRet=0;
    PS_BEGIN(BF_TOdbcDBCSDatabase_Disconnect,this);
    if(hdbc!=NULL)
    {
        if(m_iConnectFlag==1)
        {
            //默认回退
            Rollback();

            //DISCONNECT
            //rc=SQLDisconnect(hdbc);
            rc=TTTCSFactory::GetTTInstance()->cs_SQLDisconnect(hdbc);
            CheckError("TOdbcDBDatabase::SQLDisconnect");
            m_iConnectFlag=0;
        }

        //FREE CONNECT
        //rc=SQLFreeConnect(hdbc);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLFreeConnect(hdbc);
        CheckError("TOdbcDBDatabase::SQLFreeConnect");

        hdbc=NULL;
    }
    PS_END(BF_TOdbcDBCSDatabase_Disconnect);
    return iRet;
}

//数据库开启事务
void TOdbcDBCSDatabase::TransBegin() 
{
    return;
}


//数据库提交
void TOdbcDBCSDatabase::Commit() 
{
    PS_BEGIN(BF_TOdbcDBCSDatabase_Commit,this);
	if(!m_bAutoCommitFlag)
	{
    	//rc=SQLTransact(henv,hdbc,SQL_COMMIT);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLTransact(henv,hdbc,SQL_COMMIT);
    	CheckError("TOdbcDBDatabase::SQLTransact");
  	}
  	PS_END(BF_TOdbcDBCSDatabase_Commit);
    return;
}

//数据库回滚
void TOdbcDBCSDatabase::Rollback() 
{
    PS_BEGIN(BF_TOdbcDBCSDatabase_Rollback,this);
    //rc=SQLTransact(henv,hdbc,SQL_ROLLBACK);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLTransact(henv,hdbc,SQL_ROLLBACK);
    CheckError("TOdbcDBDatabase::SQLTransact");
    PS_END(BF_TOdbcDBCSDatabase_Rollback);
    return;
}

//获取数据连接信息
void TOdbcDBCSDatabase::GetDataInfo(HENV &henv,HDBC &hdbc)
{
    henv=this->henv;
    hdbc=this->hdbc;
    return;
}


bool TOdbcDBCSDatabase::IsConnect()  
{
    bool bConnectFlag=false;
    PS_BEGIN(BF_TOdbcDBCSDatabase_IsConnect,this);
    if(m_iConnectFlag==1)
    {
        bConnectFlag=true;

        try
        {
            if(m_pTOdbcDBQuery==NULL)
            {
                m_pTOdbcDBQuery=new TOdbcDBCSQuery(this);
            }

            m_pTOdbcDBQuery->Close();
            m_pTOdbcDBQuery->SetSQL("SELECT 1 FROM DUAL");
            m_pTOdbcDBQuery->Open();
            /*if(m_pTOdbcDBQuery->Next())
            {
            }*/
            m_pTOdbcDBQuery->Close();
        }
        catch(TOdbcDBException &oe)
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
            cout<<"TOdbcDBDatabase::IsConnect() TOdbcDBException:" << oe.GetErrMsg() << endl;
        }
        catch(...)
        {
            PS_END(BF_TOdbcDBCSDatabase_IsConnect);
            throw TOdbcDBException("","TOdbcDBDatabase::IsConnect() catch ... Exception.\n");
        }
    }
    PS_END(BF_TOdbcDBCSDatabase_IsConnect);
    return bConnectFlag;
}

TOdbcDBCSQuery * TOdbcDBCSDatabase::CreateDBQuery() throw (TOdbcDBException)
{
    return new TOdbcDBCSQuery(this);
}


//private函数
void TOdbcDBCSDatabase::CheckError(const char* sSql) throw (TOdbcDBException)
{
    static ostringstream ostrErr;
    int iRet=0;
    ostrErr.str("");

    if((rc!=SQL_SUCCESS)
        &&(rc!=SQL_SUCCESS_WITH_INFO)
        )
    {
        iRet=CS_DBError(rc,henv,hdbc,SQL_NULL_HSTMT,ostrErr);
    }

    if(iRet!=0)
    {
        throw TOdbcDBException(sSql,"Application Error Message=%s",ostrErr.str().c_str());
    }
}

/*类TOdbcDBField定义部分*/
TOdbcDBCSField::TOdbcDBCSField()
{
    sData=NULL;
    pslValueLen=NULL;
    iMaxRowsCount=0;
    ClearInfo();
}

TOdbcDBCSField::~TOdbcDBCSField()
{
    ClearInfo();
}

bool TOdbcDBCSField::isNULL()
{
    if(pslValueLen[m_pTOdbcDBQuery->m_iRowIndex]==-1)
    {
        return true;
    }
    return false;
}

char* TOdbcDBCSField::AsString() throw (TOdbcDBException)
{
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBField::AsString","Application Error Message=Data set not opened!");
    }
    /*PS_BEGIN(BF_TOdbcDBCSField_AsString,this);*/
    memset(sDataValue,0,sizeof(sDataValue));
    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
        {
            /*PS_END(BF_TOdbcDBCSField_AsString);*/
            return sData;
            //break;//ZMP:305593,无用，去掉
        }
    case SQL_INTEGER:
        {
            SDWORD value=*((SDWORD*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            //ZMP:467367 
            snprintf(sDataValue, sizeof(sDataValue), "%d",value);
            sDataValue[sizeof(sDataValue)-1] = '\0';
            break;
        }
    case SQL_SMALLINT:
        {
            SWORD value=*((SWORD*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            //ZMP:467367 
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
            //ZMP:467367 
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
            /*PS_END(BF_TOdbcDBCSField_AsString);*/
            return AsDateTimeString();
            //break;//ZMP:305593,无用，去掉
        }
    default:
        {
            /*PS_END(BF_TOdbcDBCSField_AsString);*/
            throw TOdbcDBException("TOdbcDBField::AsString","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
        }
    }
    /*PS_END(BF_TOdbcDBCSField_AsString);*/
    return sDataValue;
}

int TOdbcDBCSField::DataType(void)
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
            throw TOdbcDBException("TOdbcDBCSField::AsString","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
        }
    }
    return -1;
}

double TOdbcDBCSField::AsFloat() throw (TOdbcDBException)
{
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBCSField::AsFloat","Application Error Message=Data set not opened!");
    }

    /*PS_BEGIN(BF_TOdbcDBCSField_AsFloat,this);*/
    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
        {
            /*PS_END(BF_TOdbcDBCSField_AsFloat);*/
            return atof(sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            //break;//ZMP:305593,无用，去掉
        }
    case SQL_INTEGER:
        {
            /*PS_END(BF_TOdbcDBCSField_AsFloat);*/
            return *((SDWORD*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            //break;//ZMP:305593,无用，去掉
        }
    case SQL_SMALLINT:
        {
            /*PS_END(BF_TOdbcDBCSField_AsFloat);*/
            return *((SWORD*)sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));
            //break;//ZMP:305593,无用，去掉
        }
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_BIGINT:
        {
            /*PS_END(BF_TOdbcDBCSField_AsFloat);*/
            return *(static_cast<double *>((void *)sData)+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex));//ZMP:305593
            //break;//ZMP:305593,无用，去掉
        }
    case SQL_TIMESTAMP:
    case SQL_TIME:
    case SQL_DATE:
        {
            /*PS_END(BF_TOdbcDBCSField_AsFloat);*/
            /*不能转换成DOUBLE类型*/
            throw TOdbcDBException("TOdbcDBCSField::AsFloat","Application Error Message=SQL_DATA_TYPE(%ld) can not convert!",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
        }
    default:
        {
            /*PS_END(BF_TOdbcDBCSField_AsFloat);*/
            throw TOdbcDBException("TOdbcDBCSField::AsFloat","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
        }
    }
    dDataValue=0;
    /*PS_END(BF_TOdbcDBCSField_AsFloat);*/
    return dDataValue;
}

long long TOdbcDBCSField::AsInteger() throw (TOdbcDBException)
{
    /*PS_BEGIN(BF_TOdbcDBCSField_AsInteger,this);*/
    llDataValue=(long long)this->AsFloat();
    /*PS_END(BF_TOdbcDBCSField_AsInteger);*/
    return llDataValue;
}

void TOdbcDBCSField::AsDateTime(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException)
{
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBField::AsDateTime","Application Error Message=Data set not opened!");
    }
    /*PS_BEGIN(BF_TOdbcDBCSField_AsDateTime,this);*/
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
            /*PS_END(BF_TOdbcDBCSField_AsDateTime);*/
            throw TOdbcDBException("TOdbcDBField::AsDateTime","Application Error Message=SQL_DATA_TYPE(%ld) can not convert!",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
        }
    case SQL_TIMESTAMP:
    case SQL_TIME:
    case SQL_DATE:
        {
            bool bNumber=false;
            int iSegIndex=0;
            char *sCur=sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);
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
    default:
        {
            /*PS_END(BF_TOdbcDBCSField_AsDateTime);*/
            throw TOdbcDBException("TOdbcDBField::AsDateTime","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
        }
    }
    /*PS_END(BF_TOdbcDBCSField_AsDateTime);*/
    return;
}

//转换时间格式为YYYYMMDDHHMISS
char* TOdbcDBCSField::AsDateTimeString() throw (TOdbcDBException)
{
    /*PS_BEGIN(BF_TOdbcDBCSField_AsDateTimeString,this);*/
    int iYear=0;
    int iMonth=0;
    int iDay=0;
    int iHour=0;
    int iMinute=0;
    int iSecond=0;

    this->AsDateTime(iYear,iMonth,iDay,iHour,iMinute,iSecond);
    memset(sDateTime,0,sizeof(sDateTime));
    //ZMP:467367 
    snprintf(sDateTime, sizeof(sDateTime), "%04d%02d%02d%02d%02d%02d",iYear,iMonth,iDay,iHour,iMinute,iSecond);
    sDateTime[sizeof(sDateTime)-1] = '\0';
    /*PS_END(BF_TOdbcDBCSField_AsDateTimeString);*/
    return sDateTime;
}

char* TOdbcDBCSField::AsBlobBuffer(int &iBufferLen)
{
    char *sBlobBuffer=NULL;

    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBField::AsBlobBuffer","Application Error Message=Data set not opened!");
    }
    /*PS_BEGIN(BF_TOdbcDBCSField_AsBlobBuffer,this);*/
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
            /*PS_END(BF_TOdbcDBCSField_AsBlobBuffer);*/
            throw TOdbcDBException("TOdbcDBField::AsDateTime","Application Error Message=SQL_DATA_TYPE(%ld) can not convert!",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
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
            /*PS_END(BF_TOdbcDBCSField_AsBlobBuffer);*/
            throw TOdbcDBException("TOdbcDBField::AsDateTime","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
        }
    }
    /*PS_END(BF_TOdbcDBCSField_AsBlobBuffer);*/
    return sBlobBuffer;
}

const char * TOdbcDBCSField::AsName()
{
    return strFieldName.c_str();
}


//private函数
void TOdbcDBCSField::ClearInfo()
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
    
    DELETE_A(pslValueLen); //ZMP_591266
    
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
TOdbcDBCSParam::TOdbcDBCSParam()
{
    pData=NULL;
    allValueLen=NULL;
    ClearInfo();
}

TOdbcDBCSParam::~TOdbcDBCSParam()
{
    ClearInfo();
}

//private函数
void TOdbcDBCSParam::ClearInfo()
{
    m_pTOdbcDBQuery=NULL;
    strParamName="";
    iIndex=-1;
    ssiSqlDataType=0;
    sulPrecision=0;
    ssiScale=0;
    ssiNullable=0;

    //DELETE(pData);
    //ZMP:477242
    if(pData != NULL)
    {
        delete [](char*)pData;
        pData = NULL;
    }
    slDataLen=-1;
    ssiDataType=0;
    iElementCount=-1;
    DELETE_A(allValueLen); //ZMP_591266
    
    bSetValue=false;
    return;
}

/*类TOdbcDBCSQuery定义部分*/
TOdbcDBCSQuery::TOdbcDBCSQuery(TOdbcDBCSDatabase *pTOdbcDBDatabase)
{
    henv=NULL;
    hdbc=NULL;
    hstmt=NULL;
    rc=0;

    if(pTOdbcDBDatabase==NULL)
    {
        throw TOdbcDBException("TOdbcDBCSQuery::TOdbcDBCSQuery","Application Error Message=Parameter Error!");
    }

    if(pTOdbcDBDatabase->m_iConnectFlag!=1)
    {
        throw TOdbcDBException("TOdbcDBCSQuery::TOdbcDBCSQuery","Application Error Message=Database not connected!");
    }

    pTOdbcDBDatabase->GetDataInfo(henv,hdbc);
    m_pTOdbcDBDatabase=pTOdbcDBDatabase;

    //ALLOCATE STMT
    //rc=SQLAllocStmt(hdbc,&hstmt);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLAllocStmt(hdbc,&hstmt);
    CheckError("TOdbcDBCSQuery::SQLAllocStmt");

    m_iParamCount=0;
    m_pTOdbcDBParamArray=new TOdbcDBCSParam[MAX_COLUMN_COUNT];
    if(m_pTOdbcDBParamArray==NULL)
    {
        throw TOdbcDBException("TOdbcDBCSQuery::TOdbcDBCSQuery","Application Error Message=Malloc error!");
    }

    m_iFieldCount=0;
    m_pTOdbcDBFieldArray=new TOdbcDBCSField[MAX_COLUMN_COUNT];
    if(m_pTOdbcDBFieldArray==NULL)
    {
        throw TOdbcDBException("TOdbcDBCSQuery::TOdbcDBCSQuery","Application Error Message=Malloc error!");
    }

    m_iQueryState=QUERY_BEFORE_OPEN;
    aRowsStatus=NULL;
    m_iRowIndex=0;
    m_iPreFetchRows=0;
    m_iFetchedRows=0;    

    m_sSql = NULL; 
////////////////////////////////////////////////////////////////////////////////
//133994 begin
    m_iPrevprefetchRows = 0;
//133994 end
////////////////////////////////////////////////////////////////////////////////

}

TOdbcDBCSQuery::~TOdbcDBCSQuery()
{
    if(hstmt!=NULL && (NULL != m_pTOdbcDBDatabase->hdbc) )
    {
        //关闭执行的SQL
        //rc=SQLFreeStmt(hstmt,SQL_CLOSE);
        //CheckError(m_sSql);
        Close();

        //FREE STMT
        //rc=SQLFreeStmt(hstmt,SQL_DROP);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLFreeStmt(hstmt,SQL_DROP);
        CheckError(m_sSql);
        hstmt=NULL;

////////////////////////////////////////////////////////////////////////////////
//133994 begin
        m_iPrevprefetchRows = 0;
//133994 end
////////////////////////////////////////////////////////////////////////////////

    }
    henv=NULL;
    hdbc=NULL;

    m_pTOdbcDBDatabase=NULL;
    DELETE_A(m_sSql);
    DELETE_A(m_pTOdbcDBFieldArray);
    DELETE_A(m_pTOdbcDBParamArray);
    DELETE_A(aRowsStatus);
}

void TOdbcDBCSQuery::Close()
{
    PS_BEGIN(BF_TOdbcDBCSQuery_Close,this);
    //rc=SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    CheckError(m_sSql);

    //rc=SQLFreeStmt(hstmt,SQL_UNBIND);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLFreeStmt(hstmt,SQL_UNBIND);
    CheckError(m_sSql);

    //关闭执行的SQL
    //rc=SQLFreeStmt(hstmt,SQL_CLOSE);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLFreeStmt(hstmt,SQL_CLOSE);
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
    //memset(m_sSql,0,sizeof(m_sSql));
    DELETE_A(m_sSql);
    m_iQueryState=QUERY_BEFORE_OPEN;
    m_lAffectRows=0;
    m_iPreFetchRows=0;

    m_iRowIndex=0;
    m_iFetchedRows=0;
    PS_END(BF_TOdbcDBCSQuery_Close);
    return;
}

void TOdbcDBCSQuery::CloseSQL()
{
	//rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    rc = TTTCSFactory::GetTTInstance()->cs_SQLFreeStmt(hstmt, SQL_CLOSE);
	CheckError(m_sSql);
	 
	m_iQueryState=QUERY_BEFORE_OPEN;
    m_iPreFetchRows=0;
    m_iRowIndex=0;
    m_iFetchedRows=0;
}

void TOdbcDBCSQuery::SetSQL(const char *sSqlStatement,int iPreFetchRows) throw (TOdbcDBException)
{
    char sFieldName[MAX_COLUMN_NAME]={0}; //356853 for cppcheck
    int iNameLen=0;
    iPreFetchRows = 0;
    if(sSqlStatement==NULL)
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetSQL","Application Error Message=Parameter Error!");
    }
/*
    if(strlen(sSqlStatement)>=sizeof(m_sSql))
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetSQL","Application Error Message=Length of sql is beyond limit(%ld)",sizeof(m_sSql));
    }
*/
    DELETE_A(m_sSql);
    size_t iLen=strlen(sSqlStatement);
    m_sSql=new char[iLen+1];
    if(m_sSql==NULL)
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetSQL","no more memory can be allocate!");
    }
    strncpy(m_sSql,sSqlStatement,iLen);
    m_sSql[iLen]='\0';

    //需要替换:参数格式到?
    ParseSql(m_sSql);

    //PREPARE
    //rc=SQLPrepare(hstmt,(SQLCHAR *)m_sSql,sizeof(m_sSql));
    //rc=TTTCSFactory::GetTTInstance()->cs_SQLPrepare(hstmt,(SQLCHAR *)m_sSql,sizeof(m_sSql));
    rc=TTTCSFactory::GetTTInstance()->cs_SQLPrepare(hstmt,(SQLCHAR *)m_sSql,SQL_NTS);
    CheckError(m_sSql);

    //NUMBER PARAM;
    int iParamCount=0;
    SWORD FAR siParamCount=0;
    //rc=SQLNumParams(hstmt,(SWORD FAR*)&siParamCount);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLNumParams(hstmt,(SWORD FAR*)&siParamCount);
    iParamCount=siParamCount;
    CheckError(m_sSql);

    if(iParamCount!=m_iParamCount)
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetSQL","Application Error Message=ParseSql() Error!");
    }
    //获取参数信息
    for(int iParamIndex=1;iParamIndex<=m_iParamCount;iParamIndex++)
    {
        //rc=SQLDescribeParam(hstmt,iParamIndex,
        //    (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType),
        //    (SQLULEN*)&(m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision),
        //    (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiScale),
        //    (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable));
        rc=TTTCSFactory::GetTTInstance()->cs_SQLDescribeParam(hstmt,(unsigned short)iParamIndex,
            (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiSqlDataType),
            (SQLULEN*)&(m_pTOdbcDBParamArray[iParamIndex-1].sulPrecision),
            (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiScale),
            (SQLSMALLINT*)&(m_pTOdbcDBParamArray[iParamIndex-1].ssiNullable));
        CheckError(m_sSql);
    }

    //NUMBER FIELD;
    int iFieldCount=0;
    SWORD FAR siFieldCount=0;
    //rc=SQLNumResultCols(hstmt,(SWORD FAR*)&siFieldCount);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLNumResultCols(hstmt,(SWORD FAR*)&siFieldCount);
    iFieldCount=siFieldCount;
    CheckError(m_sSql);

    if(iFieldCount>MAX_COLUMN_COUNT)
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetSQL","Application Error Message=Count of field beyond limit(%ld)",MAX_COLUMN_COUNT);
    }
    m_iFieldCount=iFieldCount;
    
    //new position
    if(iPreFetchRows>1)
    {
        //SQLSetStmtOption(hstmt,SQL_CONCURRENCY,SQL_CONCUR_READ_ONLY);
        //SQLSetStmtOption(hstmt,SQL_CURSOR_TYPE,SQL_CURSOR_KEYSET_DRIVEN);
        //SQLSetStmtOption(hstmt,SQL_ROWSET_SIZE,iPreFetchRows);
        TTTCSFactory::GetTTInstance()->cs_SQLSetStmtOption(hstmt,SQL_CONCURRENCY,SQL_CONCUR_READ_ONLY);
        TTTCSFactory::GetTTInstance()->cs_SQLSetStmtOption(hstmt,SQL_CURSOR_TYPE,SQL_CURSOR_KEYSET_DRIVEN);
        TTTCSFactory::GetTTInstance()->cs_SQLSetStmtOption(hstmt,SQL_ROWSET_SIZE,(SQLULEN)iPreFetchRows);
    }
    
    for(int iFieldIndex=1;iFieldIndex<=m_iFieldCount;iFieldIndex++)
    {
        iNameLen=0;

        //获取各个选择列信息
        //rc=SQLDescribeCol(hstmt,iFieldIndex,(SQLCHAR*)sFieldName,MAX_COLUMN_NAME,
        //    (SQLSMALLINT *)&iNameLen,
        //    (SQLSMALLINT *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].ssiSqlDataType),
        //    (SQLULEN *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].sulPrecision),
        //    (SQLSMALLINT *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].ssiScale),
        //    (SQLSMALLINT *)&(m_pTOdbcDBFieldArray[iFieldIndex-1].ssiNullable));
        rc=TTTCSFactory::GetTTInstance()->cs_SQLDescribeCol(hstmt,(unsigned short)iFieldIndex,(SQLCHAR*)sFieldName,MAX_COLUMN_NAME,
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
        //rc=SQLBindCol(hstmt,iFieldIndex,
        //    (SQLSMALLINT)m_pTOdbcDBFieldArray[iFieldIndex-1].ssiDataType,
        //    (SQLPOINTER)m_pTOdbcDBFieldArray[iFieldIndex-1].sData,
        //    (SQLLEN)m_pTOdbcDBFieldArray[iFieldIndex-1].slDataLen,
        //    (SQLLEN*)m_pTOdbcDBFieldArray[iFieldIndex-1].pslValueLen);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLBindCol(hstmt,(unsigned short)iFieldIndex,
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

void TOdbcDBCSQuery::Open(int prefetchRows) throw (TOdbcDBException)
{
    m_lAffectRows=0;
    if((m_iQueryState!=QUERY_BEFORE_OPEN)
        &&(m_iQueryState!=QUERY_FETCH_EOF)
        &&(m_iQueryState!=QUERY_FETCH_BOF)
        &&(m_iQueryState!=QUERY_AFTER_OPEN)
        )
    {//判断执行状态
        //应用修改,不可能到达此处
        throw TOdbcDBException("TOdbcDBCSQuery::Open","Application Error Message=Data set not closed!");
    }
    PS_BEGIN(BF_TOdbcDBCSQuery_Open,this);
    if( prefetchRows < 0 )
      prefetchRows = 0;
    else if( prefetchRows > 128 )
      prefetchRows = 20;

////////////////////////////////////////////////////////////////////////////////
//133994 begin
//    if( prefetchRows < 0 || prefetchRows > 128 )
//    {
//      throw TOdbcDBException("TOdbcDBCSQuery::Open","Application Error Message=TT_PREFETCH_COUNT parameter is error!");
//    }
//    else
//    {
      if( m_iPrevprefetchRows != prefetchRows )
      {
        TTTCSFactory::GetTTInstance()->cs_SQLSetStmtOption(hstmt,TT_PREFETCH_COUNT,(SQLULEN)prefetchRows);
        m_iPrevprefetchRows = prefetchRows;
      }
//    }
//133994 end
////////////////////////////////////////////////////////////////////////////////

    if(m_iQueryState!=QUERY_BEFORE_OPEN)
    {//不是CLOSE后调用
        //rc=SQLFreeStmt(hstmt,SQL_CLOSE);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLFreeStmt(hstmt,SQL_CLOSE);
        CheckError(m_sSql);
    }

    if(m_iFieldCount<=0)
    {
        PS_END(BF_TOdbcDBCSQuery_Open);
        throw TOdbcDBException("TOdbcDBCSQuery::Open","Application Error Message=SQL is not for SELECT!");
    }

    if(m_iParamCount>0)
    {//存在参数情况下,校验
        for(int iParamIndex=1;iParamIndex<=m_iParamCount;iParamIndex++)
        {
            if(m_pTOdbcDBParamArray[iParamIndex-1].iElementCount<=0)
            {
                PS_END(BF_TOdbcDBCSQuery_Open);
                throw TOdbcDBException("TOdbcDBCSQuery::Open","Application Error Message=Param(%d) not be set!",iParamIndex);
            }
        }//end of for(遍历所有参数,校验是否已经赋值)
    }

    //回复设置参数数组元素个数
    iRow_CS=0;
    //rc=SQLParamOptions(hstmt,1,&iRow);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLParamOptions(hstmt,1,&iRow_CS);
    CheckError(m_sSql);
    
    //执行SQL
    //rc=SQLExecute(hstmt);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLExecute(hstmt);
    CheckError(m_sSql);

    m_iQueryState=QUERY_AFTER_OPEN;
    PS_END(BF_TOdbcDBCSQuery_Open);
    return;
}

bool TOdbcDBCSQuery::Next() throw (TOdbcDBException)
{
    if((m_iQueryState!=QUERY_AFTER_OPEN)
        &&(m_iQueryState!=QUERY_FETCH_BOF))
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBCSQuery::Next","Application Error Message=Data set not opened!");
    }

    m_iQueryState=QUERY_FETCH_BOF;
    PS_BEGIN(BF_TOdbcDBCSQuery_Next,this);
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
            memset(m_pTOdbcDBFieldArray[iFieldIndex-1].pslValueLen,0,(size_t)m_pTOdbcDBFieldArray[iFieldIndex-1].iRowsCount*sizeof(SQLLEN));
        }

        m_iRowIndex=0;
        m_iFetchedRows=0;
        if(m_iPreFetchRows<=1)
        {
            //rc=SQLFetch(hstmt);
            rc=TTTCSFactory::GetTTInstance()->cs_SQLFetch(hstmt);
            if((rc==SQL_NO_DATA_FOUND)
                //||(rc==SQL_NO_DATA)TimesTen for unix无
                )
            {//未找到数据
                m_iQueryState=QUERY_FETCH_EOF;
                PS_END(BF_TOdbcDBCSQuery_Next);
                return false;
            }
            CheckError(m_sSql);
            m_iFetchedRows=1;    
        }
        else
        {
            //rc=SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,m_iPreFetchRows,&m_iFetchedRows,aRowsStatus);
            rc=TTTCSFactory::GetTTInstance()->cs_SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,m_iPreFetchRows,&m_iFetchedRows,aRowsStatus);
            if((rc==SQL_NO_DATA_FOUND)
                //||(rc==SQL_NO_DATA)TimesTen for unix无
                )
            {//未找到数据
                m_iQueryState=QUERY_FETCH_EOF;
                PS_END(BF_TOdbcDBCSQuery_Next);
                return false;
            }
            CheckError(m_sSql);
        }

    }
    m_lAffectRows=m_lAffectRows+m_iFetchedRows;
    PS_END(BF_TOdbcDBCSQuery_Next);
    return true;
}

bool TOdbcDBCSQuery::Execute(int iExecuteRows)throw (TOdbcDBException)
{
    /*begin zmp 841469 */
    /*判断数据库是否允许进行更新*/
    if(!TOdbcDBCSQuery::m_bUpdateMDBFlag)
    {
        throw TOdbcDBException("TOdbcDBQuery::Execute","Prohibited execution");
    }
    /*end zmp 841469 */
    
    int iMaxParamElementCount=0;

    m_lAffectRows=0;
    if(m_iFieldCount>0)
    {
        throw TOdbcDBException("TOdbcDBCSQuery::Execute","Application Error Message=SQL can not Execute!");
    }

    if(m_iQueryState!=QUERY_BEFORE_OPEN)
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBCSQuery::Execute","Application Error Message=Called with error env!");
    }
    PS_BEGIN(BF_TOdbcDBCSQuery_Execute,this);
    if(m_iParamCount>0)
    {//存在参数情况下,校验
        iMaxParamElementCount=m_pTOdbcDBParamArray[0].iElementCount;
        for(int iParamIndex=1;iParamIndex<=m_iParamCount;iParamIndex++)
        {
            if(m_pTOdbcDBParamArray[iParamIndex-1].iElementCount<=0)
            {
                PS_END(BF_TOdbcDBCSQuery_Execute);
                throw TOdbcDBException("TOdbcDBCSQuery::Execute","Application Error Message=Param(%d) not be set!",iParamIndex);
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
        iRow_CS=0;
        //rc=SQLParamOptions(hstmt,iMaxParamElementCount,&iRow);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLParamOptions(hstmt,(SQLUINTEGER)iMaxParamElementCount,&iRow_CS);
        CheckError(m_sSql);
    }


    //Execute
    //rc=SQLExecute(hstmt);
    rc=TTTCSFactory::GetTTInstance()->cs_SQLExecute(hstmt);
    CheckError(m_sSql);

    m_lAffectRows=0;
    rc=TTTCSFactory::GetTTInstance()->cs_SQLRowCount(hstmt,(SQLLEN*)&m_lAffectRows);
    CheckError(m_sSql);
    PS_END(BF_TOdbcDBCSQuery_Execute);
    return true;
}

bool TOdbcDBCSQuery::TransBegin()
{
    m_pTOdbcDBDatabase->TransBegin();
    return true;
}

bool TOdbcDBCSQuery::Commit()
{
    m_pTOdbcDBDatabase->Commit();
    return true;
}

bool TOdbcDBCSQuery::Rollback()
{
    m_pTOdbcDBDatabase->Rollback();
    return true;
}

int TOdbcDBCSQuery::RowsAffected()
{
    return (int)m_lAffectRows;//ZMP:477242
}

int TOdbcDBCSQuery::FieldCount()
{
    return m_iFieldCount;
}

TOdbcDBCSField& TOdbcDBCSQuery::Field(int iIndex) throw (TOdbcDBException)
{
    if((iIndex<0)
        ||(iIndex>=m_iFieldCount)
        )
    {
        throw TOdbcDBException("TOdbcDBCSQuery::Field","Application Error Message=Parameter Error!");
    }
    return m_pTOdbcDBFieldArray[iIndex];
}

TOdbcDBCSField& TOdbcDBCSQuery::Field(const char *sFieldName) throw (TOdbcDBException)
{
    if((sFieldName==NULL)
        ||(strlen(sFieldName)==0)
        ||(m_iFieldCount<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBCSQuery::Field","Application Error Message=Parameter Error!");
    }

    for(int iIndex=0;iIndex<m_iFieldCount;iIndex++)
    {
        if(FUNC_STR::StrCmpNoCase(sFieldName,m_pTOdbcDBFieldArray[iIndex].strFieldName.c_str())==0)
        {//名字无大小区分
            return m_pTOdbcDBFieldArray[iIndex];
        }
    }

    throw TOdbcDBException("TOdbcDBCSQuery::Field","Application Error Message=Field(%s) not exist!",sFieldName);
}

/*begin zmp 841469 */
void TOdbcDBCSQuery::GetValue(void *pStruct,int* Column)throw (TOdbcDBException)
{
    throw TOdbcDBException("TOdbcDBQuery::GetValue","Not Support!");
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
    }*/
}
void TOdbcDBCSQuery::SetUpdateMDBFlag(bool flag)throw (TOdbcDBException)
{
    TOdbcDBCSQuery::m_bUpdateMDBFlag=flag;
}
bool TOdbcDBCSQuery::GetUpdateMDBFlag()throw (TOdbcDBException)
{
    return TOdbcDBCSQuery::m_bUpdateMDBFlag;
}
/*end zmp 841469 */

int TOdbcDBCSQuery::ParamCount()
{
    return m_iParamCount;
}

TOdbcDBCSParam& TOdbcDBCSQuery::Param(int iIndex)
{
    if((iIndex<0)
        ||(iIndex>=m_iParamCount)
        )
    {
        throw TOdbcDBException("TOdbcDBCSQuery::Param","Application Error Message=Parameter Error!");
    }
    return m_pTOdbcDBParamArray[iIndex];
}

TOdbcDBCSParam& TOdbcDBCSQuery::Param(const char*sParamName)
{
    if((sParamName==NULL)
        ||(strlen(sParamName)==0)
        ||(m_iParamCount<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBCSQuery::Param","Application Error Message=Parameter Error!");
    }

    return *(ParamByCondition(sParamName));
}

//设置参数值
void TOdbcDBCSQuery::SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBCSParam,sParamValue);
    return;
}

void TOdbcDBCSQuery::SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
    SetParameter(sParamName,(long)iParamValue);
    return;
}

void TOdbcDBCSQuery::SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBCSParam,lParamValue);
    return;
}

void TOdbcDBCSQuery::SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBCSParam,dParamValue);
    return;
}

void TOdbcDBCSQuery::SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
#ifdef _WIN32
    //ZMP:467367 
    snprintf(m_sLongLong, sizeof(m_sLongLong), "%I64d", llParamValue);
    m_sLongLong[sizeof(m_sLongLong)-1] = '\0';
#else
    snprintf(m_sLongLong, sizeof(m_sLongLong), "%lld", llParamValue);
#endif
    SetParameter(sParamName,m_sLongLong);
    return;
}

void TOdbcDBCSQuery::SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=ParamByCondition(sParamName);
    SetParameter(pTOdbcDBCSParam,sParamValue,iBufferLen);
    return;
}

void TOdbcDBCSQuery::SetParameterNULL(const char *sParamName)throw (TOdbcDBException)
{
    SetParameter(sParamName, (char*)NULL);
    return;
}

void TOdbcDBCSQuery::SetParameter(int iParamIndex,const char* sParamValue)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBCSParam,sParamValue);
    return;
}

void TOdbcDBCSQuery::SetParameter(int iParamIndex,int iParamValue)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBCSParam,(long)iParamValue);
    return;
}
void TOdbcDBCSQuery::SetParameter(int iParamIndex,long lParamValue)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBCSParam,lParamValue);
    return;
}
void TOdbcDBCSQuery::SetParameter(int iParamIndex,double dParamValue)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBCSParam,dParamValue);
    return;
}
void TOdbcDBCSQuery::SetParameter(int iParamIndex,long long llParamValue)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBCSParam,(double)llParamValue);
    return;
}
void TOdbcDBCSQuery::SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBCSParam,sParamValue,iBufferLen);
    return;
}

void TOdbcDBCSQuery::SetParameterNULL(int iParamIndex)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=&Param(iParamIndex);
    SetParameter(pTOdbcDBCSParam,(char*)NULL);
    return;
}

//设置数组参数值
void TOdbcDBCSQuery::SetParamArray(const char *sParamName,char **asParamValue,int iInterval,
                                 int iElementSize,int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if((asParamValue==NULL)
        ||(iElementSize<=0)
        ||(iArraySize<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    TOdbcDBCSParam *pTOdbcDBCSParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;

    if((pTOdbcDBCSParam->pData!=NULL)
        &&(pTOdbcDBCSParam->slDataLen<iElementSize*iElementCount)
        )
    {
        //DELETE(pTOdbcDBCSParam->pData);
        //ZMP:477242
        if(pTOdbcDBCSParam->pData != NULL)
        {
            delete [](char*)pTOdbcDBCSParam->pData;
            pTOdbcDBCSParam->pData = NULL;
        }
        pTOdbcDBCSParam->bSetValue=false;
    }
    
    if((pTOdbcDBCSParam->allValueLen!=NULL)
        &&(pTOdbcDBCSParam->iElementCount<iElementCount)
        )
    {
        DELETE_A(pTOdbcDBCSParam->allValueLen);//ZMP_591266
        pTOdbcDBCSParam->bSetValue=false;
    }

    if(pTOdbcDBCSParam->bSetValue==false)
    {
        if(pTOdbcDBCSParam->pData==NULL)
        {
            pTOdbcDBCSParam->pData=new char[iElementSize*iElementCount];
            pTOdbcDBCSParam->slDataLen=iElementSize*iElementCount;
        }
        
        memcpy(pTOdbcDBCSParam->pData,asParamValue,(size_t)iArraySize);
        pTOdbcDBCSParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBCSParam->llElementSize=iElementSize;
           
        if(pTOdbcDBCSParam->allValueLen==NULL)
        {
            pTOdbcDBCSParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBCSParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        }
    
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBCSParam->allValueLen)[iIndex]=SQL_NTS;
        }
    
        //rc=SQLBindParameter(hstmt,pTOdbcDBCSParam->iIndex+1,SQL_PARAM_INPUT,
        //    (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
        //    (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
        //    (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->llElementSize,
        //    (SQLLEN*)pTOdbcDBCSParam->allValueLen);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBCSParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->llElementSize,
            (SQLLEN*)pTOdbcDBCSParam->allValueLen);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBCSParam->slDataLen=iElementSize*iElementCount;
        memcpy(pTOdbcDBCSParam->pData,asParamValue,(size_t)iArraySize);
        pTOdbcDBCSParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBCSParam->llElementSize=iElementSize;
        memset(pTOdbcDBCSParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBCSParam->allValueLen)[iIndex]=SQL_NTS;
        }
    }

    pTOdbcDBCSParam->iElementCount=iElementCount;
    pTOdbcDBCSParam->bSetValue=true;
    return;
}

void TOdbcDBCSQuery::SetParamArray(const char *sParamName,int *aiParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if((aiParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    int iElementSize=sizeof(int);
    TOdbcDBCSParam *pTOdbcDBCSParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;

    if((pTOdbcDBCSParam->pData!=NULL)
        &&(pTOdbcDBCSParam->slDataLen<iElementSize*iElementCount)
        )
    {
        //DELETE(pTOdbcDBCSParam->pData); 
        //ZMP:477242
        if(pTOdbcDBCSParam->pData != NULL)
        {
            delete [](char*)pTOdbcDBCSParam->pData;
            pTOdbcDBCSParam->pData = NULL;
        }
        pTOdbcDBCSParam->bSetValue=false;
    }
    
    if((pTOdbcDBCSParam->allValueLen!=NULL)
        &&(pTOdbcDBCSParam->iElementCount<iElementCount)
        )
    {
        DELETE_A(pTOdbcDBCSParam->allValueLen);//ZMP_591266
        pTOdbcDBCSParam->bSetValue=false;
    }

    if(pTOdbcDBCSParam->bSetValue==false)
    {
        if(pTOdbcDBCSParam->pData==NULL)
        {
            pTOdbcDBCSParam->pData=new int[iElementCount];
            pTOdbcDBCSParam->slDataLen=iElementSize*iElementCount;
        }
        
        pTOdbcDBCSParam->ssiDataType=SQL_C_SLONG;
        pTOdbcDBCSParam->llElementSize=iElementSize;
        
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((int*)pTOdbcDBCSParam->pData)[iIndex]=aiParamValue[iIndex];
        }       
            
        if(pTOdbcDBCSParam->allValueLen==NULL)
        {
            pTOdbcDBCSParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBCSParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        }
    
        //rc=SQLBindParameter(hstmt,pTOdbcDBCSParam->iIndex+1,SQL_PARAM_INPUT,
        //    (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
        //    (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
        //    (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->llElementSize,
        //    (SQLLEN*)pTOdbcDBCSParam->allValueLen);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBCSParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->llElementSize,
            (SQLLEN*)pTOdbcDBCSParam->allValueLen);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBCSParam->slDataLen=iElementSize*iElementCount;
        pTOdbcDBCSParam->ssiDataType=SQL_C_SLONG;
        pTOdbcDBCSParam->llElementSize=iElementSize;
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((int*)pTOdbcDBCSParam->pData)[iIndex]=aiParamValue[iIndex];
        }       
        memset(pTOdbcDBCSParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
    }

    pTOdbcDBCSParam->iElementCount=iElementCount;
    pTOdbcDBCSParam->bSetValue=true;    
    return;
}

void TOdbcDBCSQuery::SetParamArray(const char *sParamName,long *alParamValue,int iInterval,
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
            DELETE_A(aiParamValue);//ZMP_591266
            //ZMP:305593
            TOdbcDBException ex(e);
            throw ex;
        }
        DELETE_A(aiParamValue);//ZMP_591266
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
            SetParamArray(sParamName,(long long*)allParamValue,(int)sizeof(long long),iElementCount*(int)sizeof(long long),bOutput);
        }
        catch(TOdbcDBException &e)
        {
            DELETE_A(allParamValue);//ZMP_591266
            throw e;
        }

        DELETE_A(allParamValue);//ZMP_591266

    }

    return;
}

void TOdbcDBCSQuery::SetParamArray(const char *sParamName,double *adParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if((adParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    int iElementSize=sizeof(double);
    TOdbcDBCSParam *pTOdbcDBCSParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;

    if((pTOdbcDBCSParam->pData!=NULL)
        &&(pTOdbcDBCSParam->slDataLen<iElementSize*iElementCount)
      )
    {
        //DELETE(pTOdbcDBCSParam->pData);
        //ZMP:477242
        if(pTOdbcDBCSParam->pData != NULL)
        {
            delete [](char*)pTOdbcDBCSParam->pData;
            pTOdbcDBCSParam->pData = NULL;
        }
        pTOdbcDBCSParam->bSetValue=false;
    }
    
    if((pTOdbcDBCSParam->allValueLen!=NULL)
        &&(pTOdbcDBCSParam->iElementCount<iElementCount)
        )
    {
        DELETE_A(pTOdbcDBCSParam->allValueLen);//ZMP_591266
        pTOdbcDBCSParam->bSetValue=false;
    }

    if(pTOdbcDBCSParam->bSetValue==false)
    {
        if(pTOdbcDBCSParam->pData==NULL)
        {
            pTOdbcDBCSParam->pData=new double[iElementCount];
            pTOdbcDBCSParam->slDataLen=iElementSize*iElementCount;
        }

        memcpy(pTOdbcDBCSParam->pData,adParamValue,(size_t)iArraySize);
        pTOdbcDBCSParam->ssiDataType=SQL_C_DOUBLE;
        pTOdbcDBCSParam->llElementSize=iElementSize;
    
        if(pTOdbcDBCSParam->allValueLen==NULL)
        {
            pTOdbcDBCSParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBCSParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        }
    
        rc=SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBCSParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->llElementSize,
            (SQLLEN*)pTOdbcDBCSParam->allValueLen);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBCSParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->llElementSize,
            (SQLLEN*)pTOdbcDBCSParam->allValueLen);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBCSParam->slDataLen=iElementSize*iElementCount;
        memcpy(pTOdbcDBCSParam->pData,adParamValue,(size_t)iArraySize);
        pTOdbcDBCSParam->ssiDataType=SQL_C_DOUBLE;
        pTOdbcDBCSParam->llElementSize=iElementSize;
        memset(pTOdbcDBCSParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
    }
    
    pTOdbcDBCSParam->iElementCount=iArraySize/iElementSize;
    pTOdbcDBCSParam->bSetValue=true;
    return;
}

void TOdbcDBCSQuery::SetParamArray(const char *sParamName,long long *allParamValue,int iInterval,
                                 int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if((allParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetParamArray","Application Error Message=Parameter Error!");
    }

    
    TOdbcDBCSParam *pTOdbcDBCSParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iInterval;
    int iElementSize=SIZE_LONG_LONG;

    if((pTOdbcDBCSParam->pData!=NULL)
        &&(pTOdbcDBCSParam->slDataLen<iElementSize*iElementCount)
      )
    {
        //DELETE(pTOdbcDBCSParam->pData);
        //ZMP:477242
        if(pTOdbcDBCSParam->pData != NULL)
        {
            delete [](char*)pTOdbcDBCSParam->pData;
            pTOdbcDBCSParam->pData = NULL;
        }
        pTOdbcDBCSParam->bSetValue=false;
    }
    
     if((pTOdbcDBCSParam->allValueLen!=NULL)
        &&(pTOdbcDBCSParam->iElementCount<iElementCount)
        )
    {
        DELETE_A(pTOdbcDBCSParam->allValueLen);//ZMP_591266
        pTOdbcDBCSParam->bSetValue=false;
    }
    
    if(pTOdbcDBCSParam->bSetValue==false)
    {
        if(pTOdbcDBCSParam->pData==NULL)
        {
            pTOdbcDBCSParam->pData=new char[iElementCount*SIZE_LONG_LONG];
            pTOdbcDBCSParam->slDataLen=iElementSize*iElementCount;
        }
       
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            #ifdef _WIN32
                //ZMP:467367 
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%I64d", allParamValue[iIndex]);
                m_sLongLong[sizeof(m_sLongLong)-1] = '\0';
            #else
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%lld",allParamValue[iIndex]);
            #endif
            memcpy((char*)pTOdbcDBCSParam->pData+SIZE_LONG_LONG*iIndex,m_sLongLong,sizeof(m_sLongLong));
        }
        
        pTOdbcDBCSParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBCSParam->llElementSize=iElementSize;
       
        if(pTOdbcDBCSParam->allValueLen==NULL)
        {
            pTOdbcDBCSParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBCSParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        }
    
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBCSParam->allValueLen)[iIndex]=SQL_NTS;
        }
    
        //rc=SQLBindParameter(hstmt,pTOdbcDBCSParam->iIndex+1,SQL_PARAM_INPUT,
        //    (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
        //    (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
        //    (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->llElementSize,
        //    (SQLLEN*)pTOdbcDBCSParam->allValueLen);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBCSParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->llElementSize,
            (SQLLEN*)pTOdbcDBCSParam->allValueLen);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBCSParam->slDataLen=iElementSize*iElementCount;
                
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            #ifdef _WIN32
                //ZMP:467367 
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%I64d",allParamValue[iIndex]);
                m_sLongLong[sizeof(m_sLongLong)-1] = '\0';
            #else
                snprintf(m_sLongLong, sizeof(m_sLongLong), "%lld",allParamValue[iIndex]);
            #endif
            memcpy((char*)pTOdbcDBCSParam->pData+SIZE_LONG_LONG*iIndex,m_sLongLong,sizeof(m_sLongLong));
        }
        
        pTOdbcDBCSParam->ssiDataType=SQL_C_CHAR;
        pTOdbcDBCSParam->llElementSize=iElementSize;
        
        memset(pTOdbcDBCSParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBCSParam->allValueLen)[iIndex]=SQL_NTS;
        }
    }

    pTOdbcDBCSParam->iElementCount=iElementCount;
    pTOdbcDBCSParam->bSetValue=true;
    return;
}

void TOdbcDBCSQuery::SetBlobParamArray(const char *sParamName,char *sParamValue,
                                     int iBufferLen,int iArraySize,bool bOutput)throw (TOdbcDBException)
{
    if((sParamValue==NULL)
        ||(iArraySize<=0)
        )
    {
        throw TOdbcDBException("TOdbcDBCSQuery::SetBlobParamArray","Application Error Message=Parameter Error!");
    }

    int iElementSize=iBufferLen;
    TOdbcDBCSParam *pTOdbcDBCSParam=ParamByCondition(sParamName);
    int iElementCount=iArraySize/iBufferLen;

    if((pTOdbcDBCSParam->pData!=NULL)
        &&(pTOdbcDBCSParam->slDataLen<iElementSize*iElementCount)
      )
    {
        //DELETE(pTOdbcDBCSParam->pData);
        //ZMP:477242
        if(pTOdbcDBCSParam->pData != NULL)
        {
            delete [](char*)pTOdbcDBCSParam->pData;
            pTOdbcDBCSParam->pData = NULL;
        }
        pTOdbcDBCSParam->bSetValue=false;
    }
    
    if((pTOdbcDBCSParam->allValueLen!=NULL)
        &&(pTOdbcDBCSParam->iElementCount<iElementCount)
        )
    {
        DELETE_A(pTOdbcDBCSParam->allValueLen);//ZMP_591266
        pTOdbcDBCSParam->bSetValue=false;
    }
    
    if(pTOdbcDBCSParam->bSetValue==false)
    {
        if(pTOdbcDBCSParam->pData==NULL)
        {
            pTOdbcDBCSParam->pData=new char[iArraySize];
            pTOdbcDBCSParam->slDataLen=iElementSize*iElementCount;
        }
        
        memcpy(pTOdbcDBCSParam->pData,sParamValue,(size_t)(iElementSize*iElementCount));
        pTOdbcDBCSParam->ssiDataType=SQL_C_BINARY;
        pTOdbcDBCSParam->llElementSize=iElementSize;
        
        if(pTOdbcDBCSParam->allValueLen==NULL)
        {
            pTOdbcDBCSParam->allValueLen=new SQLLEN[iElementCount];
            memset(pTOdbcDBCSParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        }
    
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBCSParam->allValueLen)[iIndex]=iElementSize;
        }
    
        //rc=SQLBindParameter(hstmt,pTOdbcDBCSParam->iIndex+1,SQL_PARAM_INPUT,
        //    (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
        //    (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
        //    (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->llElementSize,
        //    (SQLLEN*)pTOdbcDBCSParam->allValueLen);
        rc=TTTCSFactory::GetTTInstance()->cs_SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBCSParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->llElementSize,
            (SQLLEN*)pTOdbcDBCSParam->allValueLen);
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBCSParam->slDataLen=iElementSize*iElementCount;
        memcpy(pTOdbcDBCSParam->pData,sParamValue,(size_t)(iElementSize*iElementCount));
        pTOdbcDBCSParam->ssiDataType=SQL_C_BINARY;
        pTOdbcDBCSParam->llElementSize=iElementSize;
        
        memset(pTOdbcDBCSParam->allValueLen,0,(size_t)iElementCount*sizeof(SQLLEN));
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            ((SQLLEN*)pTOdbcDBCSParam->allValueLen)[iIndex]=iElementSize;
        }
    }

    pTOdbcDBCSParam->iElementCount=iElementCount;
    pTOdbcDBCSParam->bSetValue=true;
    return;
}

//private函数
void TOdbcDBCSQuery::CheckError(const char* sSql) throw (TOdbcDBException)
{
    static ostringstream ostrErr;
    int iRet=0;

    if((rc!=SQL_SUCCESS)
        &&(rc!=SQL_SUCCESS_WITH_INFO)
        )
    {
        ostrErr.str("");
        iRet=CS_DBError(rc,henv,hdbc,hstmt,ostrErr);
    }

    if(iRet!=0)
    {
        throw TOdbcDBException(sSql,"Application Error Message=%s",ostrErr.str().c_str());
    }
}

void TOdbcDBCSQuery::ParseSql(char* sSqlStatement)
{
    int iLen=0;
    int iIndex=0;
    int iParamBegin=0;
    int iParamEnd=0;
    int iQuoteFlag=0;                                       //为0表示无引号,为1表示单引号开始
    char    cCur=0;
    string strParamName="";

    iLen=(int)strlen(sSqlStatement);//ZMP:477242
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
                throw TOdbcDBException("TOdbcDBCSQuery::ParseSql","Application Error Message=Param name error");
            }
            else
            {
                if(m_iParamCount==MAX_COLUMN_COUNT)
                {
                    throw TOdbcDBException("TOdbcDBCSQuery::ParseSql","Application Error Message=Count of prams beyond limit(%ld)",MAX_COLUMN_COUNT);
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

void TOdbcDBCSQuery::GetFieldBufferInfo(TOdbcDBCSField &oTOdbcDBField,int &iRowsCount)
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
        //if(iRowsCount<=oTOdbcDBField.iMaxRowsCount)
        //{//已经分配足够大小的缓冲区了
        //    return;
        //}
        bool bFlag = ( (oTOdbcDBField.ssiDataType==SQL_C_CHAR)
            ||(oTOdbcDBField.ssiDataType==SQL_C_BINARY)
            ||(oTOdbcDBField.iMaxRowsCount>1) ); //315428 
        if( bFlag )
        {
            DELETE(oTOdbcDBField.sData);
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
            throw TOdbcDBException("TOdbcDBCSQuery::GetFieldBufferInfo","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
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
TOdbcDBCSParam* TOdbcDBCSQuery::ParamByCondition(const char *sParamName,int iValueFlag,int iIndex)
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
        throw TOdbcDBException("TOdbcDBCSQuery::ParamByCondition","Application Error Message=Parameter Error!");
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

    throw TOdbcDBException("TOdbcDBCSQuery::ParamByCondition","Application Error Message=Param(%s) not exist!",sParamName);

    return NULL;
}

//设置参数值
void TOdbcDBCSQuery::SetParameter(TOdbcDBCSParam *pTOdbcDBCSParam,const char* sParamValue) throw (TOdbcDBException)
{
    int iElementCount=1;
    size_t iLenParamValue = 0;//ZMP:477242
    if (sParamValue!=NULL)
    {
        iLenParamValue = strlen(sParamValue);
    }
    if((pTOdbcDBCSParam->pData!=NULL)
        &&(pTOdbcDBCSParam->slDataLen<=(SQLLEN)iLenParamValue)
        )
    {
        //DELETE(pTOdbcDBCSParam->pData);
        //ZMP:477242
        if(pTOdbcDBCSParam->pData != NULL)
        {
            delete [](char*)pTOdbcDBCSParam->pData;
            pTOdbcDBCSParam->pData = NULL;
        }
        pTOdbcDBCSParam->slDataLen=0;
        pTOdbcDBCSParam->bSetValue=false;
    }

    if(pTOdbcDBCSParam->bSetValue==false)
    {
        pTOdbcDBCSParam->llValueLen=0;
        if(sParamValue!=NULL)
        {
            if(pTOdbcDBCSParam->pData==NULL)
            {
                pTOdbcDBCSParam->pData=new char[strlen(sParamValue)+1];
                pTOdbcDBCSParam->slDataLen=(SQLLEN)strlen(sParamValue)+1;
            }
            memset(pTOdbcDBCSParam->pData,0,strlen(sParamValue)+1);
            memcpy(pTOdbcDBCSParam->pData,sParamValue,strlen(sParamValue));
            pTOdbcDBCSParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBCSParam->llValueLen=SQL_NTS;
        }
        else
        {
            if(pTOdbcDBCSParam->pData==NULL)
            {
                pTOdbcDBCSParam->pData=new char[1];
                pTOdbcDBCSParam->slDataLen=1;
            }
            ((char*)pTOdbcDBCSParam->pData)[0]='\0';
            pTOdbcDBCSParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBCSParam->llValueLen=SQL_NULL_DATA;
        }
    
        //rc=SQLBindParameter(hstmt,pTOdbcDBCSParam->iIndex+1,SQL_PARAM_INPUT,
        //    (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
        //    (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
        //    (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->slDataLen,
        //    (SQLLEN*)&(pTOdbcDBCSParam->llValueLen));
        rc=TTTCSFactory::GetTTInstance()->cs_SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBCSParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->slDataLen,
            (SQLLEN*)&(pTOdbcDBCSParam->llValueLen));
        CheckError(m_sSql);
    }
    else
    {
        if(sParamValue!=NULL)
        {
            // ZMP_597442
            size_t iLen=strlen(sParamValue);
            if(pTOdbcDBCSParam->pData==NULL)
            {
                pTOdbcDBCSParam->pData=new char[iLen+1];
                pTOdbcDBCSParam->slDataLen=(SQLLEN)(iLen+1);
            }
            memset(pTOdbcDBCSParam->pData,0,iLen+1);
            memcpy(pTOdbcDBCSParam->pData,sParamValue,iLen);
            pTOdbcDBCSParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBCSParam->llValueLen=SQL_NTS;
        }
        else
        {
            // ZMP_597442
            if( pTOdbcDBCSParam->pData == NULL )
            {
                pTOdbcDBCSParam->pData=new char[1];
                pTOdbcDBCSParam->slDataLen=1;                
            }
            ((char*)pTOdbcDBCSParam->pData)[0]='\0';
            pTOdbcDBCSParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBCSParam->llValueLen=SQL_NULL_DATA;
        }
    }

    pTOdbcDBCSParam->iElementCount=iElementCount;
    pTOdbcDBCSParam->bSetValue=true;
    
    return;
}

void TOdbcDBCSQuery::SetParameter(TOdbcDBCSParam *pTOdbcDBCSParam,int iParamValue)throw (TOdbcDBException)
{
    int iElementCount=1;

    if(pTOdbcDBCSParam->bSetValue==false)
    {
        pTOdbcDBCSParam->iData=iParamValue;
        pTOdbcDBCSParam->slDataLen=sizeof(iParamValue);
        pTOdbcDBCSParam->ssiDataType=SQL_C_SLONG;
        pTOdbcDBCSParam->llValueLen=0;
    
        //rc=SQLBindParameter(hstmt,pTOdbcDBCSParam->iIndex+1,SQL_PARAM_INPUT,
        //    (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
        //    (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
        //    (SQLPOINTER*)&(pTOdbcDBCSParam->iData),(SQLLEN)pTOdbcDBCSParam->slDataLen,
        //    (SQLLEN*)&(pTOdbcDBCSParam->llValueLen));
        rc=TTTCSFactory::GetTTInstance()->cs_SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBCSParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
            (SQLPOINTER*)&(pTOdbcDBCSParam->iData),(SQLLEN)pTOdbcDBCSParam->slDataLen,
            (SQLLEN*)&(pTOdbcDBCSParam->llValueLen));
        CheckError(m_sSql);
    }
    else
    {
        pTOdbcDBCSParam->iData=iParamValue;
    }
    
    pTOdbcDBCSParam->iElementCount=iElementCount;
    pTOdbcDBCSParam->bSetValue=true;
    
    return;
}

void TOdbcDBCSQuery::SetParameter(TOdbcDBCSParam *pTOdbcDBCSParam,long lParamValue)throw (TOdbcDBException)
{
    double dParamValue=(double)lParamValue;
    SetParameter(pTOdbcDBCSParam,dParamValue);
    return;
}

void TOdbcDBCSQuery::SetParameter(TOdbcDBCSParam *pTOdbcDBCSParam,double dParamValue)throw (TOdbcDBException)
{
    int iElementCount=1;

    //if(pTOdbcDBCSParam->bSetValue==false) //ZMP_597442
    //{
        pTOdbcDBCSParam->dData=dParamValue;
        pTOdbcDBCSParam->slDataLen=sizeof(double);
        pTOdbcDBCSParam->ssiDataType=SQL_C_DOUBLE;
        pTOdbcDBCSParam->llValueLen=0;
    
        //rc=SQLBindParameter(hstmt,pTOdbcDBCSParam->iIndex+1,SQL_PARAM_INPUT,
        //    (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
        //    (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
        //    (SQLPOINTER*)&(pTOdbcDBCSParam->dData),(SQLLEN)pTOdbcDBCSParam->slDataLen,
        //    (SQLLEN*)&(pTOdbcDBCSParam->llValueLen));
        rc=TTTCSFactory::GetTTInstance()->cs_SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBCSParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
            (SQLPOINTER*)&(pTOdbcDBCSParam->dData),(SQLLEN)pTOdbcDBCSParam->slDataLen,
            (SQLLEN*)&(pTOdbcDBCSParam->llValueLen));
        CheckError(m_sSql);
    //}
    //else  //ZMP_597442
    //{
    //    pTOdbcDBCSParam->dData=dParamValue;
    //}
    
    pTOdbcDBCSParam->iElementCount=iElementCount;
    pTOdbcDBCSParam->bSetValue=true;
    
    return;
}

void TOdbcDBCSQuery::SetParameter(TOdbcDBCSParam *pTOdbcDBCSParam,long long llParamValue)throw (TOdbcDBException)
{
    SetParameter(pTOdbcDBCSParam,(double)llParamValue);
    return;
}

void TOdbcDBCSQuery::SetParameter(TOdbcDBCSParam *pTOdbcDBCSParam,const char* sParamValue,int iBufferLen)throw (TOdbcDBException)
{
    int iElementCount=1;

    if((pTOdbcDBCSParam->pData!=NULL)
        &&(pTOdbcDBCSParam->slDataLen<(SQLLEN)iBufferLen)
        )
    {
        //DELETE(pTOdbcDBCSParam->pData);
        //ZMP:477242
        if(pTOdbcDBCSParam->pData != NULL)
        {
            delete [](char*)pTOdbcDBCSParam->pData;
            pTOdbcDBCSParam->pData = NULL;
        }
        pTOdbcDBCSParam->slDataLen=0;
        pTOdbcDBCSParam->bSetValue=false;
    }

    if(pTOdbcDBCSParam->bSetValue==false)
    {
        pTOdbcDBCSParam->llValueLen=0;
        if((sParamValue!=NULL)&&(iBufferLen>0))
        {
            if(pTOdbcDBCSParam->pData==NULL)
            {
                pTOdbcDBCSParam->pData=new char[iBufferLen];
                pTOdbcDBCSParam->slDataLen=iBufferLen;
            }
            memset(pTOdbcDBCSParam->pData,0,(size_t)iBufferLen);
            memcpy(pTOdbcDBCSParam->pData,sParamValue,(size_t)iBufferLen);
            pTOdbcDBCSParam->ssiDataType=SQL_C_BINARY;
            pTOdbcDBCSParam->llValueLen=iBufferLen;
        }
        else
        {
            if(pTOdbcDBCSParam->pData==NULL)
            {
                pTOdbcDBCSParam->pData=new char[1];
                pTOdbcDBCSParam->slDataLen=1;
            }
            ((char*)pTOdbcDBCSParam->pData)[0]='\0';
            pTOdbcDBCSParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBCSParam->llValueLen=0;
        }
    
        //rc=SQLBindParameter(hstmt,pTOdbcDBCSParam->iIndex+1,SQL_PARAM_INPUT,
        //    (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
        //    (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
        //    (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->slDataLen,
        //    (SQLLEN*)&(pTOdbcDBCSParam->llValueLen));
        rc=TTTCSFactory::GetTTInstance()->cs_SQLBindParameter(hstmt,(unsigned short)(pTOdbcDBCSParam->iIndex+1),SQL_PARAM_INPUT,
            (SQLSMALLINT)pTOdbcDBCSParam->ssiDataType,(SQLSMALLINT)pTOdbcDBCSParam->ssiSqlDataType,
            (SQLULEN)pTOdbcDBCSParam->sulPrecision,(SQLSMALLINT)pTOdbcDBCSParam->ssiScale,
            (SQLPOINTER*)pTOdbcDBCSParam->pData,(SQLLEN)pTOdbcDBCSParam->slDataLen,
            (SQLLEN*)&(pTOdbcDBCSParam->llValueLen));
        CheckError(m_sSql);
    }
    else
    {
        if((sParamValue!=NULL)&&(iBufferLen>0))
        {
            memset(pTOdbcDBCSParam->pData,0,(size_t)iBufferLen);
            memcpy(pTOdbcDBCSParam->pData,sParamValue,(size_t)iBufferLen);
            pTOdbcDBCSParam->ssiDataType=SQL_C_BINARY;
            pTOdbcDBCSParam->llValueLen=iBufferLen;
        }
        else
        {
            ((char*)pTOdbcDBCSParam->pData)[0]='\0';
            pTOdbcDBCSParam->ssiDataType=SQL_C_CHAR;
            pTOdbcDBCSParam->llValueLen=0;
        }
    }
    
    pTOdbcDBCSParam->iElementCount=iElementCount;
    pTOdbcDBCSParam->bSetValue=true;
    
    return;
}

void TOdbcDBCSQuery::SetParameterNULL(TOdbcDBCSParam *pTOdbcDBCSParam) throw (TOdbcDBException)
{
    SetParameter(pTOdbcDBCSParam,(char*)NULL);
    return;
}

 //ZMP_559430
long long TOdbcDBCSQuery::GetSequenceByName(const char * sName)throw (TOdbcDBException)
{
    throw TOdbcDBException("TOdbcDBCSQuery::GetSequenceByName", "doesn't support GetSequenceByName() now.");
}
////////////////////////////////////////////////////////////////////////////////
//138555 begin
time_t TOdbcDBCSField::AsTimeT() throw (TOdbcDBException)
{
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBCSField::AsTimeT","Application Error Message=Data set not opened!");
    }

    time_t tValue = 0;
//    memset(sDataValue,0,sizeof(sDataValue));
    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_VARCHAR:
    case SQL_TIMESTAMP:
    case SQL_TIME:
    case SQL_DATE:
        {
            tValue = AsDateTime_t();
            break;
        }
    default:
        {
            throw TOdbcDBException("TOdbcDBCSField::AsTimeT","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
        }
    }

    return tValue;
}

time_t TOdbcDBCSField::AsDateTime_t() throw (TOdbcDBException)
{
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

int TOdbcDBCSField::AsDateTime_Detail(int &iYear,int &iMonth,int &iDay,int &iHour,int &iMinute,int &iSecond) throw (TOdbcDBException)
{
    if(m_pTOdbcDBQuery->m_iQueryState!=QUERY_FETCH_BOF)
    {//判断执行状态
        throw TOdbcDBException("TOdbcDBCSField::AsDateTime_Detail","Application Error Message=Data set not opened!");
    }

    switch((SQLSMALLINT)ssiSqlDataType)
    {
    case SQL_VARCHAR:
        {
            char *sTemp = NULL;
            sTemp = sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);
            size_t iLen=strlen(sTemp);//ZMP:477242
    
            if((iLen!=14)&&(iLen!=8)&&(iLen != 0))
                throw TOdbcDBException("TOdbcDBCSField::AsDateTime_Detail","Application Error Message=FieldLength is error!");

            if(iLen == 0)
            {
                return -1;
            }

            char sTempYear[5]={0},sTempMon[3]={0},sTempMDay[3]={0},sTempHour[3]={0},sTempMin[3]={0},sTempSec[3]={0}; //356853 for cppcheck
            strncpy(sTempYear,sTemp,4);
            //sTempYear[4]=0;  //356853 初始化时已做,此处没必要了
            sTempMon[0]=sTemp[4];
            sTempMon[1]=sTemp[5];
            //sTempMon[2]=0;   //356853  初始化时已做,此处没必要了
            sTempMDay[0]=sTemp[6]; 
            sTempMDay[1]=sTemp[7]; 
            //sTempMDay[2]=0;  //356853  初始化时已做,此处没必要了

            // 支持8位
            if(8 == iLen)
            {
                sTempHour[0] = '0';
                sTempHour[1] = '0';
                //sTempHour[2]=0; //356853  初始化时已做,此处没必要了

                sTempMin[0]= '0';
                sTempMin[1]= '0';
                //sTempMin[2]=0; //356853  初始化时已做,此处没必要了

                sTempSec[0]= '0';
                sTempSec[1]= '0';
                //sTempSec[2]=0; //356853  初始化时已做,此处没必要了
            }
            else  // 14位
            {
                sTempHour[0]=sTemp[8];
                sTempHour[1]=sTemp[9]; 
                //sTempHour[2]=0; //356853  初始化时已做,此处没必要了

                sTempMin[0]=sTemp[10];
                sTempMin[1]=sTemp[11]; 
                //sTempMin[2]=0; //356853  初始化时已做,此处没必要了

                sTempSec[0]=sTemp[12];
                sTempSec[1]=sTemp[13]; 
                //sTempSec[2]=0; //356853  初始化时已做,此处没必要了
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
    case SQL_DATE:
        {
            bool bNumber=false;
            int iSegIndex=0;
            char *sCur=sData+(slDataLen)*(m_pTOdbcDBQuery->m_iRowIndex);

            if( !strcmp(sCur,"") )
              return -1;

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
    default:
        {
            throw TOdbcDBException("TOdbcDBCSField::AsDateTime_Detail","Application Error Message=SQL_DATA_TYPE(%ld) not supported",ssiSqlDataType);
            //break;//ZMP:305593,无用，去掉
        }
    }
    return 1;
}

void TOdbcDBCSQuery::SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused)throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=ParamByCondition(sParamName);
    char sParamValue[40] = {0};//ZMP:477242
    //memset(sParamValue,0x00,sizeof(sParamValue));
    struct tm * tm_Cur;
    tm_Cur = localtime(&tParamValue);
    //ZMP:467367
    snprintf(sParamValue, sizeof(sParamValue), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
    sParamValue[sizeof(sParamValue)-1] = '\0';
    SetParameter(pTOdbcDBCSParam,sParamValue);
    return;
}

//138555 end
////////////////////////////////////////////////////////////////////////////////

//141877 begin
void TOdbcDBCSQuery::SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException)
{
    TOdbcDBCSParam *pTOdbcDBCSParam=&Param(iParamIndex);
    char sParamValue[40] = {0};//ZMP:477242
    //memset(sParamValue,0x00,sizeof(sParamValue));
    struct tm * tm_Cur;
    tm_Cur = localtime(&tParamValue);
    //ZMP:467367
    snprintf(sParamValue, sizeof(sParamValue), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
    sParamValue[sizeof(sParamValue)-1] = '\0';
    SetParameter(pTOdbcDBCSParam,sParamValue);
    return;
}
//141877 end
 
#endif 


