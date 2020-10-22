#!/usr/bin/env python
#encoding:utf-8
'''
With this python script you can do this as follows:
   * encrypt string
   * encrypt file
   * decrypt string
   * decrypt file
   * get database password and username from web server
   use -h,--help for more information
author:zhang.he 
email :0027003921@zte.com.cn
date  :2015-3-20
'''
import sys
import time
import getopt
import codecs
import urllib
import urllib2
import socket
import shutil
import os
import logging
import ConfigParser

########################################################################
###@brief : 日志初始化
###@param : void
###@return: void
########################################################################
def loginit():
    logging.basicConfig(level=logging.DEBUG,
        format='%(asctime)s [%(levelname)-7s] [%(filename)25s:%(lineno)-5d]%(message)s',
        datefmt='%Y-%m-%d %H:%M:%S',
        filename='GetDBPassword.log',
        filemode='a')
########################################################################
###@brief : 获取日志句柄
###@param : void
###@return: void
########################################################################
def GetLogger():
    return logging.getLogger()


########################################################################
###@brief : 基类，包含对错误信息的处理
###@param : void
###@return: void
########################################################################
class Base:
    def __init__(self):
        self.code        = 0      #操作状态码0--成功，-1--失败，-2--成功，但是有警告
        self.reason      = ""     #失败原因或警告内容
    ########################################################################
    ###@brief : 设置错误信息
    ###@param : code    [in] 0----成功，-1----失败
    ###@param : reason  [in] code值为-1的时候，此处为失败的原因
    ###@return: void
    ########################################################################
    def set_errinfo(self,code=0,reason=""):
        self.code   = code
        self.reason = reason


########################################################################
###@brief : 字典类,对标准库类dict进行封装
###@param : void
###@return: void
########################################################################
class Dictionary:
    def __init__(self):
        self.__keyarray = {}
    def __len__(self):
        '''for len(object)'''
        return len(self.__keyarray)
    def __getitem__(self,key):
        '''for object[key]'''
        return self.__GetValue(key)
    def __setitem__(self,key,value):
        '''for object[key]=value'''
        self.__keyarray[key] = value
    def __str__(self):
        '''for str(object)'''
        str = ''
        for key in self.__keyarray:
            str = str + key + '=['+self.__keyarray[key]+'],'
        return str
    def GetIntegerValue(self,key,defint = -1):
        '''get integer value'''
        value = self.__GetValue(key)
        if(isinstance(value,int)):
            return value
        else:
            if(len(value) == 0):
                return defint
            else:
                return int(value)
    def GetStringValue(self,key,defstr = ''):
        '''get string value'''
        value = self.__GetValue(key)
        if(len(value) == 0):
            return defstr
        else:
            return value
    def __GetValue(self,deskey):
        '''get value'''
        for key in self.__keyarray:
            if(cmp(key,deskey) == 0):
                return self.__keyarray[key]
        return ''   
    def clear(self):
        self.__keyarray.clear()
    def keys(self):
        return self.__keyarray.keys()
        

########################################################################
###@brief : 加密解密工具类定义
###@param : void
###@return: void
########################################################################
class  TranslateTool(Base):
    ''' Function list:
        1、encrypt() Encrypt a string.
        2、decrypt() Decrypt a string.
        3、encrypt_file() Encrypt a file.
        4、decrypt_file() Decrypt a file.'''
    def __init__(self):
        Base.__init__(self)
    ########################################################################
    ###@brief : 加密明文
    ###@param : password [in] 明文，可见字符串
    ###@return: (state,string) state: 0-成功,-1-失败，string密文或者失败原因
    ########################################################################
    def encrypt(self,password):
        '''Encrypt a password string ,which consists of visible characters and numbers!'''
        newstr  = ''
        seq     = []
        if len(password) < 1:
            reason = 'The password inputed is error!'
            self.set_errinfo(-1,reason)
            return (-1,reason)
        
        for i in range(0, len(password)):
            j = ord(password[i])+30
            seq.append(j)
        
        for k in range(0, len(seq)):
            newstr = newstr + hex(seq[k])
                
        str_split = newstr.split('0x')
        
        for l in range(0, len(str_split)-1):
            if len(str_split[l+1]) < 2:
                str_split[l] = '0' + str_split[l+1]
        self.set_errinfo()
        return (0,''.join(str_split))
    ########################################################################
    ###@brief : 解密密文
    ###@param : password [in] 密文，16进制字符串
    ###@return: (state,string)state: 0-成功,-1-失败，string:明文或者失败原因
    ########################################################################
    def decrypt(self,password):
        '''Decrypt  a password string,which consists of hexadecimal numbers'''
        try:
            strRet = ''
            if len(password) < 2:
                reason = 'The password inputed is error!'
                self.set_errinfo(-1,reason)
                return (-1,reason)
            for i in range(0, len(password)/2):
                mid = password[i*2:(i+1)*2]
                strRet = strRet+(chr(int(mid,16)-30))
            self.set_errinfo()
            return (0,strRet)
        except ValueError,e:
            reason=str(e)
            if (reason[0] == '<'):
                reason = reason[1:len(reason)-1]
            self.set_errinfo(-1,reason)
            return (-1,reason)
    ########################################################################
    ###@brief : 加密文件
    ###@param : File    [in] 文件名
    ###@return: (state,string)state: 0-成功,-1-失败，string:失败原因
    ########################################################################
    def encrypt_file(self,File):
        ''' Encrypt file!'''
        try:
            srcfile=File
            desfile=srcfile+".bak"
            shutil.copy(srcfile,desfile)
            os.remove(srcfile)
            readfile  = open(desfile)
            writefile = codecs.open(srcfile,"a+","utf-8")
            line = readfile.readline()
            while line:
                if(len(line)==1 and (line == ' ' or line == '\n' or line == '\t')):
                    writefile.write(line)
                else:
                    (state,string) = self.encrypt(line)
                    if(state == 0):
                        writefile.write(string+"\n")
                    else:
                        writefile.write(line)
                line = readfile.readline()
            readfile.close()
            writefile.close()
            self.set_errinfo()
            return (0,"")
        except IOError,e:
            #print e
            reason=str(e)
            if (reason[0] == '<'):
                reason = reason[1:len(reason)-1]
            self.set_errinfo(-1,reason)
            return (-1,reason)
    ########################################################################
    ###@brief : 解密文件
    ###@param : File    [in] 文件名
    ###@return: (state,string)state: 0-成功,-1-失败，string:失败原因
    ########################################################################
    def decrypt_file(self,File):
        ''' Decrypt file!'''
        try:
            srcfile=File
            desfile=srcfile+".bak"
            shutil.copy(srcfile,desfile)
            os.remove(srcfile)
            readfile  = codecs.open(desfile,"r","utf-8")
            writefile = codecs.open(srcfile,"a+","utf-8")
            line = readfile.readline()
            while line:
                if(len(line)==1 and (line == ' ' or line == '\n' or line == '\t')):
                    writefile.write(line)
                else:
                    (state,string) = self.decrypt(line[0:len(line)-1])
                    if(state == 0):
                        writefile.write(string)
                    else:
                        writefile.write(line)
                line = readfile.readline()
            readfile.close()
            writefile.close()
            self.set_errinfo()
            return (0,"")
        except IOError,e:
            #print e
            reason=str(e)
            if (reason[0] == '<'):
                reason = reason[1:len(reason)-1]
            self.set_errinfo(-1,reason)
            return (-1,reason)

