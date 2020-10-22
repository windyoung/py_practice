//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#include <string>
#include <iostream>
#include <sys/stat.h>

#include "TException.h"
#include "ReadIni.h"
#include "TStrFunc.h"
#include "debug_new.h"
#include "Aes.h"
#include "Aes_Encryptor.h"


//ZMP:467373     
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

char * TReadIni::Trim( char *sp )
{
    char sDest[LINEMAXCHARS]={0}; //357131 for cppcheck
    char *pStr=NULL; //357131 for cppcheck
    int i = 0;//ZMP:477242  ZMP:1275455 ��ֹiΪ-1ת��Ϊunsigned intΪ����ֵԽ��

    if ( sp == NULL )
        return NULL;

    pStr = sp;
    while ( *pStr == ' ' || *pStr == '\t' )  pStr ++;
    //ZMP:467373 
    strncpy( sDest, pStr ,sizeof(sDest)-1);
    sDest[sizeof(sDest)-1] = '\0';

    i = (int)strlen( sDest ) - 1;
    while((i>=0)&&(sDest[i] == ' ' || sDest[i] == '\t') )
    {
        sDest[i] = '\0';
        i-- ;
    }
    //ZMP:467373 
    strncpy( sp, sDest ,(size_t)(i+1));
    sp[i+1] = '\0';
    
    return ( sp );
}

char * TReadIni::TruncStr( char *sp , char *sDst , int len)
{
    memset(sDst , 0 ,  (size_t)(len+1));
    strncpy(sDst ,     sp , (size_t)len) ;
    return sDst;
}

//351951 for cppcheck
/*
char * TReadIni::SubStr( char *sp , char *sDst , int len)
{
    memset(sDst , 0 , 32);
    strncpy(sDst ,     sp , len) ;
    return sDst;
}
*/

/************************************************
Check if the Section exists in the File 
Exists: return 1,
else: return 0;
************************************************/
bool TReadIni::SectionCheck(const char *Section)
{
    char sSect[LINEMAXCHARS]={0}; //357131 for cppcheck
    char sBuffer[LINEMAXCHARS]={0}; //357131 for cppcheck
    
    //453229 begin
    int iRet = fseek(fd,0,SEEK_SET ); 
    if( iRet != 0 )    
    {    
        fclose(fd);        
        fd = NULL;        
        return false;    
    }   
    //453229 end
    //ZMP:467373 
    snprintf(sSect, sizeof(sSect), "[%s]", Section);
    sSect[sizeof(sSect)-1] = '\0';
    while ( fgets(sBuffer,LINEMAXCHARS,fd))
    {
        Trim( sBuffer );

        /* note or empty line */
        if ( ( sBuffer[0] == '#' ) || ( sBuffer[0] == '\n' ) )
            continue;

        /* into the section */
        if(strstr(sBuffer,sSect))
            return true;
    }

    return false;
}

