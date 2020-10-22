#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import urllib2
import requests
from threading import Timer

weather_url = "http://www.weather.com.cn/data/sk/101190101.html"
dingding    = "https://oapi.dingtalk.com/robot/send?access_token=c330939d7059651cd8a1e88b33a80143e0f9f696a7a66d99b297df3fc17cc9d2"
interval    = 600;

def get_weather():
    r = requests.get(weather_url)
    r.encoding = 'utf-8'
    weather =  r.json()['weatherinfo']
    city = weather['city']
    wind = weather['WD']
    temp = weather['temp']
    return u"[天气播报] 城市:%s, 风向: %s, 温度: %s" % (city, wind, temp)

def send_message(interval):
    header = {
        "Content-Type": "application/json",
        "charset":      "utf-8"
    }

    data = {
        "msgtype": "text",
        "text": {
            "content": get_weather()
        },
        "at": {
            "isAtAll": False
        }
    }

    sendData = json.dumps(data)
    request = urllib2.Request(dingding, data = sendData, headers = header)
    urlopen = urllib2.urlopen(request)
    print urlopen.read()
    Timer(interval, send_message, (interval,)).start()

def main():
    send_message(interval)

if __name__ == '__main__':
    main()
