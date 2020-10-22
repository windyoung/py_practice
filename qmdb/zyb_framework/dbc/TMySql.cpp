#include <stdlib.h>
#include <stdarg.h>
#include "TMySql.h"
#include "Common.h"
#include "TStrFunc.h"
#include "debug_new.h"
#include "ReadIni.h"


/*************************************************
*                                                *
*         ---- TMySqlException实现 ----           *
*                                                *
*************************************************/

TMySqlException::TMySqlException(const char *sSql, const char* sFormat, ...)
{
    va_list args;
    
    memset(m_sErrMsg,0,sizeof(m_sErrMsg));
    memset(m_sErrSql,0,sizeof(m_sErrSql));
    m_lErrCode=0;

    va_start(args,sFormat); 
    vsnprintf(m_sErrMsg,sizeof(m_sErrMsg),sFormat,args);
    m_sErrMsg[sizeof(m_sErrMsg)-1]='\0'; 
    va_end(args);       

    strncpy(m_sErrSql,sSql,sizeof(m_sErrSql)-1);
    m_sErrSql[sizeof(m_sErrSql)-1]='\0';
}

/*************************************************
*                                                *
*         ---- TMySqlInterface实现 ----           *
*                                                *
*************************************************/

TMySqlInterface::TMySqlInterface(void)
{
    char sItem[512] = {0};
    char *sSOPath = NULL;    
    /*--不再读配置文件---
    char sAppCfgName[256] = {0};
    memset(sAppCfgName, 0, sizeof(sAppCfgName));
    const char * theEnv=getenv("HOME");
    if(theEnv==NULL)
    {
        strncpy(sAppCfgName,"App.config", sizeof(sAppCfgName)-1);
        sAppCfgName[sizeof(sAppCfgName)-1] = '\0';
    }
    else
    {
#if defined(__OS_WINDOWS__)
        snprintf(sAppCfgName, sizeof(sAppCfgName), "%s\\etc\\App.config",theEnv);
        sAppCfgName[sizeof(sAppCfgName)-1] = '\0';
#else
        snprintf(sAppCfgName, sizeof(sAppCfgName), "%s/etc/App.config",theEnv);
#endif
    }
    
    TReadIni ReadIni(sAppCfgName); 
    sSOPath = ReadIni.ReadString("SO_PATH","MYSQL_LIB",sItem,NULL);
    */
    //if (sSOPath==NULL)
    //{
        sSOPath = getenv(MYSQL_HOME_PATH_ENV);
        if( sSOPath==NULL )
        {
            throw TMySqlException(" ","TMySqlInterface::TMySqlInterface() : Can't find %s,please check you config!\n", MYSQL_LIB_NAME);
        }
#ifdef WIN32
        snprintf(sItem, sizeof(sItem), "%s\\lib\\%s",sSOPath,MYSQL_LIB_NAME);
        sItem[sizeof(sItem)-1] = '\0';
#else
        snprintf(sItem, sizeof(sItem), "%s/lib/%s",sSOPath,MYSQL_LIB_NAME);
#endif
    //}
    
    m_pSoHelper = new SOHelper();
    if( !m_pSoHelper->OpenLibrary( sItem ) )// sItem 为 mysqlclient库的路径
    {
        throw TMySqlException(" ","TMySqlInterface::TMySqlInterface() : OpenLibrary %s return False\n", sSOPath);
    }
    fpMySqlInit   = (MySqlInit)m_pSoHelper->LoadSymbol("mysql_init");
    fpMySqlClose  = (MySqlClose)m_pSoHelper->LoadSymbol("mysql_close");
    fpMySqlCommit = (MySqlCommit)m_pSoHelper->LoadSymbol("mysql_commit");
    fpMySqlRollBack = (MySqlRollBack)m_pSoHelper->LoadSymbol("mysql_rollback");
    fpMySqlAutoCommit = (MySqlAutoCommit)m_pSoHelper->LoadSymbol("mysql_autocommit");
    fpMySqlRealConnect = (MySqlRealConnect)m_pSoHelper->LoadSymbol("mysql_real_connect");
    fpMySqlErrno = (MySqlErrno)m_pSoHelper->LoadSymbol("mysql_errno");
    fpMySqlError = (MySqlError)m_pSoHelper->LoadSymbol("mysql_error");
    fpMySqlFetchFieldDirect = (MySqlFetchFieldDirect)m_pSoHelper->LoadSymbol("mysql_fetch_field_direct");
    fpMySqlStmtInit = (MySqlStmtInit)m_pSoHelper->LoadSymbol("mysql_stmt_init");
    fpMySqlStmtPrepare = (MySqlStmtPrepare)m_pSoHelper->LoadSymbol("mysql_stmt_prepare");
    fpMySqlStmtExecute = (MySqlStmtExecute)m_pSoHelper->LoadSymbol("mysql_stmt_execute");
    fpMySqlStmtFetch = (MySqlStmtFetch)m_pSoHelper->LoadSymbol("mysql_stmt_fetch");
    fpMySqlStmtFetchColumn = (MySqlStmtFetchColumn)m_pSoHelper->LoadSymbol("mysql_stmt_fetch_column");
    fpMySqlStmtStoreResult = (MySqlStmtStoreResult)m_pSoHelper->LoadSymbol("mysql_stmt_store_result");
    fpMySqlStmtParamCount =    (MySqlStmtParamCount)m_pSoHelper->LoadSymbol("mysql_stmt_param_count");
    fpMySqlStmtAttrSet =    (MySqlStmtAttrSet)m_pSoHelper->LoadSymbol("mysql_stmt_attr_set");
    fpMySqlStmtAttrGet =    (MySqlStmtAttrGet)m_pSoHelper->LoadSymbol("mysql_stmt_attr_get");
    fpMySqlStmtBindParam = (MySqlStmtBindParam)m_pSoHelper->LoadSymbol("mysql_stmt_bind_param");
    fpMySqlStmtBindResult = (MySqlStmtBindResult)m_pSoHelper->LoadSymbol("mysql_stmt_bind_result");
    fpMySqlStmtClose = (MySqlStmtClose)m_pSoHelper->LoadSymbol("mysql_stmt_close");
    fpMySqlStmtFreeResult = (MySqlStmtFreeResult)m_pSoHelper->LoadSymbol("mysql_stmt_free_result");
    fpMySqlStmtResultMetadata = (MySqlStmtResultMetadata)m_pSoHelper->LoadSymbol("mysql_stmt_result_metadata");
    fpMySqlStmtErrno = (MySqlStmtErrno)m_pSoHelper->LoadSymbol("mysql_stmt_errno");
    fpMySqlStmtError = (MySqlStmtError)m_pSoHelper->LoadSymbol("mysql_stmt_error");
    fpMySqlStmtNumRows =    (MySqlStmtNumRows)m_pSoHelper->LoadSymbol("mysql_stmt_num_rows");
    fpMySqlStmtAffectedRows = (MySqlStmtAffectedRows)m_pSoHelper->LoadSymbol("mysql_stmt_affect_rows");
    fpMySqlStmtFieldCount = (MySqlStmtFieldCount)m_pSoHelper->LoadSymbol("mysql_stmt_field_count");
    fpMySqlFreeResult = (MySqlFreeResult)m_pSoHelper->LoadSymbol("mysql_free_result");
    fpMySqlFetchLengths = (MySqlFetchLengths)m_pSoHelper->LoadSymbol("mysql_fetch_lengths");
}
TMySqlInterface::~TMySqlInterface(void)
{
    if(m_pSoHelper)
    {
        m_pSoHelper->CloseLibrary();
        DELETE(m_pSoHelper);
    }
    fpMySqlInit = NULL ;
    fpMySqlClose = NULL ;
    fpMySqlCommit = NULL ;
    fpMySqlRollBack = NULL ;
    fpMySqlAutoCommit = NULL ;
    fpMySqlRealConnect = NULL;
    fpMySqlErrno = NULL ;
    fpMySqlError = NULL ;
    fpMySqlFetchFieldDirect = NULL;
    fpMySqlStmtInit = NULL ;
    fpMySqlStmtPrepare = NULL ;
    fpMySqlStmtExecute = NULL ;
    fpMySqlStmtFetch = NULL ;
    fpMySqlStmtFetchColumn = NULL ;
    fpMySqlStmtStoreResult = NULL ;
    fpMySqlStmtResultMetadata = NULL;
    fpMySqlStmtParamCount = NULL ;
    fpMySqlStmtAttrSet = NULL ;
    fpMySqlStmtAttrGet = NULL ;
    fpMySqlStmtBindParam = NULL ;
    fpMySqlStmtBindResult = NULL ;
    fpMySqlStmtClose = NULL ;
    fpMySqlStmtFreeResult = NULL ;
    fpMySqlStmtErrno = NULL ;
    fpMySqlStmtError = NULL ;
    fpMySqlStmtNumRows = NULL ;
    fpMySqlStmtAffectedRows = NULL ;
    fpMySqlStmtFieldCount = NULL ;
    fpMySqlFreeResult = NULL;
    fpMySqlFreeResult = NULL;
}
/*************************************************
*                                                *
*         ---- TMySqlFactory 实现 ----           *
*                                                *
*************************************************/
TMySqlInterface* TMySqlFactory::m_pInstance = NULL;

