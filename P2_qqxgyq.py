# coding=utf-8
#!/usr/bin/env python3
'''
@Author: Zhu Jian
@Date: 2020-04-13 14:57:54
@LastEditTime: 2020-04-20 16:17:38
@LastEditors: Zhu Jian
@Description: 
@FilePath: \py_practice\qqxgyq.py
@
'''

import urllib
import base64
import time
import bs4
import lxml
import pandas
import requests
import json


def get_all_data(url):
    proxies = {
        "http": "http://127.0.0.1:10809",
        'socks5': 'socks5://127.0.0.1:10808'
    }  # 代理ip
    res = requests.get(url, proxies=proxies)
    res_ele = lxml.etree.HTML(res.text)
    table = res_ele.xpath('//table[@id="main_table_countries_today"]')
    table = lxml.etree.tostring(table[0], encoding='utf-8').decode()

    # html 数据 转成 dataframe
    df = pandas.DataFrame(
        pandas.read_html(table, encoding='utf-8', header=0)[0])
    # 找出Country,Other列为Total: 的那一行, 取反集
    df = df[~df['Country,Other'].isin(['Total:'])]
    print(df)
    # 根据TotalCases 排序 , ascending 真为正序
    df = df.sort_values(by=['TotalCases'], ascending=False,
                        na_position='last').drop_duplicates()

    # # 写文件
    df.to_excel('data_zhujian.xlsx',
                index=False,
                sheet_name='worldcoronavirusdata')
    # df.to_csv('data_zhujian.csv', index=False)
    # csv = pandas.read_csv('data_zhujian.csv', encoding='utf-8')
    # csv.to_excel('data_zhujian.xlsx', sheet_name='coronavirusdata')
    all_data = list(df.T.to_dict().values())
    return all_data


def dict_2_line(dictdata, dtype):
    res = ''
    line = '|'
    # print(dictdata)
    if dtype == 'key':
        for key in dictdata:
            line = line + str(key) + '|'
        # print(line)
        return line
    elif dtype == 'value':
        for key in dictdata:
            line = line + str(dictdata[key]) + '|'
        # print(line)
        return line
    elif dtype == 'spe':
        for key in dictdata:
            line = line + ':' + len(key) * '-' + ':|'
        # print(line)
        return line


def data_2_md(data):
    md = []
    tab_head = dict_2_line(data[0], 'key')
    tab_spe = dict_2_line(data[0], 'spe')
    md.append(tab_head)
    md.append(tab_spe)
    for k in data:
        md.append(dict_2_line(k, 'value'))

    print(md)


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


def send_dingdingfile(dd_webhook_TOKEN,
                      dd_webhook_SECRET,
                      msg,
                      file,
                      atall=False,
                      **at_person_list):

    pass



def get_all_data_cn(url):
    res = requests.get(url)
    res_ele = lxml.etree.HTML(res.text)
    # print(res_ele)
    data = res_ele.xpath('//script[@id="captain-config"]')[0].text
    data_js = json.loads(data)
    globalList = data_js['component'][0]['globalList']
    dfall = pandas.json_normalize(globalList)
    newdf = pandas.DataFrame(columns=[
                             'area', 'country', 'confirmed', 'curConfirm', 'died', 'crued'])
    # dfall.to_csv('data.csv', index=False)
    # print(newdf)
    # print(dfall['subList'].size, dfall['subList'].shape)
    for row in dfall.itertuples():
        area_ = getattr(row, 'area')
        df_c = pandas.DataFrame(getattr(row, 'subList'))
        if area_ == '热门':
            break
        for country in df_c.itertuples():
            newline = {'area': area_,
                       'country': getattr(country, 'country'),
                       'confirmed': getattr(country, 'confirmed'),
                       'curConfirm': getattr(country, 'curConfirm'),
                       'died': getattr(country, 'died'),
                       'crued': getattr(country, 'crued')}
            # print(newline)
            newdf = newdf.append(pandas.DataFrame(
                newline, index=[1]), ignore_index=True)
    # 转换数据格式 两种皆可
    # newdf['confirmed']=newdf['confirmed'].astype(int)
    newdf['confirmed'] = newdf['confirmed'].apply(pandas.to_numeric)
    newdf = newdf.sort_values(by=['confirmed'], ascending=False,
                              na_position='last')
    # newdf = newdf. [:,['area', 'country', 'confirmed', 'curConfirm', 'died', 'crued']        ]
    # with open(r'./1.txt', 'w', encoding='utf-8') as fp:
    #     json.dump(globalList,fp)
    newdf.to_excel('data_zhujian_baidu.xlsx', index=False, sheet_name='cornav')
    return True



dd_webhook_TOKEN = '5e3daeff1f18aa6d55d7b9f7d29fa66f18502d24e8d9a097ab3c19624d720b8c'
dd_webhook_SECRET = 'SECe436d190bead54753076a089572ea574c14203bb1af33e20645e0d8439b9f15f'

atpersonlist = ['18260414208', '18260414268']
atpersonlist = ['18963640027']

data_url = 'https://feiyan.wecity.qq.com/wuhan/dist/index.html#/'
# 'https://who.sprinklr.com/'
data_url3 = 'https://voice.baidu.com/act/newpneumonia/newpneumonia'
data_url2 = 'https://www.worldometers.info/coronavirus/#countries'

# a = []
# data_2_md(a)

# a = get_all_data(data_url2)
get_all_data_cn(data_url3)