########################################################################
###@brief : ConfigFile类定义(操作.config类型文件)
###@param : void
###@return: void
########################################################################
class ConfigFile(Base):
    def __init__(self):
        self.file     = ""    #文件名称
        self.AddFlag  = False #True--记录不存在添加,False--记录不存在添加失败
        self.config   = ConfigParser.ConfigParser()
        self.config.optionxform = str#大小写敏感
    def __len__(self):
        '''for len(object)!'''
        CubeList=self.config.sections()
        return len(CubeList)
    def __str__(self):
        '''for str(object)!'''
        string = ""
        CubeList=self.config.sections()
        for i in range(0,len(CubeList)):
            print CubeList[i]
            string = string+"[%s]\n" % CubeList[i]
            itemlist = self.config.items(CubeList[i])
            for j in range(0,len(itemlist)):
                string = string + "%s=%s\n" %(itemlist[j][0],str(itemlist[j][1]))
            string = string + "\n"
        return string
    #暂定
    def __getitem__(self,cube):
        '''defined for obj[cube]'''
        index = self.FindCube(cube)
        if(index != -1):
            return self.config.items(cube)
    ########################################################################
    ###@brief : 清理函数
    ###@param : void
    ###@return: void
    ########################################################################
    def clear(self):
        self.file     = ""
    ########################################################################
    ###@brief : 获取cube名称
    ###@param : void
    ###@return: string cube名称
    ########################################################################
    def GetCubeName(self,index):
        CubeList=self.config.sections() 
        return  CubeList[index]
    ########################################################################
    ###@brief : 设置添加标志
    ###@param : flag   [in] bool型,false--不存在不添加,true--不存在添加
    ###@return: void
    ########################################################################
    def SetAddFlag(self,flag):
        self.AddFlag = flag 
    ########################################################################
    ###@brief : 查找cube,并返回序号
    ###@param : cube   [in] cube名称
    ###@return: int    cube序号
    ########################################################################
    def FindCube(self,cube):
        CubeList=self.config.sections()
        for i in range(0,len(CubeList)):
            if(CubeList[i] == cube):
                return i
        return -1
    ########################################################################
    ###@brief : 读取整形配置
    ###@param : cube     [in] cube名称
    ###@param : record   [in] record名称
    ###@param : defint   [in] 缺省值,不存在则返回该值,默认为0
    ###@return: int  整数
    ########################################################################
    def GetInteger(self,cube,record,defint = 0):
        (state,value) = self.GetValue(cube,record)
        if(state == -1):
            return defint
        else:
            return int(value)
    ########################################################################
    ###@brief : 读取string配置
    ###@param : cube     [in] cube名称
    ###@param : record   [in] record名称
    ###@param : defint   [in] 缺省值,不存在则返回该值,默认为0
    ###@return: int  整数
    ########################################################################
    def GetString(self,cube,record,defstr = ""):
        (state,value) = self.GetValue(cube,record)
        if(state == -1):
            return defstr
        else:
            return value
    ########################################################################
    ###@brief : 获取记录值
    ###@param : cube     [in] cube名称
    ###@param : record   [in] record名称
    ###@return: (state,string) 
    ########################################################################
    def GetValue(self,cube,record):
        try:
            value = self.config.get(cube,record)
            if(len(value) == 0):
                return (-1,"")
            else:
                return (0,self.Trim(value))
        except ConfigParser.NoSectionError,e:
            self.set_errinfo(-1,str(e))
            return(-1,self.reason)
        except ConfigParser.NoOptionError,e:
            self.set_errinfo(-1,str(e))
            return(-1,self.reason)
    ########################################################################
    ###@brief : 查找记录
    ###@param : key   [in] 比如:PASS_WORD|pass_word
    ###@return: (int,str) (状态,存在key)
    ########################################################################
    def FindMultiRecord(self,cube,key):
        keylist = key.split('|')
        for i in range(0,len(keylist)):
            if(self.config.has_option(cube,keylist[i])):
                return (0,keylist[i])
        return (-1,keylist[0])
    ########################################################################
    ###@brief : 更新配置
    ###@param : cube     [in] cube名称
    ###@param : record   [in] record名称
    ###@return: int   0--成功，1--失败 
    ########################################################################
    def update(self,cube,record,value):
        (state,record) = self.FindMultiRecord(cube,record)
        if(state == 0):
            self.config.set(cube,record,str(value))
            return 0
        else:
            #不存在添加
            if(self.AddFlag):
                index = self.FindCube(cube)
                #不存在cube添加之
                if(index == -1):
                    self.config.add_section(cube)
                    self.set_errinfo(-2,"Waring:No section: [%s]\n" % cube)
                else:
                    self.set_errinfo(-2,"Waring:No option '%s' in section: '%s'" %(record,cube))
                self.config.set(cube,record,str(value))
                return 0
            else:
                index = self.FindCube(cube)
                if(index == -1):
                    self.set_errinfo(-1,"Error:No section: [%s]\n" % cube)
                else:
                    self.set_errinfo(-1,"Error:No option '%s' in section: '%s'" %(record,cube))
                return -1
    ########################################################################
    ###@brief : 解析配置文件
    ###@param : file     [in] 配置文件名称
    ###@return: int   0--成功，1--失败 
    ########################################################################           
    def parse(self,file):
        '''read config from file'''
        if os.path.exists(file):
            self.file = file
        else:
            self.set_errinfo(-1,"file %s not exists!" % file)
            return -1;
        try:
            self.file = file
            self.config.read(self.file)
            return 0
        except ConfigParser.ParsingError,e:
            self.set_errinfo(-1,str(e))
            return -1
    ########################################################################
    ###@brief : 写配置文件
    ###@param : void
    ###@return: int   0--成功，1--失败 
    ########################################################################   
    def write(self):
        self.config.write(open(self.file, "w")) 
    ########################################################################
    ###@brief : 去掉字符串两边的无效字符
    ###@param : string    [in] 待处理字符串
    ###@return: void
    ########################################################################
    def Trim(self,string):
        if(len(string) == 0):
            return string
        else:
            #去掉字符串左边的无效字符
            ilength = len(string) 
            i = 0
            for i in range(0,ilength):
                if(string[i] == ' ' or string[i] == '\t' or string[i] == '\n' or string[i] == '\r'):
                    continue
                else:
                    string = string[i:]
                    break
            if(len(string)== 0):
                return ""
            #去掉字符串右边的无效字符
            ilength = len(string)
            for i in range(0,len(string)):
                if(string[len(string)-i-1] == ' ' or string[len(string)-i-1] == '\t' or string[len(string)-i-1] == '\n' or string[len(string)-i-1] == '\r'):
                    continue
                else:
                    string = string[0:len(string)-i]
                    break
            if(len(string)==0):
                return ""
            return string
    
########################################################################
###@brief : XmlNode类定义(操作.xml类型文件中的一个结点)
###@param : void
###@return: void
########################################################################
class XmlNode(Base):
    def __init__(self,name = ""):
        Base.__init__(self)
        self.NodeName    = name #结点名称
        self.RecordList  = []     #记录
        self.SubNodeList = []     #子结点
    def __getitem__(self,value):
        '''for object[value]!'''
        if(isinstance(value,int)):
            return self.SubNodeList[value]
        elif(isinstance(value,str)):
            return self.RecordList[int(value)]
    def __len__(self):
        '''for len(object)!'''
        return len(self.SubNodeList)
    def __eq__(self,obj):
        return self.NodeName == obj.NodeName
    ########################################################################
    ###@brief : 获取子结点的个数
    ###@param : void
    ###@return: int   子结点的个数 
    ########################################################################
    def GetSubNodeCount(self):
        return len(self.SubNodeList)
    ########################################################################
    ###@brief : 获取记录的行数
    ###@param : void
    ###@return: int   记录的行数
    ########################################################################
    def GetRecordCount(self):
        return len(self.RecordList)
    ########################################################################
    ###@brief : 添加结点或者记录
    ###@param : obj   [in]string-记录行；XmlNode对象--XmlNode对象
    ###@return: int   记录的行数
    ########################################################################
    def append(self,obj):
        if(isinstance(obj,str)):
            self.RecordList.append(obj)
        elif(isinstance(obj,XmlNode)):
            self.SubNodeList.append(obj)
    ########################################################################
    ###@brief : 查找子结点是否存在
    ###@param : nodeName   [in]子结点名称
    ###@return: int   结点序号
    ########################################################################
    def findNode(self,nodeName):
        for i in range(0,len(self.SubNodeList)):
            if(cmp(self.SubNodeList[i].NodeName,nodeName) == 0):
                return i
        return -1
    ########################################################################
    ###@brief : 查找记录是否存在
    ###@param : key   [in]记录行键值，唯一标识一条记录
    ###@return: int   记录的序号
    ########################################################################
    def findRecord(self,key):
        for i in range(0,len(self.RecordList)):
            record = self.RecordList[i]
            #跳过注释
            if(record.find('<!--') != -1 and record.find('/-->') != -1):
                continue
            if(record.find(key) != -1):
                return  i
        return -1
    ########################################################################
    ###@brief : 更新item值
    ###@param : key   [in]记录行键值，唯一标识一条记录
    ###@param : item  [in]要更新的项
    ###@param : value [in]新的值
    ###@return: int   0----成功，-1----失败
    ########################################################################
    def UpdateItemValue(self,key,item,value):
        Recordindex = -1#记录序号
        for i in range(0,len(self.RecordList)):
            record = self.RecordList[i]
            #跳过注释
            if(record.find('<!--') != -1 and record.find('/-->') != -1):
                continue
            if(record.find(key) != -1):
                Recordindex = i
                break
        if(Recordindex == -1):
            self.set_errinfo(-1,"Can't find key[%s]-item[%s]"%(key,item))
            return -1
        #取出记录
        record = self.RecordList[Recordindex]
        head   = record[:record.find('<')+1]#保存头部
        tail   = record[record.find('/>'):] #保存尾部
        string = record[record.find('<')+1:record.find('/>')]
        #分解记录,分割符为空格
        itemlist = string.split()
        itemindex = -1#item序号
        for i in range(0,len(itemlist)):
            if(itemlist[i].find(item) != -1):
                itemindex = i
                break
        if(itemindex == -1):
            self.set_errinfo(-1,"Can't find key[%s]-item[%s]!\n"%(key,item))
            return -1
        #取出item
        item = itemlist[itemindex]
        #分解item,分隔符为'='
        strlist = item.split('=')
        #赋值
        strlist[1] = "\"%s\"" % value
        #拼接
        item = '='.join(strlist)
        itemlist[itemindex] = item
        record = head+' '.join(itemlist)+tail
        self.RecordList[Recordindex] = record
        return 0
    ########################################################################
    ###@brief : 查找子结点，返回符合条件的子结点的个数
    ###@param : name   [in]子节点名称
    ###@return: []  子结点序号列表
    ########################################################################
    def SameSubCount(self,name):
        indexlist = []
        for i in range(0,len(self.SubNodeList)):
            if(cmp(self.SubNodeList[i].NodeName,name) == 0):
                indexlist.append(i)
        return indexlist
    ########################################################################
    ###@brief : 获取item值
    ###@param : key   [in]记录行键值，唯一标识一条记录
    ###@param : item  [in]要更新的项
    ###@param : value [in]新的值
    ###@return: int   0----成功，-1----失败
    ########################################################################
    def GetItemValue(self,key,item):
        recordindex = -1#record序号
        for i in range(0,len(self.RecordList)):
            record = self.RecordList[i]
            #跳过注释
            if(record.find('<!--') != -1 and record.find('/-->') != -1):
                continue
            if(record.find(key) != -1):
                recordindex = i
                break
        if(recordindex == -1):
            self.set_errinfo(-1,"Can't find key[%s]-item[%s]"%(key,item))
            return (-1,self.reason)
        #取出记录
        record = self.RecordList[recordindex]
        string = record[record.find('<')+1:record.find('/>')]
        #分解记录,分割符为空格
        itemlist = string.split()
        itemindex = -1#item序号
        for i in range(0,len(itemlist)):
            if(itemlist[i].find(item) != -1):
                itemindex = i
                break
        if(itemindex == -1):
            self.set_errinfo(-1,"Can't find key[%s]-item[%s]"%(key,item))
            return (-1,self.reason)
        #取出item
        item = itemlist[itemindex]
        #分解item,分隔符为'='
        strlist = item.split('=')
        value   = strlist[1]
        value   = value[1:len(value)-1]
        return (0,value)
    ########################################################################
    ###@brief : 清理函数
    ###@param : void
    ###@return: void
    ########################################################################
    def clear(self):
        self.RecordList  = []
        self.SubNodeList = []


