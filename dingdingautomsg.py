#-*- coding:utf-8 -*-
import sys
'''
@Author: Zhu Jian
@Date: 2019-09-12 04:25:12
@LastEditTime: 2020-04-20 21:09:48
@LastEditors: Zhu Jian
@Description: 
@FilePath: \py_practice\dingdingautomsg.py
@
'''
import yaml
import datetime
import os
import string
import base64
import json
import requests


def send_dingdingmsg(dd_webhook,msg,atall=True,**at_person_list):
    print(at_person_list['at_person_list'])
    HEADERS={"Content-Type":"application/json;charset=utf-8"}
    #String_textMsg={"msgtype":"text","text":{"content":msg},"at":{"atMobiles":at_person_list['at_person_list'],"isAtAll":atall}}
    String_textMsg={"msgtype":"text","text":{"content":msg},"at":{"isAtAll":atall}}
#   String_textMsg={"msgtype":"text","text":{"content":msg},"at":{"isAtAll":atall}}
    String_textMsg=json.dumps(String_textMsg)
    res=requests.post(dd_webhook,data=String_textMsg,headers=HEADERS)
    print(res.text)
dd_webhook='https://oapi.dingtalk.com/robot/send?access_token=c7a2f9675bdc80005caf9f5fa3ea0393ddaf84e8d097c702d1039adc58f6e0cd'
atpersonlist=['18260414208','18260414268']
atpersonlist=[]
send_dingdingmsg(dd_webhook,'python 自动发送测试 05 at all',at_person_list=atpersonlist)