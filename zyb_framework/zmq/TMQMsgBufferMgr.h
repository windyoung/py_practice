#ifndef T_MSG_BUFFER_MGR_H_
#define T_MSG_BUFFER_MGR_H_

#include <iostream>
#include <string>

class TMQMsgBufferMgr
{
public:
    static TMQMsgBufferMgr& Instance();

    TMQMsgBufferMgr(const TMQMsgBufferMgr&);
    TMQMsgBufferMgr& operator=(const TMQMsgBufferMgr&);
    void SetProperties(const std::string & sProperties);
    //iDelayLevel 消息延迟等级
    //level取�?1-18  : "1s 5s 10s 30s 1m 2m 3m 4m 5m 6m 7m 8m 9m 10m 20m 30m 1h 2h"
    void SetMsg(const std::string & sTopic, const std::string& sKey,  const std::string& sMsg, int iDelayLevel = -1);
    const std::string& GetMsg();
    void Reset();
    bool IsEmpty() const;

private:
    TMQMsgBufferMgr();
    ~TMQMsgBufferMgr();

private:
    std::string m_sBuffer;
    std::string m_strProperties;
};

#endif