TMySqlInterface* TMySqlFactory::GetMySqlInstance(void)
{
    if(m_pInstance == NULL)
    {
        m_pInstance = new TMySqlInterface();
        if (m_pInstance==NULL)
        {
            
            throw TException("TMySqlFactory::GetMySqlInstance() new TMySqlInterface() Failed.\n");
        }
    }
    return m_pInstance;
}
/*************************************************
*                                                *
*         ---- TMySqlDatabase实现 ----           *
*                                                *
*************************************************/
TMySqlDatabase::TMySqlDatabase()
{
    memset( m_sUsr, 0 , sizeof(m_sUsr) );
    memset( m_sPwd, 0 , sizeof(m_sPwd) ); 
    memset( m_sTns, 0 , sizeof(m_sTns) );
    m_bConnFlag = false ;
    m_uiPort = 0;
    m_pUnixSocket = NULL;
    m_ulClientFlag = 0;
    m_pHost = NULL;
    m_pTMySqlQuery = NULL;
    m_pHandleConn = NULL;
    m_bAutoCommit = false;
    /*
    m_pHandleConn = pMySqlInstance->fpMySqlInit(NULL);
    if( m_pHandleConn == NULL )
    {
        throw TMySqlException("mysql_init()","mysql_init failed: no memory");
    }
    */
}

void TMySqlDatabase::SetServer(const char *pHost , unsigned int uiPort,char *sUnixSocket,unsigned long ulClientFlag )
{
    size_t iLen = 0;
    if( pHost )
    {
        iLen = strlen( pHost );
        m_pHost = new char[ iLen +1 ];
        strncpy( m_pHost , pHost, iLen);
        m_pHost[iLen] = '\0';
    }
    if( sUnixSocket )
    {
        iLen = strlen( sUnixSocket );
        m_pUnixSocket = new char[ iLen +1 ];
        strncpy( m_pUnixSocket , sUnixSocket,iLen );
        m_pUnixSocket[iLen] = '\0';
    }
    m_uiPort = uiPort;
    m_ulClientFlag = ulClientFlag;
}

void TMySqlDatabase::CheckError(const char* sInfo ) throw (TMySqlException)
{
    int iErrno = pMySqlInstance->fpMySqlErrno( m_pHandleConn );
    if ( iErrno != 0 )
    {
        char szErrMsg[4096]={'\0'}; //错误信息
        const char* pStr = pMySqlInstance->fpMySqlError( m_pHandleConn );
        strncpy(szErrMsg,pStr,sizeof(szErrMsg));
        szErrMsg[sizeof(szErrMsg)-1] = '\0';
        if( sInfo )
        {
            strcat(szErrMsg , sInfo );
        }
        throw TMySqlException(" ", "Error info: Errno = %d,ErrMsg: %s\n",iErrno, szErrMsg);
    }
}
bool TMySqlDatabase::Connect(bool bAutoCommit) throw (TMySqlException)
{    
    if( m_pHandleConn == NULL )
    {
        m_pHandleConn = pMySqlInstance->fpMySqlInit(NULL);
        if( m_pHandleConn == NULL )
        {
            throw TMySqlException("mysql_init()","mysql_init failed: no memory");
        }
    }    
    // MYSQL *mysql_real_connect(MYSQL *mysql, const char *host,
    // const char *user, const char *passwd, const char *db,
    // unsigned int port, const char *unix_socket, unsigned long client_flag)
    pMySqlInstance->fpMySqlRealConnect(m_pHandleConn, m_pHost , m_sUsr, m_sPwd, m_sTns, m_uiPort, m_pUnixSocket, m_ulClientFlag);
    CheckError(" mysql_real_connect ");
    m_bAutoCommit = bAutoCommit;
    pMySqlInstance->fpMySqlAutoCommit( m_pHandleConn, m_bAutoCommit ) ;
    CheckError( "mysql_autocommit" );
    return m_bConnFlag = true;
}
bool TMySqlDatabase::Connect(const char *sUsr, const char *sPwd, const char *sTns, bool bUnused) throw (TMySqlException)
{
    SetLogin(sUsr, sPwd, sTns);
    return Connect();
}
int TMySqlDatabase::Disconnect(void) throw (TMySqlException)
{
    //void mysql_close(MYSQL *mysql)
    pMySqlInstance->fpMySqlClose( m_pHandleConn );
    CheckError(" mysql_close ");
    m_bConnFlag   = false;
    m_pHandleConn =NULL;
    return 0;
}

bool TMySqlDatabase::IsConnect() throw (TMySqlException)
{
    if( m_bConnFlag==true )
    {
        if( m_pTMySqlQuery==NULL )
        {
            m_pTMySqlQuery = new TMySqlQuery( this );
        }

        try
        {
            m_pTMySqlQuery->Close();
            m_pTMySqlQuery->SetSQL("SELECT 1 FROM DUAL");
            m_pTMySqlQuery->Open();
            m_pTMySqlQuery->Close();
        }
        catch( TMySqlException& )
        {
            Disconnect();
            m_bConnFlag = false;
            DELETE_A(m_pTMySqlQuery);
            
        }
        catch(...)
        {
            DELETE_A(m_pTMySqlQuery);
            throw TMySqlException("","TMySqlDatabase::IsConnect() catch ... Exception");
        }
    }

    return m_bConnFlag;
}
void TMySqlDatabase::Commit()
{
    //my_bool mysql_commit(MYSQL *mysql)提交事务
    pMySqlInstance->fpMySqlCommit( m_pHandleConn );
    CheckError(" mysql_commit ");
}
void TMySqlDatabase::Rollback()
{
    //my_bool mysql_rollback(MYSQL *mysql)
    pMySqlInstance->fpMySqlRollBack( m_pHandleConn );
    CheckError(" mysql_rollback ");
}
void TMySqlDatabase::SetLogin(const char *sUser, const char *sPassword, const char *sTnsString) throw (TMySqlException)
{
    if (m_bConnFlag )
    {
        throw TMySqlException("SetLogin()", "MySql: You can only set login infomation on disconnect status: line %d" , __LINE__);
    }

    if( !sUser || !sPassword || !sTnsString )
    {
        throw TMySqlException("SetLogin()", "MySql: You can only set login without infomation : line %d" , __LINE__);
    }
    strncpy( m_sUsr, sUser, sizeof( m_sUsr ) );
    strncpy( m_sPwd, sPassword, sizeof( m_sPwd ) );
    strncpy( m_sTns, sTnsString, sizeof( m_sTns ) );
}
TMySqlQuery* TMySqlDatabase::CreateDBQuery() throw (TDBExcpInterface)
{
    return new TMySqlQuery(this);
}

TMySqlDatabase::~TMySqlDatabase()
{
    m_uiPort = 0;

    if( NULL != m_pUnixSocket )
    {
        delete [] m_pUnixSocket;
        m_pUnixSocket = NULL;
    }

    if( NULL != m_pHost )
    {
        delete [] m_pHost;
        m_pHost = NULL;
    }

    if( m_pHandleConn )
    {
        pMySqlInstance->fpMySqlClose( m_pHandleConn );  //关闭连接
        m_pHandleConn = NULL; 
    }
    m_ulClientFlag = 0;
    m_pHost = NULL;
}


/*************************************************
*                                                *
*     ---------- TMySqlParam ----------          *
*                                                *
*************************************************/

TMySqlParam::TMySqlParam()
{
    m_pName = NULL;
    m_IntArray = NULL;
    m_DBLArray = NULL;
    m_LongArray = NULL;
    m_LLArray = NULL;
    //m_StrArray = NULL;
    m_bNullIndictor = false;
    //m_BufType = 0;     
    m_iIntBuf = 0;             
    m_dblBuf = 0;                                
    m_lLongBuf = 0;          
    m_llBuf = 0;                        
    m_StrBuf = NULL;              
    //m_StrBlb="";       
    //m_iElemNum = 0;
    m_iParamIndex = 0;
    //m_cCharBuf = '\0';
    //m_uiBufferType = (enum_field_types)0;
    //m_lBufferLen = 0;
    m_lLongBuf = 0;
}
void  TMySqlParam::ClearInfo()
{
    if ( m_pName )
    {
        DELETE_A( m_pName );
    }
}
TMySqlParam::~TMySqlParam()
{
    if ( m_pName )
    {
        DELETE_A( m_pName );
    }
    if ( m_IntArray )  
    {
        DELETE_A( m_IntArray );
    }
    if ( m_DBLArray )
    {
        DELETE_A( m_DBLArray );
    }
    if( m_LongArray )
    {
        DELETE_A( m_LongArray );
    }
    if( m_LLArray )
    {
        DELETE_A( m_LLArray );
    }
}

