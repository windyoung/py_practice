//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#include "TQMDBCSQuery.h"
#include "debug_new.h"
//ZMP:467365   
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

class TOdbcDBCSQuery;
bool TOdbcDBCSQuery::m_bUpdateMDBFlag=true;/*判断数据库是否允许进行更新*/
/*类TOdbcDBException定义部分*/
/*
TOdbcDBException::TOdbcDBException(const char *sSql, const char* sFormat, ...)
{
    va_list args;

    memset(m_sErrMsg,0,sizeof(m_sErrMsg));
    memset(m_sErrSql,0,sizeof(m_sErrSql));
    m_lErrCode=0;

    va_start(args,sFormat); 
    vsprintf(m_sErrMsg,sFormat,args);
    va_end(args);
    m_sErrMsg[sizeof(m_sErrMsg)-1]='\0';    

    strncpy(m_sErrSql,sSql,sizeof(m_sErrSql)-1);
    m_sErrSql[sizeof(m_sErrSql)-1]='\0';
}
*/

/*
//比较2个字符串是否相同(不考虑大小写)
bool inline CompareStrNoCase(const char *ori, const char *des)
{
    int j,nLen1,nLen2;
    bool sameChar;
    nLen1 = strlen(ori);
    nLen2 = strlen(des);
    if (nLen1!=nLen2) return false;
    sameChar = true;

    for (j=0; j<nLen1 && sameChar; j++)
        sameChar =(toupper(ori[j]) == toupper(des[j]));
    return sameChar;
}
*/
/********* TConnection implementation *********/
TOdbcDBCSDatabase::TOdbcDBCSDatabase() throw (TOdbcDBException)
{
    try
    {
       m_pMdbDatabase = new TMdbClientDatabase();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

TOdbcDBCSDatabase::~TOdbcDBCSDatabase()
{
    /*
    try
    {
        if(m_pMdbDatabase!=NULL)
        {
            delete m_pMdbDatabase;
            m_pMdbDatabase=NULL;
        }
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
    */
    
    //ZMP:305593,C++不建议析构函数中抛出异常，此处也不会抛出异常
    if (m_pMdbDatabase != NULL)
    {
        delete m_pMdbDatabase;
        m_pMdbDatabase = NULL;
    } 
}

void TOdbcDBCSDatabase::SetLogin(const char *user, const char *password, const char *tnsString) throw (TOdbcDBException)
{
    try
    {
        m_pMdbDatabase->SetLogin(user,password,tnsString);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSDatabase::SetServer(const char* pszIP,int pszPort) throw (TOdbcDBException)
{
    try
    {
        m_pMdbDatabase->SetServer(pszIP,pszPort);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSDatabase::CheckError(const char* sSql) throw (TOdbcDBException)
{
    return;

}


bool TOdbcDBCSDatabase::Connect(bool bIsAutoCommit) throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBCSDatabase_Connect,this);
    try
    {
        bool bRet = m_pMdbDatabase->Connect(bIsAutoCommit);
        PS_END(BF_TOdbcDBCSDatabase_Connect);
        return bRet;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSDatabase_Connect);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSDatabase_Connect);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}


bool TOdbcDBCSDatabase::Connect( const char* db, const char* server,const char* user, const char* password, unsigned int port  ) throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBCSDatabase_Connect2,this);
    try
    {
    	m_pMdbDatabase->SetServer(server,port);
    	m_pMdbDatabase->SetLogin(user,password,db);
    	bool bRet = m_pMdbDatabase->Connect();
    	PS_END(BF_TOdbcDBCSDatabase_Connect2);
    	return bRet;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSDatabase_Connect2);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSDatabase_Connect2);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}


bool TOdbcDBCSDatabase::Connect(const char *sUser,const char *sPassword,const char *sServerName,bool bIsAutoCommit) throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBCSDatabase_Connect3,this);
    try
    {
        bool bRet = m_pMdbDatabase->Connect(sUser,sPassword,sServerName,bIsAutoCommit);
        PS_END(BF_TOdbcDBCSDatabase_Connect3);
        return bRet;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSDatabase_Connect3);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSDatabase_Connect3);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

