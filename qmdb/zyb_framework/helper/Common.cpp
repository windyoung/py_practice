//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#include <iostream>
#include <string>
#include <time.h>
#ifdef WIN32
#include <process.h>
#include "io.h"
#include "direct.h"
#endif
#include <sstream>
#include "Common.h"
#include "TStrFunc.h"
#include "TException.h"
//#include "TShmTable.h"
#include "debug_new.h"

//ZMP:467370   
#if defined(WIN32) || defined(_WIN32) 
#define snprintf _snprintf
#endif

/*������־�ļ�*/
const long MAX_PERF_LOG_LINE=300000;

char  g_progName[64];
char  g_sExcuteCmd[128]; //��������������

//TPerfStatMgr g_PerfStatMgr;

//////////////////////////////////////////////////////////////////////////
//
// TBaseObject ����
//
//////////////////////////////////////////////////////////////////////////
TBaseObject::TBaseObject()
{
  // memset(sObjectName,0,OBJECT_NAME_SIZE+1);
  // strcpy(sObjectName,"Object");
  Tag = 1119;
}

TBaseObject::~TBaseObject()
{
}

//���������Ϣ
string TBaseObject::ToString()
{
  string LineText("test");
  return LineText;
}

char * TBaseObject::ToStringEx()
{
  return NULL;
}

//��������
void TBaseObject::Assign(TBaseObject *pObject)
{
  // strncpy(this->sObjectName,pObject->GetObjectName(),OBJECT_NAME_SIZE);
}

int TBaseObject::Compare(int iKey) const
{
  throw TException("TBaseObject::Compare(int iKey) : Not Write Code.");
}

int TBaseObject::Compare(int iKey1, int iKey2) const
{
  throw TException("TBaseObject::Compare(int iKey1, int iKey2) : Not Write Code.");
}

int TBaseObject::Compare(const TBaseObject *pObject) const
{
  throw TException("TBaseObject::Compare(const TBaseObject *pObject) : Not Write Code.");
}

int TBaseObject::CompareKey(const TBaseObject *pObject) const
{
  throw TException("TBaseObject::CompareKey() : Not Write Code.");
}

bool TBaseObject::CompareKeyByName(const TBaseObject *pObject) const
{
  throw TException("TBaseObject::CompareKeyByName() : Not Write Code.");
}

bool TBaseObject::Assigned(const TBaseObject *pObject)
{
  bool bRetFlag = false;
  if(pObject)
  {
    try
    {
      bRetFlag = pObject->Tag == 1119;
    }
    catch(...)
    {
    }

  }
  return bRetFlag;
}

//////////////////////////////////////////////////////////////////////////
//
// TBaseObject ����
//
//////////////////////////////////////////////////////////////////////////
TRecordData::TRecordData()
{
  RecordType=RECORD_TYPE::ALL_TYPE;
  //ZMP:305593
  memset(sObjectName,0,sizeof(sObjectName));
  
}

TRecordData::~TRecordData()
{

}

void TRecordData::ClearData()
{

}

bool TRecordData::SetData(const int iDataCode,const char *sDataValue,const char *sDataPath)
{
  printf("TRecordData::SetData(DataCode=[%d],DataValue=[%s],DataPath=[%s])\n",iDataCode,sDataValue,sDataPath);
  return false;
}

