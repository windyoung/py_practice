# -*- coding:utf-8 -*-
import datetime
import json
import os

import requests
import xlsxwriter
from bs4 import BeautifulSoup


'''
Author: Zhu Jian
Date: 2020-09-17 20:08:46
LastEditTime: 2020-09-17 20:26:50
LastEditors: Zhu Jian
Description: chen.jianxin
FilePath: \py_practice\爬数据.py

'''


def write_xlsx_export(xlsx_file, data_dist={}, data_validation_dist={}):
    workbook = xlsxwriter.Workbook(xlsx_file, {'constant_memory': True})
    title_format = workbook.add_format({'font_name': 'Arial', 'font_size': 10})
    title_format.set_bg_color('yellow')
    cell_format = workbook.add_format({'font_name': 'Arial', 'font_size': 10})
    date_format = workbook.add_format(
        {'font_name': 'Arial', 'font_size': 10, 'num_format': 'yyyy-mm-dd hh:mm:ss'})
    for one in data_dist:
        # if len(data_dist[one])== 1:
            # continue
        worksheet = workbook.add_worksheet(one)
        worksheet.freeze_panes(1, 1)
        data = data_dist[one]
        row = 0
        len_map = {}
        for one_r in data:
            col = 0
            for one_c in one_r:
                _x = 10
                # if len_map.has_key(col): python2
                if col in len_map:
                    if len_map[col] < _x:
                        len_map[col] = _x
                else:
                    len_map[col] = _x
                worksheet.set_column(col, col, len_map[col])

                if row == 0:
                    worksheet.write(row, col, one_c, title_format)
                else:
                    if type(one_c) == datetime.datetime:
                        worksheet.write(row, col, one_c, date_format)
                    else:
                        try:
                            worksheet.write(row, col, one_c, cell_format)
                        except BaseException as e:
                            print(str(e))
                col += 1
            row += 1

        # if data_validation_dist.has_key(one): python2
        if one in data_validation_dist:
            try:
                for one_data_validation in data_validation_dist[one]:
                    data_list = data_validation_dist[one][one_data_validation]
                    worksheet.data_validation("%s2:%s%s" % (one_data_validation, one_data_validation, len(
                        data)), {'validate': 'list', 'source': data_list})
            except BaseException as e:
                print(str(e))
    workbook.close()


def take(elem):
    return elem[4]


url = 'https://voice.baidu.com/act/newpneumonia/newpneumonia/?from=osari_pc_3'
strhtml = requests.get(url).text
bs = BeautifulSoup(strhtml, "html.parser")

data = bs.find(id='captain-config').text
# print(data) 屏蔽输出
data_dist = json.loads(data)
globalList = data_dist['component'][0]['globalList']
data_map = {}
data_map[u'疫情播报'] = [(u'疫情地区', u'疫情国家', u'新增', u'现有', u'累计', u'治愈', u'死亡')]
data_list = set()
for one_area in globalList:
    area = one_area['area']
    if area == u'热门':
        continue
    for one_country in one_area['subList']:
        country = one_country['country']
        confirmedRelative = one_country['confirmedRelative']
        if confirmedRelative:
            confirmedRelative = int(confirmedRelative)
        else:
            confirmedRelative = 0
        curConfirm = one_country['curConfirm']
        if curConfirm:
            curConfirm = int(curConfirm)
        else:
            curConfirm = 0
        confirmed = one_country['confirmed']
        if confirmed:
            confirmed = int(confirmed)
        else:
            confirmed = 0
        crued = one_country['crued']
        if crued:
            crued = int(crued)
        else:
            crued = 0
        died = one_country['died']
        if died:
            died = int(died)
        else:
            died = 0
        data_list.add((area, country, confirmedRelative,
                       curConfirm, confirmed, crued, died))
data_list = list(data_list)
data_list.sort(key=take, reverse=True)
data_map[u'疫情播报'].extend(data_list)
xlsx_file = os.path.join('./', 'coronavirus_report_%s.xlsx' %
                         str(datetime.datetime.now())[0:10])
write_xlsx_export(xlsx_file, data_map)
