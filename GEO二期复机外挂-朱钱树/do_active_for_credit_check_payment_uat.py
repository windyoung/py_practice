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
            
url="http://10.10.181.32:8080/services/DbepService.DbepServiceHttpSoap11Endpoint/"
xml_str_post='''
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsd="http://com.ztesoft.zsmart/xsd" xmlns:par="http://com.ztesoft.zsmart/xsd/param">
   <soapenv:Header/>
   <soapenv:Body>
      <xsd:qryAvailableCredit>
         <!--Optional:-->
         <xsd:args0>
            <!--Optional:-->
            <par:id>995%s</par:id>
            <!--Optional:-->
            <par:idType>1</par:idType>
            <!--Optional:-->
            <par:msgId></par:msgId>
         </xsd:args0>
      </xsd:qryAvailableCredit>
   </soapenv:Body>
</soapenv:Envelope>
'''
xml_str_pre='''
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsd="http://com.ztesoft.zsmart/xsd" xmlns:par="http://com.ztesoft.zsmart/xsd/param">
   <soapenv:Header/>
   <soapenv:Body>
      <xsd:queryAcctBal>
         <!--Optional:-->
         <xsd:args0>
            <!--Optional:-->
            <par:accountCode></par:accountCode>
            <!--Optional:-->
            <par:msisdn>995%s</par:msisdn>
            <!--Optional:-->
            <par:offerCode></par:offerCode>
         </xsd:args0>
      </xsd:queryAcctBal>
   </soapenv:Body>
</soapenv:Envelope>
'''
con=cx_Oracle.connect("zmm/1jian8Shu!@10.10.181.10:21521/cc")
cur=con.cursor()
con_pmt=cx_Oracle.connect("zmm/1jian8Shu!@10.10.181.11:21521/pmt")
cur_pmt=con_pmt.cursor()
sql_get_data_bak='''
select a.prod_inst_state_id,
       a.prod_inst_id,
       a.stop_type,
       sysdate status_date,
       to_char(sysdate, 'yyyymmddhh24miss') status_date_str,
       b.payment_mode_cd,
       b.acc_num
  from coc.prod_inst_state@TO_COC    a,
       coc.prod_inst@TO_COC          b,
       coc.prod_inst_acct_rel@TO_COC c
 where a.stop_type in ('300004', '300005')
   and a.prod_inst_id = b.prod_inst_id
   and a.prod_inst_id = c.prod_inst_id
   and c.if_default_acct_id = 1
   and b.status_cd in ('120001', '120002')
   --and b.payment_mode_cd = '2100'
   and a.prod_inst_id not in
       (SELECT PROD_INST_ID
          FROM (SELECT OT.ORDER_ITEM_ID, PI.PROD_INST_ID
                  FROM coc.PROD_INST@TO_COC PI, coc.ORDER_ITEM@TO_COC OT
                 WHERE PI.COMP_INST_ID = OT.OBJ_ID
                   AND OT.OFFER_KIND = '10'
                   AND OT.SERVICE_OFFER_ID in (7038,7040)
                UNION
                SELECT OT.ORDER_ITEM_ID, PI.PROD_INST_ID
                  FROM coc.OFFER_OBJ_INST_REL@TO_COC OOIR,
                       coc.PROD_INST@TO_COC          PI,
                       coc.ORDER_ITEM@TO_COC         OT
                 WHERE PI.PROD_INST_ID = OOIR.PROD_INST_ID
                   AND OOIR.COMP_INST_ID = OT.OBJ_ID
                   AND OT.SERVICE_OFFER_ID in (7038,7040)
                   AND OOIR.ACTION_TYPE != 'K'
                   AND OOIR.OBJ_TYPE = '100000'
                   AND OT.OFFER_KIND != '10') A
         WHERE EXISTS
         (SELECT ORDER_ITEM_ID
                  FROM coc.ORDER_ITEM_CHANGE@TO_COC B
                 WHERE B.ORDER_ITEM_ID = A.ORDER_ITEM_ID
                   AND B.TABLE_NAME in ('PROD_INST', 'PROD_INST_STATE')))
   and c.acct_id in
       (select acct_id
          from balc.BC_TRANSACTION_202008 a
         where (txn_type_id in (1, 3) or
               (txn_type_id = 6 and trans_direction = 'I'))
           and txn_date is not null
           and txn_date between sysdate - 2   and sysdate - 120 / 24 / 3600
           and txn_date >
               (select nvl(max(status_date), sysdate - 10) - 600 / 24 / 3600
                  from zmm.active_for_credit_check_payment_msg@TO_ZMM)
        union
        select acct_id
          from abc.rm_txn a
         where txn_type_id in (1, 3)
           and entry_date is not null
           and entry_date between sysdate - 6  and sysdate - 120 / 24 / 3600
           and entry_date >
               (select nvl(max(status_date), sysdate - 10) - 600 / 24 / 3600
                  from zmm.active_for_credit_check_payment_msg@TO_ZMM))
'''
sql_get_data='''
select a.prod_inst_state_id,
       a.prod_inst_id,
       a.stop_type,
       sysdate status_date,
       to_char(sysdate, 'yyyymmddhh24miss') status_date_str,
       b.payment_mode_cd,
       b.acc_num
  from coc.prod_inst_state@TO_COC    a,
       coc.prod_inst@TO_COC          b,
       coc.prod_inst_acct_rel@TO_COC c
 where a.stop_type in ('300004', '300005')
   and a.prod_inst_id = b.prod_inst_id
   and a.prod_inst_id = c.prod_inst_id
   and c.if_default_acct_id = 1
   and b.status_cd in ('120001', '120002')
   --and b.payment_mode_cd = '2100'
   and a.prod_inst_id not in
       (SELECT PROD_INST_ID
          FROM (SELECT OT.ORDER_ITEM_ID, PI.PROD_INST_ID
                  FROM coc.PROD_INST@TO_COC PI, coc.ORDER_ITEM@TO_COC OT
                 WHERE PI.COMP_INST_ID = OT.OBJ_ID
                   AND OT.OFFER_KIND = '10'
                   AND OT.SERVICE_OFFER_ID in (7038,7040)
                UNION
                SELECT OT.ORDER_ITEM_ID, PI.PROD_INST_ID
                  FROM coc.OFFER_OBJ_INST_REL@TO_COC OOIR,
                       coc.PROD_INST@TO_COC          PI,
                       coc.ORDER_ITEM@TO_COC         OT
                 WHERE PI.PROD_INST_ID = OOIR.PROD_INST_ID
                   AND OOIR.COMP_INST_ID = OT.OBJ_ID
                   AND OT.SERVICE_OFFER_ID in (7038,7040)
                   AND OOIR.ACTION_TYPE != 'K'
                   AND OOIR.OBJ_TYPE = '100000'
                   AND OT.OFFER_KIND != '10') A
         WHERE EXISTS
         (SELECT ORDER_ITEM_ID
                  FROM coc.ORDER_ITEM_CHANGE@TO_COC B
                 WHERE B.ORDER_ITEM_ID = A.ORDER_ITEM_ID
                   AND B.TABLE_NAME in ('PROD_INST', 'PROD_INST_STATE')))
   and c.acct_id in
       (select acct_id
          from balc.BC_TRANSACTION_%s a
         where (txn_type_id in (1, 3) or
               (txn_type_id = 6 and trans_direction = 'I'))
           and txn_date is not null
           and txn_date between sysdate - 2   and sysdate - 120 / 24 / 3600
           and txn_date >
               (select nvl(max(status_date), sysdate - 10) - 600 / 24 / 3600
                  from zmm.active_for_credit_check_payment_msg@TO_ZMM)
        union
        select acct_id
          from abc.rm_txn a
         where txn_type_id in (1, 3)
           and entry_date is not null
           and entry_date between sysdate - 6  and sysdate - 120 / 24 / 3600
           and entry_date >
               (select nvl(max(status_date), sysdate - 10) - 600 / 24 / 3600
                  from zmm.active_for_credit_check_payment_msg@TO_ZMM))
'''
while True:
    now=datetime.datetime.now()
    curr_table_time=now.strftime("%Y%m")
    #print sql_get_data % curr_table_time
    cur_pmt.execute(sql_get_data % curr_table_time)
    for one in cur_pmt.fetchall():
        prod_inst_state_id,prod_inst_id,stop_type,status_date,status_date_str,payment_mode_cd,acc_num = one
        #print xml_str % acc_num
        if str(payment_mode_cd) == '2100':
            ret = send_ws(url,xml_str_pre % acc_num)
            dom = minidom.parseString(ret)
            root = dom.documentElement
            names = root.getElementsByTagName("ax21:balDtoList")
            balances=0
            for name in names:
                try:
                    node = name.getElementsByTagName("ax21:acctResCode")[0].childNodes[0].nodeValue 
                    if node == "LOCAL_CURRENCY":
                        balances=name.getElementsByTagName("ax21:balance")[0].childNodes[0].nodeValue 
                except BaseException,e:
                    pass
            if float(balances) >= 0.005:
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
                dict_key="CREDIT|2|%s|10|%s" % (transactionId,triggerTime)
                dict_value='{"subsId":%s,"eventSrc":"2","eventType":10,"triggerTime":%s,"prepayFlag":1,"transactionId":"%s","channelId":8}' % (subs_id,triggerTime,transactionId)
                r.hset(name=dict_name,key=dict_key,value=dict_value)
                print dict_name,r.hgetall(dict_name)
                
                #普通索引消息 Key是COLL_SUBS，value是Set，存放subsId值
                r.sadd("COLL_SUBS",subs_id)
                print "COLL_SUBS",r.smembers("COLL_SUBS")
                
                #复机索引消息 Key是COLL_RESUME_SUBS，value是Set，存放subsId值
                r.sadd("COLL_RESUME_SUBS",subs_id)
                print "COLL_RESUME_SUBS",r.smembers("COLL_RESUME_SUBS")
                sql="insert into active_for_credit_check_payment_msg(prod_inst_id,acc_num,stop_type,balance,status_date,mq_msg) values(:0,:1,:2,:3,:4,:5)"
                cur.execute(sql,[prod_inst_id,acc_num,stop_type,balances,status_date,dict_value])
                con.commit()
        elif str(payment_mode_cd) == '1200':
            ret = send_ws(url,xml_str_post % acc_num)
            print ret
            dom = minidom.parseString(ret)
            root = dom.documentElement
            names = root.getElementsByTagName("ns:return")
            availableBalance=0
            unpaidBillingTotal=0
            companySharedThresholdLeft=0
            for name in names:
                try:
                    availableBalance=name.getElementsByTagName("ax21:availableBalance")[0].childNodes[0].nodeValue 
                except BaseException,e:
                    pass
                try:
                    companySharedThresholdLeft=name.getElementsByTagName("ax21:companySharedThresholdLeft")[0].childNodes[0].nodeValue 
                except BaseException,e:
                    pass
                try:
                    unpaidBillingTotal=name.getElementsByTagName("ax21:unpaidBillingTotal")[0].childNodes[0].nodeValue 
                except BaseException,e:
                    pass
            if float(availableBalance) >= 0.005:
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
                dict_key="CREDIT|2|%s|10|%s" % (transactionId,triggerTime)
                dict_value='{"subsId":%s,"eventSrc":"2","eventType":10,"triggerTime":%s,"prepayFlag":0,"transactionId":"%s","channelId":8}' % (subs_id,triggerTime,transactionId)
                r.hset(name=dict_name,key=dict_key,value=dict_value)
                print dict_name,r.hgetall(dict_name)
                
                #普通索引消息 Key是COLL_SUBS，value是Set，存放subsId值
                r.sadd("COLL_SUBS",subs_id)
                print "COLL_SUBS",r.smembers("COLL_SUBS")
                
                #复机索引消息 Key是COLL_RESUME_SUBS，value是Set，存放subsId值
                r.sadd("COLL_RESUME_SUBS",subs_id)
                print "COLL_RESUME_SUBS",r.smembers("COLL_RESUME_SUBS")

                sql="insert into active_for_credit_check_payment_msg(prod_inst_id,acc_num,stop_type,balance,status_date,mq_msg) values(:0,:1,:2,:3,:4,:5)"
                cur.execute(sql,[prod_inst_id,acc_num,stop_type,availableBalance,status_date,dict_value])
                con.commit()
    time.sleep(60)
    
