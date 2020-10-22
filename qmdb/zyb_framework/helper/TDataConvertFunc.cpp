//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TDataConvertFunc.h
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: ����ת����
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
*  ��  �� �� // ��BCD��ת��Ϊ�����ַ��������ж��ڴ���0x9��BCD�ַ�ȫ����'0'����
* ��������� // Bcd         �������BCD�봮
*            // DestSize    ��ת������ַ�������󳤶ȣ��������һ��'\0'����
*                           ����Bcd = 0X12 23����DestSize = min(2*2+1,AscNum��size)
*            // EndChar     ����BCD�봮����ֹ�룬Ĭ��Ϊ0xff����ʾ����ֹ��
*            // bBitFlag    ��Ϊtrue��ʾ������0X0102ת��Ϊ"0102"��
*                           ��Ϊfalse��ʾ������0X0102ת��Ϊ"1020"��
*                           ��Ĭ��Ϊtrue
* ��������� // AscNum  ��ת������ַ�������'\0'��β
* �� �� ֵ�� // ��
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
        //�õ���λ����
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

        //�õ���λ������
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
*  ��  �� �� // ��ָ�����ȵ�Ascll��ת��Ϊ�ַ������������һ���ַ���ӽ�����'\0'��
* ��������� // Asc         �������Ascll�봮
*            // AscLength   ����ת����Ascll��ĳ���
* ��������� // Str         ��ת������ַ�������'\0'��β
* �� �� ֵ�� // ��
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
*  ��  �� �� // ��BCD��ת��Ϊ�����ַ��������ж��ڴ���0x9��BCD�ַ�0XA..0XFת��Ϊ��A'...'F'����
* ��������� // Bcd         �������BCD�봮
*            // DestSize    ��ת������ַ�������󳤶ȣ��������һ��'\0'����
*                           ����Bcd = 0X12 23����DestSize = min(2*2+1,AscNum��size)
*            // bBitFlag    ��Ϊtrue��ʾ������0X0102ת��Ϊ"0102"��
*                           ��Ϊfalse��ʾ������0X0102ת��Ϊ"1020"��
*                           ��Ĭ��Ϊtrue
* ��������� // dest   ��ת������ַ�������'\0'��β
* �� �� ֵ�� // ��
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
*  ��  �� �� // ��unsigned char ��ΪINT������
* ��������� // Byte        �������unsigned char ��
*            // ByteSize    ��unsigned char ���ĳ���
*                           ����Byte = 0X12 23����ByteSize = 2
* ��������� // ��
* �� �� ֵ�� // int��
***********************************************************************/ 
int TDataConvertFunc::Byte2Int(unsigned char * Byte, int ByteSize)
{
    if (ByteSize > (int)sizeof(int)) //���������int���͵����ֵ
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
