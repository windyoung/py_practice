#ifndef SRC_UTILS_AES_ENCRYPTOR_H
#define SRC_UTILS_AES_ENCRYPTOR_H
#include <string>

class AES;

class AesEncryptor
{
public:
    AesEncryptor(unsigned char* key);
    AesEncryptor(const AesEncryptor& AesInput);
    ~AesEncryptor(void);

    std::string EncryptString(std::string strInfor);
    std::string DecryptString(std::string strMessage);

private:
    void Byte2Hex(const unsigned char* src, int len, char* dest);
    void Hex2Byte(const char* src, int len, unsigned char* dest);
    int  Char2Int(char c);

private:
    AES* m_pEncryptor;
};

#endif        // SRC_UTILS_AES_ENCRYPTOR_H