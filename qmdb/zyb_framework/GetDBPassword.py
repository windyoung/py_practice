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
###@brief : ��־��ʼ��
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
###@brief : ��ȡ��־���
###@param : void
###@return: void
########################################################################
def GetLogger():
    return logging.getLogger()


########################################################################
###@brief : ���࣬�����Դ�����Ϣ�Ĵ���
###@param : void
###@return: void
########################################################################
class Base:
    def __init__(self):
        self.code        = 0      #����״̬��0--�ɹ���-1--ʧ�ܣ�-2--�ɹ��������о���
        self.reason      = ""     #ʧ��ԭ��򾯸�����
    ########################################################################
    ###@brief : ���ô�����Ϣ
    ###@param : code    [in] 0----�ɹ���-1----ʧ��
    ###@param : reason  [in] codeֵΪ-1��ʱ�򣬴˴�Ϊʧ�ܵ�ԭ��
    ###@return: void
    ########################################################################
    def set_errinfo(self,code=0,reason=""):
        self.code   = code
        self.reason = reason


########################################################################
###@brief : �ֵ���,�Ա�׼����dict���з�װ
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
###@brief : ���ܽ��ܹ����ඨ��
###@param : void
###@return: void
########################################################################
class  TranslateTool(Base):
    ''' Function list:
        1��encrypt() Encrypt a string.
        2��decrypt() Decrypt a string.
        3��encrypt_file() Encrypt a file.
        4��decrypt_file() Decrypt a file.'''
    def __init__(self):
        Base.__init__(self)
    ########################################################################
    ###@brief : ��������
    ###@param : password [in] ���ģ��ɼ��ַ���
    ###@return: (state,string) state: 0-�ɹ�,-1-ʧ�ܣ�string���Ļ���ʧ��ԭ��
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
    ###@brief : ��������
    ###@param : password [in] ���ģ�16�����ַ���
    ###@return: (state,string)state: 0-�ɹ�,-1-ʧ�ܣ�string:���Ļ���ʧ��ԭ��
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
    ###@brief : �����ļ�
    ###@param : File    [in] �ļ���
    ###@return: (state,string)state: 0-�ɹ�,-1-ʧ�ܣ�string:ʧ��ԭ��
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
    ###@brief : �����ļ�
    ###@param : File    [in] �ļ���
    ###@return: (state,string)state: 0-�ɹ�,-1-ʧ�ܣ�string:ʧ��ԭ��
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
###@brief : ConfigFile�ඨ��(����.config�����ļ�)
###@param : void
###@return: void
########################################################################
class ConfigFile(Base):
    def __init__(self):
        self.file     = ""    #�ļ�����
        self.AddFlag  = False #True--��¼���������,False--��¼���������ʧ��
        self.config   = ConfigParser.ConfigParser()
        self.config.optionxform = str#��Сд����
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
    #�ݶ�
    def __getitem__(self,cube):
        '''defined for obj[cube]'''
        index = self.FindCube(cube)
        if(index != -1):
            return self.config.items(cube)
    ########################################################################
    ###@brief : ������
    ###@param : void
    ###@return: void
    ########################################################################
    def clear(self):
        self.file     = ""
    ########################################################################
    ###@brief : ��ȡcube����
    ###@param : void
    ###@return: string cube����
    ########################################################################
    def GetCubeName(self,index):
        CubeList=self.config.sections() 
        return  CubeList[index]
    ########################################################################
    ###@brief : ������ӱ�־
    ###@param : flag   [in] bool��,false--�����ڲ����,true--���������
    ###@return: void
    ########################################################################
    def SetAddFlag(self,flag):
        self.AddFlag = flag 
    ########################################################################
    ###@brief : ����cube,���������
    ###@param : cube   [in] cube����
    ###@return: int    cube���
    ########################################################################
    def FindCube(self,cube):
        CubeList=self.config.sections()
        for i in range(0,len(CubeList)):
            if(CubeList[i] == cube):
                return i
        return -1
    ########################################################################
    ###@brief : ��ȡ��������
    ###@param : cube     [in] cube����
    ###@param : record   [in] record����
    ###@param : defint   [in] ȱʡֵ,�������򷵻ظ�ֵ,Ĭ��Ϊ0
    ###@return: int  ����
    ########################################################################
    def GetInteger(self,cube,record,defint = 0):
        (state,value) = self.GetValue(cube,record)
        if(state == -1):
            return defint
        else:
            return int(value)
    ########################################################################
    ###@brief : ��ȡstring����
    ###@param : cube     [in] cube����
    ###@param : record   [in] record����
    ###@param : defint   [in] ȱʡֵ,�������򷵻ظ�ֵ,Ĭ��Ϊ0
    ###@return: int  ����
    ########################################################################
    def GetString(self,cube,record,defstr = ""):
        (state,value) = self.GetValue(cube,record)
        if(state == -1):
            return defstr
        else:
            return value
    ########################################################################
    ###@brief : ��ȡ��¼ֵ
    ###@param : cube     [in] cube����
    ###@param : record   [in] record����
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
    ###@brief : ���Ҽ�¼
    ###@param : key   [in] ����:PASS_WORD|pass_word
    ###@return: (int,str) (״̬,����key)
    ########################################################################
    def FindMultiRecord(self,cube,key):
        keylist = key.split('|')
        for i in range(0,len(keylist)):
            if(self.config.has_option(cube,keylist[i])):
                return (0,keylist[i])
        return (-1,keylist[0])
    ########################################################################
    ###@brief : ��������
    ###@param : cube     [in] cube����
    ###@param : record   [in] record����
    ###@return: int   0--�ɹ���1--ʧ�� 
    ########################################################################
    def update(self,cube,record,value):
        (state,record) = self.FindMultiRecord(cube,record)
        if(state == 0):
            self.config.set(cube,record,str(value))
            return 0
        else:
            #���������
            if(self.AddFlag):
                index = self.FindCube(cube)
                #������cube���֮
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
    ###@brief : ���������ļ�
    ###@param : file     [in] �����ļ�����
    ###@return: int   0--�ɹ���1--ʧ�� 
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
    ###@brief : д�����ļ�
    ###@param : void
    ###@return: int   0--�ɹ���1--ʧ�� 
    ########################################################################   
    def write(self):
        self.config.write(open(self.file, "w")) 
    ########################################################################
    ###@brief : ȥ���ַ������ߵ���Ч�ַ�
    ###@param : string    [in] �������ַ���
    ###@return: void
    ########################################################################
    def Trim(self,string):
        if(len(string) == 0):
            return string
        else:
            #ȥ���ַ�����ߵ���Ч�ַ�
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
            #ȥ���ַ����ұߵ���Ч�ַ�
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
###@brief : XmlNode�ඨ��(����.xml�����ļ��е�һ�����)
###@param : void
###@return: void
########################################################################
class XmlNode(Base):
    def __init__(self,name = ""):
        Base.__init__(self)
        self.NodeName    = name #�������
        self.RecordList  = []     #��¼
        self.SubNodeList = []     #�ӽ��
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
    ###@brief : ��ȡ�ӽ��ĸ���
    ###@param : void
    ###@return: int   �ӽ��ĸ��� 
    ########################################################################
    def GetSubNodeCount(self):
        return len(self.SubNodeList)
    ########################################################################
    ###@brief : ��ȡ��¼������
    ###@param : void
    ###@return: int   ��¼������
    ########################################################################
    def GetRecordCount(self):
        return len(self.RecordList)
    ########################################################################
    ###@brief : ��ӽ����߼�¼
    ###@param : obj   [in]string-��¼�У�XmlNode����--XmlNode����
    ###@return: int   ��¼������
    ########################################################################
    def append(self,obj):
        if(isinstance(obj,str)):
            self.RecordList.append(obj)
        elif(isinstance(obj,XmlNode)):
            self.SubNodeList.append(obj)
    ########################################################################
    ###@brief : �����ӽ���Ƿ����
    ###@param : nodeName   [in]�ӽ������
    ###@return: int   ������
    ########################################################################
    def findNode(self,nodeName):
        for i in range(0,len(self.SubNodeList)):
            if(cmp(self.SubNodeList[i].NodeName,nodeName) == 0):
                return i
        return -1
    ########################################################################
    ###@brief : ���Ҽ�¼�Ƿ����
    ###@param : key   [in]��¼�м�ֵ��Ψһ��ʶһ����¼
    ###@return: int   ��¼�����
    ########################################################################
    def findRecord(self,key):
        for i in range(0,len(self.RecordList)):
            record = self.RecordList[i]
            #����ע��
            if(record.find('<!--') != -1 and record.find('/-->') != -1):
                continue
            if(record.find(key) != -1):
                return  i
        return -1
    ########################################################################
    ###@brief : ����itemֵ
    ###@param : key   [in]��¼�м�ֵ��Ψһ��ʶһ����¼
    ###@param : item  [in]Ҫ���µ���
    ###@param : value [in]�µ�ֵ
    ###@return: int   0----�ɹ���-1----ʧ��
    ########################################################################
    def UpdateItemValue(self,key,item,value):
        Recordindex = -1#��¼���
        for i in range(0,len(self.RecordList)):
            record = self.RecordList[i]
            #����ע��
            if(record.find('<!--') != -1 and record.find('/-->') != -1):
                continue
            if(record.find(key) != -1):
                Recordindex = i
                break
        if(Recordindex == -1):
            self.set_errinfo(-1,"Can't find key[%s]-item[%s]"%(key,item))
            return -1
        #ȡ����¼
        record = self.RecordList[Recordindex]
        head   = record[:record.find('<')+1]#����ͷ��
        tail   = record[record.find('/>'):] #����β��
        string = record[record.find('<')+1:record.find('/>')]
        #�ֽ��¼,�ָ��Ϊ�ո�
        itemlist = string.split()
        itemindex = -1#item���
        for i in range(0,len(itemlist)):
            if(itemlist[i].find(item) != -1):
                itemindex = i
                break
        if(itemindex == -1):
            self.set_errinfo(-1,"Can't find key[%s]-item[%s]!\n"%(key,item))
            return -1
        #ȡ��item
        item = itemlist[itemindex]
        #�ֽ�item,�ָ���Ϊ'='
        strlist = item.split('=')
        #��ֵ
        strlist[1] = "\"%s\"" % value
        #ƴ��
        item = '='.join(strlist)
        itemlist[itemindex] = item
        record = head+' '.join(itemlist)+tail
        self.RecordList[Recordindex] = record
        return 0
    ########################################################################
    ###@brief : �����ӽ�㣬���ط����������ӽ��ĸ���
    ###@param : name   [in]�ӽڵ�����
    ###@return: []  �ӽ������б�
    ########################################################################
    def SameSubCount(self,name):
        indexlist = []
        for i in range(0,len(self.SubNodeList)):
            if(cmp(self.SubNodeList[i].NodeName,name) == 0):
                indexlist.append(i)
        return indexlist
    ########################################################################
    ###@brief : ��ȡitemֵ
    ###@param : key   [in]��¼�м�ֵ��Ψһ��ʶһ����¼
    ###@param : item  [in]Ҫ���µ���
    ###@param : value [in]�µ�ֵ
    ###@return: int   0----�ɹ���-1----ʧ��
    ########################################################################
    def GetItemValue(self,key,item):
        recordindex = -1#record���
        for i in range(0,len(self.RecordList)):
            record = self.RecordList[i]
            #����ע��
            if(record.find('<!--') != -1 and record.find('/-->') != -1):
                continue
            if(record.find(key) != -1):
                recordindex = i
                break
        if(recordindex == -1):
            self.set_errinfo(-1,"Can't find key[%s]-item[%s]"%(key,item))
            return (-1,self.reason)
        #ȡ����¼
        record = self.RecordList[recordindex]
        string = record[record.find('<')+1:record.find('/>')]
        #�ֽ��¼,�ָ��Ϊ�ո�
        itemlist = string.split()
        itemindex = -1#item���
        for i in range(0,len(itemlist)):
            if(itemlist[i].find(item) != -1):
                itemindex = i
                break
        if(itemindex == -1):
            self.set_errinfo(-1,"Can't find key[%s]-item[%s]"%(key,item))
            return (-1,self.reason)
        #ȡ��item
        item = itemlist[itemindex]
        #�ֽ�item,�ָ���Ϊ'='
        strlist = item.split('=')
        value   = strlist[1]
        value   = value[1:len(value)-1]
        return (0,value)
    ########################################################################
    ###@brief : ������
    ###@param : void
    ###@return: void
    ########################################################################
    def clear(self):
        self.RecordList  = []
        self.SubNodeList = []


