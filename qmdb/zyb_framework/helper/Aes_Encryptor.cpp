#include "Aes_Encryptor.h"
#include "Aes.h"
#include <fstream>
#include <iostream>
#include <cstring>

using std::string;

AesEncryptor::AesEncryptor(unsigned char* key)
{
    m_pEncryptor = new AES(key);
}
AesEncryptor::AesEncryptor(const AesEncryptor& AesInput)
{
    if (NULL != AesInput.m_pEncryptor)
    {
        delete m_pEncryptor;
        m_pEncryptor = NULL;
    }	
}


AesEncryptor::~AesEncryptor(void)
{
    delete m_pEncryptor;
    m_pEncryptor = NULL;
}

void AesEncryptor::Byte2Hex(const unsigned char* src, int len, char* dest) 
{
    for (int i = 0; i < len; ++i) 
    {
        snprintf(dest + (long long)i*2, 3, "%02X", src[i]);
    }
}

void AesEncryptor::Hex2Byte(const char* src, int len, unsigned char* dest) 
{
    int length = len / 2;
    for (int i = 0; i < length; ++i) 
    {
        dest[i] = Char2Int(src[i * 2]) * 16 + Char2Int(src[i * 2 + 1]);
    }
}

int AesEncryptor::Char2Int(char c) 
{
    if ('0' <= c && c <= '9') 
    {
        return (c - '0');
    }
    else if ('a' <= c && c <= 'f') 
    {
        return (c - 'a' + 10);
    }
    else if ('A' <= c && c <= 'F') 
    {
        return (c - 'A' + 10);
    }
    return -1;
}

string AesEncryptor::EncryptString(string strInfor) 
{
    size_t nLength = strInfor.length();
    size_t spaceLength = 16 - (nLength % 16);
    size_t iAllLength = nLength + spaceLength;
    unsigned char* pBuffer = new unsigned char[iAllLength];
    memset(pBuffer, '\0', iAllLength);
    //memcpy_s(pBuffer, iAllLength, strInfor.c_str(), nLength);
    memcpy(pBuffer, strInfor.c_str(), nLength);
    m_pEncryptor->Cipher(pBuffer, (int)iAllLength);

    // 这里需要把得到的字符数组转换成十六进制字符串 
    size_t iAllLengthNext = 2 * iAllLength + 1;
    char* pOut = new char[iAllLengthNext];
    memset(pOut, '\0', iAllLengthNext);
    Byte2Hex(pBuffer, (int)iAllLength, pOut);

    string retValue(pOut);
    delete[] pBuffer;
    delete[] pOut;
    pBuffer = NULL;
    pOut = NULL;
    return retValue;
}

string AesEncryptor::DecryptString(string strMessage) 
{
    size_t nLength = strMessage.length() / 2;
    unsigned char* pBuffer = new unsigned char[nLength];
    memset(pBuffer, '\0', nLength);
    Hex2Byte(strMessage.c_str(), (int)strMessage.length(), pBuffer);

    m_pEncryptor->InvCipher(pBuffer, (int)nLength);
    string retValue((char*)pBuffer);
    delete[] pBuffer;
    pBuffer = NULL;
    return retValue;
}

