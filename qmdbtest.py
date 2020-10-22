# -*- coding:utf-8 -*-

'''
@Author: Zhu Jian
@Date: 2020-05-20 16:43:05
LastEditTime: 2020-09-14 23:29:53
LastEditors: Zhu Jian
@Description: 
FilePath: \py_practice\qmdbtest.py
@
'''
import jaydebeapi


def qmdb():
    "mdbcsql ocs/ocs@ocs 172.16.25.85:13110 "
    url = 'jdbc:qdb:@172.16.25.85:13110/ocs'
    url_group = 'jdbc:qdb:group:@172.16.25.85:13110/ocs'
    user = 'ocs'
    password = 'ocs'
    dirver = 'oracle.jdbc.driver.OracleDriver'
    dirver = "com.ztesoft.zsmart.jdbc.qdb.driver"
    jarFile = './libs/qmdbjdbc-9.0.3-mdb.jar'
    # jarFile = r'./libs/qmdbjdbc-9.0.3-mdb.jar'
    sqlStr = 'select * from dual'
    # conn=jaydebeapi.connect('oracle.jdbc.driver.OracleDriver','jdbc:oracle:thin:@127.0.0.1:1521/orcl',['hwf_model','hwf_model'],'E:/pycharm/lib/ojdbc14.jar')
    conn = jaydebeapi.connect(dirver, url_group, [user, password], jarFile)
    curs = conn.cursor()
    curs.execute(sqlStr)
    result = curs.fetchall()
    print(result)
    curs.close()
    conn.close()


qmdb()