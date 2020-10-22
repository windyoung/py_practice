#-*- coding:utf-8 -*-
'''
Author: Zhu Jian
Date: 2020-10-22 16:32:52
LastEditTime: 2020-10-22 16:34:49
LastEditors: Zhu Jian
Description: 
FilePath: \py_practice\testpinyin.py

'''
import pypinyin as pyp 
a=pyp.lazy_pinyin("银行很行")
print(a)