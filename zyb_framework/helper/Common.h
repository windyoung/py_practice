//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#ifndef _COMMON_H_
#define _COMMON_H_

#include "CommonDef.h"


#ifdef DELETE
#undef DELETE
#endif
#define DELETE(P)\
  do{ \
  if(NULL != (P))\
        {\
        delete (P);\
        (P) = NULL;\
        }\
  }while(0)

#ifdef DELETE_A
#undef DELETE_A
#endif
#define DELETE_A(P)\
  do{ \
  if(NULL != (P))\
        {\
        delete [] (P);\
        (P) = NULL;\
        }\
  }while(0)

//--------------------------�ඨ��-------------------------------
//////////////////////////////////////////////////////////////////////////
//
// TBaseObject ����
//
//////////////////////////////////////////////////////////////////////////
class TBaseObject {
public:
  TBaseObject();//���캯��   
  virtual ~TBaseObject(); // ��������
  //const char *GetObjectName() const;//ȡ��������    
  //void SetObjectName(const char *sName);
  virtual void Assign(TBaseObject *pObject);//��������
  virtual string ToString();//���������Ϣ
  virtual char * ToStringEx();
  virtual int Compare(int iKey) const;
  virtual int Compare(int iKey1, int iKey2) const;
  virtual int Compare(const TBaseObject *pObject) const;
  virtual int CompareKey(const TBaseObject *pObject) const;
  virtual bool CompareKeyByName(const TBaseObject *pObject) const;
  static bool Assigned(const TBaseObject *pObject);
protected:
  int Tag;
  //char sObjectName[OBJECT_NAME_SIZE];//��������
};

//////////////////////////////////////////////////////////////////////////
//
// TRecordData ����
//
//////////////////////////////////////////////////////////////////////////

class TRecordData : public TBaseObject {
public:
  int RecordType;//����RECORD_TYPE
  char sObjectName[OBJECT_NAME_SIZE];//��������
public:
  TRecordData();
  virtual ~TRecordData();
  virtual void ClearData(void);
  virtual bool SetData(const int iDataCode,const char *sDataValue,const char *sDataPath=NULL);
  virtual bool SetDataString(const char *sDataString);
  int GetCurrPath(const char *sDataPath);
  int GetCurrPathCount(const char *sDataPath);
  const char * GetNextPath(const char *sDataPath);
  const char *GetObjectName() const;//ȡ��������    
  void SetObjectName(const char *sName);
};


//////////////////////////////////////////////////////////////////////////
//
// TLLong ����
//
//////////////////////////////////////////////////////////////////////////
class TLongObject : public TBaseObject {
private:
  llong m_iValue;
  char sObjectName[OBJECT_NAME_SIZE];//��������
public:
  TLongObject();
  TLongObject(llong iValue);
  TLongObject(const TLongObject& right);
  virtual ~TLongObject();
public:
  void SetValue(llong iValue);
  llong GetValue() const;
  const char *GetObjectName() const;//ȡ��������    
  void SetObjectName(const char *sName);
public:
  virtual void Assign(TBaseObject *pObject);//��������
  virtual string ToString();//���������Ϣ
  virtual int Compare(int iKey) const;
  virtual int Compare(int iKey1, int iKey2) const;  
  virtual int Compare(const TBaseObject *pObject) const;
  virtual int CompareKey(const TBaseObject *pObject) const;
  virtual bool CompareKeyByName(const TBaseObject *pObject) const;
};

//////////////////////////////////////////////////////////////////////////
//
// TStringObject ����
//
//////////////////////////////////////////////////////////////////////////
class TStringObject : public TBaseObject {
private:
  char *m_sValue;
  char sObjectName[OBJECT_NAME_SIZE];//��������
public:
  TStringObject();
  TStringObject(const char *sStringValue);
  TStringObject(const TStringObject& right);
  virtual ~TStringObject();
  const char *GetObjectName() const;//ȡ��������    
  void SetObjectName(const char *sName);
public:
  void SetValue(const char *sStringValue);
  const char * GetValue() const;
public:
  virtual void Assign(TBaseObject *pObject);//��������
  virtual string ToString();//���������Ϣ
  virtual int Compare(int iKey) const;
  virtual int Compare(int iKey1, int iKey2) const;  
  virtual int Compare(const TBaseObject *pObject) const;
  virtual int CompareKey(const TBaseObject *pObject) const;
  virtual bool CompareKeyByName(const TBaseObject *pObject) const;
  virtual void ClearData(void);
};