########################################################################
###@brief : xml类定义(操作.xml类型文件)
###@param : void
###@return: void
########################################################################
class  XmlTree(Base):
    def __init__(self):
        Base.__init__(self)
        self.root   = XmlNode("root")
    def __str__(self):
        '''for str(object)'''
        return self.__PrintNode(self.root,-1)  
    def clear(self):
        self.root.clear()
    ########################################################################
    ###@brief : 获取整形值
    ###@param : key      [in] xml结点键值串
    ###@param : record   [in] 记录行键值
    ###@param : item     [in] 项
    ###@param : defint   [in] 缺省值,不存在则返回该值,默认为0
    ###@return: int  整数
    ########################################################################
    def GetInteger(self,key,record,item,defint = 0):
        (state,value) = self.GetValue(key,record,item)
        if(state == -1):
            return defint
        else:
            return int(value)
    ########################################################################
    ###@brief : 获取string值
    ###@param : key      [in] xml结点键值串
    ###@param : record   [in] 记录行键值
    ###@param : item     [in] 项
    ###@param : defstr   [in] 缺省值,不存在则返回该值,默认为0
    ###@return: string  字符串
    ########################################################################
    def GetString(self,key,record,item,defstr = 0):
        (state,value) = self.GetValue(key,record,item)
        if(state == -1):
            return defstr
        else:
            return value
    ########################################################################
    ###@brief : 获取值
    ###@param : key    [in] xml结点键值串，比如StreamServer.Server.slave
    ###@param : record [in] 记录行键值，比如slave
    ###@param : item   [in] 项，比如value
    ###@return: value  字符串或者整数
    ########################################################################
    def GetValue(self,key,record,item):
        (state,obj) = self.SearchNode(self.root,key)
        if(state == 0):
            return obj.GetItemValue(record,item)
        else:
            self.set_errinfo(-1,"Can't find <%s> !\n" % key)
            return (-1,self.reason)
    ########################################################################
    ###@brief : 更新值
    ###@param : key    [in] xml结点键值串，比如StreamServer.Server.slave
    ###@param : record [in] 记录行键值，比如slave
    ###@param : item   [in] 项，比如value
    ###@param : value  [in] 新值
    ###@return: int    0----成功，-1----失败
    ########################################################################
    def update(self,key,record,item,value):
        recordlist = record.split('|')
        itemlist   = item.split('|')
        valuelist  = str(value).split('|')
        if(len(recordlist) != len(itemlist) or len(recordlist) != len(valuelist) or len(valuelist) != len(itemlist)):
            self.set_errinfo(-1,"Data error:<%s> [%s][%s][%s]!\n"%(key,record,item,value))
            return -1
        (state,obj) = self.SearchNode(self.root,key)
        if(state == 0):
            for i in range(0,len(recordlist)):
                iRet = obj.UpdateItemValue(recordlist[i],itemlist[i],valuelist[i])
                if(iRet == -1):
                    self.set_errinfo(-1,obj.reason)
                    return -1
        else:
            self.set_errinfo(-1,"Can't find [%s]!\n"%key)
            return -1
    ########################################################################
    ###@brief : 递归查找结点
    ###@param : node   [in] 要查找的xml结点，通常从root结点开始
    ###@param : key    [in] xml结点键值串
    ###@param : item   [in] 项，比如value
    ###@param : value  [in] 新值
    ###@return: (state,object) state:0-成功，-1-失败
    ########################################################################
    def SearchNode(self,node,key):
        NodeName  = ""#临时保存键值
        keylist   =  key.split('.')
        if(len(keylist) == 0):
            return (0,node)
        indexlist =  node.SameSubCount(keylist[0])
        NodeName = keylist[0]
        del keylist[0] #键值已使用，删除掉
        #未找到子结点
        if(len(indexlist) == 0):
            #存在含有NodeName的记录，返回当前结点
            #不存在，返回失败
            if(node.findRecord(NodeName) != -1):
                return (0,node)
            else:
                return (-1,"")
        #存在唯一的一个子结点
        elif(len(indexlist) == 1):
            #key序列已结束，返回找到的子结点
            if(len(keylist) == 0):
                return (0,node[indexlist[0]])
            #key序列未结束，调用SearchNode继续查找
            elif(len(keylist) > 0):
                return self.SearchNode(node[indexlist[0]],'.'.join(keylist))
        #多个子node名称一样
        elif(len(indexlist) > 1):
            #没有后续key,无法找到匹配子结点
            if(len(keylist) == 0):
                print 'error,too many node named[%s]!\n' % NodeName
                return (-1,"")
            #优先查询子node的子node中是否存在名称和后续key(keylist[0])一致的
            for i in range(0,len(indexlist)):
                if(node[indexlist[i]].findNode(keylist[0]) != -1):
                    return self.SearchNode(node[indexlist[i]],'.'.join(keylist))
            #未找到,则查询子node中是否存在记录含有字符串keylist[0]
            for i in range(0,len(indexlist)):
                if(node[indexlist[i]].findRecord(keylist[0]) != -1):
                    del keylist[0]#删除掉匹配键
                    return self.SearchNode(node[indexlist[i]],'.'.join(keylist))
            #未找到匹配子节点
            return (-1,"")
    ########################################################################
    ###@brief : 解析xml文件
    ###@param : file   [in] 要查找的xml结点，通常从root结点开始
    ###@return: int    0----成功，-1----失败
    ########################################################################
    def parse(self,file):
        try:
            self.clear()
            self.__filename=file
            readfile = open(self.__filename,"r")
            self.__BuildXmlTree(readfile,self.root)
            readfile.close()
            self.set_errinfo()
            return 0
        except IOError,e:
            #print e
            reason=str(e)
            if (reason[0] == '<'):
                reason = reason[1:len(reason)-1]
            self.set_errinfo(-1,reason)
            return -1
    ########################################################################
    ###@brief : 递归创建xml树
    ###@param : file       [in] xml配置文件
    ###@param : patentnode [in] 父结点，通常从root结点开始
    ###@return: void
    ########################################################################
    def __BuildXmlTree(self,file,patentnode):
        CurrentNode   = patentnode
        line = file.readline()
        while line:
            (state,string)=self.AnalysisLine(line)
            #新结点
            if(state == 1):
                #创建xml结点
                NewNode  = XmlNode(string)
                CurrentNode.append(NewNode)
                #递归调用
                self.__BuildXmlTree(file,NewNode)
            elif(state == 2):
                #此结点读取完毕，返回
                if(string == CurrentNode.NodeName):
                    return
            elif(state == 3 or state == 4):
                CurrentNode.append(line)
            line = file.readline()
    ########################################################################
    ###@brief : 递归打印xml文件
    ###@param : patentnode [in] 要打印的结点，通常从root结点开始
    ###@return: string  字符串
    ########################################################################
    def __PrintNode(self,node,level):
        string = ""
        prefix = ""
        #子结点缩进
        if(node.NodeName != 'root'):
            for i in range(0,level):
                prefix = prefix + '    '
            string = string +prefix+'<'+node.NodeName+'>\n'
        for i in range(0,node.GetSubNodeCount()):
            string = string +self.__PrintNode(node[i],level+1)
        for i in range(0,node.GetRecordCount()):
            string = string +node[str(i)]
        if(node.NodeName != 'root'):
            string = string +prefix+'</'+node.NodeName+'>\n'
        return string
    ########################################################################
    ###@brief : 分析记录行
    ###@param : string [in] 从文件中读取的一行记录
    ###@return: string  字符串
    ########################################################################
    def AnalysisLine(self,string):
        if(len(string)==0):
            return(0,"")
        #比如<sys>
        if (string.find('<') != -1 and string.find('>') != -1 
            and string.find('</') == -1 and string.find('/>') == -1 
              and string.find('<!--') == -1 and string.find('/-->') == -1):
            string = string[string.find('<')+1:string.find('>')]
            name = string.split()[0]
            return (1,name)
        #比如</sys>
        elif(string.find('</') != -1 and string.find('>') != -1):
            string = string[string.find('</')+2:string.find('>')]
            name = string.split()[0]
            return (2,name)
        #比如<section name="dsn" value="C3" />
        elif(string.find('<') != -1 and string.find('/>') != -1):
            return (3,"")
        #比如<!--???/-->
        if (string.find('<!--') != -1 and string.find('/-->') != -1):
            string = string[string.find('<!--')+1:string.find('/-->')]
            return (4,"")
        else:
            return (0,"")


