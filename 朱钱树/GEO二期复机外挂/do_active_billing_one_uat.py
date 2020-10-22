#-*- coding:utf-8 -*-
import yaml,threading,time,sys,os
from socket import *
import logging
import logging.config
import copy
import struct
import binascii
import re
import collections
import urllib2
from xml.dom import minidom
import base64
import datetime
import cx_Oracle
import subprocess
from rediscluster import RedisCluster

redis_nodes = [
        {'host': '10.10.181.126','port': 28001},
        {'host': '10.10.181.126','port': 28002},
        {'host': '10.10.181.126','port': 28003},
        {'host': '10.10.181.126','port': 28004},
        {'host': '10.10.181.126','port': 28005},
        {'host': '10.10.181.126','port': 28006},
        {'host': '10.10.181.121','port': 28001},
        {'host': '10.10.181.121','port': 28002},
        {'host': '10.10.181.121','port': 28003},
        {'host': '10.10.181.121','port': 28004},
        {'host': '10.10.181.121','port': 28005},
        {'host': '10.10.181.121','port': 28006}
    ]
r = RedisCluster(startup_nodes=redis_nodes)

def send_ws(url,data,Content_Type="text/xml"):
    req = urllib2.Request(url, data)
    req.add_header('user-agent', 'Jakarta Commons-HttpClient/3.1')
    req.add_header('SOAPAction', "")
    #req.add_header('action', str(action))
    req.add_header('Accept-Encoding', 'gzip,deflate')
    #req.add_header('Content-Type', 'application/soap+xml;charset=UTF-8;')
    req.add_header('Content-Type', '%s;charset=UTF-8' % Content_Type)
    #req.add_header('connection', 'keep-alive')
    try:
        resp = urllib2.urlopen(req)
        msg=resp.read()
    except urllib2.HTTPError,e:
        msg=e.read()
    except urllib2.URLError,e:
        msg=str(e.reason)
    except BaseException,e:
        msg=str(e.message)
    return msg
            
con=cx_Oracle.connect("zmm/1jian8Shu!@10.10.181.10:21521/cc")
cur=con.cursor()
sql_get_data='''
SELECT L.PROD_INST_ID, L.ACC_NUM
  FROM COC.PROD_INST L
 WHERE L.PROD_INST_ID IN
       (SELECT H.PROD_INST_ID
          FROM (SELECT A.STOP_TYPE,
                       (SELECT NVL(SUM(NVL(C.ORI_CHARGE, 0) +
                                       NVL(C.SETT_CHARGE, 0)),
                                   0)
                          FROM ABC.RM_DOC@TO_ABC C
                         WHERE C.ACCT_ID = B.ACCT_ID
                           AND C.DUE_DATE < SYSDATE
                           AND C.DOC_TYPE_ID = 1) AS ALL_DUE,
                       (SELECT NVL(SUM(NVL(E.KIV_AMOUNT, 0) +
                                       NVL(E.KIV_SETTLE_AMOUNT, 0)),
                                   0)
                          FROM ABC.COLL_KIV_LIST@TO_ABC E
                         WHERE E.ACCT_ID = B.ACCT_ID
                           AND E.EXP_DATE > SYSDATE) AS ALL_KIV_AMOUNT,
                       (SELECT NVL(SUM(NVL(Q.DUE_AMOUNT, 0) +
                                       NVL(Q.PAYEMTN_AMOUNT, 0)),
                                   0)
                          FROM ABC.COLL_PAYMENT_PLAN@TO_ABC Q
                         WHERE Q.ACCT_ID = B.ACCT_ID
                           AND Q.PLAN_STATUS = 'A'
                           AND Q.DUE_DATE > SYSDATE) AS INSTALLMENT_AMOUNT,
                       (SELECT NVL(SUM(NVL(N.AMOUNT, 0)), 0)
                          FROM ABC.RM_MAIN_BAL@TO_ABC N
                         WHERE N.ACCT_ID = B.ACCT_ID) AS ADVANCE_PAYMENT,
                       B.*
                  FROM COC.PROD_INST_STATE A
                  LEFT JOIN COC.PROD_INST_ACCT_REL B
                    ON A.PROD_INST_ID = B.PROD_INST_ID
                 WHERE A.STOP_TYPE = '130001'
                   AND (B.IF_DEFAULT_ACCT_ID = 1 OR
                       (B.IF_DEFAULT_ACCT_ID = 0 AND
                       B.ACCT_ITEM_GROUP_ID = 0 AND B.PAYMENT_LIMIT > 0))) H
         WHERE H.ALL_DUE - H.ALL_KIV_AMOUNT - H.INSTALLMENT_AMOUNT +
               H.ADVANCE_PAYMENT <= 0
           AND EXISTS (SELECT 1
                  FROM COC.PROD_INST P
                 WHERE H.PROD_INST_ID = P.PROD_INST_ID
                   AND P.STATUS_CD = '120001')
         GROUP BY H.PROD_INST_ID
        HAVING COUNT(*) > 1)
'''
while True:
    cur.execute(sql_get_data)
    #curr_time=time.strftime('%Y%m%d%H%M%S',time.localtime(time.time()))
    #print(curr_time)
    for one in cur.fetchall():
        curr_time=time.strftime('%Y%m%d%H%M%S',time.localtime(time.time()))
        print(curr_time)
        prod_inst_id,acc_num = one
        #基础参数
        subs_id=prod_inst_id
        #停复机消息
        #信控停复机Hash的field：CREDIT|{eventSrc}|{transactionId}|{eventType}|{triggerTime}
        #欠费停复机Hash的field：DEBT|{eventSrc}|{transactionId}|{eventType}|{triggerTime}
        #余额过期停复机Hash的field：EXP|{eventSrc}|{transactionId}|{eventType}|{triggerTime}
        #功能产品暂停/恢复Hash的field：PROD|{eventSrc}|{transactionId}|{eventType}|{triggerTime}
        #销售品暂停/恢复Hash的field：OFFER|{eventSrc}|{transactionId}|{eventType}|{triggerTime }
        dict_name="COLL_%s" % subs_id
        transactionId = "99899%s" % int(time.time()*1000)
        triggerTime = datetime.datetime.now().strftime('%Y%m%d%H%M%S%f')[:-3]
        dict_key="DEBT|2|%s|10|%s" % (transactionId,triggerTime)
        dict_value='{"subsId":%s,"eventSrc":"4","eventType":7,"triggerTime":%s,"prepayFlag":0,"transactionId":"%s","channelId":8}' % (subs_id,triggerTime,transactionId)
        r.hset(name=dict_name,key=dict_key,value=dict_value)
        print dict_name,r.hgetall(dict_name)
        
        #普通索引消息 Key是COLL_SUBS，value是Set，存放subsId值
        r.sadd("COLL_SUBS",subs_id)
        print "COLL_SUBS",r.smembers("COLL_SUBS")
        
        #复机索引消息 Key是COLL_RESUME_SUBS，value是Set，存放subsId值
        r.sadd("COLL_RESUME_SUBS",subs_id)
        print "COLL_RESUME_SUBS",r.smembers("COLL_RESUME_SUBS")

        sql="insert into active_trigger_billing_msg(prod_inst_id,acc_num,mq_msg) values(:0,:1,:2)"
        cur.execute(sql,[prod_inst_id,acc_num,dict_value])
        con.commit()
    time.sleep(300)