/*************************************************
*                                                *
*     ---------- TMySqlField ----------          *
*                                                *
*************************************************/
TMySqlField::TMySqlField()
{
    m_pParentQuery = NULL;
    m_pName = NULL;
    m_pDataBuf = NULL;
//    m_lDataBufLen = 0;
    m_lType  = 0;
//    m_iRetDataLen = 0 ;
    m_bNullable = false;
    memset(m_StrBuffer, 0, sizeof( m_StrBuffer ) );
    m_sBlob = NULL ;
    m_lBufLen = 0;
    m_bIsNull = false;
    m_StrLen = 0;
}

bool TMySqlField::isNULL()
{
    return m_bIsNull;
}

char*  TMySqlField::AsString() throw (TMySqlException)
{
    if( m_pParentQuery->m_bEofRow )
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"AsString, Result Dataset is on Bof \n");
    }

    if ( isNULL() )
    {
        snprintf((char *)m_StrBuffer,sizeof(m_StrBuffer),"%s", "");
        return (char *)m_StrBuffer;
    }
    switch ( m_lType )
    {
        case MYSQL_TYPE_TINY:
        {
            snprintf(m_StrBuffer,sizeof(m_StrBuffer),"%c", *(char *)m_pDataBuf);
            break;
        }
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_LONG :
        {
            snprintf(m_StrBuffer,sizeof(m_StrBuffer),"%d", *(int *)m_pDataBuf);
            break;
        }
        case MYSQL_TYPE_LONGLONG:
        case MYSQL_TYPE_NEWDECIMAL :
        {
            snprintf(m_StrBuffer,sizeof(m_StrBuffer),"%lld", *(long long *)m_pDataBuf);
            break;        
        }
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
        {
            snprintf(m_StrBuffer,sizeof(m_StrBuffer),"%lf", *(double *)m_pDataBuf);
            break;
        }
        case MYSQL_TYPE_STRING:   //字符串型
        case MYSQL_TYPE_VAR_STRING:
        {
            snprintf(m_StrBuffer,sizeof(m_StrBuffer),"%s", (char *)m_pDataBuf);
            break;
        }
        case MYSQL_TYPE_TIME :
        case MYSQL_TYPE_DATE :
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
        {
            MYSQL_TIME* pt = (MYSQL_TIME*)m_pDataBuf ;
            snprintf(m_StrBuffer,sizeof(m_StrBuffer), "%04u-%02u-%02u %02u:%02u:%02u ",pt->year, pt->month, pt->day,  pt->hour, pt->minute, pt->second );
            break;
        }
        case MYSQL_TYPE_BLOB:
        {
            snprintf(m_StrBuffer,sizeof(m_StrBuffer),"%s", "BLOB..." );
            break;
        }
        default:
        {
            throw TMySqlException(m_pParentQuery->m_pSqlStmt,"AsString() othre type not surpport !\n");
        }
    }
    return m_StrBuffer;
}
char *  TMySqlField::AsDateTimeString() throw (TMySqlException)
{
    if(  m_pParentQuery->m_bEofRow )
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"AsString, Result Dataset is on Bof \n");
    }
    if ( isNULL() )
    {
        snprintf((char *)m_StrBuffer,sizeof(m_StrBuffer),"%s", "");
        return (char *)m_StrBuffer;
    }
    /*
    if ( m_lType != MYSQL_TYPE_DATETIME && m_lType!=MYSQL_TYPE_TIMESTAMP)
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"Data type convertion error: field:%s data type:%d can not be access by %s", m_pName, m_lType, "AsTimeString()");
    }
    else
    {
        MYSQL_TIME* pt = (MYSQL_TIME*)m_pDataBuf ;
        snprintf(m_StrBuffer, sizeof(m_StrBuffer),"%04d-%02d-%02d %02d:%02d:%02d ",pt->year, pt->month, pt->day,  pt->hour, pt->minute, pt->second );
        return m_StrBuffer;
    }
    */
    switch ( m_lType )
    {
        case MYSQL_TYPE_DATETIME :
        case MYSQL_TYPE_TIMESTAMP:
        {
            MYSQL_TIME* pt = (MYSQL_TIME*)m_pDataBuf ;
            snprintf(m_StrBuffer, sizeof(m_StrBuffer),"%04u-%02u-%02u %02u:%02u:%02u ",pt->year, pt->month, pt->day,  pt->hour, pt->minute, pt->second );
            break;
        }
        case MYSQL_TYPE_DATE :
        {
            MYSQL_TIME* pt = (MYSQL_TIME*)m_pDataBuf ;
            snprintf(m_StrBuffer, sizeof(m_StrBuffer),"%04u-%02u-%02u 00:00:00 ",pt->year, pt->month, pt->day);  
            break;
        }
        case MYSQL_TYPE_TIME :
        {
            MYSQL_TIME* pt = (MYSQL_TIME*)m_pDataBuf ;
            snprintf(m_StrBuffer, sizeof(m_StrBuffer)," %02u:%02u:%02u ", pt->hour, pt->minute, pt->second );
            break;
        }
        default:
        {
            throw TMySqlException(m_pParentQuery->m_pSqlStmt,"Data type convertion error: field:%s data type:%ld can not be access by %s", m_pName, m_lType, "AsTimeString()");
        }
    }
    return m_StrBuffer; 
    
}
void   TMySqlField::AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TMySqlException)
{
    if(  m_pParentQuery->m_bEofRow )
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"AsString, Result Dataset is on Bof \n");
    }
    if ( isNULL() )
    {
        year = 0;
        month = 0;
        day = 0;
        hour = 0;
        minute = 0;
        second = 0;
        return;
    }
    /*
    if ( m_lType != MYSQL_TYPE_DATETIME)
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"Data type convertion error: field:%s data type:%d can not be access by %s", m_pName, m_lType, "AsTimeString()");
    }
    else
    {
        MYSQL_TIME* pt = (MYSQL_TIME*)m_pDataBuf ;
        year = pt->year;
        month = pt->month; 
        day = pt->day; 
        hour = pt->hour;
        minute = pt->minute; 
        second = pt->second ;
    }*/
    switch ( m_lType )
    {
        case MYSQL_TYPE_DATETIME :
        case MYSQL_TYPE_TIMESTAMP:
        {
            MYSQL_TIME* pt = (MYSQL_TIME*)m_pDataBuf ;
            year = pt->year;
            month = pt->month; 
            day = pt->day; 
            hour = pt->hour;
            minute = pt->minute; 
            second = pt->second ;
            break;
        }
        case MYSQL_TYPE_DATE :
        {
            MYSQL_TIME* pt = (MYSQL_TIME*)m_pDataBuf ;
            year = pt->year;
            month = pt->month; 
            day = pt->day; 
            hour = 0;
            minute = 0; 
            second = 0 ;
            break;
        }
        case MYSQL_TYPE_TIME :
        {
            MYSQL_TIME* pt = (MYSQL_TIME*)m_pDataBuf ;
            year = 0;
            month = 0; 
            day = 0; 
            hour = pt->hour;
            minute = pt->minute; 
            second = pt->second ;
            break;
        }
        default:
        {
            throw TMySqlException(m_pParentQuery->m_pSqlStmt,"Data type convertion error: field:%s data type:%ld can not be access by %s", m_pName, m_lType, "AsTimeString()");
        }
    }

}

