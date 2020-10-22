//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
/*TOraDBQuery.cpp*/
#include "TOraDBQuery.h"
#include "debug_new.h"
#include "Common.h"
#include "ReadIni.h"
#include "TStrFunc.h"


// FILE_CODE stands for the current file's code. FILE_CODE must be unique in a module. FILE_CODE corresponds to a CPP file uniquely.
#define FILE_CODE 1

// CURRENT_MAX_ERROR_LOG stands for the maximum of the current error level logCode (error level logCode must be between 1 and 200), if you want to add a new ADD_ERROR in this file, please use CURRENT_MAX_ERROR_LOG+1 as its Log code and then change CURRENT_MAX_ERROR_LOG into CURRENT_MAX_ERROR_LOG+1!
#define CURRENT_MAX_ERROR_LOG  FILE_CODE * 1000 + 0 

// CURRENT_MAX_ERROR_LOG stands for the maximum of the current warn level logCode (warn level logCode must be between 201 and 400), if you want to add a new ADD_WARN in this file, please use CURRENT_MAX_WARN_LOG+1 as its Log code and then change CURRENT_MAX_ERROR_LOG into CURRENT_MAX_WARN_LOG+1!
#define CURRENT_MAX_WARN_LOG  FILE_CODE * 1000 + 200 

// CURRENT_MAX_ERROR_LOG stands for the maximum of the current normal level logCode (other level logCode must be between 401 and 999), if you want to add a new ADD_TRACK_XX in this file, please use CURRENT_MAX_OTHER_LOG+1 aas its Log code and then change CURRENT_MAX_ERROR_LOG into CURRENT_MAX_ERROR_LOG+1!
#define CURRENT_MAX_OTHER_LOG  FILE_CODE * 1000 + 402 


#define LOG_TDBQuery        LOG_CODE_DBC+100

//ZMP:467365   
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

TDBException::TDBException(sword errNumb, OCIError *hErr,const char *cat,const char *sql)
{
    int nLen = 0;
    sb4 errcode;
    
    if( cat ) //325456
    {
        nLen = (int)strlen(cat);//ZMP:477242
        nLen = (nLen >= MAX_ERR_CAT_LENGTH)? MAX_ERR_CAT_LENGTH : nLen;
        strncpy(errCategory,cat,(size_t)nLen);//331093
    }
    errCategory[nLen] = '\0';
    nLen = 0 ;
    if( sql ) //325456
    {
        nLen = (int)strlen(sql);
        nLen = nLen >= MAX_SQLSTMT_LENGTH ? MAX_SQLSTMT_LENGTH : nLen;
        strncpy(m_sErrSql,sql,(size_t)nLen);
    }
    m_sErrSql[nLen] = '\0';

    m_lErrCode = errNumb;

    (void)OCIErrorGet ((dvoid *) hErr, (ub4) 1, (text *) NULL, &errcode,
        (text*)m_sErrMsg, (ub4)sizeof(m_sErrMsg)-1, (ub4) OCI_HTYPE_ERROR);
}

TDBException::TDBException(const char *sql, const char* errFormat, ...)
{
    int nLen = 0;
    if( sql ) //325456
    {
        nLen = (int)strlen(sql);//ZMP:477242
        nLen = (nLen >= MAX_SQLSTMT_LENGTH) ? MAX_SQLSTMT_LENGTH :nLen;
        strncpy(m_sErrSql,sql,(size_t)nLen);
    }
    m_sErrSql[nLen] = '\0';

    va_list ap;
    va_start(ap, errFormat);
    //ZMP:467365  
    vsnprintf((char *)m_sErrMsg, sizeof(m_sErrMsg), errFormat, ap); 
    m_sErrMsg[sizeof(m_sErrMsg)-1] = '\0';
    va_end(ap);
    //ZMP:305593
     memset(errCategory,0,sizeof(errCategory));
}

char * TDBException::ToString() const
{
    static char sBuf[MAX_ERRMSG_LENGTH+MAX_SQLSTMT_LENGTH+1] = {0};  //478823
    sBuf[0] = 0;

    snprintf(sBuf, sizeof(sBuf), "TDBException:\n    ErrMsg=%s\n    ErrSql=%s\n", GetErrMsg(), GetErrSql());
    return sBuf;
}


/********* TConnection implementation *********/
TDBDatabase::TDBDatabase() throw (TDBException)
{
    sword errorNo;

    hUser = NULL;
    hDBSvc = NULL;  
    hDBErr = NULL;
    hEnv = NULL;
    hSvr = NULL;

    errorNo = OCIInitialize((ub4) OCI_DEFAULT|OCI_OBJECT,0, 0,0,0 );
    errorNo = errorNo + OCIEnvInit( (OCIEnv **) &hEnv, (ub4) OCI_DEFAULT,(size_t) 0, (dvoid **) 0 );
    errorNo = errorNo + OCIHandleAlloc( (dvoid *) hEnv, (dvoid **) &hDBSvc,(ub4) OCI_HTYPE_SVCCTX,(size_t) 0, (dvoid **) 0);
    errorNo = errorNo + OCIHandleAlloc( (dvoid *) hEnv, (dvoid **) &hDBErr,(ub4) OCI_HTYPE_ERROR,(size_t) 0, (dvoid **) 0); 
    errorNo = errorNo + OCIHandleAlloc( (dvoid *) hEnv, (dvoid **) &hSvr,(ub4) OCI_HTYPE_SERVER,(size_t) 0, (dvoid **) 0);

    if ( errorNo != 0 )
        throw TDBException( "TDBDatabase()", ERR_DB_INIT, __LINE__);
    fConnected = false;
    usr = NULL;
    pwd = NULL;
    tns = NULL;
    m_pTDBQuery=NULL;

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
        snprintf(sAppCfgName, sizeof(sAppCfgName), "%s/etc/App.config",theEnv);
    }

    TReadIni ReadIni(sAppCfgName); 
    char sItem[512] = {0};

    ReadIni.ReadString("COMMON","ORA_ERROR_IGNORE",sItem,NULL);
    strncpy(m_sErrIgnore,sItem,sizeof(m_sErrIgnore)-1);


}

TDBDatabase::~TDBDatabase()
{
    if(m_pTDBQuery!=NULL)
    {
        m_pTDBQuery->Close();
        DELETE(m_pTDBQuery);
    }

    DELETE_A(usr);
    DELETE_A(pwd);
    DELETE_A(tns);

    if (fConnected) 
        OCIServerDetach(hSvr, hDBErr, OCI_DEFAULT );

    if( hSvr ) //ZMP_579047
    {
        OCIHandleFree(hSvr, OCI_HTYPE_SERVER); 
        hSvr = NULL;
    }
    if( hDBSvc ) //ZMP_579047
    {
        OCIHandleFree(hDBSvc, OCI_HTYPE_SVCCTX);
        hDBSvc=NULL;
    }
    if( hDBErr ) //ZMP_579047
    {
        OCIHandleFree(hDBErr,OCI_HTYPE_ERROR);
        hDBErr = NULL;
    }
    if( hEnv ) //ZMP_579047
    {
        OCIHandleFree(hEnv,OCI_HTYPE_ENV);
        hEnv = NULL; 
    }
}

void TDBDatabase::SetLogin(const char *user, const char *password, const char *tnsString) throw (TDBException)
{
    if (fConnected)
        throw TDBException("SetLogin()", ERR_SET_LOGIN , __LINE__);

    int nLen;
    //保存外部传递的参数
    DELETE_A(usr);
    DELETE_A(pwd);
    DELETE_A(tns);

    //保存外部传递的参数
    if (user)
    {
        nLen = (int)strlen(user);
        usr = new char[nLen+1];
        strncpy(usr,user,(size_t)nLen);
        usr[nLen] = '\0';
    }
    else
    {
        nLen = 0;
        usr = new char[1];
        usr[0] = '\0';
    }

    if (password)
    {
        nLen = (int)strlen(password);
        pwd = new char[nLen+1];
        strncpy(pwd,password,(size_t)nLen);
        pwd[nLen] = '\0';
    }
    else
    {
        nLen = 0;
        pwd = new char[1];
        pwd[0] = '\0';
    }

    if (tnsString)
    {
        nLen = (int)strlen(tnsString);
        tns = new char[nLen+1];
        strncpy(tns,tnsString,(size_t)nLen);
        tns[nLen] = '\0';
    }
    else    
    {
        nLen = 0;
        tns = new char[1];
        tns[0] = '\0';
    }
}

void TDBDatabase::CheckError(const char* sSql) throw (TDBException)
{
    try
    {
        if (fErrorNo != OCI_SUCCESS) 
            throw TDBException(fErrorNo, hDBErr, "Oracle OCI Call", "OCIDatabase");
    }
    catch (TDBException &oe)
    {
        if (m_sErrIgnore[0] !='\0' && strstr(oe.GetErrMsg(),m_sErrIgnore) != NULL)
        {
            printf("Ignore OCI Error: %s\n", oe.GetErrMsg());
            return;
        }
        throw ;
    }
}


bool TDBDatabase::Connect(bool bUnused) throw (TDBException)
{
    PS_BEGIN(BF_TDBDatabase_Connect,this);
    sword errorNo;

    if (fConnected)
    {
        PS_END(BF_TDBDatabase_Connect);
        return true;
    }

    if ( (usr == NULL) || (tns==NULL) )
    {
        PS_END(BF_TDBDatabase_Connect);
        throw TDBException("Connect()", ERR_CONNECT_NO_LOGIN_INFO, __LINE__);
    }

    errorNo = OCIServerAttach(hSvr, hDBErr, (text *)tns, (int)strlen(tns), 0);
    if (errorNo != OCI_SUCCESS)
    {
        PS_END(BF_TDBDatabase_Connect);
        throw TDBException(errorNo, hDBErr, "Connect()", "try to connect Server");
    }

    //modified: 2003.1
    fErrorNo = OCIHandleAlloc(hEnv, (dvoid **) &hUser,(ub4) OCI_HTYPE_SESSION,(size_t) 0, (dvoid **) 0);
    CheckError();

    fErrorNo = OCIHandleAlloc(hEnv, (dvoid **)&hDBSvc, OCI_HTYPE_SVCCTX,0, 0);
    CheckError();

    fErrorNo = OCIAttrSet (hDBSvc, OCI_HTYPE_SVCCTX, hSvr, 0, OCI_ATTR_SERVER, hDBErr);
    CheckError();

    /* set the username/password in user handle */
    OCIAttrSet(hUser, OCI_HTYPE_SESSION, usr, (ub4)strlen(usr),OCI_ATTR_USERNAME, hDBErr);
    OCIAttrSet(hUser, OCI_HTYPE_SESSION, pwd, (ub4)strlen(pwd),OCI_ATTR_PASSWORD, hDBErr);

    // Set the Authentication handle in the service handle
    fErrorNo = OCIAttrSet(hDBSvc, OCI_HTYPE_SVCCTX, hUser, 0, OCI_ATTR_SESSION, hDBErr);
    CheckError();

    fErrorNo=OCISessionBegin (hDBSvc, hDBErr, hUser, OCI_CRED_RDBMS, OCI_DEFAULT);
    CheckError();
    //Set Trans:
    //OCIAttrSet(hDBSvc, OCI_HTYPE_SVCCTX, hTrans, 0, OCI_ATTR_TRANS, hErr);
    PS_END(BF_TDBDatabase_Connect);
    return (fConnected = (errorNo == OCI_SUCCESS));
}

bool TDBDatabase::Connect(const char *inUsr, const char *inPwd, const char *inTns, bool bUnused) throw (TDBException)
{
    SetLogin(inUsr, inPwd, inTns);
    return Connect();
}

