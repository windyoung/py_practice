#include <list>
#include <iostream>
#include "TZSmartMQMsgListener.h"
#include "TMQMsgBufferMgr.h"

using namespace std;

//TZSmartMQMsgListener::TZSmartMQMsgListener(std::string sKey, const char* sMsg, std::size_t iMsgLe)
//{
//}
TZSmartMQMsgListener::TZSmartMQMsgListener(ConsumerFunc pFunc)
: m_pFunc(pFunc)
{
}

ConsumeConcurrentlyStatus TZSmartMQMsgListener::consumeMessage(
        std::list<MessageExt*>& listMsg,
        ConsumeConcurrentlyContext& context)
{
    if (listMsg.size() != 1)
    {
        cout << "Error: Get " << listMsg.size() << " messages once !!! It's Must be 1!!!" << endl;
    }

    MessageExt* &msg = listMsg.front();
    TMQMsgBufferMgr::Instance().Reset();
    std::map<std::string, std::string> &mPropertys=msg->getProperties();
    string str;
    const string HeadKey("Pinpoint");
    for(std::map<std::string, std::string>::iterator it=mPropertys.begin();it != mPropertys.end(); ++it)
    {
        if(it->first.substr(0,HeadKey.length())!=HeadKey)
        {
            continue;
        }
        str.append(it->first.c_str());
        str.append("=");
        str.append(it->second.c_str());
        str.append(",");
    }
    cout << "Str: " << str.c_str()<<endl;
    TMQMsgBufferMgr::Instance().SetProperties(str);
    //cout << "Keys: " << msg->getKeys() << "sg->getBody():"<< msg->getBody()<<"getBodyLen():"<<msg->getBodyLen()<< endl;
    if (m_pFunc(msg->getKeys(), msg->getBody(), static_cast<size_t>(msg->getBodyLen())))
    {
        return CONSUME_SUCCESS;
    }
    else
    {
        return RECONSUME_LATER;
    }
}
