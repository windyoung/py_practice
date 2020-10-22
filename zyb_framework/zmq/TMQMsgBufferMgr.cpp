#include "TMQMsgBufferMgr.h"
#include<stdio.h>


using namespace std;

namespace
{
const string MSG_HEAD_FLAG = "MESSAGE:\n";
}

TMQMsgBufferMgr& TMQMsgBufferMgr::Instance()
{
    static TMQMsgBufferMgr inst;
    return inst;
}

void TMQMsgBufferMgr::SetProperties(const std::string & sProperties)
{
    if(sProperties.size() > 0)
    {
        m_strProperties = "__PROPERTIES__:";
        m_strProperties.append(sProperties);
        m_strProperties.append("\n");
    }
}

void TMQMsgBufferMgr::SetMsg(const std::string & sTopic, const std::string& sKey,  const std::string& sMsg, int iDelayLevel)
{
    printf("TMsgBufferMgr::SetMsg() sTopic=[%s] sKey=[%s] sMsg=[%s]\n", sTopic.c_str(), sKey.c_str(), sMsg.c_str());
    if(m_strProperties.size() > 0)
    {
        m_sBuffer.append(m_strProperties);
    }
    char sDelayLevel[20] = {0};
    snprintf(sDelayLevel,20,"%d",iDelayLevel);
    if(iDelayLevel > 0)
    {
        m_sBuffer.append(sTopic).append(":").append(sKey).append(":").append(sDelayLevel).append(":").append(sMsg).append("\n");
    }
    else
    {
        m_sBuffer.append(sTopic).append(":").append(sKey).append(":").append(sMsg).append("\n");
    }
}

const string& TMQMsgBufferMgr::GetMsg()
{
    return m_sBuffer;
}

void TMQMsgBufferMgr::Reset()
{
    m_strProperties.clear();
    m_sBuffer = MSG_HEAD_FLAG;
}

bool TMQMsgBufferMgr::IsEmpty() const
{
    return m_sBuffer == MSG_HEAD_FLAG;
}

TMQMsgBufferMgr::TMQMsgBufferMgr()
{
    Reset();
}

TMQMsgBufferMgr::~TMQMsgBufferMgr()
{
}
