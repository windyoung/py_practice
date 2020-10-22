//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TSplit.h
// Author: ������
// Date: 2004/10/19
// Description: Ӧ�ó��򹫹���
////////////////////////////////////////////////
/*
 */
#ifndef __SPLIT_H__
#define __SPLIT_H__

#define MAX_FIELD_NUM    1024
#define MAX_FIELD_LENGTH 255

#ifdef HP_UNIX
//  #pragma pack 8
#endif

class TSplit {
public:
    enum
    {
        iInCressFieldLen = 50,       ///m_ppFieldValue����ռ�ĵ���ֵ
        iMaxFieldLength = 1024,      ///TSplit����� �� ÿ���ֶεĳ���
        iMaxBufSize = 255 * 1024 + 1,///TSplit���BuffSize
        iInCressLen = 4 * 1024       ///TSplit������ռ�ĵ���ֵ
    };
    TSplit(const int iFieldCount=MAX_FIELD_NUM,const int iFieldLength=MAX_FIELD_LENGTH);
    virtual ~TSplit();
    /**
      * @brief ���÷ָ���
      * @param cSplitChar [in] �ָ���
      * @retval void
      */
    void SetSplitter(char cSplitChar);
    /**
      * @brief ���ô��ָ���ַ���
      * @param sString [in] ���ָ���ַ���
      * @retval int 0ʧ��/�����ɹ������ָ����ֶ���
      */
    int SetString(const char *sString);
    /**
      * @brief �ָ��
      * @param sSplitString [in] ���ָ��ַ�
      * @param pSplitChar [in] �ָ��
      * @retval  int 0ʧ��/�����ɹ������ָ����ֶ���
      */
    int QSplit(const char *sSplitString, const char *pSplitChar);
    /**
      * @brief �ָ��
      * @param sSplitString [in] ���ָ��ַ�
      * @param pSplitChar [in] �ָ��
      * @retval  int 0ʧ��/�����ɹ������ָ����ֶ���
      */
    int Split(const char *sSplitString, const char *pSplitChar);
    /**
      * @brief �ָ��
      * @param sSplitString [in] ���ָ��ַ�
      * @param pSplitChar [in] �ָ��
      * @retval  int 0ʧ��/�����ɹ������ָ����ֶ���
      */
    int Split2(const char *sSplitString, const char *pSplitChar);
    /**
      * @brief �ָ��
      * @param sSplitString [in] ���ָ��ַ�
      * @param cSplitChar [in] �ָ��
      * @retval  int 0ʧ��/�����ɹ������ָ����ֶ���
      */
    int Split(const char *sSplitString, const char cSplitChar);
    /**
      * @brief �õ��ָ����ֶ���
      * @retval  int  �ָ����ֶ���
      */
    int GetCount(void);
    /**
      * @brief ���������ŵõ��ֶ�ֵ
      * @param  iIndex�ֶ�����
      * @retval const char * �ֶ�������Ӧ���ֶ�ֵ
      */
    const char * GetIndex(int iIndex);
    /**
     * @brief ���������±�õ��ֶ�ֵ
     * @param iIndex [in] �����±�
     * @retval const char *  �����±��Ӧ���ֶ�ֵ
     */
    const char * operator [](int iIndex);
private:
    /**
      * @brief ����ռ亯��
      * @param iLen [in] ����ռ�Ĵ�С 
      * @retval  bool true�ɹ�/falseʧ��
      */
    bool MallocBuf(int iLen = iInCressLen);

    /**
      * @brief Ϊm_ppFieldValue ����ռ亯��
      * @param iLen [in] ����ռ�Ĵ�С 
      * @retval  bool true�ɹ�/falseʧ��
      */
    bool AddFieldBuf(int iLen = iInCressFieldLen);
private:
    int          m_iFieldValueCount; ///m_ppFieldValue���鳤��
    int          m_iBufLen;          ///m_pBuffer����Ŀռ䳤��
    char         m_cSplitChar;       ///�ָ��
    int          m_uiFieldCount;     ///�ָ����ֶ���
    char **      m_ppFieldValue;     ///��Ŵ��ָ��ַ���
    char *       m_pBuffer;          ///�����Ҫ�ֽ���ַ�
};

#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif //__T_SPLIT_H__
