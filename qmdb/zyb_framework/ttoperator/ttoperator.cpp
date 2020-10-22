//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/ttoperator/
// timesten_operator.cpp : 定义DLL 应用程序的入口点。
//
 
#include "TDBFactory.h"
#include "TDateTimeFunc.h"

//ZMP:467376     
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

//#define _OUT_PUT_DEFINE_ 1

enum FIELD_TYPE
{
    FIELD_TYPE_INT    = 1,
    FIELD_TYPE_STRING = 2,
    FIELD_TYPE_DATE   = 3,
    FIELD_TYPE_FLOAT  = 4,
    FIELD_TYPE_BLOB   = 5,
    FIELD_TYPE_LLONG  = 6
};
#define ERROR_BUF  1024
#define WriteLog_TCI  WriteLog(g_sDlllLogFile

#ifdef _WIN32
char g_sDlllLogFile[250]    ="C:\\ZTESoft\\LOG\\tci.log";
#else
char g_sDlllLogFile[250]    ="./../log/tci.log";
#endif

#define DLL_API extern "C" __declspec(dllexport)
//创建一个ODBC一个连接
DLL_API bool CreateODBCDBDatabase(void **pODBCDBDateBase,char *sUser,char *sPassword,char *sTNS,char *errorInfo);
//创建一个ODBC一个实例
DLL_API bool CreateODBCDB(void **pODBCDBDateBase,void **pODBCDB,char *errorInfo);
//设置SQL语句
DLL_API bool SetSQL(void **pODBC,const char* pSql,char *errorInfo);
//设置SQL参数
DLL_API bool SetParameterString(void **pODBC,const char *sParamName,const char* sParamValue,char *errorInfo);
//设置SQL参数
DLL_API bool SetParameterInt(void **pODBC,const char *sParamName,const int iParamValue,char *errorInfo);
//设置SQL参数
DLL_API bool SetParameterLong(void **pODBC,const char *sParamName,const long lParamValue,char *errorInfo);
//设置SQL参数
DLL_API bool SetParameterLongLong(void **pODBC,const char *sParamName,const long long lParamValue,char *errorInfo);
//设置SQL参数
DLL_API bool SetParameterDouble(void **pODBC,const char *sParamName,const double lParamValue,char *errorInfo);
//设置SQL参数
DLL_API bool SetParameterBLOB(void **pODBC,const char *sParamName,const char* sParamValue,int iBufferLen,char *errorInfo);
//设置SQL参数
DLL_API bool SetParameterNULL(void **pODBC,const char *sParamName,char *errorInfo);
//执行SQL语句
DLL_API bool Execute(void **pODBC,char *errorInfo);
//执行SQL语句
DLL_API int GetRowsAffected(void **pODBC,char *errorInfo);
//关闭SQL语句，以准备接收下一个sql语句
DLL_API bool Close(void **pODBC,char *errorInfo);
//事务开启
DLL_API bool TransBegin(void **pODBC,char *errorInfo);
//事务提交
DLL_API bool Commit(void **pODBC,char *errorInfo);
//事务回滚
DLL_API bool Rollback(void **pODBC,char *errorInfo);
//释放一个ODBC一个连接
DLL_API int FreeODBCDBDatabase(void **pODBCDBDateBase);
//释放一个ODBC一个实例
DLL_API int FreeODBCDB(void **pODBCDB);
//重新连接,TOdbcDBDatabase
DLL_API bool ReConnect(void **pODBCDBDateBase);
//删除一个ODBC一个连接
DLL_API void DeleteODBCDBDatabase(void **pODBCDBDateBase);
//删除一个ODBC一个实例
DLL_API void DeleteODBCDB(void **pODBCDB);
//打开的查询会话
DLL_API bool Open(void **pODBC,const long iMaxCount,char *errorInfo);
//获取当前行的指定字段的值
DLL_API bool GetValueByName(void **pODBC,const char *sFieldName,int iFieldType,char *sFieldValue,char *errorInfo);
//取下一条记录
DLL_API int  Next(void **pODBC,char *errorInfo);

