/*
 * TZSmartMQProducer.h
 *
 *  Created on: 2019Äê7ÔÂ22ÈÕ
 *      Author: 0027009397
 */

#ifndef CODE_FRAMEWORK_ZMQ_TZSMARTMQPRODUCER_H_
#define CODE_FRAMEWORK_ZMQ_TZSMARTMQPRODUCER_H_

#include <string>

class DefaultMQProducer;
class Message;

class TMQProducerInterface;
class MessageQueueSelector;

class TZSmartMQProducer: public TMQProducerInterface
{
public:
    TZSmartMQProducer();
    virtual ~TZSmartMQProducer();

    bool Initialize(
            const std::string& sNameServerAddr,
            const std::string& sGroup,
            const std::string& sTopic);
    void SetTopic(const std::string& sTopic);

    void SetProperty(const std::string& name, const std::string& value);
    void ClearProperties();
    bool Send(const char* sMsg, size_t iMsgSize, void* arg = NULL);
    bool Send(const std::string& sKeys, const char* sMsg, size_t iMsgSize, void* arg = NULL);
    const std::string& GetErrMsg();
    const std::string& GetMsgId();
    void Shutdown();
private:
    DefaultMQProducer *m_pProducer;
    MessageQueueSelector* m_pMessageQueueByHash;
    Message *m_pMessage;
};


#endif /* CODE_FRAMEWORK_ZMQ_TZSMARTMQPRODUCER_H_ */
