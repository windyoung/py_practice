# coding=utf-8
#!/usr/bin/env python3
'''
@Author: Zhu Jian
@Date: 2020-04-15 16:25:26
@LastEditTime: 2020-04-20 21:17:40
@LastEditors: Zhu Jian
@Description: 
@FilePath: \py_practice\db_data_comp.py
@
'''


import datetime
import logging
import os
import time

import pandas as pd

import cx_Oracle as cxora


def getTableInfo(ora_str, sql_str):
    logging.debug('{},{},{}'.format(ora_str, '', sql_str))
    conn = cxora.connect(ora_str)
    cur = conn.cursor()
    res = cur.execute(sql_str)
    restext = []
    for row in res:
        onerow = []
        for i in range(0, len(row)):
            if type(row[i]) == cxora.LOB:
                onerow.append(row[i].read())
            else:
                onerow.append(row[i])
        restext.append(onerow)
    logging.debug('{},{},{}'.format(sql_str, restext, ''))
    cur.close()
    conn.close()
    return restext


def insertIntoTable(ora_str, tablename, data):
    # tablename='ADVICE_TYPE_NEW'
    conn = cxora.connect(ora_str)
    cur = conn.cursor()
    s_sql = 'TRUNCATE TABLE {}'.format(tablename)
    cur.execute(s_sql)
    col = ', '.join(data.columns.tolist())
    s = ', '.join([':'+str(i) for i in range(1, data.shape[1]+1)])
    sql = 'insert into {}({}) values({})'.format(tablename, col, s)
    insdata = []
    insline = []
    for line in data.values.tolist():
        # if line[0] != 222:
        #     break
        for col in line:
            if type(col) == pd._libs.tslibs.timestamps.Timestamp:
                s_col = col.strftime('%Y-%m-%d %H%M%S')
                # print(type(s_col) , '------')
                insline.append(time.strptime(s_col, "%Y-%m-%d %H%M%S"))
            else:
                insline.append(col)
        insdata.append(line)
        # print(line)
        cur.execute(sql, line)
    # print(insdata)
    # logging.debug(col,'\n',s,'\n',sql)
    # logging.info(data.values.tolist()[0])
    # cur.executemany(sql, insdata)
    conn.commit()
    cur.close()
    return insdata


# test_db(ora_str,sql_str)
ora_str = 'zj_mig/smart@10.45.82.28:1521/orcl'
sql_str_data_a = 'select * from advice_type_a '
sql_str_data_b = 'select * from advice_type_b '
sql_str_column = '''SELECT TO_CHAR(WM_CONCAT(A.COLUMN_NAME))   FROM ALL_TAB_COLUMNS A WHERE A.OWNER = 'CC'   AND A.TABLE_NAME = 'ADVICE_TYPE' ORDER BY A.COLUMN_ID ASC'''
ora_str_A = r'abc/1jian8Shu)@172.16.81.82:21521/cc'  # geo v9p
ora_str_B = r'ab/1jian8Shu!@172.16.81.202:21521/cc'  # geo v9 new
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - %(name)s - %(funcName)s - %(levelname)s - %(message)s')
a = getTableInfo(ora_str, sql_str_data_a)
b = getTableInfo(ora_str, sql_str_data_b)
col = getTableInfo(ora_str, sql_str_column)[0][0]
# print(col,len(col),)
dfa = pd.DataFrame(a, columns=col.split(',')).fillna('').astype('object')
dfb = pd.DataFrame(b, columns=col.split(',')).fillna('').astype('object')
logging.debug('--------------dataframe a b --------------------------')
logging.debug('dfA \n {}{}'.format(dfa, dfa.dtypes))
logging.debug('dfB \n {}{}'.format(dfb, dfb.dtypes))
logging.debug('column name :{} \n'.format(col))
logging.debug('--------------the same data --------------------------')
dfb_new = pd.DataFrame(columns=col.split(','))
df_same = pd.merge(dfa, dfb, on=col.split(','), how='inner')
dfb_new = dfb_new.append(df_same)
logging.debug(
    '--------------dfb_new --------------------------\n{}'.format(dfb_new))
logging.debug('check if the same \n  {}'.format(dfa.isin(df_same)))
logging.debug('---------------data diffs   -------------------------')
dfa1 = dfa[~dfa['ADVICE_TYPE'].isin(df_same['ADVICE_TYPE'])]
dfb1 = dfb[~dfb['ADVICE_TYPE'].isin(df_same['ADVICE_TYPE'])]
logging.debug('dfa1\n  {} dfb1\n {}'.format(dfa1, dfb1))
dfb_new = dfb_new.append(dfb1).fillna('').astype('object').reindex()
# dfb_new=dfb_new['UPDATE_DATE'].astype('date')
logging.debug('---------------data diffs   -------------------------')
logging.debug(
    '--------------dfb_new --------------------------\n{}'.format(dfb_new))
tablename = 'ADVICE_TYPE_NEW'
logging.debug('dfb_new \n{}{}'.format(dfb_new, dfb_new.dtypes))
insertIntoTable(ora_str, tablename, dfb_new)
