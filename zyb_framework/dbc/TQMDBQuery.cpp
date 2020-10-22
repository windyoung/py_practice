//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#include "TQMDBQuery.h"
#include "debug_new.h"
#include "Common.h"
#include "TStrFunc.h"
#include "debug_new.h"
#include "TSplit.h"
#include "ReadIni.h"
#include "TDateTimeFunc.h"

//ZMP:467366   
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

class TOdbcDBQuery;
bool TOdbcDBQuery::m_bUpdateMDBFlag=true;/*判断数据库是否允许进行更新*/

namespace TQMDB
{
    int iRestartFlag = 0;
}

int GetRestartFlag(void)
{
    return TQMDB::iRestartFlag;
}

bool TOdbcDBException::m_bReadFlag = false;
int  TOdbcDBException::m_aErrCode[100] = {0};
int  TOdbcDBException::m_iErrCodeNum   = 0;

TOdbcDBException::TOdbcDBException(const char *sSql, const char* sFormat, ...)
{
    va_list args;

    memset(m_sErrMsg,0,sizeof(m_sErrMsg));
    //memset(m_sErrSql,0,sizeof(m_sErrSql));//在基类已做
    m_lErrCode=0;

    va_start(args, sFormat); 
    //ZMP:467366 
    vsnprintf(m_sErrMsg, sizeof(m_sErrMsg), sFormat, args);
    m_sErrMsg[sizeof(m_sErrMsg)-1] = '\0';
    va_end(args);

    m_sErrMsg[sizeof(m_sErrMsg)-1] = '\0';   

    strncpy(m_sErrSql, sSql, MAX_SQLSTMT_LENGTH);
    m_sErrSql[MAX_SQLSTMT_LENGTH] = '\0';

    SetRestartFlag();
}

void TOdbcDBException::ReadConfig()
{
    //从App.config中读取错误码列表，只读取一次
    if(!m_bReadFlag)
    {
        m_bReadFlag = true;
        char sErrorCodeList[1024] = "";
        char sAppCfgName[256] = "";
        const char * theEnv=getenv("HOME");
        if(theEnv==NULL)
        {
            strncpy(sAppCfgName,"App.config", sizeof(sAppCfgName)-1);
            sAppCfgName[sizeof(sAppCfgName)-1] = '\0';
        }
        else
        {
            snprintf(sAppCfgName, sizeof(sAppCfgName), "%s/etc/App.config",theEnv);
        }

        TReadIni ReadIni(sAppCfgName);
        ReadIni.ReadString("COMMON","ERROE_CODE_LIST",sErrorCodeList,"");
        sErrorCodeList[sizeof(sErrorCodeList)-1]='\0';

        if(sErrorCodeList[0] != 0)
        {
            TSplit split;
            split.Split(sErrorCodeList, '|');
            m_iErrCodeNum = split.GetCount();
            if(m_iErrCodeNum != 0)
            {
                char Item[32] = {0};
                for(int i = 0; i < m_iErrCodeNum; ++i)
                {
                    strncpy(Item,split[i],sizeof(Item)-1);
                    TStrFunc::Trim(Item);
                    m_aErrCode[i] = atoi(Item);
                }
           }
        }
    }

}

void TOdbcDBException::SetRestartFlag()
{
    ReadConfig();

    int i=0;
    char sErrCode[32] = {0};
    char *pPosition = strstr(m_sErrMsg,"Error=[");
    if(pPosition != NULL && TQMDB::iRestartFlag == 0)
    {
        pPosition += 7;
        for( i=0; pPosition[i]!= ']'; ++i)
        {
          sErrCode[i] = pPosition[i];
        }
        m_lErrCode = atoi(sErrCode);

        for( i = 0; i < m_iErrCodeNum; ++i)
        {
            if(m_lErrCode == m_aErrCode[i])
            {
                TQMDB::iRestartFlag = 1;
                break;
            }
        }
    }
}

char * TOdbcDBException::ToString() const
{
    static char sBuf[1024];
    sBuf[0] = 0;

    snprintf(sBuf, sizeof(sBuf), "TOdbcDBException:\n    ErrMsg=%s\n    ErrSql=%s\n", GetErrMsg(), GetErrSql());
    return sBuf;
}

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
/*类TOdbcDBParam定义部分*/
TOdbcDBParam::TOdbcDBParam()
{
    ClearInfo();
}

