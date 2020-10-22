#ifndef MQ_MGR_T_MQ_MSG_LISTENER_H_
#define MQ_MGR_T_MQ_MSG_LISTENER_H_

#include "MessageListener.h"
#include <cwchar>
#include <functional>
#include <string>

class TZSmartMQMsgListener : public MessageListenerConcurrently
{
public:
    typedef bool(*ConsumerFunc)(std::string sKey, const char* sMsg, std::size_t iMsgLen);
public:
    TZSmartMQMsgListener(ConsumerFunc pFunc);

    virtual ConsumeConcurrentlyStatus consumeMessage(
            std::list<MessageExt*>& listMsg,
            ConsumeConcurrentlyContext& context);
private:
    ConsumerFunc m_pFunc;
};

#endif
