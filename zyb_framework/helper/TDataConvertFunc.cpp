//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TDataConvertFunc.h
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: 数据转换类
////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>
#include "TDataConvertFunc.h"
#include "debug_new.h"
#include "TException.h"

/**********************************************************************
*  功  能 ： // 将BCD码转换为数字字符串，其中对于大于0x9的BCD字符全部以'0'代替
* 输入参数： // Bcd         ：输入的BCD码串
*            // DestSize    ：转换后的字符串的最大长度，包括最后一个'\0'在内
*                           ：如Bcd = 0X12 23，则DestSize = min(2*2+1,AscNum的size)
*            // EndChar     ：该BCD码串的终止码，默认为0xff，表示无终止码
*            // bBitFlag    ：为true表示正序（如0X0102转换为"0102"）
*                           ：为false表示逆序（如0X0102转换为"1020"）
*                           ：默认为true
* 输出参数： // AscNum  ：转换后的字符串，以'\0'结尾
* 返 回 值： // 无
***********************************************************************/
void TDataConvertFunc::Bcd2AscNum(const unsigned char * Bcd,char * AscNum,int DestSize,unsigned char EndChar,bool bBitFlag)
{
    int i;
    unsigned char s[500]={0};
    unsigned char temp;
	#ifdef _ASSERT_PARAM
	if( NULL==AscNum )
	{
		throw TException("TDataConvertFunc::Bcd2AscNum():  Parameter AscNum can`t be NULL" );
	}
	#endif
	
    if((DestSize/2) > 500)
    {
        return;
    }

    for(i=0;i<int(DestSize/2) ; i++)
    {
        s[i]=Bcd[i];
        if(!bBitFlag)
        {
            temp=(unsigned char)((s[i]>>4)|(s[i]<<4));//ZMP:477242
            s[i]=temp;
        }
        //得到高位数字
        temp=(unsigned char)((s[i]>>4)&0x0f);
        if (temp == EndChar)
        {
            AscNum[2*i] = '\0';
            break;
        }
        if(temp<=9)
            AscNum[2*i]=(char)(temp+'0');/*'0' '1'...*/
        else
            AscNum[2*i]='0';

        //得到低位的数字
        temp=(unsigned char)(s[i]&0x0f);
        if (temp == EndChar)
        {
            AscNum[2*i+1] = '\0';
            break;
        }
        if(temp<=9)
            AscNum[2*i+1]=(char)(temp+'0');
        else
            AscNum[2*i+1]='0';
    }
    AscNum[DestSize-1]='\0';
}

/**********************************************************************
*  功  能 ： // 将指定长度的Ascll串转换为字符串（即在最后一个字符后加结束符'\0'）
* 输入参数： // Asc         ：输入的Ascll码串
*            // AscLength   ：需转换的Ascll码的长度
* 输出参数： // Str         ：转换后的字符串，以'\0'结尾
* 返 回 值： // 无
***********************************************************************/
void TDataConvertFunc::Asc2Str(const unsigned char * Asc,char * Str,int AscLength)
{
    #ifdef _ASSERT_PARAM
	if( NULL==Str )
	{
		throw TException("TDataConvertFunc::Asc2Str():  Parameter Str can`t be NULL");
	}
	#endif
	memcpy(Str,Asc,(size_t)AscLength);
    Str[AscLength] = '\0';
}

/**********************************************************************
*  功  能 ： // 将BCD码转换为数字字符串，其中对于大于0x9的BCD字符0XA..0XF转换为‘A'...'F'代替
* 输入参数： // Bcd         ：输入的BCD码串
*            // DestSize    ：转换后的字符串的最大长度，包括最后一个'\0'在内
*                           ：如Bcd = 0X12 23，则DestSize = min(2*2+1,AscNum的size)
*            // bBitFlag    ：为true表示正序（如0X0102转换为"0102"）
*                           ：为false表示逆序（如0X0102转换为"1020"）
*                           ：默认为true
* 输出参数： // dest   ：转换后的字符串，以'\0'结尾
* 返 回 值： // 无
***********************************************************************/
void TDataConvertFunc::Bcd2Str(unsigned char *source, char *dest, int Destlen, bool bcdsortFlag /* = true*/)
{
    int i;
    unsigned char temp;
    // *** BEGIN *** 16755 Fu.Changyou 2007-12-28 modified
    unsigned char s = 0;
	#ifdef _ASSERT_PARAM
    if( NULL==dest )
    {
		throw TException("TDataConvertFunc::Bcd2Str():  Parameter dest can`t be NULL");
    }
    #endif

    //unsigned char s[100]={0};
    //if((Destlen/2) > 100 )
    //{
    //    return ;
    //}

    int iHalfLen = Destlen / 2;

    //for(i=0;i<(Destlen/2);i++)
    for (i = 0; i < iHalfLen; i++)
    {
        //s[i]=source[i];
        s = source[i];
        if(!bcdsortFlag)
        {
            //temp=(s[i]>>4)|(s[i]<<4);
            //s[i]=temp;
            temp = (unsigned char)((s >> 4) | (s << 4));//ZMP:477242
            s = temp;
        }
        temp = (unsigned char)((s >> 4) & 0x0f);
        dest[2 * i] = Hex2Char(temp);
        temp = (unsigned char)(s & 0x0f);
        dest[2 * i + 1] = Hex2Char(temp);
        //temp=(s[i]>>4)&0x0f;
        /*                if(temp == 0xe) 
        {
        dest[2*i]='\0';
        return;
        } */
        //if(temp<=9)
        //    dest[2*i]=temp+'0';/*'0' '1'...*/
        //else
        //    dest[2*i]=temp+'0'+7;/*'A' 'B'...*/

        //if(2*i>Destlen)
        //    break;

        //temp=s[i]&0x0f;
        /*                if(temp == 0xe)
        {
        dest[2*i+1]='\0';
        return;
        }*/
        //if(temp<=9)
        //    dest[2*i+1]=temp+'0';
        //else
        //    dest[2*i+1]=temp+'0'+7;
    }
    // *** END *** 16755 Fu.Changyou 2007-12-28 modified
    dest[Destlen-1]='\0';
}

/**********************************************************************
*  功  能 ： // 将unsigned char 串为INT型数字
* 输入参数： // Byte        ：输入的unsigned char 串
*            // ByteSize    ：unsigned char 串的长度
*                           ：如Byte = 0X12 23，则ByteSize = 2
* 输出参数： // 无
* 返 回 值： // int型
***********************************************************************/ 
int TDataConvertFunc::Byte2Int(unsigned char * Byte, int ByteSize)
{
    if (ByteSize > (int)sizeof(int)) //如果超过了int类型的最大值
    {
        return -1;
    }
    int result = 0;
    for (int i=0;i<ByteSize;++i)
    {
        result = result * 256 + int(*Byte);
        ++Byte;
    }
    return result;
}