//Example:
//iDataCode,"sDataValue",sDataPath|...|iDataCode,"sDataValue",sDataPath
bool TRecordData::SetDataString(const char *sDataString)
{
    //ZMP:312672,Ȧ���Ӷȴ����ع�,�������ò�������ֱ��ע�͵�
    /*
    bool bRetflag = false;

    if(sDataString && strlen(sDataString)>0)
    {
    char *sPtr = (char *)sDataString;

    int iDataCode=0;
    char sDataValue[255]={0};
    char sDataPath[128]={0};

    int iBegin,iIndex=0;
    char sText[255]={0},*sTextPtr;
    bool bNewFlag = true;

    while(*sPtr!='\0')
    {
      if(bNewFlag)
      {
        bNewFlag = false;
        if(iIndex>=1&&iIndex<=3)
        {
          sTextPtr = sText + strlen(sText) -1;
          while(*sTextPtr!='\0' && *sTextPtr==' ' && sTextPtr>=sText)
          {
            *sTextPtr='\0';
            sTextPtr--;
          }
          sTextPtr = sText;
          while(*sTextPtr!='\0' && *sTextPtr==' ')
          {
            sTextPtr++;
          }
        }
        switch(iIndex)
        {
        case 1: 
          iDataCode = atol(sTextPtr);                           
          break;
        case 2: 
          strncpy(sDataValue,sTextPtr,sizeof(sDataValue)-1);                     
          break;
        case 3: 
          strncpy(sDataPath,sTextPtr,sizeof(sDataPath)-1); 
          iIndex=0;
          if(SetData(iDataCode,sDataValue,sDataPath)==false)
          {
            cout << "TRecordData::SetDataString() : DataCode=" << iDataCode << " DataPath=" << sDataPath << " DataValue=" << sDataValue << " Data Not Found!"<<endl;
          }
          iDataCode=0;
          memset(sDataValue,0,sizeof(sDataValue));
          memset(sDataPath,0,sizeof(sDataPath));
          break;
        }

        iBegin = 0;
        memset(sText,0,sizeof(sText));
      }

      //����ڶ��У��տ�ʼ�����Ϊ""
      if(iIndex==1)
      {
        if(*sPtr=='"')
        {
          iIndex++;
          sPtr++;
          while(*sPtr !='\0' && *sPtr==' ')
          {
            sPtr++;
          }
          bNewFlag = true;
        } 
        else 
        {
          if(*sPtr=='\\')
          {
            sPtr++;
          }
          sText[iBegin++] = *sPtr;
        }
      }
      else
      {
        if(*sPtr==',' || *sPtr=='\r' || *sPtr=='\n')
        {
          iIndex++;
          if(iIndex==1)
          {                       
            sPtr++;

            while(*sPtr !='\0' && *sPtr==' ')
            {
              sPtr++;
            }

            if(*sPtr!='"')
            {
              throw TException("TRecordData::SetDataString([%s]) : Postion=[%d] Is Not '\"',Format Error!",sDataString,sPtr-sDataString);
            }
          }

          if(iIndex==3)
          {
            if(*sPtr=='\r')
            {
              if(*(sPtr+1)=='\n')
                sPtr++;
            }
            if(*sPtr=='\n')
            {
              if(*(sPtr+1)=='\r')
                sPtr++;
            }
            if(*sPtr!='\r'&&*sPtr!='\n')
            {
              throw TException("TRecordData::SetDataString([%s]) : Postion=[%d] Is Not '\r\n',Format Error!",sDataString,sPtr-sDataString);
            }
          }

          bNewFlag = true;
        }
        else
        {
          sText[iBegin++] = *sPtr;
        }
      }
      sPtr++;
    }
    if(bNewFlag==false)
    {
      iIndex++;
      if(iIndex>=1&&iIndex<=3)
      {
        sTextPtr = sText + strlen(sText) -1;
        while(*sTextPtr!='\0' && *sTextPtr==' ' && sTextPtr>=sText)
        {
          *sTextPtr='\0';
          sTextPtr--;
        }
        sTextPtr = sText;
        while(*sTextPtr!='\0' && *sTextPtr==' ')
        {
          sTextPtr++;
        }
      }
      switch(iIndex)
      {
      case 1: 
        iDataCode = atol(sTextPtr);                           
        break;
      case 2: 
        strncpy(sDataValue,sTextPtr,sizeof(sDataValue)-1); 
        break;
      case 3: 
        strncpy(sDataPath,sTextPtr,sizeof(sDataPath)-1);                 
        if(SetData(iDataCode,sDataValue,sDataPath)==false)
        {
          cout << "TRecordData::SetDataString() : DataCode=" << iDataCode << " DataPath=" << sDataPath << " DataValue=" << sDataValue << " Data Not Found!"<<endl;
        }
        break;
      }
    }
    bRetflag = true;
    }
    return bRetflag;
    */
    return false;
}

int TRecordData::GetCurrPath(const char *sDataPath)
{
  char sCurrPath[DATA_PATH_SIZE+1]={0};
  strncpy(sCurrPath,sDataPath,DATA_PATH_SIZE);
  return atoi(sCurrPath);
}

int TRecordData::GetCurrPathCount(const char *sDataPath)
{
  char sCurrPathCount[DATA_PATH_COUNT+1]={0};
  strncpy(sCurrPathCount,sDataPath+DATA_PATH_SIZE,DATA_PATH_COUNT);
  return atoi(sCurrPathCount);
}