int TDBDatabase::Disconnect() throw (TDBException)
{
    PS_BEGIN(BF_TDBDatabase_Disonnect,this);
    //if (!fConnected) return 1;
    OCISessionEnd (hDBSvc, hDBErr, hUser, OCI_DEFAULT);
    sword errorNo = OCIServerDetach(hSvr, hDBErr, OCI_DEFAULT );
    if (errorNo != OCI_SUCCESS)
    {
        PS_END(BF_TDBDatabase_Disonnect);
        throw TDBException(errorNo, hDBErr,"TDBDatabase::Disconnect()", "OCIServerDetach error");
    }
    if( hUser ) //ZMP_579047
    {
        OCIHandleFree(hUser, OCI_HTYPE_SESSION);
        hUser=NULL;
    }
    if( hDBSvc ) //ZMP_579047
    {
        OCIHandleFree(hDBSvc, OCI_HTYPE_SVCCTX);
        hDBSvc=NULL;
    }
    
    fConnected = false;
    PS_END(BF_TDBDatabase_Disonnect);
    return 1;
}

void TDBDatabase::Commit()
{
    PS_BEGIN(BF_TDBDatabase_Commit,this);
    OCITransCommit(hDBSvc, hDBErr, OCI_DEFAULT);
    PS_END(BF_TDBDatabase_Commit);
}

void TDBDatabase::Rollback()
{
    PS_BEGIN(BF_TDBDatabase_Rollback,this);
    OCITransRollback(hDBSvc, hDBErr, OCI_DEFAULT);
    PS_END(BF_TDBDatabase_Rollback);
}

bool TDBDatabase::IsConnect() throw (TDBException)
{
    PS_BEGIN(BF_TDBDatabase_IsConnect,this);
    if(fConnected==true)
    {
        if(m_pTDBQuery==NULL)
        {
            m_pTDBQuery=new TDBQuery(this);
        }

        try
        {
            m_pTDBQuery->Close();
            m_pTDBQuery->SetSQL("SELECT 1 FROM DUAL");
            m_pTDBQuery->Open();
            /*if(m_pTDBQuery->Next())
            {
            }*/
            m_pTDBQuery->Close();
        }
        catch(TDBException &)
        {
            Disconnect();
            fConnected=false;
        }
        catch(...)
        {
            PS_END(BF_TDBDatabase_IsConnect);
            throw TDBException("","TDBDatabase::IsConnect() catch ... Exception");
        }
    }
    PS_END(BF_TDBDatabase_IsConnect);
    return fConnected;
}

TDBQuery * TDBDatabase::CreateDBQuery() throw (TDBExcpInterface)
{
    return new TDBQuery(this);
}

TDBField::TDBField()
{
    //初始化列信息,有部分的初始化信息在Describe中进行
    name = NULL;
    hBlob = NULL;
    hDefine = (OCIDefine *) 0; ;
    fDataBuf = NULL;
    fDataIndicator = NULL;
    fParentQuery = NULL;

    fReturnDataLen = 0;
    size = 0;
    precision = 0;
    scale = 0;
    size = 0;
    lDataBufLen=0;

    sBlob=NULL;
    ahBlob=NULL;
    iBlobCount=0;
    memset(intStr,0x00,sizeof(intStr));
};

TDBField::~TDBField()
{
    DELETE_A(fDataIndicator);
    DELETE_A(name);
    DELETE_A(fDataBuf);
    //if (type == SQLT_BLOB)
    //    OCIDescriptorFree((dvoid *)hBlob, (ub4) OCI_DTYPE_LOB);
    if(ahBlob!=NULL)
    {
        for(int iIndex=0;iIndex<iBlobCount;iIndex++)
        {
            OCIDescriptorFree((dvoid *)ahBlob[iIndex], (ub4) OCI_DTYPE_LOB);
        }
        DELETE_A(ahBlob);
    }

    DELETE_A(sBlob);
}

char* TDBField::AsString() throw (TDBException)
{
    
    //static char intStr[100]={0}; //356853 for cppcheck
    //int status;//ZMP:305593,变量未用到

    if (fParentQuery->fBof || fParentQuery->fEof)
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "AsString()", name);
    /*PS_BEGIN(BF_TDBField_AsString,this);*/
    if ( isNULL() )
    {
        //ZMP:467365  
        snprintf((char *)fStrBuffer, sizeof(fStrBuffer), "%s",NULL_STRING);
        fStrBuffer[sizeof(fStrBuffer)-1] = '\0';
        /*PS_END(BF_TDBField_AsString);*/
        return (char *)fStrBuffer;
    }

    switch ( this->type )
    {
    case DATE_TYPE:
        int year, month, day, hour, minute, second;
        this->AsDateTimeInternal(year, month, day, hour, minute, second);
        //ZMP:467365 
        snprintf((char *)fStrBuffer, sizeof(fStrBuffer), "%04d%02d%02d%02d%02d%02d", year, month, day,hour, minute, second);
        fStrBuffer[sizeof(fStrBuffer)-1] = '\0';
        /*PS_END(BF_TDBField_AsString);*/
        return (char *)fStrBuffer;
    case INT_TYPE:
        llong intValue;
        if (OCINumberToInt(fParentQuery->hErr, (OCINumber *)(fDataBuf + (size+1) * fParentQuery->fCurrRow ),sizeof(intValue), OCI_NUMBER_SIGNED,&intValue)!= OCI_SUCCESS)
        {
            fParentQuery->CheckError();
        }
        //ZMP:467365 
        snprintf(intStr, sizeof(intStr), "%ld", intValue);
        intStr[sizeof(intStr)-1] = '\0';
        /*PS_END(BF_TDBField_AsString);*/
        return intStr;
    case FLOAT_TYPE:
        //int status;
        double floatValue;
        if (OCINumberToReal(fParentQuery->hErr, (OCINumber *)(fDataBuf + (size+1) * fParentQuery->fCurrRow ),sizeof(floatValue), &floatValue)!= OCI_SUCCESS)
        {
            fParentQuery->CheckError();
        }
        //ZMP:467365
        snprintf(intStr, sizeof(intStr), "%lf", floatValue);
        intStr[sizeof(intStr)-1] = '\0';
        /*PS_END(BF_TDBField_AsString);*/
        return intStr;      
        //return((char *)(fDataBuf + (size+1) * fParentQuery->fCurrRow));
    case STRING_TYPE:
    case ROWID_TYPE:
        /*PS_END(BF_TDBField_AsString);*/
        return((char *)(fDataBuf + (size+1) * fParentQuery->fCurrRow));
    case SQLT_BLOB:
        //ZMP:467365
        snprintf((char *)fStrBuffer, sizeof(fStrBuffer), "BLOB...");
        fStrBuffer[sizeof(fStrBuffer)-1] = '\0';
        /*PS_END(BF_TDBField_AsString);*/
        return (char *)fStrBuffer;
    default:
        /*PS_END(BF_TDBField_AsString);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "AsString()");
    }
}

bool TDBField::isNULL()
{
    return (fDataIndicator[fParentQuery->fCurrRow]==-1);
}


void  TDBField::AsLobString(char* const buf, unsigned int maxLength, unsigned int& bufLength) throw (TDBException)
{
    ub1 innerBuf[MAX_LOB_BUFFER_LENGTH] = {0};
    ub4 remainder = 0, nActual = 0, nTry = 0;
    ub4 offset = 1;
    bufLength = 0;
    if (fParentQuery->fBof || fParentQuery->fEof)
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "AsBlobBuffer()", name);

    if (type != SQLT_BLOB  && type != SQLT_CLOB)
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "asLobBuffer()");
    //ZMP:305593,maxLength为u类型，不可能小于0
    if (NULL == buf || maxLength == 0)
        throw TDBException(fParentQuery->fSqlStmt, ERR_READ_PARAM_DATA, name,"AsBlobBuffer()");

    hBlob=ahBlob[fParentQuery->fCurrRow];
    OCILobGetLength(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, &remainder);
    if(remainder <= 0)
    {
         bufLength = 0;
        return ;
    }

    if(remainder > maxLength)
    {
        throw TDBException(fParentQuery->fSqlStmt, ERR_READ_PARAM_DATA, name, "asLobBuffer()");
    }
    /*PS_BEGIN(BF_TDBField_AsLobString,this);*/
    nTry = MAX_LOB_BUFFER_LENGTH-4;//防止MAX_LOB_BUFFER_LENGTH都写满，strlen((char*)innerBuf)访问过界
    int strInnerLen = 0;
    do
    {
        memset(innerBuf, 0, sizeof(innerBuf));
        nActual = 0;
        fParentQuery->fErrorNo = OCILobRead(fParentQuery->db->hDBSvc, fParentQuery->hErr, 
            hBlob, &nActual, (ub4)offset, (dvoid *)innerBuf, (ub4) nTry, (dvoid *)0, 
            (sb4 (*)(dvoid *, CONST dvoid *, ub4, ub1)) 0, (ub2) 0, (ub1) SQLCS_IMPLICIT);
        
        if(OCI_NEED_DATA != fParentQuery->fErrorNo
            && OCI_SUCCESS != fParentQuery->fErrorNo )
        {
            fParentQuery->CheckError();
        }

        strInnerLen = (int)strlen((char*)innerBuf);//ZMP:477242
        if(maxLength > (bufLength+(unsigned int)strInnerLen))
        {
            memcpy( (buf) + bufLength, innerBuf, (size_t)strInnerLen);
        }
        else 
        {
            /*PS_END(BF_TDBField_AsLobString);*/
            throw TDBException(fParentQuery->fSqlStmt, ERR_READ_PARAM_DATA, name, "asLobBuffer()");
        }
        
        if (nActual<=0) break;

        offset += nActual;
        bufLength += (unsigned int)strInnerLen;
    }while(OCI_NEED_DATA == fParentQuery->fErrorNo);

    if(fParentQuery->fErrorNo!=OCI_SUCCESS)
    {
        bufLength=0;    
    }   
    /*PS_END(BF_TDBField_AsLobString);*/
}


void  TDBField::AsBlobFile(const char *fileName) throw (TDBException)
{
    ub4 offset = 1;
    ub1 buf[MAX_LOB_BUFFER_LENGTH]={0}; //356853 for cppcheck
    ub4 nActual = 0;    //实际读取数
    ub4 nTry = 0;       //试图读取数
    ub4 nleftLen = 0;    
    ub4 totalSize = 0;
    FILE *fileHandle = NULL; //356853 for cppcheck
    ub4 lobLength = 0;

    if (fParentQuery->fBof || fParentQuery->fEof)
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "AsBlobFile()", name);

    if (type != SQLT_BLOB && type != SQLT_CLOB)
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "asLobFile()");

    fileHandle = fopen( fileName, "wb");
    if (NULL == fileHandle)
    {
        int iError = errno;
        throw TDBException(fParentQuery->fSqlStmt, ERR_OPEN_FILE, fileName, iError, strerror(iError));
    }
    int iRet = fseek(fileHandle, 0, 0);
    //356853 for cppcheck 增加对fseek返回值的判断
    if( iRet != 0 )
    {
        fclose(fileHandle);
        fileHandle = NULL;
        throw TDBException(fParentQuery->fSqlStmt,ERR_INVALID_METHOD_CALL,"TDBField::AsBlobFile","fseek");      
    }
    /* set amount to be read per iteration */
    nTry = MAX_LOB_BUFFER_LENGTH;
    hBlob=ahBlob[fParentQuery->fCurrRow];
    OCILobGetLength(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, &lobLength);
    if(lobLength <= 0)
    {
        fclose(fileHandle);//ZMP:305593
        fileHandle = NULL; //356853 for cppcheck
        return ;
    }
    /*PS_BEGIN(BF_TDBField_AsBlobFile,this);*/
    nleftLen = lobLength;
    while (nleftLen > 0)
    {
        nActual = 0; 
        fParentQuery->fErrorNo = OCILobRead(fParentQuery->db->hDBSvc, fParentQuery->hErr, 
            hBlob, &nActual, (ub4)offset, (dvoid *) buf, (ub4) nTry, (dvoid *)0, 
            (sb4 (*)(dvoid *, CONST dvoid *, ub4, ub1)) 0, (ub2) 0, (ub1) SQLCS_IMPLICIT);
        if(OCI_NEED_DATA != fParentQuery->fErrorNo 
            && OCI_SUCCESS != fParentQuery->fErrorNo )
        {
            fParentQuery->CheckError();
        }     

        if (nActual<=0) break;

        totalSize += nActual;
        int iTotalWrite = (int)fwrite((void *)buf, (size_t)nActual, (size_t)1, fileHandle);//ZMP:477242
        if( iTotalWrite != 1 )
        {
            fclose(fileHandle);
            fileHandle = NULL;
            /*PS_END(BF_TDBField_AsBlobFile);*/
            throw TDBException(fParentQuery->fSqlStmt,ERR_INVALID_METHOD_CALL,"TDBField::AsBlobFile","fwrite");        
        }
        offset += nActual;
        nleftLen -= nActual;					
    }
    fclose(fileHandle);
    fileHandle = NULL; //356853 for cppcheck
    /*PS_END(BF_TDBField_AsBlobFile);*/
}