/* Read from system ini file parameter */
char * TReadIni::ReadString(const char *Section, const char *Ident, char *sItem, const char *defstr )
{
    char sBuffer[LINEMAXCHARS]={0}, sSect[LINEMAXCHARS]={0}, *p=NULL ; //357131 for cppcheck
    int  iLine = 0, iFindCount = 0, arrLine[10];
    memset(arrLine,0,sizeof(arrLine));
    bool bFindSection=false;
    char *pTmp=NULL; //357131 for cppcheck
    char sTmp1[LINEMAXCHARS]={0}; //357131 for cppcheck
    #ifdef _ASSERT_PARAM
    if (NULL==sItem) 
    {
        throw TException("TReadIni::ReadString(): Parameter sItem can't be NULL");
    }
    #endif
    if ( defstr == NULL )
    {
        //ZMP:467373 
        strncpy( sItem, "" ,0);
        sItem[0] = '\0';
    }
    else
    {
        strncpy( sItem, defstr ,strlen(defstr));
        sItem[strlen(defstr)] = '\0';
    }
    if(!fd)
        return sItem;
    //ZMP:467373 
    strncpy( sSect, "[" ,sizeof(sSect)-1);
    strcat( sSect, Section );
    strcat( sSect, "]" );

    int iRet = fseek(fd,0,SEEK_SET );
    //357131 for cppcheck,���ӶԷ���ֵ���ж�
    if( iRet != 0 ) 
    {
        fclose(fd);
        fd = NULL;
        return sItem;
    }
    
    char sValue[LINEMAXCHARS] = {0}; //�����ȡ�����������ֵ
    strncpy( sValue, sItem , sizeof(sValue)-1);
    sValue[sizeof(sValue) - 1] = '\0';
        
    while ( fgets( sBuffer, LINEMAXCHARS, fd ) )    
    {
        iLine++;
        Trim( sBuffer );
        pTmp = (char *) sBuffer;
        /* note or empty line */
        bool bSkipFlag =( ( sBuffer[0] == '#' )||( sBuffer[0] == '\n' ) ) ; //���͸��Ӷ�
        if ( bSkipFlag ) //���͸��Ӷ� 
            continue;
        //206289 begin
        char* pNewLine = NULL;
        //ZMP:305593
        pNewLine = strchr( sBuffer, '#' );
        if( pNewLine != NULL )
        {
            *pNewLine = '\0';
        }
        //206289 end
        /* into back section of the section */
        if ( bFindSection && ( strchr( sBuffer, '[' ) ) && ( strchr( sBuffer, ']' ) ) )
            break;
        /* into the section */
        if ( !bFindSection && strstr( sBuffer, sSect ) ){
            bFindSection = true;
            continue;
        }
        /* front of the section */
        if ( !bFindSection )
            continue;
    //��ȡ��������������ж�� �����һ��Ϊ׼
        if ( ( p = (char *) strchr( sBuffer, '=' ))  &&  
             (strcmp(Trim(TruncStr(sBuffer,sTmp1,(int)(p-pTmp))),Ident)==0) ) //��sBuffer���ҵ�����Ident,����nAct=2.
        {
            arrLine[iFindCount] = iLine; 
            iFindCount++;
            FUNC_STR::FormatChar(p+1); //20628
            char *pItem = Trim( p + 1 );
            memset(sValue,0,sizeof(sValue));
            strncpy( sValue, pItem, strlen(pItem));//Trim���'='����Ŀո���˸����'\0'֮ǰ�Ŀո���˸�,�����'\0'ǰ��.
            sValue[strlen(pItem)] = '\0';        
            bFindSection = false;
        }
    }
  
    if ( iFindCount > 1 )
    {
        printf("\n\n\nTReadIni::ReadString() Error. There are more than one same section in config file. Please check config at : ");
        for(int i=0;i<iFindCount;i++)
        {
            printf("Line[%d] ",arrLine[i]);
        }
        printf("\n\n\n");
    }
  
    //���δ��ȡ���������Ĭ��ֵΪNULL,����NULL
    if ( iFindCount == 0 && defstr == NULL)
    {
        return NULL;
    }
    //��ֹShadow��ShadowPassword�������ڳ����������޸�(Y->N)�������������ֶν��н��ܲ���
    if(0 == TStrFunc::StrCmpNoCase(Ident, "password") || 0 == TStrFunc::StrCmpNoCase(Ident, "pass_word"))
    {
        char sTemp[256] = {0};
        if(IsEncryptStr(sValue))
        {
            DecryptEx(sValue,sTemp);
        }
        if(IsEncryptStr_Aes(sValue))
        {
            DecryptEx_Aes(sValue, sTemp);
        }
        if(sTemp[0]!='\0')
        {
            memset(sValue, 0, sizeof(sValue));
            memccpy(sValue, sTemp,0,strlen(sTemp));
            sValue[strlen(sTemp)]='\0';	
        }
    }
   
    if(m_iSecretPassword == 1)
    {
        if(m_bUseNewDecode && (0 == TStrFunc::StrCmpNoCase(Ident, "username")|| 0 == TStrFunc::StrCmpNoCase(Ident, "user_name")))
        {
            char sTemp[21] = {0};
            //strncpy(sTemp, Decrypt(sItem),sizeof(sTemp)-1);
            char* pUserName = Decrypt(sValue);
            if(pUserName != NULL)
            {
                strncpy(sTemp,pUserName,sizeof(sTemp)-1);;
            }
            snprintf(sValue,20,"%s", sTemp);
        }
    }
    
    strncpy(sItem, sValue, strlen(sValue));
    sItem[strlen(sValue)] = '\0';

    return sItem; //FUNC_STR::FormatChar(sItem) ;
}