double  TMySqlField::AsFloat() throw(TMySqlException)
{
    if(  m_pParentQuery->m_bEofRow )
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"AsString, Result Dataset is on Bof \n");
    }
    if ( isNULL() )
    {
        return 0;
    }
    switch( m_lType )
    {
        case MYSQL_TYPE_FLOAT : //单精度
        {
            return *(float* )m_pDataBuf;
        }
        case MYSQL_TYPE_DOUBLE :  //双精度
        {
            return *(double* )m_pDataBuf ;
        }
        case MYSQL_TYPE_LONG :  //整型
        {
            return *(int* )m_pDataBuf ;
        }
        case MYSQL_TYPE_LONGLONG : //整型
        {
            return (double)(*(long long* )m_pDataBuf) ;
        }
        default:
        {
            throw TMySqlException(m_pParentQuery->m_pSqlStmt,"Data type convertion error: field:%s data type:%ld can not be access by %s", m_pName, m_lType, "AsFloat()");
        }
    }
}
long long TMySqlField::AsInteger() throw(TMySqlException)
{
    if(  m_pParentQuery->m_bEofRow )
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"AsInteger, Result Dataset is on Bof \n");
    }
    if ( isNULL() )
    {
        return 0;
    }
    switch ( m_lType )
    {
        case MYSQL_TYPE_LONG :
        {
            return *(int* )m_pDataBuf ;
        }
        case MYSQL_TYPE_LONGLONG :
        case MYSQL_TYPE_NEWDECIMAL :
        {
            return *(long long* )m_pDataBuf ;
        }
        default:
        {
            throw TMySqlException(m_pParentQuery->m_pSqlStmt,"Data type convertion error: field:%s data type:%ld can not be access by %s", m_pName, m_lType, "AsInteger()");
        }
    }
}
time_t  TMySqlField::AsTimeT()throw (TMySqlException)
{
    if(  m_pParentQuery->m_bEofRow )
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"AsTimeT, Result Dataset is on Bof \n");
    }
    if ( isNULL() )
    {
        return 0;
    }
    if ( m_lType != MYSQL_TYPE_DATETIME)
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"Data type convertion error: field:%s data type:%ld can not be access by %s", m_pName, m_lType, "AsTimeT()");
    }
    else
    {
        MYSQL_TIME* pt = (MYSQL_TIME*)m_pDataBuf ;
        struct tm  TimeInfo;
        TimeInfo.tm_year =  static_cast<int>(pt->year  - 1900);
        TimeInfo.tm_mon =  static_cast<int>(pt->month - 1);
        TimeInfo.tm_mday = static_cast<int>(pt->day);
        TimeInfo.tm_hour = static_cast<int>(pt->hour);
        TimeInfo.tm_min = static_cast<int>(pt->minute);
        TimeInfo.tm_sec = static_cast<int>(pt->second);
        TimeInfo.tm_isdst =(int)-1;
        return  mktime( &TimeInfo );
    }
}

char* TMySqlField::AsBlobBuffer(int &iBufferLen)
{
    if(  m_pParentQuery->m_bEofRow )
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"AsBlobBuffer, The query has reached the last record \n");
    }
    if ( isNULL() )
    {
        return NULL;
    }
    if ( m_lType != MYSQL_TYPE_BLOB )
    {
        throw TMySqlException(m_pParentQuery->m_pSqlStmt,"Data type convertion error: field:%s data type:%ld can not be access by %s", m_pName, m_lType, "AsBlobBuffer()");
    }
    if( NULL != m_sBlob )
    {
        delete [] m_sBlob;
        m_sBlob = NULL;
    }
    m_sBlob = new char[m_lBufLen+1];
    memset(m_sBlob,0x00,m_lBufLen+1);
    memcpy( m_sBlob, m_pDataBuf, m_lBufLen );
    return m_sBlob;
}
long  TMySqlField::GetFieldType()
{
    return m_lType;
}

char*  TMySqlField::GetFieldName()
{
    return m_pName;
}
void  TMySqlField::ClearInfo()
{
    if( m_pDataBuf )
    {
        memset( m_pDataBuf, 0, static_cast<size_t>(m_lBufLen) );
    }
}
void TMySqlField::ClearDataBuf()
{
    switch  ( m_lType )
    {
        case MYSQL_TYPE_LONG :  //整型 int
        {
            int* pInt = static_cast<int*>(m_pDataBuf) ;
            DELETE_A( pInt );
            break;
        }
        case MYSQL_TYPE_LONGLONG : //long long 型
        case MYSQL_TYPE_NEWDECIMAL :
        {
            long long * pLLong=static_cast<long long*>(m_pDataBuf);
            DELETE_A( pLLong );
            break;
        }
        case MYSQL_TYPE_FLOAT :  //单精度浮点型
        {
            float* pFloat=static_cast<float*>(m_pDataBuf);
            DELETE_A( pFloat );
            break;
        }
        case MYSQL_TYPE_DOUBLE : //双精度浮点型
        {
            double* pDouble=static_cast<double *>(m_pDataBuf);
            DELETE_A( pDouble );
            break;
        }
        case MYSQL_TYPE_SHORT :  //短整型
        {
            short* pShort = static_cast<short *>(m_pDataBuf);
            DELETE_A( pShort );
            break;
        }
        case MYSQL_TYPE_TINY :
        case MYSQL_TYPE_BLOB :
        case MYSQL_TYPE_STRING : //字符串型
        case MYSQL_TYPE_VAR_STRING : //字符串型
        {
            char* pStr = static_cast<char *>(m_pDataBuf);
            DELETE_A( pStr );
            break ;
        }
        case MYSQL_TYPE_TIME :
        case MYSQL_TYPE_DATE :
        case MYSQL_TYPE_DATETIME  :
        case MYSQL_TYPE_TIMESTAMP : //日期时间型
        {
            MYSQL_TIME* pMySqlTime=static_cast<MYSQL_TIME *>(m_pDataBuf);
            DELETE_A( pMySqlTime );
            break;
        }
    }
    m_pDataBuf = NULL;
}

TMySqlField::~TMySqlField()
{
    if ( m_pName )
    {
        DELETE_A( m_pName );
    }
    ClearDataBuf();
    if ( m_sBlob )
    {
        DELETE_A( m_sBlob );
    }
    m_pParentQuery = NULL;
}



/*************************************************
*                                                *
*     ---------- TMySqlQuery ----------          *
*                                                *
*************************************************/

TBillLog* TMySqlQuery::gpBillLog=NULL; //578112

TMySqlQuery::TMySqlQuery( TMySqlDatabase* pTMySqlDb)
{
    if (! pTMySqlDb->m_bConnFlag)
    {
        pTMySqlDb->Connect();
    }

    if(! pTMySqlDb->m_bConnFlag)
    {
        throw TMySqlException("","TMySqlQuery( TMySqlDatabase* pTMySqlDb): Can not declare a TMySqlQuery when the database is not connected");
    }

    m_pTMySqlDb = pTMySqlDb;
    m_pHandleConn = m_pTMySqlDb -> m_pHandleConn;
    //m_pHandleStmt =  pMySqlInstance->fpMySqlStmtInit( m_pHandleConn ) ;
    //CheckError(" mysql_stmt_init " );

    m_pHandleStmt = NULL;
    m_pSqlStmt = NULL;
    m_pResult = NULL;
    m_pTFieldList = NULL;
    m_pTParmList = NULL;
    m_iParamCount = 0;
    m_iFieldNum = 0;
    m_ulPrefetchRows = 1;
    m_iRowAffected = 0;
    memset( aParamBind, 0, sizeof(aParamBind) );
    memset( aFieldBind, 0, sizeof(aFieldBind) );
    m_bEofRow = false;
}

void  TMySqlQuery::Close()
{
    if (m_pSqlStmt )
    {
        DELETE_A( m_pSqlStmt );
    }
    
    if (m_iFieldNum >0)
    {
        DELETE_A( m_pTFieldList );
    }
    
    if (m_iParamCount >0)
    {
        DELETE_A( m_pTParmList );
    }

    m_pResult = NULL;
    m_iFieldNum = 0;
    m_iParamCount = 0;
    memset( aParamBind, 0, sizeof(aParamBind) );
    memset( aFieldBind, 0, sizeof(aFieldBind) );
}
void  TMySqlQuery::SetSQL(const char *sSqlstmt, int iPreFetchRows) throw(TMySqlException)
{
    if (! m_pTMySqlDb->m_bConnFlag)
    {
        throw TMySqlException(sSqlstmt, "SetSQL(): can't set sqlstmt on disconnected Database");
    }
    //释放空间，新空间保存sql语句
    if( NULL != m_pSqlStmt )
    {
        delete [] m_pSqlStmt;
        m_pSqlStmt = NULL;
    }
    size_t iLen = strlen( sSqlstmt );
    m_pSqlStmt = new char[iLen + 1];
    if ( m_pSqlStmt == NULL )
    {
        throw TMySqlException(sSqlstmt,  "no more memory can be allocate when :SetSQL(), source code:%d",  __LINE__);
    }
    strncpy(m_pSqlStmt,sSqlstmt,iLen);
    m_pSqlStmt[iLen] = '\0';

    ParseSQL();
    //初始化预处理语句句柄
    m_pHandleStmt =  pMySqlInstance->fpMySqlStmtInit( m_pHandleConn ) ; //提到构造函数中做
    CheckError(" mysql_stmt_init " );
    
    SetPrefetch(iPreFetchRows);   
    
    //int mysql_stmt_prepare(MYSQL_STMT *stmt, const char *query, unsigned long length)
    pMySqlInstance->fpMySqlStmtPrepare( m_pHandleStmt, m_pSqlStmt, strlen(m_pSqlStmt) );
    CheckError(" mysql_stmt_prepare " );

}

