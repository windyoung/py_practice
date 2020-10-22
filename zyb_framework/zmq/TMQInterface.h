/*
 * TMQInterface.h
 *
 *  Created on: 2019Äê7ÔÂ22ÈÕ
 *      Author: 0027009397
 */

#ifndef CODE_FRAMEWORK_ZMQ_TMQINTERFACE_H_
#define CODE_FRAMEWORK_ZMQ_TMQINTERFACE_H_

#include <string>
#include <functional>

namespace MQ
{
    const std::string DEF_MQ="ZSMART";
    const std::string ZSMART_MQ="ZSMART";
}


class TMQProducerInterface
{
public:
    TMQProducerInterface(){};
    virtual ~TMQProducerInterface(){};

    virtual bool Initialize(
            const std::string& sNameServerAddr,
            const std::string& sGroup,
            const std::string& sTopic){return false;}
    virtual void SetTopic(const std::string& sTopic){}
    virtual void SetProperty(const std::string& name, const std::string& value){}
    virtual bool Send(const char* sMsg, size_t iMsgSize, void* arg = NULL){ return false; }
    virtual bool Send(const std::string& sKeys, const char* sMsg, size_t iMsgSize, void* arg = NULL){ return false; }
    virtual const std::string& GetErrMsg() = 0;
    virtual const std::string& GetMsgId() = 0;
    virtual void Shutdown(){}

protected:
    std::string m_sGroup;
    std::string m_sNameServerAddr;
    std::string m_sTopic;

    std::string m_sErrMsg;
    std::string m_sMsgId;
};
class TMQConsumerInterface
{
public:
    //typedef std::function<bool(std::string sKey, const char* sMsg, std::size_t iMsgLen)> ConsumerFunc;
    typedef bool(*ConsumerFunc)(std::string sKey, const char* sMsg, std::size_t iMsgLen);
    TMQConsumerInterface(){};
    virtual ~TMQConsumerInterface(){};

    virtual bool Initialize(
            const std::string& sNameServerAddr,
            const std::string& sGroup,
            const std::string& sTopic){ return false;}
    virtual void Run(ConsumerFunc pFunc){}
    virtual void Shutdown(){}

protected:
    std::string m_sGroup;
    std::string m_sNameServerAddr;
    std::string m_sTopic;
};

#endif /* CODE_FRAMEWORK_ZMQ_TMQINTERFACE_H_ */
