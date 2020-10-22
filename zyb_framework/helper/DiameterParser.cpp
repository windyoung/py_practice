//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
////////////////////////////////////////////////
// Name: DiameterPareser.cpp
// Author: 张宗放
// Date: 2007/03/01
// Description: OCS在线计费消息动态配置接口
////////////////////////////////////////////////
#include <iostream>
#include <string>
#include <sstream>
#include "math.h"
#include "DiameterParser.h"
#include "DataStruct.h"
#include "TSplit.h"
#include "TStrFunc.h"
#include "TException.h"
#include "debug_new.h"
//del//#include "TRatableEvent.h"

//#define SPLIT_CHAR  '\t'
#define SPLIT_CHAR  ' '

//ZMP:467371    
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

TAvpInfo::TAvpInfo()
{
    memset(AvpKey,0,sizeof(AvpKey));
    AttrId = 0;
    AvpType = 0;
    memset(AvpName,0,sizeof(AvpName));
    AvpCode = 0;
    VendorId = 0;
    Index = 0;
    HashCode = 0;
    IsGroup = false;
    AvpLevel = 0;
    ParentAvpInfo = NULL;
    SubAvpItem = NULL;
    AvpNode = NULL;

    memset(bOption,0,sizeof(bOption));
    memset(bNoEncode,0,sizeof(bNoEncode));
    memset(cDefaultValue,0,sizeof(cDefaultValue));
    AvpLenLoc = 0;
    bMustFlag = false;
    bIsOctetString = false;
    bHasDefVal = false;//ZMP:491230
}
TAvpInfo::~TAvpInfo()
{
    TBaseList pListCtrl;
    pListCtrl.SetList(SubAvpItem);
    pListCtrl.Clear(true);
}

int TAvpInfo::Compare(int iKey) const
{
  throw TException("TAvpInfo::Compare(int iKey) : Not Write Code.");
}

int TAvpInfo::Compare(int iKey1, int iKey2) const
{
  throw TException("TAvpInfo::Compare(int iKey1, int iKey2) : Not Write Code.");
}

int TAvpInfo::Compare(const TBaseObject *pObject) const
{
    TAvpInfo *pFileInfo = (TAvpInfo *)pObject;
    return strcmp(AvpKey,pFileInfo->AvpKey);
}

string TAvpInfo::ToString()
{
    ostringstream sLineText;
    char sLineInfo[1024]={0}; //357111 for cppcheck
    //ZMP:467371 
    snprintf(sLineInfo, sizeof(sLineInfo), "%-5u%-6u%-2u%3u%-1s%2s%*s%-*s %-4u%-40s",//ZMP:305593
        AvpCode,VendorId,Index,AttrId,(IsGroup?"+":" "),bOption[0]?"M":"Oc",AvpLevel*2," ",60-AvpLevel*2,AvpName,HashCode,AvpKey);
    sLineInfo[sizeof(sLineInfo)-1] = '\0';
    sLineText<<sLineInfo;
    return sLineText.str();
}

TAvpFile::TAvpFile()
{
    m_pHashKey = new THashList();
    m_pListCtrl = new TBaseList();
    m_pHashKey->SetHashNum(509);
    m_Split.SetSplitter(SPLIT_CHAR);
    //ZMP:467371 
    strncpy(m_AvpHead.AvpName, "AVP_ROOT" ,sizeof(m_AvpHead.AvpName)-1);
    m_AvpHead.AvpName[sizeof(m_AvpHead.AvpName)-1] = '\0';
    strncpy(m_AvpHead.AvpKey, "OCP1.0", sizeof(m_AvpHead.AvpKey)-1);
    m_AvpHead.AvpKey[sizeof(m_AvpHead.AvpKey)-1] = '\0';
    m_AvpHead.IsGroup = true; 
    m_AvpHead.AvpLevel = 0;
    m_AvpHead.AvpCode = 0;
}

TAvpFile::~TAvpFile()
{
    Clear();
    DELETE(m_pHashKey);
    DELETE(m_pListCtrl);
}

void TAvpFile::Clear(void)
{
    ClearAvpNode(&m_AvpHead);
    m_pHashKey->Clear();
}
void TAvpFile::ClearData(TAvpInfo *pAvpInfo)
{
    TAvpInfo *pCurrAvpInfo = pAvpInfo ? pAvpInfo : &m_AvpHead;

    if(pCurrAvpInfo==NULL) return;

    if(pCurrAvpInfo->AvpNode != NULL)
    {
        pCurrAvpInfo->AvpNode = NULL;
    }

    TBaseListItem *pItem = pCurrAvpInfo->SubAvpItem;
    while(pItem)
    {
        ClearData((TAvpInfo *)pItem->data);
        pItem = pItem->next;
    }
}
void TAvpFile::ClearAvpNode(TAvpInfo *pAvpInfo)
{
    TAvpInfo *pCurrAvpInfo = pAvpInfo ? pAvpInfo : &m_AvpHead;

    if(pCurrAvpInfo==NULL) return;

    TBaseListItem *pItem = pCurrAvpInfo->SubAvpItem;
    while(pItem)
    {
        ClearAvpNode((TAvpInfo *)pItem->data);
        pItem = pItem->next;
    }
    while (pCurrAvpInfo->SubAvpItem)
    {
        m_pListCtrl->SetList(pCurrAvpInfo->SubAvpItem);
        m_pListCtrl->Clear(true);
    }

    //if ((pCurrAvpInfo !=&m_AvpHead) && (pCurrAvpInfo != NULL))
    //{
    //    delete pCurrAvpInfo;
    //    pCurrAvpInfo = NULL;
    //}
}

void TAvpFile::PrintAvpFile(TAvpInfo *pAvpInfo)
{
    if(pAvpInfo == NULL)
      return;
      
    TAvpInfo *pCurrAvpInfo = pAvpInfo ? pAvpInfo : &m_AvpHead;

    if(pCurrAvpInfo==NULL) return;

    if(pCurrAvpInfo->AvpCode>0)
        printf("%s\n",pCurrAvpInfo->ToString().c_str());

    TBaseListItem *pItem = pCurrAvpInfo->SubAvpItem;
    while(pItem)
    {
        PrintAvpFile((TAvpInfo *)pItem->data);
        pItem = pItem->next;
    }
}

//取KEY_INFO对应的配置信息
TAvpInfo * TAvpFile::GetAvpInfo(const char *sKey)
{
    strncpy(m_tAvpInfo.AvpKey,sKey,sizeof(m_tAvpInfo.AvpKey));

    TAvpInfo *pAvpInfo = (TAvpInfo *)m_pHashKey->Seek(sKey,&m_tAvpInfo);

    return pAvpInfo;
}

TAvpInfo * TAvpFile::GetAvpInfoByText(const char *sLineText,const int iLineLength)
{
    char *pLinePtr = (char *)sLineText;
    TAvpInfo *pAvpInfo = NULL;

    //整理输入行
    {
        char pTrimChar[]={0x20,0x0D,0x0A,0x09};//需要过虑掉的字符 空格/回车符/换行符/TAB        
        bool bInstead = true;
        //bool bHasParentNode = false;//ZMP:305593,变量未用到
        int iCopyIndex=0;
        for (int iCurrIndex=0;iCurrIndex<iLineLength;iCurrIndex++)
        {

            if (pLinePtr[iCurrIndex] != pTrimChar[3] && pLinePtr[iCurrIndex] != pTrimChar[0] &&
                pLinePtr[iCurrIndex] != pTrimChar[1] && pLinePtr[iCurrIndex] != pTrimChar[2])
            {
                //有效字符，无条件COPY
                bInstead = true;
                pLinePtr[iCopyIndex] = pLinePtr[iCurrIndex];
                iCopyIndex++;
            }
            else if (bInstead && pLinePtr[iCurrIndex] == SPLIT_CHAR)
            {
                //第一无效字符，需要COPY
                bInstead = false;
                pLinePtr[iCopyIndex] = pLinePtr[iCurrIndex];
                iCopyIndex++;
            }
        }
        memset(pLinePtr+iCopyIndex,0,(size_t)(iLineLength-iCopyIndex));
        TStrFunc::Trim(pLinePtr,pTrimChar,sizeof(pTrimChar));
    }


    //分割数据
    {
        int iCount = m_Split.SetString(pLinePtr);
        if (iCount == CONFIG_NUMBER || iCount == CONFIG_NUMBER_5G)
        {
            char pErrorChar[]={'{','}','[',']','<','>','*'};
            pAvpInfo = new TAvpInfo();
            memcpy(pAvpInfo->AvpName,m_Split[0],sizeof(pAvpInfo->AvpName));
            TStrFunc::Trim(pAvpInfo->AvpName,pErrorChar,sizeof(pErrorChar));
            pAvpInfo->AvpLevel = atoi(m_Split[1]);
            pAvpInfo->AvpCode = (unsigned int)atoi(m_Split[2]);
            pAvpInfo->VendorId = (unsigned int)atoi(m_Split[3]);
            pAvpInfo->AvpType = GetAvpType(m_Split[4]);
            pAvpInfo->bIsOctetString = (strcmp(m_Split[4], "OctetString") == 0);
            pAvpInfo->IsGroup = pAvpInfo->AvpType == AVP_TYPE_GROUP;
            pAvpInfo->Index = (unsigned int)atoi(m_Split[5]);
            pAvpInfo->AttrId = (unsigned int)atoi(m_Split[6]);

            /*   一维下标采用类型标识的2为底的对数表示 
            enum SERVICE_TYPE
            {
            PS_SERVICE = 1,         //PS域业务信息组
            IN_SERVICE = 2,         //智能网业务信息组
            P2PSMS_SERVICE = 4,     //点到点短信业务信息组
            ISMP_SERVICE = 8,       //ISMP业务信息组
            INTERNAL_SERVICE = 16,  //属性对表示的计费事件信息 软创自定义
            SMGW_SERVICE = 32,  //SMGW域业务信息组
            IPTV_SERVICE = 64,   //IPTV域业务信息组
            IM_SERVICE = 128,
            POC_SERVICE = 256,
            IMS_SERVICE = 512
            };*/
            memcpy(pAvpInfo->cDefaultValue,m_Split[7],sizeof(pAvpInfo->cDefaultValue));
            //ZMP:491230
            if(strlen (pAvpInfo->cDefaultValue) >1 || atol(pAvpInfo->cDefaultValue) >0 ) 
            {
                pAvpInfo->bHasDefVal=true;
            }
            pAvpInfo->bMustFlag = (char)m_Split[8][0] == 'M'?true:false;
            pAvpInfo->bOption[0][0] = (char)m_Split[9][0] == 'M'?true:false;
            pAvpInfo->bNoEncode[0][0] = (char)m_Split[9][0] == 'N'?true:false;
            pAvpInfo->bOption[0][1] = (char)m_Split[9][1] == 'M'?true:false;
            pAvpInfo->bNoEncode[0][1] = (char)m_Split[9][1] == 'N'?true:false;
            pAvpInfo->bOption[0][2] = (char)m_Split[9][2] == 'M'?true:false;
            pAvpInfo->bNoEncode[0][2] = (char)m_Split[9][2] == 'N'?true:false;
            pAvpInfo->bOption[0][3] = (char)m_Split[9][3] == 'M'?true:false;
            pAvpInfo->bNoEncode[0][3] = (char)m_Split[9][3] == 'N'?true:false;

            pAvpInfo->bOption[1][0] = (char)m_Split[10][0] == 'M'?true:false;
            pAvpInfo->bNoEncode[1][0] = (char)m_Split[10][0] == 'N'?true:false;
            pAvpInfo->bOption[1][1] = (char)m_Split[10][1] == 'M'?true:false;
            pAvpInfo->bNoEncode[1][1] = (char)m_Split[10][1] == 'N'?true:false;
            pAvpInfo->bOption[1][2] = (char)m_Split[10][2] == 'M'?true:false;
            pAvpInfo->bNoEncode[1][2] = (char)m_Split[10][2] == 'N'?true:false;
            pAvpInfo->bOption[1][3] = (char)m_Split[10][3] == 'M'?true:false;
            pAvpInfo->bNoEncode[1][3] = (char)m_Split[10][3] == 'N'?true:false;

            pAvpInfo->bOption[2][0] = (char)m_Split[11][0] == 'M'?true:false;
            pAvpInfo->bNoEncode[2][0] = (char)m_Split[11][0] == 'N'?true:false;
            pAvpInfo->bOption[2][1] = (char)m_Split[11][1] == 'M'?true:false;
            pAvpInfo->bNoEncode[2][1] = (char)m_Split[11][1] == 'N'?true:false;
            pAvpInfo->bOption[2][2] = (char)m_Split[11][2] == 'M'?true:false;
            pAvpInfo->bNoEncode[2][2] = (char)m_Split[11][2] == 'N'?true:false;
            pAvpInfo->bOption[2][3] = (char)m_Split[11][3] == 'M'?true:false;
            pAvpInfo->bNoEncode[2][3] = (char)m_Split[11][3] == 'N'?true:false;

            pAvpInfo->bOption[3][0] = (char)m_Split[12][0] == 'M'?true:false;
            pAvpInfo->bNoEncode[3][0] = (char)m_Split[12][0] == 'N'?true:false;
            pAvpInfo->bOption[3][1] = (char)m_Split[12][1] == 'M'?true:false;
            pAvpInfo->bNoEncode[3][1] = (char)m_Split[12][1] == 'N'?true:false;
            pAvpInfo->bOption[3][2] = (char)m_Split[12][2] == 'M'?true:false;
            pAvpInfo->bNoEncode[3][2] = (char)m_Split[12][2] == 'N'?true:false;
            pAvpInfo->bOption[3][3] = (char)m_Split[12][3] == 'M'?true:false;
            pAvpInfo->bNoEncode[3][3] = (char)m_Split[12][3] == 'N'?true:false;

            pAvpInfo->bOption[4][0] = (char)m_Split[13][0] == 'M'?true:false;
            pAvpInfo->bNoEncode[4][0] = (char)m_Split[13][0] == 'N'?true:false;
            pAvpInfo->bOption[4][1] = (char)m_Split[13][1] == 'M'?true:false;
            pAvpInfo->bNoEncode[4][1] = (char)m_Split[13][1] == 'N'?true:false;
            pAvpInfo->bOption[4][2] = (char)m_Split[13][2] == 'M'?true:false;
            pAvpInfo->bNoEncode[4][2] = (char)m_Split[13][2] == 'N'?true:false;
            pAvpInfo->bOption[4][3] = (char)m_Split[13][3] == 'M'?true:false;
            pAvpInfo->bNoEncode[4][3] = (char)m_Split[13][3] == 'N'?true:false;

            pAvpInfo->bOption[5][0] = (char)m_Split[14][0] == 'M'?true:false;
            pAvpInfo->bNoEncode[5][0] = (char)m_Split[14][0] == 'N'?true:false;
            pAvpInfo->bOption[5][1] = (char)m_Split[14][1] == 'M'?true:false;
            pAvpInfo->bNoEncode[5][1] = (char)m_Split[14][1] == 'N'?true:false;
            pAvpInfo->bOption[5][2] = (char)m_Split[14][2] == 'M'?true:false;
            pAvpInfo->bNoEncode[5][2] = (char)m_Split[14][2] == 'N'?true:false;
            pAvpInfo->bOption[5][3] = (char)m_Split[14][3] == 'M'?true:false;
            pAvpInfo->bNoEncode[5][3] = (char)m_Split[14][3] == 'N'?true:false;

            pAvpInfo->bOption[6][0] = (char)m_Split[15][0] == 'M'?true:false;
            pAvpInfo->bNoEncode[6][0] = (char)m_Split[15][0] == 'N'?true:false;
            pAvpInfo->bOption[6][1] = (char)m_Split[15][1] == 'M'?true:false;
            pAvpInfo->bNoEncode[6][1] = (char)m_Split[15][1] == 'N'?true:false;
            pAvpInfo->bOption[6][2] = (char)m_Split[15][2] == 'M'?true:false;
            pAvpInfo->bNoEncode[6][2] = (char)m_Split[15][2] == 'N'?true:false;
            pAvpInfo->bOption[6][3] = (char)m_Split[15][3] == 'M'?true:false;
            pAvpInfo->bNoEncode[6][3] = (char)m_Split[15][3] == 'N'?true:false;

            pAvpInfo->bOption[7][0] = (char)m_Split[16][0] == 'M'?true:false;
            pAvpInfo->bNoEncode[7][0] = (char)m_Split[16][0] == 'N'?true:false;
            pAvpInfo->bOption[7][1] = (char)m_Split[16][1] == 'M'?true:false;
            pAvpInfo->bNoEncode[7][1] = (char)m_Split[16][1] == 'N'?true:false;
            pAvpInfo->bOption[7][2] = (char)m_Split[16][2] == 'M'?true:false;
            pAvpInfo->bNoEncode[7][2] = (char)m_Split[16][2] == 'N'?true:false;
            pAvpInfo->bOption[7][3] = (char)m_Split[16][3] == 'M'?true:false;
            pAvpInfo->bNoEncode[7][3] = (char)m_Split[16][3] == 'N'?true:false;

            pAvpInfo->bOption[8][0] = (char)m_Split[17][0] == 'M'?true:false;
            pAvpInfo->bNoEncode[8][0] = (char)m_Split[17][0] == 'N'?true:false;
            pAvpInfo->bOption[8][1] = (char)m_Split[17][1] == 'M'?true:false;
            pAvpInfo->bNoEncode[8][1] = (char)m_Split[17][1] == 'N'?true:false;
            pAvpInfo->bOption[8][2] = (char)m_Split[17][2] == 'M'?true:false;
            pAvpInfo->bNoEncode[8][2] = (char)m_Split[17][2] == 'N'?true:false;
            pAvpInfo->bOption[8][3] = (char)m_Split[17][3] == 'M'?true:false;
            pAvpInfo->bNoEncode[8][3] = (char)m_Split[17][3] == 'N'?true:false;
              
            pAvpInfo->bOption[9][0] = (char)m_Split[18][0] == 'M'?true:false;
            pAvpInfo->bNoEncode[9][0] = (char)m_Split[18][0] == 'N'?true:false;
            pAvpInfo->bOption[9][1] = (char)m_Split[18][1] == 'M'?true:false;
            pAvpInfo->bNoEncode[9][1] = (char)m_Split[18][1] == 'N'?true:false;
            pAvpInfo->bOption[9][2] = (char)m_Split[18][2] == 'M'?true:false;
            pAvpInfo->bNoEncode[9][2] = (char)m_Split[18][2] == 'N'?true:false;
            pAvpInfo->bOption[9][3] = (char)m_Split[18][3] == 'M'?true:false;
            pAvpInfo->bNoEncode[9][3] = (char)m_Split[18][3] == 'N'?true:false;
            if(iCount == CONFIG_NUMBER_5G)
            {
                pAvpInfo->bOption[10][0] = (char)m_Split[19][0] == 'M'?true:false;
                pAvpInfo->bNoEncode[10][0] = (char)m_Split[19][0] == 'N'?true:false;
                pAvpInfo->bOption[10][1] = (char)m_Split[19][1] == 'M'?true:false;
                pAvpInfo->bNoEncode[10][1] = (char)m_Split[19][1] == 'N'?true:false;
                pAvpInfo->bOption[10][2] = (char)m_Split[19][2] == 'M'?true:false;
                pAvpInfo->bNoEncode[10][2] = (char)m_Split[19][2] == 'N'?true:false;
                pAvpInfo->bOption[10][3] = (char)m_Split[19][3] == 'M'?true:false;
                pAvpInfo->bNoEncode[10][3] = (char)m_Split[19][3] == 'N'?true:false;
                  
                pAvpInfo->bOption[11][0] = (char)m_Split[20][0] == 'M'?true:false;
                pAvpInfo->bNoEncode[11][0] = (char)m_Split[20][0] == 'N'?true:false;
                pAvpInfo->bOption[11][1] = (char)m_Split[20][1] == 'M'?true:false;
                pAvpInfo->bNoEncode[11][1] = (char)m_Split[20][1] == 'N'?true:false;
                pAvpInfo->bOption[11][2] = (char)m_Split[20][2] == 'M'?true:false;
                pAvpInfo->bNoEncode[11][2] = (char)m_Split[20][2] == 'N'?true:false;
                pAvpInfo->bOption[11][3] = (char)m_Split[20][3] == 'M'?true:false;
                pAvpInfo->bNoEncode[11][3] = (char)m_Split[20][3] == 'N'?true:false;
            }
        }
        else
        {
            printf("\n*******The error line as follows************\n%s\n",pLinePtr);
            printf("The error code line is [%s][%d]************\n",__FILE__,__LINE__);
            throw TException("\nTAvpFile::GetAvpInfoByText() Format Error\n");
        }
    }

    return pAvpInfo;
}

