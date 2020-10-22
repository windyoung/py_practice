#-*- coding:utf-8 -*-
'''
Author: windyoung
Date: 2020-10-22 16:32:52
LastEditTime: 2020-10-22 19:03:13
LastEditors: windyoung
Description: 
FilePath: \py_practice\testpinyin.py

'''
import pypinyin as pyp 
a = "银行很行"
t1="经视经视近视镜时"
b=pyp.lazy_pinyin(a)
c=pyp.pinyin(t1)
print(b,c)