TOdbcDBParam::~TOdbcDBParam()
{
    ClearInfo();
}
void TOdbcDBParam::ClearInfo()
{
    return;
}

/********* TConnection implementation *********/
TOdbcDBDatabase::TOdbcDBDatabase() throw (TOdbcDBException)
{
    try
    {
       m_pMdbDatabase = new TMdbDatabase();
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

TOdbcDBDatabase::~TOdbcDBDatabase()
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

void TOdbcDBDatabase::SetLogin(const char *user, const char *password, const char *tnsString) throw (TOdbcDBException)
{
    try
    {
        m_pMdbDatabase->SetLogin(user,password,tnsString);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::SetLogin","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::SetLogin","Unknow ERROR");
    }
}

void TOdbcDBDatabase::CheckError(const char* sSql) throw (TOdbcDBException)
{
    return;

}


bool TOdbcDBDatabase::Connect(bool bUnused) throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBDatabase_Connect,this);
    try
    {
        bool bRet = m_pMdbDatabase->Connect(bUnused);
        PS_END(BF_TOdbcDBDatabase_Connect);
        return bRet;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBDatabase_Connect);
        throw TOdbcDBException("TOdbcDBDatabase::Connect","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBDatabase_Connect);
        throw TOdbcDBException("TOdbcDBDatabase::Connect","Unknow ERROR");
    }
}

bool TOdbcDBDatabase::Connect(const char *usr, const char *pwd, const char *tns, bool bUnused) throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBDatabase_Connect2,this);
    try
    { 
        bool bRet = m_pMdbDatabase->Connect(usr,pwd,tns,bUnused);
        PS_END(BF_TOdbcDBDatabase_Connect2);
        return bRet;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBDatabase_Connect2);
        throw TOdbcDBException("TOdbcDBDatabase::Connect","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBDatabase_Connect2);
        throw TOdbcDBException("TOdbcDBDatabase::Connect","Unknow ERROR");
    }
}

bool TOdbcDBDatabase::Connect( const char* db, const char* server,const char* user, const char* password, unsigned int port ) throw (TOdbcDBException)
{
	return false;
}

int TOdbcDBDatabase::Disconnect() throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBDatabase_Disconnect,this);
    try
    { 
        int iRet = m_pMdbDatabase->Disconnect();
        PS_END(BF_TOdbcDBDatabase_Disconnect);
        return iRet;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBDatabase_Disconnect);
        throw TOdbcDBException("TOdbcDBDatabase::Disconnect","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBDatabase_Disconnect);
        throw TOdbcDBException("TOdbcDBDatabase::Disconnect","Unknow ERROR");
    }
}

void TOdbcDBDatabase::Commit()
{
    PS_BEGIN(BF_TOdbcDBDatabase_Commit,this);
    try
    {
        m_pMdbDatabase->Commit();
        PS_END(BF_TOdbcDBDatabase_Commit);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBDatabase_Commit);
        throw TOdbcDBException("TOdbcDBDatabase::Commit","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBDatabase_Commit);
        throw TOdbcDBException("TOdbcDBDatabase::Commit","Unknow ERROR");
    }
}

void TOdbcDBDatabase::Rollback()
{
    PS_BEGIN(BF_TOdbcDBDatabase_Rollback,this);
    try
    {
    	m_pMdbDatabase->Rollback();
    	PS_END(BF_TOdbcDBDatabase_Rollback);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBDatabase_Rollback);
        throw TOdbcDBException("TOdbcDBDatabase::Rollback","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBDatabase_Rollback);
        throw TOdbcDBException("TOdbcDBDatabase::Rollback","Unknow ERROR");
    }
}

bool TOdbcDBDatabase::IsConnect() throw (TOdbcDBException)
{
    try
    {
        return m_pMdbDatabase->IsConnect();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::IsConnect","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::IsConnect","Unknow ERROR");
    }
}

TOdbcDBQuery * TOdbcDBDatabase::CreateDBQuery() throw (TDBExcpInterface)
{
    try
    {
        return new TOdbcDBQuery(this);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBDatabase::CreateDBQuery","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBDatabase::CreateDBQuery","Unknow ERROR");
    }
}


