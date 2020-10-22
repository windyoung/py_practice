//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#ifndef _READINI_H
#define    _READINI_H

#include <string>
#include <iostream>

#define LINEMAXCHARS 4000
#define MAXSTRINGNAME 255     //最大文件名长度   

using namespace std;

class TReadIni {
public:
    TReadIni();
    void SetFileName(const char *FileName);
    TReadIni(const char *FileName);
    virtual ~TReadIni();
    //[COMMON]ShadowPassword=Y或[COMMON]ShadowPassword=y时,
    //Ident为"password"或"pass_word"的配置项为密文，需要解密
    char *ReadString (const char *Section, const char *Ident, char *sItem, const char *defstr );
    /**
     * @brief 读取srting配置项
     * 
     * @param uiType      [in] 比如[COMMON]
     * @param Ident       [in] 比如USER_NAME
     * @param defstr      [in] 读取不到，返回该默认值
     * @param iInBufLen   [in] sItem的长度，配置项的值超过该长度，则截断
     * @param sItem       [out] 存放读取到的值，该参数不能为空
     * @return bool
     * @retval true--成功，false--失败
     */
    bool ReadStringPlus(const char *Section, const char *Ident, char *sItem, const char *defstr, size_t uItemMaxSize);
    int ReadInteger(const char *Section, const char *Ident, const int defint );    
    int ReadIntegerPlus(const char *Section, const char *Ident, const int defint );   
	
    bool SectionCheck(const char *Section);
	  //解密字符串
	  char *Decrypt(const char * password);
	
	  /******************************************************************************
	  * 函数名称  :  EncryptEx_Aes()
	  * 函数描述  :  aes加密字符串
	  * 输入      :  cSrc 加密前字符串
	  * 输出      :  cDest 加密后字符串
	  * 返回值    :  无
	  * 作者      :  
	  *******************************************************************************/
	  static void EncryptEx_Aes(const char* cSrc,char* cDest);
	  /******************************************************************************
	  * 函数名称  :  EncryptEx()
	  * 函数描述  :  加密字符串
	  * 输入      :  cSrc 加密前字符串
	  * 输出      :  cDest 加密后字符串
	  * 返回值    :  无
	  * 作者      :  
	  *******************************************************************************/
	  static void EncryptEx(const char* cSrc,char* cDest);
	  /******************************************************************************
	  * 函数名称  :  DecryptEx_Aes()
	  * 函数描述  :  aes解密字符串
	  * 输入      :  cSrc 加密前字符串
	  * 输出      :  cDest 加密后字符串
	  * 返回值    :  无
	  * 作者      :  
	  *******************************************************************************/
	  static void DecryptEx_Aes(const char* cSrc,char* cDest);
	  /******************************************************************************
	  * 函数名称  :  DecryptEx()
	  * 函数描述  :  解密字符串
	  * 输入      :  cSrc 加密前字符串
	  * 输出      :  cDest 加密后字符串
	  * 返回值    :  无
	  * 作者      :  
	  *******************************************************************************/
	  static void DecryptEx(const char* cSrc,char* cDest);
	  /******************************************************************************
	  * 函数名称  :  IsEncryptStr()
	  * 函数描述  :  判断一个字符串是否为内部加密字符串
	  * 输入      :  pchString 源字符串
	  * 输出      :  无
	  * 返回值    :  true 为内部加密字符串；false : 非加密字符串
	  * 作者      :  
    *******************************************************************************/
    static bool IsEncryptStr(const char *pchString);
    /******************************************************************************
    * 函数名称  :  IsEncryptStr_Aes()
    * 函数描述  :  判断一个字符串是否为内部加密字符串
    * 输入      :  pchString 源字符串
    * 输出      :  无
    * 返回值    :  true 为内部加密字符串；false : 非加密字符串
    * 作者      :  
    *******************************************************************************/
    static bool IsEncryptStr_Aes(const char *pchString);
    /******************************************************************************
	  * 函数名称  :  ReloadFile()
	  * 函数描述  :  关闭并重新打开配置文件。为刷新而新增（ZMP:1085764）
	  * 输入      :  无
	  * 输出      :  无
	  * 返回值     :  bool  True:成功，false:失败。
	  * 作者      :  
	  *******************************************************************************/
    bool ReloadFile();
private:
    char* Decode(const char * pass);
    bool ResetPassWord(const char* FileName);
private:
    char *TruncStr( char *sp , char *sDst , int len);
    //char *SubStr (char *, char *, int); //351951 for cppcheck
    char *Trim( char *str );
    FILE *fd;
    int m_iSecretPassword; // 0 未初始化，1 密码配置为密文， 2 密码配置为明文
    bool m_bUseNewDecode;  //true--使用新解密算法，false--使用旧的解密算法
    int m_iMethod;// 0 未初始化，  1 使用原来的加密/解密字符串,  2 使用Aes加密/解密字符串
    char m_sFileName[255];
};

#endif