bool TReadIni::ReadStringPlus(const char* Section, const char* Ident, char* sItem, const char* defstr,
                              size_t uItemMaxSize)
{
#ifdef _ASSERT_PARAM
  if (NULL==sItem || uItemMaxSize == 0) 
  {
        throw TException("TReadIni::ReadString(): Parameter sItem can't be NULL");
    }
#endif

    if (defstr == NULL)
    {
        sItem[0] = '\0';
    }
    else
    {
        strncpy(sItem, defstr, uItemMaxSize);
        sItem[uItemMaxSize - 1] = '\0';
    }

    if (NULL == fd)
    {
        return false;
    }

    if (0 != fseek(fd, 0, SEEK_SET))
    {
        fclose(fd);
        fd = NULL;
        return false;
    }

    char sBuffer[LINEMAXCHARS] = {0}; //�л���
    char sSect[LINEMAXCHARS] = {0}; //����[Section]
    char sValue[LINEMAXCHARS] = {0}; //�����ȡ�����������ֵ
    char sTmp1[LINEMAXCHARS] = {0};
    char* p = NULL;
    char* pTmp = NULL;
    bool bCutOff = false; //�Ƿ����ض�
    int iLine = 0, iFindCount = 0, arrLine[10];
    memset(arrLine, 0, sizeof(arrLine));
    bool bFindSection = false;


    //ƴ��[Section]
    strcat(sSect, "[");
    strcat(sSect, Section);
    strcat(sSect, "]");
    while (NULL != fgets(sBuffer, sizeof(sBuffer), fd))
    {
        iLine++;
        Trim(sBuffer);
        pTmp = (char *)sBuffer;
        /* note or empty line */
        bool bSkipFlag = ((sBuffer[0] == '#') || (sBuffer[0] == '\n'));
        if (bSkipFlag)
            continue;

        char* pNewLine = NULL;
        pNewLine = strchr(sBuffer, '#'); //ȥ��'#'֮���ע��
        if (pNewLine != NULL)
        {
            *pNewLine = '\0';
        }

        /* into back section of the section */
        if (bFindSection && (strchr(sBuffer, '[')) && (strchr(sBuffer, ']')))
            break;

        /* into the section */
        if (!bFindSection && strstr(sBuffer, sSect))
        {
            bFindSection = true;
            continue;
        }

        /* front of the section */
        if (!bFindSection)
            continue;

        //��ȡ��������������ж�� �����һ��Ϊ׼
        if ((p = (char *)strchr(sBuffer, '=')) &&
            (strcmp(Trim(TruncStr(sBuffer, sTmp1, (int)(p - pTmp))), Ident) == 0))
        {
            arrLine[iFindCount] = iLine;
            iFindCount++;
            FUNC_STR::FormatChar(p + 1);
            char* pItem = Trim(p + 1);
            memset(sValue, 0, sizeof(sValue));
            strncpy(sValue, pItem, strlen(pItem)); //Trim���'='����Ŀո���˸����'\0'֮ǰ�Ŀո���˸�,�����'\0'ǰ��.
            sValue[strlen(pItem)] = '\0';
            bFindSection = false;
        }
    }

    if (iFindCount > 1)
    {
        printf(
            "\n\n\nTReadIni::ReadStringPlus() Error. There are more than one same section in config file. Please check config at : ");
        for (int i = 0; i < iFindCount; i++)
        {
            printf("Line[%d] ", arrLine[i]);
        }
        printf("\n\n\n");
    }

    //716442 ����������Ĭ��ֵʱ����ʹû�и�������Ҳ��Ҫ����false
    if ((defstr == NULL) && (iFindCount == 0))
    {
        return false;
    }
    //��ֹShadow��ShadowPassword�������ڳ����������޸�(Y->N)�������������ֶν��н��ܲ���
    if (0 == TStrFunc::StrCmpNoCase(Ident, "password") || 0 == TStrFunc::StrCmpNoCase(Ident, "pass_word"))
    {
        char sTemp[256] = {0};
        if(IsEncryptStr(sValue))
        {
            DecryptEx(sValue,sTemp);
        }
        if(IsEncryptStr_Aes(sValue))
        {
            DecryptEx_Aes(sValue, sTemp);
        }
        if(sTemp[0]!='\0')
        {
            memset(sValue, 0, sizeof(sValue));
            memccpy(sValue, sTemp, 0, strlen(sTemp));
            sValue[strlen(sTemp)] = '\0';
        }
    }

    if (m_iSecretPassword == 1)
    {
        if (m_bUseNewDecode && (0 == TStrFunc::StrCmpNoCase(Ident, "username") || 0 == TStrFunc::StrCmpNoCase(
            Ident, "user_name")))
        {
            char sTemp[21] = {0};
            //strncpy(sTemp, Decrypt(sValue), sizeof(sTemp) - 1);
            char* pUserName = Decrypt(sValue);
            if(pUserName != NULL)
            {
                strncpy(sTemp,pUserName,sizeof(sTemp)-1);;
            }
            snprintf(sValue, 20, "%s", sTemp);
        }
    }
    //�ж��Ƿ����ض�
    bCutOff = strlen(sValue) >= uItemMaxSize;
    strncpy(sItem, sValue, uItemMaxSize - 1);
    sItem[uItemMaxSize - 1] = '\0';

    //������ضϣ���ʾδ��ȡ����ɵ����������false
    return bCutOff == false;
}