########################################################################
###@brief : 配置文件控制类定义
###@param : file_type [in] 配置文件类型,.config or .xml,缺省.config
###@param : ch        [in] 注释符，for .config
###@return: void
########################################################################
class FileControl(Base):
    def __init__(self,file_type = ".config",ch = '#'):
        Base.__init__(self)
        self.filetype   = file_type#文件类型
        self.__filename = ""       #配置文件名称
        self.AddFlag    = False    #添加标志
        if(self.filetype == '.config'):
            self.FileObject = ConfigFile()
        elif(self.filetype == '.xml'):
            self.FileObject = XmlTree()
        self._CodeTools = TranslateTool()#加密工具
    def __len__(self):
        '''for len(object)'''
        return len(self.FileObject)
    def __str__(self):
        '''for str(object)'''
        return str(self.FileObject)
    ########################################################################
    ###@brief : 清理函数
    ###@param : void
    ###@return: void
    ########################################################################
    def clear(self):
        self.FileObject.clear()
        self.__filename=""
    ########################################################################
    ###@brief : 设置是否加密标志
    ###@param : void
    ###@return: void
    ########################################################################
    def SetEncodeFlag(self,flag):
        self.flag = flag
    ########################################################################
    ###@brief : 设置添加标志
    ###@param : flag   [in]bool类型，false--不存在不添加，true--不存在添加
    ###@return: void
    ########################################################################
    def SetAddFlag(self,flag):
        self.AddFlag = flag
        if (self.filetype == '.config'):
            self.FileObject.SetAddFlag(flag)
    ########################################################################
    ###@brief : 获取文件名后缀
    ###@param : string   [in]文件名
    ###@return: string   后缀
    ########################################################################
    def __GetSuffix(self,string):
        iIndex = string.rfind('.')
        if(iIndex != -1):
            return string[iIndex:]
        else:
            return ""
    ########################################################################
    ###@brief : 设置读取文件类型.config 或 .xml
    ###@param : string   [in]文件类型
    ###@return: void
    ########################################################################
    def SetFileType(self,filetype):
        self.filetype = filetype
    ########################################################################
    ###@brief : 获取文件类型
    ###@param : void
    ###@return: string 文件类型，.config or .xml
    ########################################################################
    def GetFileType(self):
        return self.filetype
    ########################################################################
    ###@brief : 获取文件名称
    ###@param : void
    ###@return: string 文件名称
    ########################################################################
    def GetFileName(self):
        return self.__filename
    ########################################################################
    ###@brief : 获取整形值
    ###@param : cube   [in] cube名称或键值串
    ###@param : record [in] 记录行键值
    ###@param : item   [in] 项，for.xml
    ###@param : defint [in] 缺省值
    ###@return: int  
    ########################################################################
    def GetInteger(self,cube,record,item='',defint = 0):
        if(self.filetype == '.config'):
            return self.FileObject.GetInteger(cube,record,defint)
        elif(self.filetype == '.xml'):
            return self.FileObject.GetInteger(cube,record,item,defint)
    ########################################################################
    ###@brief : 获取字符串值
    ###@param : cube   [in] cube名称或键值串
    ###@param : record [in] 记录行键值
    ###@param : item   [in] 项，for.xml
    ###@param : defstr [in] 缺省值
    ###@return: string  
    ########################################################################
    def GetString(self,cube,record,item='',defstr = ""):
        if(self.filetype == '.config'):
            return self.FileObject.GetString(cube,record,defstr)
        elif(self.filetype == '.xml'):
            return self.FileObject.GetString(cube,record,item,defstr)
    ########################################################################
    ###@brief : 获取值
    ###@param : cube   [in] cube名称或键值串
    ###@param : record [in] 记录行键值
    ###@param : item   [in] 项，for.xml
    ###@return: string  or int
    ########################################################################
    def GetValue(self,cube,record,item=''):
        if(self.filetype == '.config'):
            return self.FileObject.GetValue(cube,record)
        elif(self.filetype == '.xml'):
            return self.FileObject.GetValue(cube,record,item)
    ########################################################################
    ###@brief : 更新值
    ###@param : cube   [in] cube名称或键值串
    ###@param : record [in] 记录行键值
    ###@param : item   [in] 项，for.xml
    ###@param : value  [in] 新值
    ###@return: string  or int
    ########################################################################
    def update(self,key,record,value,item=''):
        if (cmp(self.filetype,".config") == 0):
            (state,string)=self.FileObject.GetValue("COMMON","Shadow")
            if(string == '\0'):
                (state,string)=self.FileObject.GetValue("COMMON","ShadowPassword")
            if(state == 0 and (string == 'Y' or string == 'y')):
                (state,string) = self._CodeTools.encrypt(str(value))
                if(state == 0):
                    value = string
                else:
                    self.set_errinfo(-1,string)
                    return -1
            iRet = self.FileObject.update(key,record,value)
            if(iRet == -1):
                self.set_errinfo(-1,self.FileObject.reason)
            else:
                self.set_errinfo()
        elif(cmp(self.filetype,".xml") == 0):
            if(self.flag==1):
                valuelist = str(value).split('|')
                for i in range(0, len(valuelist)):              
                    (state,string) = self._CodeTools.encrypt(str(valuelist[i]))
                    if(state == 0):
                        valuelist[i] = "!!%s!!" % string
                    else:
                        self.set_errinfo(-1,string)
                        return -1
                value = '|'.join(valuelist)
            iRet = self.FileObject.update(key,record,item,value)
            if(iRet == -1):
                self.set_errinfo(-1,self.FileObject.reason)
            else:
                self.set_errinfo()
        return iRet
    ########################################################################
    ###@brief : 读取配置文件
    ###@param : file     [in] 配置文件名称
    ###@param : autotype [in] 自动识别文件标识，1自动识别
    ###@return: int    0----成功，-1----失败
    ########################################################################
    def ReadConfig(self,file,autotype = 0):
        #加入自动识别后缀功能
        self.__filename = file
        if( len(self.__filename) == 0 ):
            self.set_errinfo(-1,"Configure file name is NULL!")
            return -1
        if(autotype == 1):
            suffix = self.__GetSuffix(file)
            if(cmp(suffix,'.config') == 0 or cmp(suffix,'.ini') == 0):
                self.filetype = '.config'
                self.FileObject = ConfigFile()
            elif(cmp(suffix,'.xml') == 0):
                self.filetype = '.xml'
                self.FileObject = XmlTree()
            else:
                #打印错误日志
                self.set_errinfo(-1,"The format of file[%s] can't be recognized!" % self.__filename)
                return -1
        iRet = self.FileObject.parse(file)
        if(iRet == -1):
            self.set_errinfo(-1,self.FileObject.reason)
            return -1
        else:
            return 0
    ########################################################################
    ###@brief : 写配置文件
    ###@param : void
    ###@return: int  0----成功，-1----失败
    ########################################################################
    def WriteConfig(self):
        '''write config to file'''
        try:
            #先备份配置文件
            srcfile=self.__filename
            desfile=srcfile+".bak"
            shutil.copy(srcfile,desfile)
            #覆盖文件
            if(cmp(self.filetype,'.config') == 0):
                self.FileObject.write()
            else:
                writefile = open(srcfile,"w")
                writefile.write(str(self.FileObject))
                writefile.close()
            #转换为unix格式
            temp_config_file = srcfile + ".temp"
            modify_shell_command = "cat  " + srcfile + " | perl -pe '~s/\r//g' >" + temp_config_file
            save_shell_command = "mv " + temp_config_file + "   " + srcfile
            os.system(modify_shell_command)
            os.system(save_shell_command)
            self.set_errinfo()
            return 0
        except IOError,e:
            #print e
            reason=str(e)
            if (reason[0] == '<'):
                reason = reason[1:len(reason)-1]
            self.set_errinfo(-1,reason)
            return -1


