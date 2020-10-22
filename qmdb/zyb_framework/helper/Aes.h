#ifndef SRC_UTILS_AES_H
#define SRC_UTILS_AES_H

class AES
{
public:
    AES(unsigned char* key);
    virtual ~AES();
    unsigned char* Cipher(unsigned char* input);//加密，传入的数组大小必须是16字节
    unsigned char* InvCipher(unsigned char* input);//解密，传入的数组也必须是16字节
    void* Cipher(void* input, int length = 0);//可以传入数组，大小必须是16的整数倍，如果不是将会越界操作；如果不传length而默认为0，那么将按照字符串处理，遇'\0'结束
    void* InvCipher(void* input, int length);//必须传入数组和大小，必须是16的整数倍

private:
    unsigned char Sbox[256];
    unsigned char InvSbox[256];
    unsigned char w[11][4][4];

    void KeyExpansion(unsigned char* key, unsigned char w[][4][4]);//密钥扩展
    unsigned char FFmul(int a, unsigned char b);

    void SubBytes(unsigned char state[][4]);//字节替代变换
    void ShiftRows(unsigned char state[][4]);//行位移变换
    void MixColumns(unsigned char state[][4]);//列混淆变换
    void AddRoundKey(unsigned char state[][4], unsigned char k[][4]);//轮密钥加变换

    void InvSubBytes(unsigned char state[][4]);//逆字节替代
    void InvShiftRows(unsigned char state[][4]);//逆行位移
    void InvMixColumns(unsigned char state[][4]);//逆列混淆
};

#endif    // SRC_UTILS_AES_H