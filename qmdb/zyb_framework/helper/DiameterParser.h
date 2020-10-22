//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: DiameterPareser.h
// Author: 张宗放
// Date: 2007/03/01
// Description: OCS在线计费消息动态配置接口
////////////////////////////////////////////////
#ifndef __T_DMTETER_PARSER_H_
#define __T_DMTETER_PARSER_H_

#include "Common.h"
#include "TSplit.h"

#define CONFIG_NUMBER 19
#define CONFIG_NUMBER_5G 21
#define OPER_CONFIG_NUMBER 5

//确定协议中的位序是UNIX描述(参照基础协议).位序和字节序保持一致
#define VM_BODY_FLAG 0XC0
#define MP_BODY_FLAG 0X60
#define VP_BODY_FLAG 0XA0
#define VMP_BODY_FLAG 0XE0

#define V_BODY_FLAG 0X80
#define M_BODY_FLAG 0X40
#define P_BODY_FLAG 0X20
#define RESERVE_BODY_FLAG 0X1F

#define R_HEAD_FLAG 0X80
#define P_HEAD_FLAG 0X40
#define E_HEAD_FLAG 0X20
#define T_HEAD_FLAG 0X10
#define RESERVE_HEAD_FLAG 0X0F

#define AVP_TYPE_GROUP  0
#define AVP_TYPE_INT    1
#define AVP_TYPE_UINT   2
#define AVP_TYPE_LONG   3
#define AVP_TYPE_ULONG  4
#define AVP_TYPE_CHAR   5
#define AVP_TYPE_ADRESS 6
#define AVP_TYPE_MEM    7
#define AVP_TYPE_ERROR  9

//#define log2(x) (log10(x)/log10(2.0))

#ifndef _COUNTERLLONG_
#define _COUNTERLLONG_
struct counterllong {
    unsigned int high;
    unsigned int low;
};
#endif

class TSplit;
//temp//class TRatableEvent;
class THashList;
class TBaseList;
class TAvpInfo;
class TBaseListItem;
class TAvpNode;
class TAvpEncode;
class TAvpParser;
class TAvpOperItem;
class TAvpOperStruct;
class TDmtOper;
class TAvpSingle;
//TAvpInfo---解析组包配置文件avpfile.ini的数据项结构类
class TAvpInfo : public TBaseObject {
public:
    TAvpInfo();
    virtual ~TAvpInfo();
public:
    char AvpKey[128];//HASH的键值
    unsigned int AttrId;//avp字段对应于TRatableEvent的attrid
    unsigned int AvpType;//数据类型
    char         AvpName[128];//avp的名字
    unsigned int AvpCode;//avp的code
    unsigned int VendorId;//avp的vendorid
    unsigned int Index;//avp出现的次数标识
    unsigned int HashCode;//avp字段在HASH链中的HashCode
    bool IsGroup;//avp在规范中的结构类型
    int AvpLevel;//avp在规范中的层次位置
    bool bIsOctetString; // 是否是OctetString型, ZMP:487585