DLL_API void GetString(char *errorInfo)
{
    const int iErrInfoLen = 7;
    //ZMP:467376
    strncpy(errorInfo,"TEST!!!",iErrInfoLen);
    errorInfo[iErrInfoLen] = '\0';
}

BOOL APIENTRY DllMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    return TRUE;
}

void WriteLog(char *strDlllLogFile,const char * fmt, ...)
{
    char sDate[20] = "\0";

    TDateTimeFunc::GetCurrentTimeStr(sDate,sizeof(sDate));

    char sTciLog[40] = "\0";
    strncpy(sTciLog,strDlllLogFile,sizeof(sTciLog));
    strncat(sTciLog,sDate,8);
    FILE *fp=fopen(sTciLog,"a+");
    if (NULL == fp)
    {
        int iError = errno;
        printf("fopen(%s, %s) failed. errno : %d, reason : %s \n", sTciLog, "a+", iError, strerror(iError));
        return;
    }

    va_list ap;
    va_start (ap,fmt);
    clock_t cTimeSec = clock();

    try
    {
        char sLine[10240]={0}; //357133 for cppcheck
        //ZMP:467376
        vsnprintf(sLine, sizeof(sLine), fmt,ap);
        sLine[sizeof(sLine)-1] = '\0';
        va_end (ap);
    }
    catch(...)
    {
        printf("Write Buffer Overflow Or Format Paramter Error.\n");
        cout << "FormatString : " <<fmt << endl;
    }

    printf("%s (%-9ld)> ", sDate,cTimeSec); 
    va_start (ap,fmt);
    vprintf(fmt, ap);
    va_end (ap);

    fprintf (fp, "%s (%-9ld) > ", sDate,cTimeSec); 
    va_start (ap,fmt);
    vfprintf (fp,fmt,ap);
    fflush(fp);
    fclose(fp);
    fp = NULL;  //357133 for cppcheck
    va_end (ap);
}