const char * TRecordData::GetNextPath(const char *sDataPath)
{
  return sDataPath+DATA_PATH_SIZE+DATA_PATH_COUNT;
}

//ȡ��������    
const char *TRecordData::GetObjectName() const
{
  return sObjectName;
}

void TRecordData::SetObjectName(const char *sName)
{
  strncpy(sObjectName,sName,OBJECT_NAME_SIZE);
}

//////////////////////////////////////////////////////////////////////////
//
// TLongObject ����
//
//////////////////////////////////////////////////////////////////////////
TLongObject::TLongObject()
{
  SetObjectName("TLongObject");
  m_iValue = -1;
}

TLongObject::TLongObject(llong iValue)
{
  SetObjectName("TLongObject");
  m_iValue = iValue;
}

TLongObject::TLongObject(const TLongObject& right)
{
  Assign((TBaseObject *)&right);
}

TLongObject::~TLongObject()
{
}

void TLongObject::SetValue(llong iValue)
{
  m_iValue = iValue;
}

llong TLongObject::GetValue() const
{
  return m_iValue;
}

//ȡ��������    
const char *TLongObject::GetObjectName() const
{
  return sObjectName;
}

void TLongObject::SetObjectName(const char *sName)
{
  strncpy(sObjectName,sName,OBJECT_NAME_SIZE);
}

//��������
void TLongObject::Assign(TBaseObject *pObject)
{
  if(TBaseObject::Assigned(pObject))
  {
    if (this != pObject)
    {
      //�����Assigned����
      TBaseObject::Assign(pObject);
      m_iValue = ((TLongObject *)pObject)->m_iValue;
    }

  }
}

//���������Ϣ
string TLongObject::ToString()
{
  ostringstream ostrOut;

  ostrOut << sObjectName << endl;
  ostrOut << "" << "Value = [" << m_iValue << "]";

  return ostrOut.str();
}

int TLongObject::Compare(int iKey) const
{
  throw TException("TLongObject::Compare(int iKey) : Not Write Code.");
}

int TLongObject::Compare(int iKey1, int iKey2) const
{
  throw TException("TLongObject::Compare(int iKey1, int iKey2) : Not Write Code.");
}

int TLongObject::Compare(const TBaseObject *pObject) const
{
  if(TBaseObject::Assigned(pObject)==false)
    throw TException("TLLong::Compare() : Object Is NULL");

//  return (m_iValue == ((TLongObject *)pObject)->m_iValue) ? 0 : -1;

  if( m_iValue == ((TLongObject *)pObject)->m_iValue )
    return 0;
  else
  if( m_iValue < ((TLongObject *)pObject)->m_iValue )
    return -1;
  else
    return 1;

}

int TLongObject::CompareKey(const TBaseObject *pObject) const
{
  if(TBaseObject::Assigned(pObject)==false)
    throw TException("TLLong::CompareKey() : Object Is NULL");

  int iRetCode = 0;
  TLongObject *pData2 = (TLongObject *)pObject;


  if (m_iValue > pData2->m_iValue)
  {
    iRetCode = 1;
  }
  else if (m_iValue == pData2->m_iValue)
  {
    iRetCode = 0;
  }
  else
  {
    iRetCode = -1;
  }

  return iRetCode;
}

bool TLongObject::CompareKeyByName(const TBaseObject *pObject) const
{
  if(TBaseObject::Assigned(pObject)==false)
    throw TException("TLLong::CompareKeyByName() : Object Is NULL");

  bool iRetCode = false;
  TLongObject *pData2 = (TLongObject *)pObject;


  if (m_iValue <= pData2->m_iValue)
  {
    iRetCode = true;
  }
  else
  {
    iRetCode = false;
  }

  return iRetCode;
}

//////////////////////////////////////////////////////////////////////////
//
// TStringObject ����
//
//////////////////////////////////////////////////////////////////////////
TStringObject::TStringObject()
{
  SetObjectName("TStringObject");
  m_sValue = NULL;
  //ZMP:305593
  memset(sObjectName,0,sizeof(sObjectName));
}