    //组包需要使用的配置选项
    bool bMustFlag;//标志M的配置选项
    bool bNoEncode[16][4];//非编码强制位
    bool bOption[16][4];//编码强制位
    char cDefaultValue[128];//默认编码的数据值
    int  AvpLenLoc;//本avp在消息缓存中的长度位置
    bool bHasDefVal;//ZMP:491230,表示是否有默认值，true有默认值，false没有
public:
    TAvpInfo *ParentAvpInfo;//父节点指针
    TBaseListItem *SubAvpItem;//孩子节点首指针
    TAvpNode *AvpNode;//(组包所使用的)对应数据avp实体指针
public:
    virtual int Compare(int iKey) const;
    virtual int Compare(int iKey1, int iKey2) const;
    virtual int Compare(const TBaseObject *pObject) const;
    virtual string ToString();//输出对象信息
};
//TAvpFile---解析组包配置文件avpfile.ini的数据类
class TAvpFile : public TBaseObject {
public:
    TAvpFile();
    virtual ~TAvpFile();
public:    
    //取KEY_INFO对应的配置信息
    TAvpInfo * GetAvpInfo(const char *sKey);  
    //解析配置文件的一行输入信息到TAvpInfo中
    TAvpInfo * GetAvpInfoByText(const char *sLineText,const int iLineLength);
    //获取avpfile第一个配置项的节点
    TAvpInfo * GetAvpHead(void);
    //读取配置文件avpfile.ini的外部接口
    void ReadConfig(const char *sFileName);
    //清除所有的配置项信息,实clear,清除HASH链,释放内存
    void Clear(void);
    void ClearData(TAvpInfo *pAvpInfo = NULL);
    //获取数据类型字符串标识对应的内部码
    unsigned int GetAvpType(const char *sAvpTypeStr);
    //打印配置文件信息avpfile.ini,默认从头节点m_AvpHead开始打印
    void PrintAvpFile(TAvpInfo *pAvpInfo=NULL);
private:
    //释放一个节点及其子节点
    void ClearAvpNode(TAvpInfo *pAvpInfo);
private:
    THashList *m_pHashKey;//HASH链
    TBaseList *m_pListCtrl;//链表控制类
    TAvpInfo m_AvpHead;//对应配置文件的第一个配置项(虚节点)
    TAvpInfo m_tAvpInfo;//HASH查找中使用的临时变量
    TSplit m_Split;//解析avpfile使用的分割工具类
};
//TAvpHead---OCP协议消息头对应的数据实体类
class TAvpHead : public TBaseObject { 
public:
    int  Version;
    unsigned int  Length;
    int  CommandCode;
    char CommandFlag;
    int  AppId;
    unsigned int  HopId;
    unsigned int  EndId;
public:
    TAvpHead();
    virtual void ClearData(void);
    virtual void Assign(TBaseObject *pObject);
};
//TAvpNode---OCP协议消息中AVP数据项对应的实体类
class TAvpNode : public TRecordData { 
public:
    //常规包含AvpFile所用到的数据项
    unsigned int AvpCode;
    unsigned int VendorId;
    bool bHaveVendorId;
    unsigned int AvpIndex;

    char AvpKey[128];//AVP的路径(AvpNode1,VerderId1,Index1|...|AvpNoden,VerderIdn,Indexn)

    unsigned char *AvpNode;//avp结构在消息缓存中的位置
    unsigned int AvpSize;//avp的大小

    unsigned char * AvpNodeData;//avp的数据在消息缓存中的位置
    unsigned int AvpDataSize;//avp数据的大小

    TAvpNode *ParentNode;//父节点
    TAvpNode *SubNode;//孩子首节点
    TAvpNode *SubTailNode;//孩子首节点
    TAvpNode *NextNode;//兄弟节点
    TAvpInfo *AvpInfo;//对应的配置项信息