void  TDBField::AsBlobBuffer(unsigned char* &buf, unsigned int *lobLength) throw (TDBException)
{
    /*PS_BEGIN(BF_TDBField_AsBlobBuffer,this);*/
    ub1 innerBuf[MAX_LOB_BUFFER_LENGTH]={0}; //356853 for cppcheck
    ub4 remainder = 0, nActual = 0, nTry = 0;
    ub4 offset = 1;

    if (fParentQuery->fBof || fParentQuery->fEof)
    {
        /*PS_END(BF_TDBField_AsBlobBuffer);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "AsBlobBuffer()", name);
    }
    if (type != SQLT_BLOB && type != SQLT_CLOB)
    {
        /*PS_END(BF_TDBField_AsBlobBuffer);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "asLobBuffer()");
    }
    hBlob=ahBlob[fParentQuery->fCurrRow];
    OCILobGetLength(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, &remainder);
    *lobLength = nActual = nTry = remainder;
    if(remainder <= 0)
    {
        buf = NULL;
        /*PS_END(BF_TDBField_AsBlobBuffer);*/
        return ;
    }		

    try
    {
        buf = new unsigned char[sizeof(ub1) * remainder+1];
        buf[sizeof(ub1) * remainder] = 0;
    }
    catch (...)
    {
        /*PS_END(BF_TDBField_AsBlobBuffer);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_NOMORE_MEMORY_BE_ALLOCATED, "AsBlobBuffer()", __LINE__);
    }

    nTry = MAX_LOB_BUFFER_LENGTH;
    while (remainder > 0)
    {
        nActual = 0;    
        fParentQuery->fErrorNo = OCILobRead(fParentQuery->db->hDBSvc, fParentQuery->hErr, 
            hBlob, &nActual, (ub4)offset, (dvoid *)innerBuf, (ub4) nTry, (dvoid *)0, 
            (sb4 (*)(dvoid *, CONST dvoid *, ub4, ub1)) 0, (ub2) 0, (ub1) SQLCS_IMPLICIT);
        if(OCI_NEED_DATA != fParentQuery->fErrorNo
            && OCI_SUCCESS != fParentQuery->fErrorNo )
        {
            fParentQuery->CheckError();
        }
        
        if(remainder <= nActual)
        {
            memcpy( (buf) + offset -1, innerBuf, remainder);        
            break;
        }
        else 
        {
            memcpy( (buf) + offset -1, innerBuf, nActual);
        }
				
        if (nActual<=0) break;

        offset += nActual;
        remainder -= nActual;
    }
    /*PS_END(BF_TDBField_AsBlobBuffer);*/
}

char* TDBField::AsBlobBuffer(int &iBufferLen) throw (TDBException)
{
    /*PS_BEGIN(BF_TDBField_AsBlobBuffer2,this);*/
    ub1 innerBuf[MAX_LOB_BUFFER_LENGTH]={0}; //356853 for cppcheck
    ub4 remainder = 0, nActual = 0, nTry = 0;
    ub4 offset = 1;

    if (fParentQuery->fBof || fParentQuery->fEof)
    {
        /*PS_END(BF_TDBField_AsBlobBuffer2);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "AsBlobBuffer()", name);
    }
    if (type != SQLT_BLOB && type != SQLT_CLOB)
    {
        /*PS_END(BF_TDBField_AsBlobBuffer2);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "asLobBuffer()");
    }
    hBlob=ahBlob[fParentQuery->fCurrRow];
    OCILobGetLength(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, &remainder);
    iBufferLen = (int)(nActual = nTry = remainder);
    if(remainder <= 0)
    {
        /*PS_END(BF_TDBField_AsBlobBuffer2);*/
        return NULL;
    }

    try
    {
        DELETE_A(sBlob);
        sBlob = new char[sizeof(ub1) * remainder + 1];
        sBlob[sizeof(ub1) * remainder] = 0;
    }
    catch (...)
    {
        /*PS_END(BF_TDBField_AsBlobBuffer2);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_NOMORE_MEMORY_BE_ALLOCATED, "AsBlobBuffer()", __LINE__);
    }

    nTry = MAX_LOB_BUFFER_LENGTH;
    while (remainder > 0)
    { 
        nActual = 0; 
        fParentQuery->fErrorNo = OCILobRead(fParentQuery->db->hDBSvc, fParentQuery->hErr, 
            hBlob, &nActual, (ub4)offset, (dvoid *)innerBuf, (ub4) nTry, (dvoid *)0, 
            (sb4 (*)(dvoid *, CONST dvoid *, ub4, ub1)) 0, (ub2) 0, (ub1) SQLCS_IMPLICIT);
        if(OCI_NEED_DATA != fParentQuery->fErrorNo
            && OCI_SUCCESS != fParentQuery->fErrorNo )
        {
            fParentQuery->CheckError();
        }

        if(remainder <= nActual)
        {
            memcpy( (sBlob) + offset -1, innerBuf, remainder);        
            break;
        }
        else 
        {
            memcpy( (sBlob) + offset -1, innerBuf, nActual);
        }
				
        if (nActual<=0) break;

        offset += nActual;
        remainder -= nActual;
    }
    /*PS_END(BF_TDBField_AsBlobBuffer2);*/
    return sBlob;
}

void  TDBField::LoadFromFile(const char *fileName) throw (TDBException)
{
    ub4 remainder, nActual, nTry, offset = 1;//从文件中读取的剩余数据量
    ub1 buf[MAX_LOB_BUFFER_LENGTH]={0}; //356853 for cppcheck
    ub4 LobLength;
    ub4  flushedAmount = 0;
    FILE *fileHandle=NULL ; //356853 for cppcheck

    if (fParentQuery->fBof || fParentQuery->fEof)
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "LoadFromFile()", name);

    if (type != SQLT_BLOB)
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "LoadFromFile()");

    if (NULL == (fileHandle = fopen(fileName, "rb")))
    {
        int iError = errno;
        throw TDBException(fParentQuery->fSqlStmt, ERR_OPEN_FILE, fileName, iError, strerror(iError));
    }

    int iRet = fseek(fileHandle,0,SEEK_END);
    //356853 for cppcheck,增加对返回值的判断
    if( iRet != 0 )
    {
        fclose(fileHandle);
        fileHandle = NULL;
        throw TDBException(fParentQuery->fSqlStmt,ERR_INVALID_METHOD_CALL,"TDBField::LoadFromFile","fseek");        
    }
    remainder = (unsigned int)ftell(fileHandle);//ZMP:477242
    iRet = fseek(fileHandle, 0, 0);
    //356853 for cppcheck,增加对返回值的判断
    if( iRet != 0 )
    {
        fclose(fileHandle);
        fileHandle = NULL;
        throw TDBException(fParentQuery->fSqlStmt,ERR_INVALID_METHOD_CALL,"TDBField::LoadFromFile","Second fseek");        
    }
    
    hBlob=ahBlob[fParentQuery->fCurrRow];
    fParentQuery->fErrorNo = OCILobGetLength(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, &LobLength);
    fParentQuery->CheckError();

    fParentQuery->fErrorNo = OCILobTrim(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, 0);
    fParentQuery->CheckError();

    /* enable the BLOB locator for buffering operations */
    fParentQuery->fErrorNo = OCILobEnableBuffering(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob);
    fParentQuery->CheckError();
    PS_BEGIN(BF_TDBField_LoadFromFile,this);
    while ( (remainder > 0) && !feof(fileHandle))
    {
        nActual = nTry = ((int)remainder > MAX_LOB_BUFFER_LENGTH) ? MAX_LOB_BUFFER_LENGTH : remainder;

        if (fread((void *)buf, (size_t)nTry, (size_t)1, fileHandle) != (size_t)1)
            throw TDBException(fParentQuery->fSqlStmt, ERR_MEM_BUFFER_IO, name, fileName, __LINE__);

        fParentQuery->fErrorNo = OCILobWrite(fParentQuery->db->hDBSvc, fParentQuery->hErr, 
            hBlob, &nActual, offset, (dvoid *) buf, (ub4) nTry, OCI_ONE_PIECE, (dvoid *)0,
            (sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 *)) 0,    (ub2) 0, (ub1) SQLCS_IMPLICIT);
        if ( fParentQuery->fErrorNo != OCI_SUCCESS) 
        {
            fclose(fileHandle);
            fileHandle = NULL; //356853 for cppcheck
            fParentQuery->CheckError();
        }

        flushedAmount += nTry;
        remainder -= nTry;
        offset += nTry;
        //incase the internal buffer is not big enough for the lob , flush the buffer content to db after some interval:
        if ((int)flushedAmount >= LOB_FLUSH_BUFFER_SIZE)
        {
            flushedAmount = 0;
            fParentQuery->fErrorNo = OCILobFlushBuffer(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, OCI_LOB_BUFFER_NOFREE);
            fParentQuery->CheckError(); 
        }
    }

    if ( flushedAmount )
    {
        fParentQuery->fErrorNo = OCILobFlushBuffer(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, OCI_LOB_BUFFER_NOFREE);
        fParentQuery->CheckError(); 
    }
    fclose(fileHandle);
    fileHandle = NULL; //356853 for cppcheck
    PS_END(BF_TDBField_LoadFromFile);
}

void  TDBField::LoadFromBuffer(unsigned char *buf, unsigned int bufLength) throw (TDBException)
{
    ub1 innerBuf[MAX_LOB_BUFFER_LENGTH]={0}; //356853 for cppcheck

    ub4 remainder, nActual, nTry;
    ub4  flushedAmount = 0, offset = 1;

    if (fParentQuery->fBof || fParentQuery->fEof)
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "LoadFromBuffer()", name);

    if (type != SQLT_BLOB)
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "LoadFromBuffer()");

    hBlob=ahBlob[fParentQuery->fCurrRow];
    fParentQuery->fErrorNo = OCILobTrim(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, 0);
    fParentQuery->CheckError();
    remainder = bufLength;

    /* enable the BLOB locator for buffering operations */
    fParentQuery->fErrorNo = OCILobEnableBuffering(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob);
    fParentQuery->CheckError();
    PS_BEGIN(BF_TDBField_LoadFromBuffer,this);
    while (remainder > 0)
    {
        nActual = nTry = ((int)remainder > MAX_LOB_BUFFER_LENGTH) ? MAX_LOB_BUFFER_LENGTH : remainder;

        memcpy(innerBuf, buf + offset-1, nActual);

        fParentQuery->fErrorNo = OCILobWrite(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob,
            &nActual, offset, (dvoid *)innerBuf, (ub4)nTry, OCI_ONE_PIECE, (dvoid *)0,
            (sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 *)) 0,    (ub2) 0, (ub1) SQLCS_IMPLICIT);
        fParentQuery->CheckError();

        flushedAmount += nTry;
        remainder -= nTry;
        offset += nTry;

        if ((int)flushedAmount >= LOB_FLUSH_BUFFER_SIZE)
        {
            flushedAmount = 0;
            fParentQuery->fErrorNo = OCILobFlushBuffer(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, OCI_LOB_BUFFER_NOFREE);
            fParentQuery->CheckError(); 
        }
    }

    if ( flushedAmount )
    {
        fParentQuery->fErrorNo = OCILobFlushBuffer(fParentQuery->db->hDBSvc, fParentQuery->hErr, hBlob, OCI_LOB_BUFFER_NOFREE);
        fParentQuery->CheckError(); 
    }
    PS_END(BF_TDBField_LoadFromBuffer);
}