//------------------------------字段相关------------------------------
TOdbcDBField::TOdbcDBField()
{
   //ZMP:305593
   m_MdbField = NULL;
   memset(sBlobBuf,0x00,sizeof(sBlobBuf));
}

TOdbcDBField::~TOdbcDBField()
{
    
}
void TOdbcDBField::SetTMdbField(TMdbField* field)
{
    try
    {
    	m_MdbField = field;
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBField::SetTMdbField","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBField::SetTMdbField","Unknow ERROR");
    }
}

char* TOdbcDBField::AsString() throw (TOdbcDBException)
{
    /*PS_BEGIN(BF_TOdbcDBField_AsString,this);*/
    try
    {
        char *pstr = m_MdbField->AsString();
        return pstr;
    }
    catch(TMdbException &e)
    {
        /*PS_END(BF_TOdbcDBField_AsString);*/
        throw TOdbcDBException("TOdbcDBField::AsString","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        /*PS_END(BF_TOdbcDBField_AsString);*/
        throw TOdbcDBException("TOdbcDBField::AsString","Unknow ERROR");
    }
}

bool TOdbcDBField::isNULL()
{
    try
    {
        return m_MdbField->isNULL();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBField::isNULL","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBField::isNULL","Unknow ERROR");
    }
}


char* TOdbcDBField::AsBlobBuffer(int &iBufferLen) throw (TOdbcDBException)
{
    /*PS_BEGIN(BF_TOdbcDBField_AsBlobBuffer,this);*/
    try
    {
    	//static char buf[1024]={0}; //356853 for cppcheck
        memset(sBlobBuf,0x00,sizeof(sBlobBuf));
        m_MdbField->AsBlobBuffer((unsigned char *)sBlobBuf,iBufferLen);
        /*PS_END(BF_TOdbcDBField_AsBlobBuffer);*/
        return sBlobBuf;
    }
    catch(TMdbException &e)
    {
        /*PS_END(BF_TOdbcDBField_AsBlobBuffer);*/
        throw TOdbcDBException("TOdbcDBField::AsBlobBuffer","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        /*PS_END(BF_TOdbcDBField_AsBlobBuffer);*/
        throw TOdbcDBException("TOdbcDBField::AsBlobBuffer","Unknow ERROR");
    }
}


double TOdbcDBField::AsFloat() throw (TOdbcDBException)
{
    /*PS_BEGIN(BF_TOdbcDBField_AsFloat,this);*/
    try
    {
        double iRet = m_MdbField->AsFloat();
        /*PS_END(BF_TOdbcDBField_AsFloat);*/
        return iRet;
    }
    catch(TMdbException &e)
    {
        /*PS_END(BF_TOdbcDBField_AsFloat);*/
        throw TOdbcDBException("TOdbcDBField::AsFloat","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        /*PS_END(BF_TOdbcDBField_AsFloat);*/
        throw TOdbcDBException("TOdbcDBField::AsFloat","Unknow ERROR");
    }
}

char* TOdbcDBField::AsDateTimeString() throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBField_AsDateTimeString,this);
    try
    {
        char *pstr = m_MdbField->AsDateTimeString();
        PS_END(BF_TOdbcDBField_AsDateTimeString);
       	return pstr;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBField_AsDateTimeString);
        throw TOdbcDBException("TOdbcDBField::AsDateTimeString","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBField_AsDateTimeString);
        throw TOdbcDBException("TOdbcDBField::AsDateTimeString","Unknow ERROR");
    }
}

void    TOdbcDBField::AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBField_AsDateTime,this);
    try
    {
        m_MdbField->AsDateTime(year,month,day , hour,minute,second);
        PS_END(BF_TOdbcDBField_AsDateTime);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBField_AsDateTime);
        throw TOdbcDBException("TOdbcDBField::AsDateTime","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBField_AsDateTime);
        throw TOdbcDBException("TOdbcDBField::AsDateTime","Unknow ERROR");
    }
}

