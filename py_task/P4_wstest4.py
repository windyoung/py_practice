# -*- coding:utf-8 -*-
# 作业4：使用WS接口，查询用户的余额，并把返回包的所有属性 使用yaml格式写到文件中 （需要区分父子节点）
# url = 'http://172.16.24.191:9902/services/DbepService.DbepServiceHttpSoap11Endpoint/'
'''
@Author: windyoung
@Date: 2020-04-30 10:05:06
LastEditTime: 2020-10-22 19:04:10
LastEditors: windyoung
@Description: 
FilePath: \py_practice\py_task\P4_wstest4.py
@
'''
# 报文
# <soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsd="http://com.ztesoft.zsmart/xsd" xmlns:par="http://com.ztesoft.zsmart/xsd/param">
#    <soapenv:Header/>
#    <soapenv:Body>
#       <xsd:queryAcctBal>
#          <!--Optional:-->
#          <xsd:args0>
#             <!--Optional:-->
#             <par:accountCode></par:accountCode>
#             <!--Optional:-->
#             <par:msisdn>{0}</par:msisdn>
#             <!--Optional:-->
#             <par:offerCode></par:offerCode>
#          </xsd:args0>
#       </xsd:queryAcctBal>
#    </soapenv:Body>
#  </soapenv:Envelope>
#  ab/1jian8Shu!@10.45.10.164:1521/dbep
# bc/1jian8Shu!@10.45.10.164:1521/dbep
# inv/1jian8Shu!@10.45.10.164:1521/dbep
# rb/1jian8Shu!@172.16.81.202:21521/cc
# src/1jian8Shu!@10.45.10.164:1521/dbep
# vcon/1jian8Shu!@10.45.10.164:1521/dbep
# vcoff/1jian8Shu!@10.45.10.164:1521/dbep
# oc/1jian8Shu!@10.45.10.164:1521/dbep
# custc/1jian8Shu!@10.45.10.164:1521/dbep
# ebc/1jian8Shu!@10.45.10.164:1521/dbep


from suds.client import Client
import yaml
import pandas as pd
import xmltodict

ws_url = 'http://172.16.24.191:9902/services/DbepService.DbepServiceHttpSoap11Endpoint/'
ws_url = 'http://172.16.24.191:9902/DbepService?wsdl'
ws_body = '''
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsd="http://com.ztesoft.zsmart/xsd" xmlns:par="http://com.ztesoft.zsmart/xsd/param">
   <soapenv:Header/>
   <soapenv:Body>
      <xsd:queryAcctBal>
         <!--Optional:-->
         <xsd:args0>
            <!--Optional:-->
            <par:accountCode></par:accountCode>
            <!--Optional:-->
            <par:msisdn>{0}</par:msisdn>
            <!--Optional:-->
            <par:offerCode></par:offerCode>
         </xsd:args0>
      </xsd:queryAcctBal>
   </soapenv:Body>
 </soapenv:Envelope>
'''
ws_param = ['<par:msisdn>995593080850</par:msisdn>', '123456']


def test4_getws_resp(url, param):
    clt = Client(url)
    clt.service.queryAcctBal({'msisdn': '995593080850'})
    req = str(clt.last_sent())
    resp = str(clt.last_received())
    # print(req)
    # print(resp)
    return {'request': req, 'response': resp}


def test4_resp2yaml(resp):
    xml_data=xmltodict.parse(resp)
    # print(xml_data['soapenv:Envelope']['soapenv:Body'])
    res=pd.DataFrame.from_dict(xml_data['soapenv:Envelope']['soapenv:Body']).to_json().replace('ns:','').replace('ax21:','')
    # print(str(res.replace('ns:','').replace('ax21:','')))
    ws_yaml=yaml.unsafe_load(res)
    # print(ws_yaml)
    with open('./ws.txt','w',encoding='utf-8') as fp:
        yaml.dump(ws_yaml,fp)


res=test4_getws_resp(ws_url,ws_param)
# test4_resp2yaml(res['response'])
print(res['response'])
