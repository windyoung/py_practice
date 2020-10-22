# coding: utf-8
__author__ = 'li.rongqing@nj'
__date__ = '$__DATE__$'

import sys, os, re, platform
Import("env")
env = env.Clone()
dubug_new_cpp=env.Expand(" $(DEBUG_NEW_FLAG)")
ALL_SRC='''
  BillLog.cpp
	Common.cpp
	DataStruct.cpp
  DiameterParser.cpp
  ReadIni.cpp
	SOHelper.cpp
	TDataConvertFunc.cpp
	TDateTimeFunc.cpp
	TException.cpp
	TFileOper.cpp
	TFileScanOper.cpp
	trie.cpp
	TSem.cpp
	TShm.cpp
	TSplit.cpp
	TStrFunc.cpp
	TMathFunc.cpp
	wgetopt.cpp
	tinyxml.cpp
	tinyxmlerror.cpp
	tinyxmlparser.cpp
	Mutex.cpp
	TThreadBase.cpp
	md5.cpp
  Aes_Encryptor.cpp
  Aes.cpp
'''+ dubug_new_cpp

env.SetOptionObj2So('$(AR)')
env.BuildShlib('helper',ALL_SRC.split(),'')