//Modify by zhangyu 20050314
long long TOdbcDBField::AsInteger() throw (TOdbcDBException)
{   
    /*PS_BEGIN(BF_TOdbcDBField_AsInteger,this);*/
    try
    {
        long long iRet = m_MdbField->AsInteger();
        /*PS_END(BF_TOdbcDBField_AsInteger);*/
        return iRet;
    }
    catch(TMdbException &e)
    {
        /*PS_END(BF_TOdbcDBField_AsInteger);*/
        throw TOdbcDBException("TOdbcDBField::AsInteger","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        /*PS_END(BF_TOdbcDBField_AsInteger);*/
        throw TOdbcDBException("TOdbcDBField::AsInteger","Unknow ERROR");
    }
}

void TOdbcDBField::ClearDataBuf()
{
    try
    {
        m_MdbField->ClearDataBuf();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBField::ClearDataBuf","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBField::ClearDataBuf","Unknow ERROR");
    }
}

//ZMP:305920
time_t TOdbcDBField::AsTimeT() throw (TOdbcDBException)
{
    int iYear=0;
    int iMonth=0;
    int iDay=0;
    int iHour=0;
    int iMinute=0;
    int iSecond=0;
    try
    {
        m_MdbField->AsDateTime(iYear, iMonth, iDay , iHour, iMinute, iSecond);

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
        throw TOdbcDBException("TOdbcDBField::AsTimeT","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBField::AsTimeT","Unknow ERROR");
    }

}

/*********** TQMDBQuery Implementation************/
TOdbcDBQuery::TOdbcDBQuery(TOdbcDBDatabase *pTMdbDatabase) throw (TOdbcDBException)
{
    try
    {
        m_pQuery = (pTMdbDatabase->m_pMdbDatabase)->CreateDBQuery();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::TOdbcDBQuery","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::TOdbcDBQuery","Unknow ERROR");
    }
}



TOdbcDBQuery::~TOdbcDBQuery()
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

bool TOdbcDBQuery::Eof(void)
{
    try
    {
    	return m_pQuery->Eof(); 
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::Eof","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::Eof","Unknow ERROR");
    }
}

void TOdbcDBQuery::Close()
{
    PS_BEGIN(BF_TOdbcDBQuery_Close,this);
    try
    {
        m_pQuery->Close();
        PS_END(BF_TOdbcDBQuery_Close);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBQuery_Close);
        throw TOdbcDBException("TOdbcDBQuery::Close","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBQuery_Close);
        throw TOdbcDBException("TOdbcDBQuery::Close","Unknow ERROR");
    }
}

int TOdbcDBQuery::RowsAffected()
{
    try
    {
    	return m_pQuery->RowsAffected();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::RowsAffected","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::RowsAffected","Unknow ERROR");
    }
	//return 0;
}

bool TOdbcDBQuery::Commit()
{
    PS_BEGIN(BF_TOdbcDBQuery_Commit,this);
    try
    {
        m_pQuery->Commit(); 
        PS_END(BF_TOdbcDBQuery_Commit);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBQuery_Commit);
        throw TOdbcDBException("TOdbcDBQuery::Commit","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBQuery_Commit);
        throw TOdbcDBException("TOdbcDBQuery::Commit","Unknow ERROR");
    }
    return true; 
}

bool TOdbcDBQuery::Rollback()
{
    PS_BEGIN(BF_TOdbcDBQuery_Rollback,this);
    try
    {
        m_pQuery->Rollback();
        PS_END(BF_TOdbcDBQuery_Rollback);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBQuery_Rollback);
        throw TOdbcDBException("TOdbcDBQuery::Rollback","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBQuery_Rollback);
        throw TOdbcDBException("TOdbcDBQuery::Rollback","Unknow ERROR");
    }
     return true; 
}



void TOdbcDBQuery::SetSQL(const char *inSqlstmt,int iPreFetchRows) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetSQL(inSqlstmt);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetSQL","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetSQL","Unknow ERROR");
    }
}