TStringObject::TStringObject(const char *sStringValue)
{
  if(TStrFunc::IsEmpty(sStringValue)==false)
  {
    size_t iStrLen=strlen(sStringValue);//ZMP:477242
    m_sValue = new char [iStrLen+1];
    strncpy(m_sValue,sStringValue,iStrLen);
    m_sValue[iStrLen]='\0';
  }
  else
  {
    m_sValue = new char [1];
    m_sValue[0]='\0';
  }
  //ZMP:305593
  memset(sObjectName,0,sizeof(sObjectName));
}

TStringObject::TStringObject(const TStringObject& right)
{
  Assign((TBaseObject *)&right);
}

TStringObject::~TStringObject()
{
  ClearData();
}

void TStringObject::SetValue(const char *sStringValue)
{
  if( TStrFunc::IsEmpty(sStringValue)==false )
  {
    ClearData();

    size_t iStrLen=strlen(sStringValue);
    m_sValue = new char [iStrLen+1];
    strncpy(m_sValue,sStringValue,iStrLen);
    m_sValue[iStrLen]='\0';
  }
  else
  {
    m_sValue[0]='\0';
  }
}

const char * TStringObject::GetValue() const
{
  return m_sValue;
}

//ȡ��������    
const char *TStringObject::GetObjectName() const
{
  return sObjectName;
}

void TStringObject::SetObjectName(const char *sName)
{
  strncpy(sObjectName,sName,OBJECT_NAME_SIZE);
}

void TStringObject::Assign(TBaseObject *pObject)
{
  if(TBaseObject::Assigned(pObject))
  {
    if (this != pObject)
    {
      ClearData();

      //�����Assigned����
      TBaseObject::Assign(pObject);

      TStringObject *pStrObj = (TStringObject *)pObject;
      if(TStrFunc::IsEmpty(pStrObj->GetValue())==false)
      {
        size_t iStrLen=strlen(pStrObj->GetValue());
        m_sValue = new char [iStrLen+1];
        strncpy(m_sValue,pStrObj->GetValue(),iStrLen);
        m_sValue[iStrLen]=0;
      }
    }
  }
}

string TStringObject::ToString()
{
  ostringstream ostrOut;

  ostrOut << sObjectName << endl;
  ostrOut << "  " << "Value = [" << m_sValue << "]";

  return ostrOut.str();
}

int TStringObject::Compare(int iKey) const
{
  throw TException("TStringObject::Compare(int iKey) : Not Write Code.");
}

int TStringObject::Compare(int iKey1, int iKey2) const
{
  throw TException("TStringObject::Compare(int iKey1, int iKey2) : Not Write Code.");
}

int TStringObject::Compare(const TBaseObject *pObject) const
{
  if(TBaseObject::Assigned(pObject)==false||m_sValue==NULL)
    throw TException("TStringObject::Compare() : Object Is NULL");
  TStringObject *pStrObj = (TStringObject *)pObject;
  return strcmp(m_sValue,pStrObj->GetValue());
}

int TStringObject::CompareKey(const TBaseObject *pObject) const
{
  if(TBaseObject::Assigned(pObject)==false||m_sValue==NULL)
    throw TException("TStringObject::Compare() : Object Is NULL");
  TStringObject *pStrObj = (TStringObject *)pObject;
  return strcmp(m_sValue,pStrObj->GetValue());
}

bool TStringObject::CompareKeyByName(const TBaseObject *pObject) const
{
  if(TBaseObject::Assigned(pObject)==false||m_sValue==NULL)
    throw TException("TStringObject::CompareKeyByName() : Object Is NULL");
  TStringObject *pStrObj = (TStringObject *)pObject;
  return (strcmp(m_sValue,pStrObj->GetValue()) <0 )? true : false;
}

void TStringObject::ClearData(void)
{
  DELETE_A(m_sValue);
}