void TMySqlQuery::ParseSQL()
{
    char *params[MAX_PARAMS_COUNT]={0};
    int iNumCh = 0; //参数所含字符的个数
    int iParamId = 0; //第几个参数
    int in_literal,nFlag = 0;
    char *pCh = m_pSqlStmt ;
    char *pPlaceholder = NULL;
    
    // Find and bind input variables for placeholders. 
    for (in_literal = 0 ; *pCh != 0; pCh++)
    {

        if (*pCh == '\'')
        {
            in_literal = ~in_literal;
        }
        if (*pCh == ':' && *(pCh+1) != '=' && !in_literal)
        {
            *pCh = '?';
            for ( iNumCh = 0, pPlaceholder = ++pCh ;  *pCh && (isalnum(*pCh) || *pCh == '_'); pCh++, iNumCh++);
            if(*pCh == 0)
            {
                nFlag = 1;
            }
            if ( iParamId > MAX_PARAMS_COUNT)
            {
                throw TMySqlException(" ","Error: param count execedes max numbers\n");
            }

            params[iParamId] = new char[iNumCh+1];
            strncpy(params[iParamId],(char *)pPlaceholder,iNumCh);
            params[iParamId][iNumCh] = '\0';
            
            memset(pPlaceholder,' ',iNumCh);

            //printf("pPlaceholder-1 = %s ,\n",pPlaceholder-1);

            memset(pPlaceholder,' ',iNumCh );
            iParamId++;
            if(nFlag == 1) break;
        }
    }
    m_iParamCount = iParamId ;
    if ( m_iParamCount>0 )
    {
        m_pTParmList = new TMySqlParam[m_iParamCount];

        for (int i=0; i<m_iParamCount; i++)
        {
            unsigned long iParamLen = strlen(params[i]);
            m_pTParmList[i].m_pName = new char[iParamLen+1];
            strncpy(m_pTParmList[i].m_pName, params[i], iParamLen);
            m_pTParmList[i].m_pName[iParamLen] = '\0';
            m_pTParmList[i].m_iParamIndex = i;
            DELETE_A(params[i]);
        }
    }
}

void TMySqlQuery::Open(int iPrefetchRows) throw(TMySqlException)
{
    // 初始化预处理语句句柄
    // m_pHandleStmt =  mysql_stmt_init( m_pHandleConn ) ;
    // CheckError(__FILE__,__LINE__," mysql_stmt_init " );

    // //int mysql_stmt_prepare(MYSQL_STMT *stmt, const char *query, unsigned long length)
    // mysql_stmt_prepare( m_pHandleStmt, m_pSqlStmt, strlen(m_pSqlStmt) );
    // CheckError(__FILE__,__LINE__," mysql_stmt_prepare " );
    SetPrefetch(iPrefetchRows);
    //绑定参数
    if( m_iParamCount > 0 )
    {
        pMySqlInstance->fpMySqlStmtBindParam( m_pHandleStmt, aParamBind );
        CheckError( "mysql_stmt_bind_param" );
    }

    //int mysql_stmt_execute(MYSQL_STMT *stmt)

    pMySqlInstance->fpMySqlStmtExecute( m_pHandleStmt );
    CheckError(" mysql_stmt_execute " );

    GetFieldsDef( );
}

bool  TMySqlQuery::Next() throw(TMySqlException)
{
    if( m_pTFieldList!=NULL )
    {
        for(int iIndex=0;iIndex<m_iFieldNum; iIndex++)
        {
            m_pTFieldList[iIndex].ClearInfo();
        }
    }
    int iRet = pMySqlInstance->fpMySqlStmtFetch( m_pHandleStmt );
    if ( iRet == MYSQL_NO_DATA ) //已到达最后一行
    {
        m_bEofRow = true;
        return false;
    }
    else
    {
        CheckError(" mysql_stmt_fetch " );
    }
    return true;
}

void TMySqlQuery::CheckError(const char* sInfo) throw (TMySqlException)
{
    //unsigned int mysql_stmt_errno(MYSQL_STMT *stmt)
    //const char *mysql_stmt_error(MYSQL_STMT *stmt)
    int iErrno = pMySqlInstance->fpMySqlStmtErrno( m_pHandleStmt );
    if ( iErrno != 0 )
    {
        char szErrMsg[4096]={'\0'}; //错误信息
        const char* pStr =pMySqlInstance->fpMySqlStmtError(m_pHandleStmt) ;
        strncpy(szErrMsg,pStr,sizeof(szErrMsg));
        szErrMsg[sizeof(szErrMsg)-1] = '\0';
        if( sInfo )
        {
            strcat(szErrMsg , sInfo );
        }
        throw TMySqlException( m_pSqlStmt,"Error info: Errno = %d,ErrMsg: %s\n",iErrno, szErrMsg);
    }
}
TMySqlParam& TMySqlQuery::Param(int iParamIndex)    
{
    if ( m_pSqlStmt == NULL )
    {
        throw TMySqlException("ParamByName()", "ParamByName(): sql statement is empty.");
    }
    if( iParamIndex < 0 
            || m_iParamCount < 1
            || iParamIndex >= m_iParamCount
      )
    {
        throw TMySqlException(m_pSqlStmt, "iParamIndex:%d does not exists.", iParamIndex);
    }
    else
    {
        return m_pTParmList[iParamIndex];
    }
}                                                 
TMySqlParam& TMySqlQuery::Param(const char*sParamName)
{
    bool bFound = false;
    int i=0;

    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException(sParamName, "ParamByName(): sql statement is empty.");
    }
    for(i=0; i<m_iParamCount; i++)
    {
        bFound = !TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName);
        if ( bFound )
        {
            break;
        }
    }
    if ( bFound ) 
    {
        return m_pTParmList[i];
    }
    else 
    {
        throw TMySqlException(m_pSqlStmt, "param:%s does not exists.", sParamName);
    }
}

TMySqlParam* TMySqlQuery::ParamByName(const char *sParamName)
{
    TMySqlParam *pParam  = NULL;
    bool bFound = false;
    int i=0;

    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException(sParamName, "ParamByName(): sql statement is empty.");
    }
    for(i=0; i<m_iParamCount; i++)
    {
        bFound = !TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName);
        if ( bFound )
        {
            break;
        }
    }
    if ( bFound ) 
    {
        pParam = &m_pTParmList[i];
    }
    else 
    {
        throw TMySqlException(m_pSqlStmt, "param:%s does not exists.", sParamName);
    }
    return pParam;    
}

TMySqlParam* TMySqlQuery::ParamByIndex(int iParamIndex)
{
    TMySqlParam *pParam = NULL;
    if ( m_pSqlStmt == NULL )
    {
        throw TMySqlException("ParamByName()", "ParamByName(): sql statement is empty.");
    }
    if( iParamIndex < 0 
            || m_iParamCount < 1
            || iParamIndex >= m_iParamCount
      )
    {
        throw TMySqlException(m_pSqlStmt, "iParamIndex:%d does not exists.", iParamIndex);
    }
    else
    {
        pParam = &m_pTParmList[iParamIndex];
    }
    return pParam;
}

bool TMySqlQuery::IsFieldExist(const char *sFieldName)
{
    for(int i=0; i<m_iFieldNum; i++)
    {
        bool bFound = false;
        bFound = !TStrFunc::StrNoCaseCmp(m_pTFieldList[i].m_pName,sFieldName);
        if ( bFound )
        {
            return true;
        }
    }
    return false;
}

int TMySqlQuery::ParamCount() 
{
    return m_iParamCount;
} 

bool TMySqlQuery::IsParamExist(const char *sParamName)
{
    for(int i=0; i<m_iParamCount; i++)
    {
        if (!TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName))
        {
            return true;
        }
    }
    return false;
}
void TMySqlQuery::SetPrefetch( int iPreFetchRows ) throw (TMySqlException)
{
    if( iPreFetchRows> 1)
    {
        m_ulPrefetchRows = iPreFetchRows;
        unsigned long type= (unsigned long) CURSOR_TYPE_READ_ONLY;
        pMySqlInstance->fpMySqlStmtAttrSet(m_pHandleStmt, STMT_ATTR_CURSOR_TYPE, (void*) &type);
        CheckError(" mysql_stmt_attr_set " );
        pMySqlInstance->fpMySqlStmtAttrSet(m_pHandleStmt, STMT_ATTR_PREFETCH_ROWS,(void*) &m_ulPrefetchRows);
        CheckError(" mysql_stmt_attr_set " );
    }    
}
bool   TMySqlQuery::Eof()
{
    return m_bEofRow;
}
int    TMySqlQuery::FieldCount()
{
    return m_iFieldNum;
}
TMySqlField&  TMySqlQuery::Field(int iIndex) throw(TMySqlException)
{
    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException("", "Field(iIndex): sql statement is not presented");
    }

    if ( (iIndex>=0) && (iIndex<m_iFieldNum) )
    {
        return m_pTFieldList[iIndex];
    }
    else
    {
        throw TMySqlException( m_pSqlStmt , "field index out of bound when call Field(iIndex)");
    }
}
TMySqlField&  TMySqlQuery::Field(const char *sFieldName) throw(TMySqlException)
{
    bool bFound = false;
    int i=0;
    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException("", "Field(*sFieldName): sql statement is not presented");
    }

    //if (! fOpened)
    //    throw TDBException(m_pSqlStmt, ERR_GENERAL, "can not access field before open");

    for(; i<m_iFieldNum; i++)
    {
        bFound = !TStrFunc::StrNoCaseCmp(Field(i).m_pName,sFieldName);
        if ( bFound )
        {
            break;
        }
    }
    if ( bFound )
    {
        return m_pTFieldList[i];
    }
    else
    {
        throw TMySqlException(m_pSqlStmt, "field:%s does not exists.", sFieldName);
    }

}
/*begin zmp 841469 */
void TMySqlQuery::GetValue(void *pStruct,int* Column)throw (TMySqlException)
{
    throw TMySqlException("TMySqlException::GetValue","Not Support!");
}
void TMySqlQuery::SetUpdateMDBFlag(bool flag)throw (TMySqlException)
{
    ;
}
bool TMySqlQuery::GetUpdateMDBFlag()throw (TMySqlException)
{
    return true;
}