void TOdbcDBQuery::SetParameterNULL(const char *paramName) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameterNULL(paramName);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameterNULL","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameterNULL","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParameter(const char *paramName, double paramValue, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
       m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParameter(const char *paramName, long paramValue, bool isOutput) throw (TOdbcDBException)
{
    try
    {
       m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}


void TOdbcDBQuery::SetParameter(const char *paramName, long long paramValue, bool isOutput) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParameter(const char *paramName, int paramValue, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
       m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}
//add by fu.wenjun@20041125
void TOdbcDBQuery::SetParameter(const char *paramName, const char paramValue, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}
void TOdbcDBQuery::SetParameter(const char *paramName, const char* paramValue, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
       m_pQuery->SetParameter(paramName,paramValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParameter(const char *paramName,const char* paramValue,int iBufferLen,bool isOutput) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParameter(paramName,paramValue,iBufferLen);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParameter(int iParamIndex,const char* sParamValue) throw (TOdbcDBException)
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,sParamValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParameter(int iParamIndex,int iParamValue) throw (TOdbcDBException)
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,iParamValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}
void TOdbcDBQuery::SetParameter(int iParamIndex,long lParamValue) throw (TOdbcDBException)
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,lParamValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}
void TOdbcDBQuery::SetParameter(int iParamIndex,double dParamValue) throw (TOdbcDBException)
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,dParamValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}
void TOdbcDBQuery::SetParameter(int iParamIndex,long long llParamValue) throw (TOdbcDBException)
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,llParamValue);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}
void TOdbcDBQuery::SetParameter(int iParamIndex,const char* sParamValue,int iBufferLen) throw (TOdbcDBException)//用于传入BLOB/BINARY类型字段
{
    try
    {
    	m_pQuery->SetParameter(iParamIndex,sParamValue,iBufferLen);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameter","Unknow ERROR");
    }
}
void TOdbcDBQuery::SetParameterNULL(int iParamIndex) throw (TOdbcDBException)     //设置参数为空
{
    try
    {
    	m_pQuery->SetParameterNULL(iParamIndex);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameterNULL","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameterNULL","Unknow ERROR");
    }
}	

//ZMP:305920 
void TOdbcDBQuery::SetParameterTime_t(const char *sParamName,time_t tParamValue, bool isOutput_Unused /*= false*/) throw (TOdbcDBException)
{
    try
    {
        char sParamValue[40];
        memset(sParamValue,0x00,sizeof(sParamValue));
        struct tm * tm_Cur;
        tm_Cur = localtime(&tParamValue);
        //ZMP:467366 
        snprintf(sParamValue, sizeof(sParamValue), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
        sParamValue[sizeof(sParamValue)-1] = '\0';
        m_pQuery->SetParameter(sParamName,sParamValue,isOutput_Unused);
        return;
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameterTime_t","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameterTime_t","Unknow ERROR");
    }
    
}


//ZMP:305920 
void TOdbcDBQuery::SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TOdbcDBException)
{
    try
    {
        char sParamValue[40];
        memset(sParamValue,0x00,sizeof(sParamValue));
        struct tm * tm_Cur;
        tm_Cur = localtime(&tParamValue);
        //ZMP:467366 
        snprintf(sParamValue, sizeof(sParamValue), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
        sParamValue[sizeof(sParamValue)-1] = '\0';
        m_pQuery->SetParameter(iParamIndex,sParamValue);
        return;
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameterTime_t","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParameterTime_t","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParamArray(const char *paramName, char ** paramValue,int iStructSize,int iStrSize,int iArraySize,bool isOutput) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParamArray(paramName,paramValue,iStructSize,iStrSize,iArraySize);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParamArray(const char *paramName, int * paramValue, int iStructSize,int iArraySize, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
         m_pQuery->SetParamArray(paramName,paramValue,iStructSize,iArraySize,iArraySize);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParamArray(const char *paramName, double * paramValue,int iStructSize, int iArraySize,bool isOutput)  throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParamArray(paramName,paramValue,iStructSize,iArraySize,iArraySize);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParamArray(const char *paramName, long * paramValue, int iStructSize,int iArraySize, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParamArray(paramName,paramValue,iStructSize,iArraySize,iArraySize);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetParamArray(const char *paramName, long long * paramValue, int iStructSize,int iArraySize, bool isOutput ) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetParamArray(paramName,paramValue,iStructSize,iArraySize,iArraySize); 
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetParamArray","Unknow ERROR");
    }
}

void TOdbcDBQuery::SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize,bool isOutput) throw (TOdbcDBException)
{
    try
    {
        m_pQuery->SetBlobParamArray(paramName,paramValue,iBufferLen,iArraySize);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetBlobParamArray","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::SetBlobParamArray","Unknow ERROR");
    }
}

void TOdbcDBQuery::CheckError(const char* sSql) throw (TOdbcDBException)
{
    return;
}

bool TOdbcDBQuery::Execute(int iters) throw (TOdbcDBException)
{
    /*begin zmp 841469 */
    /*调用GetValue接口后,数据库不允许在进行更新*/
    if(!TOdbcDBQuery::m_bUpdateMDBFlag)
    {
        throw TOdbcDBException("TOdbcDBQuery::Execute","Prohibited execution");
    }
    /*end zmp 841469 */
    
    try
    {
        return m_pQuery->Execute(iters);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::Execute","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::Execute","Unknow ERROR");
    }
}


void TOdbcDBQuery::Open(int prefetch_Row) throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBQuery_Open,this);
    try
    {
        m_pQuery->Open();
        PS_END(BF_TOdbcDBQuery_Open);
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBQuery_Open);
        throw TOdbcDBException("TOdbcDBQuery::Open","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBQuery_Open);
        throw TOdbcDBException("TOdbcDBQuery::Open","Unknow ERROR");
    }
}

