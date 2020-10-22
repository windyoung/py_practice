//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: DiameterPareser.h
// Author: ���ڷ�
// Date: 2007/03/01
// Description: OCS���߼Ʒ���Ϣ��̬���ýӿ�
////////////////////////////////////////////////
#ifndef __T_DMTETER_PARSER_H_
#define __T_DMTETER_PARSER_H_

#include "Common.h"
#include "TSplit.h"

#define CONFIG_NUMBER 19
#define CONFIG_NUMBER_5G 21
#define OPER_CONFIG_NUMBER 5

//ȷ��Э���е�λ����UNIX����(���ջ���Э��).λ����ֽ��򱣳�һ��
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
//TAvpInfo---������������ļ�avpfile.ini��������ṹ��
class TAvpInfo : public TBaseObject {
public:
    TAvpInfo();
    virtual ~TAvpInfo();
public:
    char AvpKey[128];//HASH�ļ�ֵ
    unsigned int AttrId;//avp�ֶζ�Ӧ��TRatableEvent��attrid
    unsigned int AvpType;//��������
    char         AvpName[128];//avp������
    unsigned int AvpCode;//avp��code
    unsigned int VendorId;//avp��vendorid
    unsigned int Index;//avp���ֵĴ�����ʶ
    unsigned int HashCode;//avp�ֶ���HASH���е�HashCode
    bool IsGroup;//avp�ڹ淶�еĽṹ����
    int AvpLevel;//avp�ڹ淶�еĲ��λ��
    bool bIsOctetString; // �Ƿ���OctetString��, ZMP:487585

    //�����Ҫʹ�õ�����ѡ��
    bool bMustFlag;//��־M������ѡ��
    bool bNoEncode[16][4];//�Ǳ���ǿ��λ
    bool bOption[16][4];//����ǿ��λ
    char cDefaultValue[128];//Ĭ�ϱ��������ֵ
    int  AvpLenLoc;//��avp����Ϣ�����еĳ���λ��
    bool bHasDefVal;//ZMP:491230,��ʾ�Ƿ���Ĭ��ֵ��true��Ĭ��ֵ��falseû��
public:
    TAvpInfo *ParentAvpInfo;//���ڵ�ָ��
    TBaseListItem *SubAvpItem;//���ӽڵ���ָ��
    TAvpNode *AvpNode;//(�����ʹ�õ�)��Ӧ����avpʵ��ָ��
public:
    virtual int Compare(int iKey) const;
    virtual int Compare(int iKey1, int iKey2) const;
    virtual int Compare(const TBaseObject *pObject) const;
    virtual string ToString();//���������Ϣ
};
//TAvpFile---������������ļ�avpfile.ini��������
class TAvpFile : public TBaseObject {
public:
    TAvpFile();
    virtual ~TAvpFile();
public:    
    //ȡKEY_INFO��Ӧ��������Ϣ
    TAvpInfo * GetAvpInfo(const char *sKey);  
    //���������ļ���һ��������Ϣ��TAvpInfo��
    TAvpInfo * GetAvpInfoByText(const char *sLineText,const int iLineLength);
    //��ȡavpfile��һ��������Ľڵ�
    TAvpInfo * GetAvpHead(void);
    //��ȡ�����ļ�avpfile.ini���ⲿ�ӿ�
    void ReadConfig(const char *sFileName);
    //������е���������Ϣ,ʵclear,���HASH��,�ͷ��ڴ�
    void Clear(void);
    void ClearData(TAvpInfo *pAvpInfo = NULL);
    //��ȡ���������ַ�����ʶ��Ӧ���ڲ���
    unsigned int GetAvpType(const char *sAvpTypeStr);
    //��ӡ�����ļ���Ϣavpfile.ini,Ĭ�ϴ�ͷ�ڵ�m_AvpHead��ʼ��ӡ
    void PrintAvpFile(TAvpInfo *pAvpInfo=NULL);
private:
    //�ͷ�һ���ڵ㼰���ӽڵ�
    void ClearAvpNode(TAvpInfo *pAvpInfo);
private:
    THashList *m_pHashKey;//HASH��
    TBaseList *m_pListCtrl;//���������
    TAvpInfo m_AvpHead;//��Ӧ�����ļ��ĵ�һ��������(��ڵ�)
    TAvpInfo m_tAvpInfo;//HASH������ʹ�õ���ʱ����
    TSplit m_Split;//����avpfileʹ�õķָ����
};
//TAvpHead---OCPЭ����Ϣͷ��Ӧ������ʵ����
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
//TAvpNode---OCPЭ����Ϣ��AVP�������Ӧ��ʵ����
class TAvpNode : public TRecordData { 
public:
    //�������AvpFile���õ���������
    unsigned int AvpCode;
    unsigned int VendorId;
    bool bHaveVendorId;
    unsigned int AvpIndex;