/*end zmp 841469 */

void TMySqlQuery::SetParameter(const char *sParamName,const char* sParamValue, bool isOutput_Unused ) throw (TMySqlException)
{
    //TMySqlParam *pParam = ParamByName( sParamName );
    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException(sParamName, "SetParameter(const char*,const char*,bool): sql statement is empty.");
    }
    bool bFound = false ;
    for(int i=0; i<m_iParamCount; i++)
    {
        if ( !TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName) )
        {
            bFound = true;
            if(sParamValue[0] != '\0')
            {
                int iParamIndex = m_pTParmList[i].m_iParamIndex;
                aParamBind[iParamIndex].buffer_type = MYSQL_TYPE_STRING;
                DELETE_A( m_pTParmList[i].m_StrBuf );
                unsigned long iLen = strlen(sParamValue);
                m_pTParmList[i].m_StrBuf = new char[iLen+1];
                aParamBind[iParamIndex].buffer = m_pTParmList[i].m_StrBuf;
                aParamBind[iParamIndex].buffer_length= iLen; //最多容纳多少字符
                aParamBind[iParamIndex].is_null = 0;
                strncpy((char *)m_pTParmList[i].m_StrBuf,sParamValue,iLen);
                m_pTParmList[i].m_StrBuf[iLen] = '\0';
            }
            else
            {
                SetParameterNULL(sParamName);
            }
        }
    }
    if( false == bFound )
    {
        throw TMySqlException(m_pSqlStmt, "param:%s does not exists.", sParamName);
    } 
}
void TMySqlQuery::SetParameter(const char *sParamName, const char paramValue, bool isOutput) throw (TMySqlException) 
{
    char aStrParamValue[2] = {0};
    aStrParamValue[0] = paramValue ;
    aStrParamValue[1] = '\0';
    SetParameter(sParamName,aStrParamValue,isOutput);
}