//获取当前行的指定字段的值
DLL_API bool GetValueByName(void **pODBC,const char *sFieldName,int iFieldType,char *sFieldValue,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF];
	memset(strErrorInfo,0,ERROR_BUF);
	char strFieldValue[ERROR_BUF];
	memset(strFieldValue,0,ERROR_BUF);
    if (!*pODBC)
    {
        strncpy(errorInfo,"GetValueByName: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"GetValueByName: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        int iEventType = 0;
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
		if (iFieldType == FIELD_TYPE_INT)
        {
            //ZMP:467376
            snprintf(strFieldValue, sizeof(strFieldValue), "%ld",pODBCInstance->Field(sFieldName).AsInteger());
            strFieldValue[sizeof(strFieldValue)-1] = '\0';
        }
        else if (iFieldType == FIELD_TYPE_LLONG)
        {
			//printf("----%d--%d---\n",sizeof(long),sizeof(long long));
			//llong ivalue = pODBCInstance->Field(sFieldName).AsInteger();
			//long long iiiii = ivalue;
			//printf("----%ld--%lld-- %ld -[%lld]\n",ivalue,ivalue,iiiii,iiiii);
			//char caaaa[40];
			//sprintf(caaaa,"%lld",ivalue);
			//printf("----%ld--%lld---%s\n",ivalue,ivalue,caaaa);
			//sprintf(caaaa,"%ld",ivalue);
			//printf("----%ld--%lld---%s\n",ivalue,ivalue,caaaa);
			
			//ZMP:467376
            snprintf(strFieldValue, sizeof(strFieldValue), "%lld",(long long)(pODBCInstance->Field(sFieldName).AsInteger()));
            strFieldValue[sizeof(strFieldValue)-1] = '\0';
        }
        else if (iFieldType == FIELD_TYPE_STRING)
            strncpy(strFieldValue,pODBCInstance->Field(sFieldName).AsString(),ERROR_BUF);
        else if (iFieldType == FIELD_TYPE_DATE)
            strncpy(strFieldValue,pODBCInstance->Field(sFieldName).AsDateTimeString(),ERROR_BUF);
        else if (iFieldType == FIELD_TYPE_FLOAT)
        {
            //ZMP:467376
            snprintf(strFieldValue, sizeof(strFieldValue), "%lf",pODBCInstance->Field(sFieldName).AsFloat());
            strFieldValue[sizeof(strFieldValue)-1] = '\0';
        }
        else if (iFieldType == FIELD_TYPE_BLOB)
            strncpy(strFieldValue,pODBCInstance->Field(sFieldName).AsBlobBuffer(iEventType),ERROR_BUF);
        else
        {
            WriteLog_TCI,"GetValueByName: [%d] TYPE IS NOT EXIST!!\n",iFieldType);
        }
		strFieldValue[ERROR_BUF-1] = '\0';
        strncpy(sFieldValue,strFieldValue,ERROR_BUF);
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //strcpy(strErrorInfo,) = "SetSQL() : TDBException:";
        //strErrorInfo += oe.GetErrMsg();
        //strErrorInfo += "\nSQLString : ";
        //strErrorInfo += oe.GetErrSql();
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));

        //sprintf(strErrorInfo,"TDBException:%s\nSQLString :%s",oe.GetErrMsg(),oe.GetErrSql());
    }
    catch(TException &oe)
    {
        //strErrorInfo = "SetSQL() : TException:";
        //strErrorInfo += oe.GetErrMsg();
        //sprintf(strErrorInfo,"TException:%s",oe.GetErrMsg());
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //strErrorInfo = "SetSQL() : Unknow Exception." ;
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    //cout<<strErrorInfo<<endl;
    if (!bRetValue)
    {
        //strErrorInfo[ERROR_BUF-1] = '\0';
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"GetValueByName Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}

//打开的查询会话
DLL_API bool Open(void **pODBC,const long iMaxCount,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF];
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"Open: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"Open: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->Open();
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"Open Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}

//取下一条记录
DLL_API int Next(void **pODBC,char *errorInfo)
{
    int iReturnValue = -1;
    char strErrorInfo[ERROR_BUF];
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"Next: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"Next: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return iReturnValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        if (pODBCInstance->Next())
        {
            iReturnValue = 1;
        }
        else
        {
            iReturnValue = 0;
        }
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.",sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (iReturnValue < 0)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"Next Error: [%s]\n",strErrorInfo);
    }

    return iReturnValue;
}

//创建一个ODBC一个连接
DLL_API bool CreateODBCDBDatabase(void **pODBCDBDateBase,char *sUser,char *sPassword,char *sTNS,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF] ;
	memset(strErrorInfo,0,ERROR_BUF);
    TDBInterface * pOdbcDBLink = NULL;
         
 
    try  
    {
        pOdbcDBLink = TDBFactory::CreateDB(_TTDB_CONN_STRING_);    
        pOdbcDBLink->SetLogin(sUser,sPassword,sTNS);
        bRetValue = pOdbcDBLink->Connect();
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }
    //如果失败,删除ODBCDATABASE,成功则传指针
    if (!bRetValue)
    {
        try
        {
            if (pOdbcDBLink)
            {
                delete pOdbcDBLink;
                pOdbcDBLink = NULL; //357133 for cppcheck
            }
        }
        catch(...)
        {
        }
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"CreateODBCDBDatabase Error: [%s]\n",strErrorInfo);
    }
    else
    {
        *pODBCDBDateBase = (void*)pOdbcDBLink;
    }
    return bRetValue;
}

//创建一个ODBC一个实例
DLL_API bool CreateODBCDB(void **pODBCDBDateBase,void **pODBCDB,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF];
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBCDBDateBase)
    {
        strncpy(errorInfo,"CreateODBCDB: ODBCDBDateBase is NULL,Please Create ODBCDBDateBase first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"CreateODBCDB: ODBCDBDateBase is NULL,Please Create ODBCDBDateBase first.\n");
        return bRetValue;
    }

    TOdbcDBQuery *pODBCInstance = NULL;

    try
    {
        TOdbcDBDatabase * pOdbcDBLink = (TOdbcDBDatabase*)(*pODBCDBDateBase);
        pODBCInstance = new TOdbcDBQuery(pOdbcDBLink);
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        if (pODBCInstance)
        {
            try
            {
                delete pODBCInstance;
                pODBCInstance = NULL; //357133 for cppcheck
            }
            catch(...)
            {
            }
        }
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"CreateODBCDB Error: [%s]\n",strErrorInfo);
    }
    else
    {
        *pODBCDB = pODBCInstance;
    }
    return bRetValue;
}