    char AvpKey[128];//AVP��·��(AvpNode1,VerderId1,Index1|...|AvpNoden,VerderIdn,Indexn)

    unsigned char *AvpNode;//avp�ṹ����Ϣ�����е�λ��
    unsigned int AvpSize;//avp�Ĵ�С

    unsigned char * AvpNodeData;//avp����������Ϣ�����е�λ��
    unsigned int AvpDataSize;//avp���ݵĴ�С

    TAvpNode *ParentNode;//���ڵ�
    TAvpNode *SubNode;//�����׽ڵ�
    TAvpNode *SubTailNode;//�����׽ڵ�
    TAvpNode *NextNode;//�ֵܽڵ�
    TAvpInfo *AvpInfo;//��Ӧ����������Ϣ

    //DmtStruct�����ͱ������õ���������
    unsigned char AvpFlag;//avp�ṹ�еı�־�ֽ�
    unsigned int AvpType;//avp����������
    char DefaultValue[512];//avp��Ĭ�ϱ���ֵ
    bool AvpGroup;//avp�Ƿ�ṹ����
    int AvpLenLoc;//(���ʱʹ��)avp����ʱ�����ڻ����е�λ��
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
    llong AsInteger();     //�������Ե�����ֵ
    char *AsString();      //�������Ե��ַ�����ֵ
    string AsOctetString();  //����OctetString���ַ�����16������,ZMP:487585
public:
    char m_sValue[256];
    char m_tTempValue[256];
    unsigned int HashCode;//DmtOper���޸Ķ�ӦTDiamterInfo��avpʱʹ��
};
//TDiamterInfo---OCP��Ϣʵ����
class TDiamterInfo : public TRecordData{
public:
    TDiamterInfo(unsigned int iAvpListSize=1024);
    virtual ~TDiamterInfo();
public:
    virtual void ClearData(void);
public:
    TAvpHead *AvpHead;//��Ϣͷ
    TAvpNode **AvpNodes;//��Ϣ�����Ϣ
    TAvpNode AvpNodeHead;//��Ϣ������
    unsigned int AvpNodeSize;//��Ϣ���С(*AvpNodes�����С)
    unsigned int AvpListSize;//��Ϣ�������С
};
//TAvpOperItem---ҵ����������ļ�avpoper.ini��������ṹ��
class TAvpOperItem : public TBaseObject
{
public:
    TAvpOperItem();
     virtual ~TAvpOperItem();
    virtual int Compare(int iKey) const;
    virtual int Compare(int iKey1, int iKey2) const;
    virtual int Compare(const TBaseObject *pObject) const;
    virtual string ToString();//���������Ϣ
public:
    char Avpkey[128];//��ҵ���ֶζ�Ӧ�ڹ淶�еļ�ֵ
    char AttrName[64];//ҵ���ֶ���
    unsigned int AttrId;//��Ӧ��TRatableEvent������ID,Ĭ��ʹ��avpfile.ini
    unsigned char AvpFlag;//���ʱ��Ҫ���õı�־,Ĭ��ʹ��avpfile.ini
    char DefaultValue[128];//Ĭ��ֵ,Ĭ��ʹ��avpfile.ini
    unsigned int HashCode;//TAvpOperItem��HASH���е�HashCode
};
//TAvpOperStruct---ҵ����������ļ�avpoper.ini��������
class TAvpOperStruct : public TBaseObject
{
public:
    TAvpOperStruct();
    virtual ~TAvpOperStruct();
    //virtual int Compare(const TBaseObject *pObject) const;
    //virtual string ToString();//���������Ϣ
public:
    //��ȡ�����ļ�avpoper.ini���ⲿ�ӿ�
    void ReadConfig(const char *sFileName);
    //ȡKEY_INFO��Ӧ��������Ϣ
    TAvpOperItem * GetAvpOperItem (const char *sKey); 
    //�޸�KEY_INFO��Ӧ��ֵ
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
//TDmtOper---ҵ�����-���������
class TDmtOper :public TBaseObject{
friend class TDiameterTransForm;
public:
    TDmtOper();
    virtual ~TDmtOper(void);
    /**********************************************************************
    ��   ��: void ReadConfig(TAvpFile * pAvpfile,TAvpOperStruct * pOperStruct);
    ��   ��: �л������ļ���Ϣ��avpfile.ini��avpoper.ini
    �������: TAvpFile * pAvpfile---avpfile.ini�����ļ���
    TAvpOperStruct * pOperStruct---avpoper.ini�����ļ���
    ���ؽ��: ��
    ************************************************************************/
    void ReadConfig(TAvpFile * pAvpfile,TAvpOperStruct * pAvpStruct,bool NeedOperStuct = false);
    /**********************************************************************
    ��   ��: void ReadConfig(const char *sFileName,const char *sStrFileName);
    ��   ��: ��ȡ�����ļ���Ϣ��avpfile.ini��avpoper.ini
    �������: const char *sFileName---avpfile.ini�����ļ�λ��
              const char *sStrFileName---avpoper.ini�����ļ�λ��
    ���ؽ��: ��
    ************************************************************************/
    void ReadConfig(const char *sFileName,const char *sStrFileName,bool NeedOperStuct = false);
    /**********************************************************************
    ��   ��: void SetDmtSerType(int iSerType);
    ��   ��: ������Ϣ��ҵ������
    �������: int iSerType---��Ϣ��ҵ������
    ���ؽ��: ��
    ************************************************************************/
    void SetDmtSerType(int iSerType,int iCcrType);
    /**********************************************************************
    ��   ��: void SetMsgHeadInfo(TAvpHead *pInfo);
    ��   ��: ������Ϣͷ��Ϣ
    �������: TAvpHead *pInfo---��Ϣͷ���ݽṹ
    ���ؽ��: ��
    ************************************************************************/
    void SetMsgHeadInfo(TAvpHead *pInfo);
    /**********************************************************************
    ��   ��: bool DmtEncode(unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize);
    ��   ��: ��Ϣ������ⲿ�ӿ�
    �������: unsigned char *pDataBuffer---ת����������׵�ַ
              unsigned int iDataSize---�洢ת��������ݵ���󳤶�   
              int *iRealSize----ת��������ݵ�ʵ�ʳ���
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool DmtEncode(unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize);
    /**********************************************************************
    ��   ��: bool DmtEncode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize);
    ��   ��: ��Ϣ������ⲿ�ӿ�
    �������: TRatableEvent *pEvent---��������AVPԴ
              unsigned char *pDataBuffer---ת����������׵�ַ
              unsigned int iDataSize---�洢ת��������ݵ���󳤶�   
              int *iRealSize----ת��������ݵ�ʵ�ʳ���
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    //del//bool DmtEncode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize);
    void DmtPrepareEncode();
    /**********************************************************************
    ��   ��: bool SetItemValue(const char *sKey,const char *newVlaue);
    ��   ��: ����KEY_INFO��Ӧ��ֵ�����������½ڵ㣩�����ڵ�����ǿ��Զ�̬����AVP������Ϣ
    �������: const char *sKey---ҵ���ֶεļ�ֵ
              const char *newVlaue---��ֵ   
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool SetItemValue(const char *sKey,const char *newVlaue);
    /**********************************************************************
    ��   ��: bool ModifyItemValue(const char *sKey,const char *newVlaue);
    ��   ��: �޸�KEY_INFO��Ӧ��ֵ�����ڵ���������ܲ���ʱ��̬�޸Ķ�Ӧavp�ڵ������
    �������: const char *sKey---ҵ���ֶεļ�ֵ
              const char *newVlaue---��ֵ   
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool ModifyItemValue(const char *sKey,const char *newVlaue);
    /**********************************************************************
    ��   ��: bool DmtParser(unsigned char *pDataBuffer,unsigned int iDataSize);
    ��   ��: ��Ϣ�������ⲿ�ӿ�
    �������: unsigned char *pDataBuffer---��Ϣ�����׵�ַ
              unsigned int iDataSize---�������ݵ���󳤶�
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool DmtParser(unsigned char *pDataBuffer,unsigned int iDataSize);
    /**********************************************************************
    ��   ��: TAvpNode * GetAvpNode(const char *sKey);
    ��   ��: �޸�KEY_INFO��Ӧ��Avp���ݽṹ
    �������: const char *sKey---ҵ���ֶεļ�ֵ
    ���ؽ��: TAvpNode * ������ָ��
    ************************************************************************/
    TAvpNode * GetAvpNode(const char *sKey);    
    /**********************************************************************
    ��   ��: bool SetRatableEvent(TRatableEvent *pRatableEvent); 
    ��   ��: ������ת����TRatableEvent,���ڵ������ֻ���ù�����ҵ���ֶ�
    �������: TRatableEvent *pRatableEvent---ת��������ݽṹ
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    //del//bool SetRatableEvent(TRatableEvent *pRatableEvent); 
    /**********************************************************************
    ��   ��: TDiamterInfo * GetDiamterInfo(void); 
    ��   ��: �õ����������Ϣ�ṹ
    �������: ��
    ���ؽ��: ��TDiamterInfo * ��������
    ************************************************************************/
    TDiamterInfo * GetDiamterInfo(void);
    /**********************************************************************
    ��   ��: bool SetDmtSrcData(unsigned char *pAvp,int iOrgDataLen);
    ��   ��: ��������������ԭʼ����
    �������: unsigned char *pDataBuffer---��Ϣ�����׵�ַ
              int iOrgDataLen---�������ݵ���󳤶�
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool SetDmtSrcData(unsigned char *pAvp,int iOrgDataLen);
    /**********************************************************************
    ��   ��: bool GetAvpByKey(const char *cFinding,unsigned char *pDataBuffer,int size,int *iDataSize,int *iDataType);
    ��   ��: ��ȡcFindingָ����AVP
    �������: const char *cFinding---��ѯ����key
              unsigned char *pDataBuffer---AVP��ԭʼ����
              int size ---����������Ĵ�С
              int *iDataSize ---���ݴ�С
              int *iDataType ---��������
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool GetAvpByKey(const char *cFinding,unsigned char *pDataBuffer,int size,int *iDataSize,int *iDataType);
    virtual string ToString();
private:
    bool Cnvt2DmtStruct();
    bool GetEncodeFlag(TAvpOperStruct *pAvpStruct,TAvpInfo *pAvpInfo);
    TAvpNode * SetDmtNode(TAvpNode *pParentNode,TAvpNode *pNode,TAvpInfo *pSrcData);
private:
    //THashList *m_pHashKey;//��̬�޸���������ࣨ���㣩TDiamterInfo��HASH������װ�������ʱ����
    TBaseList *m_pListCtrl;//���������
    TAvpSingle *m_pAvpSingle;//����AVP������
    TAvpParser *m_pAvpParser;//��Ϣ������TDiamterInfo�Ĳ�����
    TAvpEncode *m_pAvpEncode;//TDiamterInfo����Ϣ�����Ĳ�����
    TDiamterInfo *m_pDmtData;//ocp��Ϣ��Ӧ��ʵ����(�����������ļ�����Ϣ�������м�ṹ)
    int m_iIndex;//TDiamterInfo�Ľڵ��±�����
    TAvpFile *m_pAvpFile;//�����ļ��洢��Ϣ 
    TAvpOperStruct *m_pAvpStruct;//ҵ���������ת���ṹ
    int m_iServiceType;//�洢ת��ʱ��ҵ��������
    int m_iCcrType;//ҵ����������
    bool m_bHasFresh;//�Ƿ��������ڵ��־��Ϊ������װ����ʹ��
    TAvpNode *m_pConNode[256];//����ӿڶ�̬����AVPʱʹ�õģ����TRatableEvent��̬�����
    TAvpNode m_tAvpNode;
    int m_iConIndex;
    TSplit Split;
    bool m_bCloneConfig;//������Ϣ��������
};

//TAvpParser---�����ļ���avpfile.ini��������
class TAvpParser : public TBaseObject{
friend class TDiameterTransForm;
public:
    //ת�������ļ���Ϣ����TAvpFile����,��avpNode����keyֵ���Ҷ�Ӧ��ת����Ϣ
    TAvpParser(TAvpFile *pAvpFile,unsigned int iAvpListSize = 1024);
    virtual ~TAvpParser(void);
public:
    /**********************************************************************
    ��   ��: bool Parser(unsigned char *pDataBuffer,unsigned int iDataSize);
    ��   ��: ԭʼ��Ϣ��������
    �������: unsigned char *pDataBuffer---��Ϣ�׵�ַ
              unsigned int iDataSize    ---��Ϣ����
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool Parser(unsigned char *pDataBuffer,unsigned int iDataSize);
    /**********************************************************************
    ��   ��: bool SetRatableEvent(TRatableEvent *pRatableEvent); 
    ��   ��: ������ת����TRatableEvent
    �������: TRatableEvent *pRatableEvent---ת��������ݽṹ
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    //del//virtual bool SetRatableEvent(TRatableEvent *pRatableEvent); 
    /**********************************************************************
    ��   ��: TDiamterInfo * GetDiamterInfo(void); 
    ��   ��: �õ����������Ϣ�ṹ
    �������: ��
    ���ؽ��: ��TDiamterInfo * ��������
    ************************************************************************/
    TDiamterInfo * GetDiamterInfo(void);
    /**********************************************************************
    ��   ��: TAvpNode * GetAvpNodeInfo(const char *sKey);
    ��   ��: �޸�KEY_INFO��Ӧ��ֵ
    �������: const char *sKey---ҵ���ֶεļ�ֵ
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    TAvpNode * GetAvpNodeInfo(const char *sKey);
    /**********************************************************************
    ��   ��: string ToString();
    ��   ��: ���������Ϣ
    �������: ��
    ���ؽ��: ��string��������
    ************************************************************************/
    virtual string ToString();
    unsigned int GetErrorCode(void);
    char * GetErrorInfo(void);
    void SetConfigInfo(TAvpFile *pAvpFile);
private:
    /**********************************************************************
    ��   ��: bool ParserHead(unsigned char *pDataBuffer,unsigned int iDataSize);
    ��   ��: ������Ϣͷ��Ϣ
    �������: unsigned char *pDataBuffer ������Ϣͷ�׵�ַ
              unsigned int iDataSize������Ϣ����
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool ParserHead(unsigned char *pDataBuffer,unsigned int iDataSize);
    /**********************************************************************
    ��   ��: bool ParserNode(TAvpNode *pParentNode,unsigned char *pDataBuffer,unsigned int iDataSize);
    ��   ��: ���������Ϣ
    �������: TAvpNode *pParentNode������������ݽṹ
              unsigned char *pDataBuffer��������׵�ַ
              unsigned int iDataSize����������ݳ���
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool ParserNode(TAvpNode *pParentNode,unsigned char *pDataBuffer,unsigned int iDataSize);
    /**********************************************************************
    ��   ��: TAvpNode * GetNodeInfo(TAvpNode *pParentNode,unsigned char *pDataBuffer,unsigned int iDataSize); 
    ��   ��: ����AVP���Ľ����������洢��HASH�����KEYֵ�Ļ�ȡ
    �������: TAvpNode *pParentNode������������ݽṹ
              unsigned char *pDataBuffer��������׵�ַ
              unsigned int iDataSize����������ݳ���
    ���ؽ��: ��TAvpNode *��������
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
    //�����ǽ������õĺ���,�������ͽ�������
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
    TDiamterInfo *m_pDiamterInfo;//������Ľ������
    unsigned char m_pDataBuffer[4096];//���������ԭʼ��Ϣ
    int m_iDataSize;//���������ԭʼ��Ϣ�ĳ���    
    unsigned int m_iNodeIndex;//��ʱ������ָ��һ���յ�NODE���
    TAvpFile *m_pAvpFile;//�����ļ��洢��Ϣ
private:
    int m_iErrorCode;
    char m_sErrorMsg[128];
};
//TAvpEncode---�����ļ���avpfile.ini�������
class TAvpEncode : public TBaseObject{
friend class TDiameterTransForm;
public:
    TAvpEncode(TAvpFile *pAvpFile);
    virtual ~TAvpEncode();
public:
    /**********************************************************************
    ��   ��: bool Encode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize,int iServType = 0);
    ��   ��: DIAMETER�������
    �������: TRatableEvent *pEvent---��Ҫת�������ݽṹ
              unsigned char *pDataBuffer---ת����������׵�ַ
              unsigned int iDataSize---�洢ת��������ݵ���󳤶�   
              int *iRealSize----ת��������ݵ�ʵ�ʳ���
              int iServType = 0---ת������Ϣҵ�������ͣ�����Ĳ��������ļ���
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    //del//bool Encode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize,int iServType = 0);
    /**********************************************************************
    ��   ��: bool Encode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize,int iServType = 0);
    ��   ��: DIAMETER�������
    �������: TDiamterInfo *pInfo---��Ҫת�������ݽṹ
              unsigned char *pDataBuffer---ת����������׵�ַ
              unsigned int iDataSize---�洢ת��������ݵ���󳤶�   
              int *iRealSize----ת��������ݵ�ʵ�ʳ���
              int iServType = 0---ת������Ϣҵ�������ͣ�����Ĳ��������ļ���
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool Encode(TDiamterInfo *pInfo,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize,int iServType = 0);
    /**********************************************************************
    ��   ��: string ToString();
    ��   ��: ���������Ϣ
    �������: ��
    ���ؽ��: ��string��������
    ************************************************************************/
    virtual string ToString(void);
    /**********************************************************************
    ��   ��: void SetMsgHeadInfo(TAvpHead *pInfo);
    ��   ��: ������Ϣͷ������Ϣ
    �������: TAvpHead *pInfo---��Ҫ��������ݽṹ
    ���ؽ��: ��
    ************************************************************************/
    void SetMsgHeadInfo(TAvpHead *pInfo);
    /**********************************************************************
    ��   ��: bool GetEncodeFlag(TAvpInfo *pInfo,bool IsOperStr = false);
    ��   ��: ��ȡAVP �������־
    �������: TAvpInfo *pInfo---��Ҫ��������ݽṹ
              bool IsOperStr = false --- ��ҵ����Խṹ���ʱʹ�ã�����TRatableEvent�в���
              bool IsEncodeView = true --- �����ǰ���жϺ��������ʱ�����֣���ѡ��־�ڽṹ���еĴ�����
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    bool GetEncodeFlag(TAvpInfo *pInfo,bool IsOperStr = false,bool IsEncodeView = true);
    /**********************************************************************
    ��   ��: void SetMsgSerType(int iServiceType);
    ��   ��: ������Ϣͷ������Ϣ
    �������: int iServiceType---��Ҫ�����ҵ������
    ���ؽ��: ��
    ************************************************************************/
    void SetMsgSerType(int iServiceType,int iCcrType);
    /**********************************************************************
    ��   ��: void SetDmtData(THashList *m_pDmtHash);
    ��   ��: ���������Ϣ�ṹ
    �������: THashList *m_pDmtHash---��Ҫ���õ���Ϣ�ṹHASH��
    ���ؽ��: ��
    ************************************************************************/
    void SetDmtHash(THashList *pDmtHash);
    void SetConfigInfo(TAvpFile *pAvpFile);
    static int StrToByte(unsigned char *psOut,const char *psSrc);    
private:
    /**********************************************************************
    ��   ��: bool EncodeMsgHead(TAvpHead *pHead);
    ��   ��: ��Ϣͷ���뺯��
    �������: TAvpHead *pHead---��Ҫת�������ݽṹ
    ���ؽ��: true---�ɹ�,false---ʧ��
    ************************************************************************/
    static bool EncodeMsgHead(TAvpHead *pHead,unsigned char * mp_pDataBuffer,int &mp_iDataIndex);
    /**********************************************************************
    ��   ��: int EncodeNode(TAvpInfo *pInfo);
    ��   ��: AVP���ݱ��뺯��
    �������: TAvpInfo *pInfo---��Ҫת�������ݽṹ
    ���ؽ��: ����AVP���������ݳ���
    ************************************************************************/
    //del//int EncodeNode(TAvpInfo *pInfo);
    /**********************************************************************
    ��   ��: int EncodeNode(TAvpNode *pInfo);
    ��   ��: AVP���ݱ��뺯��
    �������: TAvpInfo *pInfo---��Ҫת�������ݽṹ
    ���ؽ��: ����AVP���������ݳ���
    ************************************************************************/
    int EncodeNode(TAvpNode *pInfo,unsigned char * mp_pDataBuffer,int &mp_iDataIndex);
    /**********************************************************************
    ��   ��: int EncodeNode(TAvpNode *pInfo,TAvpNode *pNode);
    ��   ��: AVP���ݱ��뺯��
    �������: TAvpInfo *pInfo---��Ҫת�������ݽṹ
              TAvpNode *pNode---��Ҫת�������ݽṹ
    ���ؽ��: ����AVP���������ݳ���
    ************************************************************************/
    //del//int EncodeNode(TAvpInfo *pInfo,TAvpNode *pNode);
    /**********************************************************************
    ��   ��: int EncodeNodeHead(TAvpInfo *pInfo);
    ��   ��: AVPͷ���뺯��
    �������: TAvpInfo *pInfo---��Ҫת�������ݽṹ
    ���ؽ��: ����AVPͷ���������ݳ���
    ************************************************************************/
    static int EncodeNodeHead(TAvpInfo *pInfo,unsigned char * mp_pDataBuffer,int &mp_iDataIndex);
private:
    //�����Ǳ������õĺ���,�������ͱ��뺯��
    static int IntToBin(unsigned char *pcOut,int iData);
    static int UintToBin(unsigned char *pcOut,unsigned int iData);
    static int UllongToBin(unsigned char *pcOut,unsigned long long iData);
    static int LlongToBin(unsigned char *pcOut,long long iData);
    static int CnvtIpvToDmtIP(unsigned char *pcOut,const char *psSrc);
private:
    //temp//TRatableEvent *m_pRateEvent;//�洢��Ҫת�����ڲ����ݽṹ
    unsigned char m_pDataBuffer[4096];//�洢ת�������Ϣ
    int m_iDataIndex;//�洢ת�������Ϣ����
    int m_iServiceType;//�洢ת��ʱ��ҵ��������
    int m_iCcrType;//ҵ����������
    TAvpFile *m_pAvpFile;//�����ļ���Ϣ
    TAvpHead *m_pAvpHead;//ת������Ϣͷ��Ϣ
    THashList *m_pDmtHash;//TRatableEvent����Ϣ��ͬʱ�����HASH
    bool m_bDoubleSrc;//���˫��Դ�ı�־
    TAvpNode m_tAvpNode;
};
//TAvpSingle---����AVP ������
class TAvpSingle {
public:
    TAvpSingle();
    virtual ~TAvpSingle();
public:
    //���ò�ѯ��������������
    bool SetAvpSrcData(unsigned char *pAvp,int iOrgDataLen);
    //��ȡAvpcode=iAvpCode,VendorId=iVendorId,·��ΪcFinding��AVP
    //cFinding = "437,0,1|413,0,1|445,0,1|447,0,1"
    int GetAvpByCondition(int iAvpCode,int iVendorId=0,const char *cFinding=NULL);
private:
    //����ͬ�Ĳ���в�ѯAvp
    bool GetAvpInfoByCode(int iAvpCode,int iTimes,int iVendorId=0);
    //����ѯ����������m_iCondition��
    bool SetCondition(int iAvpCode,int iVendorId=0,const char *cFinding=NULL);
    //�����ѯ����
    virtual void ClearData(void);
    /**
    *@brief:����m_cResultDataBuf��m_iResultDataLen
    *@return:void
    */
    void SetResultDataBuf();
public:
    int m_iSrcDataLen;//ԭʼ���ݳ���
    unsigned char m_cSrcDataBuf[4001];//ԭʼ����buffer
    unsigned char m_cResultDataBuf[4001];//��Ž������Value
    int m_iResultDataLen;//������ݳ���
    int m_iResultDataLoc;//���������ԭʼ�����е�λ��
    bool m_bHasFlag;//�Ƿ��ҵ��ı�־
    int m_iConNumber;//��ѯ·�����
    int m_iCondition[255][4];//��ѯ����·��
    int m_iFindLoc;//��ѯʱ�õ�ƥ��ʱ��ָ��λ��,�����ظ���avpʱʹ��,������
};
#endif //__T_DMTETER_PARSER_H_


