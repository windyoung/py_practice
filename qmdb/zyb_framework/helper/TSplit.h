//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TSplit.h
// Author: 刘启铨
// Date: 2004/10/19
// Description: 应用程序公共类
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
        iInCressFieldLen = 50,       ///m_ppFieldValue申请空间的递增值
        iMaxFieldLength = 1024,      ///TSplit类解析 的 每个字段的长度
        iMaxBufSize = 255 * 1024 + 1,///TSplit类的BuffSize
        iInCressLen = 4 * 1024       ///TSplit类申请空间的递增值
    };
    TSplit(const int iFieldCount=MAX_FIELD_NUM,const int iFieldLength=MAX_FIELD_LENGTH);
    virtual ~TSplit();
    /**
      * @brief 设置分隔符
      * @param cSplitChar [in] 分隔符
      * @retval void
      */
    void SetSplitter(char cSplitChar);
    /**
      * @brief 设置待分割的字符串
      * @param sString [in] 待分割的字符串
      * @retval int 0失败/正数成功，即分割后的字段数
      */
    int SetString(const char *sString);
    /**
      * @brief 分割函数
      * @param sSplitString [in] 待分割字符
      * @param pSplitChar [in] 分割符
      * @retval  int 0失败/正数成功，即分割后的字段数
      */
    int QSplit(const char *sSplitString, const char *pSplitChar);
    /**
      * @brief 分割函数
      * @param sSplitString [in] 待分割字符
      * @param pSplitChar [in] 分割符
      * @retval  int 0失败/正数成功，即分割后的字段数
      */
    int Split(const char *sSplitString, const char *pSplitChar);
    /**
      * @brief 分割函数
      * @param sSplitString [in] 待分割字符
      * @param pSplitChar [in] 分割符
      * @retval  int 0失败/正数成功，即分割后的字段数
      */
    int Split2(const char *sSplitString, const char *pSplitChar);
    /**
      * @brief 分割函数
      * @param sSplitString [in] 待分割字符
      * @param cSplitChar [in] 分割符
      * @retval  int 0失败/正数成功，即分割后的字段数
      */
    int Split(const char *sSplitString, const char cSplitChar);
    /**
      * @brief 得到分割后的字段数
      * @retval  int  分割后的字段数
      */
    int GetCount(void);
    /**
      * @brief 根据索引号得到字段值
      * @param  iIndex字段索引
      * @retval const char * 字段索引对应的字段值
      */
    const char * GetIndex(int iIndex);
    /**
     * @brief 根据数组下标得到字段值
     * @param iIndex [in] 数组下标
     * @retval const char *  数组下表对应的字段值
     */
    const char * operator [](int iIndex);
private:
    /**
      * @brief 申请空间函数
      * @param iLen [in] 申请空间的大小 
      * @retval  bool true成功/false失败
      */
    bool MallocBuf(int iLen = iInCressLen);

    /**
      * @brief 为m_ppFieldValue 申请空间函数
      * @param iLen [in] 申请空间的大小 
      * @retval  bool true成功/false失败
      */
    bool AddFieldBuf(int iLen = iInCressFieldLen);
private:
    int          m_iFieldValueCount; ///m_ppFieldValue数组长度
    int          m_iBufLen;          ///m_pBuffer申请的空间长度
    char         m_cSplitChar;       ///分割符
    int          m_uiFieldCount;     ///分割后的字段数
    char **      m_ppFieldValue;     ///存放待分割字符串
    char *       m_pBuffer;          ///存放需要分解的字符
};

#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif //__T_SPLIT_H__
