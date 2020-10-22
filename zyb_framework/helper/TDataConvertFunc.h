//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TDataConvertFunc.cpp
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: ����ת����
////////////////////////////////////////////////

#ifndef __T_DATA_CONVERT_FUNC_H__
#define __T_DATA_CONVERT_FUNC_H__

//�з��š��޷��Ų�ƥ��
#ifdef __OS_WINDOWS__
#pragma warning(disable:4018)
#endif

class TDataConvertFunc {
public:
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
    static void Bcd2AscNum(const unsigned char * Bcd,char * AscNum,int DestSize,unsigned char EndChar = 0xf,bool bBitFlag = true);
   
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
    static void Bcd2Str(unsigned char *source, char *dest, int Destlen, bool bcdsortFlag = true);
    
    /**********************************************************************
    *  ��  �� �� // ��unsigned char ��ΪINT������
    * ��������� // Byte        �������unsigned char ��
    *            // ByteSize    ��unsigned char ���ĳ���
    *                           ����Byte = 0X12 23����ByteSize = 2
    * ��������� // ��
    * �� �� ֵ�� // int��
    ***********************************************************************/    
    static int Byte2Int(unsigned char * Byte, int ByteSize);
    
    /**********************************************************************
    *  ��  �� �� // ��ָ�����ȵ�Ascll��ת��Ϊ�ַ������������һ���ַ���ӽ�����'\0'��
    * ��������� // Asc         �������Ascll�봮
    *            // AscLength   ����ת����Ascll��ĳ���
    * ��������� // Str         ��ת������ַ�������'\0'��β
    * �� �� ֵ�� // ��
    ***********************************************************************/
    static void Asc2Str(const unsigned char * Asc,char * Str,int AscLength);

    // *** BEGIN *** 16755 Fu.Changyou 2007-12-28 Added
    // ��16���������ַ���ʾ������0��ʾ���ַ�'0'��1��ʾ���ַ���1��
    inline static char Hex2Char(int iHex)
    {
        if (iHex >= 0 && iHex <= 9)
        {
            return (char)('0' + iHex);//ZMP:477242
        }

        if (iHex >= 10 && iHex <= 15)
        {
            return (char)('A' + iHex - 10);
        }

        return (char)iHex;
    }
    // *** END *** 16755 Fu.Changyou 2007-12-28 added

};

#define FUNC_DC TDataConvertFunc

#endif //__T_STRING_FUNC_H__