//设置SQL语句
DLL_API bool SetSQL(void **pODBC,const char* pSql,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF];
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"SetSQL: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"SetSQL: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
#ifdef _DEBUG
        WriteLog_TCI,"SetSQL: Process Begin.\n");
#endif
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
#ifdef _DEBUG
        WriteLog_TCI,"SetSQL pODBCInstance address:[%x].\n",*pODBC);
        WriteLog_TCI,"SQL:[%s].\n",pSql);
#endif
        pODBCInstance->SetSQL(pSql);
        bRetValue = true;
#ifdef _DEBUG
        WriteLog_TCI,"SetSQL: Process End.\n");
#endif
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"SetSQL Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}


//设置SQL参数
DLL_API bool SetParameterString(void **pODBC,const char *sParamName,const char* sParamValue,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF];
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"SetParameterString: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"SetParameterString: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->SetParameter(sParamName,sParamValue);
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%s].TOdbcDBException:",sParamName,sParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%s].TException:",sParamName,sParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%s].Unknow Exception:",sParamName,sParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';  
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"SetParameterString Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//设置SQL参数
DLL_API bool SetParameterInt(void **pODBC,const char *sParamName,const int iParamValue,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF];
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"SetParameterInt: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"SetParameterInt: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->SetParameter(sParamName,iParamValue);
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%d].TOdbcDBException:",sParamName,iParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%d].TException:",sParamName,iParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%d].Unknow Exception:",sParamName,iParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"SetParameterInt Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//设置SQL参数
DLL_API bool SetParameterLong(void **pODBC,const char *sParamName,const long lParamValue,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF];
	memset(strErrorInfo,0,ERROR_BUF);

    char sParamValue[20] = "\0";
    memset(sParamValue,0,sizeof(sParamValue));
    //ZMP:467376
    snprintf(sParamValue, sizeof(sParamValue), "%ld",lParamValue);
    sParamValue[sizeof(sParamValue)-1] = '\0';

    if (!*pODBC)
    {
        strncpy(errorInfo,"SetParameterLong: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"SetParameterLong: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->SetParameter(sParamName,lParamValue);
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%ld].TOdbcDBException:",sParamName,lParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%ld].TException:",sParamName,lParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%ld].Unknow Exception:",sParamName,lParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"SetParameterLong Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//设置SQL参数
DLL_API bool SetParameterLongLong(void **pODBC,const char *sParamName,const long long lParamValue,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);
    char sParamValue[20] = "\0";
    memset(sParamValue,0,sizeof(sParamValue));
    //ZMP:467376
    snprintf(sParamValue, sizeof(sParamValue), "%lld",lParamValue);
    sParamValue[sizeof(sParamValue)-1] = '\0';

    if (!*pODBC)
    {
        strncpy(errorInfo,"SetParameterLongLong: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"SetParameterLongLong: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->SetParameter(sParamName,lParamValue);
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%lld].TOdbcDBException:",sParamName,lParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%lld].TException:",sParamName,lParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%lld].Unknow Exception:",sParamName,lParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"SetParameterLongLong Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//设置SQL参数
DLL_API bool SetParameterDouble(void **pODBC,const char *sParamName,const double lParamValue,char *errorInfo)
{
    //WriteLog_TCI,"SetParameterLongLong sParamName = [%s],lParamValue = [%lf]\n",sParamName,lParamValue);
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);
    char sParamValue[20] = "\0";
    memset(sParamValue,0,sizeof(sParamValue));
    //ZMP:467376
    snprintf(sParamValue, sizeof(sParamValue), "%lf",lParamValue);
    sParamValue[sizeof(sParamValue)-1] = '\0';

    if (!*pODBC)
    {
        strncpy(errorInfo,"SetParameterDouble: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"SetParameterDouble: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->SetParameter(sParamName,lParamValue);
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%lf].TOdbcDBException:",sParamName,lParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%lf].TException:",sParamName,lParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%lf].Unknow Exception:",sParamName,lParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"SetParameterDouble Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//设置SQL参数
DLL_API bool SetParameterBLOB(void **pODBC,const char *sParamName,const char* sParamValue,int iBufferLen,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"SetParameterBLOB: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"SetParameterBLOB: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->SetParameter(sParamName,sParamValue,iBufferLen);
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%s].TOdbcDBException:",sParamName,sParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%s].TException:",sParamName,sParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s],ParamValue=[%s].Unknow Exception:",sParamName,sParamValue);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"SetParameterBLOB Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//设置SQL参数
DLL_API bool SetParameterNULL(void **pODBC,const char *sParamName,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"SetParameterNULL: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"SetParameterNULL: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->SetParameterNULL(sParamName);
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s].TOdbcDBException:",sParamName);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s].TException:",sParamName);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        snprintf(strErrorInfo, sizeof(strErrorInfo), "ParamName=[%s].Unknow Exception:",sParamName);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"SetParameterNULL Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//执行SQL语句
DLL_API bool Execute(void **pODBC,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"Execute: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"Execute: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
#ifdef _DEBUG
        WriteLog_TCI,"Execute: Process Begin.\n");