double TDBField::AsFloat() throw (TDBException)
{
    double iRet; 
    //int status;//ZMP:305593,变量未用到
    if (fParentQuery->fBof || fParentQuery->fEof)
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "AsFloat()", name);
    /*PS_BEGIN(BF_TDBField_AsFloat,this);*/
    if ( isNULL() )
    {
        /*PS_END(BF_TDBField_AsFloat);*/
        return 0;
    }
    if ( (type == FLOAT_TYPE) || ( type == INT_TYPE) ){
        if (OCINumberToReal(fParentQuery->hErr, (OCINumber *)(fDataBuf + (size+1) * fParentQuery->fCurrRow ),sizeof(iRet), &iRet)!= OCI_SUCCESS)
        {
            fParentQuery->CheckError();
        }
        /*PS_END(BF_TDBField_AsFloat);*/
        return iRet;
    }

    else    
    {
        /*PS_END(BF_TDBField_AsFloat);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "AsFloat()");
    }
}

char* TDBField::AsDateTimeString() throw (TDBException)
{
    int year, month, day, hour, minute, second;

    if (fParentQuery->fBof || fParentQuery->fEof)
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "AsDateTimeString()", name);

    if (type != DATE_TYPE)
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "AsDateTimeString()");
    else
    {
        /*PS_BEGIN(BF_TDBField_AsDateTimeString,this);*/
        this->AsDateTimeInternal(year, month, day, hour, minute, second);
        if ( year == 0 )
        {
            //ZMP:467365
            snprintf( (char *)fStrBuffer, sizeof(fStrBuffer), "%s", NULL_STRING);
            fStrBuffer[sizeof(fStrBuffer)-1] = '\0';
        }
        else
        {
            snprintf( (char *)fStrBuffer, sizeof(fStrBuffer), "%04d%02d%02d%02d%02d%02d", year, month, day, hour, minute,second);
            fStrBuffer[sizeof(fStrBuffer)-1] = '\0';
        }
        /*PS_END(BF_TDBField_AsDateTimeString);*/
        return (char *)fStrBuffer;
    }
}

void    TDBField::AsDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBException)
{
    if (fParentQuery->fBof || fParentQuery->fEof)
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "AsDateTime()", name);

    if (type != DATE_TYPE)
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "AsDateTime()");
    else 
    {
        /*PS_BEGIN(BF_TDBField_AsDateTime,this);*/
        this->AsDateTimeInternal(year, month, day, hour, minute, second);
        /*PS_END(BF_TDBField_AsDateTime);*/
    }
}


time_t TDBField::AsTimeT() throw (TDBException)
{
    /*PS_BEGIN(BF_TDBField_AsTimeT,this);*/
    time_t tSeconds;
    struct tm ts;
    
    if (fParentQuery->fBof || fParentQuery->fEof)
    {
        /*PS_END(BF_TDBField_AsTimeT);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "AsTimeT()", name);
    }

    if ((type != DATE_TYPE) && (type != STRING_TYPE))
    {
        /*PS_END(BF_TDBField_AsTimeT);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "AsTimeT()");
    }
    else if (isNULL())
    {
        /*PS_END(BF_TDBField_AsTimeT);*/
        return (time_t)0;
    }
    else if(type == DATE_TYPE)
    {
        this->AsDateTimeInternal(ts.tm_year, ts.tm_mon, ts.tm_mday, ts.tm_hour, ts.tm_min, ts.tm_sec);
        
        ts.tm_year -= 1900;
        ts.tm_mon  -= 1;
        ts.tm_isdst = (int)-1;

        tSeconds = mktime(&ts);
        /*PS_END(BF_TDBField_AsTimeT);*/
        return tSeconds;
    }
    else
    {
        int i = -1;
        i = this->AsDateTimeInternalNew(ts.tm_year, ts.tm_mon, ts.tm_mday, ts.tm_hour, ts.tm_min, ts.tm_sec);
        if( 0 == i )
        {
            /*PS_END(BF_TDBField_AsTimeT);*/
            return (time_t)0;
        }

        ts.tm_year -= 1900;
        ts.tm_mon  -= 1;
        ts.tm_isdst = (int)-1;

        tSeconds = mktime(&ts);
        /*PS_END(BF_TDBField_AsTimeT);*/
        return tSeconds;    
    }
}


void    TDBField::AsDateTimeInternal(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBException)
{
    unsigned char cc,yy,mm,dd,hh,mi,ss;
    ub1 *data=NULL; //356853 for cppcheck

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
    data = fDataBuf + 7 * (fParentQuery->fCurrRow);
    cc=data[0]; 
    yy=data[1]; 
    mm=data[2]; 
    dd=data[3]; 
    hh=(unsigned char)(data[4]-1); //ZMP:477242
    mi=(unsigned char)(data[5]-1); 
    ss=(unsigned char)(data[6]-1); 
    cc=(unsigned char)ABS(cc-100);
    yy=(unsigned char)ABS(yy-100);
    year = (int)cc*100 + (int)yy;
    month = mm;
    day = dd;
    hour = hh;
    minute = mi;
    second = ss;
}

//Modify by zhangyu 20050314
long long TDBField::AsInteger() throw (TDBException)
{   
    /*PS_BEGIN(BF_TDBField_AsInteger,this);*/
    long long iRet=0;
    //int status;//ZMP:305593,变量未用到
    if (fParentQuery->fBof || fParentQuery->fEof)
    {
        /*PS_END(BF_TDBField_AsInteger);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_NO_DATASET, "AsInteger()", name);
    }
    if (type != INT_TYPE && type != FLOAT_TYPE)
    {
        /*PS_END(BF_TDBField_AsInteger);*/
        throw TDBException(fParentQuery->fSqlStmt, ERR_DATA_TYPE_CONVERT, name, type, "AsInteger()");
    }
    else {
        if (OCINumberToInt(fParentQuery->hErr, (OCINumber *)(fDataBuf + (size+1) * fParentQuery->fCurrRow ),sizeof(iRet), OCI_NUMBER_SIGNED,&iRet)!= OCI_SUCCESS)
        {
            fParentQuery->CheckError();
        }
        /*PS_END(BF_TDBField_AsInteger);*/
        return iRet;
    }
}

void TDBField::ClearDataBuf()
{
    if((fDataBuf!=NULL)
        &&(lDataBufLen>0)
        )
    {
        memset(fDataBuf,0,(size_t)lDataBufLen);
    }

    return;
}

char *TDBField::GetFieldName()    //获取列名
{
    return name;
}

long TDBField::GetFieldType()    //获取列名类型
{
    return type;
}

long  TDBField::GetFieldSize()     //获取列大小
{
    return size;
}

int  TDBField::GetFieldPrecision()     //获取列精度
{
    return precision;
}

/*********** TDBQuery Implementation************/
TBillLog *TDBQuery::gpBillLog = NULL;
TDBQuery::TDBQuery(TDBDatabase *oradb) throw (TDBException)
{
	if ( NULL == oradb )	
		throw TDBException("", ERR_GENERAL, "TDBQuery(TDBDatabase &db): param db is NULL");
	
	
    if (! oradb->fConnected)
        throw TDBException("", ERR_GENERAL, "TDBQuery(TDBDatabase &db): Can not declare a TDBQuery when the database is not connected");

    fFetched = 0;
    fPrefetchRows = 1;
    fCurrRow = 0;
    fTotalRowsFetched = 0;
    fBof = false;
    fEof = false;
    fLastRecordFlag = false;
    
#ifdef __DEBUG__
    bExecuteFlag = false;
#endif
    nTransTimes = 0;
    db = oradb;
    fActivated = false;
    fFieldCount = 0;
    fParamCount = 0;

    fSqlStmt = NULL;
    paramList = NULL;
    fieldList = NULL;


    fErrorNo = OCIHandleAlloc(db->hEnv, (dvoid **) &hErr,(ub4) OCI_HTYPE_ERROR,(size_t) 0, (dvoid **) 0);
    CheckError();

    fErrorNo = OCIHandleAlloc(db->hEnv, (dvoid **)&hStmt, OCI_HTYPE_STMT, 0, 0);
    CheckError();

    //fErrorNo = OCIHandleAlloc(db->hEnv, (dvoid **)&hTrans, OCI_HTYPE_TRANS, 0, 0);
    //CheckError();
}

TDBQuery::TDBQuery(TDBDatabase *oradb,TDBSession *session) throw (TDBException)
{
	if ( NULL == oradb )	
		throw TDBException("", ERR_GENERAL, "TDBQuery(TDBDatabase &db): param db is NULL");

	if ( NULL == session )	
		throw TDBException("", ERR_GENERAL, "TDBQuery(TDBDatabase &db): param session is NULL");

    if (! session->m_bActive)
        throw TDBException("", ERR_GENERAL, "TDBQuery(TDBDatabase &db): Can not declare a TDBQuery when the database is not connected");

    fFetched = 0;
    fPrefetchRows = 1;
    fCurrRow = 0;
    fTotalRowsFetched = 0;
    fBof = false;
    fEof = false;
    fLastRecordFlag = false;
    
#ifdef __DEBUG__
    bExecuteFlag = false;
#endif
    nTransTimes = 0;
    db = oradb;
    fActivated = false;
    fFieldCount = 0;
    fParamCount = 0;

    fSqlStmt = NULL;
    paramList = NULL;
    fieldList = NULL;

    /*  hUser = session->m_hSession;
    */
    hErr = session->m_hError;
    /*hSvc = session->m_hSrvCtx;
    */
    fErrorNo = OCIHandleAlloc(db->hEnv, (dvoid **)&hStmt, OCI_HTYPE_STMT, 0, 0);
    CheckError();

    //fErrorNo = OCIHandleAlloc(db->hEnv, (dvoid **)&hTrans, OCI_HTYPE_TRANS, 0, 0);
    //CheckError(); 
}


TDBQuery::~TDBQuery()
{
    DELETE_A(fSqlStmt);
    if (fParamCount >0)
        DELETE_A(paramList);
    if (fFieldCount >0)
        DELETE_A(fieldList);
    if (nTransTimes)
    {
        //fErrorNo = OCITransRollback(db->hDBSvc, hErr, OCI_DEFAULT);
        //CheckError();
    }
#ifdef __DEBUG__
    if(bExecuteFlag)
        userlog("TDBQuery执行了Execute()但是没有提交或回滚,可能造成锁表。"); 
#endif
    /*
    OCISessionEnd (hSvc, hErr, hUser, OCI_DEFAULT);
    */
    OCIHandleFree(hStmt, OCI_HTYPE_STMT);
    //OCIHandleFree(hTrans,OCI_HTYPE_TRANS);
    /*OCIHandleFree(hSvc, OCI_HTYPE_SVCCTX);
    OCIHandleFree(hUser,OCI_HTYPE_SESSION);
    */
    OCIHandleFree(hErr,OCI_HTYPE_ERROR);
}

void TDBQuery::Close()
{
    PS_BEGIN(BF_TDBQuery_Close,this);
    if (! fActivated)
    {
        PS_END(BF_TDBQuery_Close);
        return;
    }

    DELETE_A(fSqlStmt);
  
    if (fParamCount > 0)
    {
        DELETE_A(paramList);
    }
    
    if (fFieldCount > 0)
    {
        DELETE_A(fieldList);
    }

    fFieldCount = 0;
    fParamCount = 0;
    fActivated = false;

    fFetched = 0;
    fPrefetchRows = PREFETCH_ROWS;
    fCurrRow = 0;
    fTotalRowsFetched = 0;
    PS_END(BF_TDBQuery_Close);
}

