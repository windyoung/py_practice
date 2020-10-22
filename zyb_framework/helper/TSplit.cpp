//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TSplit.cpp
// Author: ������
// Date: 2005/03/01
// Description: Ӧ�ó��򹫹���
////////////////////////////////////////////////
/*
*/
#include "string.h"
#include "TSplit.h"
#include "stdio.h"
#include "Common.h"
#include "TException.h"
#include "debug_new.h"

TSplit::TSplit(const int iFieldCount,const int iFieldLength)
{
    //ZMP:392633
    #ifdef _ASSERT_PARAM
    if((0 == iFieldCount)&&(0 == iFieldLength))
    {
        throw TException("TSplit::TSplit:Input Parameters is not right");                
    }
    #endif
    m_iFieldValueCount = 255;
    m_iBufLen = 0;
    m_pBuffer = NULL;
    m_ppFieldValue = NULL; 
    //ZMP:506413
    m_uiFieldCount = 0;
    m_cSplitChar = '=';
}

TSplit::~TSplit()
{
    m_iFieldValueCount = 0;
    m_iBufLen = 0;
    if (NULL != m_pBuffer)
    {
        delete [] m_pBuffer;
        m_pBuffer = NULL;
    }

    if (NULL != m_ppFieldValue)
    {
        delete [] m_ppFieldValue;
        m_ppFieldValue = NULL;
    }
}

void TSplit::SetSplitter(char cSplitChar)
{
    m_cSplitChar = cSplitChar;
}

int TSplit::SetString(const char *sString)
{
    m_uiFieldCount = 0;//�ָ����ֶ�����Ϊ0
    
    if (NULL == sString )
    {
        return 0;  
    }
    
    #ifdef _ASSERT_PARAM
    if('\0' == m_cSplitChar)
    {
        throw TException("TSplit::SetString:the Splitter is not right"); 
    }
    #endif

    int iStringlength = (int)strlen(sString);//ZMP:485923 
    
    //�������Ŀռ��С��������Ҫ�������� 
    if (!MallocBuf(iStringlength))
    {
        return 0;
    }

    if ( (NULL == m_pBuffer) 
        || (NULL == m_ppFieldValue))
    {
        return 0;
    }

    //�����������ֶ�ֵָ���������
    m_pBuffer[0] = 0;
    memset(m_ppFieldValue, 0, (size_t)m_iFieldValueCount*sizeof(char *));

    //�����ָ��ַ���������������
    int iMinLen = (m_iBufLen - 1) > iStringlength? iStringlength:(m_iBufLen - 1);
    memcpy(m_pBuffer, sString, (size_t)iMinLen);
    m_pBuffer[iMinLen] = 0;

    //�ָ��ַ���
    char *pStr = m_pBuffer;
    
    m_ppFieldValue[m_uiFieldCount++] = pStr;
    
    while (NULL != (pStr = strchr(pStr, m_cSplitChar)))
    {
        *pStr = '\0';
        m_ppFieldValue[m_uiFieldCount++] = ++pStr;

        ///����ߵ����ﱻ�ָ���ַ�������Ҫ����
        ///m_ppFieldValue������50���ռ�;
        if (m_iFieldValueCount == m_uiFieldCount)
        {
            if (!AddFieldBuf())
            {
                return 0;
            }
        }
    }

    return m_uiFieldCount;
}

int TSplit::QSplit(const char *sSplitString, const char *pSplitChar)
{
    m_uiFieldCount = 0;
    
    #ifdef _ASSERT_PARAM
    if(pSplitChar==NULL || !strcmp(pSplitChar,""))
    {
        throw TException("TSplit::QSplit:the Splitter is NULL,or is \"\"");                 
    }
    #endif
    
    ///���÷ָ���
    SetSplitter(*pSplitChar);
    return SetString(sSplitString);
}