#endif
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
#ifdef _DEBUG
        WriteLog_TCI,"Execute pODBCInstance address:[%x].\n",*pODBC);
#endif
        pODBCInstance->Execute();
#ifdef _DEBUG
        WriteLog_TCI,"Execute: Process End.\n");
#endif
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"Execute Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//关闭SQL语句，以准备接收下一个sql语句
DLL_API int GetRowsAffected(void **pODBC,char *errorInfo)
{
    int iRetValue = -1;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"GetRowsAffected: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"GetRowsAffected: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return iRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        iRetValue = pODBCInstance->RowsAffected();
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (iRetValue < 0)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"GetRowsAffected Error: [%s]\n",strErrorInfo);
    }

    return iRetValue;
}
//关闭SQL语句，以准备接收下一个sql语句
DLL_API bool Close(void **pODBC,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);
    if (!*pODBC)
    {
        strncpy(errorInfo,"Close: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"Close: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->Close();
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
     }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"Close Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//事务开启
DLL_API bool TransBegin(void **pODBC,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"TransBegin: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"TransBegin: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->TransBegin();
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"TransBegin Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//事务提交
DLL_API bool Commit(void **pODBC,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);

    if (!*pODBC)
    {
        strncpy(errorInfo,"Commit: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"Commit: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->Commit();
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:",sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:",sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"Commit Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}
//事务回滚
DLL_API bool Rollback(void **pODBC,char *errorInfo)
{
    bool bRetValue=false;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);
    if (!*pODBC)
    {
        strncpy(errorInfo,"Rollback: OdbcDB Handle is NULL,Please Create ODBCDB first.",ERROR_BUF);
        errorInfo[ERROR_BUF-1]='\0';
        WriteLog_TCI,"Rollback: OdbcDB Handle is NULL,Please Create ODBCDB first.\n");
        return bRetValue;
    }

    try
    {
        TOdbcDBQuery *pODBCInstance = (TOdbcDBQuery *)*pODBC;
        pODBCInstance->Rollback();
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        strncpy(errorInfo,strErrorInfo,ERROR_BUF);
        WriteLog_TCI,"Rollback Error: [%s]\n",strErrorInfo);
    }

    return bRetValue;
}


//删除一个ODBC一个连接
DLL_API void DeleteODBCDBDatabase(void **pODBCDBDateBase)
{
    bool bRetValue = false;
    if (!*pODBCDBDateBase)
        return;
    TOdbcDBDatabase * pOdbcDBLink = NULL;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);
    try
    {
        pOdbcDBLink = (TOdbcDBDatabase *)*pODBCDBDateBase;
        delete pOdbcDBLink;
        pOdbcDBLink = NULL; //357133 for cppcheck
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:",sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {   
        //ZMP:467376
        strncpy(strErrorInfo,"TException:",sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        WriteLog_TCI,"DeleteODBCDBDatabase Error: [%s]\n",strErrorInfo);
    }

    pOdbcDBLink = NULL;
    *pODBCDBDateBase = NULL;
}
//删除一个ODBC一个实例
DLL_API void DeleteODBCDB(void **pODBCDB)
{
    bool bRetValue = false;
    if (!*pODBCDB)
        return;

    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);
    TOdbcDBQuery * pODBCInstance = NULL;
    try
    {
        pODBCInstance = (TOdbcDBQuery *)*pODBCDB;//ZMP:305593
        delete pODBCInstance;
        pODBCInstance = NULL;  //357133 for cppcheck
        bRetValue = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:",sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,"\nSQLString :",sizeof(strErrorInfo)-strlen(strErrorInfo));
        strncat(strErrorInfo,oe.GetErrSql(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    { 
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.",sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (!bRetValue)
    {
        strErrorInfo[ERROR_BUF-1] = '\0';
        WriteLog_TCI,"DeleteODBCDB Error: [%s]\n",strErrorInfo);
    }

    pODBCInstance = NULL;
    *pODBCDB = NULL;
}

//释放一个ODBC一个连接
DLL_API int FreeODBCDBDatabase(void **pODBCDBDateBase)
{
    int iRetCode = -1;
    if (!*pODBCDBDateBase)
        return iRetCode;
    //TOdbcDBDatabase * pOdbcDBLink = NULL;//ZMP:305593,变量未用到
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);
    try
    {
        //pOdbcDBLink = (TOdbcDBDatabase *)*pODBCDBDateBase;
        //iRetCode = pOdbcDBLink->FreeOdbcDBDatabase();
        iRetCode = 0;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TException:",sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (iRetCode != 0)
    {
        WriteLog_TCI,"FreeODBCDBDatabase iRetCode = [%d],ErrorInfo = [%s]\n",iRetCode,strErrorInfo);
    }
    return iRetCode;
}
//释放一个ODBC一个实例
DLL_API int FreeODBCDB(void **pODBCDB)
{
    int iRetCode = -1;
    if (!*pODBCDB)
        return iRetCode;

    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);
    //TOdbcDBQuery * pODBCInstance = NULL;//ZMP:305593,变量未用到
    try
    {
        //pODBCInstance = (TOdbcDBQuery *)*pODBCDB;
        //iRetCode = pODBCInstance->FreeOdbcDBQuery();
        iRetCode = 0;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:",sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    { 
        //ZMP:467376
        strncpy(strErrorInfo,"TException:",sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }
    if (iRetCode != 0)
    {
        WriteLog_TCI,"FreeODBCDB iRetCode = [%d],ErrorInfo = [%s]\n",iRetCode,strErrorInfo);
    }
    return iRetCode;
}

