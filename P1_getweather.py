# coding=utf-8
#!/usr/bin/env python3
'''
@Author: Zhu Jian
@Date: 2020-04-11 23:25:35
@LastEditTime: 2020-04-20 16:17:26
@LastEditors: Zhu Jian
@Description: 
@FilePath: \py_practice\getweather.py
@
'''

import requests


def get_weather():
    citypage = r'http://www.weather.com.cn/data/sk/101190101.html'
    r = requests.get(citypage)
    r.encoding = 'utf-8'
    # wea_json = r.content.decode('utf-8')
    wea_json = r.json()
    # print(wea_json)  #
    # print(str(wea_json['weatherinfo']['city']))
    wea_string = 'zhujian: city:{};temperature:{};wind:{},{};humidity:{};;updatetime:{}'.format(
        wea_json['weatherinfo']['city'], wea_json['weatherinfo']['temp'],
        wea_json['weatherinfo']['WD'], wea_json['weatherinfo']['WS'],
        wea_json['weatherinfo']['SD'], wea_json['weatherinfo']['time'])
    print(wea_string)
    return wea_string


def send_dingdingmsg(dd_webhook_TOKEN,
                     dd_webhook_SECRET,
                     msg,
                     atall=False,
                     **at_person_list):

    TOKEN = dd_webhook_TOKEN
    SECRET = dd_webhook_SECRET
    HEADERS = {"Content-Type": "application/json;charset=utf-8"}
    # 钉钉官方要求，请求的url中必须携带三个参数，access_token， timestamp，sign(签名是由secret加密而来)
    timestamp = round(time.time() * 1000)
    secret_enc = SECRET.encode('utf-8')
    string_to_sign = '{}\n{}'.format(timestamp, SECRET)
    string_to_sign_enc = string_to_sign.encode('utf-8')
    hmac_code = hmac.new(secret_enc,
                         string_to_sign_enc,
                         digestmod=hashlib.sha256).digest()
    sign = urllib.parse.quote_plus(base64.b64encode(hmac_code))
    # 完整的url
    api_url = "https://oapi.dingtalk.com/robot/send?access_token={}&timestamp={}&sign={}".format(
        TOKEN, timestamp, sign)
    # 发送的消息类型可查看文档，此处只做测试

    String_Msg = {
        "msgtype": "text",
        "text": {
            "content": msg
        },
        "at": {
            "isAtAll": atall
        }
    }
    String_textMsg = json.dumps(String_Msg).encode('utf-8')
    res = requests.post(api_url, data=String_textMsg, headers=HEADERS)
    print(res.text)


if __name__ == '__main__':
    dd_webhook_TOKEN = '5e3daeff1f18aa6d55d7b9f7d29fa66f18502d24e8d9a097ab3c19624d720b8c'
    dd_webhook_SECRET = 'SECe436d190bead54753076a089572ea574c14203bb1af33e20645e0d8439b9f15f'
    # https://oapi.dingtalk.com/robot/send?access_token=e0e03f895c12957484cf4cb2e3224d89ceb1d832f5dc745b3a8ea80b2618aa02
    atpersonlist = ['18260414208', '18260414268']
    atpersonlist = ['18963640027']

    # get_weather()
    send_dingdingmsg(dd_webhook_TOKEN,
                     dd_webhook_SECRET,
                     get_weather(),
                     at_person_list=atpersonlist)