########################################################################
###@brief : 获取数据库密码类定义
###@param : void
###@return: void
########################################################################
class WebClient(Base):
    ''' Get DB password from web server!'''
    def __init__(self):
        Base.__init__(self)
        self.encode_tools = TranslateTool()
        self.ParamDic = Dictionary()
        self.__Result = ConfigFile()
        self.__Result.SetAddFlag(True)
    ########################################################################
    ###@brief : 设置参数字典,
    ###@param : paramdic [in] 参数字典
    ###@return: void
    ########################################################################
    def SetParamDic(self,paramdic):
        self.ParamDic = paramdic
        self.url      = self.ParamDic['url']
        self.dbtype   = self.ParamDic['dbtype']
        self.dbmark   = self.ParamDic['dbmark']
        self.pmark    = self.ParamDic['pmark']
        self.type     = self.ParamDic['dbtype']
        self.version  = self.ParamDic.GetIntegerValue('version',1)#缺省非分布式
    ########################################################################
    ###@brief : 获取网络访问结果
    ###@param : paramdic [in] 参数字典
    ###@return: void
    ########################################################################
    def GetResult(self):
        return self.__Result
    ########################################################################
    ###@brief : 拼装密钥
    ###@param : void
    ###@return: 密钥串
    ########################################################################
    def GetKey(self):
        time_now = time.strftime('%Y%m%d%H%M%S',time.localtime(time.time()))
        return  'ZSmartInnerData_DB_PWD_TOKEN_'+time_now
    ########################################################################
    ###@brief : 创建表单
    ###@param : void
    ###@return:{} 表单
    ########################################################################
    def CreateForm(self):
        form ={}
        #获取密钥
        (code,string) = self.encode_tools.encrypt(self.GetKey())
        if(code == -1):
            self.set_errinfo(-1,string);
            return -1
        else:
            key = string
        #拼接表单
        if(self.version == 1):
            form = {
                'TOKEN':key,
                'DB_NAME':self.type,
                'ROUTING_CODE':'',
                'MODULE_NAME':self.pmark
            }
        elif(self.version == 2):
            form = {
                'TOKEN':key,
                'DB_NAME':self.type,
                'ROUTING_CODE':self.dbmark,
                'MODULE_NAME':self.pmark
            }
        else:
            self.set_errinfo(-1,"Uknown version [%s]!\n" % self.version);
        return form
    ########################################################################
    ###@brief : 从web server上获取密码
    ###@return: int  0--成功;1--失败
    ########################################################################
    def GetDBPassword(self):
        '''get db password from web server.'''
        #设置超时时间
        socket.setdefaulttimeout(5)
        #生成表单
        form = self.CreateForm()
        #表单值编码
        postdata=urllib.urlencode(form);
        try:
            #向服务器发出请求
            req = urllib2.Request(self.url,postdata)
            #打开url
            response = urllib2.urlopen(req)
            #读取返回数据
            retData = response.read()
            state   = self.Analyze(retData)
            if(state == 0):
                self.set_errinfo();
                return 0
            else:
                return -1
        except ValueError, e:
            reason=str(e)
            if (reason[0] == '<'):
                reason = errorinfo[1:len(errorinfo)-1]
            self.set_errinfo(-1,reason)
            return -1
        except urllib2.HTTPError, e:
            reason=str(e)
            if (reason[0] == '<'):
                reason = errorinfo[1:len(errorinfo)-1]
            self.set_errinfo(-1,reason)
            return -1
        except urllib2.URLError, e:
            errorinfo=str(e)
            reason=str(e)
            if (reason[0] == '<'):
                reason = errorinfo[1:len(errorinfo)-1]
            self.set_errinfo(-1,reason)
            return -1
    ########################################################################
    ###@brief : 解析返回的字符串，并进行合法性判断
    ###@param : string [in] web server返回的字符串
    ###@return: int   0--成功，-1失败;
    ########################################################################
    def Analyze(self,string):
        #清除结果
        self.__Result.clear()
        if(len(string) == 0):
            self.set_errinfo(-1,"Web Server return a NULL string!\n");
            return -1
        elif(len(string) > 61 and self.version == 1):
            #oracle数据库的用户名和密码的最大长度均为30，因此返回的字符串最大长度为61
            self.set_errinfo(-1,"Unknown string format:string is too long!Check url!\n");
            return -1
        elif(len(string) > 1240 and self.version == 2):
            #oracle数据库的用户名和密码的最大长度均为30，假设分库个数不操过20个，则长度最长为1240
            self.set_errinfo(-1,"Unknown string format:string is too long!Check url!\n");
            return -1
        #密钥验证失败
        elif(cmp(string,"0001") == 0):
            self.set_errinfo(-1,"Authentication failed!")
            return -1
        #数据库不存在
        elif(cmp(string,"0002") == 0):
            self.set_errinfo(-1,"The DB_TYPE[%s] does not exist!" % self.type)
            return  -1
        else:
            if(self.version == 1):
                datalist = string.split(':')
                if(len(datalist)!= 2):
                    self.set_errinfo(-1,"Invalid data format[%s]!\n"% string)
                    return -1
                else:
                    self.__Result.update('noraml','USER_NAME',datalist[0])
                    self.__Result.update('noraml','PASS_WORD',datalist[1])
                    return 0
            elif(self.version == 2):
                baselist = string.split('|')
                for i in range(0,len(baselist)):
                    dblist=baselist[i].split('#')
                    if(len(dblist)!= 2):
                        self.set_errinfo(-1,"Invalid data format[%s]!\n"% baselist[i])
                        return -1
                    cube = dblist[0]
                    userlist = dblist[1].split(':')
                    if(len(userlist)!= 2):
                        self.set_errinfo(-1,"Invalid data format[%s]!\n"% dblist[1])
                        return -1
                    self.__Result.update(cube,'USER_NAME',userlist[0])
                    self.__Result.update(cube,'PASS_WORD',userlist[1])
                return 0

########################################################################
###@brief : 配置文件池类定义
###@param : size   [in] 本脚本配置中module的个数
###@param : index  [in] 执行指定的module序号,缺省全部执行
###@return: void
########################################################################
class ConfigureFilePool(Base):
    def __init__(self,size,index = -1):
        Base.__init__(self)
        self.__size  =size
        self.__index = index
        self.AddFlag = False
        self._pool = []
        self.__loghandle =GetLogger()
    def __setitem__(self,index,value):
        '''for object[index]=value'''
        self._pool[index]=value
    def __getitem__(self,index):
        '''for object[index]'''
        return self._pool[index]
    def __str__(self):
        '''for str(object)'''
        string = "ConfigureFilePool:\n"
        for i in range(0, len(self._pool)):
            string = string + "File_Name:%s\n" % self._pool[i].GetFileName()
            string = string + "content:\n\n"
            string = string + str(self._pool[i])+"\n"
        return string
    ########################################################################
    ###@brief : 设置添加标志
    ###@param : flag [in] bool类型，false--不存在不添加，true--不存在添加
    ###@return: void
    ########################################################################
    def SetAddFlag(self,flag):
        self.AddFlag = flag
    ########################################################################
    ###@brief : 根据配置文件名获取在文件池中的序号
    ###@param : filename [in] 配置文件名
    ###@return: int   序号
    ########################################################################
    def GetFileIndex(self,filename):
        index  = -1  #序列号
        reason = ""  #错误原因
        for i in range(0, len(self._pool)):
            if (cmp(self._pool[i].GetFileName(),filename)==0):
                index = i
                break
        if(index == -1):
            reason = 'file ['+filename +'] not found!\n'
            self.set_errinfo(-1,reason)
            return (-1,reason)
        return (index, "")
    ########################################################################
    ###@brief : 初始化池
    ###@param : scriptconfig [in] 保存脚本配置文件的ConfigFile类对象
    ###@return: void  初始化失败的module会打印错误日志
    ########################################################################
    def InitPool(self,scriptconfig):
        self.__loghandle.debug('ConfigureFilePool::InitPool():Begin!')
        self.__ScriptConfig = scriptconfig
        count = 0
        if(self.__index != -1):
            state = self.__ReadModule(self.__index)
            if(state == 0):
                count = count+1
        else:
            for i in range(0, self.__size):
                state = self.__ReadModule(i+1)
                if(state == 0):
                    count = count+1
        self.__loghandle.debug('ConfigureFilePool::InitPool():Finished!')
        if( count == 0 ):
            self.__loghandle.debug('ConfigureFilePool::InitPool():Finished! return = [false]')
            return -1
        elif( count > 0 ):
            self.__loghandle.debug('ConfigureFilePool::InitPool():Finished! return = [true]')
            return 0
    ########################################################################
    ###@brief : 写配置文件
    ###@param : void
    ###@return: void
    ########################################################################
    def WriteConfig(self):
        for i in range(0, len(self._pool)):
            state = self._pool[i].WriteConfig()
            if(state == -1):
                self.__loghandle.error('ConfigureFilePool::WriteConfig():%s file name = [%s]'%(self._pool[i].reason,self._pool[i].GetFileName()))
    ########################################################################
    ###@brief : 读取指定模块的配置信息
    ###@param : index [in] 指定模块编号
    ###@return: int  0--成功，-1失败
    ########################################################################
    def __ReadModule(self,index):
        UNnormalFlag = 0 #模块异常标志
        sSuffix = ""     #后缀  
        #拼接cube名称
        cube="%s%d" %('module',index)
        #获取配置文件名
        file=self.__ScriptConfig.GetString(cube,'file')
        #配置文件名为空
        if (len(file) == 0):
            reason = "Setting_Error:[%s] file = NULL, file name can't be NULL!" % cube
            self.__loghandle.error('ConfigureFilePool::__ReadModule():'+reason)
            self.__setstate(cube)
            self.set_errinfo(-1,reason)
            return -1
        else:            
            (iIndex,string) = self.GetFileIndex(file)
            #配置文件已在池中,设置状态和序号,然后返回
            if(iIndex != -1):
                self.__setstate(cube,1,iIndex)
                return 0
        #获取保存方式
        save_method=self.__ScriptConfig.GetInteger(cube,'savemethod',2)
        #保存方式配置错误
        if(save_method != 1 and save_method != 2):
            reason = "Setting_Error:[%s] savemethod = %d is invalid!" %(cube,save_method)
            self.__loghandle.error('ConfigureFilePool::__ReadModule():'+reason)
            self.__setstate(cube)
            self.set_errinfo(-1,reason)
            return -1
        if(save_method == 1):
            self.__setstate(cube,0)
            return 0
        #配置正常
        CurModule = FileControl()
        CurModule.SetAddFlag(self.AddFlag)
        state = CurModule.ReadConfig(file,1)#自动识别文件
        if(state == -1):
            self.__loghandle.error('ConfigureFilePool::__ReadModule():%s' % CurModule.reason)
            self.__setstate(cube)
            return -1;
        else:
            self._pool.append(CurModule)
            self.__setstate(cube,1,len(self._pool)-1)
            return 0
    ########################################################################
    ###@brief : 保存状态信息到模块
    ###@param : cube     [in] 指定模块
    ###@param : state    [in] 配置状态
    ###@param : fileindex[in] 池中文件序号
    ###@return: void
    ########################################################################
    def __setstate(self,cube,state = -1,fileindex = -1):
        #配置无效
        if(state == -1):
            self.__ScriptConfig.update(cube,'state',-1)
            self.__ScriptConfig.update(cube,'fileindex',-1)
        #配置正常,保存方式1
        elif(state == 0):
            self.__ScriptConfig.update(cube,'state',0)
            self.__ScriptConfig.update(cube,'fileindex',-1)
        #配置正常,保存方式2
        elif(state == 1):
            self.__ScriptConfig.update(cube,'state',0)
            self.__ScriptConfig.update(cube,'fileindex',fileindex)