void TMySqlQuery::SetParameter(const char *sParamName,int iParamValue, bool isOutput_Unused ) throw (TMySqlException)
{
    //TMySqlParam *pParam = ParamByName( sParamName );
    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException(sParamName, "SetParameter(const char *, int,bool): sql statement is empty.");
    }
    bool bFound = false;
    for(int i=0; i<m_iParamCount; i++)
    {
        if ( !TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName) )
        {
            bFound = true;
            int iParamIndex = m_pTParmList[i].m_iParamIndex;
            m_pTParmList[i].m_iIntBuf = iParamValue;
            aParamBind[iParamIndex].buffer_type = MYSQL_TYPE_LONG;
            aParamBind[iParamIndex].buffer = (char*)(&m_pTParmList[i].m_iIntBuf);
        }
    }
    if( false == bFound )
    {
        throw TMySqlException(m_pSqlStmt, "param:%s does not exists.", sParamName);
    }

}
void TMySqlQuery::SetParameter(const char *sParamName,long lParamValue, bool isOutput_Unused ) throw (TMySqlException)
{
    //TMySqlParam *pParam = ParamByName( sParamName );
    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException(sParamName, "SetParameter(const char *, long,bool): sql statement is empty.");
    }
    bool bFound = false;
    for(int i=0; i<m_iParamCount; i++)
    {
        if ( !TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName) )
        {
            bFound = true;
            int iParamIndex = m_pTParmList[i].m_iParamIndex;
            m_pTParmList[i].m_lLongBuf = lParamValue;
            aParamBind[iParamIndex].buffer_type = MYSQL_TYPE_LONG;
            aParamBind[iParamIndex].buffer = (char*)(&m_pTParmList[i].m_lLongBuf);
            aParamBind[iParamIndex].is_null = 0; 
        }
    }
    if( false == bFound )
    {
        throw TMySqlException(m_pSqlStmt, "param:%s does not exists.", sParamName);
    }   
}
void TMySqlQuery::SetParameter(const char *sParamName,double dParamValue, bool isOutput_Unused ) throw (TMySqlException)
{
    //TMySqlParam *pParam = ParamByName( sParamName ); 
    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException(sParamName, "SetParameter(const char *, int,bool): sql statement is empty.");
    }
    bool bFound = false;
    for(int i=0; i<m_iParamCount; i++)
    {
        if ( !TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName) )
        {
            bFound = true;
            int iParamIndex = m_pTParmList[i].m_iParamIndex;
            m_pTParmList[i].m_dblBuf = dParamValue;
            aParamBind[iParamIndex].buffer_type = MYSQL_TYPE_DOUBLE;
            aParamBind[iParamIndex].buffer = (char*)(&m_pTParmList[i].m_dblBuf);
            aParamBind[iParamIndex].is_null = 0;
        }
    }
    if( false == bFound )
    {
        throw TMySqlException(m_pSqlStmt, "param:%s does not exists.", sParamName);
    } 
}
void TMySqlQuery::SetParameter(const char *sParamName,long long llParamValue, bool isOutput_Unused ) throw (TMySqlException)
{
    //TMySqlParam *pParam = ParamByName( sParamName );
    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException(sParamName, "SetParameter(const char *, long long,bool): sql statement is empty.");
    }
    bool bFound = false;
    for(int i=0; i<m_iParamCount; i++)
    {
        if ( !TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName) )
        {
            bFound = true;
            int iParamIndex = m_pTParmList[i].m_iParamIndex;
            m_pTParmList[i].m_llBuf = llParamValue;
            aParamBind[iParamIndex].buffer_type = MYSQL_TYPE_LONGLONG;
            aParamBind[iParamIndex].buffer = (char*)(&m_pTParmList[i].m_llBuf);
            aParamBind[iParamIndex].is_null = 0;    
        }
    }
    if( false == bFound )
    {
        throw TMySqlException(m_pSqlStmt, "param:%s does not exists.", sParamName);
    } 

}
void TMySqlQuery::SetParameter(const char *sParamName,const char* sParamValue,int iBufferLen, bool isOutput_Unused ) throw (TMySqlException)
{
    //TMySqlParam *pParam = ParamByName( sParamName );
    
    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException(sParamName, "SetParameter(const char *,const char*, int,bool): sql statement is empty.");
    }
    bool bFound = false;
    for(int i=0; i<m_iParamCount; i++)
    {
        if ( !TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName) )
        {
            bFound = true;
            if((sParamValue==NULL)
                    ||(iBufferLen<=0)
              )
            {
                SetParameterNULL( sParamValue );
            }
            else
            {
                int iParamIndex = m_pTParmList[i].m_iParamIndex;
                aParamBind[iParamIndex].buffer_type = MYSQL_TYPE_STRING;
                DELETE_A( m_pTParmList[i].m_StrBuf );
                int iLen = iBufferLen;
                m_pTParmList[i].m_StrBuf = new char[iLen+1];
                aParamBind[iParamIndex].buffer = m_pTParmList[i].m_StrBuf;
                aParamBind[iParamIndex].buffer_length= iLen; //最多容纳多少字符
                aParamBind[iParamIndex].is_null = 0;
                strncpy((char *)m_pTParmList[i].m_StrBuf,sParamValue,iLen);
                m_pTParmList[i].m_StrBuf[iLen] = '\0';
            }   
        }
    }
    if( false == bFound )
    {
        throw TMySqlException(m_pSqlStmt, "param:%s does not exists.", sParamName);
    }   
}
void TMySqlQuery::SetParameterNULL(const char *sParamName) throw (TMySqlException)  //设置参数为空
{
    //TMySqlParam *pParam = ParamByName( sParamName );
    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException(sParamName, "SetParameterNULL(const char *): sql statement is empty.");
    }
    bool bFound = false;
    for(int i=0; i<m_iParamCount; i++)
    {
        if ( !TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName) )
        {
            bFound = true;
            int iParamIndex = m_pTParmList[i].m_iParamIndex;
            m_pTParmList[i].m_bNullIndictor =true;
            aParamBind[iParamIndex].is_null = &(m_pTParmList[i].m_bNullIndictor) ; 
        }
    }
    if( false == bFound )
    {
        throw TMySqlException(m_pSqlStmt, "param:%s does not exists.", sParamName);
    }        

}
void TMySqlQuery::SetParameterTime_t(const char *sParamName,time_t tParamValue,bool isOutput_Unused ) throw (TMySqlException)
{
    //TMySqlParam *pParam = ParamByName( sParamName ); 
    if (m_pSqlStmt == NULL)
    {
        throw TMySqlException(sParamName, "SetParameter(const char *,time_t, int,bool): sql statement is empty.");
    }
    bool bFound = false;
    for(int i=0; i<m_iParamCount; i++)
    {
        if ( !TStrFunc::StrNoCaseCmp(m_pTParmList[i].m_pName,sParamName) )
        {
            bFound = true;
            
            int iParamIndex = m_pTParmList[i].m_iParamIndex;
            aParamBind[iParamIndex].buffer_type = MYSQL_TYPE_STRING;
            DELETE_A( m_pTParmList[i].m_StrBuf );
            
            char aParamValue[40]={'\0'};
            struct tm * tm_Cur;
            struct tm tmLocalTime;
            tm_Cur = localtime_r(&tParamValue,&tmLocalTime);
            snprintf(aParamValue,sizeof(aParamValue),"%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
            aParamValue[sizeof(aParamValue)-1]='\0';
            
            unsigned long iLen = strlen(aParamValue);
            m_pTParmList[i].m_StrBuf = new char[iLen+1];
            aParamBind[iParamIndex].buffer = m_pTParmList[i].m_StrBuf;
            aParamBind[iParamIndex].buffer_length= iLen; //最多容纳多少字符    
            strncpy( m_pTParmList[i].m_StrBuf,aParamValue,iLen);
            m_pTParmList[i].m_StrBuf[iLen] = '\0';

        }
    }
    if( false == bFound )
    {
        throw TMySqlException(m_pSqlStmt, "param:%s does not exists.", sParamName);
    }     
}

//简化
void TMySqlQuery::SetParameter(int iParamIndex,const char* sParamValue) throw (TMySqlException)
{
    TMySqlParam *pParam = ParamByIndex( iParamIndex );
    aParamBind[iParamIndex].buffer_type = MYSQL_TYPE_STRING;
    DELETE_A( pParam->m_StrBuf );
    
    if(sParamValue != NULL)
    {
        unsigned long iLen = strlen(sParamValue);
        pParam->m_StrBuf = new char[iLen+1];
        aParamBind[iParamIndex].buffer = pParam->m_StrBuf;
        aParamBind[iParamIndex].buffer_length= iLen; //最多容纳多少字符
        aParamBind[iParamIndex].is_null = 0;
        strncpy((char *)pParam->m_StrBuf,sParamValue,iLen);
        pParam->m_StrBuf[iLen] = '\0';
    }
    else
    {
        SetParameterNULL( iParamIndex );
    }    
}
void TMySqlQuery::SetParameter(int iParamIndex,int iParamValue) throw (TMySqlException)
{
    TMySqlParam *pParam = ParamByIndex( iParamIndex );
    int iIndex = pParam->m_iParamIndex;
    pParam->m_iIntBuf = iParamValue;
    aParamBind[iIndex].buffer_type = MYSQL_TYPE_LONG;
    aParamBind[iIndex].buffer = (char*)(&pParam->m_iIntBuf);
    aParamBind[iIndex].is_null = 0;    
}
void TMySqlQuery::SetParameter(int iParamIndex,long lParamValue) throw (TMySqlException)
{
    TMySqlParam *pParam = ParamByIndex( iParamIndex );
    int iIndex = pParam->m_iParamIndex;
    pParam->m_lLongBuf = lParamValue;
    aParamBind[iIndex].buffer_type = MYSQL_TYPE_LONG;
    aParamBind[iIndex].buffer = (char*)(&pParam->m_lLongBuf);
    aParamBind[iIndex].is_null = 0;    
}
void TMySqlQuery::SetParameter(int iParamIndex,double dParamValue) throw (TMySqlException)
{
    TMySqlParam *pParam = ParamByIndex( iParamIndex );
    int iIndex = pParam->m_iParamIndex;
    pParam->m_dblBuf = dParamValue;
    aParamBind[iIndex].buffer_type = MYSQL_TYPE_DOUBLE;
    aParamBind[iIndex].buffer = (char*)(&pParam->m_dblBuf);
    aParamBind[iIndex].is_null = 0;
}
void TMySqlQuery::SetParameter(int iParamIndex,long long llParamValue) throw (TMySqlException)
{
    TMySqlParam *pParam = ParamByIndex( iParamIndex );
    int iIndex = pParam->m_iParamIndex;
    pParam->m_llBuf = llParamValue;
    aParamBind[iIndex].buffer_type = MYSQL_TYPE_LONGLONG;
    aParamBind[iIndex].buffer = (char*)(&pParam->m_llBuf);
    aParamBind[iIndex].is_null = 0;    
}
void TMySqlQuery::SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TMySqlException)//用于传入BLOB/BINARY类型字段
{
    TMySqlParam *pParam = ParamByIndex( iParamIndex );
    int iIndex = pParam->m_iParamIndex;
    aParamBind[iIndex].buffer_type = MYSQL_TYPE_STRING;
    DELETE_A( pParam->m_StrBuf );

    if((sParamValue==NULL)
            ||(iBufferLen<=0)
      )
    {
        SetParameterNULL( sParamValue );
    }
    else
    {
        int iLen = iBufferLen;
        pParam->m_StrBuf = new char[iLen+1];
        aParamBind[iIndex].buffer = pParam->m_StrBuf;
        aParamBind[iIndex].buffer_length= iLen; //最多容纳多少字符
        aParamBind[iIndex].is_null = 0;
        strncpy((char *)pParam->m_StrBuf,sParamValue,iLen);
        pParam->m_StrBuf[iLen] = '\0';
    }    
}
void TMySqlQuery::SetParameterNULL(int iParamIndex) throw (TMySqlException)     //设置参数为空
{
    TMySqlParam *pParam = ParamByIndex( iParamIndex );
    int iIndex = pParam->m_iParamIndex;
    pParam->m_bNullIndictor =true;
    aParamBind[iIndex].is_null = &(pParam->m_bNullIndictor) ;
}