########################################################################
###@brief : xml�ඨ��(����.xml�����ļ�)
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
    ###@brief : ��ȡ����ֵ
    ###@param : key      [in] xml����ֵ��
    ###@param : record   [in] ��¼�м�ֵ
    ###@param : item     [in] ��
    ###@param : defint   [in] ȱʡֵ,�������򷵻ظ�ֵ,Ĭ��Ϊ0
    ###@return: int  ����
    ########################################################################
    def GetInteger(self,key,record,item,defint = 0):
        (state,value) = self.GetValue(key,record,item)
        if(state == -1):
            return defint
        else:
            return int(value)
    ########################################################################
    ###@brief : ��ȡstringֵ
    ###@param : key      [in] xml����ֵ��
    ###@param : record   [in] ��¼�м�ֵ
    ###@param : item     [in] ��
    ###@param : defstr   [in] ȱʡֵ,�������򷵻ظ�ֵ,Ĭ��Ϊ0
    ###@return: string  �ַ���
    ########################################################################
    def GetString(self,key,record,item,defstr = 0):
        (state,value) = self.GetValue(key,record,item)
        if(state == -1):
            return defstr
        else:
            return value
    ########################################################################
    ###@brief : ��ȡֵ
    ###@param : key    [in] xml����ֵ��������StreamServer.Server.slave
    ###@param : record [in] ��¼�м�ֵ������slave
    ###@param : item   [in] �����value
    ###@return: value  �ַ�����������
    ########################################################################
    def GetValue(self,key,record,item):
        (state,obj) = self.SearchNode(self.root,key)
        if(state == 0):
            return obj.GetItemValue(record,item)
        else:
            self.set_errinfo(-1,"Can't find <%s> !\n" % key)
            return (-1,self.reason)
    ########################################################################
    ###@brief : ����ֵ
    ###@param : key    [in] xml����ֵ��������StreamServer.Server.slave
    ###@param : record [in] ��¼�м�ֵ������slave
    ###@param : item   [in] �����value
    ###@param : value  [in] ��ֵ
    ###@return: int    0----�ɹ���-1----ʧ��
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
    ###@brief : �ݹ���ҽ��
    ###@param : node   [in] Ҫ���ҵ�xml��㣬ͨ����root��㿪ʼ
    ###@param : key    [in] xml����ֵ��
    ###@param : item   [in] �����value
    ###@param : value  [in] ��ֵ
    ###@return: (state,object) state:0-�ɹ���-1-ʧ��
    ########################################################################
    def SearchNode(self,node,key):
        NodeName  = ""#��ʱ�����ֵ
        keylist   =  key.split('.')
        if(len(keylist) == 0):
            return (0,node)
        indexlist =  node.SameSubCount(keylist[0])
        NodeName = keylist[0]
        del keylist[0] #��ֵ��ʹ�ã�ɾ����
        #δ�ҵ��ӽ��
        if(len(indexlist) == 0):
            #���ں���NodeName�ļ�¼�����ص�ǰ���
            #�����ڣ�����ʧ��
            if(node.findRecord(NodeName) != -1):
                return (0,node)
            else:
                return (-1,"")
        #����Ψһ��һ���ӽ��
        elif(len(indexlist) == 1):
            #key�����ѽ����������ҵ����ӽ��
            if(len(keylist) == 0):
                return (0,node[indexlist[0]])
            #key����δ����������SearchNode��������
            elif(len(keylist) > 0):
                return self.SearchNode(node[indexlist[0]],'.'.join(keylist))
        #�����node����һ��
        elif(len(indexlist) > 1):
            #û�к���key,�޷��ҵ�ƥ���ӽ��
            if(len(keylist) == 0):
                print 'error,too many node named[%s]!\n' % NodeName
                return (-1,"")
            #���Ȳ�ѯ��node����node���Ƿ�������ƺͺ���key(keylist[0])һ�µ�
            for i in range(0,len(indexlist)):
                if(node[indexlist[i]].findNode(keylist[0]) != -1):
                    return self.SearchNode(node[indexlist[i]],'.'.join(keylist))
            #δ�ҵ�,���ѯ��node���Ƿ���ڼ�¼�����ַ���keylist[0]
            for i in range(0,len(indexlist)):
                if(node[indexlist[i]].findRecord(keylist[0]) != -1):
                    del keylist[0]#ɾ����ƥ���
                    return self.SearchNode(node[indexlist[i]],'.'.join(keylist))
            #δ�ҵ�ƥ���ӽڵ�
            return (-1,"")
    ########################################################################
    ###@brief : ����xml�ļ�
    ###@param : file   [in] Ҫ���ҵ�xml��㣬ͨ����root��㿪ʼ
    ###@return: int    0----�ɹ���-1----ʧ��
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
    ###@brief : �ݹ鴴��xml��
    ###@param : file       [in] xml�����ļ�
    ###@param : patentnode [in] ����㣬ͨ����root��㿪ʼ
    ###@return: void
    ########################################################################
    def __BuildXmlTree(self,file,patentnode):
        CurrentNode   = patentnode
        line = file.readline()
        while line:
            (state,string)=self.AnalysisLine(line)
            #�½��
            if(state == 1):
                #����xml���
                NewNode  = XmlNode(string)
                CurrentNode.append(NewNode)
                #�ݹ����
                self.__BuildXmlTree(file,NewNode)
            elif(state == 2):
                #�˽���ȡ��ϣ�����
                if(string == CurrentNode.NodeName):
                    return
            elif(state == 3 or state == 4):
                CurrentNode.append(line)
            line = file.readline()
    ########################################################################
    ###@brief : �ݹ��ӡxml�ļ�
    ###@param : patentnode [in] Ҫ��ӡ�Ľ�㣬ͨ����root��㿪ʼ
    ###@return: string  �ַ���
    ########################################################################
    def __PrintNode(self,node,level):
        string = ""
        prefix = ""
        #�ӽ������
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
    ###@brief : ������¼��
    ###@param : string [in] ���ļ��ж�ȡ��һ�м�¼
    ###@return: string  �ַ���
    ########################################################################
    def AnalysisLine(self,string):
        if(len(string)==0):
            return(0,"")
        #����<sys>
        if (string.find('<') != -1 and string.find('>') != -1 
            and string.find('</') == -1 and string.find('/>') == -1 
              and string.find('<!--') == -1 and string.find('/-->') == -1):
            string = string[string.find('<')+1:string.find('>')]
            name = string.split()[0]
            return (1,name)
        #����</sys>
        elif(string.find('</') != -1 and string.find('>') != -1):
            string = string[string.find('</')+2:string.find('>')]
            name = string.split()[0]
            return (2,name)
        #����<section name="dsn" value="C3" />
        elif(string.find('<') != -1 and string.find('/>') != -1):
            return (3,"")
        #����<!--???/-->
        if (string.find('<!--') != -1 and string.find('/-->') != -1):
            string = string[string.find('<!--')+1:string.find('/-->')]
            return (4,"")
        else:
            return (0,"")