TAvpInfo * TAvpFile::GetAvpHead(void)
{
    return &m_AvpHead;
}

void TAvpFile::ReadConfig(const char *sFileName)
{
    FILE *fp = fopen(sFileName, "r");
    if (NULL == fp)
    {
        int iError = errno;
        throw TException("TAvpFile::ReadConfig() Open File [%s] Error. errno [%d], error_msg [%s]\n",
            (sFileName?sFileName:"NULL"), iError, strerror(iError));
    }
    else
    {
        printf("\nTAvpFile::ReadConfig() Open File . [%s]\n",sFileName);
    }
    this->Clear();

    TAvpInfo *pCurrAvpInfo=NULL,*pParentAvpInfo=&m_AvpHead; //357111 for cppcheck
    char sLineText[1024]={0}; //357111 for cppcheck
    int iIndex = 0;

    memset(sLineText,0,sizeof(sLineText));    
    while (fgets(sLineText,sizeof(sLineText),fp))
    {
        if(iIndex++==0)  continue;

        //得到一个配置项
        pCurrAvpInfo = GetAvpInfoByText(sLineText,(int)strlen(sLineText));

        //构造子结点
        if(pParentAvpInfo)
        {
            bool bFindParentFlag = false;
            if (pParentAvpInfo->AvpLevel < pCurrAvpInfo->AvpLevel)
            {
                bFindParentFlag = true;
            }
            else
            {
                while(pParentAvpInfo->ParentAvpInfo && bFindParentFlag == false)
                {
                    pParentAvpInfo = pParentAvpInfo->ParentAvpInfo;
                    bFindParentFlag = pParentAvpInfo->AvpLevel < pCurrAvpInfo->AvpLevel;
                }
            }

            if(bFindParentFlag)
            {
                m_pListCtrl->SetList(pParentAvpInfo->SubAvpItem);
                m_pListCtrl->Add(pCurrAvpInfo);

                pCurrAvpInfo->ParentAvpInfo = pParentAvpInfo;   
            }
        }

        //保存上一结点
        pParentAvpInfo = pCurrAvpInfo;

        //整理AVP_KEY值
        if(pCurrAvpInfo->ParentAvpInfo && pCurrAvpInfo->ParentAvpInfo->AvpCode>0)
        {
            //ZMP:467371 
            snprintf(pCurrAvpInfo->AvpKey, sizeof(pCurrAvpInfo->AvpKey), "%s|%d,%d,%d",pCurrAvpInfo->ParentAvpInfo->AvpKey,pCurrAvpInfo->AvpCode,pCurrAvpInfo->VendorId,pCurrAvpInfo->Index);
            pCurrAvpInfo->AvpKey[sizeof(pCurrAvpInfo->AvpKey)-1] = '\0';
        }
        else
        {
            snprintf(pCurrAvpInfo->AvpKey, sizeof(pCurrAvpInfo->AvpKey), "%d,%d,%d",pCurrAvpInfo->AvpCode,pCurrAvpInfo->VendorId,pCurrAvpInfo->Index);
            pCurrAvpInfo->AvpKey[sizeof(pCurrAvpInfo->AvpKey)-1] = '\0';
        }

        //插入HASH表
        m_pHashKey->Add(pCurrAvpInfo->AvpKey,pCurrAvpInfo);
        pCurrAvpInfo->HashCode = (unsigned int)m_pHashKey->GetKeyCode(pCurrAvpInfo->AvpKey);
    }
    fclose(fp);//ZMP:305593
    fp = NULL; //357111 for cppcheck
}

unsigned int TAvpFile::GetAvpType(const char *sAvpTypeStr)
{
    int iRetType = AVP_TYPE_CHAR;

    if (strcmp(sAvpTypeStr,"UTF8String")== 0   ||
        strcmp(sAvpTypeStr,"UTF8Str") == 0 ||
        strcmp(sAvpTypeStr,"OctetString") == 0 ||
        strcmp(sAvpTypeStr,"OctStr") == 0 ) 
    {
        iRetType = AVP_TYPE_CHAR;
    }
    else if (strcmp(sAvpTypeStr,"Unsigned32") == 0 || strcmp(sAvpTypeStr,"UINT32") == 0 || 
        strcmp(sAvpTypeStr,"Enumerated") == 0 || strcmp(sAvpTypeStr,"Enum") == 0 || 
        strcmp(sAvpTypeStr,"Time")== 0)
    {
        iRetType = AVP_TYPE_UINT;
    }
    else if(strcmp(sAvpTypeStr,"Unsigned64") == 0 || strcmp(sAvpTypeStr,"UINT64") == 0)
    {
        iRetType = AVP_TYPE_ULONG;
    }
    else if(strcmp(sAvpTypeStr,"Integer64")== 0 || strcmp(sAvpTypeStr,"INT64")== 0)
    {
        iRetType = AVP_TYPE_LONG;
    }
    else if (strcmp(sAvpTypeStr,"Integer32") == 0 || strcmp(sAvpTypeStr,"INT32") == 0 || 
        strcmp(sAvpTypeStr,"Enumerated")== 0 || strcmp(sAvpTypeStr,"Enum") == 0  )
    {
        iRetType = AVP_TYPE_INT;
    }
    else if (strcmp(sAvpTypeStr,"Grouped") == 0)
    {
        iRetType = AVP_TYPE_GROUP;
    }
    else if (strcmp(sAvpTypeStr,"Address") == 0 || 
        strcmp(sAvpTypeStr,"IPAddress")== 0 || strcmp(sAvpTypeStr,"IPAddr")== 0)
    {
        iRetType = AVP_TYPE_ADRESS;
    }
    else if (strcmp(sAvpTypeStr,"DiameterIdentity") == 0 || strcmp(sAvpTypeStr,"DccID") == 0)
    {
        iRetType = AVP_TYPE_CHAR;
    }
    else if (strcmp(sAvpTypeStr,"MEMString") == 0)
    {
        iRetType = AVP_TYPE_MEM;
    }         
    else
    {
        iRetType = AVP_TYPE_ERROR;
    }
    return (unsigned int)iRetType;
}

TAvpHead::TAvpHead()
{
    ClearData();
}
void TAvpHead::Assign(TBaseObject *pObject)//拷贝对象
{
    TAvpHead *avpHead = (TAvpHead *)pObject;
    Version = avpHead->Version;
    Length = avpHead->Length;
    CommandCode = avpHead->CommandCode;
    CommandFlag = avpHead->CommandFlag;
    AppId =avpHead->AppId;
    HopId =avpHead->HopId;
    EndId =avpHead->EndId;
}
void TAvpHead::ClearData(void)
{
    Version = 0;
    Length = 0;
    CommandCode = 0;
    CommandFlag = 0;
    AppId = 0;
    HopId = 0;
    EndId = 0;
}

TAvpNode::TAvpNode()
{
    ClearData();
    //ZMP:305593
    AvpCode       = 0;
    VendorId      = 0;
    bHaveVendorId = false;
    AvpIndex      = 0;
    AvpNode       = NULL;
    AvpSize       = 0;
    AvpNodeData   = NULL;
    AvpDataSize   = 0;
    AvpType       = 0;
    AvpGroup      = false;
    AvpLenLoc     = 0;
    HashCode      = 0;
    memset(DefaultValue, 0, sizeof(DefaultValue));
    
}

void TAvpNode::ClearData(void)
{
    memset(AvpKey,0,sizeof(AvpKey));
    memset(m_tTempValue,0,sizeof(m_tTempValue));
    ParentNode = NULL;
    AvpInfo = NULL;
    NextNode = NULL;
    SubNode = NULL;
    SubTailNode = NULL;
    AvpFlag = 0x40;
    bHaveVendorId = false;
    m_sValue[0]='\0';
}

int TAvpNode::Compare(int iKey) const
{
  throw TException("TAvpNode::Compare(int iKey) : Not Write Code.");
}

int TAvpNode::Compare(int iKey1, int iKey2) const
{
  throw TException("TAvpNode::Compare(int iKey1, int iKey2) : Not Write Code.");
}

int TAvpNode::Compare(const TBaseObject *pObject) const
{
    TAvpNode *pDataObject = (TAvpNode *)pObject;
    return strcmp(AvpKey,pDataObject->AvpKey);
}
//int TAvpNode::Compare(const TBaseObject *pObject) const
//{
//    TAvpNode *pDataObject = (TAvpNode *)pObject;
//    if(pDataObject->AvpCode == AvpCode)
//    {
//        return VendorId - pDataObject->VendorId;
//    }
//    else
//    {
//        return AvpCode - pDataObject->AvpCode;
//    }
//}

const char * TAvpNode::AsValue(void) const
{
    return m_sValue;
}
llong TAvpNode::AsInteger()
{
    //llong iRet;
    //switch(AvpType)
    //{
    //case AVP_TYPE_INT:
    //    TAvpParser::CnvtBinToDec((unsigned char *)m_sValue,AvpDataSize,(int *)(&iRet));
    //    break;
    //case AVP_TYPE_UINT:
    //    TAvpParser::CnvtUintToDec((unsigned char *)m_sValue,AvpDataSize,(unsigned int *)(&iRet));
    //    break;
    //case AVP_TYPE_LONG:
    //    TAvpParser::CnvtLlongBinToDec((unsigned char *)m_sValue,AvpDataSize,(long long *)&iRet);
    //    break;
    //case AVP_TYPE_ULONG:
    //    TAvpParser::CnvtUllongBinToDec((unsigned char *)m_sValue,AvpDataSize,(unsigned long long *)&iRet);
    //    break;
    //default:
    //    iRet = 0;
    //    break;
    //}
    //return iRet;
    return (llong)atol(m_sValue);
}
char *TAvpNode::AsString()
{
    //switch(AvpType)
    //{
    //case AVP_TYPE_CHAR:
    //    TAvpParser::CnvtByteToStr((unsigned char *)m_sValue,AvpDataSize,m_tTempValue);
    //    break;
    //case AVP_TYPE_ADRESS:
    //    TAvpParser::CnvtDmtIPToIpv((unsigned char *)m_sValue,AvpDataSize,m_tTempValue);
    //    break;
    //default:
    //    break;
    //}
    //return m_tTempValue;
    return m_sValue;
}

string TAvpNode::AsOctetString()
{
    ostringstream strOut;
    char sHex[5]={0};

    unsigned char sValue[2048] = {0};
    unsigned int iDataSize = 0;
    
    if(AvpDataSize < 2048)
    {
        iDataSize = AvpDataSize;
    }
    else
    {
        iDataSize = 2047;
    }
    
    memcpy(sValue,AvpNodeData,iDataSize);
    
    for(unsigned int i=0;i<iDataSize;i++)
    {
        snprintf(sHex, sizeof(sHex), "%02X ", sValue[i]);
        sHex[sizeof(sHex)-1] = '\0';
        strOut<<sHex;
        memset(sHex, 0, sizeof(sHex));
    }

    return strOut.str();
}

void TAvpNode::SetValue(const char *sValue)
{
    if(sValue!=NULL)
    {
        strncpy(m_sValue,sValue,sizeof(m_sValue)-1);
        m_sValue[sizeof(m_sValue)-1]='\0';
    }
    else
    {
        m_sValue[0]='\0';
    }
}

void TAvpNode::SetValue(const char *sValue,int iSize)
{
    if(sValue == NULL || iSize == 0)
    {
        m_sValue[0]='\0';
    }
    else
    {
        size_t iLength = (size_t)iSize < sizeof(m_sValue)? (size_t)iSize:sizeof(m_sValue)-1;
        memcpy(m_sValue, sValue, iLength);
        m_sValue[iLength] = '\0';
    }
}

TDiamterInfo::TDiamterInfo(unsigned int iAvpListSize)
{
    AvpListSize = iAvpListSize;
    AvpNodeHead.AvpCode = 0;
    AvpNodeHead.VendorId = 0;
    AvpNodeHead.AvpIndex = 0;
    memset(AvpNodeHead.AvpKey,0,sizeof(AvpNodeHead.AvpKey));
    AvpHead = new TAvpHead();
    AvpNodes = new TAvpNode * [iAvpListSize];
    for(unsigned int i=0;i<iAvpListSize;i++)
    {
        AvpNodes[i] = new TAvpNode();
    }
    //ZMP:305593
    AvpNodeSize = 0;
}

TDiamterInfo::~TDiamterInfo()
{
    for(unsigned int i=0;i<AvpListSize;i++)
    {
        DELETE(AvpNodes[i]);
    }
    DELETE_A(AvpNodes);
    DELETE(AvpHead);
}

void TDiamterInfo::ClearData(void)
{
    AvpHead->ClearData();
    for(unsigned int i=0;i<AvpListSize;i++)
    {
        AvpNodes[i]->ClearData();
    }
    AvpNodeHead.ClearData();
}
TAvpOperItem::TAvpOperItem()
{
    //ZMP:305593
    memset(Avpkey, 0, sizeof(Avpkey));
    memset(AttrName, 0, sizeof(AttrName));
    memset(DefaultValue, 0, sizeof(DefaultValue));
    AttrId = 0;
    AvpFlag = ' ';
    HashCode = 0; 
}
TAvpOperItem::~TAvpOperItem()
{
}

int TAvpOperItem::Compare(int iKey) const
{
  throw TException("TAvpOperItem::Compare(int iKey) : Not Write Code.");
}

int TAvpOperItem::Compare(int iKey1, int iKey2) const
{
  throw TException("TAvpOperItem::Compare(int iKey1, int iKey2) : Not Write Code.");
}

int TAvpOperItem::Compare(const TBaseObject *pObject) const
{
    TAvpOperItem *pItemInfo = (TAvpOperItem *)pObject;
    return strcmp((const char*)Avpkey,(const char*)pItemInfo->Avpkey);
}