void TMySqlQuery::SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TMySqlException)
{
    TMySqlParam *pParam = ParamByIndex( iParamIndex );
    int iIndex = pParam->m_iParamIndex;
    aParamBind[iIndex].buffer_type = MYSQL_TYPE_STRING;
    DELETE_A( pParam->m_StrBuf );
    
    char aParamValue[40]={'\0'};
    memset(aParamValue,0x00,sizeof(aParamValue));
    struct tm * tm_Cur;
    tm_Cur = localtime(&tParamValue);
    snprintf(aParamValue,sizeof(aParamValue),"%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
    aParamValue[sizeof(aParamValue)-1]='\0';

    unsigned long iLen = strlen(aParamValue);
    pParam->m_StrBuf = new char[iLen+1];
    aParamBind[iIndex].buffer = pParam->m_StrBuf;
    aParamBind[iIndex].buffer_length= iLen; //最多容纳多少字符    
    strncpy( pParam->m_StrBuf,aParamValue,iLen);
    pParam->m_StrBuf[iLen] = '\0';    
}

void TMySqlQuery::GetFieldsDef( ) throw(TMySqlException)
{
    if ( m_iFieldNum >0 )
    {
        DELETE_A(m_pTFieldList);
    }

    m_pResult = pMySqlInstance->fpMySqlStmtResultMetadata( m_pHandleStmt );
    CheckError("mysql_stmt_result_metadata" );

    //获取元数据信息
    //MYSQL_RES *mysql_stmt_result_metadata(MYSQL_STMT *stmt)
    //MYSQL_RES * m_pResult = mysql_stmt_result_metadata( m_pHandleStmt );
    //m_iFieldNum = mysql_num_fields( m_pResult );

    m_iFieldNum = static_cast<int>(pMySqlInstance->fpMySqlStmtFieldCount( m_pHandleStmt ));
    CheckError( "mysql_stmt_field_count" );

    //m_iFieldNum =  mysql_num_fields( m_pResult );
    //CheckError(__FILE__,__LINE__, "mysql_num_fields" );
    m_pTFieldList = new TMySqlField[ m_iFieldNum ];
    TMySqlField* pCurrField = NULL;


    for(int iColIndex = 0; iColIndex < m_iFieldNum ; iColIndex ++)
    {
        MYSQL_FIELD *pField = pMySqlInstance->fpMySqlFetchFieldDirect(m_pResult,static_cast<int>(iColIndex));
        CheckError("mysql_fetch_field_direct");
        //int iColNameLen = pField -> length;
        size_t iColNameLen = strlen( pField -> name );

        pCurrField = &m_pTFieldList[iColIndex];

        //获取字段的名称
        pCurrField->m_pName = new char[iColNameLen+1];
        strncpy( pCurrField->m_pName, pField->name,iColNameLen);
        pCurrField->m_pName[iColNameLen] = '\0';

        //得到字段的类型
        pCurrField->m_lType = pField->type;
        pCurrField->m_bNullable = pField->type == MYSQL_TYPE_NULL;

        //此字段对应的查询对象
        pCurrField->m_pParentQuery =  this;

        //pCurrField->m_precision = static_cast<int>(pField->length); //字段宽度
        // pCurrField->m_scale = static_cast<int>(pField->decimals); //小数点的个数

        switch  ( pCurrField->m_lType )
        {
            case MYSQL_TYPE_LONG :  //整型 int
            {
                pCurrField->m_pDataBuf = new int;
                pCurrField->m_lBufLen = sizeof(int);
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_LONG;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                break ;
            }
            case MYSQL_TYPE_LONGLONG : //long long 型
            {
                pCurrField->m_pDataBuf = new long long;
                pCurrField->m_lBufLen = sizeof(long long);
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_LONGLONG;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                break ;
            }
            case MYSQL_TYPE_FLOAT :  //单精度浮点型
            {
                pCurrField->m_pDataBuf = new float;
                pCurrField->m_lBufLen = sizeof( float );
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_FLOAT;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                break ;
            }
            case MYSQL_TYPE_DOUBLE : //双精度浮点型
            {
                pCurrField->m_pDataBuf = new double;
                pCurrField->m_lBufLen = sizeof( double );
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_DOUBLE;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                break ;
            }
            case MYSQL_TYPE_NEWDECIMAL :  
            {
                pCurrField->m_pDataBuf = new long;
                pCurrField->m_lBufLen = sizeof( long long );
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_LONGLONG;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                break ;
            }
            case MYSQL_TYPE_SHORT :  //短整型
            {
                pCurrField->m_pDataBuf = new short;
                pCurrField->m_lBufLen = sizeof( short );
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_SHORT;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                break ;
            }
            case MYSQL_TYPE_TINY :
            {
                pCurrField->m_pDataBuf = new char[1];
                pCurrField->m_lBufLen = sizeof( char );
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_TINY;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                break;
            }
            case MYSQL_TYPE_STRING : //字符串型
            {
                unsigned long  iLen = pField->length;
                pCurrField->m_pDataBuf = new char[iLen+1];
                pCurrField->m_lBufLen = iLen+1 ;
                memset(pCurrField->m_pDataBuf,0,iLen+1);
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_STRING;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                aFieldBind[iColIndex].buffer_length = iLen;
                aFieldBind[iColIndex].length = &pCurrField->m_StrLen;
                break ;
            }
            case MYSQL_TYPE_VAR_STRING : //字符串型
            {
                unsigned long iLen = pField->length;
                pCurrField->m_pDataBuf = new char[iLen+1];
                pCurrField->m_lBufLen = iLen+1;
                memset(pCurrField->m_pDataBuf,0,iLen+1);
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_VAR_STRING;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                aFieldBind[iColIndex].buffer_length = iLen;
                break ;
            }
            case MYSQL_TYPE_BLOB : //blob 型
            {
                unsigned long iBuffLen = pMySqlInstance->fpMySqlFetchLengths(m_pResult)[iColIndex];
                pCurrField->m_lBufLen = iBuffLen;
                pCurrField->m_pDataBuf = new char[iBuffLen +1] ;
                memset(pCurrField->m_pDataBuf,0x00,iBuffLen+1);
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_BLOB;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                break;
            }
            case MYSQL_TYPE_TIME :
            case MYSQL_TYPE_DATE :
            case MYSQL_TYPE_DATETIME  :
            case MYSQL_TYPE_TIMESTAMP : //日期时间型
            {
                pCurrField->m_pDataBuf = new MYSQL_TIME ;
                pCurrField->m_lBufLen = sizeof( MYSQL_TIME );
                aFieldBind[iColIndex].buffer_type = MYSQL_TYPE_TIMESTAMP;
                aFieldBind[iColIndex].buffer= (char *)(pCurrField->m_pDataBuf);
                aFieldBind[iColIndex].is_null = &pCurrField->m_bIsNull;
                break ;
            }
            default:
            {
                throw TMySqlException(m_pSqlStmt, "field:%s, datatype:%d not yet supported",pCurrField->m_pName,pCurrField->m_lType);
            }
        }
    }
    //绑定缓冲区
    pMySqlInstance->fpMySqlStmtBindResult( m_pHandleStmt, aFieldBind ) ;
    CheckError("mysql_stmt_bind_result" );
    pMySqlInstance->fpMySqlStmtStoreResult( m_pHandleStmt ) ;
    CheckError( "mysql_stmt_store_result" );
}

bool TMySqlQuery::Execute(int iters) throw(TMySqlException)
{
    if ( m_pSqlStmt == NULL )
    {
        throw TMySqlException(" ","Execute(): sql statement is not presented");
    }
    // if  ( m_StmtType == OCI_STMT_SELECT)
    // {
    // throw TMySqlException( m_pSqlStmt, ERR_GENERAL, "Execute(): Can't Execute a select statement.");
    // }

    //绑定参数
    if( m_iParamCount > 0 )
    {
        pMySqlInstance->fpMySqlStmtBindParam( m_pHandleStmt, aParamBind );
        CheckError( "mysql_stmt_bind_param" );
    }

    //int mysql_stmt_execute(MYSQL_STMT *stmt)
    pMySqlInstance->fpMySqlStmtExecute( m_pHandleStmt );
    CheckError("mysql_stmt_execute" );
    m_iRowAffected = static_cast<int>(pMySqlInstance->fpMySqlStmtNumRows( m_pHandleStmt ));
    CheckError( "mysql_affected_rows" );
    return true;
}
int  TMySqlQuery::RowsAffected()
{
    return m_iRowAffected;
}

bool TMySqlQuery::Commit()
{
    m_pTMySqlDb->Commit();
    return true;
}
bool TMySqlQuery::Rollback()
{
    m_pTMySqlDb->Rollback();
    return true;
}
void TMySqlQuery::SetParamArray(const char *paramName, char ** paramValue,int iStructSize,int iStrSize ,int iArraySize,bool isOutput) throw (TMySqlException)
{
    throw TMySqlException(m_pSqlStmt, "MySql doesn't support builk operation ");
}
void TMySqlQuery::SetParamArray(const char *paramName, int  * paramValue, int iStructSize,   int iArraySize,bool isOutput ) throw (TMySqlException)
{
    throw TMySqlException(m_pSqlStmt, "MySql doesn't support builk operation ");
}
void TMySqlQuery::SetParamArray(const char *paramName, double * paramValue, int iStructSize, int iArraySize,bool isOutput ) throw (TMySqlException)
{
    throw TMySqlException(m_pSqlStmt, "MySql doesn't support builk operation ");
}
void TMySqlQuery::SetParamArray(const char *paramName, long   * paramValue, int iStructSize,  int iArraySize,bool isOutput) throw (TMySqlException)
{
    throw TMySqlException(m_pSqlStmt, "MySql doesn't support builk operation ");
}
void TMySqlQuery::SetParamArray(const char *paramName, long long * paramValue, int iStructSize,  int iArraySize,bool isOutput) throw (TMySqlException)
{
    throw TMySqlException(m_pSqlStmt, "MySql doesn't support builk operation ");
}
void TMySqlQuery::SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize,bool isOutput) throw (TMySqlException)
{
    throw TMySqlException(m_pSqlStmt, "MySql doesn't support builk operation ");
}
//ZMP_559430
long long TMySqlQuery::GetSequenceByName(const char * sName)throw (TMySqlException)
{
    throw TMySqlException(m_pSqlStmt, "MySql doesn't support TMySqlQuery::GetSequenceByName() now. ");
}
void TMySqlQuery::SetBillLog(TBillLog *pBillLog)
{
    gpBillLog = pBillLog;
}
TMySqlQuery::~TMySqlQuery()
{

    DELETE_A(m_pSqlStmt);

    if (m_iFieldNum >0)
    {
        DELETE_A(m_pTFieldList);
    }
    if( m_iParamCount>0 )
    {
        DELETE_A(m_pTParmList);
    }

    //释放由mysql_store_result()、mysql_use_result()、mysql_list_dbs()等为结果集分配的内存
    if( m_pResult )
    {
        pMySqlInstance->fpMySqlFreeResult( m_pResult );
        m_pResult = NULL;
    }
    if( m_pHandleStmt )
    {
        pMySqlInstance->fpMySqlStmtClose(m_pHandleStmt);
        m_pHandleStmt = NULL;
    }
}