########################################################################
###@brief : �����ļ������ඨ��
###@param : file_type [in] �����ļ�����,.config or .xml,ȱʡ.config
###@param : ch        [in] ע�ͷ���for .config
###@return: void
########################################################################
class FileControl(Base):
    def __init__(self,file_type = ".config",ch = '#'):
        Base.__init__(self)
        self.filetype   = file_type#�ļ�����
        self.__filename = ""       #�����ļ�����
        self.AddFlag    = False    #��ӱ�־
        if(self.filetype == '.config'):
            self.FileObject = ConfigFile()
        elif(self.filetype == '.xml'):
            self.FileObject = XmlTree()
        self._CodeTools = TranslateTool()#���ܹ���
    def __len__(self):
        '''for len(object)'''
        return len(self.FileObject)
    def __str__(self):
        '''for str(object)'''
        return str(self.FileObject)
    ########################################################################
    ###@brief : ������
    ###@param : void
    ###@return: void
    ########################################################################
    def clear(self):
        self.FileObject.clear()
        self.__filename=""
    ########################################################################
    ###@brief : �����Ƿ���ܱ�־
    ###@param : void
    ###@return: void
    ########################################################################
    def SetEncodeFlag(self,flag):
        self.flag = flag
    ########################################################################
    ###@brief : ������ӱ�־
    ###@param : flag   [in]bool���ͣ�false--�����ڲ���ӣ�true--���������
    ###@return: void
    ########################################################################
    def SetAddFlag(self,flag):
        self.AddFlag = flag
        if (self.filetype == '.config'):
            self.FileObject.SetAddFlag(flag)
    ########################################################################
    ###@brief : ��ȡ�ļ�����׺
    ###@param : string   [in]�ļ���
    ###@return: string   ��׺
    ########################################################################
    def __GetSuffix(self,string):
        iIndex = string.rfind('.')
        if(iIndex != -1):
            return string[iIndex:]
        else:
            return ""
    ########################################################################
    ###@brief : ���ö�ȡ�ļ�����.config �� .xml
    ###@param : string   [in]�ļ�����
    ###@return: void
    ########################################################################
    def SetFileType(self,filetype):
        self.filetype = filetype
    ########################################################################
    ###@brief : ��ȡ�ļ�����
    ###@param : void
    ###@return: string �ļ����ͣ�.config or .xml
    ########################################################################
    def GetFileType(self):
        return self.filetype
    ########################################################################
    ###@brief : ��ȡ�ļ�����
    ###@param : void
    ###@return: string �ļ�����
    ########################################################################
    def GetFileName(self):
        return self.__filename
    ########################################################################
    ###@brief : ��ȡ����ֵ
    ###@param : cube   [in] cube���ƻ��ֵ��
    ###@param : record [in] ��¼�м�ֵ
    ###@param : item   [in] �for.xml
    ###@param : defint [in] ȱʡֵ
    ###@return: int  
    ########################################################################
    def GetInteger(self,cube,record,item='',defint = 0):
        if(self.filetype == '.config'):
            return self.FileObject.GetInteger(cube,record,defint)
        elif(self.filetype == '.xml'):
            return self.FileObject.GetInteger(cube,record,item,defint)
    ########################################################################
    ###@brief : ��ȡ�ַ���ֵ
    ###@param : cube   [in] cube���ƻ��ֵ��
    ###@param : record [in] ��¼�м�ֵ
    ###@param : item   [in] �for.xml
    ###@param : defstr [in] ȱʡֵ
    ###@return: string  
    ########################################################################
    def GetString(self,cube,record,item='',defstr = ""):
        if(self.filetype == '.config'):
            return self.FileObject.GetString(cube,record,defstr)
        elif(self.filetype == '.xml'):
            return self.FileObject.GetString(cube,record,item,defstr)
    ########################################################################
    ###@brief : ��ȡֵ
    ###@param : cube   [in] cube���ƻ��ֵ��
    ###@param : record [in] ��¼�м�ֵ
    ###@param : item   [in] �for.xml
    ###@return: string  or int
    ########################################################################
    def GetValue(self,cube,record,item=''):
        if(self.filetype == '.config'):
            return self.FileObject.GetValue(cube,record)
        elif(self.filetype == '.xml'):
            return self.FileObject.GetValue(cube,record,item)
    ########################################################################
    ###@brief : ����ֵ
    ###@param : cube   [in] cube���ƻ��ֵ��
    ###@param : record [in] ��¼�м�ֵ
    ###@param : item   [in] �for.xml
    ###@param : value  [in] ��ֵ
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
    ###@brief : ��ȡ�����ļ�
    ###@param : file     [in] �����ļ�����
    ###@param : autotype [in] �Զ�ʶ���ļ���ʶ��1�Զ�ʶ��
    ###@return: int    0----�ɹ���-1----ʧ��
    ########################################################################
    def ReadConfig(self,file,autotype = 0):
        #�����Զ�ʶ���׺����
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
                #��ӡ������־
                self.set_errinfo(-1,"The format of file[%s] can't be recognized!" % self.__filename)
                return -1
        iRet = self.FileObject.parse(file)
        if(iRet == -1):
            self.set_errinfo(-1,self.FileObject.reason)
            return -1
        else:
            return 0
    ########################################################################
    ###@brief : д�����ļ�
    ###@param : void
    ###@return: int  0----�ɹ���-1----ʧ��
    ########################################################################
    def WriteConfig(self):
        '''write config to file'''
        try:
            #�ȱ��������ļ�
            srcfile=self.__filename
            desfile=srcfile+".bak"
            shutil.copy(srcfile,desfile)
            #�����ļ�
            if(cmp(self.filetype,'.config') == 0):
                self.FileObject.write()
            else:
                writefile = open(srcfile,"w")
                writefile.write(str(self.FileObject))
                writefile.close()
            #ת��Ϊunix��ʽ
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
###@brief : ��ȡ���ݿ������ඨ��
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
    ###@brief : ���ò����ֵ�,
    ###@param : paramdic [in] �����ֵ�
    ###@return: void
    ########################################################################
    def SetParamDic(self,paramdic):
        self.ParamDic = paramdic
        self.url      = self.ParamDic['url']
        self.dbtype   = self.ParamDic['dbtype']
        self.dbmark   = self.ParamDic['dbmark']
        self.pmark    = self.ParamDic['pmark']
        self.type     = self.ParamDic['dbtype']
        self.version  = self.ParamDic.GetIntegerValue('version',1)#ȱʡ�Ƿֲ�ʽ
    ########################################################################
    ###@brief : ��ȡ������ʽ��
    ###@param : paramdic [in] �����ֵ�
    ###@return: void
    ########################################################################
    def GetResult(self):
        return self.__Result
    ########################################################################
    ###@brief : ƴװ��Կ
    ###@param : void
    ###@return: ��Կ��
    ########################################################################
    def GetKey(self):
        time_now = time.strftime('%Y%m%d%H%M%S',time.localtime(time.time()))
        return  'ZSmartInnerData_DB_PWD_TOKEN_'+time_now
    ########################################################################
    ###@brief : ������
    ###@param : void
    ###@return:{} ��
    ########################################################################
    def CreateForm(self):
        form ={}
        #��ȡ��Կ
        (code,string) = self.encode_tools.encrypt(self.GetKey())
        if(code == -1):
            self.set_errinfo(-1,string);
            return -1
        else:
            key = string
        #ƴ�ӱ�
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
    ###@brief : ��web server�ϻ�ȡ����
    ###@return: int  0--�ɹ�;1--ʧ��
    ########################################################################
    def GetDBPassword(self):
        '''get db password from web server.'''
        #���ó�ʱʱ��
        socket.setdefaulttimeout(5)
        #���ɱ�
        form = self.CreateForm()
        #��ֵ����
        postdata=urllib.urlencode(form);
        try:
            #���������������
            req = urllib2.Request(self.url,postdata)
            #��url
            response = urllib2.urlopen(req)
            #��ȡ��������
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
    ###@brief : �������ص��ַ����������кϷ����ж�
    ###@param : string [in] web server���ص��ַ���
    ###@return: int   0--�ɹ���-1ʧ��;
    ########################################################################
    def Analyze(self,string):
        #������
        self.__Result.clear()
        if(len(string) == 0):
            self.set_errinfo(-1,"Web Server return a NULL string!\n");
            return -1
        elif(len(string) > 61 and self.version == 1):
            #oracle���ݿ���û������������󳤶Ⱦ�Ϊ30����˷��ص��ַ�����󳤶�Ϊ61
            self.set_errinfo(-1,"Unknown string format:string is too long!Check url!\n");
            return -1
        elif(len(string) > 1240 and self.version == 2):
            #oracle���ݿ���û������������󳤶Ⱦ�Ϊ30������ֿ�������ٹ�20�����򳤶��Ϊ1240
            self.set_errinfo(-1,"Unknown string format:string is too long!Check url!\n");
            return -1
        #��Կ��֤ʧ��
        elif(cmp(string,"0001") == 0):
            self.set_errinfo(-1,"Authentication failed!")
            return -1
        #���ݿⲻ����
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
###@brief : �����ļ����ඨ��
###@param : size   [in] ���ű�������module�ĸ���
###@param : index  [in] ִ��ָ����module���,ȱʡȫ��ִ��
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
    ###@brief : ������ӱ�־
    ###@param : flag [in] bool���ͣ�false--�����ڲ���ӣ�true--���������
    ###@return: void
    ########################################################################
    def SetAddFlag(self,flag):
        self.AddFlag = flag
    ########################################################################
    ###@brief : ���������ļ�����ȡ���ļ����е����
    ###@param : filename [in] �����ļ���
    ###@return: int   ���
    ########################################################################
    def GetFileIndex(self,filename):
        index  = -1  #���к�
        reason = ""  #����ԭ��
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
    ###@brief : ��ʼ����
    ###@param : scriptconfig [in] ����ű������ļ���ConfigFile�����
    ###@return: void  ��ʼ��ʧ�ܵ�module���ӡ������־
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
    ###@brief : д�����ļ�
    ###@param : void
    ###@return: void
    ########################################################################
    def WriteConfig(self):
        for i in range(0, len(self._pool)):
            state = self._pool[i].WriteConfig()
            if(state == -1):
                self.__loghandle.error('ConfigureFilePool::WriteConfig():%s file name = [%s]'%(self._pool[i].reason,self._pool[i].GetFileName()))
    ########################################################################
    ###@brief : ��ȡָ��ģ���������Ϣ
    ###@param : index [in] ָ��ģ����
    ###@return: int  0--�ɹ���-1ʧ��
    ########################################################################
    def __ReadModule(self,index):
        UNnormalFlag = 0 #ģ���쳣��־
        sSuffix = ""     #��׺  
        #ƴ��cube����
        cube="%s%d" %('module',index)
        #��ȡ�����ļ���
        file=self.__ScriptConfig.GetString(cube,'file')
        #�����ļ���Ϊ��
        if (len(file) == 0):
            reason = "Setting_Error:[%s] file = NULL, file name can't be NULL!" % cube
            self.__loghandle.error('ConfigureFilePool::__ReadModule():'+reason)
            self.__setstate(cube)
            self.set_errinfo(-1,reason)
            return -1
        else:            
            (iIndex,string) = self.GetFileIndex(file)
            #�����ļ����ڳ���,����״̬�����,Ȼ�󷵻�
            if(iIndex != -1):
                self.__setstate(cube,1,iIndex)
                return 0
        #��ȡ���淽ʽ
        save_method=self.__ScriptConfig.GetInteger(cube,'savemethod',2)
        #���淽ʽ���ô���
        if(save_method != 1 and save_method != 2):
            reason = "Setting_Error:[%s] savemethod = %d is invalid!" %(cube,save_method)
            self.__loghandle.error('ConfigureFilePool::__ReadModule():'+reason)
            self.__setstate(cube)
            self.set_errinfo(-1,reason)
            return -1
        if(save_method == 1):
            self.__setstate(cube,0)
            return 0
        #��������
        CurModule = FileControl()
        CurModule.SetAddFlag(self.AddFlag)
        state = CurModule.ReadConfig(file,1)#�Զ�ʶ���ļ�
        if(state == -1):
            self.__loghandle.error('ConfigureFilePool::__ReadModule():%s' % CurModule.reason)
            self.__setstate(cube)
            return -1;
        else:
            self._pool.append(CurModule)
            self.__setstate(cube,1,len(self._pool)-1)
            return 0
    ########################################################################
    ###@brief : ����״̬��Ϣ��ģ��
    ###@param : cube     [in] ָ��ģ��
    ###@param : state    [in] ����״̬
    ###@param : fileindex[in] �����ļ����
    ###@return: void
    ########################################################################
    def __setstate(self,cube,state = -1,fileindex = -1):
        #������Ч
        if(state == -1):
            self.__ScriptConfig.update(cube,'state',-1)
            self.__ScriptConfig.update(cube,'fileindex',-1)
        #��������,���淽ʽ1
        elif(state == 0):
            self.__ScriptConfig.update(cube,'state',0)
            self.__ScriptConfig.update(cube,'fileindex',-1)
        #��������,���淽ʽ2
        elif(state == 1):
            self.__ScriptConfig.update(cube,'state',0)
            self.__ScriptConfig.update(cube,'fileindex',fileindex)