########################################################################
###@brief : 操作类定义
###@param : void
###@return: void
########################################################################
class Operate:
    def __init__(self):
        self._url     = ''     #url
        self._count   = 1      #module count
        self._index   = -1     #index
        self._addflag = False  #
        self._WebClient    = WebClient()    #
        self.__paramarray  = Dictionary()   #参数字典
        self.__loghandle   = GetLogger()    #日志句柄
        self._ScriptConfig = ConfigFile()
        self._ScriptConfig.SetAddFlag(True)
    def WriteConfig(self):
        self._ConfigureFilePool.WriteConfig()
    ########################################################################
    ###@brief : 设置参数字典,从命令行获取参数，调用该函数传入
    ###@param : array    [in] 参数字典
    ###@return: void
    ########################################################################
    def SetParamArray(self,array):
        self.__paramarray.clear()
        for key in array.keys():
            self.__paramarray[key]=array[key]
    ########################################################################
    ###@brief : 初始化,从脚本配置文件中读取命令参数,可一次执行n次get密码动作
    ###@param : file    [in] 本脚本的配置文件
    ###@return: void
    ########################################################################
    def Initialize(self,file='./GetDBPassword.config'):
        self.__loghandle.debug('Operate::Initialize():Begin!')
        #读取脚本配置文件
        iRet = self._ScriptConfig.parse(file)
        if(iRet == -1):
            self.__loghandle.error('Operate::Initialize():read script configure file [%s] error [%s]'% (file,self._ScriptConfig.reason))
            self.__loghandle.debug('Operate::Initialize():Finished! return = [false]')
            return -1

        #读取全局配置
        self._url     = self._ScriptConfig.GetString('COMMON','url')
        self._count   = self._ScriptConfig.GetInteger('COMMON','count',0)
        self._index   = self._ScriptConfig.GetInteger('COMMON','index',-1)
        self._addflag = self._ScriptConfig.GetInteger('COMMON','addflag',0) == 1
        
        #全局配置进行正确性检验
        iRet = self.__CheckGlobalParam()
        if (iRet == -1):
            self.__loghandle.debug('Operate::Initialize():Finished! return = [false]')
            return -1
        #创建配置文件池
        self._ConfigureFilePool = ConfigureFilePool(self._count,self._index)
        self._ConfigureFilePool.SetAddFlag(self._addflag)
        iRet = self._ConfigureFilePool.InitPool(self._ScriptConfig)
        if(iRet == -1):
            self.__loghandle.debug('Operate::Initialize():Finished! return = [false]')
            return -1
        self.__loghandle.debug('Operate::Initialize():Finished! return = [true]')
        return 0
    ########################################################################
    ###@brief : 执行动作,执行指定或者所有module中配置的get密码动作
    ###@param : void
    ###@return: void
    ########################################################################
    def Excute(self):
        self.__loghandle.debug('Operate::Excute():Begin!')
        if (self._index > 0):
            self.__ExcuteModule(self._index)
        else:
            for i in range(0,self._count):
                self.__ExcuteModule(i+1)
        #将配置文件写到磁盘
        self._ConfigureFilePool.WriteConfig()
        self.__loghandle.debug('Operate::Excute():Finished!')
    ########################################################################
    ###@brief : 执行第index个module
    ###@param : index    [in] module编号
    ###@return: void
    ########################################################################
    def __ExcuteModule(self,index):
        self.__loghandle.debug('[%s%d]:' %('module',index))
        #拼接cube名称
        module="%s%d" %('module',index)
        state = self._ScriptConfig.GetInteger(module,'state',-1)
        if (state == -1):
            self.__loghandle.error('setting errors found in [%s],skip the current module!' % module)
            self.__loghandle.debug('Finished! return = [false]')
            return -1
        #清空参数数组
        self.__paramarray.clear()
        #转存配置
        self.__paramarray['fileindex']= self._ScriptConfig.GetInteger(module,'fileindex',-1) #pool中编号
        self.__paramarray['flag']     = self._ScriptConfig.GetInteger(module,'flag',1)       #加密标志,0不加密,1加密
        self.__paramarray['addflag']  = self._ScriptConfig.GetInteger('COMMON','addflag',1)       #加密标志,0不加密,1加密
        self.__paramarray['method']   = self._ScriptConfig.GetInteger(module,'savemethod',2) #保存方式,1保存单个文件,2保存到配置文件中
        self.__paramarray['version']  = self._ScriptConfig.GetInteger(module,'version',1)    #版本,1非分布式,2分布式
        self.__paramarray['file']     = self._ScriptConfig.GetString(module,'file')          #配置文件名
        self.__paramarray['cube']     = self._ScriptConfig.GetString(module,'cube')          #cube
        self.__paramarray['record']   = self._ScriptConfig.GetString(module,'record')        #record名称
        self.__paramarray['item']     = self._ScriptConfig.GetString(module,'item')          #item
        self.__paramarray['dbtype']   = self._ScriptConfig.GetString(module,'dbtype')        #db_type
        self.__paramarray['dbmark']   = self._ScriptConfig.GetString(module,'dbmark')        #db_mark
        self.__paramarray['pmark']    = self._ScriptConfig.GetString(module,'productmark')   #product_mark
        self.__paramarray['url']      = self._ScriptConfig.GetString(module,'url')           #url
        self.__paramarray['way']      = 'file' #标明从文件中读取
        #执行动作
        state = self.ExcuteAction()
        if(state == -1):
            self.__loghandle.debug("Finished! return = [false]")
            return -1
        self.__loghandle.debug("Finished! return = [true]")
        return 0
    ########################################################################
    ###@brief : 执行module的动作
    ###@param : void
    ###@return: void
    ########################################################################
    def ExcuteAction(self):
        self.__loghandle.debug('Operate::ExcuteAction():Begin!')
        method    = self.__paramarray.GetIntegerValue('method',2)      #缺省保存到配置文件
        addflag   = self.__paramarray.GetIntegerValue('addflag',0) == 1#不存在是否添加，缺省不添加
        way       = self.__paramarray['way']
        fileindex = self.__paramarray['fileindex']
        file      = self.__paramarray['file']
        flag      = self.__paramarray.GetIntegerValue('flag',1)#缺省加密,对非.config文件有用
        #获取配置文件管理对象
        myConfig = FileControl()
        if(method == 2):
            #从配置文件池中获取配置文件管理对象
            if(cmp(way,'file') == 0):
                myConfig = self._ConfigureFilePool[fileindex]
            else:
                state = myConfig.ReadConfig(file,1)
                if(state == -1):
                    self.__loghandle.error('Operate::ExcuteAction():'+myConfig.reason)
                    self.__loghandle.debug('Operate::ExcuteAction():Finished !return = [false]')
                    return -1
        #设置相关标识
        myConfig.SetEncodeFlag(flag) #加密标志
        myConfig.SetAddFlag(addflag) #添加标志
        #参数检测
        state = self.CheckParam(myConfig)
        if(state == -1):
            self.__loghandle.debug('Operate::ExcuteAction():Finished !return = [false]')
            return -1
        #从服务器上获取用户名和密码
        self._WebClient.SetParamDic(self.__paramarray)#设置网络访问参数
        (state,reason) = self.__GetDBPasswd()
        if(state == 0):
            #保存到对应的配置文件
            state = 0
            if(method == 1):
                state = self.SaveToFileAlone()
            elif(method == 2):
                state = self.SaveToConfig(myConfig)
            if(state == -1):
                self.__loghandle.debug('Operate::ExcuteAction():Finished !return = [false]')
                return -1
            #从命令行读取参数，要单独保存
            if(cmp(way,'file') != 0 and method == 2):
                state = myConfig.WriteConfig()
                if(state == -1):
                    self.__loghandle.error('Operate::ExcuteAction():'+myConfig.reason)
                    self.__loghandle.debug('Operate::ExcuteAction():Finished !return = [false]')
                    return -1
        else:
            self.__loghandle.error('Operate::ExcuteAction():'+reason)
            self.__loghandle.debug('Operate::ExcuteAction():Finished !return = [false]')
            return -1
        self.__loghandle.debug('Operate::ExcuteAction():Finished !return = [true]')
        return 0
    ########################################################################
    ###@brief : 保存到配置文件中
    ###@param : void
    ###@return: state -1-失败,0-成功
    ########################################################################
    def SaveToConfig(self,Configobject):
        myConfig  = Configobject
        #读取相关参数
        version   = self.__paramarray.GetIntegerValue('version',1)#缺省使用版本1
        cube      = self.__paramarray['cube']
        record    = self.__paramarray['record']
        item      = self.__paramarray['item']
        #获取
        result    = self._WebClient.GetResult()
        #临时变量
        user_name = ""
        pass_word = ""
        state     = 0
        #非分布式,一次获取一个user_name:pass_word值对
        if(version == 1):
            user_name = result.GetString('noraml','USER_NAME')
            pass_word = result.GetString('noraml','PASS_WORD')
            if(cmp(myConfig.GetFileType(),'.config') == 0):
                state = self.__updateconfig(myConfig,cube,user_name,pass_word)
            elif(cmp(myConfig.GetFileType(),'.xml') == 0):
                state = self.__updatexml(myConfig,cube,record,item,user_name,pass_word)
            if(state == -1):
                return -1
        #分布式,一次获取n个user_name:pass_word值对,n>=1
        elif(version == 2):
            for i in range(0,len(result)):
                cube = result.GetCubeName(i)
                user_name = result.GetString(cube,'USER_NAME')
                pass_word = result.GetString(cube,'PASS_WORD')
                if(cmp(myConfig.GetFileType(),'.config') == 0):
                    state = self.__updateconfig(myConfig,cube,user_name,pass_word)
                elif(cmp(myConfig.GetFileType(),'.xml') == 0):
                    #此处.xml文件的处理还需要斟酌
                    state = self.__updatexml(myConfig,cube,record,item,user_name,pass_word)
                if(state == -1):
                    return -1
        return 0
    ########################################################################
    ###@brief : 保存到单独的文件中
    ###@param : void
    ###@return: state  -1-失败,0-成功
    ########################################################################
    def SaveToFileAlone(self):
        #读取相关参数
        file      = self.__paramarray['file']
        dbtype    = self.__paramarray['dbtype']
        flag      = self.__paramarray.GetIntegerValue('flag',1)    #缺省加密
        version   = self.__paramarray.GetIntegerValue('version',1)#缺省使用版本1
        result    = self._WebClient.GetResult()
        #临时变量
        content   = ""
        cube      = ""
        user_name = ""
        pass_word = ""
        #加密类对象
        DecodeTools = TranslateTool()
        if(version == 1):
            content = content + "[%s]\n" % dbtype
            user_name = result.GetString('noraml','USER_NAME')
            pass_word = result.GetString('noraml','PASS_WORD')
            if(flag==1):
                (code,string)  = DecodeTools.encrypt(user_name)
                if(code != 0):
                    self.__loghandle.error(string)
                    return -1
                else:
                    user_name = string
                (code,string) = DecodeTools.encrypt(pass_word)
                if(code != 0):
                    self.__loghandle.error(string)
                    return -1
                else:
                    pass_word = string
            content = content + "USER_NAME=%s\n"%user_name
            content = content + "PASS_WORD=%s\n"%pass_word
        elif(version == 2):
            for i in range(0,len(result)):
                cube    = result.GetCubeName(i)#此时cube值为分库名称
                content = content + "[%s]\n" % cube
                user_name = result.GetString(cube,'USER_NAME')
                pass_word = result.GetString(cube,'PASS_WORD')
                if(flag==1):
                    (code,string)  = DecodeTools.encrypt(user_name)
                    if(code != 0):
                        self.__loghandle.error(string)
                        return -1
                    else:
                        user_name = string
                    (code,string) = DecodeTools.encrypt(pass_word)
                    if(code != 0):
                        self.__loghandle.error(string)
                        return -1
                    else:
                        pass_word = string
                content = content + "USER_NAME=%s\n"%user_name
                content = content + "PASS_WORD=%s\n"%pass_word
        #写入文件
        file=codecs.open(file,"w","utf-8")
        file.write(content)
        file.close()
        return  0
    ########################################################################
    ###@brief : 更新.config类型的配置文件
    ###@param : void
    ###@return: void
    ########################################################################
    def __updateconfig(self,object,cube,user_name,pass_word):
        #更新用户名
        state = object.update(cube,'USER_NAME|user_name',user_name)
        if(state == -1):
            self.__loghandle.error(object.reason)
            return -1
        #更新密码
        state = object.update(cube,'PASS_WORD|pass_word',pass_word)
        if(state == -1):
            self.__loghandle.error(object.reason)
            return -1
        else:
            return 0
    ########################################################################
    ###@brief : 更新.xml类型的配置文件
    ###@param : void
    ###@return: void
    ########################################################################
    def __updatexml(self,object,cube,record,item,user_name,pass_word):
        state = object.update(cube,record,"%s|%s" %(user_name,pass_word),item)
        if(state == -1):
            self.__loghandle.error(object.reason)
            return -1
        else:
            return 0
    ########################################################################
    ###@brief : 从web上获取密码
    ###@param : void
    ###@return: void
    ########################################################################
    def __GetDBPasswd(self):
        #设置参数
        self._WebClient.SetParamDic(self.__paramarray)
        #获取密码
        state = self._WebClient.GetDBPassword()
        if(state == -1):
            return (-1,self._WebClient.reason)
        else:
            return (0,"")
    ########################################################################
    ###@brief : 检测全局配置是否正确
    ###@param : void
    ###@return: void
    ########################################################################
    def __CheckGlobalParam(self):
        error_flag = 0
        if (len(self._url)==0):
            reason = "Operate::__CheckGlobalParam():[COMMON] URL=NULL,may be not good!"
            self.__loghandle.warning(reason)
        if (self._count <= 0):
            error_flag = 1
            reason = "Operate::__CheckGlobalParam():setting error! [COMMON] COUNT = "+str(self._count)+" is invalid! it Must >0!"
            self.__loghandle.error(reason)
        if (self._index > self._count):
            error_flag = 1
            reason = "Operate::__CheckGlobalParam():setting error! [COMMON] INDEX = "+str(self._index)+" is invalid! it Must <=COUNT!"
            self.__loghandle.error(reason)
        if (error_flag == 0):
            return 0
        else:
            return -1
    ########################################################################
    ###@brief : 参数检测,参数保存在参数字典中
    ###@param : void
    ###@return: void
    ########################################################################
    def CheckParam(self,configobject):
        myConfig  = configobject#当前处理的配置文件
        file      = self.__paramarray['file']
        url       = self.__paramarray['url']
        file      = self.__paramarray['file']
        dbtype    = self.__paramarray['dbtype']
        dbmark    = self.__paramarray['dbmark']
        pmark     = self.__paramarray['pmark']
        cube      = self.__paramarray['cube']
        record    = self.__paramarray['record']
        item      = self.__paramarray['item']
        method    = self.__paramarray.GetIntegerValue('method',2)  #缺省保存到配置文件
        version   = self.__paramarray.GetIntegerValue('version',1) #缺省使用版本1
        way       = self.__paramarray['way']
        if(len(url) == 0):
            if(cmp(way,'file') == 0):
                if(len(self._url)!= 0):
                    self.__paramarray['url'] = self._url
                else:
                    self.__loghandle.error('Operate::CheckParam():url is NULL!')
                    return -1
            else:
                self.__loghandle.error('Operate::CheckParam():url is NULL!')
                return -1
        #检测配置文件中module中配置的file
        if(len(file)== 0):
            self.__loghandle.error('Operate::CheckParam():configure file is NULL!')
            return -1
        if(len(dbtype)==0):
            self.__loghandle.error('Operate::CheckParam():dbtype is NULL!')
            return -1
        if(method != 1 and method != 2):
            self.__loghandle.error('Operate::CheckParam():method is invalid,it must in [1,2]!')
            return -1
        if(version != 1 and version != 2):
            self.__loghandle.error('Operate::CheckParam():version is invalid,it must in [1,2]!')
            return -1
        if(method == 2):
            if(len(cube) == 0 and version == 1):
                self.__loghandle.error('Operate::CheckParam():cube is NULL!')
                return -1
            #.xml文件必须要配置record和item
            if(cmp(myConfig.GetFileType(),'.xml') == 0):
                if(len(record) == 0):
                    self.__loghandle.error('Operate::CheckParam():record is NULL!')
                    return -1
                if(len(item) == 0):
                    self.__loghandle.error('Operate::CheckParam():item is NULL!')
                    return -1
        #非分布式和分布式中的pmark可配置，也可不配置，不配置时给出警告
        if(len(pmark) == 0):
            self.__loghandle.warning('Operate::CheckParam():pmark is NULL!')
        #分布式必须要配置dbmark
        if(version == 2):
            if(len(dbmark) == 0):
                self.__loghandle.error('Operate::CheckParam():dbmark is NULL!')
                return -1
        return 0

