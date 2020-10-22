
/**********************************************************
* 版权所有(C)2002, 深圳市中兴通讯股份有限公司。
*
* 文件名称： md5.h
* 文件标识：
* 内容摘要： md5算法定义。
*
* 其它说明： 无
* 当前版本： 1.0
* 作者：     chen.yunbin
* 完成日期： 2002-07-02
*
* 修改记录1：
* 修改日期：
* 版本号：
* 修改人：
* 修改内容：
**********************************************************/

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
#if defined(__alpha)
typedef unsigned int UINT4;
#else
typedef unsigned int UINT4;
#endif

/* MD5.H - header file for MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

/* MD5 context. */
typedef struct
{
    UINT4 state[4];                                   /* state (ABCD) */
    UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

void MD5Init (MD5_CTX *context);
void MD5Update (MD5_CTX *context, unsigned char *input, unsigned int inputLen);
void MD5Final (unsigned char digest[16], MD5_CTX *context);
/* HMAC-MD5 */
extern void hmac_md5(unsigned char* text, int text_len, unsigned char* key, int key_len , unsigned char *poDigest);

extern void md5_calc(unsigned char *output, unsigned char *input, unsigned int inlen);

extern void md5_filecalc(unsigned char *output, char *filename);