string TAvpOperItem::ToString()
{
    ostringstream sLineText;
    char sLineInfo[1024]={0}; //357111 for cppcheck
    //ZMP:467371 
    snprintf(sLineInfo, sizeof(sLineInfo), "%-16s%-6u%-16s%3u%-16s",//ZMP:305593
        AttrName,AttrId,DefaultValue,HashCode,Avpkey);
    sLineInfo[sizeof(sLineInfo)-1] = '\0';
    sLineText<<sLineInfo;
    return sLineText.str();
}
TAvpOperStruct::TAvpOperStruct()
{
    m_pHashKey = new THashList();
    m_pHashKey->SetHashNum(509);
    m_pAvpOperItem = new TAvpOperItem[128];
    m_Split.SetSplitter(SPLIT_CHAR);
    m_iIndex = 0;
}
TAvpOperStruct::~TAvpOperStruct()
{
    DELETE(m_pHashKey);
    DELETE_A(m_pAvpOperItem);
}
void TAvpOperStruct::ReadConfig(const char *sFileName)
{
    FILE *fp = fopen(sFileName, "r");
    if (NULL == fp)
    {
        int iError = errno;
        throw TException("TAvpOperStruct::ReadConfig() Open File [%s] Error. errno [%d], error_msg [%s]\n",
            (sFileName?sFileName:"NULL"), iError, strerror(iError));
    }
    m_pHashKey->Clear();

    char sLineText[1024]  = {0};//ZMP:477242
    int iIndex = 0;

    //memset(sLineText,0,sizeof(sLineText));    
    while (fgets(sLineText,sizeof(sLineText),fp) && iIndex<128)
    {
        if(iIndex++==0) continue;

        char *pLinePtr = (char *)sLineText;
        //TAvpInfo *pAvpInfo = NULL;//ZMP:305593该变量未用到
        int iLineLength = (int)strlen(sLineText);
        //整理输入行
        {
            char pTrimChar[]={0x20,0x0D,0x0A,0x09};//需要过虑掉的字符 空格/回车符/换行符/TAB        
            bool bInstead = true;
            //bool bHasParentNode = false;//ZMP:305593该变量未用到
            int iCopyIndex=0;
            for (int iCurrIndex=0;iCurrIndex<iLineLength;iCurrIndex++)
            {

                if (pLinePtr[iCurrIndex] != pTrimChar[3] && pLinePtr[iCurrIndex] != pTrimChar[0] &&
                    pLinePtr[iCurrIndex] != pTrimChar[1] && pLinePtr[iCurrIndex] != pTrimChar[2])
                {
                    //有效字符，无条件COPY
                    bInstead = true;
                    pLinePtr[iCopyIndex] = pLinePtr[iCurrIndex];
                    iCopyIndex++;
                }
                else if (bInstead && pLinePtr[iCurrIndex] == SPLIT_CHAR)
                {
                    //第一无效字符，需要COPY
                    bInstead = false;
                    pLinePtr[iCopyIndex] = pLinePtr[iCurrIndex];
                    iCopyIndex++;
                }
            }
            memset(pLinePtr+iCopyIndex,0,(size_t)(iLineLength-iCopyIndex));
            TStrFunc::Trim(pLinePtr,pTrimChar,sizeof(pTrimChar));
        }


        //分割数据
        {       
            if (m_Split.SetString(pLinePtr) == OPER_CONFIG_NUMBER)
            {
                char pErrorChar[]={'{','}','[',']','<','>','*'};

                memcpy(m_pAvpOperItem[iIndex].AttrName,m_Split[0],sizeof(m_pAvpOperItem[iIndex].AttrName));
                TStrFunc::Trim((char *)m_pAvpOperItem[iIndex].AttrName,pErrorChar,sizeof(pErrorChar));
                m_pAvpOperItem[iIndex].AttrId = (unsigned int)atoi(m_Split[1]);
                m_pAvpOperItem[iIndex].AvpFlag = (unsigned char)((char)*m_Split[2] == 'M'?0x40:0x00);
                memcpy(m_pAvpOperItem[iIndex].DefaultValue,m_Split[3],sizeof(m_pAvpOperItem[iIndex].DefaultValue));
                TStrFunc::Trim((char *)m_pAvpOperItem[iIndex].DefaultValue,pErrorChar,sizeof(pErrorChar));
                memcpy(m_pAvpOperItem[iIndex].Avpkey,m_Split[4],sizeof(m_pAvpOperItem[iIndex].Avpkey));
                TStrFunc::Trim((char *)m_pAvpOperItem[iIndex].Avpkey,pErrorChar,sizeof(pErrorChar));
            }
            else
            {
                fclose(fp);//ZMP:305593
                fp = NULL; //357111 for cppcheck
                throw TException("TAvpOperStruct::ReadConfig Format Error\n");
            }
        }

        //插入HASH表
        m_pHashKey->Add((const char*)m_pAvpOperItem[iIndex].Avpkey,&m_pAvpOperItem[iIndex]);
        m_pAvpOperItem[iIndex].HashCode = (unsigned int)m_pHashKey->GetKeyCode((const char*)m_pAvpOperItem[iIndex].Avpkey);
    }
    m_iIndex = iIndex;
    fclose(fp);//ZMP:305593
    fp = NULL; //357111 for cppcheck
}

TAvpOperItem * TAvpOperStruct::GetAvpOperItem (const char *sKey)
{
    memset(m_tAvpOperItem.Avpkey,0,sizeof(m_tAvpOperItem.Avpkey));
    strncpy(m_tAvpOperItem.Avpkey,sKey,sizeof(m_tAvpOperItem.Avpkey));

    TAvpOperItem *pAvpOperItem = (TAvpOperItem *)m_pHashKey->Seek(sKey,&m_tAvpOperItem);

    return pAvpOperItem;
}

bool TAvpOperStruct::SetItemValue(const char *sKey,const char *newVlaue)
{
    bool bRetFlag = true;
    TAvpOperItem * pTempItem = GetAvpOperItem(sKey);
    if (pTempItem != NULL)
    {
        if (strlen(newVlaue) > 0)
        {
            memset(pTempItem->DefaultValue,0,sizeof(pTempItem->DefaultValue));
            memcpy(pTempItem->DefaultValue,newVlaue,sizeof(pTempItem->DefaultValue));
        }        
    }
    else if((strlen(sKey) > 0) && (m_iIndex < 128))
    {
        m_iIndex++;
        //ZMP:467371 
        strncpy(m_pAvpOperItem[m_iIndex].AttrName,"AOD",sizeof(m_pAvpOperItem[m_iIndex].AttrName)-1);
        m_pAvpOperItem[m_iIndex].AttrName[sizeof(m_pAvpOperItem[m_iIndex].AttrName)-1] = '\0';
        m_pAvpOperItem[m_iIndex].AttrId = 999;
        m_pAvpOperItem[m_iIndex].AvpFlag = 0x00;
        if (strlen(newVlaue) > 0)
        {
            memcpy(m_pAvpOperItem[m_iIndex].DefaultValue,newVlaue,sizeof(m_pAvpOperItem[m_iIndex].DefaultValue));
        }
        memcpy(m_pAvpOperItem[m_iIndex].Avpkey,sKey,sizeof(m_pAvpOperItem[m_iIndex].Avpkey));
        //插入HASH表
        m_pHashKey->Add((const char*)m_pAvpOperItem[m_iIndex].Avpkey,&m_pAvpOperItem[m_iIndex]);
        m_pAvpOperItem[m_iIndex].HashCode = (unsigned int)m_pHashKey->GetKeyCode((const char*)m_pAvpOperItem[m_iIndex].Avpkey);
    }
    else
    {
        bRetFlag = false;
    }
    return bRetFlag;
}

TDmtOper::TDmtOper()
{
    //m_pHashKey = new THashList();
    //m_pHashKey->SetHashNum(1000);
    m_pDmtData = new TDiamterInfo();
    m_pListCtrl = new TBaseList();
    m_pAvpFile = new TAvpFile();
    m_pAvpStruct = new TAvpOperStruct();
    m_pAvpSingle = new TAvpSingle();
    m_pAvpParser = new TAvpParser(m_pAvpFile);
    m_pAvpEncode = new TAvpEncode(m_pAvpFile);
    m_iServiceType = 2;
    m_iCcrType = 0;
    m_iIndex = 0;
    m_bHasFresh = true;
    m_iConIndex = 0;
    for(unsigned int i=0;i<256;i++)
    {
        m_pConNode[i] = new TAvpNode();
    }
    Split.SetSplitter('|');
    m_bCloneConfig = true;
}
TDmtOper::~TDmtOper(void)
{
    //DELETE(m_pHashKey);
    DELETE(m_pDmtData);
    DELETE(m_pAvpSingle);
    if (m_bCloneConfig)
    {
        DELETE(m_pAvpFile);
        DELETE(m_pAvpStruct);
    }
    DELETE(m_pAvpParser);
    DELETE(m_pAvpEncode);
    DELETE(m_pListCtrl);

    for(unsigned int i=0;i<256;i++)
    {
        DELETE(m_pConNode[i]);
    }
}

void TDmtOper::ReadConfig(TAvpFile * pAvpfile,TAvpOperStruct * pAvpStruct,bool NeedOperStuct)
{
    if (pAvpfile != NULL && pAvpStruct != NULL)
    {
        if (m_bCloneConfig)
        {
            DELETE(m_pAvpFile);
            DELETE(m_pAvpStruct);
            m_bCloneConfig = false;
        }
        m_pAvpFile = pAvpfile;
        m_pAvpParser->SetConfigInfo(m_pAvpFile);
        m_pAvpEncode->SetConfigInfo(m_pAvpFile);
        m_pAvpStruct = pAvpStruct;
        if (NeedOperStuct)
        {
            bool bRetFlag = Cnvt2DmtStruct();
            if (bRetFlag)
            {
                m_bHasFresh = false;
            }
        }
    }
}
void TDmtOper::ReadConfig(const char *sFileName,const char *sStrFileName,bool NeedOperStuct)
{
    m_pAvpFile->ReadConfig(sFileName);
    if (NeedOperStuct)
    {
        m_pAvpStruct->ReadConfig(sStrFileName);
        bool bRetFlag = Cnvt2DmtStruct();
        if (bRetFlag)
        {
            m_bHasFresh = false;
        }
    }
}
void TDmtOper::SetDmtSerType(int iSerType,int iCcrType)
{
    if (iSerType > 0 && (iSerType < 12) && iCcrType>-1 && iCcrType<5)
    {
        m_iServiceType = iSerType - 1;
        m_iCcrType = iCcrType-1;
    }
    else if (iSerType >= 12  && iSerType <= 13 && iCcrType>-1 && iCcrType<5)
    {
        m_iServiceType = iSerType-2;
        m_iCcrType = iCcrType-1;
    }

    m_pAvpEncode->SetMsgSerType(iSerType,iCcrType);
}
void TDmtOper::SetMsgHeadInfo(TAvpHead *pInfo)
{
    //m_pHashKey->Clear();
    m_iConIndex = 0;  
    m_pDmtData->AvpHead->Assign(pInfo);
}
bool TDmtOper::Cnvt2DmtStruct()
{
    bool bRetFlag = true;
    if (m_pDmtData == NULL)
    {
        bRetFlag = false;
    }
    else
    {
        //m_pHashKey->Clear();
        TAvpInfo * pCurrAvpInfo = NULL;
        if (m_pAvpFile!=NULL)
        {
            m_iIndex = 0;
            pCurrAvpInfo = m_pAvpFile->GetAvpHead();
            TBaseListItem * pMemItem[16] = {0};
            TAvpNode * pNodeMemItem[16] = {0};
            TAvpNode * pBroNodeMemItem[16] = {0};
            int iLevel = 0;
            TBaseListItem *pItem = pCurrAvpInfo->SubAvpItem;
            TAvpNode *pCurrAvpNode = &(m_pDmtData->AvpNodeHead);
            pNodeMemItem[0] = pCurrAvpNode;
            while (bRetFlag && (iLevel>0 || pItem != NULL))
            {
                while (pItem == NULL && iLevel>0)
                {
                    pItem = pMemItem[--iLevel]->next;
                    if (pItem != NULL)
                    {
                        pCurrAvpNode = pBroNodeMemItem[iLevel];
                        break;
                    }
                }
                pCurrAvpInfo = (TAvpInfo *)pItem->data;
                if(!GetEncodeFlag(m_pAvpStruct,pCurrAvpInfo))
                {
                    pItem = pItem->next;
                    continue;
                }
                pCurrAvpNode = SetDmtNode(pNodeMemItem[iLevel],pCurrAvpNode,pCurrAvpInfo);
                TAvpOperItem *pAvpItem = m_pAvpStruct->GetAvpOperItem(pCurrAvpInfo->AvpKey);
                if (pAvpItem != NULL)
                {
                    pCurrAvpNode->AvpFlag = (unsigned char)(pCurrAvpNode->AvpFlag|pAvpItem->AvpFlag);
                    if (strlen(pAvpItem->DefaultValue) > 0)
                    {
                        memset(pCurrAvpNode->DefaultValue,0,sizeof(pCurrAvpNode->DefaultValue));
                        memcpy(pCurrAvpNode->DefaultValue,pAvpItem->DefaultValue,sizeof(pCurrAvpNode->DefaultValue));
                    }
                }
                //递归获取avpfile的每一项,采用堆栈数组pMemItem实现顺序获取
                if (pCurrAvpInfo->IsGroup)
                {
                    pBroNodeMemItem[iLevel] = pCurrAvpNode;
                    pMemItem[iLevel++] = pItem;
                    pNodeMemItem[iLevel] = pCurrAvpNode;
                    m_pListCtrl->Reset();
                    m_pListCtrl->SetList(pCurrAvpInfo->SubAvpItem);
                    pItem = m_pListCtrl->GetFirstItem();
                }
                else
                {
                    pItem = pItem->next;
                }
            }
        }
        m_pDmtData->AvpNodeSize = (unsigned int)m_iIndex;
    }
    return bRetFlag;
}

TAvpNode * TDmtOper::SetDmtNode(TAvpNode *pParentNode,TAvpNode *pNode,TAvpInfo *pSrcData)
{
    //取一个未使用的结构
    TAvpNode *pAvpNode = m_pDmtData->AvpNodes[m_iIndex];

    //清除数据
    pAvpNode->ClearData();

    //设置Dmt数据节点信息，从avpInfo中获取
    pAvpNode->AvpCode = pSrcData->AvpCode;
    pAvpNode->VendorId = pSrcData->VendorId;
    pAvpNode->AvpGroup = pSrcData->IsGroup;
    pAvpNode->AvpType = pSrcData->AvpType;
    pAvpNode->AvpFlag = (unsigned char)((pSrcData->bMustFlag?0x40:0x00)|(pSrcData->VendorId>0?0x80:0x00));//ZMP:477242 
    memcpy(pAvpNode->DefaultValue,pSrcData->cDefaultValue,sizeof(pSrcData->cDefaultValue));

    //构造父子树形结构
    if (m_iIndex == 0 || pNode->AvpGroup)
    {
        pAvpNode->ParentNode = pParentNode;
        if (pNode->ParentNode == pParentNode)
        {
            pNode->NextNode = pAvpNode;
            pParentNode->SubTailNode = pAvpNode;
        }
        else
        {
            pNode->SubNode = pAvpNode;
            pNode->SubTailNode = pAvpNode;
        }
    }
    else
    {
        pAvpNode->ParentNode = pParentNode;
        pNode->NextNode = pAvpNode;
        pParentNode->SubTailNode = pAvpNode;
    }
    pAvpNode->NextNode = NULL;
    pAvpNode->AvpIndex = 1;
    //ZMP:467371   
    strncpy(pAvpNode->AvpKey,pSrcData->AvpKey, sizeof(pAvpNode->AvpKey)-1);
    pAvpNode->AvpKey[sizeof(pAvpNode->AvpKey)-1] = '\0';

    //读取配置文件信息
    pAvpNode->AvpInfo = m_pAvpFile->GetAvpInfo(pAvpNode->AvpKey);
    if(pAvpNode->AvpInfo)
    {
        pAvpNode->AvpInfo->AvpNode = pAvpNode;
        //指向下一个结构体
        m_iIndex++;
        //插入HASH表
        //m_pHashKey->Add((const char*)pAvpNode->AvpKey,pAvpNode);
        //pAvpNode->HashCode = (unsigned int)m_pHashKey->GetKeyCode((const char*)pAvpNode->AvpKey);
    }
    else
    {
        if (pNode->NextNode == pAvpNode)
        {
            pAvpNode->ParentNode->SubTailNode = pNode;
        }
        else if (pNode->SubNode == pAvpNode)
        {
            pNode->SubNode = NULL;
            pNode->SubTailNode = NULL;
        }
        pAvpNode = pAvpNode->ParentNode;
    }

    return pAvpNode;
}

bool TDmtOper::DmtEncode(unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize)
{
    bool bRetFlag = true;
    if (m_bHasFresh)
    {
        bRetFlag = Cnvt2DmtStruct();
        m_bHasFresh = false;
    }
    if (bRetFlag)
    {
        bRetFlag = m_pAvpEncode->Encode(m_pDmtData,pDataBuffer,iDataSize,iRealSize);
    }
    return bRetFlag;
}

void TDmtOper::DmtPrepareEncode()
{
    //bool bRetFlag = true;//ZMP:305593该变量未用到
    if (m_bHasFresh)
    {
        Cnvt2DmtStruct();
        m_bHasFresh = false;
    }
    m_pAvpEncode->SetMsgHeadInfo(m_pDmtData->AvpHead);
    //m_pAvpEncode->SetDmtHash(m_pHashKey);
}

//del//bool TDmtOper::DmtEncode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize)
//del//{
//del//    bool bRetFlag = true;
//del//    if (m_bHasFresh)
//del//    {
//del//        bRetFlag = Cnvt2DmtStruct();
//del//        m_bHasFresh = false;
//del//    }
//del//    m_pAvpEncode->SetMsgHeadInfo(m_pDmtData->AvpHead);
//del//    m_pAvpEncode->SetDmtHash(m_pHashKey);
//del//    if (bRetFlag)
//del//    {
//del//        bRetFlag = m_pAvpEncode->Encode(pEvent,pDataBuffer,iDataSize,iRealSize);
//del//    }
//del//    return bRetFlag;
//del//}