########################################################################
###@brief : 参数解析类定义,解析命令行参数
###@param : void
###@return: void
########################################################################
class ParamAnalysis:
    def __init__(self):
        self.__commandlines=' '.join(sys.argv)#完整命令行
        self.__paramarray  = Dictionary()#参数解析结果
    def __len__(self):
        '''for len(object)'''
        return len(self.__paramarray)
    def __getitem__(self,key):
        '''for object[key]'''
        return self.__paramarray[key]
    def __setitem__(self,key,value):
        '''for object[key]=value'''
        self.__paramarray[key]=value
    def __str__(self):
        '''for str(object)'''
        return str(self.__paramarray)
    ########################################################################
    ###@brief : 获取命令行
    ###@param : void
    ###@return: string  命令行
    ########################################################################
    def GetCommandLine(self):
        return self.__commandlines
    ########################################################################
    ###@brief : 获取解析后的参数字典
    ###@param : void
    ###@return: dict  参数字典
    ########################################################################
    def GetParamArray(self):
        return self.__paramarray
    ########################################################################
    ###@brief : 获取整形参数
    ###@param : key    [in] 参数名称
    ###@param : defint [in] 缺省值
    ###@return: int 
    ########################################################################
    def GetIntegerParam(self,key,defint = -1):
        value = self.__paramarray[key]
        if(len(value) == 0):
            return defint
        else:
            return int(value)
    ########################################################################
    ###@brief : 获取字符串参数
    ###@param : key    [in] 参数名称
    ###@param : defstr [in] 缺省值
    ###@return: int 
    ########################################################################
    def GetStringParam(self,key,defstr = ''):
        value = self.__paramarray[key]
        if(len(value) == 0):
            return defstr
        else:
            return value
    ########################################################################
    ###@brief : 参数解析
    ###@param : void
    ###@return: int   0----成功，-1----失败
    ########################################################################
    def Analysis(self):
        #参数个数大于等于2
        if (len(sys.argv) < 2):
            print 'Parameter error:None Parameters!'
            self.__paramarray['action'] = 'h'
            return -1
        try:
            opts, args = getopt.getopt(sys.argv[1:],'deghnaR:c:r:i:t:D:p:u:f:F:s:l:L:m:v:',
            [
                'decrypt',
                'encrypt',
                'get',
                'help',
                'unEncrypt',
                'addflag',
                'read=',
                'cube=',
                'record=',
                'item=',
                'type=',
                'db_mark=',
                'pmark=',
                'url=',
                'file=',
                'File=',
                'string=',
                'log=',
                'level=',
                'method=',
                'version='
            ])
        except getopt.GetoptError:
            print 'Parameter error!'
            self.__paramarray['action'] = 'h'
            return -1
        ActionFlag = 0#设置动作标志
        ObjectFlag = 0#设置处理对象标志,针对加密，解密操作
        #解析命令参数
        for option,value in opts:
            if option in ["-d", "--decrypt"]:
                if(ActionFlag == 0):
                    self.__paramarray['action'] = 'd'
                    ActionFlag = 1
            elif option in ["-e", "--encrypt"]:
                if(ActionFlag == 0):
                    self.__paramarray['action'] = 'e'
                    ActionFlag = 1
            elif option in ["-g", "--get"]:
                if(ActionFlag == 0):
                    self.__paramarray['action'] = 'g'
                    ActionFlag = 1
            elif option in ["-h", "--help"]:
                if(ActionFlag == 0):
                    self.__paramarray['action'] = 'h'
                    ActionFlag = 1
                    #打印帮助信息，直接返回
                    return 0
            elif option in ["-R", "--read"]:
                if(ActionFlag == 0):
                    self.__paramarray['action'] = 'R'
                    self.__paramarray['config'] = value
                    ActionFlag = 1
                    #从脚本的配置文件中读取参数，直接返回
                    return 0
            elif option in ["-a", "--addflag"]:
                self.__paramarray['addflag'] = 1    #不存在添加，缺省不添加
            elif option in ["-n", "--unEncrypt"]:
                self.__paramarray['flag'] = 0#不加密，对config配置文件无效
            elif option in ["-c", "--cube"]:
                self.__paramarray['cube'] = value
            elif option in ["-r", "--record"]:
                self.__paramarray['record'] = value
            elif option in ["-i", "--item"]:
                self.__paramarray['item'] = value
            elif option in ["-t", "--type"]:
                self.__paramarray['dbtype'] = value
            elif option in ["-D", "--dbmark"]:
                self.__paramarray['dbmark'] = value
            elif option in ["-p", "--pmark"]:
                self.__paramarray['pmark'] = value
            elif option in ["-u", "--url"]:
                self.__paramarray['url'] = value
            elif option in ["-f", "--file"]:
                self.__paramarray['file'] = value
            elif option in ["-s", "--string"]:
                if(ObjectFlag == 0):
                    self.__paramarray['string'] = value
                    ObjectFlag = 1
            elif option in ["-F", "--File"]:
                if(ObjectFlag == 0):
                    self.__paramarray['dealfile'] = value
                    ObjectFlag = 1
            elif option in ["-m", "--method"]:
                self.__paramarray['method'] = value
            elif option in ["-L", "--level"]:
                self.__paramarray['level'] = value
            elif option in ["-v", "--version"]:
                self.__paramarray['version'] = value
        return 0
    ########################################################################
    ###@brief : 打印命令行解析结果
    ###@param : void
    ###@return: int   0----成功，-1----失败
    ########################################################################
    def  GetAnalysisResult(self):
        pre = '                                                              '
        str = "Param Analysis Result:\n"
        str = str + pre + "[Command ] %s\n" % self.__commandlines
        action = self.__paramarray['action']
        
        if(action == 'h'):
            str = str+pre+'[Action  ] print help usage\n'
        elif(action == 'd'):
            str = str+pre+'[Action  ] decrypt string or file\n'
        elif(action == 'e'):
            str = str+pre+'[Action  ] encrypt string or file\n'
        elif(action == 'g'):
            str = str+pre+'[Action  ] get db password from webserver,and get param from commandline\n'
        elif(action == 'R'):
            str = str+pre+'[Action  ] get db password from webserver,and get param from script config file\n'
        for key in self.__paramarray.keys():
            if(cmp(key,"action") != 0):
                str = str+pre+'[%-8s] %s\n' %(key,self.__paramarray[key])
        return str