int TReadIni::ReadInteger(const char *Section, const char *Ident, const int defint )
{
    char defstr[LINEMAXCHARS]={0}, sItem[LINEMAXCHARS]={0}, *ptr=NULL; //357131 for cppcheck
    //ZMP:467373 
    snprintf(defstr, sizeof(defstr)-1, "%d", defint);
    defstr[sizeof(defstr)-1] = '\0';
    ptr = ReadString(Section, Ident, sItem, defstr);
    if ( ptr == NULL )
        return defint;
    else
        return atoi(sItem) ;
}

/**
 * @brief  ��ȡ������
 *
 * ��ȡ�����������
 *
 * @param Section  [in]  ����
 * @param Ident  [in]  ����
 * @param defint  [in]  ȱʡ��ֵ
 * @return  int
 * @retval  defint  �޼�ֵ���޼����޶�
 * @retval  ��ֵ
 */
int TReadIni::ReadIntegerPlus(const char *Section, const char *Ident, const int defint )
{
    char defstr[LINEMAXCHARS]={0}, sItem[LINEMAXCHARS]={0}, *ptr=NULL; //357131 for cppcheck
    //ZMP:467373 
    snprintf(defstr, sizeof(defstr), "%d", defint);
    defstr[sizeof(defstr)-1] = '\0';
    ptr = ReadString(Section, Ident, sItem, defstr);
    if ( ( ptr == NULL ) || ( sItem[0] == '\0' ) )
        return defint;
    else
        return atoi(sItem) ;
}

TReadIni::TReadIni():fd(NULL)
{
    m_iSecretPassword = 0;
    m_bUseNewDecode   = false;
    m_sFileName[0] = '\0';
}


TReadIni::TReadIni(const char *FileName):fd(NULL)
{
    m_iSecretPassword = 0;
    m_bUseNewDecode   = false;
    m_sFileName[0] = '\0';
    SetFileName(FileName);
  
}

TReadIni::~TReadIni()
{
    if(fd)
    {
        fclose(fd);
        fd = NULL; //357131 for cppcheck
    }
}


void TReadIni::SetFileName(const char *FileName)
{
    try
    {
        memccpy(m_sFileName, FileName, 0, sizeof(m_sFileName) - 1);
        m_sFileName[sizeof(m_sFileName) - 1] = '\0';
        fd = fopen(FileName, "r");
        if (NULL == fd)
        {
            int iError = errno;
            throw TException("Open [%s] Error. errno [%d], error_msg [%s]",
                FileName, iError, strerror(iError));
        }
    }
    catch (TException &oe)
    {
        cout << endl << endl << "TReadIni::SetFileName()   : " << oe.GetErrMsg() << endl;
        exit(0);
    }
    catch (...)
    {
        cout << endl << endl << "TReadIni::SetFileName() Unknow Exception " << endl;
        exit(0);
    }

    if (m_iSecretPassword == 0)
    {
        char sTemp[128] = { 0 };//ZMP:477242
        //memset(sTemp, 0, sizeof(sTemp));
        ReadString("COMMON", "Shadow", sTemp, NULL);
        if (sTemp[0] == '\0')
        {
            ReadString("COMMON", "ShadowPassword", sTemp, "N");
        }
        if (sTemp[0] == 'Y' || sTemp[0] == 'y')
        {//������Ҫ����
            m_iSecretPassword = 1;
        }
        else
        {//���ģ�����Ҫ����
            m_iSecretPassword = 2;
        }
        m_bUseNewDecode = ReadInteger("COMMON", "USE_NEW_DECODE_FLAG", 0) == 1;
    }
    
        char sMode[128] = { 0 };
        //memset(sMode, 0, sizeof(sMode));
        ReadString("COMMON", "EncryptMethod", sMode, "N");
        if (sMode[0] == 'Y' || sMode[0] == 'y')
        {//ʹ��aes�ļ���/�����ַ���
            m_iMethod = 2;
        }
        else
        {//ʹ��ԭ���ļ���/�����ַ���
            m_iMethod = 1;
        }
          
    if (1 == m_iSecretPassword)
    {
        fclose(fd);
        ResetPassWord(FileName);
        try
        {
            fd = fopen(FileName, "r");
            if (NULL == fd)
            {
                int iError = errno;
                throw TException("Open [%s] Error. errno [%d], error_msg [%s]",
                    FileName, iError, strerror(iError));
            }
        }
        catch (TException &oe)
        {
            cout << endl << endl << "TReadIni::SetFileName()   : " << oe.GetErrMsg() << endl;
            exit(0);
        }
        catch (...)
        {
            cout << endl << endl << "TReadIni::SetFileName() Unknow Exception " << endl;
            exit(0);
        }
    }
}