bool TDmtOper::SetItemValue(const char *sKey,const char *newVlaue)
{
    bool bRetFlag = true;
    bRetFlag = m_pAvpStruct->SetItemValue(sKey,newVlaue);
    if (bRetFlag)
    {
        m_bHasFresh = true;
    }
    return bRetFlag;
}    
bool TDmtOper::ModifyItemValue(const char *sKey,const char *newVlaue)
{
    return true;
    /*
    bool bRetFlag = true;
    if (m_bHasFresh)
    {
        bRetFlag = Cnvt2DmtStruct();
        m_bHasFresh = false;
    }
    if (strlen(sKey) > 0)
    {    
        memset(m_tAvpNode.AvpKey,0,sizeof(m_tAvpNode.AvpKey));
        strncpy(m_tAvpNode.AvpKey,sKey,sizeof(m_tAvpNode.AvpKey));
        TAvpNode *pNode = (TAvpNode *)m_pHashKey->Seek(sKey,&m_tAvpNode);
        if (pNode != NULL)
        {
            memset(pNode->DefaultValue,0,sizeof(pNode->DefaultValue));
            if (strlen(newVlaue) > 0)
            {
                memcpy(pNode->DefaultValue,newVlaue,sizeof(pNode->DefaultValue));
            }
        }
        else
        {
            if (m_iConIndex%256 == 0)
            {
                m_iConIndex = 0;
            }
            TAvpInfo * pInfo = m_pAvpFile->GetAvpInfo(sKey);
            if (pInfo != NULL && !pInfo->IsGroup)
            {
                char sTempKey[128] = {0};
                int iNumber = Split.SetString(sKey);
                int i = 0;
                for (i=iNumber;i>=1 && pNode==NULL;i--)
                {
                    memset(sTempKey,0,sizeof(sTempKey));
                    for (int j=0;j<i-1;j++)
                    {
                        strcat(sTempKey,Split[(unsigned int)j]);
                        if (j+1<i-1)
                        {
                            strcat(sTempKey,"|");
                        }
                    }
                    memset(m_tAvpNode.AvpKey,0,sizeof(m_tAvpNode.AvpKey));
                    strncpy(m_tAvpNode.AvpKey,sTempKey,sizeof(m_tAvpNode.AvpKey));
                    pNode = (TAvpNode *)m_pHashKey->Seek(sTempKey,&m_tAvpNode);
                    if (pNode==NULL && i==1)
                    {
                        pNode = &(m_pDmtData->AvpNodeHead);
                    }
                }
                if (pNode != NULL)
                {
                    TAvpNode *pParent = pNode;
                    if (i>0)
                    {
                        strcat(sTempKey,"|");
                    }
                    while (i<iNumber)
                    {
                        strcat(sTempKey,Split[(unsigned int)(i++)]);
                        TAvpInfo * pInfoTemp = m_pAvpFile->GetAvpInfo(sTempKey);
                        if (pInfoTemp == NULL)
                        {
                            bRetFlag = false;
                            break;
                        }
                        m_pConNode[m_iConIndex]->AvpType = pInfoTemp->AvpType;
                        m_pConNode[m_iConIndex]->AvpCode = pInfoTemp->AvpCode;
                        m_pConNode[m_iConIndex]->AvpGroup = pInfoTemp->IsGroup;
                        memcpy(m_pConNode[m_iConIndex]->AvpKey,pInfoTemp->AvpKey,sizeof(m_pConNode[m_iConIndex]->AvpKey));
                        m_pConNode[m_iConIndex]->VendorId = pInfoTemp->VendorId;

                        if (pParent->SubTailNode == NULL)
                        {
                            pParent->SubNode = m_pConNode[m_iConIndex];
                            pParent->SubTailNode = m_pConNode[m_iConIndex];
                        }
                        else
                        {
                            pParent->SubTailNode->NextNode = m_pConNode[m_iConIndex];
                        }
                        m_pConNode[m_iConIndex]->ParentNode = pParent;
                        pParent->SubTailNode = m_pConNode[m_iConIndex];
                        //m_pConNode[m_iConIndex]->NextNode = pParent->SubNode;
                        //pParent->SubNode = m_pConNode[m_iConIndex];
                        pParent = m_pConNode[m_iConIndex];

                        if (!pInfoTemp->IsGroup && i<iNumber)
                        {
                            bRetFlag = false;
                            break;
                        }
                        else if(!pInfoTemp->IsGroup && i==iNumber)
                        {
                            if (strlen(newVlaue) > 0)
                            {
                                memset(m_pConNode[m_iConIndex]->DefaultValue,0,sizeof(m_pConNode[m_iConIndex]->DefaultValue));
                                memcpy(m_pConNode[m_iConIndex]->DefaultValue,newVlaue,sizeof(m_pConNode[m_iConIndex]->DefaultValue)); //351951 for cppcheck
                            }
                        }
                        //插入HASH表
                        m_pHashKey->Add((const char*)m_pConNode[m_iConIndex]->AvpKey,m_pConNode[m_iConIndex]);
                        m_pConNode[m_iConIndex]->HashCode = (unsigned int)m_pHashKey->GetKeyCode((const char*)m_pConNode[m_iConIndex]->AvpKey);
                        if (i<iNumber)
                        {
                            strcat(sTempKey,"|");
                        }
                        m_iConIndex++;
                    }
                }
                else
                {
                    bRetFlag = false;
                }
            }
            else
            {
                bRetFlag = false;
            }
        }
    }
    else
    {
        bRetFlag = false;
    }
    return bRetFlag;
    */
}


bool TDmtOper::DmtParser(unsigned char *pDataBuffer,unsigned int iDataSize)
{
    bool bRetFlag = true;
    bRetFlag = m_pAvpParser->Parser(pDataBuffer,iDataSize);
    return bRetFlag;
}
TAvpNode * TDmtOper::GetAvpNode(const char *sKey)
{
    TAvpNode * pNodeTemp = NULL;
    pNodeTemp = m_pAvpParser->GetAvpNodeInfo(sKey);
    return pNodeTemp;
}

//del//bool TDmtOper::SetRatableEvent(TRatableEvent *pRatableEvent)
//del//{
//del//    bool bRetFlag = true;
//del//    bRetFlag = m_pAvpParser->SetRatableEvent(pRatableEvent);
//del//    return bRetFlag;
//del//}

TDiamterInfo * TDmtOper::GetDiamterInfo(void)
{
    TDiamterInfo * pDmtTemp = NULL;
    pDmtTemp = m_pAvpParser->GetDiamterInfo();
    return pDmtTemp;
}
bool TDmtOper::SetDmtSrcData(unsigned char *pAvp,int iOrgDataLen)
{
    bool bRetFlag = true;
    bRetFlag = m_pAvpSingle->SetAvpSrcData(pAvp,iOrgDataLen);
    return bRetFlag;
}
bool TDmtOper::GetAvpByKey(const char *cFinding,unsigned char *pDataBuffer,int size,int *iDataSize,int *iDataType)
{
    bool bRetFlag = true;
    int iRet = 0;
    iRet = m_pAvpSingle->GetAvpByCondition(0,0,cFinding);
    if (iRet>=0 || iRet>size)
    {
        *iDataSize = iRet;
        memcpy(pDataBuffer,m_pAvpSingle->m_cResultDataBuf,(size_t)iRet);
        TAvpInfo *pInfoTemp = m_pAvpFile->GetAvpInfo(cFinding);
        if (pInfoTemp != NULL)
        {
            *iDataType = (int)pInfoTemp->AvpType;
        }
    }
    else
    {
        bRetFlag = false;
    }
    return bRetFlag;
}
string TDmtOper::ToString()
{
    return this->m_pAvpParser->ToString();
}
bool TDmtOper::GetEncodeFlag(TAvpOperStruct *pAvpStruct,TAvpInfo *pAvpInfo)
{
    bool bRetFlag = false;
    TAvpInfo * pInfoTemp = NULL;
    
    //分离:每个来源独立判断, DmtOper模式下不考虑attr的情况
    //bRetFlag = m_pAvpEncode->GetEncodeFlag(pAvpInfo,true,false);
    if (!bRetFlag && pAvpStruct != NULL)
    {
        if (pAvpInfo->IsGroup)
        {
            TBaseListItem *pItem = pAvpInfo->SubAvpItem;
            while (pItem != NULL)
            {
                pInfoTemp = (TAvpInfo *)pItem->data;
                bRetFlag = GetEncodeFlag(pAvpStruct,pInfoTemp);
                if (bRetFlag)
                {
                    break;
                }
                pItem = pItem->next;
            }
        }
        TAvpOperItem *pAvpItem = pAvpStruct->GetAvpOperItem(pAvpInfo->AvpKey);
        if (pAvpItem != NULL)
        {
            bRetFlag = true;
        }
    }
    return bRetFlag;
}
TAvpParser::TAvpParser(TAvpFile *pAvpFile,unsigned int iAvpListSize)
{
    m_pAvpFile = pAvpFile;
    m_pDiamterInfo = new TDiamterInfo(iAvpListSize);
    //ZMP:305593
    memset(m_pDataBuffer, 0, sizeof(m_pDataBuffer));
    memset(m_sErrorMsg, 0, sizeof(m_sErrorMsg));
    m_iDataSize = 0;
    m_iNodeIndex = 0;
    m_iErrorCode = 0;
}

TAvpParser::~TAvpParser()
{
    DELETE(m_pDiamterInfo);
}

unsigned int TAvpParser::GetAvpNode(unsigned char *pDataBuffer)
{
    unsigned int iRetData = 0;
    iRetData = (unsigned int)(pDataBuffer[0]*256*256*256+pDataBuffer[1]*256*256+pDataBuffer[2]*256+pDataBuffer[3]);
    return iRetData;
}

unsigned int TAvpParser::GetVendorId(unsigned char *pDataBuffer)
{
    unsigned int iRetData = 0;

    if ((pDataBuffer[0] & V_BODY_FLAG) == V_BODY_FLAG)
    {
        iRetData = (unsigned int)(pDataBuffer[4]*256*256*256+pDataBuffer[5]*256*256+pDataBuffer[6]*256+pDataBuffer[7]);
    }
    return iRetData;
}
bool TAvpParser::HaveVendorId(unsigned char *pDataBuffer)
{

    if ((pDataBuffer[0] & V_BODY_FLAG) == V_BODY_FLAG)
    {
        return true;
    }
    return false;
}

unsigned int TAvpParser::GetAvpSize(unsigned char *pDataBuffer)
{
    unsigned int iRetData = 0;
    iRetData = (unsigned int)(pDataBuffer[0]*256*256+pDataBuffer[1]*256+pDataBuffer[2]);
    return iRetData;
}

//输出对象信息
string TAvpParser::ToString()
{
    ostringstream sAddText;
    char sFlag1[3] = "00";
    char sFlag2[3] = "80";

    sAddText << "  TDiamterInfo" << endl;
    sAddText << "  ========================================================" << endl;
    sAddText << "    Version = " << m_pDiamterInfo->AvpHead->Version << endl;
    sAddText << "    Length = " << m_pDiamterInfo->AvpHead->Length << endl;
    sAddText << "    CommandCode = " << m_pDiamterInfo->AvpHead->CommandCode << endl;
    if (m_pDiamterInfo->AvpHead->CommandFlag==0)
    {
        sAddText << "    CommandFlag = " << sFlag1<< endl;
    }
    else
    {
        sAddText << "    CommandFlag = " << sFlag2<< endl;
    }
    sAddText << "    AppId = " << m_pDiamterInfo->AvpHead->AppId << endl;
    sAddText << "    HopId = " << m_pDiamterInfo->AvpHead->HopId << endl;
    sAddText << "    EndId = " << m_pDiamterInfo->AvpHead->EndId << endl;
    sAddText << "  --------------------------------------------------------"<< endl;

    TAvpInfo *pAvpInfo = NULL;
    //unsigned char sValue[255] = {0};//ZMP:305593该变量未用到
    //unsigned int  iValue = 0;//ZMP:305593该变量未用到
    //char cValue[255] = {0};//ZMP:305593该变量未用到
    for(unsigned int i=0;i<m_pDiamterInfo->AvpNodeSize;i++)
    {
        pAvpInfo = m_pDiamterInfo->AvpNodes[i]->AvpInfo;
        if(pAvpInfo==NULL) 
        {
            continue;
        }

        for(int j=1;j<=pAvpInfo->AvpLevel;j++) 
        {
            sAddText << "  " ;
        }

        if(pAvpInfo->IsGroup)
        {
            sAddText << " +" << pAvpInfo->AvpName << endl;
            continue;
        }

        GetAvpValue(m_pDiamterInfo->AvpNodes[i]);
        if (pAvpInfo->bIsOctetString)
        {
            sAddText << "  " << pAvpInfo->AvpName << " = [" << m_pDiamterInfo->AvpNodes[i]->AsOctetString() << "]" << endl;
        }
        else
        {
            sAddText << "  " << pAvpInfo->AvpName << " = [" << m_pDiamterInfo->AvpNodes[i]->AsValue() << "]" << endl;
        }
    }

    return sAddText.str();
}
void TAvpParser::SetConfigInfo(TAvpFile *pAvpFile)
{
    if (pAvpFile != NULL)
    {
        m_pAvpFile = pAvpFile;
    }
}
bool TAvpParser::GetAvpValue(TAvpNode *pAvpNode)
{
    if(pAvpNode==NULL || pAvpNode->AvpInfo==NULL) return false;

    if(pAvpNode->AvpDataSize==0) 
    {
        pAvpNode->SetValue(NULL);
        return true;
    }

    unsigned char sValue[255] = {0};
    unsigned int  iValue = 0;
    int iValueTemp = 0;
    unsigned long long lValue = 0;
    long long lValueTemp = 0;
    char cValue[255] = {0};

    // 增加长度校验
    if(pAvpNode->AvpDataSize >= 255)
    {
        pAvpNode->SetValue(NULL);
        return false;
    }

    memcpy(sValue,pAvpNode->AvpNodeData,pAvpNode->AvpDataSize);
    switch(pAvpNode->AvpInfo->AvpType)
    {
    case AVP_TYPE_UINT:
        {
            CnvtUintToDec(sValue,(int)pAvpNode->AvpDataSize,&iValue);
            //sprintf(cValue,"%d",iValue);
            //itoa(iValue,cValue,10);
            IntToStrNum((long long)iValue,cValue);
            break;
        }
    case AVP_TYPE_INT:
        {
            CnvtBinToDec(sValue,(int)pAvpNode->AvpDataSize,&iValueTemp);
            //sprintf(cValue,"%d",iValueTemp);
            //itoa(iValueTemp,cValue,10);
            IntToStrNum(iValueTemp,cValue);
            break;
        }
    case AVP_TYPE_ULONG:
        {
            CnvtUllongBinToDec(sValue,(int)pAvpNode->AvpDataSize,&lValue);
            //sprintf(cValue,"%ld",lValue);
            //ltoa(lValue,cValue,10);
            IntToStrNum((long long)lValue,cValue);
            break;
        }
    case AVP_TYPE_LONG:
        {
            CnvtLlongBinToDec(sValue,(int)pAvpNode->AvpDataSize,&lValueTemp);
            //sprintf(cValue,"%ld",lValueTemp);
            //ltoa(lValueTemp,cValue,10);
            IntToStrNum(lValueTemp,cValue);
            break;
        }
    case AVP_TYPE_CHAR:
        {
            CnvtByteToStr(sValue,(int)pAvpNode->AvpDataSize,cValue);//ZMP:477242
            pAvpNode->SetValue(cValue,(int)pAvpNode->AvpDataSize);
            return true;
            //break;
        }
    case AVP_TYPE_ADRESS:
        {
            CnvtDmtIPToIpv(sValue,(int)pAvpNode->AvpDataSize,cValue);
            break;
        }
    case AVP_TYPE_MEM:
        {
            CnvtByteToStr(sValue,(int)pAvpNode->AvpDataSize,cValue);
            break;
        }
    default:
        {
            return false;
            //break;//ZMP:305593,多余,去掉
        }
    }

    pAvpNode->SetValue(cValue);
    return true;
}

//输出对象信息
string TAvpParser::GetBobyString(TAvpInfo *pCurrAvpInfo)
{
    if(pCurrAvpInfo==NULL) return "";

    ostringstream sAddText;
    if(pCurrAvpInfo->AvpNode)
    {
        sAddText << pCurrAvpInfo->ToString() << endl;
    }

    TBaseListItem *pItem = pCurrAvpInfo->SubAvpItem;
    while(pItem)
    {
        sAddText << GetBobyString((TAvpInfo *)pItem->data);
        pItem = pItem->next;
    }

    return sAddText.str();
}