void TDBQuery::SetBillLog(TBillLog *pBillLog)
{
    gpBillLog = pBillLog;
}

bool TDBQuery::Commit()
{
    PS_BEGIN(BF_TDBQuery_Commit,this);
    //bool exeSuccess = false;//ZMP:305593,变量未用到
#ifdef __DEBUG__
    bExecuteFlag = false;
#endif
    fErrorNo = OCITransCommit(db->hDBSvc, hErr, OCI_DEFAULT);
    CheckError();
    if (fErrorNo == OCI_SUCCESS)
        nTransTimes = 0;
    PS_END(BF_TDBQuery_Commit);
    return (fErrorNo == OCI_SUCCESS);   
}

bool TDBQuery::Rollback()
{
    PS_BEGIN(BF_TDBQuery_Rollback,this);
    bool exeSuccess = false;
#ifdef __DEBUG__
    bExecuteFlag = false;
#endif
    fErrorNo = OCITransRollback(db->hDBSvc, hErr, OCI_DEFAULT);
    if (fErrorNo == OCI_SUCCESS)
    {
        nTransTimes = 0;
        exeSuccess = true;
    }
    else
        CheckError();
    PS_END(BF_TDBQuery_Rollback);
    return exeSuccess;
}

void TDBQuery::GetFieldsDef() throw (TDBException)
{
    PS_BEGIN(BF_TDBQuery_GetFieldsDef,this);
    TDBField *pCurrField=NULL; //356853 for cppcheck
    OCIParam    *param = (OCIParam *)0;
    ub4  counter;                 //计数器，在循环语句中实用
    ub4  nColumnCount;            //在分析sqlstmt后，获得的列数目
    //以下参数的长度参见"OCI Programmer's" Guide P208 , 6-10
    //356853 for cppcheck
    text   *columnName=NULL;           //字段名称
    ub4  columnNameLength,j;      //字段名称长度 unsigned int
    ub2  innerDataSize;           //数据长度 unsigned short
    ub2  innerDataType;           //Oracle 内部数据类型 signed short
    ub2  innerPrecision;          //包括小数点的总位数, ub1 is a bug in documentation?
    sb1  innerScale;              //小数点个数
    ub1  innerIsNULL;             //是否允许为空值

    if(fStmtType==OCI_STMT_SELECT)
        fErrorNo = OCIStmtExecute(db->hDBSvc, hStmt, hErr, (ub4)0, (ub4)0, 0, 0, OCI_DEFAULT);
    else    
        fErrorNo = OCIStmtExecute(db->hDBSvc, hStmt, hErr, (ub4)1, (ub4)0, 0, 0, OCI_DEFAULT);
    CheckError(); 
    //在Execute后，可以获得列的个数和行的个数?
    //如果没有为select语句的返回值定义输出缓冲区，则hErr后的参数iters应当设置为0而不是>0;如果是非SELECT语句，此值>0;
    fErrorNo = OCIAttrGet((dvoid *)hStmt, (ub4)OCI_HTYPE_STMT, (dvoid *)&nColumnCount, (ub4 *) 0, (ub4)OCI_ATTR_PARAM_COUNT, hErr);
    CheckError();

    if (fFieldCount >0 )
    {
        DELETE_A(fieldList);
    }
    fieldList = new TDBField[nColumnCount];
    fFieldCount = (int)nColumnCount;

    for(counter=1; counter<=nColumnCount ; counter ++)
    {
        fErrorNo = OCIParamGet(hStmt, OCI_HTYPE_STMT, hErr, (dvoid **)&param, counter);
        CheckError();

        // column name and column name length
        fErrorNo = OCIAttrGet((dvoid*)param, OCI_DTYPE_PARAM, (dvoid**)&columnName,(ub4 *)&columnNameLength, OCI_ATTR_NAME, hErr);
        CheckError();

        // data length 
        fErrorNo = OCIAttrGet((dvoid*)param, OCI_DTYPE_PARAM, (dvoid *)&innerDataSize, (ub4 *)0, OCI_ATTR_DATA_SIZE, hErr);
        CheckError();

        // precision 
        fErrorNo = OCIAttrGet((dvoid*)param, OCI_DTYPE_PARAM, (dvoid *)&innerPrecision, (ub4 *)0, OCI_ATTR_PRECISION, hErr);
        CheckError();

        // scale 
        fErrorNo = OCIAttrGet((dvoid*)param, OCI_DTYPE_PARAM, (dvoid *)&innerScale, (ub4 *)0, OCI_ATTR_SCALE, hErr);
        CheckError();

        // isNULL
        fErrorNo = OCIAttrGet((dvoid*)param, OCI_DTYPE_PARAM, (dvoid *)&innerIsNULL, (ub4 *)0, OCI_ATTR_IS_NULL, hErr);
        CheckError();

        // data type:
        fErrorNo = OCIAttrGet((dvoid*)param, OCI_DTYPE_PARAM, (dvoid *)&innerDataType, (ub4 *)0, OCI_ATTR_DATA_TYPE, hErr);
        CheckError();

        pCurrField = &fieldList[counter-1];

        pCurrField->name = new char[columnNameLength+1];
        if (pCurrField->name == NULL)
            throw TDBException(fSqlStmt, ERR_NOMORE_MEMORY_BE_ALLOCATED, "GetFieldsDef()", __LINE__);
        for (j=0; j<columnNameLength; j++)
            pCurrField->name[j] = (char)columnName[j];
        pCurrField->name[columnNameLength] = '\0';

        pCurrField->nullable = innerIsNULL>0;

        pCurrField->type = innerDataType; //初始化为内部类型，用于错误返回
        pCurrField->fParentQuery = this;
        pCurrField->fDataIndicator = new sb2[fPrefetchRows];

        switch  (innerDataType)
        {
        case SQLT_NUM://NUMBER_TYPE: 
            if (! innerDataSize)
                pCurrField->size = 255;
            else    
                pCurrField->size = innerDataSize;
            pCurrField->precision = innerPrecision;
            pCurrField->scale = innerScale;
            //预先定义字符缓冲区，用于接收数值到字符转换的结果
            pCurrField->fDataBuf = new ub1[fPrefetchRows * (pCurrField->size+1)];
            pCurrField->lDataBufLen=(long)(sizeof(ub1)*fPrefetchRows)*(pCurrField->size+1);
            if (innerScale == 0) //没有小数点，为整数
                pCurrField->type = INT_TYPE;
            else    
                pCurrField->type = FLOAT_TYPE;

            //绑定输出数据到缓冲区(整数类型绑定到整数缓冲区)
            fErrorNo = OCIDefineByPos(hStmt, &(pCurrField->hDefine), hErr, counter,
                (dvoid *)pCurrField->fDataBuf, (int)pCurrField->size + 1, SQLT_VNU,
                (dvoid *)pCurrField->fDataIndicator, (ub2 *)0 , (ub2 *) 0, OCI_DEFAULT);
            CheckError();
            break;
        case SQLT_DAT://DATE_TYPE:
            pCurrField->type = DATE_TYPE;
            pCurrField->size = 7;
            //绑定输出数据到缓冲区(date类型也是绑定到字符串缓冲区)
            pCurrField->fDataBuf = new ub1[fPrefetchRows *(pCurrField->size)];
            pCurrField->lDataBufLen=(long)(sizeof(ub1)*fPrefetchRows) * (pCurrField->size);
            fErrorNo = OCIDefineByPos(hStmt, &(pCurrField->hDefine), hErr, counter,
                pCurrField->fDataBuf, 7, SQLT_DAT,
                (dvoid *)pCurrField->fDataIndicator, (ub2 *)0, (ub2 *) 0, OCI_DEFAULT);
            CheckError();
            break;
        case SQLT_CHR: case SQLT_AFC: //DATA_TYPE_CHAR: case VARCHAR2_TYPE: 
            pCurrField->type = STRING_TYPE;
            pCurrField->size = innerDataSize;  //以系统取得的字段长度作为数据的长度大小
            //绑定输出数据到缓冲区
            pCurrField->fDataBuf = new ub1[fPrefetchRows * (pCurrField->size+1)];
            pCurrField->lDataBufLen=(long)(sizeof(ub1)*fPrefetchRows) * (pCurrField->size+1);
            fErrorNo = OCIDefineByPos(hStmt, &(pCurrField->hDefine), hErr, counter,
                pCurrField->fDataBuf, (int)pCurrField->size+1, SQLT_STR,
                (dvoid *)pCurrField->fDataIndicator, (ub2 *)0, (ub2 *)0, OCI_DEFAULT);
            CheckError(); 
            break;
        case SQLT_RDD:
            pCurrField->type = ROWID_TYPE;
            pCurrField->size = 18;
            //绑定输出数据到缓冲区
            pCurrField->fDataBuf = new ub1[fPrefetchRows * (pCurrField->size+1)];
            pCurrField->lDataBufLen=(long)(sizeof(ub1)*fPrefetchRows) * (pCurrField->size+1);
            fErrorNo = OCIDefineByPos(hStmt, &(pCurrField->hDefine), hErr, counter,
                (dvoid *)pCurrField->fDataBuf, 
                (int)pCurrField->size+1, SQLT_STR,
                (dvoid *)pCurrField->fDataIndicator, (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT);
            CheckError();
            break;
        case SQLT_BLOB:
        case SQLT_CLOB:					
            pCurrField->size = 4;
            pCurrField->type = innerDataType;
            pCurrField->iBlobCount=(int)fPrefetchRows;
            pCurrField->ahBlob=(OCILobLocator**)new char[(int)sizeof(OCILobLocator*)*pCurrField->iBlobCount];
            for(int iIndex=0;iIndex<pCurrField->iBlobCount;iIndex++)
            {
                fErrorNo = OCIDescriptorAlloc((dvoid *)db->hEnv, (dvoid **)&pCurrField->ahBlob[iIndex],
                    (ub4)OCI_DTYPE_LOB, (size_t)0, (dvoid **) 0);
                CheckError();
            }

            fErrorNo = OCIDefineByPos(hStmt, &(pCurrField->hDefine), hErr, counter,
                (dvoid *)pCurrField->ahBlob, (sb4)-1,  innerDataType,
                (dvoid *)pCurrField->fDataIndicator,  (ub2 *)0, (ub2 *) 0, OCI_DEFAULT);
            CheckError();
            break;
        default:
            PS_END(BF_TDBQuery_GetFieldsDef);
            throw TDBException(fSqlStmt, ERR_DATA_TYPE_NOT_SUPPORT, pCurrField->name,innerDataType);
            break;
        } //end of data type convertion
        
        fErrorNo = OCIDescriptorFree((dvoid *)param, OCI_DTYPE_PARAM); //454104
        CheckError(); //454104     
        
    }//end of for loop every column
    PS_END(BF_TDBQuery_GetFieldsDef);
}

void TDBQuery::SetSQL(const char *inSqlstmt,int iPreFetchRows) throw (TDBException)
{
    if (! db->fConnected)
        throw TDBException(inSqlstmt, ERR_GENERAL, "SetSQL(): can't set sqlstmt on disconnected Database");

    if (fActivated)
        throw TDBException(inSqlstmt, ERR_GENERAL, "SetSQL(): can't set sqlstmt on opened state");

    //如果有已经分配空间给sqlstatement,则在Close()中已经释放；因为只有在Close()后才可以赋予SQLstatement值
    fActivated  = false;
    fTotalRowsFetched = 0;
    fEof = false;
    fOpened = false;

    //保存sql语句
    DELETE_A(fSqlStmt);
    size_t nLen = strlen(inSqlstmt);//ZMP:477242
    fSqlStmt = new char[nLen + 1];
    if (fSqlStmt == NULL)
        throw TDBException(inSqlstmt, ERR_NOMORE_MEMORY_BE_ALLOCATED, "SetSQL()", __LINE__);
    //ZMP:467365
    strncpy(fSqlStmt, inSqlstmt, nLen);
    fSqlStmt[nLen] = '\0';

    fErrorNo = OCIStmtPrepare(hStmt, hErr, (unsigned char *)fSqlStmt, (ub4)strlen(fSqlStmt), OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT);
    fActivated = (fErrorNo == OCI_SUCCESS);
    CheckError();

    fErrorNo = OCIAttrGet(hStmt, OCI_HTYPE_STMT, &(this->fStmtType),  0, OCI_ATTR_STMT_TYPE, hErr);
    CheckError();
    GetParamsDef();
}


TDBParam *TDBQuery::ParamByName(const char *paramName) throw (TDBException)
{
    TDBParam *para = NULL;
    bool found = false;
    int i;

    if (fSqlStmt == NULL)
        throw TDBException(paramName, ERR_GENERAL, "ParamByName(): sql statement is empty.");

    for(i=0; i<fParamCount; i++)
    {
        found = TStrFunc::StrNoCaseCmp(Param(i).name,paramName)==0;
        if ( found )
            break;
    }
    if ( found ) 
        para = &paramList[i];
    else 
        throw TDBException(fSqlStmt, ERR_PARAM_NOT_EXISTS, paramName);
    return para;
}

void TDBQuery::SetParameterNULL(const char *paramName) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->dataType = SQLT_LNG;
    fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name), //you don't have to pass any value/value length if the parameter value is null, or may raise oci success with info
        -1, (ub1 *)0,(sword)0, para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);

    CheckError();
}

