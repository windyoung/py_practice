#include <string>
#include <stdio.h>
#include "unistd.h"

#include "TMQInterface.h"
#include "DefaultMQPushConsumer.h"
#include "TZSmartMQConsumer.h"
#include "TZSmartMQMsgListener.h"

using namespace std;


TZSmartMQConsumer::TZSmartMQConsumer()
{
}

TZSmartMQConsumer::~TZSmartMQConsumer()
{
    m_pConsumer->shutdown();
    if(m_pConsumer)
    {
        delete m_pConsumer;
        m_pConsumer = NULL;
    }
    if(m_pMQMsgListener)
    {
        delete m_pMQMsgListener;
        m_pMQMsgListener = NULL;
    }  
}

bool TZSmartMQConsumer::Initialize(
        const std::string& sNameServerAddr,
        const std::string& sGroup,
        const std::string& sTopic)
{
    m_sNameServerAddr = sNameServerAddr;
    m_sGroup = sGroup;
    m_sTopic = sTopic;

    m_pConsumer = new DefaultMQPushConsumer(m_sGroup);

    pid_t pid = getpid();
    char sPid[20]={0};
    snprintf(sPid,20,"%d",pid);
    string strPid = sPid;
    m_pConsumer->setInstanceName(strPid);

    m_pConsumer->setNamesrvAddr(m_sNameServerAddr);
    m_pConsumer->setConsumeMessageBatchMaxSize(1);
    m_pConsumer->setConsumeThreadMin(1);
    m_pConsumer->setConsumeThreadMax(1);
    m_pConsumer->setPullBatchSize(1);
    m_pConsumer->subscribe(m_sTopic, "*"); //consume all tag message under the topic

    return true;
}

//void TZSmartMQConsumer::Run(std::string sKey, const char* sMsg, std::size_t iMsgLen)
void TZSmartMQConsumer::Run(ConsumerFunc pFunc)
{
    //m_pMQMsgListener = make_shared<TZSmartMQMsgListener>(sKey,sMsg,iMsgLen);
    m_pMQMsgListener = new TZSmartMQMsgListener(pFunc);
    m_pConsumer->registerMessageListener(m_pMQMsgListener);

    m_pConsumer->start();
}

void TZSmartMQConsumer::Shutdown()
{
    m_pConsumer->shutdown();
}
