/*
 * TZSmartMQProducer.cpp
 *
 *  Created on: 2019Äê7ÔÂ22ÈÕ
 *      Author: 0027009397
 */

#include <iostream>
#include <sstream>
#include <string>
#include <memory>

#include "TMQInterface.h"
#include "Message.h"
#include "TZSmartMQProducer.h"
#include "DefaultMQProducer.h"
#include "MessageQueueSelector.h"

using namespace std;

TZSmartMQProducer::TZSmartMQProducer():m_pProducer(NULL),m_pMessage(NULL)
{
    m_pMessageQueueByHash = NULL;
}

TZSmartMQProducer::~TZSmartMQProducer()
{
    m_pProducer->shutdown();
    delete m_pProducer;
    m_pProducer=NULL;

    delete m_pMessage;
    m_pMessage=NULL;

    delete m_pMessageQueueByHash;
    m_pMessageQueueByHash = NULL;
}

bool TZSmartMQProducer::Initialize(
        const std::string& sNameServerAddr,
        const std::string& sGroup,
        const std::string& sTopic)
{
    m_sNameServerAddr = sNameServerAddr;
    m_sGroup = sGroup;
    m_sTopic = sTopic;

    m_pProducer = new DefaultMQProducer(m_sGroup);
    m_pMessage = new Message();
    m_pMessage->setTopic(m_sTopic);

    m_pProducer->setNamesrvAddr(m_sNameServerAddr);
    m_pProducer->start();

    m_pMessageQueueByHash = new SelectMessageQueueByHash();
    return true;
}

void TZSmartMQProducer::SetProperty(const std::string& name, const std::string& value)
{
    m_pMessage->putProperty(name,value);
}

void TZSmartMQProducer::SetTopic(const string& sTopic)
{
    m_pMessage->setTopic(sTopic);
}

bool TZSmartMQProducer::Send(const char* sMsg, size_t iMsgSize, void* arg)
{
    m_pMessage->setBody(sMsg, static_cast<int>(iMsgSize));

    SendResult result;
    if(arg != NULL)
    {
        result = m_pProducer->send(*m_pMessage, m_pMessageQueueByHash, arg);

    }
    else
    {
        result = m_pProducer->send(*m_pMessage);
    }

    if (result.getSendStatus() != 0)
    {
        ostringstream oss;
        oss << "Send to ZSmartMQ failed: " << endl
                << "SendStatus = " <<  result.getSendStatus() << endl
                << "MsgId      = " << result.getMsgId() << endl
                << "QueueId    = " << result.getMessageQueue().getQueueId() << endl
                << "Topic      = " << m_pMessage->getTopic() << endl
                << "MsgKeys    = " << m_pMessage->getKeys() << endl;
        m_sErrMsg = oss.str();
        ClearProperties();
        return false;
    }
    m_sMsgId = result.getMsgId();
    ClearProperties();
    return true;
}

bool TZSmartMQProducer::Send(const std::string& sKeys, const char* sMsg, size_t iMsgSize, void* arg)
{
    m_pMessage->setKeys(sKeys);
    return Send(sMsg, iMsgSize);
}

const std::string& TZSmartMQProducer::GetErrMsg()
{
    return m_sErrMsg;
}

const std::string& TZSmartMQProducer::GetMsgId()
{
    return m_sMsgId;
}

void TZSmartMQProducer::Shutdown()
{
    m_pProducer->shutdown();
}

void TZSmartMQProducer::ClearProperties()
{
    m_pMessage->getProperties().clear();
}