TPerfStatMgr::TPerfStatMgr()
{
    memset(m_PSContainer,0x00,sizeof(m_PSContainer));
    m_iCount=0;
    m_iNewCount = 0;
    m_lTotalMicroseconds = 0;
    //ZMP:305593
    //m_bSwitch = false;
    //ZMP:772252 change m_bSwitch = false -> true
    //ZMP:772252 �޸�������־Ĭ��ֵΪtrue
    m_bSwitch = true;

    /*begin zmp[841490]BF1.2 ����ͳ���ļ��Ż�*/
    snprintf(m_cBillLogPath, sizeof(m_cBillLogPath), "%s",getenv("BILL_LOG_PATH"));
    m_pFile=NULL;
    m_lLineNum=0;
    m_bInitTitle=false;/*�����г�ʼ����־��ture�ѳ�ʼ����falseδ��ʼ��*//*[873200]V8.1-����ͳ���ļ��У���������ͳ�Ʊ�ʶ*/
    TPerfStatMgr::GetProcessName();
/*  
  //helper �ⲻ���������� public ��
  if(getenv("BILL_LOG_PATH"))
  {
#ifdef WIN32
    //ZMP:467370 
    snprintf(m_PerfFileName, sizeof(m_PerfFileName), "%s\\PerfStat_%s_%d.txt",getenv("BILL_LOG_PATH"),g_progName,_getpid());
    m_PerfFileName[sizeof(m_PerfFileName)-1] = '\0';
#else
    snprintf(m_PerfFileName, sizeof(m_PerfFileName), "%s/PerfStat_%s_%d.txt",getenv("BILL_LOG_PATH"),g_progName,getpid());
#endif
  }
  else
  {
#ifdef WIN32
    snprintf(m_PerfFileName, sizeof(m_PerfFileName), "PerfStat_%s_%d.txt",g_progName,_getpid());
    m_PerfFileName[sizeof(m_PerfFileName)-1] = '\0';
#else
    snprintf(m_PerfFileName, sizeof(m_PerfFileName), "PerfStat_%s_%d.txt",g_progName,getpid());
#endif   
  }
  m_bInitPerfFileNameFlag = false;
*/
/*end zmp[841490]BF1.2 ����ͳ���ļ��Ż�*/
}

/*begin zmp[841490]BF1.2 ����ͳ���ļ��Ż�*/
TPerfStatMgr::~TPerfStatMgr()
{
    if( m_pFile != NULL )
    {
        fclose( m_pFile);
        m_pFile = NULL;
    }
}
/*end zmp[841490]BF1.2 ����ͳ���ļ��Ż�*/

TPerfStatMgr& TPerfStatMgr::Instance()
{
    static TPerfStatMgr instance;

    return instance;
}

void TPerfStatMgr::ShowStatic()
{
    if(IsSwitchOn())
    {
        for (int i=0;i<m_iCount;i++)
        {
            m_PSContainer[i]->Stat();
        }
    }
}

void TPerfStatMgr::AddPS(TPerformanceStat * thePS)
{
  if(m_iCount<MaxPerfStatePointNbr)//���֧��MaxPerfStatePointNbr������ͳ�Ƶ�
    m_PSContainer[m_iCount++]=thePS;
}

/*begin zmp[841490]BF1.2 ����ͳ���ļ��Ż�*/
void TPerfStatMgr::SetLogFileName(const char* filename)
{
    /*
    memset(m_PerfFileName,0x00,256); 
    char* pLogPath = getenv("BILL_LOG_PATH"); 
    if(pLogPath != NULL) 
    { 
        //ZMP:467370
        snprintf(m_PerfFileName, sizeof(m_PerfFileName), "%s%cPerfStat_%s_%d.txt", pLogPath, 
        PATH_DELIMITATED_CHAR, filename, getpid());
        m_PerfFileName[sizeof(m_PerfFileName)-1] = '\0';
    } 
    else 
    { 
        snprintf(m_PerfFileName, sizeof(m_PerfFileName), "PerfStat_%s_%d.txt",filename,getpid()); 
        m_PerfFileName[sizeof(m_PerfFileName)-1] = '\0';
    } 
    m_bInitPerfFileNameFlag = true;
    */
}
/*end zmp[841490]BF1.2 ����ͳ���ļ��Ż�*/

void TPerfStatMgr::SetSwitch(bool bSwitch)//������ͳ�ƿ��ؽ�������
{
    m_bSwitch = bSwitch;
}
bool TPerfStatMgr::IsSwitchOn()//�����жϵ�ǰ����ͳ���Ƿ���
{
    return m_bSwitch;
}

void TPerfStatMgr::Begin()
{
#ifdef WIN32
    m_beginPoint=clock();
#else
    gettimeofday(&m_beginPoint, NULL);        
#endif
}

void TPerfStatMgr::End()
{
    long iMicSec = 0;
#ifdef WIN32
    iMicSec =  (clock() - m_beginPoint);
#else
    gettimeofday(&m_endPoint, NULL);                
    iMicSec= (m_endPoint.tv_sec - m_beginPoint.tv_sec) * 1000000 + (m_endPoint.tv_usec - m_beginPoint.tv_usec);    
#endif
   ++m_iNewCount;
   m_lTotalMicroseconds += iMicSec;
}