    //DmtStruct解析和编码所用到的数据项
    unsigned char AvpFlag;//avp结构中的标志字节
    unsigned int AvpType;//avp的数据类型
    char DefaultValue[512];//avp的默认编码值
    bool AvpGroup;//avp是否结构类型
    int AvpLenLoc;//(组包时使用)avp编码时长度在缓存中的位置
public:
    TAvpNode();
public:
    virtual int Compare(int iKey) const;
    virtual int Compare(int iKey1, int iKey2) const;
    virtual int Compare(const TBaseObject *pObject) const;
    virtual void ClearData(void);
    void SetValue(const char *sValue);
    void SetValue(const char *sValue,int iSize);
    const char * AsValue(void) const;
    llong AsInteger();     //返回属性的整形值
    char *AsString();      //返回属性的字符串型值
    string AsOctetString();  //返回OctetString型字符串的16进制码,ZMP:487585
public:
    char m_sValue[256];
    char m_tTempValue[256];
    unsigned int HashCode;//DmtOper类修改对应TDiamterInfo中avp时使用
};
//TDiamterInfo---OCP消息实体类
class TDiamterInfo : public TRecordData{
public:
    TDiamterInfo(unsigned int iAvpListSize=1024);
    virtual ~TDiamterInfo();
public:
    virtual void ClearData(void);
public:
    TAvpHead *AvpHead;//消息头
    TAvpNode **AvpNodes;//消息体的信息
    TAvpNode AvpNodeHead;//消息体根结点
    unsigned int AvpNodeSize;//消息体大小(*AvpNodes数组大小)
    unsigned int AvpListSize;//消息链表构造大小
};
//TAvpOperItem---业务测试配置文件avpoper.ini的数据项结构类
class TAvpOperItem : public TBaseObject
{
public:
    TAvpOperItem();
     virtual ~TAvpOperItem();
    virtual int Compare(int iKey) const;
    virtual int Compare(int iKey1, int iKey2) const;
    virtual int Compare(const TBaseObject *pObject) const;
    virtual string ToString();//输出对象信息
public:
    char Avpkey[128];//该业务字段对应于规范中的键值
    char AttrName[64];//业务字段名
    unsigned int AttrId;//对应在TRatableEvent的属性ID,默认使用avpfile.ini
    unsigned char AvpFlag;//组包时需要配置的标志,默认使用avpfile.ini
    char DefaultValue[128];//默认值,默认使用avpfile.ini
    unsigned int HashCode;//TAvpOperItem在HASH链中的HashCode
};
//TAvpOperStruct---业务测试配置文件avpoper.ini的数据类
class TAvpOperStruct : public TBaseObject
{
public:
    TAvpOperStruct();
    virtual ~TAvpOperStruct();
    //virtual int Compare(const TBaseObject *pObject) const;
    //virtual string ToString();//输出对象信息
public:
    //读取配置文件avpoper.ini的外部接口
    void ReadConfig(const char *sFileName);
    //取KEY_INFO对应的配置信息
    TAvpOperItem * GetAvpOperItem (const char *sKey); 
    //修改KEY_INFO对应的值
    bool SetItemValue(const char *sKey,const char *newVlaue);