void TDBQuery::SetParameter(const char *paramName, double paramValue, bool isOutput ) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_FLT;
    para->dblValue = paramValue;

    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)para->name,
        -1,  (ub1 *)&(para->dblValue),(sb4)sizeof(para->dblValue), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    else
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)para->name,
        (int)strlen(para->name), (dvoid *)&(para->dblValue),(sb4)sizeof(para->dblValue), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);

    CheckError();
}

void TDBQuery::SetParameter(const char *paramName, long paramValue, bool isOutput) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_INT;
    para->longValue = paramValue;
    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        -1, (ub1 *)&(para->longValue),(sword)sizeof(para->longValue), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    else 
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        (int)strlen(para->name), (ub1 *)&(para->longValue),(sword)sizeof(para->longValue), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);

    CheckError();
}


void TDBQuery::SetParameter(const char *paramName, long long paramValue, bool isOutput) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_FLT;
    para->dblValue = (double)paramValue;

    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)para->name,
        -1,  (ub1 *)&(para->dblValue),(sb4)sizeof(para->dblValue), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    else
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)para->name,
        (int)strlen(para->name), (dvoid *)&(para->dblValue),(sb4)sizeof(para->dblValue), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);

    CheckError();
}

void TDBQuery::SetParameter(const char *paramName, int paramValue, bool isOutput ) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_INT;
    para->intValue = paramValue;
    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        -1, (ub1 *)&(para->intValue),(sword)sizeof(para->intValue), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    else 
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        (int)strlen(para->name), (ub1 *)&(para->intValue),(sword)sizeof(para->intValue), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);

    CheckError();
}
//add by fu.wenjun@20041125
void TDBQuery::SetParameter(const char *paramName, const char paramValue, bool isOutput ) throw (TDBException)
{
    char strparamvalue[2] = {0};//ZMP:477242
    strparamvalue[0] = paramValue ;
    strparamvalue[1] = '\0';
    SetParameter(paramName,strparamvalue,isOutput);
}
void TDBQuery::SetParameter(const char *paramName, const char* paramValue, bool isOutput ) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_STR;
    DELETE_A(para->stringValue);

    int nLen;

    if (isOutput)
    {
        nLen = MAX_STRING_VALUE_LENGTH-1; 
        para->stringValue = new char[nLen+1];
        para->stringValue[nLen] = '\0';
    }
    else 
    {
        if(paramValue != NULL)
        {
            nLen = (int)strlen(paramValue);
            para->stringValue = new char[nLen+1];
            strncpy((char *)para->stringValue,paramValue,(size_t)nLen);
            para->stringValue[nLen] = '\0';
        }
        else
        {
            SetParameterNULL(paramName);
            return;
        }
    }

    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        -1,  (dvoid *)(para->stringValue),(sb4)(nLen+1), 
        para->dataType, (dvoid *) &para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *)0, OCI_DEFAULT);
    else
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        (sb4)strlen(para->name), (dvoid *)(para->stringValue),(sb4)(nLen+1), 
        para->dataType, (dvoid *)&para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    CheckError();
}

void TDBQuery::SetParameter(const char *paramName,const char* paramValue,int iBufferLen,bool isOutput) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_LBI;
    DELETE_A(para->stringValue);
    
    int iLen=0;

    if((paramValue==NULL)
        ||(iBufferLen<=0)
        )
    {
        SetParameterNULL(paramName);
        return;
    }
    else
    {
        iLen=iBufferLen;
        para->stringValue=new char[iLen];
        memcpy(para->stringValue,paramValue,(size_t)iLen);
    }

    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        -1,  (dvoid *)para->stringValue,(sb4)(iLen), 
        para->dataType, (dvoid *) &para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *)0, OCI_DEFAULT);
    else
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        (sb4)strlen(para->name), (dvoid *)para->stringValue,(sb4)(iLen), 
        para->dataType, (dvoid *)&para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    CheckError();
}

void TDBQuery::SetParamArray(const char *paramName, char ** paramValue,int iStructSize,int iStrSize,int iArraySize,bool isOutput) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_STR;
    para->stringArray = paramValue;
    para->stringSize = iStrSize;


    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        -1,  (dvoid *)(para->stringArray),(sb4)(para->stringSize), 
        para->dataType, (dvoid *) &para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *)0, OCI_DEFAULT);
    else
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        (sb4)strlen(para->name), (dvoid *)(para->stringArray),(sb4)(para->stringSize), 
        para->dataType, (dvoid *)&para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    CheckError();

    fErrorNo = OCIBindArrayOfStruct ( para->hBind, hErr,(ub4) iStructSize,0,0,0);
    CheckError();
}

void TDBQuery::SetParamArray(const char *paramName, int * paramValue, int iStructSize,int iArraySize, bool isOutput ) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_INT;
    para->intArray = paramValue;
    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        -1, (ub1 *)(para->intArray),(sword)sizeof(para->intArray[0]), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    else 
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        (int)strlen(para->name),(ub1 *)(para->intArray),(sword)sizeof(para->intArray[0]), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);

    CheckError();
    fErrorNo = OCIBindArrayOfStruct ( para->hBind, hErr,(ub4) iStructSize,0,0,0);
    CheckError();
}

void TDBQuery::SetParamArray(const char *paramName, double * paramValue,int iStructSize, int iArraySize,bool isOutput)  throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_FLT;
    para->dblArray = paramValue;

    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)para->name,
        -1,  (ub1 *)(para->dblArray),(sb4)sizeof(para->dblArray[0]), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    else
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)para->name,
        (int)strlen(para->name), (dvoid *)(para->dblArray),(sb4)sizeof(para->dblArray[0]), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);

    CheckError();
    fErrorNo = OCIBindArrayOfStruct ( para->hBind, hErr,(ub4) iStructSize,0,0,0);
    CheckError();
}

void TDBQuery::SetParamArray(const char *paramName, long * paramValue, int iStructSize,int iArraySize, bool isOutput ) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_INT;
    para->longArray = paramValue;
    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        -1, (ub1 *)(para->longArray),(sword)sizeof(para->longArray[0]), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    else 
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        (int)strlen(para->name),(ub1 *)(para->longArray),(sword)sizeof(para->longArray[0]), 
        para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);

    CheckError();
    fErrorNo = OCIBindArrayOfStruct ( para->hBind, hErr,(ub4) iStructSize,0,0,0);
    CheckError();
}

void TDBQuery::SetParamArray(const char *paramName, long long * paramValue, int iStructSize,int iArraySize, bool isOutput ) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    if(sizeof(long)==sizeof(long long))
    {
        para->fIsOutput = isOutput;
        para->dataType = SQLT_INT;
        para->longArray = (long*)paramValue;

        if (isOutput)
            fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
            -1, (ub1 *)(para->longArray),(sword)sizeof(para->longArray[0]), 
            para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
        else 
            fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
            (int)strlen(para->name),(ub1 *)(para->longArray),(sword)sizeof(para->longArray[0]), 
            para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);

        CheckError();
        fErrorNo = OCIBindArrayOfStruct ( para->hBind, hErr,(ub4) iStructSize,0,0,0);
        CheckError();
    }
    else
    {
        para->llongArray=paramValue;

        int iElementCount=iArraySize/iStructSize;
        long long llElement=0;
        char *sParam=(char*)paramValue;

        para->dblArray=new double[iElementCount];
        memset(para->dblArray,0,(size_t)iElementCount*sizeof(double));
        for(int iIndex=0;iIndex<iElementCount;iIndex++)
        {
            memcpy(&llElement,sParam+iStructSize*iIndex,sizeof(long long));
            para->dblArray[iIndex]=(double)llElement;
        }
        iStructSize=sizeof(double);
        //iArraySize=iElementCount*(int)sizeof(double);//ZMP:477242

        para->fIsOutput = isOutput;
        para->dataType = SQLT_FLT;
        

        if (isOutput)
            fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
            -1, (ub1 *)(para->dblArray),(sword)sizeof(para->dblArray[0]), 
            para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
        else 
            fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
            (int)strlen(para->name),(ub1 *)(para->dblArray),(sword)sizeof(para->dblArray[0]), 
            para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    
        CheckError();
        fErrorNo = OCIBindArrayOfStruct ( para->hBind, hErr,(ub4) iStructSize,0,0,0);
        CheckError();
    }
   
}

void TDBQuery::SetBlobParamArray(const char *paramName,char *paramValue,int iBufferLen,int iArraySize,bool isOutput) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->fIsOutput = isOutput;
    para->dataType = SQLT_LBI;
    para->stringArray =(char**) paramValue;
    para->stringSize = iBufferLen;


    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        -1,  (dvoid *)(para->stringArray),(sb4)(para->stringSize), 
        para->dataType, (dvoid *) &para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *)0, OCI_DEFAULT);
    else
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        (sb4)strlen(para->name), (dvoid *)(para->stringArray),(sb4)(para->stringSize), 
        para->dataType, (dvoid *)&para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    CheckError();

    fErrorNo = OCIBindArrayOfStruct ( para->hBind, hErr,(ub4) iBufferLen,0,0,0);
    CheckError();
}

void TDBQuery::CheckError(const char* sSql) throw (TDBException)
{
    if (fErrorNo != OCI_SUCCESS) 
        throw TDBException(fErrorNo, hErr, "Oracle OCI Call", fSqlStmt);
}

bool TDBQuery::Execute(int iters) throw (TDBException)
{
    PS_BEGIN(BF_TDBQuery_Execute,this);
    sb4 errcode;
    text errbuf[MAX_ERRMSG_LENGTH-1]={0}; //356853 for cppcheck
    bool exeResult = false;
#ifdef __DEBUG__
    bExecuteFlag = true;
#endif

    if (fSqlStmt == NULL)
    {
        PS_END(BF_TDBQuery_Execute);
        throw TDBException("", ERR_GENERAL, "Execute(): sql statement is not presented");
    }

    if  (this->fStmtType == OCI_STMT_SELECT)
    {
        PS_END(BF_TDBQuery_Execute);
        throw TDBException( fSqlStmt, ERR_GENERAL, "Execute(): Can't Execute a select statement.");
    }
    if(gpBillLog)
    {
        IS_DEBUG(4) ADD_NORMAL("TDBQuery::Execute(Iters=%d)\n%s\n",iters,fSqlStmt);
    }

    fErrorNo = OCIStmtExecute(db->hDBSvc, hStmt, hErr, (ub4)iters, (ub4)0, 0, 0, OCI_DEFAULT);
    
    OCIAttrGet((dvoid*)hStmt, OCI_HTYPE_STMT, (dvoid *)&fTotalRowsFetched, (ub4 *)0, OCI_ATTR_ROW_COUNT, hErr);
    nTransTimes ++;

    if (fErrorNo == OCI_SUCCESS)
        exeResult = true;
    else if ( fErrorNo == OCI_ERROR ) //以下允许返回空参数(1405)
    {
        OCIErrorGet (hErr, (ub4) 1, (text *) NULL, &errcode,
            errbuf, (ub4) sizeof(errbuf), (ub4) OCI_HTYPE_ERROR);
        if (errcode == 1405) 
            exeResult = true;
        else 
            CheckError();
    }
    else 
    {
        CheckError();
    }

    fLastRecordFlag = false;
    PS_END(BF_TDBQuery_Execute);
    return exeResult;
}