void TPerfStatMgr::GetBusinessStatic(long iIntervalTime,  char *sBusinessStatic, unsigned int sBusinessStaticSize)
{
    if(0 >= iIntervalTime || 0 >= sBusinessStaticSize || NULL == sBusinessStatic)
    {
        return;
    }    
    
    sprintf(sBusinessStatic,"CPU_TIME_PCT=%d|CPU_TIME_TOTAL=%ld|NUM=%d|AVG_TIME=%ld|",
        (int)(m_lTotalMicroseconds*100/1000/iIntervalTime),//(int)(m_lTotalMicroseconds/1000/iIntervalTime*100), zmp:968766
        m_lTotalMicroseconds/1000,
        m_iNewCount,
        m_iNewCount == 0? 0:m_lTotalMicroseconds/1000/m_iNewCount);
    
    unsigned long nLen = strlen(sBusinessStatic)<sBusinessStaticSize?(strlen(sBusinessStatic)):(sBusinessStaticSize-1);
    sBusinessStatic[nLen] = 0;    

    ClearStat();    
    
}

void TPerfStatMgr::ClearStat()
{
    m_iNewCount = 0;
    m_lTotalMicroseconds = 0;
}

/*begin [873200]V8.1-����ͳ���ļ��У���������ͳ�Ʊ�ʶ*/
void TPerfStatMgr::GetProcessName()
{
    //if(strcmp(g_progName,"")!=0)
    //{
    //    return;
    //}
    //zmp:1412830 ֮ǰg_progName���ܱ���ֵ��������ʽ��main�������� argv[0] Ϊ�丳ֵ��
    //Ϊ��ȷ��g_progName��ֵ�Ǹɾ��Ľ������ƣ����ԭ���ĸ�ֵ�����������¸�ֵ
    memset(g_progName, 0, sizeof(g_progName));
    
    FILE* fp=NULL;
    char cmd[256]={0};
    char buf[256]={0};
    char *pTempProgName = NULL;
    
    snprintf(cmd,sizeof(cmd), "ps -e|grep %d|grep -v grep|grep -v awk|awk -F' ' '{print $4}'",getpid());
    if(NULL != (fp=popen(cmd,"r")))
    {
        while((fgets(buf,256,fp))!=NULL);  //���ո�FILE* fp����������ȡ��buf��
        pclose(fp);
        fp=NULL;
    }

    pTempProgName = strchr(buf,'\r');
    if (pTempProgName!=NULL)
    {
        *pTempProgName = '\0';  
    }

    pTempProgName = strchr(buf,'\n');
    if (pTempProgName!=NULL)
    {
        *pTempProgName = '\0';  
    }

    if(strcmp(g_progName,"")==0)
    {
        pTempProgName = strrchr(buf,'/');
        if (pTempProgName!=NULL)
        {
            memcpy(g_progName, pTempProgName+1,sizeof(g_progName));
        }
        else
        {
            memcpy(g_progName, buf,sizeof(g_progName));
        }
        g_progName[sizeof(g_progName)-1] = '\0';
    }
}
/*end [873200]V8.1-����ͳ���ļ��У���������ͳ�Ʊ�ʶ*/

TPerformanceStat::TPerformanceStat(const char *sFunName,int perCount):m_perCount(perCount)
{
  ClearStat();

  strncpy(m_functionName,sFunName,sizeof(m_functionName)-1);
  g_PerfStatMgr.AddPS(this);

  m_fOsValue = 1000000.0;
  theMicSec=0;
}

TPerformanceStat::~TPerformanceStat()
{
  Stat();
}