char* TReadIni::Decode(const char * pass)
{
  static char password[128]   = {0};
  size_t         len          = 0;
  char        tmpbyte         = 0x00;

  if(pass == NULL)
    return NULL;

  len = strlen(pass);//ZMP:477242
      
  for(size_t i = 0; i < len - 1; i+=2)
  {
    char tmpstr[10] = {0};
    snprintf(tmpstr,sizeof(tmpstr)-1, "0x%c%c", pass[i], pass[i+1]);
    password[i/2] = (char)strtol(tmpstr, NULL, 16);
  }
  password[len/2] = '\0';

  len = strlen(password);
  tmpbyte = (char)((password[len-1] & 0x07) << 5);
  password[len-1] = (char)((unsigned char)password[len-1] >> 3);

  for(size_t i = len - 1; i > 0; i--){
    password[i] = (char)(password[i] +((password[i-1] & 0x07 ) << 5));
    password[i] = (char)(~(unsigned char)password[i]);
    password[i-1] =(char)((unsigned char)password[i-1] >> 3);
  }

  password[0] = (char)(password[0]+tmpbyte);
  password[0] = (char)(~(unsigned char)password[0]);

  password[len] = '\0';

  return password;
}

char *TReadIni::Decrypt(const char * password)
{
    if(strlen(password) < 2 || password == NULL)
        return NULL;
    
    static char sRetStr[2048] = {0};
    string srcPasswd = password;

    memset(sRetStr,0,sizeof(sRetStr));

    unsigned int ch = 0;


    long unsigned int i = 0;
    for( i = 0;i<((long unsigned int)strlen(password)/2); ++i)
    {
        sscanf(srcPasswd.substr(2*i,2).c_str(), "%x", &ch);
        sRetStr[i] = (char)(ch-30);
    }
    sRetStr[i] = '\0';
    
    return sRetStr; 
}

/******************************************************************************
* ��������  :  EncryptEx_Aes()
* ��������  :  aes�����ַ���
* ����      :  cSrc ����ǰ�ַ���
* ���      :  cDest ���ܺ��ַ���
* ����ֵ    :  ��
* ����      :  
*******************************************************************************/
void TReadIni::EncryptEx_Aes(const char* cSrc,char* cDest)   
{    
    if(NULL == cSrc || NULL == cDest)
    {
        return;
    }
    unsigned char sSecretKet[256] = "qqwwee";
    AesEncryptor oAesoperate(sSecretKet);
    string sCipherText = oAesoperate.EncryptString(cSrc);//�õ�aes���ܵ�����
    strcat(cDest,"!!"); 
    strncpy(cDest + 2, sCipherText.c_str(), sCipherText.size());//��ʾ������
    strcat(cDest,"$$");     
    return;   
}