#######################################################################
###@brief : 帮助信息
###@param : 无
###@return: void
########################################################################
def usage():
    print '''
    Usage:GetDBPassword.py [OPTIONS]
    
    use GetDBPassword.py to encrypt string、file,decrypt string、file,or
    get database's password and username from specified web server.
    
    Misc:
    -h,--help            print this help,then exit
    
    Operation:
    -g,--get             [action]Get password from web server
    -e,--encrypt         [action]encrypt a string or file.
    -d,--decrypt         [action]decrypt a string or file.
    -R,--read File       [action]Get parameters from this script's configure file[File]
    
    Options:
    -u,--url    URL      web server's url
    -f,--file   FILE     the configure file you want to update 
    -c,--cube   NAME     the cube' name you want to update in configure file    
    -r,--record KEY      the record' key(name) you want to update in cube,just for xml file
    -i,--item   NAME     the item' name you want to update in record,just for xml file
    -F,--File   NAME     the file' name you want to encrypt or decrypt
    -s,--string STRING   the string you want to encrypt or decrypt
    -t,--dbtype BDTYPE   the database's identifier,like as dbBiling,dbBackService and so on.
    -D,--dbmark DBMARK   the branch database's identifier list,like as mdb1|mdb2.just for distributed system
    -p,--pmark  PMARK    the product's identifier,from database table 'tfm_prod'
    -m,--method [1,2]    save mode,1 means save to new empty file,the 2 means save to configure file
    -v,--version[1,2]    1 means non distributed system,2 means distributed system
    -n,--unEncrypt       the flag of decrypting,without this option means encrypting
    -a,--addflag         the flag of adding a record when cube or record not exist,without this option means non adding
    
    Parameter details:
    the Operation -R,--read File,this option allows you to excute action[get] more than one once time,
    and the parameters needed for every action[get] is configured as a cube named module in the File.
    
    eg:
      [get action]:
        GetDBPassword.py -g -u url -f ./App.config -c COMMON -t dbBilling -m 2 [-p sfa]
        GetDBPassword.py -g -u url -f ./qmdb.ini  -t dbBilling -m 1 [-p sfa]
        GetDBPassword.py -g -u url -f ./mdb_config.ini  -t dbBilling -D mdb1|mdb2 -m 2 -v 2 [-p sfa]
        GetDBPassword.py -g -u url -f ./StreamSer_Sys.xml -t dbBilling -c StreamClientConfig.StreamClient -r Ora_User|Ora_Pwd -i value|value -m 2 [-p sfa]
      [encrypt action]:
        GetDBPassword.py -e  -s 123
        GetDBPassword.py -e  -F ./test.ini 
      [decrypt action]:
        GetDBPassword.py -d  -s 4f5051
        GetDBPassword.py -d  -F ./test.ini
      [read action]:
        GetDBPassword.py -R  ./GetDBPassword.config
    '''
########################################################################
###@brief : 主函数
###@param : 
###@return: 0
########################################################################
def main():
    ''' main function '''
    #日志设置
    loginit()
    logHandle = GetLogger()
    #参数解析
    myParam = ParamAnalysis()
    myParam.Analysis()
    logHandle.info(myParam.GetAnalysisResult())
    #执行动作
    if(myParam['action'] == 'h'):
        usage()
        sys.exit()
    elif(myParam['action'] == 'R'):
        myOperate = Operate()
        state = myOperate.Initialize(myParam['config'])
        if(state == 0):
            myOperate.Excute()
        else:
            print 'Error!'
    elif(myParam['action'] == 'd'):
        string   = myParam['string']
        dealfile = myParam['dealfile']
        if(len(string) == 0 and len(dealfile) == 0 ):
            print 'Error:-d must be used with -s or -F!'
            logHandle.error('-d must be used with -s or -F!')
            usage()
            sys.exit()
        else:
            decode_tools=TranslateTool()
            if(len(string) != 0):
                (code, string) = decode_tools.decrypt(string)
            elif(len(dealfile) != 0):
                (code, string) = decode_tools.decrypt_file(dealfile)
    elif(myParam['action'] == 'e'):
        string   = myParam['string']
        dealfile = myParam['dealfile']
        if(len(string) == 0 and len(dealfile) == 0):
            print 'Error:-e must be used with -s or -F!'
            logHandle.error('-e must be used with -s or -F!')
            usage()
            sys.exit()
        else:
            encode_tools=TranslateTool()
            if(len(string) != 0):
                (code, string) = encode_tools.encrypt(string)
            elif(len(dealfile) != 0):
                (code, string) = encode_tools.encrypt_file(dealfile)
    elif(myParam['action'] == 'g'):
        myOperate = Operate()
        myOperate.SetParamArray(myParam.GetParamArray())
        state = myOperate.ExcuteAction()
        if(state == -1):
            print 'Error'
    else:
        usage()
        sys.exit()

if __name__ == "__main__":
    main()