void TPerformanceStat::Stat()
{

  if(m_microseconds>0)
  {
    /*begin zmp[841490]BF1.2 ����ͳ���ļ��Ż�*/
    //const char * const theFormat1 ="%s Func:[%-30s] Total:[%8.5fs] Count:[%6d] Avg:[%6.2f]/ms 0~0.005s[%4d] 0.005s~0.01s[%4d] 0.01s~0.1s[%4d] 0.1s~1s[%4d] 1s~5s[%4d] 5s~10s[%3d] 10s~20s[%2d] >20s[%2d]\n";
    //const char * const theFormat2 ="%s Func:[%-30s] Total:[%8.5fs] Count:[%6d] 0~0.005s[%6d] 0.005s~0.01s[%6d] 0.01s~0.1s[%6d] 0.1s~1s[%5d] 1s~5s[%4d] 5s~10s[%3d] 10s~20s[%2d]  >20s[%2d]\n";
    const char * const theFormat1 ="%s,%s,%.5f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.2f\n";
    const char * const theFormat2 ="%s,%s,%.5f,%d,%d,%d,%d,%d,%d,%d,%d,%d\n";
    time_t tCurrent;
    struct tm *tm_Cur;
    char sCurtime[30]={0};
    time(&tCurrent); //ȡ�õ�ǰʱ���time_tֵ
    tm_Cur = localtime(&tCurrent); //ȡ�õ�ǰʱ���tmֵ
    snprintf(sCurtime, sizeof(sCurtime), "%04d%02d%02d %02d:%02d:%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday,tm_Cur->tm_hour,tm_Cur->tm_min,tm_Cur->tm_sec);

    if(g_PerfStatMgr.m_pFile==NULL)
    {
        char sCurDate[30]={0};
        char PerfFileName[256]={0};
        snprintf(sCurDate, sizeof(sCurDate), "%04d%02d%02d",tm_Cur->tm_year+1900,tm_Cur->tm_mon+1,tm_Cur->tm_mday);

        if(strcmp(g_PerfStatMgr.m_cBillLogPath,"")!=0)
        {
#ifdef WIN32
            //ZMP:467370 
            snprintf(PerfFileName, sizeof(PerfFileName), "%s\\PerfStat_%s_%d_%s.csv",g_PerfStatMgr.m_cBillLogPath,g_progName,_getpid(),sCurDate);
            PerfFileName[sizeof(PerfFileName)-1] = '\0';
#else
            snprintf(PerfFileName, sizeof(PerfFileName), "%s/PerfStat_%s_%d_%s.csv",g_PerfStatMgr.m_cBillLogPath,g_progName,getpid(),sCurDate);
#endif
        }
        else
        {
#ifdef WIN32
            snprintf(PerfFileName, sizeof(PerfFileName), "PerfStat_%s_%d_%s.csv",g_progName,_getpid(),sCurDate);
            PerfFileName[sizeof(PerfFileName)-1] = '\0';
#else
            snprintf(PerfFileName, sizeof(PerfFileName), "PerfStat_%s_%d_%s.csv",g_progName,getpid(),sCurDate);
#endif   
        }
        g_PerfStatMgr.m_pFile=fopen(PerfFileName, "a+");
        if (NULL == g_PerfStatMgr.m_pFile)
        {
            int iError = errno;
            printf("fopen(%s, %s) failed. errno : %d, reason : %s \n", PerfFileName, "a+", iError, strerror(iError));
        }
        /*�����г�ʼ����־��ture�ѳ�ʼ����falseδ��ʼ��*//*[873200]V8.1-����ͳ���ļ��У���������ͳ�Ʊ�ʶ*/
        if (g_PerfStatMgr.m_pFile != NULL &&  !g_PerfStatMgr.m_bInitTitle )
        {
            g_PerfStatMgr.m_bInitTitle=true;
            fprintf (g_PerfStatMgr.m_pFile,"Time,Function,ConsumeSec,Count,0~0.005s,0.005~0.01s,0.01s~0.1s,0.1s~1s,1s~5s,5s~10s,10s~20s,>20s\n"); 
        }
    }

    if (g_PerfStatMgr.m_pFile != NULL)
    {
        if((m_perCount>0)&&(m_microseconds>0)&&(m_counter % m_perCount==0))
        {
            fprintf (g_PerfStatMgr.m_pFile,theFormat1, sCurtime,m_functionName,(float)m_Totalmicseconds/m_fOsValue,m_counter,m_PerSecMap[0],m_PerSecMap[1],m_PerSecMap[2],m_PerSecMap[3],m_PerSecMap[4],m_PerSecMap[5],m_PerSecMap[6],m_PerSecMap[7],(float)(m_microseconds/m_perCount/1000)); 
        }
        else if (m_microseconds>0)
        {
            fprintf (g_PerfStatMgr.m_pFile,theFormat2, sCurtime,m_functionName,(float)m_Totalmicseconds/m_fOsValue,m_counter,m_SecMap[0],m_SecMap[1],m_SecMap[2],m_SecMap[3],m_SecMap[4],m_SecMap[5],m_SecMap[6],m_SecMap[7]);
        }

        fflush(g_PerfStatMgr.m_pFile);
        g_PerfStatMgr.m_lLineNum++;

        if(g_PerfStatMgr.m_lLineNum > MAX_PERF_LOG_LINE)
        {
            fclose(g_PerfStatMgr.m_pFile);
            g_PerfStatMgr.m_pFile = NULL;
            g_PerfStatMgr.m_lLineNum = 0;
            /*�����г�ʼ����־��ture�ѳ�ʼ����falseδ��ʼ��*//*[873200]V8.1-����ͳ���ļ��У���������ͳ�Ʊ�ʶ*/
            g_PerfStatMgr.m_bInitTitle=false;
        }
    }
    /*end zmp[841490]BF1.2 ����ͳ���ļ��Ż�*/

    ClearStat();
  }

}

void TPerformanceStat::ClearStat()
{
  m_microseconds = 0;
  m_Totalmicseconds = 0;
  m_SecMap[0]=0;
  m_SecMap[1]=0;
  m_SecMap[2]=0;
  m_SecMap[3]=0;
  m_SecMap[4]=0;
  m_SecMap[5]=0;
  m_SecMap[6]=0;
  m_SecMap[7]=0;

  m_PerSecMap[0]=0;
  m_PerSecMap[1]=0;
  m_PerSecMap[2]=0;
  m_PerSecMap[3]=0;
  m_PerSecMap[4]=0;
  m_PerSecMap[5]=0;
  m_PerSecMap[6]=0;
  m_PerSecMap[7]=0;

  m_counter = 0;

}

void TPerformanceStat::begin() 
{    
  theMicSec=0;
#ifdef WIN32
  m_beginPoint=clock();
#else
  gettimeofday(&m_beginPoint, NULL);        
#endif
}

void TPerformanceStat::end()
{
  static int theLevel=0;
#ifdef WIN32
  theMicSec =  (clock() - m_beginPoint);
#else
  gettimeofday(&m_endPoint, NULL);                
  theMicSec= (m_endPoint.tv_sec - m_beginPoint.tv_sec) * 1000000 + (m_endPoint.tv_usec - m_beginPoint.tv_usec);    
#endif
  m_microseconds +=theMicSec;
  theLevel=(int)(theMicSec / 1000);
  m_Totalmicseconds += theMicSec;
  if(theLevel<=5)//0.005s
  {
      m_SecMap[0]++;
      m_PerSecMap[0]++;
  }
  else if(theLevel<=10)//0.01s
  {
      m_SecMap[1]++;
      m_PerSecMap[1]++;
  }
  else if(theLevel<=100)//0.1s
  {
      m_SecMap[2]++;
      m_PerSecMap[2]++;
  }
  else if(theLevel<=1000)// 1s
  {
      m_SecMap[3]++;
      m_PerSecMap[3]++;
  }
  else if(theLevel<=5000)//5s
  {
      m_SecMap[4]++;
      m_PerSecMap[4]++;
  }
  else if(theLevel<=10000)//10s
  {
      m_SecMap[5]++;
      m_PerSecMap[5]++;
  }
  else if(theLevel<=20000)// 20s
  {
      m_SecMap[6]++;
      m_PerSecMap[6]++;
  }
  else 
  {
      m_SecMap[7]++;
      m_PerSecMap[7]++;
  }

  ++m_counter; 
  if(m_perCount>0)
    if(m_counter % m_perCount ==0 )
    {
      Stat();

    }
}


#ifdef __OS_WINDOWS__

void kill(int pid,int code)
{
  HANDLE process=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_TERMINATE|SYNCHRONIZE,FALSE,pid);
  TerminateProcess(process,code);
}
int waitpid(int pid, int *status, int options)
{
  HANDLE process=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_TERMINATE|SYNCHRONIZE,FALSE,pid);
  return WaitForSingleObject(process,INFINITE);
}

bool IsProcessExists(int pid)
{
  HANDLE process=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_TERMINATE|SYNCHRONIZE,FALSE,pid);
  unsigned long iExitCode;
  GetExitCodeProcess(process,&iExitCode);
  return iExitCode == STILL_ACTIVE;
}

#endif
