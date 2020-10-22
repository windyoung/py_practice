//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#ifndef _READINI_H
#define    _READINI_H

#include <string>
#include <iostream>

#define LINEMAXCHARS 4000
#define MAXSTRINGNAME 255     //����ļ�������   

using namespace std;

class TReadIni {
public:
    TReadIni();
    void SetFileName(const char *FileName);
    TReadIni(const char *FileName);
    virtual ~TReadIni();
    //[COMMON]ShadowPassword=Y��[COMMON]ShadowPassword=yʱ,
    //IdentΪ"password"��"pass_word"��������Ϊ���ģ���Ҫ����
    char *ReadString (const char *Section, const char *Ident, char *sItem, const char *defstr );
    /**
     * @brief ��ȡsrting������
     * 
     * @param uiType      [in] ����[COMMON]
     * @param Ident       [in] ����USER_NAME
     * @param defstr      [in] ��ȡ���������ظ�Ĭ��ֵ
     * @param iInBufLen   [in] sItem�ĳ��ȣ��������ֵ�����ó��ȣ���ض�
     * @param sItem       [out] ��Ŷ�ȡ����ֵ���ò�������Ϊ��
     * @return bool
     * @retval true--�ɹ���false--ʧ��
     */
    bool ReadStringPlus(const char *Section, const char *Ident, char *sItem, const char *defstr, size_t uItemMaxSize);
    int ReadInteger(const char *Section, const char *Ident, const int defint );    
    int ReadIntegerPlus(const char *Section, const char *Ident, const int defint );   
	
    bool SectionCheck(const char *Section);
	  //�����ַ���
	  char *Decrypt(const char * password);
	
	  /******************************************************************************
	  * ��������  :  EncryptEx_Aes()
	  * ��������  :  aes�����ַ���
	  * ����      :  cSrc ����ǰ�ַ���
	  * ���      :  cDest ���ܺ��ַ���
	  * ����ֵ    :  ��
	  * ����      :  
	  *******************************************************************************/
	  static void EncryptEx_Aes(const char* cSrc,char* cDest);
	  /******************************************************************************
	  * ��������  :  EncryptEx()
	  * ��������  :  �����ַ���
	  * ����      :  cSrc ����ǰ�ַ���
	  * ���      :  cDest ���ܺ��ַ���
	  * ����ֵ    :  ��
	  * ����      :  
	  *******************************************************************************/
	  static void EncryptEx(const char* cSrc,char* cDest);
	  /******************************************************************************
	  * ��������  :  DecryptEx_Aes()
	  * ��������  :  aes�����ַ���
	  * ����      :  cSrc ����ǰ�ַ���
	  * ���      :  cDest ���ܺ��ַ���
	  * ����ֵ    :  ��
	  * ����      :  
	  *******************************************************************************/
	  static void DecryptEx_Aes(const char* cSrc,char* cDest);
	  /******************************************************************************
	  * ��������  :  DecryptEx()
	  * ��������  :  �����ַ���
	  * ����      :  cSrc ����ǰ�ַ���
	  * ���      :  cDest ���ܺ��ַ���
	  * ����ֵ    :  ��
	  * ����      :  
	  *******************************************************************************/
	  static void DecryptEx(const char* cSrc,char* cDest);
	  /******************************************************************************
	  * ��������  :  IsEncryptStr()
	  * ��������  :  �ж�һ���ַ����Ƿ�Ϊ�ڲ������ַ���
	  * ����      :  pchString Դ�ַ���
	  * ���      :  ��
	  * ����ֵ    :  true Ϊ�ڲ������ַ�����false : �Ǽ����ַ���
	  * ����      :  
    *******************************************************************************/
    static bool IsEncryptStr(const char *pchString);
    /******************************************************************************
    * ��������  :  IsEncryptStr_Aes()
    * ��������  :  �ж�һ���ַ����Ƿ�Ϊ�ڲ������ַ���
    * ����      :  pchString Դ�ַ���
    * ���      :  ��
    * ����ֵ    :  true Ϊ�ڲ������ַ�����false : �Ǽ����ַ���
    * ����      :  
    *******************************************************************************/
    static bool IsEncryptStr_Aes(const char *pchString);
    /******************************************************************************
	  * ��������  :  ReloadFile()
	  * ��������  :  �رղ����´������ļ���Ϊˢ�¶�������ZMP:1085764��
	  * ����      :  ��
	  * ���      :  ��
	  * ����ֵ     :  bool  True:�ɹ���false:ʧ�ܡ�
	  * ����      :  
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
    int m_iSecretPassword; // 0 δ��ʼ����1 ��������Ϊ���ģ� 2 ��������Ϊ����
    bool m_bUseNewDecode;  //true--ʹ���½����㷨��false--ʹ�þɵĽ����㷨
    int m_iMethod;// 0 δ��ʼ����  1 ʹ��ԭ���ļ���/�����ַ���,  2 ʹ��Aes����/�����ַ���
    char m_sFileName[255];
};

#endif