int TOdbcDBQuery::FieldCount()
{
    try
    {
        return m_pQuery->FieldCount();
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::FieldCount","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::FieldCount","Unknow ERROR");
    }
}

//ZMP:305013 begin
TOdbcDBParam& TOdbcDBQuery::Param(int iIndex)
{
    throw TOdbcDBException("TOdbcDBQuery::Param","Application Error Message=QuickMDB not support Param()!");
}
TOdbcDBParam& TOdbcDBQuery::Param(const char*sParamName)
{
    throw TOdbcDBException("TOdbcDBQuery::Param","Application Error Message=QuickMDB not support Param()!");
}
//ZMP:305013 end

TOdbcDBField& TOdbcDBQuery::Field(int index) throw (TOdbcDBException)
{
    try
    {
        m_tField[0].SetTMdbField(&(m_pQuery->Field(index)));
        return m_tField[0];
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::Field","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::Field","Unknow ERROR");
    }
}

TOdbcDBField& TOdbcDBQuery::Field(const char *fieldName) throw (TOdbcDBException)
{
    try
    {
		int iPos = 0;
		TMdbField* mdbField = NULL;
		m_pQuery->FieldPos(fieldName,iPos,&mdbField);
        //ZMP:331611 
        if(iPos < MAX_COLUMN_COUNT)
        {
            m_tField[iPos].SetTMdbField(mdbField);
	        return m_tField[iPos];
        }
        else
        {
            throw TOdbcDBException("TOdbcDBQuery::Field","The [%s]'s position is [%d],but The Max Field Count Permited is [%d].\n",fieldName,iPos,MAX_COLUMN_COUNT);
        }
	    
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::Field","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(TOdbcDBException &e)
    {
        TOdbcDBException ex(e);
        throw  ex;
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::Field","Unknow ERROR");
    }
}

/*begin zmp 841469 */
void TOdbcDBQuery::GetValue(void *pStruct,int* Column)throw (TOdbcDBException)
{
    m_pQuery->GetValue(pStruct,Column);
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

bool TOdbcDBQuery::IsFieldExist(const char *fieldName)
{
    try
    {
        return m_pQuery->IsFieldExist(fieldName);
    }
    catch(TMdbException &e)
    {
        throw TOdbcDBException("TOdbcDBQuery::IsFieldExist","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        throw TOdbcDBException("TOdbcDBQuery::IsFieldExist","Unknow ERROR");
    }
}



bool TOdbcDBQuery::Next() throw (TOdbcDBException)
{
    PS_BEGIN(BF_TOdbcDBQuery_Next,this);
    try
    {
       bool bRet = m_pQuery->Next();
       return bRet;
    }
    catch(TMdbException &e)
    {
        PS_END(BF_TOdbcDBQuery_Next);
        throw TOdbcDBException("TOdbcDBQuery::Next","ERROR_MSG=%s.\n ERROR_SQL=%s.\n",e.GetErrMsg(),e.GetErrSql());
    }
    catch(...)
    {
        PS_END(BF_TOdbcDBQuery_Next);
        throw TOdbcDBException("TOdbcDBQuery::Next","Unknow ERROR");
    }
}
//ZMP_559430
long long TOdbcDBQuery::GetSequenceByName(const char * sName)throw (TOdbcDBException)
{
     throw TOdbcDBException("TOdbcDBQuery::GetSequenceByName","TQMDBQuery doesn't suppport it now.");
}