    bool SetSessionId(const char *newVlaue);
    bool SetCCRType(const char *newVlaue);
    bool SetCCRNumber(const char *newVlaue);
    bool SetCallingNbr(const char *newVlaue);
    bool SetCallednbr(const char *newVlaue);
public:
    THashList *m_pHashKey;
    TSplit m_Split;
    TAvpOperItem *m_pAvpOperItem;
    TAvpOperItem m_tAvpOperItem;
    int m_iIndex;
};
//TDmtOper---业务解析-组包控制类
class TDmtOper :public TBaseObject{
friend class TDiameterTransForm;
public:
    TDmtOper();
    virtual ~TDmtOper(void);
    /**********************************************************************
    函   数: void ReadConfig(TAvpFile * pAvpfile,TAvpOperStruct * pOperStruct);
    功   能: 切换配置文件信息，avpfile.ini和avpoper.ini
    输入参数: TAvpFile * pAvpfile---avpfile.ini配置文件类
    TAvpOperStruct * pOperStruct---avpoper.ini配置文件类
    返回结果: 无
    ************************************************************************/
    void ReadConfig(TAvpFile * pAvpfile,TAvpOperStruct * pAvpStruct,bool NeedOperStuct = false);
    /**********************************************************************
    函   数: void ReadConfig(const char *sFileName,const char *sStrFileName);
    功   能: 读取配置文件信息，avpfile.ini和avpoper.ini
    输入参数: const char *sFileName---avpfile.ini配置文件位置
              const char *sStrFileName---avpoper.ini配置文件位置
    返回结果: 无
    ************************************************************************/
    void ReadConfig(const char *sFileName,const char *sStrFileName,bool NeedOperStuct = false);
    /**********************************************************************
    函   数: void SetDmtSerType(int iSerType);
    功   能: 设置消息的业务类型
    输入参数: int iSerType---消息的业务类型
    返回结果: 无
    ************************************************************************/
    void SetDmtSerType(int iSerType,int iCcrType);
    /**********************************************************************
    函   数: void SetMsgHeadInfo(TAvpHead *pInfo);
    功   能: 设置消息头信息
    输入参数: TAvpHead *pInfo---消息头数据结构
    返回结果: 无
    ************************************************************************/
    void SetMsgHeadInfo(TAvpHead *pInfo);
    /**********************************************************************
    函   数: bool DmtEncode(unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize);
    功   能: 消息组包的外部接口
    输入参数: unsigned char *pDataBuffer---转换后的数据首地址
              unsigned int iDataSize---存储转换后的数据的最大长度   
              int *iRealSize----转换后的数据的实际长度
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool DmtEncode(unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize);
    /**********************************************************************
    函   数: bool DmtEncode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize);
    功   能: 消息组包的外部接口
    输入参数: TRatableEvent *pEvent---公有数据AVP源
              unsigned char *pDataBuffer---转换后的数据首地址
              unsigned int iDataSize---存储转换后的数据的最大长度   
              int *iRealSize----转换后的数据的实际长度
    返回结果: true---成功,false---失败
    ************************************************************************/
    //del//bool DmtEncode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize);
    void DmtPrepareEncode();
    /**********************************************************************
    函   数: bool SetItemValue(const char *sKey,const char *newVlaue);
    功   能: 设置KEY_INFO对应的值（包括增加新节点），现在的设计是可以动态增加AVP编码信息
    输入参数: const char *sKey---业务字段的键值
              const char *newVlaue---新值   
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool SetItemValue(const char *sKey,const char *newVlaue);
    /**********************************************************************
    函   数: bool ModifyItemValue(const char *sKey,const char *newVlaue);
    功   能: 修改KEY_INFO对应的值，现在的设计是性能测试时动态修改对应avp节点的数据
    输入参数: const char *sKey---业务字段的键值
              const char *newVlaue---新值   
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool ModifyItemValue(const char *sKey,const char *newVlaue);
    /**********************************************************************
    函   数: bool DmtParser(unsigned char *pDataBuffer,unsigned int iDataSize);
    功   能: 消息解析的外部接口
    输入参数: unsigned char *pDataBuffer---消息数据首地址
              unsigned int iDataSize---解析数据的最大长度
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool DmtParser(unsigned char *pDataBuffer,unsigned int iDataSize);
    /**********************************************************************
    函   数: TAvpNode * GetAvpNode(const char *sKey);
    功   能: 修改KEY_INFO对应的Avp数据结构
    输入参数: const char *sKey---业务字段的键值
    返回结果: TAvpNode * 的数据指针
    ************************************************************************/
    TAvpNode * GetAvpNode(const char *sKey);    
    /**********************************************************************
    函   数: bool SetRatableEvent(TRatableEvent *pRatableEvent); 
    功   能: 解析后转换到TRatableEvent,现在的设计是只设置公共的业务字段
    输入参数: TRatableEvent *pRatableEvent---转换后的数据结构
    返回结果: true---成功,false---失败
    ************************************************************************/
    //del//bool SetRatableEvent(TRatableEvent *pRatableEvent); 
    /**********************************************************************
    函   数: TDiamterInfo * GetDiamterInfo(void); 
    功   能: 得到解析后的消息结构
    输入参数: 无
    返回结果: （TDiamterInfo * ）的数据
    ************************************************************************/
    TDiamterInfo * GetDiamterInfo(void);
    /**********************************************************************
    函   数: bool SetDmtSrcData(unsigned char *pAvp,int iOrgDataLen);
    功   能: 单步解析类设置原始数据
    输入参数: unsigned char *pDataBuffer---消息数据首地址
              int iOrgDataLen---解析数据的最大长度
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool SetDmtSrcData(unsigned char *pAvp,int iOrgDataLen);
    /**********************************************************************
    函   数: bool GetAvpByKey(const char *cFinding,unsigned char *pDataBuffer,int size,int *iDataSize,int *iDataType);
    功   能: 获取cFinding指定的AVP
    输入参数: const char *cFinding---查询条件key
              unsigned char *pDataBuffer---AVP的原始数据
              int size ---缓存所允许的大小
              int *iDataSize ---数据大小
              int *iDataType ---数据类型
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool GetAvpByKey(const char *cFinding,unsigned char *pDataBuffer,int size,int *iDataSize,int *iDataType);
    virtual string ToString();
private:
    bool Cnvt2DmtStruct();
    bool GetEncodeFlag(TAvpOperStruct *pAvpStruct,TAvpInfo *pAvpInfo);
    TAvpNode * SetDmtNode(TAvpNode *pParentNode,TAvpNode *pNode,TAvpInfo *pSrcData);
private:
    //THashList *m_pHashKey;//动态修改组包数据类（本层）TDiamterInfo的HASH，在组装这个数据时生成
    TBaseList *m_pListCtrl;//链表控制类
    TAvpSingle *m_pAvpSingle;//单个AVP解析类
    TAvpParser *m_pAvpParser;//消息码流到TDiamterInfo的操作类
    TAvpEncode *m_pAvpEncode;//TDiamterInfo到消息码流的操作类
    TDiamterInfo *m_pDmtData;//ocp消息对应的实体类(这里是配置文件到消息码流的中间结构)
    int m_iIndex;//TDiamterInfo的节点下标索引
    TAvpFile *m_pAvpFile;//配置文件存储信息 
    TAvpOperStruct *m_pAvpStruct;//业务测试配置转换结构
    int m_iServiceType;//存储转换时的业务子类型
    int m_iCcrType;//业务请求类型
    bool m_bHasFresh;//是否有新增节点标志，为重新组装数据使用
    TAvpNode *m_pConNode[256];//组包接口动态设置AVP时使用的（配合TRatableEvent静态组包）
    TAvpNode m_tAvpNode;
    int m_iConIndex;
    TSplit Split;
    bool m_bCloneConfig;//配置信息拷贝类型
};

//TAvpParser---配置文件（avpfile.ini）解析类
class TAvpParser : public TBaseObject{
friend class TDiameterTransForm;
public:
    //转换配置文件信息由类TAvpFile传入,由avpNode根据key值查找对应的转换信息
    TAvpParser(TAvpFile *pAvpFile,unsigned int iAvpListSize = 1024);
    virtual ~TAvpParser(void);
public:
    /**********************************************************************
    函   数: bool Parser(unsigned char *pDataBuffer,unsigned int iDataSize);
    功   能: 原始消息解析函数
    输入参数: unsigned char *pDataBuffer---消息首地址
              unsigned int iDataSize    ---消息长度
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool Parser(unsigned char *pDataBuffer,unsigned int iDataSize);
    /**********************************************************************
    函   数: bool SetRatableEvent(TRatableEvent *pRatableEvent); 
    功   能: 解析后转换到TRatableEvent
    输入参数: TRatableEvent *pRatableEvent---转换后的数据结构
    返回结果: true---成功,false---失败
    ************************************************************************/
    //del//virtual bool SetRatableEvent(TRatableEvent *pRatableEvent); 
    /**********************************************************************
    函   数: TDiamterInfo * GetDiamterInfo(void); 
    功   能: 得到解析后的消息结构
    输入参数: 无
    返回结果: （TDiamterInfo * ）的数据
    ************************************************************************/
    TDiamterInfo * GetDiamterInfo(void);
    /**********************************************************************
    函   数: TAvpNode * GetAvpNodeInfo(const char *sKey);
    功   能: 修改KEY_INFO对应的值
    输入参数: const char *sKey---业务字段的键值
    返回结果: true---成功,false---失败
    ************************************************************************/
    TAvpNode * GetAvpNodeInfo(const char *sKey);
    /**********************************************************************
    函   数: string ToString();
    功   能: 输出对象信息
    输入参数: 无
    返回结果: （string）的数据
    ************************************************************************/
    virtual string ToString();
    unsigned int GetErrorCode(void);
    char * GetErrorInfo(void);
    void SetConfigInfo(TAvpFile *pAvpFile);
private:
    /**********************************************************************
    函   数: bool ParserHead(unsigned char *pDataBuffer,unsigned int iDataSize);
    功   能: 解析消息头信息
    输入参数: unsigned char *pDataBuffer －－消息头首地址
              unsigned int iDataSize－－消息长度
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool ParserHead(unsigned char *pDataBuffer,unsigned int iDataSize);
    /**********************************************************************
    函   数: bool ParserNode(TAvpNode *pParentNode,unsigned char *pDataBuffer,unsigned int iDataSize);
    功   能: 输出对象信息
    输入参数: TAvpNode *pParentNode－－父结点数据结构
              unsigned char *pDataBuffer－－结点首地址
              unsigned int iDataSize－－结点数据长度
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool ParserNode(TAvpNode *pParentNode,unsigned char *pDataBuffer,unsigned int iDataSize);
    /**********************************************************************
    函   数: TAvpNode * GetNodeInfo(TAvpNode *pParentNode,unsigned char *pDataBuffer,unsigned int iDataSize); 
    功   能: 具体AVP结点的解析，包括存储到HASH链后的KEY值的获取
    输入参数: TAvpNode *pParentNode－－父结点数据结构
              unsigned char *pDataBuffer－－结点首地址
              unsigned int iDataSize－－结点数据长度
    返回结果: （TAvpNode *）的数据
    ************************************************************************/
    TAvpNode * GetNodeInfo(TAvpNode *pParentNode,unsigned char *pDataBuffer,unsigned int iDataSize);    
private:
    unsigned int GetAvpNode(unsigned char *pDataBuffer);
    unsigned int GetVendorId(unsigned char *pDataBuffer);
    bool HaveVendorId(unsigned char *pDataBuffer);
    unsigned int GetAvpSize(unsigned char *pDataBuffer);
    string GetBobyString(TAvpInfo *pCurrAvpInfo);
    bool GetAvpValue(TAvpNode *pAvpNode);
public:
    //下面是解析所用的函数,基本类型解析函数
    static void CnvtByteToStr(unsigned char *Data,int iLength,char *pcOut);
    static void CnvtBinToDec(unsigned char *Data,int iLength,int *piOut);
    static void CnvtBinToDecStr(unsigned char *Data,int iLength,char *pcOut);
    static void CnvtUintToDec(unsigned char *Data,int iLength,unsigned int *piOut);
    static void CnvtUllongBinToDec(unsigned char *Data,int iLength,unsigned long long*plOut);
    static void CnvtLlongBinToDec(unsigned char *Data,int iLength,long long *plOut);
    static void CnvtDmtIPToIpv(unsigned char *Data,int iLength,char *pcOut);
    static int log2(int iNumber);
    static int IntToStrNum(int iNumber,char *sOut);
    static int IntToStrNum(long long iNumber,char *sOut);
private:
    TDiamterInfo *m_pDiamterInfo;//解析后的解析结果
    unsigned char m_pDataBuffer[4096];//保存解析的原始消息
    int m_iDataSize;//保存解析的原始消息的长度    
    unsigned int m_iNodeIndex;//临时变量，指向一个空的NODE结点
    TAvpFile *m_pAvpFile;//配置文件存储信息
private:
    int m_iErrorCode;
    char m_sErrorMsg[128];
};
//TAvpEncode---配置文件（avpfile.ini）组包类
class TAvpEncode : public TBaseObject{
friend class TDiameterTransForm;
public:
    TAvpEncode(TAvpFile *pAvpFile);
    virtual ~TAvpEncode();
public:
    /**********************************************************************
    函   数: bool Encode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize,int iServType = 0);
    功   能: DIAMETER组包函数
    输入参数: TRatableEvent *pEvent---需要转换的数据结构
              unsigned char *pDataBuffer---转换后的数据首地址
              unsigned int iDataSize---存储转换后的数据的最大长度   
              int *iRealSize----转换后的数据的实际长度
              int iServType = 0---转换的消息业务子类型（具体的参照配置文件）
    返回结果: true---成功,false---失败
    ************************************************************************/
    //del//bool Encode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize,int iServType = 0);
    /**********************************************************************
    函   数: bool Encode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize,int iServType = 0);
    功   能: DIAMETER组包函数
    输入参数: TDiamterInfo *pInfo---需要转换的数据结构
              unsigned char *pDataBuffer---转换后的数据首地址
              unsigned int iDataSize---存储转换后的数据的最大长度   
              int *iRealSize----转换后的数据的实际长度
              int iServType = 0---转换的消息业务子类型（具体的参照配置文件）
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool Encode(TDiamterInfo *pInfo,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize,int iServType = 0);
    /**********************************************************************
    函   数: string ToString();
    功   能: 输出对象信息
    输入参数: 无
    返回结果: （string）的数据
    ************************************************************************/
    virtual string ToString(void);
    /**********************************************************************
    函   数: void SetMsgHeadInfo(TAvpHead *pInfo);
    功   能: 设置消息头数据信息
    输入参数: TAvpHead *pInfo---需要处理的数据结构
    返回结果: 无
    ************************************************************************/
    void SetMsgHeadInfo(TAvpHead *pInfo);
    /**********************************************************************
    函   数: bool GetEncodeFlag(TAvpInfo *pInfo,bool IsOperStr = false);
    功   能: 获取AVP 结点编码标志
    输入参数: TAvpInfo *pInfo---需要处理的数据结构
              bool IsOperStr = false --- 在业务测试结构组包时使用，不从TRatableEvent中参照
              bool IsEncodeView = true --- 在组包前的判断和组包进行时的区分，可选标志在结构体中的传递性
    返回结果: true---成功,false---失败
    ************************************************************************/
    bool GetEncodeFlag(TAvpInfo *pInfo,bool IsOperStr = false,bool IsEncodeView = true);
    /**********************************************************************
    函   数: void SetMsgSerType(int iServiceType);
    功   能: 设置消息头数据信息
    输入参数: int iServiceType---需要处理的业务类型
    返回结果: 无
    ************************************************************************/
    void SetMsgSerType(int iServiceType,int iCcrType);
    /**********************************************************************
    函   数: void SetDmtData(THashList *m_pDmtHash);
    功   能: 设置组包消息结构
    输入参数: THashList *m_pDmtHash---需要设置的消息结构HASH链
    返回结果: 无
    ************************************************************************/
    void SetDmtHash(THashList *pDmtHash);
    void SetConfigInfo(TAvpFile *pAvpFile);
    static int StrToByte(unsigned char *psOut,const char *psSrc);    
private:
    /**********************************************************************
    函   数: bool EncodeMsgHead(TAvpHead *pHead);
    功   能: 消息头编码函数
    输入参数: TAvpHead *pHead---需要转换的数据结构
    返回结果: true---成功,false---失败
    ************************************************************************/
    static bool EncodeMsgHead(TAvpHead *pHead,unsigned char * mp_pDataBuffer,int &mp_iDataIndex);
    /**********************************************************************
    函   数: int EncodeNode(TAvpInfo *pInfo);
    功   能: AVP数据编码函数
    输入参数: TAvpInfo *pInfo---需要转换的数据结构
    返回结果: 返回AVP编码后的数据长度
    ************************************************************************/
    //del//int EncodeNode(TAvpInfo *pInfo);
    /**********************************************************************
    函   数: int EncodeNode(TAvpNode *pInfo);
    功   能: AVP数据编码函数
    输入参数: TAvpInfo *pInfo---需要转换的数据结构
    返回结果: 返回AVP编码后的数据长度
    ************************************************************************/
    int EncodeNode(TAvpNode *pInfo,unsigned char * mp_pDataBuffer,int &mp_iDataIndex);
    /**********************************************************************
    函   数: int EncodeNode(TAvpNode *pInfo,TAvpNode *pNode);
    功   能: AVP数据编码函数
    输入参数: TAvpInfo *pInfo---需要转换的数据结构
              TAvpNode *pNode---需要转换的数据结构
    返回结果: 返回AVP编码后的数据长度
    ************************************************************************/
    //del//int EncodeNode(TAvpInfo *pInfo,TAvpNode *pNode);
    /**********************************************************************
    函   数: int EncodeNodeHead(TAvpInfo *pInfo);
    功   能: AVP头编码函数
    输入参数: TAvpInfo *pInfo---需要转换的数据结构
    返回结果: 返回AVP头编码后的数据长度
    ************************************************************************/
    static int EncodeNodeHead(TAvpInfo *pInfo,unsigned char * mp_pDataBuffer,int &mp_iDataIndex);
private:
    //下面是编码所用的函数,基本类型编码函数
    static int IntToBin(unsigned char *pcOut,int iData);
    static int UintToBin(unsigned char *pcOut,unsigned int iData);
    static int UllongToBin(unsigned char *pcOut,unsigned long long iData);
    static int LlongToBin(unsigned char *pcOut,long long iData);
    static int CnvtIpvToDmtIP(unsigned char *pcOut,const char *psSrc);
private:
    //temp//TRatableEvent *m_pRateEvent;//存储需要转换的内部数据结构
    unsigned char m_pDataBuffer[4096];//存储转换后的消息
    int m_iDataIndex;//存储转换后的消息长度
    int m_iServiceType;//存储转换时的业务子类型
    int m_iCcrType;//业务请求类型
    TAvpFile *m_pAvpFile;//配置文件信息
    TAvpHead *m_pAvpHead;//转换的消息头信息
    THashList *m_pDmtHash;//TRatableEvent和消息类同时组包的HASH
    bool m_bDoubleSrc;//组包双来源的标志
    TAvpNode m_tAvpNode;
};
//TAvpSingle---单个AVP 解析类
class TAvpSingle {
public:
    TAvpSingle();
    virtual ~TAvpSingle();
public:
    //设置查询的数据流及长度
    bool SetAvpSrcData(unsigned char *pAvp,int iOrgDataLen);
    //获取Avpcode=iAvpCode,VendorId=iVendorId,路径为cFinding的AVP
    //cFinding = "437,0,1|413,0,1|445,0,1|447,0,1"
    int GetAvpByCondition(int iAvpCode,int iVendorId=0,const char *cFinding=NULL);
private:
    //在相同的层次中查询Avp
    bool GetAvpInfoByCode(int iAvpCode,int iTimes,int iVendorId=0);
    //将查询条件设置在m_iCondition中
    bool SetCondition(int iAvpCode,int iVendorId=0,const char *cFinding=NULL);
    //清除查询数据
    virtual void ClearData(void);
    /**
    *@brief:设置m_cResultDataBuf和m_iResultDataLen
    *@return:void
    */
    void SetResultDataBuf();
public:
    int m_iSrcDataLen;//原始数据长度
    unsigned char m_cSrcDataBuf[4001];//原始数据buffer
    unsigned char m_cResultDataBuf[4001];//存放结果数据Value
    int m_iResultDataLen;//结果数据长度
    int m_iResultDataLoc;//结果数据在原始数据中的位置
    bool m_bHasFlag;//是否找到的标志
    int m_iConNumber;//查询路径深度
    int m_iCondition[255][4];//查询条件路径
    int m_iFindLoc;//查询时得到匹配时的指针位置,处理重复性avp时使用,当等于
};
#endif //__T_DMTETER_PARSER_H_


