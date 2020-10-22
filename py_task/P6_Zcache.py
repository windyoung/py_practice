# -*- coding:utf-8 -*-

'''
@Author: windyoung
@Date: 2020-05-20 16:43:05
LastEditTime: 2020-10-22 19:04:31
LastEditors: windyoung
@Description: 
FilePath: \py_practice\py_task\P6_Zcache.py
@
'''
import redis

import T_OFFER_INST_RECORD_pb2 as kk
'''
作业6 查询 zcache中 一个号码 订购的所有offer列表 @所有人  zcache 的地址 @崔昕晖(崔昕晖)  发一下ip和端口
v9  ： 10.45.80.230 端口8个 8001-8008
v9p： 172.16.81.80 端口 28001 28002
          172.16.81.81 端口 28001 28002
'''

# r_db1=redis.Redis(host='10.45.80.230',port=8001,db=0)
r_db1=redis.StrictRedis(host='172.16.81.80',port=28001,db=0)
# print(r_db1.info())
# print("1===",r_db1.scan(0))
# print("1===",r_db1.get(0))
# print("2===",r_db1.scan('1048576'))
# print("2===",r_db1.hgetall('SUBS_31430320')) 
# print("3===",r_db1.exists('SUBS_31430320')) 
c=r_db1.hget('SUBS_31430320','OFST|131430320|1')
print("3===",type(c),c) 
oir=kk.OfferInstRecord()
oir.ParseFromString(c)
print(oir,type(oir))
oir.status_cd="1000"
d=oir.SerializeToString()
print(oir ,d)
r_db1.hset('SUBS_31430320','OFST|131430320|1',d)
r_db1.close()