int TSplit::Split(const char *sSplitString, const char *pSplitChar)
{
    m_uiFieldCount = 0;

    #ifdef _ASSERT_PARAM
    if( pSplitChar == NULL || !strcmp(pSplitChar,""))
    {
        throw TException("TSplit::Split:the Splitter is NULL,or is \"\"");                 
    } 
    #endif
    
    ///���÷ָ���
    SetSplitter(*pSplitChar);
    return SetString(sSplitString);
}

int TSplit::Split2(const char *sSplitString, const char *pSplitChar)
{
    m_uiFieldCount = 0;

    #ifdef _ASSERT_PARAM
    if( pSplitChar == NULL || !strcmp(pSplitChar,""))
    {
        throw TException("TSplit::Split2:the Splitter is NULL,or is \"\"");               
    } 
    #endif
    
    ///���÷ָ���
    SetSplitter(*pSplitChar); 
    return SetString(sSplitString);
}


int TSplit::Split(const char *sSplitString, const char cSplitChar)
{
    SetSplitter(cSplitChar);
    return SetString(sSplitString);
}

int TSplit::GetCount(void)
{
    return m_uiFieldCount;
}

const char * TSplit::GetIndex(int iIndex)
{
    const char *pRetValue=NULL;
    if(iIndex<m_uiFieldCount)
    {
        pRetValue = m_ppFieldValue[iIndex];
    }
    return pRetValue;
}

const char * TSplit::operator [](int iIndex)
{
    const char *pRetValue=NULL;
    if(iIndex<m_uiFieldCount)
    {
        //pRetValue = m_StringList[iIndex];
        return m_ppFieldValue[iIndex];
    }
    return pRetValue;
}

bool TSplit::MallocBuf(int iLen /*= iInCressLen*/)
{
    if (iLen < 1)
    {
        return false;
    }
    
    ///û�г�ʼ��
    if (NULL == m_ppFieldValue)
    {
        //m_cSplitChar =  '=';
        m_uiFieldCount = 0;

        m_ppFieldValue = new char* [m_iFieldValueCount]; 
        if (NULL == m_ppFieldValue)
        {
            return false;
        }
        
        memset(m_ppFieldValue, 0, (size_t)m_iFieldValueCount*sizeof(char *));
        m_pBuffer = NULL;
        m_iBufLen = 0;
    }
    
    ///�ռ乻��
    if ((NULL != m_pBuffer) && (iLen < m_iBufLen))
    {
        m_pBuffer[0] = '\0';
        return true;
    }
    
    //����ռ��С
    int iBufLen = iInCressLen + 1;
    int iCount = iLen / iInCressLen;
    if (0 == iLen % iInCressLen)
    {
        iBufLen = iCount * iInCressLen + 1;
    }
    else 
    {
        iBufLen = (iCount + 1) * iInCressLen + 1;
    }
    
    if (NULL != m_pBuffer)
    {
        delete [] m_pBuffer;
        m_pBuffer = NULL;
    }
    
    m_pBuffer = new char [iBufLen];
    if (NULL == m_pBuffer)
    {
        delete []m_ppFieldValue;
        m_ppFieldValue = NULL;
        return false;
    }
    m_pBuffer[0] = '\0';
    
    m_iBufLen = iBufLen;
    
    return true;
}

bool TSplit::AddFieldBuf(int iLen /*= iInCressFieldLen*/)
{
    int iFieldLen = m_iFieldValueCount + iInCressFieldLen;
    char ** pTmp = new char* [iFieldLen];
    if (NULL == pTmp)
    {
        return false;
    }
    
    memset(pTmp + (size_t)m_iFieldValueCount, 0, sizeof(char *) * (size_t)iInCressFieldLen);
    for(int i = 0; i < m_iFieldValueCount; i++)
    {
        pTmp[i] = m_ppFieldValue[i];
    }
    
    m_iFieldValueCount = iFieldLen;
    delete [] m_ppFieldValue;
    m_ppFieldValue = pTmp;
    return true;
}



