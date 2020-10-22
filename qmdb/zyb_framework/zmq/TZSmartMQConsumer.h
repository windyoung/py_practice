#ifndef T_MQ_CONSUMER_H_
#define T_MQ_CONSUMER_H_

#include <string>
#include <memory>
#include <functional>
#include "TMQInterface.h"
class DefaultMQPushConsumer;
class TZSmartMQMsgListener;
//class TMQConsumerInterface;

class TZSmartMQConsumer : public TMQConsumerInterface
{
public:
    TZSmartMQConsumer();
    virtual ~TZSmartMQConsumer();

    bool Initialize(
            const std::string& sNameServerAddr,
            const std::string& sGroup,
            const std::string& sTopic);
    //void Run(std::string sKey, const char* sMsg, std::size_t iMsgLen);
    void Run(ConsumerFunc pFunc);
    void Shutdown();
private:
    DefaultMQPushConsumer *m_pConsumer;
    TZSmartMQMsgListener *m_pMQMsgListener;
};

#endif //T_MQ_CONSUMER_H_