########################################################################
###@brief : �����ඨ��
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
        self.__paramarray  = Dictionary()   #�����ֵ�
        self.__loghandle   = GetLogger()    #��־���
        self._ScriptConfig = ConfigFile()
        self._ScriptConfig.SetAddFlag(True)
    def WriteConfig(self):
        self._ConfigureFilePool.WriteConfig()
    ########################################################################
    ###@brief : ���ò����ֵ�,�������л�ȡ���������øú�������
    ###@param : array    [in] �����ֵ�
    ###@return: void
    ########################################################################
    def SetParamArray(self,array):
        self.__paramarray.clear()
        for key in array.keys():
            self.__paramarray[key]=array[key]
    ########################################################################
    ###@brief : ��ʼ��,�ӽű������ļ��ж�ȡ�������,��һ��ִ��n��get���붯��
    ###@param : file    [in] ���ű��������ļ�
    ###@return: void
    ########################################################################
    def Initialize(self,file='./GetDBPassword.config'):
        self.__loghandle.debug('Operate::Initialize():Begin!')
        #��ȡ�ű������ļ�
        iRet = self._ScriptConfig.parse(file)
        if(iRet == -1):
            self.__loghandle.error('Operate::Initialize():read script configure file [%s] error [%s]'% (file,self._ScriptConfig.reason))
            self.__loghandle.debug('Operate::Initialize():Finished! return = [false]')
            return -1

        #��ȡȫ������
        self._url     = self._ScriptConfig.GetString('COMMON','url')
        self._count   = self._ScriptConfig.GetInteger('COMMON','count',0)
        self._index   = self._ScriptConfig.GetInteger('COMMON','index',-1)
        self._addflag = self._ScriptConfig.GetInteger('COMMON','addflag',0) == 1
        
        #ȫ�����ý�����ȷ�Լ���
        iRet = self.__CheckGlobalParam()
        if (iRet == -1):
            self.__loghandle.debug('Operate::Initialize():Finished! return = [false]')
            return -1
        #���������ļ���
        self._ConfigureFilePool = ConfigureFilePool(self._count,self._index)
        self._ConfigureFilePool.SetAddFlag(self._addflag)
        iRet = self._ConfigureFilePool.InitPool(self._ScriptConfig)
        if(iRet == -1):
            self.__loghandle.debug('Operate::Initialize():Finished! return = [false]')
            return -1
        self.__loghandle.debug('Operate::Initialize():Finished! return = [true]')
        return 0
    ########################################################################
    ###@brief : ִ�ж���,ִ��ָ����������module�����õ�get���붯��
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
        #�������ļ�д������
        self._ConfigureFilePool.WriteConfig()
        self.__loghandle.debug('Operate::Excute():Finished!')
    ########################################################################
    ###@brief : ִ�е�index��module
    ###@param : index    [in] module���
    ###@return: void
    ########################################################################
    def __ExcuteModule(self,index):
        self.__loghandle.debug('[%s%d]:' %('module',index))
        #ƴ��cube����
        module="%s%d" %('module',index)
        state = self._ScriptConfig.GetInteger(module,'state',-1)
        if (state == -1):
            self.__loghandle.error('setting errors found in [%s],skip the current module!' % module)
            self.__loghandle.debug('Finished! return = [false]')
            return -1
        #��ղ�������
        self.__paramarray.clear()
        #ת������
        self.__paramarray['fileindex']= self._ScriptConfig.GetInteger(module,'fileindex',-1) #pool�б��
        self.__paramarray['flag']     = self._ScriptConfig.GetInteger(module,'flag',1)       #���ܱ�־,0������,1����
        self.__paramarray['addflag']  = self._ScriptConfig.GetInteger('COMMON','addflag',1)       #���ܱ�־,0������,1����
        self.__paramarray['method']   = self._ScriptConfig.GetInteger(module,'savemethod',2) #���淽ʽ,1���浥���ļ�,2���浽�����ļ���
        self.__paramarray['version']  = self._ScriptConfig.GetInteger(module,'version',1)    #�汾,1�Ƿֲ�ʽ,2�ֲ�ʽ
        self.__paramarray['file']     = self._ScriptConfig.GetString(module,'file')          #�����ļ���
        self.__paramarray['cube']     = self._ScriptConfig.GetString(module,'cube')          #cube
        self.__paramarray['record']   = self._ScriptConfig.GetString(module,'record')        #record����
        self.__paramarray['item']     = self._ScriptConfig.GetString(module,'item')          #item
        self.__paramarray['dbtype']   = self._ScriptConfig.GetString(module,'dbtype')        #db_type
        self.__paramarray['dbmark']   = self._ScriptConfig.GetString(module,'dbmark')        #db_mark
        self.__paramarray['pmark']    = self._ScriptConfig.GetString(module,'productmark')   #product_mark
        self.__paramarray['url']      = self._ScriptConfig.GetString(module,'url')           #url
        self.__paramarray['way']      = 'file' #�������ļ��ж�ȡ
        #ִ�ж���
        state = self.ExcuteAction()
        if(state == -1):
            self.__loghandle.debug("Finished! return = [false]")
            return -1
        self.__loghandle.debug("Finished! return = [true]")
        return 0
    ########################################################################
    ###@brief : ִ��module�Ķ���
    ###@param : void
    ###@return: void
    ########################################################################
    def ExcuteAction(self):
        self.__loghandle.debug('Operate::ExcuteAction():Begin!')
        method    = self.__paramarray.GetIntegerValue('method',2)      #ȱʡ���浽�����ļ�
        addflag   = self.__paramarray.GetIntegerValue('addflag',0) == 1#�������Ƿ���ӣ�ȱʡ�����
        way       = self.__paramarray['way']
        fileindex = self.__paramarray['fileindex']
        file      = self.__paramarray['file']
        flag      = self.__paramarray.GetIntegerValue('flag',1)#ȱʡ����,�Է�.config�ļ�����
        #��ȡ�����ļ��������
        myConfig = FileControl()
        if(method == 2):
            #�������ļ����л�ȡ�����ļ��������
            if(cmp(way,'file') == 0):
                myConfig = self._ConfigureFilePool[fileindex]
            else:
                state = myConfig.ReadConfig(file,1)
                if(state == -1):
                    self.__loghandle.error('Operate::ExcuteAction():'+myConfig.reason)
                    self.__loghandle.debug('Operate::ExcuteAction():Finished !return = [false]')
                    return -1
        #������ر�ʶ
        myConfig.SetEncodeFlag(flag) #���ܱ�־
        myConfig.SetAddFlag(addflag) #��ӱ�־
        #�������
        state = self.CheckParam(myConfig)
        if(state == -1):
            self.__loghandle.debug('Operate::ExcuteAction():Finished !return = [false]')
            return -1
        #�ӷ������ϻ�ȡ�û���������
        self._WebClient.SetParamDic(self.__paramarray)#����������ʲ���
        (state,reason) = self.__GetDBPasswd()
        if(state == 0):
            #���浽��Ӧ�������ļ�
            state = 0
            if(method == 1):
                state = self.SaveToFileAlone()
            elif(method == 2):
                state = self.SaveToConfig(myConfig)
            if(state == -1):
                self.__loghandle.debug('Operate::ExcuteAction():Finished !return = [false]')
                return -1
            #�������ж�ȡ������Ҫ��������
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
    ###@brief : ���浽�����ļ���
    ###@param : void
    ###@return: state -1-ʧ��,0-�ɹ�
    ########################################################################
    def SaveToConfig(self,Configobject):
        myConfig  = Configobject
        #��ȡ��ز���
        version   = self.__paramarray.GetIntegerValue('version',1)#ȱʡʹ�ð汾1
        cube      = self.__paramarray['cube']
        record    = self.__paramarray['record']
        item      = self.__paramarray['item']
        #��ȡ
        result    = self._WebClient.GetResult()
        #��ʱ����
        user_name = ""
        pass_word = ""
        state     = 0
        #�Ƿֲ�ʽ,һ�λ�ȡһ��user_name:pass_wordֵ��
        if(version == 1):
            user_name = result.GetString('noraml','USER_NAME')
            pass_word = result.GetString('noraml','PASS_WORD')
            if(cmp(myConfig.GetFileType(),'.config') == 0):
                state = self.__updateconfig(myConfig,cube,user_name,pass_word)
            elif(cmp(myConfig.GetFileType(),'.xml') == 0):
                state = self.__updatexml(myConfig,cube,record,item,user_name,pass_word)
            if(state == -1):
                return -1
        #�ֲ�ʽ,һ�λ�ȡn��user_name:pass_wordֵ��,n>=1
        elif(version == 2):
            for i in range(0,len(result)):
                cube = result.GetCubeName(i)
                user_name = result.GetString(cube,'USER_NAME')
                pass_word = result.GetString(cube,'PASS_WORD')
                if(cmp(myConfig.GetFileType(),'.config') == 0):
                    state = self.__updateconfig(myConfig,cube,user_name,pass_word)
                elif(cmp(myConfig.GetFileType(),'.xml') == 0):
                    #�˴�.xml�ļ��Ĵ�����Ҫ����
                    state = self.__updatexml(myConfig,cube,record,item,user_name,pass_word)
                if(state == -1):
                    return -1
        return 0
    ########################################################################
    ###@brief : ���浽�������ļ���
    ###@param : void
    ###@return: state  -1-ʧ��,0-�ɹ�
    ########################################################################
    def SaveToFileAlone(self):
        #��ȡ��ز���
        file      = self.__paramarray['file']
        dbtype    = self.__paramarray['dbtype']
        flag      = self.__paramarray.GetIntegerValue('flag',1)    #ȱʡ����
        version   = self.__paramarray.GetIntegerValue('version',1)#ȱʡʹ�ð汾1
        result    = self._WebClient.GetResult()
        #��ʱ����
        content   = ""
        cube      = ""
        user_name = ""
        pass_word = ""
        #���������
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
                cube    = result.GetCubeName(i)#��ʱcubeֵΪ�ֿ�����
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
        #д���ļ�
        file=codecs.open(file,"w","utf-8")
        file.write(content)
        file.close()
        return  0
    ########################################################################
    ###@brief : ����.config���͵������ļ�
    ###@param : void
    ###@return: void
    ########################################################################
    def __updateconfig(self,object,cube,user_name,pass_word):
        #�����û���
        state = object.update(cube,'USER_NAME|user_name',user_name)
        if(state == -1):
            self.__loghandle.error(object.reason)
            return -1
        #��������
        state = object.update(cube,'PASS_WORD|pass_word',pass_word)
        if(state == -1):
            self.__loghandle.error(object.reason)
            return -1
        else:
            return 0
    ########################################################################
    ###@brief : ����.xml���͵������ļ�
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
    ###@brief : ��web�ϻ�ȡ����
    ###@param : void
    ###@return: void
    ########################################################################
    def __GetDBPasswd(self):
        #���ò���
        self._WebClient.SetParamDic(self.__paramarray)
        #��ȡ����
        state = self._WebClient.GetDBPassword()
        if(state == -1):
            return (-1,self._WebClient.reason)
        else:
            return (0,"")
    ########################################################################
    ###@brief : ���ȫ�������Ƿ���ȷ
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
    ###@brief : �������,���������ڲ����ֵ���
    ###@param : void
    ###@return: void
    ########################################################################
    def CheckParam(self,configobject):
        myConfig  = configobject#��ǰ����������ļ�
        file      = self.__paramarray['file']
        url       = self.__paramarray['url']
        file      = self.__paramarray['file']
        dbtype    = self.__paramarray['dbtype']
        dbmark    = self.__paramarray['dbmark']
        pmark     = self.__paramarray['pmark']
        cube      = self.__paramarray['cube']
        record    = self.__paramarray['record']
        item      = self.__paramarray['item']
        method    = self.__paramarray.GetIntegerValue('method',2)  #ȱʡ���浽�����ļ�
        version   = self.__paramarray.GetIntegerValue('version',1) #ȱʡʹ�ð汾1
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
        #��������ļ���module�����õ�file
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
            #.xml�ļ�����Ҫ����record��item
            if(cmp(myConfig.GetFileType(),'.xml') == 0):
                if(len(record) == 0):
                    self.__loghandle.error('Operate::CheckParam():record is NULL!')
                    return -1
                if(len(item) == 0):
                    self.__loghandle.error('Operate::CheckParam():item is NULL!')
                    return -1
        #�Ƿֲ�ʽ�ͷֲ�ʽ�е�pmark�����ã�Ҳ�ɲ����ã�������ʱ��������
        if(len(pmark) == 0):
            self.__loghandle.warning('Operate::CheckParam():pmark is NULL!')
        #�ֲ�ʽ����Ҫ����dbmark
        if(version == 2):
            if(len(dbmark) == 0):
                self.__loghandle.error('Operate::CheckParam():dbmark is NULL!')
                return -1
        return 0