TAvpNode * TAvpParser::GetNodeInfo(TAvpNode *pParentNode,unsigned char *pDataBuffer,unsigned int iDataSize)
{
    //int iIndex = 0;//ZMP:305593该变量未用到
    char sKeyTemp[128] = {0};
    //取一个未使用的结构
    TAvpNode *pAvpNode = m_pDiamterInfo->AvpNodes[m_iNodeIndex];

    //清除数据
    pAvpNode->ClearData();

    pAvpNode->AvpCode = GetAvpNode(pDataBuffer);
    pAvpNode->VendorId = GetVendorId(pDataBuffer+4);
    pAvpNode->bHaveVendorId = HaveVendorId(pDataBuffer+4);
    pAvpNode->AvpNode = pDataBuffer;
    pAvpNode->AvpSize = GetAvpSize(pDataBuffer+5);
    pAvpNode->AvpNodeData = pDataBuffer + (pAvpNode->bHaveVendorId?12:8);
    pAvpNode->AvpDataSize = pAvpNode->AvpSize - (pAvpNode->bHaveVendorId?12:8); 
    pAvpNode->ParentNode = pParentNode;
    pAvpNode->AvpIndex = 1;

    //生成HashCode数据
    if(pParentNode && pParentNode->AvpCode>0)
    { 
        //sprintf(pAvpNode->AvpKey,"%s|%d,%d,%d",pParentNode->AvpKey,pAvpNode->AvpCode,pAvpNode->VendorId,pAvpNode->AvpIndex);
        //ZMP:467371 
        strncpy(pAvpNode->AvpKey,pParentNode->AvpKey,sizeof(pAvpNode->AvpKey)-1);
        pAvpNode->AvpKey[sizeof(pAvpNode->AvpKey)-1] = '\0';
        strcat(pAvpNode->AvpKey,"|");
        IntToStrNum((int)pAvpNode->AvpCode,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
        strcat(pAvpNode->AvpKey,",");
        IntToStrNum((int)pAvpNode->VendorId,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
        strcat(pAvpNode->AvpKey,",");
        IntToStrNum((int)pAvpNode->AvpIndex,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
    }
    else
    {
        //sprintf(pAvpNode->AvpKey,"%d,%d,%d",pAvpNode->AvpCode,pAvpNode->VendorId,pAvpNode->AvpIndex);
        IntToStrNum((int)pAvpNode->AvpCode,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
        strcat(pAvpNode->AvpKey,",");
        IntToStrNum((int)pAvpNode->VendorId,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
        strcat(pAvpNode->AvpKey,",");
        IntToStrNum((int)pAvpNode->AvpIndex,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
    }

    if(pParentNode)
    {
        if(pParentNode->SubNode)
        {
            TAvpNode *pCurrNode = pParentNode->SubNode;
            TAvpNode *pUpNode = NULL;
            int iCompValue;
            
            while(pCurrNode)
            {
                //iCompValue = pCurrNode->Compare(pAvpNode);
                if (pCurrNode->AvpCode != pAvpNode->AvpCode)
                    iCompValue = (int)(pCurrNode->AvpCode - pAvpNode->AvpCode);
                else
                    iCompValue = (int)(pCurrNode->VendorId - pAvpNode->VendorId);
                if(iCompValue>0)
                {
                    if(pUpNode == NULL)
                    {
                        pParentNode->SubNode = pAvpNode;
                        pAvpNode->NextNode = pCurrNode;
                    }
                    else
                    {
                        pUpNode->NextNode = pAvpNode;
                        pAvpNode->NextNode = pCurrNode;
                    }
                    break;
                }
                else if(iCompValue==0)
                {
                    if(pUpNode == NULL)
                        pParentNode->SubNode = pAvpNode;
                    else
                        pUpNode->NextNode = pAvpNode;
                    pAvpNode->NextNode = pCurrNode;
                    pAvpNode->AvpIndex = pCurrNode->AvpIndex+1;
                    break;
                }
                else if(pCurrNode->NextNode==NULL)
                {
                    pCurrNode->NextNode = pAvpNode;
                    break;
                }

                //查找下一条记录
                pUpNode = pCurrNode;
                pCurrNode = pCurrNode->NextNode;
            }
        }
        else
        {
            pParentNode->SubNode = pAvpNode;
        }
    }

    //生成HashCode数据
    if(pParentNode && pParentNode->AvpCode>0)
    { 
        //sprintf(pAvpNode->AvpKey,"%s|%d,%d,%d",pParentNode->AvpKey,pAvpNode->AvpCode,pAvpNode->VendorId,pAvpNode->AvpIndex);
        memset(pAvpNode->AvpKey,0,sizeof(pAvpNode->AvpKey));
        //ZMP:467371 
        strncpy(pAvpNode->AvpKey,pParentNode->AvpKey,sizeof(pAvpNode->AvpKey)-1);
        pAvpNode->AvpKey[sizeof(pAvpNode->AvpKey)-1] = '\0';
        strcat(pAvpNode->AvpKey,"|");
        IntToStrNum((int)pAvpNode->AvpCode,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
        strcat(pAvpNode->AvpKey,",");
        IntToStrNum((int)pAvpNode->VendorId,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
        strcat(pAvpNode->AvpKey,",");
        IntToStrNum((int)pAvpNode->AvpIndex,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
    }
    else
    {
        //sprintf(pAvpNode->AvpKey,"%d,%d,%d",pAvpNode->AvpCode,pAvpNode->VendorId,pAvpNode->AvpIndex);
        memset(pAvpNode->AvpKey,0,sizeof(pAvpNode->AvpKey));
        IntToStrNum((int)pAvpNode->AvpCode,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
        strcat(pAvpNode->AvpKey,",");
        IntToStrNum((int)pAvpNode->VendorId,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
        strcat(pAvpNode->AvpKey,",");
        IntToStrNum((int)pAvpNode->AvpIndex,sKeyTemp);
        strcat(pAvpNode->AvpKey,sKeyTemp);
        memset(sKeyTemp,0,sizeof(sKeyTemp));
    }

    //读取配置文件信息
    pAvpNode->AvpInfo = m_pAvpFile->GetAvpInfo(pAvpNode->AvpKey);
    if(pAvpNode->AvpInfo)
    {
        pAvpNode->AvpInfo->AvpNode = pAvpNode;
        pAvpNode->AvpGroup = pAvpNode->AvpInfo->IsGroup;
        pAvpNode->AvpType = pAvpNode->AvpInfo->AvpType;

        if (!pAvpNode->AvpInfo->IsGroup)
        {
            //memset(pAvpNode->m_sValue,0,sizeof(pAvpNode->m_sValue));
            //memcpy(pAvpNode->m_sValue,pAvpNode->AvpNodeData,pAvpNode->AvpDataSize);
            this->GetAvpValue(pAvpNode);
        }
    }

    //指向下一个结构体
    m_iNodeIndex++;

    return pAvpNode;
}

void TAvpParser::CnvtBinToDec(unsigned char *Data,int iLength,int *piOut)
{
    if(Data==NULL||iLength<=0) return;

    int iTmp=0,iCount=0;
    while(iCount<iLength)
    {
        iTmp=(iTmp<<8)|(*Data);
        Data++;
        iCount++;
    }

    *piOut=iTmp;
}

void TAvpParser::CnvtUintToDec(unsigned char *Data,int iLength,unsigned int *piOut)
{
    if(Data==NULL||iLength<=0) return;

    unsigned int iTmp=0;
    int iCount=0;
    while(iCount<iLength)
    {
        iTmp=(iTmp<<8)|(*Data);
        Data++;
        iCount++;
    }

    *piOut=iTmp;
}

void TAvpParser::CnvtUllongBinToDec(unsigned char *Data,int iLength,unsigned long long *plOut)
{
    if(Data==NULL||iLength<=0) return;

    unsigned long long lTmp=0;
    int iCount=0;
    while(iCount<iLength)
    {
        lTmp=(lTmp<<8)|(*Data);
        Data++;
        iCount++;
    }

    *plOut=lTmp;
}

void TAvpParser::CnvtLlongBinToDec(unsigned char *Data,int iLength,long long *plOut)
{
    if(Data==NULL||iLength<=0) return;

    long long lTmp=0;
    int iCount=0;
    while(iCount<iLength)
    {
        lTmp=(lTmp<<8)|(*Data);
        Data++;
        iCount++;
    }

    *plOut=lTmp;
}

void TAvpParser::CnvtBinToDecStr(unsigned char *Data,int iLength,char *pcOut)
{
    int iTmp=-1;

    //转换原始数据为整数
    CnvtBinToDec(Data,iLength,&iTmp);

    //计算整数位数
    int iCnt=1;
    for(int i=iTmp;i/10!=0;iCnt++)
    {
        i/=10;
    }

    //逐个转换
    for(int i=iCnt;i>0;i--)
    {
        pcOut[i-1]=(char)(iTmp%10+'0');//ZMP:477242
        iTmp=iTmp/10;
    }

    pcOut[iCnt]='\0';
}

void TAvpParser::CnvtByteToStr(unsigned char *Data,int iLength,char *pcOut)
{
    if(Data==NULL||iLength<=0) return;

    memcpy(pcOut,Data,(size_t)iLength);
    pcOut[iLength]='\0';
}

void TAvpParser::CnvtDmtIPToIpv(unsigned char *Data,int iLength,char *pcOut)
{
    if(Data==NULL||iLength<=0) return;

    int iType = 0;
    int iDataTemp = 0;
    unsigned char *pDataTemp = Data;
    char *pOutTemp = pcOut;
    iType = pDataTemp[0]*256+pDataTemp[1];

    if (iType == 1)//IPV4
    {
        iDataTemp = pDataTemp[2];
        //ZMP:467371,int的最大长度是10位，因此这里设定为11，最后一位存放'\0'
        snprintf(pOutTemp,11,"%d",iDataTemp);
        pOutTemp[10] = '\0';
        pOutTemp += strlen(pOutTemp);
        for (int i=0;i<3;i++)
        {
            iDataTemp = 0;
            iDataTemp = pDataTemp[3+i];
            //ZMP:467371
            snprintf(pOutTemp,12, ".%d", iDataTemp);
            pOutTemp[11] = '\0';
            pOutTemp +=strlen(pOutTemp);
        }
    }
    else if (iType == 2)//IPV6
    {
        iDataTemp = pDataTemp[2]*256+pDataTemp[3];
        //ZMP:467371
        snprintf(pOutTemp, 11, "%d",iDataTemp);
        pOutTemp[10] = '\0';
        pOutTemp += strlen(pOutTemp);
        for (int i=0;i<7;i++)
        {
            iDataTemp = 0;
            iDataTemp = pDataTemp[2*i+4]*256+pDataTemp[2*i+5];
            //ZMP:467371
            snprintf(pOutTemp,12,":%d",iDataTemp);
            pOutTemp[11] = '\0';
            pOutTemp +=strlen(pOutTemp);
        }
    }
    else
    {
        return;
    }
}
int TAvpParser::log2(int iNumber)
{
    int iRet = 0;
    unsigned char temp = 0x00;
    int iNumTemp = iNumber;
    int iMax = (int)(sizeof(int)*8);
    for (int i=0; i < iMax; i++)
    {
        temp=(unsigned char)((iNumTemp>>(i))&0x01);
        if (temp!=0x00)
        {
            iRet = i;
            break;
        }
    }
    return iRet;
}
int TAvpParser::IntToStrNum(int iNumber,char *sOut)
{
    /*
    int iRet = 0;
    char ch,*right=NULL,*left=NULL; //357111 for cppcheck
    unsigned int Value;
    if (sOut == NULL)
    {
        return iRet;
    }
    left = right = sOut;

    //如果是负数，则应加上负号，left、right向后走。
    if(iNumber < 0)
    {
        Value = -iNumber;
        *sOut = '-';
        left++,right++;
    }
    else
    {
        Value = (unsigned)iNumber;
    }

    //把数字转换成字符串（倒置的）
    if (Value==0)
    {
        *right = 0+0x30;
        right++;
    }
    while(Value)
    {
        *right = (Value%10)+0x30;
        Value = Value/10;
        right++;
    }
    iRet = right-left;
    *right-- = '\0';
    //把倒置的字符串正放过来
    while(right > left)
    {
        ch = *left;
        *left++ = *right;
        *right-- = ch;
    }

    return iRet;
    */
    
    //ZMP:480769 
    int iRet = 0;

    if (sOut == NULL)

    {

        return iRet;

    }

    if( 0 == iNumber )

    {

        sOut[0]='0';

        sOut[1]='\0';

        return 1;

    }

    iRet = snprintf(sOut,12,"%d",iNumber);

    if( '-' == sOut[0] )

        iRet--;

    return iRet;
}
int TAvpParser::IntToStrNum(long long iNumber,char *sOut)
{
    int iRet = 0;
    char ch,*right=NULL,*left=NULL; //357111 for cppcheck
    unsigned long long Value;
    if (sOut == NULL)
    {
        return iRet;
    }
    left = right = sOut;

    //如果是负数，则应加上负号，left、right向后走。
    if(iNumber < 0)
    {
        Value = (long long unsigned int)(-iNumber);
        *sOut = '-';
        left++,right++;
    }
    else
    {
        Value = (unsigned long long)iNumber;
    }

    //把数字转换成字符串（倒置的）
    if (Value==0)
    {
        *right = 0+0x30;
        right++;
    }
    while(Value)
    {
        *right = (char)((Value%10)+0x30);//ZMP:477242
        Value = Value/10;
        right++;
    }
    iRet = (int)(right-left);//ZMP:477242
    *right-- = '\0';
    //把倒置的字符串正放过来
    while(right > left)
    {
        ch = *left;
        *left++ = *right;
        *right-- = ch;
    }

    return iRet;
}
bool TAvpParser::ParserHead(unsigned char *pDataBuffer,unsigned int iDataSize)
{
    bool bReFlag = true;
    unsigned char * buftemp = pDataBuffer;

    unsigned char  pcTmp[4] = {0};

    memcpy(pcTmp, buftemp, 4);
    buftemp+=4;

    m_pDiamterInfo->AvpHead->Version=(int)pcTmp[0];

    m_pDiamterInfo->AvpHead->Length = (unsigned int)(pcTmp[3] + pcTmp[2]*256 + pcTmp[1]*256*256);

    memcpy(pcTmp, buftemp, 4);
    buftemp+=4;
    m_pDiamterInfo->AvpHead->CommandFlag=(char)pcTmp[0];
    m_pDiamterInfo->AvpHead->CommandCode = pcTmp[3] + pcTmp[2]*256 + pcTmp[1]*256*256;

    memcpy(pcTmp, buftemp, 4);
    buftemp+=4;
    m_pDiamterInfo->AvpHead->AppId=pcTmp[3] + pcTmp[2]*256 + pcTmp[1]*256*256 + pcTmp[0]*256*256*256;

    memcpy(pcTmp, buftemp, 4);
    buftemp+=4;
    m_pDiamterInfo->AvpHead->HopId=(unsigned int)(pcTmp[3] + pcTmp[2]*256 + pcTmp[1]*256*256 + pcTmp[0]*256*256*256);

    memcpy(pcTmp, buftemp, 4);
    //buftemp+=4; //for cppcheck 351951
    m_pDiamterInfo->AvpHead->EndId=(unsigned int)(pcTmp[3] + pcTmp[2]*256 + pcTmp[1]*256*256 + pcTmp[0]*256*256*256);

    return bReFlag;
}

bool TAvpParser::ParserNode(TAvpNode *pParentNode,unsigned char *pDataBuffer,unsigned int iDataSize)
{
    TAvpNode *pAvpNode = NULL;
    unsigned char *pCurrNode = pDataBuffer;
    int iNodexSize = (int)iDataSize;
    bool bRetFlag = true;

    //取出所有NODE结点
    while(bRetFlag && pCurrNode-pDataBuffer<(int)iDataSize)
    {
        //生成结构信息
        pAvpNode = GetNodeInfo(pParentNode,pCurrNode,(unsigned int)iNodexSize);
        if(pAvpNode)
        {
            //是否有子结构
            if(pAvpNode->AvpInfo && pAvpNode->AvpInfo->IsGroup)
            {
                bRetFlag = ParserNode(pAvpNode,pCurrNode+(pAvpNode->bHaveVendorId?12:8),pAvpNode->AvpSize-(pAvpNode->bHaveVendorId?12:8));
            }

            if (bRetFlag)
            {
                //指向下一结点
                int iTemp = (int)(pAvpNode->AvpSize + (pAvpNode->AvpSize%4>0 ? (4-pAvpNode->AvpSize%4) : 0 ));
                pCurrNode  += iTemp;
                iNodexSize -= iTemp;
                if (iNodexSize < 0 || iTemp < 8)
                {
                    //写日志或抛异常后退出
                    m_iErrorCode = 6;
                    //ZMP:467371
                    strncpy(m_sErrorMsg,"Diamter Message Size Is Error.", sizeof(m_sErrorMsg)-1);
                    m_sErrorMsg[sizeof(m_sErrorMsg)-1] = '\0';
                    bRetFlag = false;
                    break;
                }
            }
        }
        else
        {
            //写日志或抛异常后退出
            m_iErrorCode = 7;
            //ZMP:467371
            strncpy(m_sErrorMsg,"Diamter Message Size Is Error.", sizeof(m_sErrorMsg)-1);
            m_sErrorMsg[sizeof(m_sErrorMsg)-1] = '\0';
            bRetFlag = false;
            break;
        }
    }

    return bRetFlag;
}

bool TAvpParser::Parser(unsigned char *pDataBuffer,unsigned int iDataSize)
{
    bool bRetFlag = true;

    //保存数据
    memcpy(m_pDataBuffer,pDataBuffer,iDataSize);
    m_iDataSize = (int)iDataSize;

    //清除数据
    m_iNodeIndex = 0;
    m_iErrorCode = 0;
    m_pDiamterInfo->AvpNodeHead.ClearData();
    m_pAvpFile->ClearData();

    bRetFlag = ParserHead(pDataBuffer,20);

    if(bRetFlag && m_pDiamterInfo->AvpHead->Length <= iDataSize)
    {
        //解析结构体信息
        bRetFlag = ParserNode(&m_pDiamterInfo->AvpNodeHead,pDataBuffer+20,m_pDiamterInfo->AvpHead->Length-20);

        //得到结构体总数
        m_pDiamterInfo->AvpNodeSize = m_iNodeIndex;

        //建立结构体与配置文件关联信息
        //CreateIndexInfo(&m_pDiamterInfo->AvpNodeHead);

    }
    else
    {
        m_iErrorCode = 5;
        //ZMP:467371
        strncpy(m_sErrorMsg,"Diamter Message Length Is Error.",sizeof(m_sErrorMsg)-1);
        m_sErrorMsg[sizeof(m_sErrorMsg)-1] = '\0';
    }

    return bRetFlag;
}

//得到解析后的结查果
TDiamterInfo * TAvpParser::GetDiamterInfo(void)
{
    return m_pDiamterInfo;
}
TAvpNode * TAvpParser::GetAvpNodeInfo(const char *sKey)
{
    TAvpNode * pAvpNodeTemp = NULL;
    TAvpInfo * pAvpInfoTemp = NULL;
    pAvpInfoTemp = m_pAvpFile->GetAvpInfo(sKey);
    if (pAvpInfoTemp != NULL)
    {
        pAvpNodeTemp = pAvpInfoTemp->AvpNode;
    }

    return pAvpNodeTemp;
}
//del//bool TAvpParser::SetRatableEvent(TRatableEvent *pRatableEvent)
//del//{
//del//    TAvpInfo *pAvpInfo = NULL;
//del//    unsigned char sValue[255] = {0};
//del//    unsigned int  iValue = 0;
//del//    int iValueTemp = 0;
//del//    unsigned long long lValue = 0;
//del//    long long lValueTemp = 0;
//del//    char cValue[255] = {0};
//del//    for(unsigned int i=0;i<m_pDiamterInfo->AvpNodeSize;i++)
//del//    {
//del//        memset(sValue,0,sizeof(sValue));
//del//        memset(cValue,0,sizeof(cValue));
//del//        iValue = 0;
//del//        iValueTemp = 0;
//del//        lValue = 0;
//del//        lValueTemp = 0;
//del//        pAvpInfo = m_pDiamterInfo->AvpNodes[i]->AvpInfo;
//del//        if(pAvpInfo==NULL || pAvpInfo->IsGroup || pAvpInfo->AttrId<=0)
//del//            continue;
//del//        memcpy(sValue,m_pDiamterInfo->AvpNodes[i]->AvpNodeData,m_pDiamterInfo->AvpNodes[i]->AvpDataSize);
//del//        switch(pAvpInfo->AvpType)
//del//        {
//del//        case AVP_TYPE_UINT:
//del//            {
//del//                CnvtUintToDec(sValue,m_pDiamterInfo->AvpNodes[i]->AvpDataSize,&iValue);
//del//                pRatableEvent->SetAttr(pAvpInfo->AttrId,iValue);
//del//                break;
//del//            }
//del//        case AVP_TYPE_INT:
//del//            {
//del//                CnvtBinToDec(sValue,m_pDiamterInfo->AvpNodes[i]->AvpDataSize,&iValueTemp);
//del//                pRatableEvent->SetAttr(pAvpInfo->AttrId,iValueTemp);
//del//                break;
//del//            }
//del//        case AVP_TYPE_ULONG:
//del//            {
//del//                CnvtUllongBinToDec(sValue,m_pDiamterInfo->AvpNodes[i]->AvpDataSize,&lValue);
//del//                pRatableEvent->SetAttr(pAvpInfo->AttrId,lValue);
//del//                break;
//del//            }
//del//        case AVP_TYPE_LONG:
//del//            {
//del//                CnvtLlongBinToDec(sValue,m_pDiamterInfo->AvpNodes[i]->AvpDataSize,&lValueTemp);
//del//                pRatableEvent->SetAttr(pAvpInfo->AttrId,lValueTemp);
//del//                break;
//del//            }
//del//        case AVP_TYPE_CHAR:
//del//            {
//del//                CnvtByteToStr(sValue,strlen((const char*)sValue),cValue);
//del//                pRatableEvent->SetAttr(pAvpInfo->AttrId,cValue);
//del//                break;
//del//            }
//del//        case AVP_TYPE_ADRESS:
//del//            {
//del//                CnvtDmtIPToIpv(sValue,m_pDiamterInfo->AvpNodes[i]->AvpDataSize,cValue);
//del//                pRatableEvent->SetAttr(pAvpInfo->AttrId,cValue);
//del//                break;
//del//            }
//del//        case AVP_TYPE_MEM:
//del//            {
//del//                CnvtByteToStr(sValue,m_pDiamterInfo->AvpNodes[i]->AvpDataSize,cValue);
//del//                pRatableEvent->SetAttr(pAvpInfo->AttrId,cValue,m_pDiamterInfo->AvpNodes[i]->AvpDataSize);
//del//                break;
//del//            }
//del//        default:
//del//            break;
//del//        }
//del//    }
//del//    return true;
//del//}

TAvpEncode::TAvpEncode(TAvpFile *pAvpFile)
{
    memset(m_pDataBuffer,0,sizeof(m_pDataBuffer));
    m_iDataIndex = 0;
    m_iServiceType = 0;
    m_iCcrType = 0;
    m_pAvpFile = pAvpFile;
    //del//m_pRateEvent = NULL;
    m_pAvpHead = new TAvpHead();
    m_bDoubleSrc = false;
    //ZMP:305593
    m_pDmtHash = NULL;
}
TAvpEncode:: ~TAvpEncode()
{
    memset(m_pDataBuffer,0,sizeof(m_pDataBuffer));
    m_iDataIndex = 0;
    m_iServiceType = 0;
    m_iCcrType = 0;
    m_pAvpFile = NULL;
    //del//m_pRateEvent = NULL;
    DELETE(m_pAvpHead);
}

//del//bool TAvpEncode::Encode(TRatableEvent *pEvent,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize,int iServType)
//del//{
//del//    bool bRetFlag = true;
//del//    int iDataLen = 0;
//del//    unsigned char  cTemp[64];
//del//    memset(cTemp,0,sizeof(cTemp));
//del//    TAvpInfo *pCurrAvpInfo = NULL;
//del//    //m_iServiceType = TAvpParser::log2(iServType);
//del//    memset(m_pDataBuffer,0,sizeof(m_pDataBuffer));
//del//    m_iDataIndex = 0;
//del//
//del//    TAvpNode *pNode = NULL;
//del//    if (pDataBuffer==NULL || iDataSize<0)
//del//    {
//del//        bRetFlag = false;
//del//    }
//del//    else
//del//    {
//del//        //对消息头进行编码
//del//        bRetFlag = EncodeMsgHead(m_pAvpHead);
//del//        if (bRetFlag)
//del//        {
//del//            m_pRateEvent = pEvent;
//del//            //对消息体进行编码
//del//            if (m_pAvpFile != NULL)
//del//            {
//del//                pCurrAvpInfo = m_pAvpFile->GetAvpHead();
//del//                TBaseListItem *pItem = pCurrAvpInfo->SubAvpItem;
//del//                while (pItem != NULL && iDataLen>=0 && iDataLen<iDataSize)
//del//                {
//del//                    pCurrAvpInfo = (TAvpInfo *)pItem->data;
//del//                    if (m_pDmtHash!=NULL)
//del//                    {
//del//                        pNode = NULL;
//del//                        memset(m_tAvpNode.AvpKey,0,sizeof(m_tAvpNode.AvpKey));
//del//                        strncpy(m_tAvpNode.AvpKey,pCurrAvpInfo->AvpKey,sizeof(m_tAvpNode.AvpKey));
//del//                        pNode = (TAvpNode *)m_pDmtHash->Seek(pCurrAvpInfo->AvpKey,&m_tAvpNode);
//del//                    }
//del//                    //不符合编码条件的AVP跳过
//del//                    if(pCurrAvpInfo->AvpCode<0 || !GetEncodeFlag(pCurrAvpInfo,false,false))
//del//                    {
//del//                        if (!pCurrAvpInfo->bNoEncode[m_iServiceType][m_iCcrType]&&m_bDoubleSrc&&pNode!=NULL)
//del//                        {
//del//                            iDataLen = EncodeNode(pCurrAvpInfo,pNode);
//del//                        }
//del//                        pItem = pItem->next;
//del//                        continue;
//del//                    }
//del//                    else
//del//                    {
//del//                        iDataLen = EncodeNode(pCurrAvpInfo,pNode);
//del//                    }
//del//                    pItem = pItem->next;
//del//                }
//del//            }
//del//            //长度进行编码
//del//            if (m_iDataIndex%4!=0)
//del//            {
//del//                bRetFlag=false;
//del//            }
//del//            else
//del//            {
//del//                int iTemp=IntToBin(cTemp,m_iDataIndex);
//del//                memcpy(m_pDataBuffer+1,cTemp+1,3);
//del//                *iRealSize = m_iDataIndex;
//del//                memcpy(pDataBuffer,m_pDataBuffer,m_iDataIndex);
//del//            }
//del//        }
//del//    }
//del//    return bRetFlag;
//del//}

bool TAvpEncode::Encode(TDiamterInfo *pInfo,unsigned char *pDataBuffer,unsigned int iDataSize,int *iRealSize,int iServType)
{
    bool bRetFlag = true;

    int iDataLen = 0;
    unsigned char  cTemp[64] = {0};//ZMP:477242
    //memset(cTemp,0,sizeof(cTemp));
    TAvpNode *pCurrAvpNode = NULL;
    //m_iServiceType = TAvpParser::log2(iServType);
    memset(m_pDataBuffer,0,sizeof(m_pDataBuffer));
    m_iDataIndex = 0;

    if (pInfo == NULL || pDataBuffer==NULL)
    {
        bRetFlag = false;
    }
    else
    {
        //对消息头进行编码
        bRetFlag = EncodeMsgHead(pInfo->AvpHead,m_pDataBuffer,m_iDataIndex);
        if (bRetFlag)
        {
            //对消息体进行编码
            //int iNumber = pInfo->AvpNodeSize;//ZMP:305593该变量未用到
            pCurrAvpNode = &(pInfo->AvpNodeHead);
            TAvpNode * pItem = pCurrAvpNode->SubNode;
            while(pItem != NULL && iDataLen>=0 && iDataLen<(int)iDataSize)
            {
                iDataLen = EncodeNode(pItem,m_pDataBuffer,m_iDataIndex);
                pItem = pItem->NextNode;
            }
            //长度进行编码
            if (m_iDataIndex%4!=0)
            {
                bRetFlag=false;
            }
            else
            {
                IntToBin(cTemp,m_iDataIndex);//ZMP:305593该变量未用到 int iTemp=
                memcpy(m_pDataBuffer+1,cTemp+1,3);
                *iRealSize = m_iDataIndex;
                memcpy(pDataBuffer,m_pDataBuffer,(size_t)m_iDataIndex);
            }
        }
    }
    return bRetFlag;
}


bool TAvpEncode::EncodeMsgHead(TAvpHead *pHead,unsigned char * mp_pDataBuffer,int &mp_iDataIndex)
{
    bool bRetFlag = true;
    TAvpHead * pDataTemp = pHead;

    mp_pDataBuffer[mp_iDataIndex++]=(unsigned char)pDataTemp->Version;

    unsigned char  cTemp[64]= {0};//ZMP:477242
    //memset(cTemp,0,sizeof(cTemp));
    int iTemp=UintToBin(cTemp,pDataTemp->Length);
    if (iTemp!=4)
    {
        bRetFlag=false;
    }
    for (int i=1;i<4;i++)
    {
        mp_pDataBuffer[mp_iDataIndex++]=cTemp[i];
    }

    mp_pDataBuffer[mp_iDataIndex++]=(unsigned char)pDataTemp->CommandFlag;

    memset(cTemp,0,sizeof(cTemp));
    iTemp=UintToBin(cTemp,(unsigned int)pDataTemp->CommandCode);
    if (iTemp!=4)
    {
        bRetFlag=false;
    }
    for (int i=1;i<4;i++)
    {
        mp_pDataBuffer[mp_iDataIndex++]=cTemp[i];
    }

    memset(cTemp,0,sizeof(cTemp));
    iTemp=UintToBin(cTemp,(unsigned int)pDataTemp->AppId);
    if (iTemp!=4)
    {
        bRetFlag=false;
    }
    for (int i=0;i<4;i++)
    {
        mp_pDataBuffer[mp_iDataIndex++]=cTemp[i];
    }

    memset(cTemp,0,sizeof(cTemp));
    iTemp=UintToBin(cTemp,pDataTemp->HopId);
    if (iTemp!=4)
    {
        bRetFlag=false;
    }
    for (int i=0;i<4;i++)
    {
        mp_pDataBuffer[mp_iDataIndex++]=cTemp[i];
    }

    memset(cTemp,0,sizeof(cTemp));
    iTemp=UintToBin(cTemp,pDataTemp->EndId);
    if (iTemp!=4)
    {
        bRetFlag=false;
    }
    for (int i=0;i<4;i++)
    {
        mp_pDataBuffer[mp_iDataIndex++]=cTemp[i];
    }
    return bRetFlag;
}

//del//int TAvpEncode::EncodeNode(TAvpInfo *pInfo)
//del//{
//del//    int iRet= 0;
//del//    unsigned char pOutData[512] = {0};
//del//    int iDataLen = 0;
//del//    int iDataVauleLen = 0;
//del//    TEventAttr * pEventAttr = NULL;
//del//    if (pInfo != NULL)
//del//    {
//del//        if (pInfo->bNoEncode[m_iServiceType][m_iCcrType])
//del//        {
//del//            iRet = 0;
//del//            return iRet;
//del//        }
//del//        if (pInfo->IsGroup)
//del//        {
//del//            iDataLen=EncodeNodeHead(pInfo);
//del//            TBaseListItem *pItem = pInfo->SubAvpItem;
//del//            while(pItem)
//del//            {
//del//                TAvpInfo * pTempNode = (TAvpInfo *)pItem->data;
//del//                iDataLen +=EncodeNode(pTempNode);
//del//                pItem = pItem->next;
//del//            }
//del//            int iLen=IntToBin(pOutData,iDataLen);
//del//            memcpy(m_pDataBuffer+pInfo->AvpLenLoc,pOutData+1,3);
//del//        }
//del//        else
//del//        {
//del//            pEventAttr = m_pRateEvent->GetAttrEx(pInfo->AttrId);
//del//            if (!pEventAttr->IsEmpty())
//del//            {
//del//                iDataLen=EncodeNodeHead(pInfo);
//del//                switch(pInfo->AvpType)
//del//                {
//del//                case AVP_TYPE_INT:
//del//                    {
//del//                        iDataVauleLen = IntToBin(pOutData,pEventAttr->AsInteger());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_LONG:
//del//                    {
//del//                        iDataVauleLen = LlongToBin(pOutData,pEventAttr->AsInteger());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_UINT:
//del//                    {
//del//                        iDataVauleLen = UintToBin(pOutData,pEventAttr->AsInteger());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_ULONG:
//del//                    {
//del//                        iDataVauleLen = UllongToBin(pOutData,pEventAttr->AsInteger());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_CHAR:
//del//                    {
//del//                        iDataVauleLen = StrToByte(pOutData,pEventAttr->AsString());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_ADRESS:
//del//                    {
//del//                        iDataVauleLen = CnvtIpvToDmtIP(pOutData,pEventAttr->AsString());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_MEM:
//del//                    {
//del//                    	  memcpy(pOutData,pEventAttr->AsString(),pEventAttr->GetSize());
//del//                    	  iDataVauleLen = pEventAttr->GetSize();
//del//                    	  pOutData[iDataVauleLen] = '\0';
//del//                        break;
//del//                    }
//del//                default:
//del//                    break;
//del//                }
//del//                if (iDataVauleLen<0)
//del//                {
//del//                    return iRet;
//del//                }
//del//                memcpy(m_pDataBuffer+m_iDataIndex,pOutData,iDataVauleLen);
//del//                iDataVauleLen = iDataVauleLen%4==0?iDataVauleLen:(iDataVauleLen+(4-iDataVauleLen%4));
//del//                m_iDataIndex +=iDataVauleLen;
//del//                iDataLen += iDataVauleLen;
//del//
//del//                int iLen=IntToBin(pOutData,iDataLen);
//del//                memcpy(m_pDataBuffer+pInfo->AvpLenLoc,pOutData+1,3);
//del//            }
//del//            else if (pInfo->bOption[m_iServiceType][m_iCcrType])
//del//            {
//del//                long long lValueTempor = 0;
//del//                iDataLen=EncodeNodeHead(pInfo);
//del//                switch(pInfo->AvpType)
//del//                {
//del//                case AVP_TYPE_INT:
//del//                    {
//del//                        lValueTempor = (long long)atoi(pInfo->cDefaultValue);
//del//                        iDataVauleLen = IntToBin(pOutData,lValueTempor);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_LONG:
//del//                    {
//del//                        lValueTempor = (long long)atol(pInfo->cDefaultValue);
//del//                        iDataVauleLen = LlongToBin(pOutData,lValueTempor);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_UINT:
//del//                    {
//del//                        lValueTempor = (long long)atol(pInfo->cDefaultValue);
//del//                        iDataVauleLen = UintToBin(pOutData,lValueTempor);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_ULONG:
//del//                    {
//del//                        lValueTempor = (long long)atol(pInfo->cDefaultValue);
//del//                        iDataVauleLen = UllongToBin(pOutData,lValueTempor);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_CHAR:
//del//                    {
//del//                        iDataVauleLen = StrToByte(pOutData,pInfo->cDefaultValue);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_ADRESS:
//del//                    {
//del//                        iDataVauleLen = CnvtIpvToDmtIP(pOutData,pInfo->cDefaultValue);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_MEM:
//del//                    {
//del//                    	  iDataVauleLen = StrToByte(pOutData,pInfo->cDefaultValue);
//del//                        break;
//del//                    }
//del//                default:
//del//                    break;
//del//                }
//del//                if (iDataVauleLen<0)
//del//                {
//del//                    return iRet;
//del//                }
//del//                memcpy(m_pDataBuffer+m_iDataIndex,pOutData,iDataVauleLen);
//del//                iDataVauleLen = iDataVauleLen%4==0?iDataVauleLen:(iDataVauleLen+(4-iDataVauleLen%4));
//del//                m_iDataIndex +=iDataVauleLen;
//del//                iDataLen += iDataVauleLen;
//del//
//del//                int iLen=IntToBin(pOutData,iDataLen);
//del//                memcpy(m_pDataBuffer+pInfo->AvpLenLoc,pOutData+1,3);
//del//            }
//del//        }
//del//    }
//del//    return iRet = iDataLen;
//del//}



int TAvpEncode::EncodeNode(TAvpNode *pInfo,unsigned char * mp_pDataBuffer,int &mp_iDataIndex)
{
    int iRet= mp_iDataIndex;

    unsigned char pOutData[512] = {0};
    int iAvpLenTemp = 0;
    int iDataVauleLen = 0;
    unsigned char  cTemp[64]= {0};//ZMP:477242
    //memset(cTemp,0,sizeof(cTemp));

    int iLen=IntToBin(cTemp,(int)pInfo->AvpCode);
    memcpy(mp_pDataBuffer+mp_iDataIndex,cTemp,(size_t)iLen);
    mp_iDataIndex+=4;

    unsigned char cFlag = (unsigned char)((pInfo->AvpFlag)|(pInfo->VendorId>0?0x80:0x00));//ZMP:477242
    memcpy(mp_pDataBuffer+mp_iDataIndex,&cFlag,1);
    mp_iDataIndex +=1;

    pInfo->AvpLenLoc = mp_iDataIndex;
    mp_iDataIndex +=3;

    if (pInfo->VendorId > 0)
    {
        memset(cTemp,0,sizeof(cTemp));
        iLen=IntToBin(cTemp,(int)pInfo->VendorId);
        memcpy(mp_pDataBuffer+mp_iDataIndex,cTemp,(size_t)iLen);
        mp_iDataIndex+=4;
    }

    memset(cTemp,0,sizeof(cTemp));
    if (pInfo->AvpGroup)
    {
        int iTempLen = 0;
        int iTempTotalLen = 0;
        TAvpNode *pItem = pInfo->SubNode;
        while (pItem != NULL && iTempLen>=0)
        {
            iTempLen = EncodeNode(pItem,mp_pDataBuffer,mp_iDataIndex);
            pItem = pItem->NextNode;
            iTempTotalLen += iTempLen;
        }
        if (iTempTotalLen == 0)
        {
            mp_iDataIndex = iRet;
            return 0;
        }
        iTempTotalLen +=(pInfo->VendorId > 0)?12:8;
        IntToBin(cTemp,iTempTotalLen);//ZMP:305593,变量未用到,int iLen=
        memcpy(mp_pDataBuffer+pInfo->AvpLenLoc,cTemp+1,3);
        return iTempTotalLen;
    }

    long long lValueTempor = 0;
    switch(pInfo->AvpType)
    {
    case AVP_TYPE_INT:
        {
            lValueTempor = (long long)atoi(pInfo->DefaultValue);
            iDataVauleLen = IntToBin(pOutData,(int)lValueTempor);
            break;
        }
    case AVP_TYPE_LONG:
        {
            lValueTempor = (long long)atol(pInfo->DefaultValue);
            iDataVauleLen = LlongToBin(pOutData,lValueTempor);
            break;
        }
    case AVP_TYPE_UINT:
        {
            lValueTempor = (long long)strtoul(pInfo->DefaultValue,0,10);
            iDataVauleLen = UintToBin(pOutData,(unsigned int)lValueTempor);
            break;
        }
    case AVP_TYPE_ULONG:
        {
            lValueTempor = (long long)strtoul(pInfo->DefaultValue,0,10);
            iDataVauleLen = UllongToBin(pOutData,(unsigned long long)lValueTempor);
            break;
        }
    case AVP_TYPE_CHAR:
        {
            iDataVauleLen = StrToByte(pOutData,pInfo->DefaultValue);
            break;
        }
    case AVP_TYPE_ADRESS:
        {
            iDataVauleLen = CnvtIpvToDmtIP(pOutData,pInfo->DefaultValue);
            break;
        }
    case AVP_TYPE_MEM:
        {
            iDataVauleLen = StrToByte(pOutData,pInfo->DefaultValue);
            break;
        }
    default:
        break;
    }
    if (iDataVauleLen>=0)
    {
        pInfo->AvpDataSize = (unsigned int)iDataVauleLen;
        memcpy(mp_pDataBuffer+mp_iDataIndex,pOutData,(size_t)iDataVauleLen);
        iDataVauleLen = iDataVauleLen%4==0?iDataVauleLen:(iDataVauleLen+(4-iDataVauleLen%4));
        mp_iDataIndex +=iDataVauleLen;
        iAvpLenTemp = mp_iDataIndex - iRet;

        IntToBin(pOutData,iAvpLenTemp);//ZMP:305593,变量未用到,int iLen=
        memcpy(mp_pDataBuffer+pInfo->AvpLenLoc,pOutData+1,3);

        pInfo->AvpSize = (unsigned int)iAvpLenTemp;
    }

    return iAvpLenTemp; //return iRet = iAvpLenTemp; //351951 for cppcheck
}


//del//int TAvpEncode::EncodeNode(TAvpInfo *pInfo,TAvpNode *pNode)
//del//{
//del//    int iRet= 0;
//del//    unsigned char pOutData[512] = {0};
//del//    int iDataLen = 0;
//del//    int iDataVauleLen = 0;
//del//    int iDataRealLen = 0;
//del//    TEventAttr * pEventAttr = NULL;
//del//
//del//    if (pNode == NULL && (!GetEncodeFlag(pInfo,false,true)))
//del//    {
//del//        return 0;
//del//    }
//del//
//del//    //while the pInfo without coding ,using the DmtData's pNode
//del//    TAvpNode tempNode;
//del//
//del//    if (pInfo != NULL)
//del//    {
//del//        if (pInfo->bNoEncode[m_iServiceType][m_iCcrType])
//del//        {
//del//            iRet = 0;
//del//            return iRet;
//del//        }
//del//        if (pInfo->IsGroup)
//del//        {
//del//            iDataLen=EncodeNodeHead(pInfo);
//del//            iDataRealLen = iDataLen;
//del//            int iHeadlen = iDataLen;
//del//            TBaseListItem *pItem = pInfo->SubAvpItem;
//del//            while(pItem)
//del//            {
//del//                TAvpInfo * pTempNode = (TAvpInfo *)pItem->data;
//del//                TAvpNode *pNode = NULL;
//del//                if (pTempNode != NULL)
//del//                {
//del//                    memset(m_tAvpNode.AvpKey,0,sizeof(m_tAvpNode.AvpKey));
//del//                    strncpy(m_tAvpNode.AvpKey,pTempNode->AvpKey,sizeof(m_tAvpNode.AvpKey));
//del//                    pNode = (TAvpNode *)m_pDmtHash->Seek(pTempNode->AvpKey,&m_tAvpNode);
//del//                }
//del//                if (pTempNode->IsGroup && !GetEncodeFlag(pTempNode,false,false) && (pNode == NULL))
//del//                {
//del//                    pItem = pItem->next;
//del//                    continue;
//del//                }
//del//                iDataLen +=EncodeNode(pTempNode,pNode);
//del//                iDataRealLen +=iDataLen;
//del//                pItem = pItem->next;
//del//            }
//del//            if (iDataLen>iHeadlen)
//del//            {
//del//                int iLen=IntToBin(pOutData,iDataLen);
//del//                memcpy(m_pDataBuffer+pInfo->AvpLenLoc,pOutData+1,3);
//del//            }
//del//            else
//del//            {
//del//                iDataLen = 0;
//del//                iDataRealLen = 0;
//del//                m_iDataIndex -= iHeadlen;
//del//                memset(m_pDataBuffer+m_iDataIndex,0,sizeof(m_pDataBuffer)-m_iDataIndex);
//del//            }
//del//        }
//del//        else
//del//        {
//del//            pEventAttr = m_pRateEvent->GetAttrEx(pInfo->AttrId);
//del//            if (!pEventAttr->IsEmpty())
//del//            {
//del//                iDataLen=EncodeNodeHead(pInfo);
//del//                iDataRealLen = iDataLen;
//del//                switch(pInfo->AvpType)
//del//                {
//del//                case AVP_TYPE_INT:
//del//                    {
//del//                        iDataVauleLen = IntToBin(pOutData,pEventAttr->AsInteger());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_LONG:
//del//                    {
//del//                        iDataVauleLen = LlongToBin(pOutData,pEventAttr->AsInteger());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_UINT:
//del//                    {
//del//                        iDataVauleLen = UintToBin(pOutData,pEventAttr->AsInteger());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_ULONG:
//del//                    {
//del//                        iDataVauleLen = UllongToBin(pOutData,pEventAttr->AsInteger());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_CHAR:
//del//                    {
//del//                        iDataVauleLen = StrToByte(pOutData,pEventAttr->AsString());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_ADRESS:
//del//                    {
//del//                        iDataVauleLen = CnvtIpvToDmtIP(pOutData,pEventAttr->AsString());
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_MEM:
//del//                    {
//del//                    	  memcpy(pOutData,pEventAttr->AsString(),pEventAttr->GetSize());
//del//                    	  iDataVauleLen = pEventAttr->GetSize();
//del//                    	  pOutData[iDataVauleLen] = '\0';
//del//                        break;
//del//                    }
//del//                default:
//del//                    break;
//del//                }
//del//                if (iDataVauleLen<0)
//del//                {
//del//                    return iRet;
//del//                }
//del//                memcpy(m_pDataBuffer+m_iDataIndex,pOutData,iDataVauleLen);
//del//                //iDataVauleLen = iDataVauleLen%4==0?iDataVauleLen:(iDataVauleLen+(4-iDataVauleLen%4));
//del//                //m_iDataIndex +=iDataVauleLen;
//del//                m_iDataIndex += (iDataVauleLen%4==0?iDataVauleLen:(iDataVauleLen+(4-iDataVauleLen%4)));
//del//                //iDataLen += iDataVauleLen;
//del//                iDataLen += (iDataVauleLen%4==0?iDataVauleLen:(iDataVauleLen+(4-iDataVauleLen%4)));
//del//                iDataRealLen += iDataVauleLen;
//del//
//del//                //int iLen=IntToBin(pOutData,iDataLen);
//del//                int iLen=IntToBin(pOutData,iDataRealLen);
//del//                memcpy(m_pDataBuffer+pInfo->AvpLenLoc,pOutData+1,3);
//del//            }
//del//            else if (pNode != NULL)
//del//            {
//del//                iDataLen = EncodeNode(pNode);
//del//                iDataRealLen = iDataLen;
//del//            }
//del//            else if (pInfo->bOption[m_iServiceType][m_iCcrType])
//del//            {
//del//                long long lValueTempor = 0;
//del//                iDataLen=EncodeNodeHead(pInfo);
//del//                iDataRealLen = iDataLen;
//del//                switch(pInfo->AvpType)
//del//                {
//del//                case AVP_TYPE_INT:
//del//                    {
//del//                        lValueTempor = (long long)atoi(pInfo->cDefaultValue);
//del//                        iDataVauleLen = IntToBin(pOutData,lValueTempor);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_LONG:
//del//                    {
//del//                        lValueTempor = (long long)atol(pInfo->cDefaultValue);
//del//                        iDataVauleLen = LlongToBin(pOutData,lValueTempor);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_UINT:
//del//                    {
//del//                        lValueTempor = (long long)strtoul(pInfo->cDefaultValue,0,10);
//del//                        iDataVauleLen = UintToBin(pOutData,lValueTempor);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_ULONG:
//del//                    {
//del//                        lValueTempor = (long long)strtoul(pInfo->cDefaultValue,0,10);
//del//                        iDataVauleLen = UllongToBin(pOutData,lValueTempor);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_CHAR:
//del//                    {
//del//                        iDataVauleLen = StrToByte(pOutData,pInfo->cDefaultValue);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_ADRESS:
//del//                    {
//del//                        iDataVauleLen = CnvtIpvToDmtIP(pOutData,pInfo->cDefaultValue);
//del//                        break;
//del//                    }
//del//                case AVP_TYPE_MEM:
//del//                    {
//del//                    	  iDataVauleLen = StrToByte(pOutData,pInfo->cDefaultValue);
//del//                        break;
//del//                    }
//del//                default:
//del//                    break;
//del//                }
//del//                if (iDataVauleLen<0)
//del//                {
//del//                    return iRet;
//del//                }
//del//                memcpy(m_pDataBuffer+m_iDataIndex,pOutData,iDataVauleLen);
//del//                //iDataVauleLen = iDataVauleLen%4==0?iDataVauleLen:(iDataVauleLen+(4-iDataVauleLen%4));
//del//                //m_iDataIndex +=iDataVauleLen;
//del//                m_iDataIndex +=(iDataVauleLen%4==0?iDataVauleLen:(iDataVauleLen+(4-iDataVauleLen%4)));
//del//                //iDataLen += iDataVauleLen;
//del//                iDataLen += (iDataVauleLen%4==0?iDataVauleLen:(iDataVauleLen+(4-iDataVauleLen%4)));
//del//				iDataRealLen += iDataVauleLen;
//del//
//del//                //int iLen=IntToBin(pOutData,iDataLen);
//del//                int iLen=IntToBin(pOutData,iDataRealLen);
//del//                memcpy(m_pDataBuffer+pInfo->AvpLenLoc,pOutData+1,3);
//del//            }
//del//        }
//del//    }
//del//    return iRet = iDataLen;
//del//}

int TAvpEncode::EncodeNodeHead(TAvpInfo *pInfo,unsigned char * mp_pDataBuffer,int &mp_iDataIndex)
{
    int iRet = mp_iDataIndex;
    unsigned char  cTemp[64]= {0};//ZMP:477242
    //memset(cTemp,0,sizeof(cTemp));
    int iLen=0;

    iLen=IntToBin(cTemp,(int)pInfo->AvpCode);
    memcpy(mp_pDataBuffer+mp_iDataIndex,cTemp,(size_t)iLen);
    mp_iDataIndex+=4;
    pInfo->AvpLenLoc = mp_iDataIndex +1;
    unsigned char cFlag = (unsigned char)((pInfo->bMustFlag?0x40:0x00)|(pInfo->VendorId>0?0x80:0x00));//ZMP:477242
    memcpy(mp_pDataBuffer+mp_iDataIndex,&cFlag,1);
    mp_iDataIndex +=4;
    
    if (pInfo->VendorId > 0)
    {
        memset(cTemp,0,sizeof(cTemp));
        iLen=IntToBin(cTemp,(int)pInfo->VendorId);
        memcpy(mp_pDataBuffer+mp_iDataIndex,cTemp,(size_t)iLen);
        mp_iDataIndex+=4;
    }

    return iRet = mp_iDataIndex - iRet;
}

//del//bool TAvpEncode::GetEncodeFlag(TAvpInfo *pInfo,bool IsOperStr,bool IsEncodeView)
//del//{
//del//    bool bRetFlag = false;
//del//    TAvpInfo *pInfoTemp = pInfo;
//del//
//del//    if ((IsEncodeView && pInfo->bOption[m_iServiceType][m_iCcrType])
//del//        ||(!IsEncodeView && pInfo->bOption[m_iServiceType][m_iCcrType] && 
//del//           (pInfo->ParentAvpInfo->AvpLevel == 0 || pInfo->ParentAvpInfo->bOption[m_iServiceType][m_iCcrType])))
//del//    {
//del//        bRetFlag = true;
//del//        return bRetFlag;
//del//    }
//del//
//del//    if (pInfo->bNoEncode[m_iServiceType][m_iCcrType])
//del//    {
//del//        bRetFlag = false;
//del//        return bRetFlag;
//del//    }
//del//
//del//    if (pInfo->IsGroup)
//del//    {
//del//        TBaseListItem *pItem = pInfo->SubAvpItem;
//del//        while (pItem != NULL)
//del//        {
//del//            pInfoTemp = (TAvpInfo *)pItem->data;
//del//            bRetFlag = GetEncodeFlag(pInfoTemp,IsOperStr,IsEncodeView);
//del//            if (bRetFlag)
//del//            {
//del//                break;
//del//            }
//del//            pItem = pItem->next;
//del//        }
//del//    }
//del//    else
//del//    {
//del//        //oper//if (!IsOperStr)
//del//        //oper//{
//del//        //oper//    TEventAttr *pEventAttr = NULL;
//del//        //oper//    pEventAttr = m_pRateEvent == NULL?NULL:m_pRateEvent->GetAttrEx(pInfoTemp->AttrId);
//del//        //oper//    if ((pEventAttr!=NULL) && !pEventAttr->IsEmpty())
//del//        //oper//    {
//del//        //oper//        bRetFlag = true;
//del//        //oper//        return bRetFlag;
//del//        //oper//    }
//del//        //oper//}
//del//        if (!IsOperStr) return true;
//del//    }
//del//    return bRetFlag;
//del//}

void TAvpEncode::SetMsgSerType(int iServiceType,int iCcrType)
{
    if (iServiceType > 0 && (iServiceType< 12) && iCcrType>-1 && iCcrType<5)
    {
        m_iServiceType = iServiceType -1;
        m_iCcrType = iCcrType-1;
    }
    else if (iServiceType >= 12  && iServiceType <= 13 && iCcrType>-1 && iCcrType<5)
    {
        m_iServiceType = iServiceType-2;
        m_iCcrType = iCcrType-1;
    }

}
void TAvpEncode::SetDmtHash(THashList *pDmtHash)
{
    if (pDmtHash!=NULL)
    {
        m_pDmtHash = pDmtHash;
        m_bDoubleSrc = true;
    }
    else
    {
        m_bDoubleSrc = false;
    }
}
void TAvpEncode::SetConfigInfo(TAvpFile *pAvpFile)
{
    if (pAvpFile != NULL)
    {
        m_pAvpFile = pAvpFile;
    }
}
string TAvpEncode::ToString(void)
{
    ostringstream sLine;
    return sLine.str();
}
void TAvpEncode::SetMsgHeadInfo(TAvpHead *pInfo)
{
    m_pAvpHead->Assign(pInfo);
}
int TAvpEncode::CnvtIpvToDmtIP(unsigned char *pcOut,const char *psSrc)
{
    int iRet = -1;
    if(!psSrc)
    {
        return iRet;
    }
    static TSplit split;
    const char *pTemp = NULL;
    const char *pInTemp = psSrc;
    unsigned char  *pOutTemp = pcOut;
    int iTemp = 0;
    pTemp = strchr(pInTemp,'.');
    if (pTemp != NULL)
    {
        split.SetSplitter('.');
        if (split.SetString(pInTemp)>0)
        {
            if (split.GetCount()!=4)
            {
                return iRet;
            }
            else
            {
                iRet = 6;
                pOutTemp[0] = 0x00;
                pOutTemp[1] = 0x01;
                for (int i=0;i<4;i++)
                {
                    iTemp = atoi(split[i]);
                    pOutTemp[2+i] = (unsigned char)iTemp;//ZMP:477242
                }
            }
        }
    }
    else
    {
        split.SetSplitter(':');
        if (split.SetString(pInTemp)>0)
        {
            if (split.GetCount()!=8)
            {
                return iRet;
            }
            else
            {
                iRet = 10;
                pOutTemp[0] = 0x00;
                pOutTemp[1] = 0x02;
                for (int i=0;i<8;i++)
                {
                    iTemp = atoi(split[i]);
                    //适应UNIX的网络字节序_BIG_ENDIAN_
                    pOutTemp[3+i] = (unsigned char)iTemp;//ZMP:477242
                    pOutTemp[2+i] = (unsigned char)(iTemp>>=8);
                }
            }
        }
    }
    return iRet;
}
int TAvpEncode::IntToBin(unsigned char *pcOut,int iData)
{
    int iLen=-1;

    unsigned char pcData[10] = {0};//ZMP:477242
    //memset(pcData,0,sizeof(pcData));

    unsigned int iDataAbs=(unsigned int)ABS(iData);
    if(iDataAbs > 0x00ffffff)
    {
        iLen=4;
        pcData[3]=(unsigned char)(iDataAbs & 0x000000ff);//ZMP:477242
        pcData[2]=(unsigned char)((iDataAbs>>=8)&0x0000f);
        pcData[1]=(unsigned char)((iDataAbs>>=8)&0x00ff);
        pcData[0]=(unsigned char)(iDataAbs>>8);
    }
    else if(iDataAbs>0x0000ffff)
    {
        iLen=3;
        pcData[2]=(unsigned char)((iDataAbs)&0x0000ff);
        pcData[1]=(unsigned char)((iDataAbs>>=8)&0x00ff);
        pcData[0]=(unsigned char)(iDataAbs>>8);
    }
    else if(iDataAbs>0x000000ff)
    {
        iLen=2;
        pcData[1]=(unsigned char)(iDataAbs%256);
        pcData[0]=(unsigned char)(iDataAbs>>8);
    }
    else
    {
        iLen=1;
        pcData[0]=(unsigned char)iDataAbs;
    }
    if(iData<0)//小于0 ,四个字节，取绝对值的补码
    {
        int i=0;
        for(;i<(4-iLen);i++)
        {
            pcOut[i]=0x00;
        }
        for(int j=0;(i+j)<4;j++)
        {
            pcOut[i+j]=pcData[j];
        }
        for(int j=0;j<4;j++)
        {
            pcOut[j]=(unsigned char)(~pcOut[j]);
        }
        for(int j=3;j>=0;j--)
        {
            pcOut[j] = (unsigned char)(pcOut[j]+1); //反码加1
            if(pcOut[j] != 0)//如果没有进位
                break;
        }
        iLen=4;
    }
    else
    {
        if(iLen==4)
        {
            for(int i=0;i<iLen;i++)
            {
                pcOut[i]=pcData[i];
            }
        }
        else
        {
            int i=0;
            int j=0;
            for (i=0;i<(4-iLen);i++)
            {
                pcOut[i]=0x00;
            }
            for (j=0;j<iLen;j++)
            {
                pcOut[i++]=pcData[j];
            }
            iLen=4;
        }
    }

    return iLen;
}
int TAvpEncode::StrToByte(unsigned char *psOut,const char *psSrc)
{
    if(!psOut || !psSrc)
    {
        return -1;
    }
    int i=0;
    for(char c=(*psSrc);c!='\0';c=*(psSrc+i))
    {
        *(psOut+i)=(unsigned char)c;
        i++;
    }

    return i;
}
int TAvpEncode::UintToBin(unsigned char *pcOut,unsigned int iData)
{
    int iLen=-1;

    unsigned char pcData[8] = {0};//ZMP:477242
    //memset(pcData,0,sizeof(pcData));

    if(iData > 0x00ffffff)
    {
        iLen=4;
        pcData[3]=(unsigned char)(iData & 0x000000ff);//ZMP:477242
        pcData[2]=(unsigned char)((iData>>=8)&0x0000ff);
        pcData[1]=(unsigned char)((iData>>=8)&0x00ff);
        pcData[0]=(unsigned char)(iData>>8);
    }
    else if(iData>0x0000ffff)
    {
        iLen=3;
        pcData[2]=(unsigned char)((iData)&0x0000ff);
        pcData[1]=(unsigned char)((iData>>=8)&0x00ff);
        pcData[0]=(unsigned char)(iData>>8);
    }
    else if(iData>0x000000ff)
    {
        iLen=2;
        pcData[1]=(unsigned char)((iData)&0x00ff);
        pcData[0]=(unsigned char)(iData>>8);
    }
    else
    {
        iLen=1;
        pcData[0]=(unsigned char)(iData&0xff);
    }
    if(iLen==4)
    {
        for(int i=0;i<iLen;i++)
        {
            pcOut[i]=pcData[i];
        }
    }
    else
    {
        int i=0;
        for (i=0;i<(4-iLen);i++)
        {
            pcOut[i]=0x00;
        }
        for (int j=0;j<iLen;j++)
        {
            pcOut[i++]=pcData[j];
        }
        iLen=4;
    }
    return iLen;
}
int TAvpEncode::UllongToBin(unsigned char *pcOut,unsigned long long iData)
{
    int iLen=-1;
    int iHighLen = -1; //高位的长度
    int iLowLen = -1; //低位的长度
    unsigned char pcData[8] = {0};//ZMP:477242
    //memset(pcData,0,sizeof(pcData));
    counterllong llData={0,0};
    llData.high = (unsigned int)((iData>>32)&0xffffffff);
    llData.low= (unsigned int)(iData&0xffffffff);


    if(llData.high> 0x00ffffff)
    {
        iHighLen=8;
        pcData[7]=(unsigned char)(llData.high & 0x000000ff);//ZMP:477242
        pcData[6]=(unsigned char)((llData.high>>=8)&0x0000ff);
        pcData[5]=(unsigned char)((llData.high>>=8)&0x00ff);
        pcData[4]=(unsigned char)(llData.high>>8);
    }
    else if(llData.high>0x0000ffff)
    {
        iHighLen=7;
        pcData[6]=(unsigned char)((llData.high)&0x0000ff);
        pcData[5]=(unsigned char)((llData.high>>=8)&0x00ff);
        pcData[4]=(unsigned char)(llData.high>>8);
    }
    else if(llData.high>0x000000ff)
    {
        iHighLen=6;
        pcData[5]=(unsigned char)((llData.high)&0x00ff);
        pcData[4]=(unsigned char)(llData.high>>8);
    }
    else
    {
        iHighLen=5;
        pcData[4]=(unsigned char)llData.high;
    }

    //低位处理
    if(llData.low > 0x00ffffff)
    {
        iLowLen=4;
        pcData[3]=(unsigned char)(llData.low & 0x000000ff);
        pcData[2]=(unsigned char)((llData.low>>=8)&0x0000ff);
        pcData[1]=(unsigned char)((llData.low>>=8)&0x00ff);
        pcData[0]=(unsigned char)(llData.low>>8);
    }
    else if(llData.low>0x0000ffff)
    {
        iLowLen=3;
        pcData[2]=(unsigned char)((llData.low)&0x0000ff);
        pcData[1]=(unsigned char)((llData.low>>=8)&0x00ff);
        pcData[0]=(unsigned char)(llData.low>>8);
    }
    else if(llData.low>0x000000ff)
    {
        iLowLen=2;
        pcData[1]=(unsigned char)((llData.low)&0x00ff);
        pcData[0]=(unsigned char)(llData.low>>8);
    }
    else
    {
        iLowLen=1;
        pcData[0]=(unsigned char)llData.low;
    }

    //高位的空位置0
    int i = 0;
    for (i=0; i<(8-iHighLen); i++)
    {
        pcOut[i] = 0x00;
    }

    //高位的非空位赋值
    for (int j=0; j<(iHighLen-4); j++)
    {
        pcOut[i++] = pcData[4+j];
    }

    //低位的空位置0
    for (int j=0; j<(4-iLowLen); j++)
    {
        pcOut[i++] = 0x00;
    }

    //低位的非空位赋值
    for (int j=0; j<iLowLen; j++)
    {
        pcOut[i++] = pcData[j];
    }

    iLen = 8;
    return iLen;
}

int TAvpEncode::LlongToBin(unsigned char *pcOut,long long iData)
{
  int iLen=-1;
  int iHighLen = -1;
  int iLowLen = -1;

  unsigned char pcData[8] = {0};//ZMP:477242
  //memset(pcData,0,sizeof(pcData));
  unsigned long long iDataAbs=(unsigned long long)ABS(iData);
  counterllong llData={0,0};
  llData.high = (unsigned int)((iDataAbs>>32)&0xffffffff);
  llData.low= (unsigned int)(iDataAbs&0xffffffff);

  if(llData.high> 0x00ffffff)
  {
    iHighLen=8;
    pcData[7]=(unsigned char)(llData.high & 0x000000ff);//ZMP:477242
    pcData[6]=(unsigned char)((llData.high>>=8)&0x0000ff);
    pcData[5]=(unsigned char)((llData.high>>=8)&0x00ff);
    pcData[4]=(unsigned char)(llData.high>>8);
  }
  else if(llData.high>0x0000ffff)
  {
    iHighLen=7;
    pcData[6]=(unsigned char)((llData.high)&0x0000ff);
    pcData[5]=(unsigned char)((llData.high>>=8)&0x00ff);
    pcData[4]=(unsigned char)(llData.high>>8);
  }
  else if(llData.high>0x000000ff)
  {
    iHighLen=6;
    pcData[5]=(unsigned char)((llData.high)&0x00ff);
    pcData[4]=(unsigned char)(llData.high>>8);
  }
  else
  {
    iHighLen=5;
    pcData[4]=(unsigned char)(llData.high);
  }
  
  if(llData.low > 0x00ffffff)
  {
    iLowLen=4;
    pcData[3]=(unsigned char)(llData.low & 0x000000ff);
    pcData[2]=(unsigned char)((llData.low>>=8)&0x0000ff);
    pcData[1]=(unsigned char)((llData.low>>=8)&0x00ff);
    pcData[0]=(unsigned char)(llData.low>>8);
  }
  else if(llData.low>0x0000ffff)
  {
    iLowLen=3;
    pcData[2]=(unsigned char)((llData.low)&0x0000ff);
    pcData[1]=(unsigned char)((llData.low>>=8)&0x00ff);
    pcData[0]=(unsigned char)(llData.low>>8);
  }
  else if(llData.low>0x000000ff)
  {
    iLowLen=2;
    pcData[1]=(unsigned char)((llData.low)&0x00ff);
    pcData[0]=(unsigned char)(llData.low>>8);
  }
  else
  {
    iLowLen=1;
    pcData[0]=(unsigned char)(llData.low);
  }

    int i=0;

    // 将高位的空位补0
    for(i = 0; i< (8 - iHighLen);i++)
    {
        pcOut[i] = 0x00;
    }

    // 赋值高位
    int j = 0;
    for(j = 0;j< (iHighLen -4);j++)
    {
        pcOut[i+j] = pcData[4+j];   	
    }

    // 调整当前下标
    i = i+j;

    // 将低位的空位补0      
    if((iLowLen < 4))
    {
        for(j =0;j< (4-iLowLen); j++)
        {
            if(i+j >= 8)break;
            pcOut[i+j] = 0x00;
        }

        // 调整当前下标
        i = i+j;
    }    	


    // 赋值低位
    for(j =0;j< iLowLen;j++)
    {
        pcOut[i+j]=pcData[j];
    }

    // 调整当前下标
    i = i+j;
    
    // 如果是负数，取绝对值的补码
    if(iData<0)
    {
        // 按位取反
        for(j=0;j<8;j++)
        {
            pcOut[j]=(unsigned char)(~pcOut[j]);
        }

        // 取补码
        for(j=7;j>=0;j--)
        {
            pcOut[j] = (unsigned char)(pcOut[j]+1); //反码加1
            if(pcOut[j] != 0)//如果没有进位
                break;
        }
    }

    iLen = 8;
    return iLen;
}

TAvpSingle::TAvpSingle()
{
    m_iSrcDataLen = 0;//原始数据长度
    memset(m_cSrcDataBuf,0,sizeof(m_cSrcDataBuf));//原始数据buffer
    memset(m_cResultDataBuf,0,sizeof(m_cResultDataBuf));//存放结果数据Value
    m_bHasFlag = false;//是否找到的标志
    m_iConNumber = 0;//查询路径深度
    memset(m_iCondition,0,sizeof(m_iCondition));//查询条件路径
    //ZMP:305593
    m_iResultDataLen = 0;
    m_iResultDataLoc = 0;
    m_iFindLoc       = 0;
}

TAvpSingle::~TAvpSingle()
{
}
void TAvpSingle::ClearData(void)
{
    m_iFindLoc = 20;
    memset(m_cResultDataBuf,0,sizeof(m_cResultDataBuf));//存放结果数据Value
    m_iResultDataLen = 0;
    m_bHasFlag = false;//是否找到的标志
    m_iConNumber = 0;//查询路径深度
    memset(m_iCondition,0,sizeof(m_iCondition));//查询条件路径
}
//设置查询的数据流及长度
bool TAvpSingle::SetAvpSrcData(unsigned char  *pAvp,int iOrgDataLen)
{
    bool bReFlag = true;
    if (pAvp == NULL)
    {
        bReFlag = false;
    }
    else
    {
        ClearData();
        m_iSrcDataLen = 0;//原始数据长度
        memset(m_cSrcDataBuf,0,sizeof(m_cSrcDataBuf));//原始数据buffer
        if ((iOrgDataLen <20) || (iOrgDataLen >4000))
        {
            bReFlag = false;
        }
        else
        {
            m_iSrcDataLen = iOrgDataLen;
            memcpy(m_cSrcDataBuf,pAvp,(size_t)iOrgDataLen);
            memcpy(m_cResultDataBuf,pAvp+20,(size_t)(iOrgDataLen-20));
            m_iResultDataLen = iOrgDataLen-20;
            m_iFindLoc = 20;
            m_iCondition[0][2] = m_iResultDataLen;
        }
    }
    return bReFlag;
}
//获取Avpcode=iAvpCode,VendorId=iVendorId,路径为cFinding的AVP
int TAvpSingle::GetAvpByCondition(int iAvpCode,int iVendorId,const char *cFinding)
{
    bool bReFlag = true;
    SetResultDataBuf();
    bReFlag = SetCondition(iAvpCode,iVendorId,cFinding);
    if (bReFlag)
    {
        int iLoop = 0;
        while (iLoop < m_iConNumber)
        {
            m_bHasFlag = false;
            bReFlag = GetAvpInfoByCode(m_iCondition[iLoop][0],m_iCondition[iLoop][3],m_iCondition[iLoop][1]);
            //当解析过程中遇到错误时,或没有找到数据时,匹配到数据时
            if (!bReFlag || (m_iFindLoc == m_iSrcDataLen) || (m_bHasFlag && (iLoop == m_iConNumber)))
            {
                break;
            }
            if (m_bHasFlag)
            {
                iLoop++;
                m_iCondition[iLoop][2] = m_iFindLoc+m_iResultDataLen;
            }
            else
            {
                do
                {
                    iLoop--;
                }while ((m_iCondition[iLoop][2] == m_iFindLoc) && (iLoop>0));

                if (m_iFindLoc>(m_iSrcDataLen-8))
                {
                    bReFlag = false;
                    break;
                }
                memcpy(m_cResultDataBuf,m_cSrcDataBuf+m_iFindLoc,(size_t)(m_iCondition[iLoop][2]-m_iFindLoc));
                m_iResultDataLen = m_iCondition[iLoop][2]-m_iFindLoc;
            }
        }
    }
    if (!bReFlag || !m_bHasFlag)
    {
        m_iResultDataLen = -1;
    }
    return m_iResultDataLen;
}
//在相同的层次中查询Avp
bool TAvpSingle::GetAvpInfoByCode(int iAvpCode,int iTimes,int iVendorId)
{
    if (m_iResultDataLen<8)
    {
        return false;
    }
    unsigned char  pcTmp[8] = {0};
    int iPos=0;
    int iAvpLenTemp=0;
    int iAvpCodeTemp=0;
    int iVendorIdTemp=0;
    bool bVTagTemp=false;
    //memcpy(pcTmp, m_cResultDataBuf, 8);
    //printf("Seeking AVP:%d\n",iAVPCode);
    while( (iPos < m_iResultDataLen) && !m_bHasFlag)
    {
        int iTempLoc = 0;
        memcpy(pcTmp,m_cResultDataBuf+iPos,8);
        iTempLoc +=8;
        //解析AvpCode
        iAvpCodeTemp = pcTmp[3]+pcTmp[2]*256 + pcTmp[1]*256*256 + pcTmp[0]*256*256*256;
        //解析AvpLength
        iAvpLenTemp = pcTmp[7] + pcTmp[6]*256 + pcTmp[5]*256*256;
        if ((iAvpLenTemp > m_iResultDataLen) || (iAvpLenTemp<8))
        {
            return false;
        }
        //解析AvpVendorId
        bVTagTemp = ((pcTmp[4] & V_BODY_FLAG) == V_BODY_FLAG);
        if (bVTagTemp)
        {
            iTempLoc +=4;
            memcpy(pcTmp,m_cResultDataBuf+iPos+8,4);
            iVendorIdTemp = pcTmp[3]+pcTmp[2]*256 + pcTmp[1]*256*256 + pcTmp[0]*256*256*256;
        }
        //规整avp数据长度
        if(iAvpLenTemp%4 != 0) 
        {
            iAvpLenTemp = iAvpLenTemp - iAvpLenTemp%4 +4;
        }
        if (iAvpLenTemp > m_iResultDataLen)
        {
            return false;
        }
        if((iAvpCodeTemp == iAvpCode) && (iVendorIdTemp == iVendorId))
        {
            if (iTimes >1)
            {
                iTimes--;
                m_iFindLoc+=iAvpLenTemp;
                iPos += iAvpLenTemp;
                iAvpLenTemp = 0;
                iAvpCodeTemp = 0;
                iVendorIdTemp = 0;
                bVTagTemp = false;
                memset(pcTmp, 0, sizeof(pcTmp));
            }
            else if (iTimes==1)
            {
                //位置平移
                m_iFindLoc+=iTempLoc;
                //保存数据
                memset(m_cResultDataBuf,0,sizeof(m_cResultDataBuf));
                memcpy(m_cResultDataBuf,m_cSrcDataBuf+m_iFindLoc,(size_t)(iAvpLenTemp-iTempLoc));
                m_iResultDataLen = iAvpLenTemp-iTempLoc;
                m_bHasFlag = true;
            }
        }
        else
        {
            m_iFindLoc+=iAvpLenTemp;
            iPos += iAvpLenTemp;
            iAvpLenTemp = 0;
            iAvpCodeTemp = 0;
            iVendorIdTemp = 0;
            bVTagTemp = false;
            memset(pcTmp, 0, sizeof(pcTmp));
        }
    }
    return true;
}
//将查询条件设置在m_iCondition中
bool TAvpSingle::SetCondition(int iAvpCode,int iVendorId,const char *cFinding)
{
    bool bReFlag = true;
    if (cFinding == NULL)
    {
        m_iConNumber = 1;
        m_iCondition[0][0] = iAvpCode;
        m_iCondition[0][1] = iVendorId;
        m_iCondition[0][3] = 1;//记录重复查询的次数
    }
    else
    {
        int i = 0;
        static TSplit splitInfo1,splitInfo12;
        splitInfo1.SetSplitter('|');
        if(splitInfo1.SetString(cFinding)>0)
        {
            m_iConNumber = splitInfo1.GetCount();
            for(i =0; i< m_iConNumber;i++)
            {
                splitInfo12.SetSplitter(',');
                if(splitInfo12.SetString(splitInfo1[i]) == 2)
                {
                    m_iCondition[i][0] = atoi(splitInfo12[0]);
                    m_iCondition[i][1] = atoi(splitInfo12[1]);
                    m_iCondition[i][3] = 1;//记录重复查询的次数
                }
                else if(splitInfo12.SetString(splitInfo1[i]) == 3)
                {
                    m_iCondition[i][0] = atoi(splitInfo12[0]);
                    m_iCondition[i][1] = atoi(splitInfo12[1]);
                    m_iCondition[i][3] = atoi(splitInfo12[2]);//记录重复查询的次数
                }
                else
                {
                    bReFlag = false;
                }
            }
        }
    }
    return bReFlag;
}
void TAvpSingle::SetResultDataBuf()
{
    m_cResultDataBuf[0]='\0';
    m_iResultDataLen = 0;
    if (m_iSrcDataLen > 20)
    {
        m_iResultDataLen = m_iSrcDataLen-20 > 4000 ? 4000 : m_iSrcDataLen-20;
        memcpy(m_cResultDataBuf,m_cSrcDataBuf+20,(size_t)m_iResultDataLen);
    }
}

