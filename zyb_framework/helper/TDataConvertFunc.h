//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: TDataConvertFunc.cpp
// Author: Liu.QiQuan
// Date: 2005/04/12
// Description: 数据转换类
////////////////////////////////////////////////

#ifndef __T_DATA_CONVERT_FUNC_H__
#define __T_DATA_CONVERT_FUNC_H__

//有符号、无符号不匹配
#ifdef __OS_WINDOWS__
#pragma warning(disable:4018)
#endif

class TDataConvertFunc {
public:
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
    static void Bcd2AscNum(const unsigned char * Bcd,char * AscNum,int DestSize,unsigned char EndChar = 0xf,bool bBitFlag = true);
   
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
    static void Bcd2Str(unsigned char *source, char *dest, int Destlen, bool bcdsortFlag = true);
    
    /**********************************************************************
    *  功  能 ： // 将unsigned char 串为INT型数字
    * 输入参数： // Byte        ：输入的unsigned char 串
    *            // ByteSize    ：unsigned char 串的长度
    *                           ：如Byte = 0X12 23，则ByteSize = 2
    * 输出参数： // 无
    * 返 回 值： // int型
    ***********************************************************************/    
    static int Byte2Int(unsigned char * Byte, int ByteSize);
    
    /**********************************************************************
    *  功  能 ： // 将指定长度的Ascll串转换为字符串（即在最后一个字符后加结束符'\0'）
    * 输入参数： // Asc         ：输入的Ascll码串
    *            // AscLength   ：需转换的Ascll码的长度
    * 输出参数： // Str         ：转换后的字符串，以'\0'结尾
    * 返 回 值： // 无
    ***********************************************************************/
    static void Asc2Str(const unsigned char * Asc,char * Str,int AscLength);

    // *** BEGIN *** 16755 Fu.Changyou 2007-12-28 Added
    // 将16进制数用字符表示，比如0表示成字符'0'，1表示成字符‘1’
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