########################################################################
###@brief : ���������ඨ��,���������в���
###@param : void
###@return: void
########################################################################
class ParamAnalysis:
    def __init__(self):
        self.__commandlines=' '.join(sys.argv)#����������
        self.__paramarray  = Dictionary()#�����������
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
    ###@brief : ��ȡ������
    ###@param : void
    ###@return: string  ������
    ########################################################################
    def GetCommandLine(self):
        return self.__commandlines
    ########################################################################
    ###@brief : ��ȡ������Ĳ����ֵ�
    ###@param : void
    ###@return: dict  �����ֵ�
    ########################################################################
    def GetParamArray(self):
        return self.__paramarray
    ########################################################################
    ###@brief : ��ȡ���β���
    ###@param : key    [in] ��������
    ###@param : defint [in] ȱʡֵ
    ###@return: int 
    ########################################################################
    def GetIntegerParam(self,key,defint = -1):
        value = self.__paramarray[key]
        if(len(value) == 0):
            return defint
        else:
            return int(value)
    ########################################################################
    ###@brief : ��ȡ�ַ�������
    ###@param : key    [in] ��������
    ###@param : defstr [in] ȱʡֵ
    ###@return: int 
    ########################################################################
    def GetStringParam(self,key,defstr = ''):
        value = self.__paramarray[key]
        if(len(value) == 0):
            return defstr
        else:
            return value
    ########################################################################
    ###@brief : ��������
    ###@param : void
    ###@return: int   0----�ɹ���-1----ʧ��
    ########################################################################
    def Analysis(self):
        #�����������ڵ���2
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
        ActionFlag = 0#���ö�����־
        ObjectFlag = 0#���ô�������־,��Լ��ܣ����ܲ���
        #�����������
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
                    #��ӡ������Ϣ��ֱ�ӷ���
                    return 0
            elif option in ["-R", "--read"]:
                if(ActionFlag == 0):
                    self.__paramarray['action'] = 'R'
                    self.__paramarray['config'] = value
                    ActionFlag = 1
                    #�ӽű��������ļ��ж�ȡ������ֱ�ӷ���
                    return 0
            elif option in ["-a", "--addflag"]:
                self.__paramarray['addflag'] = 1    #��������ӣ�ȱʡ�����
            elif option in ["-n", "--unEncrypt"]:
                self.__paramarray['flag'] = 0#�����ܣ���config�����ļ���Ч
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
    ###@brief : ��ӡ�����н������
    ###@param : void
    ###@return: int   0----�ɹ���-1----ʧ��
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
###@brief : ������Ϣ
###@param : ��
###@return: void
########################################################################
def usage():
    print '''
    Usage:GetDBPassword.py [OPTIONS]
    
    use GetDBPassword.py to encrypt string��file,decrypt string��file,or
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
###@brief : ������
###@param : 
###@return: 0
########################################################################
def main():
    ''' main function '''
    #��־����
    loginit()
    logHandle = GetLogger()
    #��������
    myParam = ParamAnalysis()
    myParam.Analysis()
    logHandle.info(myParam.GetAnalysisResult())
    #ִ�ж���
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