/******************************************************************************
* ��������  :  EncryptEx()
* ��������  :  �����ַ���
* ����      :  cSrc ����ǰ�ַ���
* ���      :  cDest ���ܺ��ַ���
* ����ֵ    :  ��
* ����      :  
*******************************************************************************/
void TReadIni::EncryptEx(const char* cSrc,char* cDest)   
{   
    char c;   
    int i;
    int h;
    int l;
    int j=2;  
    if(NULL == cSrc || NULL == cDest)
    {
        return;
    }
    strcat(cDest,"!!");
    for(i=0;i<(int)strlen(cSrc);i++)   
    {
        c = cSrc[i];
        h = (c - 0)/10;   
        l = (c - 0)%10;   
        cDest[j]=(char)(h+'A'); 
        cDest[j+1]=(char)(l+'A'); 
        j += 2;
    }   
    strcat(cDest,"@@");
    cDest[j+2]='\0';     
    return;   
}

/******************************************************************************
* ��������  :  DecryptEx_Aes()
* ��������  :  aes�����ַ���
* ����      :  cSrc ����ǰ�ַ���
* ���      :  cDest ���ܺ��ַ���
* ����ֵ    :  ��
* ����      :  
*******************************************************************************/
void TReadIni::DecryptEx_Aes(const char* cSrc,char* cDest)   
{      
    if(NULL == cSrc || NULL == cDest)
    {
        return;
    }
    unsigned char sSecretKet[256] = "qqwwee";
    AesEncryptor oAesoperate(sSecretKet);
    char cSrc_Head[256] = "";
    char cSrc_Tail[256] = "";
    strncpy(cSrc_Head,cSrc+2,strlen(cSrc)-2);//ȥ���ײ���ʶ��
    strncpy(cSrc_Tail,cSrc_Head,strlen(cSrc_Head)-2);//ȥ��β����ʶ��
    string sInvCipherText = oAesoperate.DecryptString(cSrc_Tail);//�õ�Aes��������
    strncpy(cDest,sInvCipherText.c_str(),sInvCipherText.size());
    return;
}

/******************************************************************************
* ��������  :  DecryptEx()
* ��������  :  �����ַ���
* ����      :  cSrc ����ǰ�ַ���
* ���      :  cDest ���ܺ��ַ���
* ����ֵ    :  ��
* ����      :  
*******************************************************************************/
void TReadIni::DecryptEx(const char* cSrc,char* cDest)   
{   
    int i;
    int h;
    int l;
    int j=0;   
    if(NULL == cSrc || NULL == cDest)
    {
        return;
    }
    if(!IsEncryptStr(cSrc)) return;
    cSrc = cSrc + 2;
    for(i=0;i<(int)strlen(cSrc)-2;i=i+2)   
    {   
        h = (char)(cSrc[i]-'A');   
        l = (cSrc[i+1]-'A');   
        cDest[j] = (char)(h*10 + l);   
        j++;   
    }   
    cDest[j]='\0';   
    return;   
} 

/******************************************************************************
* ��������  :  IsEncryptStr()
* ��������  :  �ж�һ���ַ����Ƿ�Ϊ�ڲ������ַ���
* ����      :  pchString Դ�ַ���
* ���      :  ��
* ����ֵ    :  true Ϊ�ڲ������ַ�����false : �Ǽ����ַ���
* ����      :  
*******************************************************************************/
bool TReadIni::IsEncryptStr(const char *pchString)
{
    if(NULL == pchString)
    {
        return false;
    }
    
    if(pchString[0] == '!'
        && pchString[1] == '!'
        && pchString[strlen(pchString)-1] == '@'
        && pchString[strlen(pchString)-2] == '@')
    {
        return true;
    }
        return false;
}

/******************************************************************************
* ��������  :  IsEncryptStr_Aes()
* ��������  :  �ж�һ���ַ����Ƿ�Ϊ�ڲ������ַ���
* ����      :  pchString Դ�ַ���
* ���      :  ��
* ����ֵ    :  true Ϊ�ڲ������ַ�����false : �Ǽ����ַ���
* ����      :  
*******************************************************************************/
bool TReadIni::IsEncryptStr_Aes(const char *pchString)
{
    if(NULL == pchString)
    {
        return false;
    }
    
    if(pchString[0] == '!'
        &&pchString[1] == '!'
        &&pchString[strlen(pchString)-1] == '$'
        &&pchString[strlen(pchString)-2] == '$')
    {
        return true;
    }
        return false;

}


/******************************************************************************
* ��������  :  ResetPassWord()
* ��������  :  �ļ���pass_word����PASS_WORD��Ӧ����������ת��������
* ����      :  FileName�ļ���
* ���      :  ��
* ����ֵ    : ��
* ����      :  
*******************************************************************************/