//////////////////////////////////////////////////////////////////////////
// ����ִ��ʱ��ͳ����
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

class TPerformanceStat {
public:
  TPerformanceStat(const char *sFunName,int perCount = 0);
  virtual ~TPerformanceStat();
  void Stat();
  void ClearStat();
  void begin();
  void end();
private:
  long m_microseconds;
  int m_SecMap[8];
  int m_PerSecMap[8];
  int m_counter;
  int m_perCount;
  long m_Totalmicseconds;

#ifdef WIN32
  clock_t m_beginPoint; 
#else
  struct timeval m_beginPoint,m_endPoint;
#endif  
  float m_fOsValue;
public:
  long theMicSec;
  char m_functionName[64];

};
#define g_PerfStatMgr TPerfStatMgr::Instance()

#define PS_BEGIN(name,parent) static TPerformanceStat PS##name( (#name)); { if(g_PerfStatMgr.IsSwitchOn()){ PS##name.begin(); } }
#define PS_END(name) { if(g_PerfStatMgr.IsSwitchOn()){ PS##name.end(); } }
#define PS_START(name) static TPerformanceStat PS##name(#name); { if(g_PerfStatMgr.IsSwitchOn()){ PS##name.begin(); } }
#define PS_START1(name,icount) static TPerformanceStat PS##name(#name,icount); { if(g_PerfStatMgr.IsSwitchOn()){ PS##name.begin(); } }


class TPerfStatMgr
{
private:
    TPerfStatMgr();
public:
    ~TPerfStatMgr();
    static TPerfStatMgr& Instance();
    void ShowStatic();
  //����ͳ���ļ���
  void SetLogFileName(const char* filename);
    void AddPS(TPerformanceStat * thePS);

    void SetSwitch(bool bSwitch);//������ͳ�ƿ��ؽ�������
    bool IsSwitchOn();//�����жϵ�ǰ����ͳ���Ƿ���
public:
    void Begin();
    void End();
   /**
    * @brief ��ȡ������Ϣ�ŵ�sBusinessStatic��
    *              
    * @param iIntervalTime  [in] ʱ��������λ����
    * @param sBusinessStatic[char*]���������Ϣ
    * @param sBusinessStaticSize [in] sBusinessStatic�����С
    * @return void
    * @retval 
    */
    void GetBusinessStatic(long iIntervalTime, char *sBusinessStatic, unsigned int sBusinessStaticSize);
    /*[873200]V8.1-����ͳ���ļ��У���������ͳ�Ʊ�ʶ*/
    static void GetProcessName();
private:
    void ClearStat();
public:	
    /*begin zmp[841490]BF1.2 ����ͳ���ļ��Ż�*/
    //char m_PerfFileName[256];	
    //bool m_bInitPerfFileNameFlag;//m_PerfFileName��ʼ����־��ture�ѳ�ʼ����falseδ��ʼ��
    char m_cBillLogPath[256];
    FILE *m_pFile;
    long m_lLineNum;
    bool m_bInitTitle;/*�����г�ʼ����־��ture�ѳ�ʼ����falseδ��ʼ��*//*[873200]V8.1-����ͳ���ļ��У���������ͳ�Ʊ�ʶ*/
    /*end zmp[841490]BF1.2 ����ͳ���ļ��Ż�*/
private:
    TPerformanceStat * m_PSContainer[MaxPerfStatePointNbr]; //���֧��512������ͳ�Ƶ�
    int m_iCount;
    bool m_bSwitch;//����ͳ�ƿ��أ�Ĭ��Ϊfalse
private:
    int m_iNewCount;//��λʱ���ڴ���Ĵ���
    long m_lTotalMicroseconds;//��λʱ�����ڻ��ѵ�ʱ��(��λ:΢��)
    #ifdef WIN32
    clock_t m_beginPoint; 
    #else
    struct timeval m_beginPoint,m_endPoint;
    #endif  
};



#ifdef __OS_WINDOWS__
void kill(int pid,int code);
int waitpid(int pid, int *status, int options);
bool IsProcessExists(int pid);
#endif

//ȡ��־�ļ����
//char *GetBillLogHandle();

#endif