int TOdbcDBCSDatabase::Disconnect() throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBCSDatabase_Disconnect,this);
    try
    {
        int iRet  = m_pMdbDatabase->Disconnect();
        PS_END(BF_TOdbcDBCSDatabase_Disconnect);
        return iRet;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSDatabase_Disconnect);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSDatabase_Disconnect);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSDatabase::TransBegin()
{
    try
    {
    	m_pMdbDatabase->TransBegin();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSDatabase::Commit()
{
    PS_BEGIN(BF_TOdbcDBCSDatabase_Commit,this);
    try
    {
        m_pMdbDatabase->Commit();
        PS_END(BF_TOdbcDBCSDatabase_Commit);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSDatabase_Commit);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSDatabase_Commit);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSDatabase::Rollback()
{
    PS_BEGIN(BF_TOdbcDBCSDatabase_Rollback,this);
    try
    {
    	m_pMdbDatabase->Rollback();
    	PS_END(BF_TOdbcDBCSDatabase_Rollback);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSDatabase_Rollback);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSDatabase_Rollback);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

bool TOdbcDBCSDatabase::IsConnect() throw (TOdbcDBException)
{
    try
    {
        return m_pMdbDatabase->IsConnect();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

bool TOdbcDBCSDatabase::IsNullConnect()
{ 
    try
    {
        return m_pMdbDatabase->IsNullConnect();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

TOdbcDBCSQuery * TOdbcDBCSDatabase::CreateDBQuery() throw (TDBExcpInterface)
{
    try
    {
        return new TOdbcDBCSQuery(this);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}


//------------------------------字段相关------------------------------
TOdbcDBCSField::TOdbcDBCSField()
{
    memset(sBlobBuf,0x00,sizeof(sBlobBuf));
}

TOdbcDBCSField::~TOdbcDBCSField()
{
}

void TOdbcDBCSField::SetTMdbField(const TMdbClientField& field)
{
    try
    {
    	m_MdbField = field;
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

char* TOdbcDBCSField::AsString() throw (TOdbcDBException)
{
    /*PS_BEGIN(BF_TOdbcDBCSField_AsString,this);*/
    try
    {
        char * pstr = m_MdbField.AsString();
        /*PS_END(BF_TOdbcDBCSField_AsString);*/
        return pstr;
    }
    catch(TMdbException &e)
    {
        /*PS_END(BF_TOdbcDBCSField_AsString);*/
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        /*PS_END(BF_TOdbcDBCSField_AsString);*/
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

bool TOdbcDBCSField::isNULL()
{
    try
    {
        return m_MdbField.isNULL();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}


char* TOdbcDBCSField::AsBlobBuffer(int &iBufferLen) throw (TOdbcDBException)
{
    /*PS_BEGIN(BF_TOdbcDBCSField_AsBlobBuffer,this);*/
    try
    {
    	//static char buf[1024]={0}; //356853 for cppcheck
        memset(sBlobBuf,0x00,sizeof(sBlobBuf));
        m_MdbField.AsBlobBuffer((unsigned char *)sBlobBuf,iBufferLen);
        /*PS_END(BF_TOdbcDBCSField_AsBlobBuffer);*/
        return sBlobBuf;
    }
    catch(TMdbException &e)
    {
        /*PS_END(BF_TOdbcDBCSField_AsBlobBuffer);*/
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        /*PS_END(BF_TOdbcDBCSField_AsBlobBuffer);*/
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}


double TOdbcDBCSField::AsFloat() throw (TOdbcDBException)
{
    /*PS_BEGIN(BF_TOdbcDBCSField_AsFloat,this);*/
    try
    {
        double iRet = m_MdbField.AsFloat();
        /*PS_END(BF_TOdbcDBCSField_AsFloat);*/
        return iRet;
    }
    catch(TMdbException &e)
    {
        /*PS_END(BF_TOdbcDBCSField_AsFloat);*/
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        /*PS_END(BF_TOdbcDBCSField_AsFloat);*/
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

char* TOdbcDBCSField::AsDateTimeString() throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBCSField_AsDateTimeString,this);
    try
    {
        char* pstr = m_MdbField.AsDateTimeString();
        PS_END(BF_TOdbcDBCSField_AsDateTimeString);
       	return pstr;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSField_AsDateTimeString);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSField_AsDateTimeString);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void  TOdbcDBCSField::AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBCSField_AsDateTime,this);
    try
    {
        m_MdbField.AsDateTime(year,month,day , hour,minute,second);
        PS_END(BF_TOdbcDBCSField_AsDateTime);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSField_AsDateTime);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSField_AsDateTime);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

//Modify by zhangyu 20050314
long long TOdbcDBCSField::AsInteger() throw (TOdbcDBException)
{   
    /*PS_BEGIN(BF_TOdbcDBCSField_AsInteger,this);*/
    try
    {
        long long iRet = m_MdbField.AsInteger();
        /*PS_END(BF_TOdbcDBCSField_AsInteger);*/
        return iRet;
    }
    catch(TMdbException &e)
    {
        /*PS_END(BF_TOdbcDBCSField_AsInteger);*/
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        /*PS_END(BF_TOdbcDBCSField_AsInteger);*/
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSField::ClearDataBuf()
{
    try
    {
        m_MdbField.ClearDataBuf();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}


int TOdbcDBCSField::DataType(void)
{
	//数据类型：1-DT_Int, 2-DT_Char,3-DT_VarChar 4-DT_DateStamp 9-DT_Blob
	switch(m_MdbField.DataType())
    {
    case 2:
    case 3:
        return 0;
    case 1:
        return 1;
       /*   ---不支持浮点 
    case SQL_FLOAT:
    case SQL_DOUBLE:
        return 2;
        break;
       */
    case 4:
        return 3;
        break;
    case 9:
        return 4;
        break;
    default:
        {
            throw TOdbcDBException("TOdbcDBCSField::AsString","Application Error Message=SQL_DATA_TYPE(%ld) not supported,only support[1-DT_Int, 2-DT_Char,3-DT_VarChar 4-DT_DateStamp 9-DT_Blob]",m_MdbField.DataType());
            //break;//ZMP:305593,多余，无用，去掉
        }
    }
    return -1;
}

const char *TOdbcDBCSField::AsName()
{
    try
    {
    	return (const char *)m_MdbField.GetName();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

//ZMP:305920
time_t TOdbcDBCSField::AsTimeT() throw (TOdbcDBException)
{
    int iYear=0;
    int iMonth=0;
    int iDay=0;
    int iHour=0;
    int iMinute=0;
    int iSecond=0;
    try
    {
        m_MdbField.AsDateTime(iYear, iMonth, iDay , iHour, iMinute, iSecond);

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
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }

}


/*********** TOdbcDBCSQuery Implementation************/
TOdbcDBCSQuery::TOdbcDBCSQuery(TOdbcDBCSDatabase *pTMdbDatabase) throw (TOdbcDBException)
{
    try
    {
        m_pQuery = (pTMdbDatabase->m_pMdbDatabase)->CreateDBQuery();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

TOdbcDBCSQuery::~TOdbcDBCSQuery()
{
    /*
    try
    {
        if (m_pQuery != NULL)
        {
            delete m_pQuery;
            m_pQuery = NULL;
        } 
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
    */
    
    //ZMP:305593,C++不建议析构函数中抛出异常，此处也不会抛出异常
    if (m_pQuery != NULL)
    {
        delete m_pQuery;
        m_pQuery = NULL;
    } 
}

bool TOdbcDBCSQuery::Eof(void)
{
    try
    {
    	return m_pQuery->Eof();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::Close()
{
    try
    {
        m_pQuery->Close();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::CloseSQL()
{
     
}

int TOdbcDBCSQuery::RowsAffected()
{
    try
    {
    	return m_pQuery->RowsAffected();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

bool TOdbcDBCSQuery::Commit()
{
    PS_BEGIN(BF_TOdbcDBCSQuery_Commit,this);
    try
    {
        m_pQuery->Commit();
        PS_END(BF_TOdbcDBCSQuery_Commit);
        return true; 
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSQuery_Commit);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSQuery_Commit);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

bool TOdbcDBCSQuery::TransBegin()
{
	return true;
}

bool TOdbcDBCSQuery::Rollback()
{
    PS_BEGIN(BF_TOdbcDBCSQuery_Rollback,this);
    try
    {
        m_pQuery->Rollback();
        PS_END(BF_TOdbcDBCSQuery_Rollback);
        return true; 
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSQuery_Rollback);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSQuery_Rollback);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}


void TOdbcDBCSQuery::SetSQL(const char *inSqlstmt,int iPreFetchRows) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetSQL(inSqlstmt);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}
void TOdbcDBCSQuery::SetParameterNULL(const char *paramName) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameterNULL(paramName);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetParameter(const char *paramName, double paramValue, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetParameter(const char *paramName, long paramValue, bool isOutput) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}


void TOdbcDBCSQuery::SetParameter(const char *paramName, long long paramValue, bool isOutput) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetParameter(const char *paramName, int paramValue, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
       m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}
//add by fu.wenjun@20041125
void TOdbcDBCSQuery::SetParameter(const char *paramName, const char paramValue, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetParameter(const char *paramName, const char* paramValue, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetParameter(const char *paramName,const char* paramValue,int iBufferLen,bool isOutput) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameter(paramName,paramValue,iBufferLen);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

//简化
void TOdbcDBCSQuery::SetParameter(int iParamIndex,const char* sParamValue) throw (TOdbcDBException)
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,sParamValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetParameter(int iParamIndex,int iParamValue) throw (TOdbcDBException)
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,iParamValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}
void TOdbcDBCSQuery::SetParameter(int iParamIndex,long lParamValue) throw (TOdbcDBException)
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,lParamValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}
void TOdbcDBCSQuery::SetParameter(int iParamIndex,double dParamValue) throw (TOdbcDBException)
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,dParamValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}
void TOdbcDBCSQuery::SetParameter(int iParamIndex,long long llParamValue) throw (TOdbcDBException)
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,llParamValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}
void TOdbcDBCSQuery::SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TOdbcDBException)//用于传入BLOB/BINARY类型字段
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,sParamValue,iBufferLen);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}
void TOdbcDBCSQuery::SetParameterNULL(int iParamIndex) throw (TOdbcDBException)     //设置参数为空
{
    try
    {
    	m_pQuery->SetParameterNULL(iParamIndex);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}	


//ZMP:305920
void TOdbcDBCSQuery::SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused /*= false*/) throw (TOdbcDBException)
{
    try
    {
        char sParamValue[40];
        memset(sParamValue,0x00,sizeof(sParamValue));
        struct tm * tm_Cur;
        tm_Cur = localtime(&tParamValue);
        //ZMP:467365 
        snprintf(sParamValue, sizeof(sParamValue), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
        sParamValue[sizeof(sParamValue)-1] = '\0';
        m_pQuery->SetParameter(sParamName,sParamValue,isOutput_Unused);
        return;
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }    
}


//ZMP:305920
void TOdbcDBCSQuery::SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException)
{
    try
    {
        char sParamValue[40];
        memset(sParamValue,0x00,sizeof(sParamValue));
        struct tm * tm_Cur;
        tm_Cur = localtime(&tParamValue);
        //ZMP:467365 
        snprintf(sParamValue, sizeof(sParamValue), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
        sParamValue[sizeof(sParamValue)-1] = '\0';
        m_pQuery->SetParameter(iParamIndex,sParamValue);
        return;
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
    
}

void TOdbcDBCSQuery::SetParamArray(const char *paramName, char ** paramValue,int iStructSize,int iStrSize,int iArraySize,bool isOutput) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParamArray(paramName,paramValue,iStructSize,iStrSize,iArraySize);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetParamArray(const char *paramName, int * paramValue, int iStructSize,int iArraySize, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParamArray(paramName,paramValue,iStructSize,iArraySize);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetParamArray(const char *paramName, double * paramValue,int iStructSize, int iArraySize,bool isOutput)  throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParamArray(paramName,paramValue,iStructSize,iArraySize);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetParamArray(const char *paramName, long * paramValue, int iStructSize,int iArraySize, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParamArray(paramName,paramValue,iStructSize,iArraySize);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetParamArray(const char *paramName, long long * paramValue, int iStructSize,int iArraySize, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParamArray(paramName,paramValue,iStructSize,iArraySize); 
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize,bool isOutput) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetBlobParamArray(paramName,paramValue,iBufferLen,iArraySize);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

void TOdbcDBCSQuery::CheckError(const char* sSql) throw (TOdbcDBException)
{
    return;
}

bool TOdbcDBCSQuery::Execute(int iters) throw (TOdbcDBException)
{
    /*begin zmp 841469 */
    /*调用GetValue接口后,数据库不允许在进行更新*/
    if(!TOdbcDBCSQuery::m_bUpdateMDBFlag)
    {
        throw TOdbcDBException("TOdbcDBQuery::Execute","Prohibited execution");
    }
    /*end zmp 841469 */
    PS_BEGIN(BF_TOdbcDBCSQuery_Execute,this);
    try
    {
        bool bRet = m_pQuery->Execute(iters);
        PS_END(BF_TOdbcDBCSQuery_Execute);
        return bRet;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSQuery_Execute);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSQuery_Execute);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}


void TOdbcDBCSQuery::Open(int prefetch_Row) throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBCSQuery_Open,this);
    try
    {
        m_pQuery->Open();
        PS_END(BF_TOdbcDBCSQuery_Open);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSQuery_Open);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSQuery_Open);
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

int TOdbcDBCSQuery::FieldCount()
{
    try
    {
        return m_pQuery->FieldCount();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

//ZMP:305013 begin
TOdbcDBParam& TOdbcDBCSQuery::Param(int iIndex)
{
    throw TOdbcDBException("TOdbcDBQuery::Param","Application Error Message=QuickMDB not support Param()!");
}
TOdbcDBParam& TOdbcDBCSQuery::Param(const char*sParamName)
{
    throw TOdbcDBException("TOdbcDBQuery::Param","Application Error Message=QuickMDB not support Param()!");
}
//ZMP:305013 end

TOdbcDBCSField& TOdbcDBCSQuery::Field(int index) throw (TOdbcDBException)
{
    try
    {
        m_tField.SetTMdbField( m_pQuery->Field(index));
        return m_tField;
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}

TOdbcDBCSField& TOdbcDBCSQuery::Field(const char *fieldName) throw (TOdbcDBException)
{
    try
    {
        m_tField.SetTMdbField( m_pQuery->Field(fieldName));
        return m_tField;
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::TOdbcDBDatabase","Unknow ERROR");
    }
}
/*begin zmp 841469 */
void TOdbcDBCSQuery::GetValue(void *pStruct,int* Column)throw (TOdbcDBException)
{
    m_pQuery->GetValue(pStruct,Column);
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

/*
bool TOdbcDBCSQuery::IsFieldExist(const char *fieldName)
{
    return m_pQuery->IsFieldExist(fieldName);
}
*/

bool TOdbcDBCSQuery::Next() throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBCSQuery_Next,this);
    try
    {
        bool bRet = m_pQuery->Next();
        PS_END(BF_TOdbcDBCSQuery_Next);
        return bRet;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBCSQuery_Next);
        throw TOdbcDBException("TOdbcDBDatabase::Next","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBCSQuery_Next);
        throw TOdbcDBException("TOdbcDBDatabase::Next","Unknow ERROR");
    }
}

 //ZMP_559430
long long TOdbcDBCSQuery::GetSequenceByName(const char * sName)throw (TOdbcDBException)
{
    try
    {
        return m_pQuery->GetSequenceByName(sName);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::GetSequenceByName","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::GetSequenceByName","Unknow ERROR");
    }    
}


