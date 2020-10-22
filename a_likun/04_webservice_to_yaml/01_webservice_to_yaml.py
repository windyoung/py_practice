#!/usr/bin/env python
# -*- coding: utf-8 -*-

import yaml
from suds.client import Client
from suds.sudsobject import asdict
from suds.sax.text import Text

message = '''
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsd="http://com.ztesoft.zsmart/xsd">
   <soapenv:Header>
      <xsd:AuthHeader>
         <Username>CCPGW1</Username>
         <Password>yyEec23J</Password>
         <username xsi:nil="true" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"/>
         <password xsi:nil="true" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"/>
      </xsd:AuthHeader>
   </soapenv:Header>
   <soapenv:Body>
      <xsd:queryUserProfile>
         <QueryUserProfileReqDto>
         <MSISDN>6620240331038</MSISDN>
         <RequestID>TEST12345678</RequestID>
         </QueryUserProfileReqDto>
      </xsd:queryUserProfile>
   </soapenv:Body>
</soapenv:Envelope>
'''

url    = 'http://10.80.175.219:8090/services/TrueWebServices?wsdl'
client = Client(url)
retmsg = client.service.queryUserProfile(__inject={'msg':message})

def recursive_dict(d):
    out = {}
    for k, v in asdict(d).items():
        if hasattr(v, '__keylist__'):
            out[k] = recursive_dict(v)
        elif isinstance(v, list):
            out[k] = []
            for item in v:
                if hasattr(item, '__keylist__'):
                    out[k].append(recursive_dict(item))
                else:
                    out[k].append(item)
        else:
            out[k] = str(v) if isinstance(v, Text) else v
    return out

d = recursive_dict(retmsg)
print(yaml.dump(d))