bool TReadIni::ResetPassWord(const char* FileName)
{
    FILE *fTmp;
    char sTmpFileName[MAXSTRINGNAME] = { 0 };
    char sLineBuff[LINEMAXCHARS] = { 0 };
    char* pNewLine = NULL;
    char *p = NULL;
    char *pTmp = NULL;
    char sTmp1[LINEMAXCHARS] = { 0 };
    char sSecretPsd[LINEMAXCHARS] = { 0 };
    char sComment[LINEMAXCHARS] = { 0 };
    char sUppItem[10] = "PASS_WORD";
    char sUppItem1[10] = "PASSWORD";
    strncpy(sTmpFileName, FileName, sizeof(sTmpFileName));
    strcat(sTmpFileName, "_tmp");
    fTmp = fopen(sTmpFileName, "w");
    if (NULL == fTmp)
    {
        int iError = errno;
        printf("fopen(%s, %s) failed. errno : %d, reason : %s \n", sTmpFileName, "w", iError, strerror(iError));
        return false;
    }

    FILE * fFile = fopen(FileName, "r");
    if (NULL == fFile)
    {
        int iError = errno;
        printf("fopen(%s, %s) failed. errno : %d, reason : %s \n", FileName, "r", iError, strerror(iError));
        
        fclose(fTmp);
        fTmp = NULL;
        return false;
    }

    bool bNoPassWordReset = true;  //�����Ƿ���������Ҫ���ܣ�û������Ҫ�����ļ�
    while (fgets(sLineBuff, LINEMAXCHARS, fFile)) /*�Ѻ�������ݻ���*/
    {
        pTmp = (char *)sLineBuff;
        memset(sTmp1, 0, sizeof(sTmp1));
        p = (char *)strchr(sLineBuff, '=');
        bool bIsPassWordLine = (p) &&
            ((TStrFunc::StrCmpNoCase(Trim(TruncStr(sLineBuff, sTmp1, (int)(p - pTmp))), sUppItem) == 0) ||
            (TStrFunc::StrCmpNoCase(Trim(TruncStr(sLineBuff, sTmp1, (int)(p - pTmp))), sUppItem1) == 0));
        if (bIsPassWordLine)
        {
            pNewLine = strchr(p + 1, '#');
            if (pNewLine != NULL)
            {
                strncpy(sComment, pNewLine, strlen(pNewLine));
                sComment[strlen(sComment) - 1] = '\0';
                *pNewLine = '\0';
            }
            else
            {
                sComment[0] = '\0';
            }
            FUNC_STR::FormatChar(p + 1);
            char *pItem = Trim(p + 1);
            char pSecretPassWord[LINEMAXCHARS] = { 0 };
            if (IsEncryptStr(pItem) || IsEncryptStr_Aes(pItem))
            {
                strncpy(pSecretPassWord, pItem, strlen(pItem));
            }
            else
            {
                bNoPassWordReset = false;
                if(m_iMethod == 1)
                {
                    EncryptEx(pItem, pSecretPassWord);
                }
                if(m_iMethod == 2)
                {
                    EncryptEx_Aes(pItem,pSecretPassWord);
                }
            }
            memset(sSecretPsd, 0, strlen(sSecretPsd));
            strncpy(sSecretPsd, sTmp1, strlen(sTmp1));
            strcat(sSecretPsd, "=");
            strcat(sSecretPsd, pSecretPassWord);
            if (sComment[0] != '\0')
            {
                strcat(sSecretPsd, "    ");
                strcat(sSecretPsd, sComment);
            }
            sSecretPsd[strlen(sSecretPsd)] = '\n';
            fputs(sSecretPsd, fTmp);
            continue;
        }
        fputs(pTmp, fTmp);
    }
    
    fclose(fFile);
    fFile = NULL;
    fflush(fTmp);
    fclose(fTmp);
    fTmp = NULL;

    if (bNoPassWordReset)
    {
        unlink(sTmpFileName);
    }
    else
    {
        rename(sTmpFileName, FileName);
    }
    return true;
}

bool TReadIni::ReloadFile()
{
    if(fd != NULL)
    {
        fclose(fd);
        fd = NULL;
    }
    fd = fopen (m_sFileName, "r");
    if (NULL == fd)
    {
        int iError = errno;
        printf("fopen(%s, %s) failed. errno : %d, reason : %s \n", m_sFileName, "r", iError, strerror(iError));
        return false;
    }
    return true;
}

