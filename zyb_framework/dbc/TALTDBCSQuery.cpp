#include "TALTDBCSQuery.h"

bool TOdbcDBCSDatabase::Connect(bool bIsAutoCommit) throw (TOdbcDBException)
{
    m_iConnectFlag=0;
    m_bAutoCommitFlag=bIsAutoCommit;

    rc=SQLAllocConnect(henv,&hdbc);
    CheckError("TOdbcDBDatabase::SQLAllocConnect");

    char NLS[20] = "US7ASCII";   // NLS_USE ( KO16KSC5601, US7ASCII )
    char ConnStr[1024] = "";


	//远程连接
	sprintf(ConnStr,
		"DSN=%s;PORT_NO=20300;UID=%s;PWD=%s;CONNTYPE=%d;NLS_USE=%s",
		m_strServerName.c_str(),m_strUser.c_str(), m_strPassword.c_str(), 1, NLS);

    printf("connect string : %s\n",ConnStr);

    /* establish connection   : "DSN=XXX.XXX.XXX.XXX;UID=alt1;PWD=smart;CONNTYPE=3;NLS_USE=US7ASCII" */
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

	//zl add for altibase fast commit and rollback begin
	rc = SQLAllocStmt(hdbc, &m_stmtCommit); 
	CheckError("TOdbcDBDatabase::Connect m_stmtCommit");
	rc = SQLPrepare(m_stmtCommit, (SQLCHAR *)"commit", SQL_NTS);
	CheckError("TOdbcDBDatabase::Connect commit");
	
	rc = SQLAllocStmt(hdbc, &m_stmtRollback); 
	CheckError("TOdbcDBDatabase::Connect m_stmtRollback");
	rc = SQLPrepare(m_stmtRollback, (SQLCHAR *)"rollback;", SQL_NTS);
	CheckError("TOdbcDBDatabase::Connect rollback");
	//zl add for altibase fast commit and rollback end
    //连接上就立刻提交内存数据库2008-1-14 10:47AM zhangyu
    
    Commit();

    return true;
}


TOdbcDBCSQuery::TOdbcDBCSQuery(TOdbcDBCSDatabase *pTOdbcDBCSDatabase):TOdbcDBQuery(pTOdbcDBCSDatabase)
{
	
}


TOdbcDBCSQuery::~TOdbcDBCSQuery()
{

}


