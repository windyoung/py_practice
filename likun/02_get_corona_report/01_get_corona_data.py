#!/usr/bin/env python
# -*- coding: utf-8 -*-

import re
import json
import requests
import pandas as pd

url = "https://voice.baidu.com/act/newpneumonia/newpneumonia/?from=osari_pc_3#tab4"

def get_data():
    block_begin = '<script type="application/json" id="captain-config">'
    block_mat1  = '<script'
    block_mat2  = '</script'
    r           = requests.get(url)
    text        = r.text
    r.close()

    idx  = text.find(block_begin)
    text = text[idx:]
    if text:
        level       = 0
        pat = re.compile(r'</?script', re.IGNORECASE)
        pat.search(text)
        for block in pat.finditer(text):
            if block.group() == block_mat1:
                level = level + 1
            elif block.group() == block_mat2:
                level = level - 1
                if level == 0:
                    text = text[0:block.end()+1]
                    text = re.sub(r'^<script.*?>', '', text, flags=re.IGNORECASE)
                    text = re.sub(r'</script>$',   '', text, flags=re.IGNORECASE)
                    return text
        return None

def to_excel(filename, sheet_name, data, columns):
    writer = pd.ExcelWriter(filename, engine='xlsxwriter')
    df     = pd.DataFrame(data=data, columns=columns)
    df.to_excel(writer, index=False, sheet_name=sheet_name)
    workbook  = writer.book
    worksheet = writer.sheets[sheet_name]

    format1 = workbook.add_format({
        'bold':      False,
        'text_wrap': False,
        'align':     'left',
        'border':    1,
        'font_name': '微软雅黑',
        'font_size': 10})
    worksheet.set_column('A:A', 20, format1)

    format2 = workbook.add_format({
        'bold':       False,
        'text_wrap':  False,
        'align':      'right',
        'border':     1,
        'num_format': '#,##0',
        'font_name':  '微软雅黑',
        'font_size':  10})
    worksheet.set_column('B:F', 10, format2)

    format_header = workbook.add_format({
        'bold':      True,
        'text_wrap': False,
        'align':     'center',
        'border':    1,
        'font_name': '微软雅黑',
        'font_size': 11,
        'bg_color':  '#8DB4E3'})
    for col_num, value in enumerate(df.columns.values):
        worksheet.write(0, col_num, value, format_header)
    writer.save()

def format_num(value, default='-'):
    if value:
        return int(value)
    return default

def main():
    data_text    = get_data()
    data_json    = json.loads(data_text)
    data_oversea = data_json['component'][0]['caseOutsideList']
    data_oversea.sort(reverse=True, key=lambda item: (
        format_num(item['confirmed'], 0),
        format_num(item['curConfirm'], 0),
        format_num(item['confirmedRelative'], 0),
        item['area']))
    data = [(
        item['area'],
        format_num(item['confirmedRelative']),
        format_num(item['curConfirm']),
        format_num(item['confirmed']),
        format_num(item['crued']),
        format_num(item['died'])
    ) for item in data_oversea]

    columns    = ['疫情地区', '新增', '现有', '累计', '治愈', '死亡']
    filename   = 'corona-virus.xlsx'
    sheet_name = 'oversea'
    to_excel(filename, sheet_name, data, columns)

if __name__ == '__main__':
    main()