//重新连接,TOdbcDBDatabase
DLL_API bool ReConnect(void **pODBCDBDateBase)
{
    bool bRetCode = false;
    if (!*pODBCDBDateBase)
        return false;
    TOdbcDBDatabase * pOdbcDBLink = NULL;
    char strErrorInfo[ERROR_BUF] = "\0";
	memset(strErrorInfo,0,ERROR_BUF);
    try
    {
        pOdbcDBLink = (TOdbcDBDatabase *)*pODBCDBDateBase;
        if (!pOdbcDBLink->IsConnect())
        {
            bRetCode = pOdbcDBLink->Connect();
        }
        bRetCode = true;
    }
    catch(TOdbcDBException &oe)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"TOdbcDBException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(TException &oe)
    {   
        //ZMP:467376
        strncpy(strErrorInfo,"TException:", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
        strncat(strErrorInfo,oe.GetErrMsg(),sizeof(strErrorInfo)-strlen(strErrorInfo));
    }
    catch(...)
    {
        //ZMP:467376
        strncpy(strErrorInfo,"Unknow Exception.", sizeof(strErrorInfo)-1);
        strErrorInfo[sizeof(strErrorInfo)-1] = '\0';
    }

    if (bRetCode == false)
    {
        WriteLog_TCI,"ReConnect bRetCode = [%s],ErrorInfo = [%s]\n",bRetCode?"TRUE":"FALSE",strErrorInfo);
    }
    return bRetCode;
}