void TDBQuery::GetParamsDef() throw (TDBException)
{
    char *params[MAX_PARAMS_COUNT] = {0};//ZMP:477242
    int i, in_literal, n, nParamLen,nFlag = 0;
    char *cp=NULL,*ph=NULL; //356853 for cppcheck
    char *sql=NULL; //356853 for cppcheck

    size_t nLen = strlen(this->fSqlStmt);
    sql = new char[nLen+1];
    //ZMP:467365
    strncpy(sql, this->fSqlStmt, nLen);
    sql[nLen] = '\0';

    if (fParamCount>0)
    {
        DELETE_A(paramList);
    }

    // Find and bind input variables for placeholders. 
    for (i = 0, in_literal = false, cp = sql; *cp != 0; cp++)
    {

        if (*cp == '\'')
            in_literal = ~in_literal;
        if (*cp == ':' && *(cp+1) != '=' && !in_literal)
        {

            for ( ph = ++cp, n = 0;  *cp && (isalnum(*cp) || *cp == '_'); cp++, n++);
            if(*cp == 0) 
                nFlag = 1;
            else 
                *cp = 0;

            if ( i > MAX_PARAMS_COUNT)
                throw TDBException(fSqlStmt, ERR_CAPABILITY_NOT_YET_SUPPORT, " param count execedes max numbers, please refer to OCIQuery.h");
            nParamLen = (int)strlen((char *)ph);//ZMP:477242

            params[i] = new char[nParamLen+1];
            //ZMP:467365
            strncpy(params[i], (char *)ph, (size_t)nParamLen);
            params[i][nParamLen] = '\0';
            i++;
            if(nFlag == 1) break;
        }   
    }
    DELETE_A(sql);

    fParamCount = i;
    if (fParamCount>0)
    {
        paramList = new TDBParam[fParamCount];

        for (i=0; i<fParamCount; i++)
        {
            nParamLen = (int)strlen(params[i]);//ZMP:477242
            paramList[i].name = new char[nParamLen+1];
            strncpy(paramList[i].name, params[i], (size_t)nParamLen);
            paramList[i].name[nParamLen] = '\0';
            DELETE_A(params[i]);
        }
    }
}
void TDBQuery::Open(int prefetch_Row) throw (TDBException)
{
    PS_BEGIN(BF_TDBQuery_Open,this);
    fPrefetchRows = (unsigned int)prefetch_Row;
    if (fOpened)
    {
        fCurrRow = 0;
        fFetched = 0;
        fCurrRow = 0;
        fTotalRowsFetched = 0;
    }
    if (fSqlStmt == NULL)
    {
        PS_END(BF_TDBQuery_Open);
        throw TDBException("", ERR_GENERAL, "Open(): sql statement is empty.");
    }

    if ( this->fStmtType !=OCI_STMT_SELECT)
    {
        PS_END(BF_TDBQuery_Open);
        throw TDBException( fSqlStmt, ERR_GENERAL, "Can't open none-select statement");
    }

    if(gpBillLog)
    {
        IS_DEBUG(4) ADD_NORMAL("TDBQuery::Open(PerfRow=%d)\n%s\n",prefetch_Row,fSqlStmt);
    }

    GetFieldsDef();
    fBof = true;
    fLastRecordFlag = false;
    fOpened = true;
    PS_END(BF_TDBQuery_Open);
}

int TDBQuery::FieldCount()
{
    return fFieldCount;
}

int TDBQuery::ParamCount()
{
    return fParamCount;
}
bool TDBQuery::IsParamExist(const char *paramName)
{
    for(int i=0; i<fParamCount; i++)
    {
        if (TStrFunc::StrNoCaseCmp(Param(i).name,paramName)==0)  
        {
            return true;
        }
    }
    return false;
}
TDBField& TDBQuery::Field(int i) throw (TDBException)
{
    if (fSqlStmt == NULL)
        throw TDBException("", ERR_GENERAL, "Field(i): sql statement is not presented");

    if ( (i>=0) && (i<fFieldCount) ) 
        return fieldList[i];
    else throw TDBException(fSqlStmt , ERR_INDEX_OUT_OF_BOUND, "field index out of bound when call Field(i)");
}

TDBField& TDBQuery::Field(const char *fieldName) throw (TDBException)
{
    int i;
    bool found = false;

    if (fSqlStmt == NULL)
        throw TDBException("", ERR_GENERAL, "Field(*fieldName): sql statement is not presented");

    if (! fOpened)
        throw TDBException(fSqlStmt, ERR_GENERAL, "can not access field before open");

    for(i=0; i<fFieldCount; i++)
    {
        found = TStrFunc::StrNoCaseCmp(Field(i).name,fieldName)==0;
        if ( found )
            break;
    }
    if ( found ) 
        return fieldList[i];
    else 
        throw TDBException(fSqlStmt, ERR_FIELD_NOT_EXISTS, fieldName);
}
/*begin zmp 841469 */
void TDBQuery::GetValue(void *pStruct,int* Column)throw (TDBException)
{
    throw TDBException("TDBException::GetValue","Not Support!");
}
void TDBQuery::SetUpdateMDBFlag(bool flag)throw (TDBException)
{
    throw TDBException("TDBException::SetUpdateMDBFlag","Not Support!");
}
bool TDBQuery::GetUpdateMDBFlag()throw (TDBException)
{
    throw TDBException("TDBException::GetUpdateMDBFlag","Not Support!");
}
/*end zmp 841469 */


bool TDBQuery::IsFieldExist(const char *fieldName)
{
    bool found = false;
    for(int i=0; i<fFieldCount; i++)
    {
        found = TStrFunc::StrNoCaseCmp(Field(i).name,fieldName)==0;
        if ( found )
            return true;
    }
    return false ;
}

TDBParam& TDBQuery::Param(int index) throw (TDBException)
{
    if (fSqlStmt == NULL)
        throw TDBException("", ERR_GENERAL, "Param(index): sql statement is not presented");

#ifdef debug
    printf("param i constructor\n");
#endif

    if ( (index>=0) && (index<fParamCount) ) 
        return paramList[index];
    else
        throw TDBException(fSqlStmt , ERR_INDEX_OUT_OF_BOUND, "param index out of bound when call Param(i)");
}

TDBParam& TDBQuery::Param(const char *inName) throw (TDBException)
{
    int i;
    bool found = false;

    if (fSqlStmt == NULL)
        throw TDBException("", ERR_GENERAL, "Param(paramName): sql statement is not presented");

    for(i=0; i<fParamCount; i++)
    {
        found = TStrFunc::StrNoCaseCmp(paramList[i].name,inName)==0;
        if (found)
            break;
    }
    if ( found ) 
        return paramList[i];
    else
        throw TDBException(fSqlStmt, ERR_PARAM_NOT_EXISTS, (const char*)inName);
}

bool TDBQuery::Next() throw (TDBException)
{
    PS_BEGIN(BF_TDBQuery_Next,this);
    int fCanFetch = 1;                  //当前记录指针的位置是否可以存取数据
    int tmpFetchedAllRows;

    sb4 errcode;
    text errbuf[MAX_ERRMSG_LENGTH]={0}; //356853 for cppcheck
    bool exeResult = true;

    if (fSqlStmt == NULL)
    {
        PS_END(BF_TDBQuery_Next);
        throw TDBException("", ERR_GENERAL, "Next(): sql statement is not presented");
    }
    if (!fOpened)
    {
        PS_END(BF_TDBQuery_Next);
        throw TDBException(fSqlStmt, ERR_GENERAL, "Next(): can not access data before open it");
    }
    fCurrRow ++ ;
    bool bFlag1 = ( (fCurrRow == fFetched) && (fFetched < fPrefetchRows) ); //315428 
    bool bFlag2 = ( fCurrRow==fFetched || ! fFetched ); //315428 
    if( bFlag1 ) 
    {
        fCanFetch=0;
    }
    else if( bFlag2 )
    {
        //清空FiledList的缓冲区
        if(fieldList!=NULL)
        {
            for(int iIndex=0;iIndex<fFieldCount;iIndex++)
            {
                fieldList[iIndex].ClearDataBuf();
            }
        }

        fErrorNo = OCIStmtFetch(hStmt, hErr, (ub4)fPrefetchRows, (ub4) OCI_FETCH_NEXT, (ub4) OCI_DEFAULT);  
        if ( fErrorNo == OCI_ERROR ) //以下允许返回空列(1405),修正：不可以返回被截断的列(1406)
        {
            OCIErrorGet (hErr, (ub4) 1, (text *) NULL, &errcode,
                errbuf, (ub4) sizeof(errbuf), (ub4) OCI_HTYPE_ERROR);
            if (errcode == 1405)
                exeResult = true;
            else CheckError();
        }

        tmpFetchedAllRows = (int)fTotalRowsFetched;
        fErrorNo = OCIAttrGet((dvoid*)hStmt, OCI_HTYPE_STMT, (dvoid *)&fTotalRowsFetched, (ub4 *)0, OCI_ATTR_ROW_COUNT, hErr);
        fFetched = fTotalRowsFetched - (unsigned int)tmpFetchedAllRows;
        if(fFetched) 
        {
            fCanFetch=1;
            fCurrRow=0;
        }
        else fCanFetch=0;

        if (fErrorNo == OCI_SUCCESS)
            exeResult = true;
        else if (fErrorNo == OCI_NO_DATA)
            exeResult = false;
        else if ( fErrorNo == OCI_ERROR ) //以下允许返回空列(1405),修正：不可以返回被截断的列(1406)
        {
            OCIErrorGet (hErr, (ub4) 1, (text *) NULL, &errcode,
                errbuf, (ub4) sizeof(errbuf), (ub4) OCI_HTYPE_ERROR);
            if (errcode == 1405)
                exeResult = true;
            else CheckError();
        }
        else    CheckError();
    }

    fBof = false;
    fEof = (fFetched && !fCanFetch);
    fLastRecordFlag = !(exeResult && fCanFetch);
    PS_END(BF_TDBQuery_Next);
    return (exeResult && fCanFetch);
}
 //ZMP_559430
long long TDBQuery::GetSequenceByName(const char * sName)throw (TDBExcpInterface)
{
    throw TDBException("TDBQuery::GetSequenceByName"," not support now.");
}
/****************** parameter implementation **************************/
TDBParam::TDBParam()
{
    fIsOutput = false;
    stringValue = NULL;
    indicator = 0;
    hBind = (OCIBind *) 0; 
    dblArray=NULL;
    llongArray=NULL;
    //ZMP:305593
    name = NULL;
    intValue = 0;
    dblValue = 0;
    longValue = 0;
    intArray = NULL;
    longArray = NULL;
    stringArray = NULL;
    stringSize = 0;
}

TDBParam::~TDBParam()
{
    DELETE_A(name);
    DELETE_A(stringValue);

    if((llongArray!=NULL)&&(dblArray!=NULL))
    {
        DELETE_A(dblArray);
        llongArray=NULL;
    }
}

int TDBParam::AsInteger() throw (TDBException)
{
    if ( isNULL() )
        intValue = 0;

    if (dataType == SQLT_INT)
        return intValue;
    else    
        throw TDBException("TDBParam", ERR_READ_PARAM_DATA, name, "AsInteger()");   
}

