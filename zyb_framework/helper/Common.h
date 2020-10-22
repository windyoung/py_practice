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

//--------------------------类定义-------------------------------
//////////////////////////////////////////////////////////////////////////
//
// TBaseObject 定义
//
//////////////////////////////////////////////////////////////////////////
class TBaseObject {
public:
  TBaseObject();//构造函数   
  virtual ~TBaseObject(); // 析构函数
  //const char *GetObjectName() const;//取对象名称    
  //void SetObjectName(const char *sName);
  virtual void Assign(TBaseObject *pObject);//拷贝对象
  virtual string ToString();//输出对象信息
  virtual char * ToStringEx();
  virtual int Compare(int iKey) const;
  virtual int Compare(int iKey1, int iKey2) const;
  virtual int Compare(const TBaseObject *pObject) const;
  virtual int CompareKey(const TBaseObject *pObject) const;
  virtual bool CompareKeyByName(const TBaseObject *pObject) const;
  static bool Assigned(const TBaseObject *pObject);
protected:
  int Tag;
  //char sObjectName[OBJECT_NAME_SIZE];//对象名称
};

//////////////////////////////////////////////////////////////////////////
//
// TRecordData 定义
//
//////////////////////////////////////////////////////////////////////////

class TRecordData : public TBaseObject {
public:
  int RecordType;//来至RECORD_TYPE
  char sObjectName[OBJECT_NAME_SIZE];//对象名称
public:
  TRecordData();
  virtual ~TRecordData();
  virtual void ClearData(void);
  virtual bool SetData(const int iDataCode,const char *sDataValue,const char *sDataPath=NULL);
  virtual bool SetDataString(const char *sDataString);
  int GetCurrPath(const char *sDataPath);
  int GetCurrPathCount(const char *sDataPath);
  const char * GetNextPath(const char *sDataPath);
  const char *GetObjectName() const;//取对象名称    
  void SetObjectName(const char *sName);
};


//////////////////////////////////////////////////////////////////////////
//
// TLLong 定义
//
//////////////////////////////////////////////////////////////////////////
class TLongObject : public TBaseObject {
private:
  llong m_iValue;
  char sObjectName[OBJECT_NAME_SIZE];//对象名称
public:
  TLongObject();
  TLongObject(llong iValue);
  TLongObject(const TLongObject& right);
  virtual ~TLongObject();
public:
  void SetValue(llong iValue);
  llong GetValue() const;
  const char *GetObjectName() const;//取对象名称    
  void SetObjectName(const char *sName);
public:
  virtual void Assign(TBaseObject *pObject);//拷贝对象
  virtual string ToString();//输出对象信息
  virtual int Compare(int iKey) const;
  virtual int Compare(int iKey1, int iKey2) const;  
  virtual int Compare(const TBaseObject *pObject) const;
  virtual int CompareKey(const TBaseObject *pObject) const;
  virtual bool CompareKeyByName(const TBaseObject *pObject) const;
};

//////////////////////////////////////////////////////////////////////////
//
// TStringObject 定义
//
//////////////////////////////////////////////////////////////////////////
class TStringObject : public TBaseObject {
private:
  char *m_sValue;
  char sObjectName[OBJECT_NAME_SIZE];//对象名称
public:
  TStringObject();
  TStringObject(const char *sStringValue);
  TStringObject(const TStringObject& right);
  virtual ~TStringObject();
  const char *GetObjectName() const;//取对象名称    
  void SetObjectName(const char *sName);
public:
  void SetValue(const char *sStringValue);
  const char * GetValue() const;
public:
  virtual void Assign(TBaseObject *pObject);//拷贝对象
  virtual string ToString();//输出对象信息
  virtual int Compare(int iKey) const;
  virtual int Compare(int iKey1, int iKey2) const;  
  virtual int Compare(const TBaseObject *pObject) const;
  virtual int CompareKey(const TBaseObject *pObject) const;
  virtual bool CompareKeyByName(const TBaseObject *pObject) const;
  virtual void ClearData(void);
};

//////////////////////////////////////////////////////////////////////////
// 程序执行时间统计类
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
  //设置统计文件名
  void SetLogFileName(const char* filename);
    void AddPS(TPerformanceStat * thePS);

    void SetSwitch(bool bSwitch);//对性能统计开关进行设置
    bool IsSwitchOn();//用于判断当前性能统计是否开启
public:
    void Begin();
    void End();
   /**
    * @brief 获取性能信息放到sBusinessStatic中
    *              
    * @param iIntervalTime  [in] 时间间隔，单位毫秒
    * @param sBusinessStatic[char*]存放性能信息
    * @param sBusinessStaticSize [in] sBusinessStatic数组大小
    * @return void
    * @retval 
    */
    void GetBusinessStatic(long iIntervalTime, char *sBusinessStatic, unsigned int sBusinessStaticSize);
    /*[873200]V8.1-性能统计文件中，出现两行统计标识*/
    static void GetProcessName();
private:
    void ClearStat();
public:	
    /*begin zmp[841490]BF1.2 性能统计文件优化*/
    //char m_PerfFileName[256];	
    //bool m_bInitPerfFileNameFlag;//m_PerfFileName初始化标志，ture已初始化，false未初始化
    char m_cBillLogPath[256];
    FILE *m_pFile;
    long m_lLineNum;
    bool m_bInitTitle;/*标题行初始化标志，ture已初始化，false未初始化*//*[873200]V8.1-性能统计文件中，出现两行统计标识*/
    /*end zmp[841490]BF1.2 性能统计文件优化*/
private:
    TPerformanceStat * m_PSContainer[MaxPerfStatePointNbr]; //最多支持512个性能统计点
    int m_iCount;
    bool m_bSwitch;//性能统计开关，默认为false
private:
    int m_iNewCount;//单位时间内处理的次数
    long m_lTotalMicroseconds;//单位时间间隔内花费的时间(单位:微秒)
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

//取日志文件句柄
//char *GetBillLogHandle();

#endif