long TDBParam::AsLong() throw (TDBException)
{
    if ( isNULL() )
        longValue = 0;

    if (dataType == SQLT_LNG)
        return longValue;
    else    
        throw TDBException("TDBParam", ERR_READ_PARAM_DATA, name, "AsLong()");  
}

double TDBParam::AsFloat() throw (TDBException)
{
    if ( isNULL() )
        dblValue = 0;

    if (dataType == SQLT_FLT)
        return dblValue;
    else    
        throw TDBException("TDBParam", ERR_READ_PARAM_DATA, name, "AsFloat()");
}

char* TDBParam::AsString() throw (TDBException)
{
    if ( isNULL() )
        stringValue[0] = '\0';

    if (dataType == SQLT_STR)
        return stringValue;
    else    
        throw TDBException("TDBParam", ERR_READ_PARAM_DATA, name, "AsString()");
}

bool TDBParam::isNULL() throw (TDBException)
{
    if (! fIsOutput)
        throw TDBException("TDBParam, not an output parameter", ERR_READ_PARAM_DATA, name, "isNULL()");
    return (indicator == -1);
}

/*********************************************************************************
*TDBSession implementation
*********************************************************************************/
TDBSession::TDBSession(TDBDatabase *pDB)
{
    if(!pDB->fConnected)
        throw TDBException("", ERR_GENERAL, "TDBSession(pDB): Can not create a TDBSession when the database is not connected");

    m_hSession = NULL;
    m_hSrvCtx = NULL;
    m_bActive = FALSE;

    OCIHandleAlloc((dvoid *)pDB->hEnv,(dvoid **)&m_hSession,(ub4)OCI_HTYPE_SESSION,(size_t)0,(dvoid **) 0);

    OCIHandleAlloc((dvoid *)pDB->hEnv,(dvoid **)&m_hSrvCtx,(ub4)OCI_HTYPE_SVCCTX,(size_t)0,(dvoid **) 0);

    OCIHandleAlloc((dvoid *)pDB->hEnv,(dvoid **)&m_hError,(ub4)OCI_HTYPE_ERROR,(size_t)0,(dvoid **)0);

    m_iErrorNo = OCIAttrSet(m_hSrvCtx,OCI_HTYPE_SVCCTX,pDB->hSvr,0,OCI_ATTR_SERVER,m_hError);
    CheckError();

    //set the username/password in session handle
    m_iErrorNo = OCIAttrSet(m_hSession,OCI_HTYPE_SESSION,pDB->usr,(ub4)strlen(pDB->usr),OCI_ATTR_USERNAME,m_hError);
    m_iErrorNo = OCIAttrSet(m_hSession,OCI_HTYPE_SESSION,pDB->pwd,(ub4)strlen(pDB->pwd),OCI_ATTR_PASSWORD,m_hError);

    //set the Authentication handle in the server context handle
    m_iErrorNo = OCIAttrSet(m_hSrvCtx,OCI_HTYPE_SVCCTX,m_hSession,0,OCI_ATTR_SESSION,m_hError);
    CheckError();

};

TDBSession::~TDBSession()
{
    if(m_bActive)
        sessionEnd();

    OCIHandleFree((dvoid *)m_hSession,(ub4)OCI_HTYPE_SESSION);
    OCIHandleFree((dvoid *)m_hSrvCtx,(ub4)OCI_HTYPE_SVCCTX);

}

void TDBSession::sessionBegin()
{
    if(m_bActive)
        return;

    //begin a session
    m_iErrorNo = OCISessionBegin(m_hSrvCtx,m_hError,m_hSession,OCI_CRED_RDBMS,(ub4)OCI_DEFAULT);
    CheckError();

    m_bActive = TRUE;                            
}

void TDBSession::sessionEnd()
{
    if(!m_bActive)
        return;
    //end a session
    m_iErrorNo = OCISessionEnd(m_hSrvCtx,m_hError,m_hSession,OCI_DEFAULT);
    CheckError();   
}
static void errprint(dvoid *errhp, ub4 htype, sb4 *errcodep)
{
    text errbuf[512]={0}; //356853 for cppcheck

    if (errhp)
    {
        sb4  errcode;

        if (errcodep == (sb4 *)0)
            errcodep = &errcode;

        (void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, errcodep,
            errbuf, (ub4) sizeof(errbuf), htype);
        (void) printf("Error - %.*s\n", 512, errbuf);
    }
}

void TDBSession::CheckError()
{

    switch (m_iErrorNo)
    {
    case OCI_SUCCESS:
        break;
    case OCI_SUCCESS_WITH_INFO:
        (void) printf( "Error - OCI_SUCCESS_WITH_INFO\n");
        errprint(m_hError, OCI_HTYPE_ERROR, &m_iErrorNo);
        break;
    case OCI_NEED_DATA:
        (void) printf( "Error - OCI_NEED_DATA\n");
        break;
    case OCI_NO_DATA:
        (void) printf( "Error - OCI_NODATA\n");
        break;
    case OCI_ERROR:
        errprint(m_hError, OCI_HTYPE_ERROR, &m_iErrorNo);
        break;
    case OCI_INVALID_HANDLE:
        (void) printf( "Error - OCI_INVALID_HANDLE\n");
        break;
    case OCI_STILL_EXECUTING:
        (void) printf( "Error - OCI_STILL_EXECUTE\n");
        break;
    case OCI_CONTINUE:
        (void) printf( "Error - OCI_CONTINUE\n");
        break;
    default:
        break;
    }  
}

////////////////////////////////////////////////////////////////////////////////
//138555 begin
void TDBQuery::SetParameterTime_t(const char *paramName, time_t tparamValue, bool isOutput ) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常
    char paramValue[40] = {0};//ZMP:477242
    //memset(paramValue,0x00,sizeof(paramValue));
    struct tm * tm_Cur;
    //tm_Cur = localtime(&tparamValue);
    struct tm tmLocalTime;
    tm_Cur=localtime_r(&tparamValue,&tmLocalTime);
    //ZMP:467365
    snprintf(paramValue, sizeof(paramValue), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
    paramValue[sizeof(paramValue)-1] = '\0';
    para->fIsOutput = isOutput;
    para->dataType = SQLT_STR;
    DELETE_A(para->stringValue);

    int nLen;

    if (isOutput)
    {
        nLen = MAX_STRING_VALUE_LENGTH-1; 
        para->stringValue = new char[nLen+1];
        para->stringValue[nLen] = '\0';
    }
    else 
    {
        if(paramValue != NULL)
        {
            nLen = (int)strlen(paramValue);
            para->stringValue = new char[nLen+1];
            strncpy((char *)para->stringValue,paramValue,(size_t)nLen);
            para->stringValue[nLen] = '\0';
        }
        else
        {
            SetParameterNULL(paramName);
            return;
        }
    }

    if (isOutput)
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        -1,  (dvoid *)(para->stringValue),(sb4)(nLen+1), 
        para->dataType, (dvoid *) &para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *)0, OCI_DEFAULT);
    else
        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        (sb4)strlen(para->name), (dvoid *)(para->stringValue),(sb4)(nLen+1), 
        para->dataType, (dvoid *)&para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    CheckError();
}
//138555 end
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//141877 begin
TDBParam *TDBQuery::ParamByName(int paramName) throw (TDBException)
{
    TDBParam *para = NULL;

    if (fSqlStmt == NULL)
        throw TDBException("TDBQuery::ParamByName", ERR_GENERAL, "ParamByName(): sql statement is empty.");

    if( paramName < 0 
     || paramName >= fParamCount
      )
        throw TDBException(fSqlStmt, "param_by_Index:%d does not exists.", paramName);

    para = &paramList[paramName];
    return para;
}

void TDBQuery::SetParameterNULL(int paramName) throw (TDBException)
{
    TDBParam *para = ParamByName(paramName); //在ParamByName中已经有判断参数不存在抛出异常

    para->dataType = SQLT_LNG;
    fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name), //you don't have to pass any value/value length if the parameter value is null, or may raise oci success with info
        -1, (ub1 *)0,(sword)0, para->dataType, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);

    CheckError();
}

void TDBQuery::SetParameterTime_t(int iParamIndex,time_t tParamValue) throw (TDBException)
{
    TDBParam *para = ParamByName(iParamIndex); //在ParamByName中已经有判断参数不存在抛出异常
    char paramValue[40] = {0};//ZMP:477242
    //memset(paramValue,0x00,sizeof(paramValue));
    struct tm * tm_Cur;
    struct tm tmLocalTime;
    //tm_Cur = localtime(&tParamValue);
    tm_Cur=localtime_r(&tParamValue,&tmLocalTime);
    //ZMP:467365
    snprintf(paramValue, sizeof(paramValue), "%04d%02d%02d%02d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);
    paramValue[sizeof(paramValue)-1] = '\0';
    para->fIsOutput = false;
    para->dataType = SQLT_STR;
    DELETE_A(para->stringValue);

    int nLen;

        if(paramValue != NULL)
        {
            nLen = (int)strlen(paramValue);
            para->stringValue = new char[nLen+1];
            strncpy((char *)para->stringValue,paramValue,(size_t)nLen);
            para->stringValue[nLen] = '\0';
        }
        else
        {
            SetParameterNULL(iParamIndex);
            return;
        }

        fErrorNo = OCIBindByName(hStmt, &para->hBind, hErr, (text *)(para->name),
        (sb4)strlen(para->name), (dvoid *)(para->stringValue),(sb4)(nLen+1), 
        para->dataType, (dvoid *)&para->indicator, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
    CheckError();
}
//141877 end
////////////////////////////////////////////////////////////////////////////////

int TDBField::AsDateTimeInternalNew(int &year, int &month, int &day, int &hour, int &minute, int &second) throw (TDBException)
{
    char *sData = NULL;
    sData = (char *)(fDataBuf + (size+1) * fParentQuery->fCurrRow);
    size_t iLen=strlen(sData);//ZMP:477242
    
    if((iLen!=14)&&(iLen!=8)&&(iLen != 0))
        throw TDBException(fParentQuery->fSqlStmt, ERR_GENERAL, "FieldLength is error!");

    if(iLen == 0)
    {
        return 0;
    }

    char sTempYear[5]={0},sTempMon[3]={0},sTempMDay[3]={0},sTempHour[3]={0},sTempMin[3]={0},sTempSec[3]={0}; //356853 for cppcheck
    strncpy(sTempYear,sData,4);
    //sTempYear[4]=0; //356853,初始化时已做,此处没必要了.
    sTempMon[0]=sData[4];
    sTempMon[1]=sData[5];
    //sTempMon[2]=0; //356853,初始化时已做,此处没必要了.
    sTempMDay[0]=sData[6]; 
    sTempMDay[1]=sData[7]; 
    //sTempMDay[2]=0; //356853,初始化时已做,此处没必要了.

    // 支持8位
    if(8 == iLen)
    {
        sTempHour[0] = '0';
        sTempHour[1] = '0';
        //sTempHour[2]=0; //356853,初始化时已做,此处没必要了.

        sTempMin[0]= '0';
        sTempMin[1]= '0';
        //sTempMin[2]=0; //356853,初始化时已做,此处没必要了.

        sTempSec[0]= '0';
        sTempSec[1]= '0';
        //sTempSec[2]=0; //356853,初始化时已做,此处没必要了.
    }
    else  // 14位
    {
        sTempHour[0]=sData[8];
        sTempHour[1]=sData[9]; 
        //sTempHour[2]=0; //356853,初始化时已做,此处没必要了.

        sTempMin[0]=sData[10];
        sTempMin[1]=sData[11]; 
        //sTempMin[2]=0; //356853,初始化时已做,此处没必要了.

        sTempSec[0]=sData[12];
        sTempSec[1]=sData[13]; 
        //sTempSec[2]=0; //356853,初始化时已做,此处没必要了.
    }

    year=atoi(sTempYear);
    month=atoi(sTempMon);
    day=atoi(sTempMDay);
    hour=atoi(sTempHour);
    minute=atoi(sTempMin);
    second=atoi(sTempSec);
    return 1;
}
