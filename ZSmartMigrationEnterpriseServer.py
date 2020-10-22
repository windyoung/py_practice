#-*- coding:utf-8 -*-
import types,os,sys,string,datetime,time,StringIO
import yaml,threading,base64
import cx_Oracle,cPickle
import subprocess
import telnetlib
from ftplib import FTP
import paramiko
import socket,logging,logging.config
from xlrd import open_workbook
from xlwt import Workbook
import xlwt,xlrd
import re as re_p
from xlutils.copy import copy
from tempfile import TemporaryFile
import multiprocessing, uuid
import lic as licen

__author__ = 'zhu.qianshu'
os.environ["NLS_LANG"]='.AL32UTF8'
reload(sys)
sys.setdefaultencoding('utf8')
sleep = threading.Event()
#upgrade-zhu.qianshu 2014-06-10 新增pdm判断逻辑，日志文件中给出提示，同步新增客户端的pdm版本校验
#upgrade-zhu.qianshu 2014-06-11 新增分割线布局，不做任何动作
class writelog():
    def __init__(self):
        #日志初始化
        LOG_FILECONF = 'config/log.conf'
        logging.config.fileConfig(LOG_FILECONF)
        self.logger = logging.getLogger()

    def write(self,level="debug",msg=''):
        #print msg
        if level == "debug":
            self.logger.debug(msg)
        elif level == "info":
            self.logger.info(msg)
        elif level == "warn":
            self.logger.warn(msg)
        elif level == "error":
            self.logger.error(msg)        
        elif level == "critical":
            self.logger.critical(msg)


log_w=writelog()
def chunks(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in xrange(0, len(l), n):
        yield l[i:i+n]
class read_xls(object):
    def __init__(self,xls_file='',list_count=0):
        self.xls_file=xls_file
        self.list_count=list_count
        self.__s_date = datetime.datetime(1899, 12, 31,  0, 0, 0, 0).toordinal()-1

    def getdate(self,time_float):
        if isinstance(time_float, float):
            date = int(time_float)

        frac = time_float - date
        seconds = int(round(frac * 86400.0))
        assert 0 <= seconds <= 86400
        if seconds == 86400:
            hour = minute = second = 0
            xldays += 1
        else:
            # second = seconds % 60; minutes = seconds // 60
            minutes, second = divmod(seconds, 60)
            # minute = minutes % 60; hour    = minutes // 60
            hour, minute = divmod(minutes, 60)

        d = datetime.datetime.fromordinal(self.__s_date + date)
        #print datetime.datetime(d.year,d.month,d.day,hour,minute,second,0)
        return datetime.datetime(d.year,d.month,d.day,hour,minute,second,0)


    def read_excel(self):
        book = open_workbook(self.xls_file)
        res=[]
        for sheet in book.sheets(): 
            if sheet.nrows > 0 and sheet.ncols > 0 and sheet.ncols==self.list_count: 
                for row in range(1, sheet.nrows): 
                    row_data = [] 
                    for col in range(sheet.ncols): 
                        data = sheet.cell(row, col).value
                        ctype=sheet.cell_type(row, col)
                        if ctype==3:
                            data=self.getdate(data)
                        elif ctype==2:
                            data=long(data)
                        else:
                            data=data
                        row_data.append(data)
                    res.append(tuple(row_data))
        return res

class write_xls(object):
    def __init__(self,xls_file='',sheet_name=''):
        self.xls_file=xls_file
        self.sheet_name=sheet_name
        self.book = Workbook(encoding='utf-8')
        self.sheet1 = self.book.add_sheet(self.sheet_name)
        self.row=0

    def write_excel(self,res_list):
        date_style = xlwt.XFStyle()
        date_style.num_format_str = 'M/D/YY h:mm'
        for one_row in res_list:
            col=0
            for one_item in one_row:
                if type(one_item)==datetime.datetime:
                    self.sheet1.write(self.row,col,one_item,date_style)
                else:
                    self.sheet1.write(self.row,col,one_item)
                col=col+1            
            self.row=self.row+1

    def excel_save(self):
        self.book.save(self.xls_file)

class xls4tep_bak():
    def __init__(self,xls_file_name=""):
        self.rb=open_workbook(xls_file_name,formatting_info=True,encoding_override="utf-8")
        self.sheet_list=[]

    def write_xls_by_tmp(self,cur=None,new_xls_file_name=""):
        for sheet in self.rb.sheets():
            self.sheet_list.append((sheet.number,sheet.name))
            col_cnt,row_cnt=sheet.ncols,sheet.nrows
            sheet_par=[]
            sql_list_str,function="",""
            for row in range(row_cnt):
                for col in range(col_cnt):
                    value=sheet.cell(row, col).value
                    ctype=sheet.cell_type(row, col)
                    if str(value)[0:5]=="sql: ":			
                        try:
                            sql_map=yaml.load(str(value))
                            sql_list_str=sql_map["sql"]
                            function=sql_map["function"]
                        except:
                            pass
                        xf=sheet.cell_xf_index(row, col)
                        sql_list=sql_list_str.split(";")
                        try:
                            sql_list.remove("")
                        except:
                            pass
                        sheet_par.append((row,col,sql_list))
            for one_par in sheet_par:
                (row,col,sql_list)=one_par
                row_start=row
                for sql in sql_list:
                    #print sql
                    if sql.upper()[0:6]!="SELECT":
                        continue
                    try:
                        cur.execute(sql)
                    except:
                        continue
                    title=tuple([one[0] for one in cur.description])
                    res_list=[]
                    while True:
                        one_row=cur.fetchone()
                        if one_row==None:
                            break
                        _xy=[]
                        for one_item in one_row:
                            if type(one_item) in [int,long,datetime.datetime,float]:
                                one_item=str(one_item)
                            elif type(one_item) in [cx_Oracle.LOB]:
                                one_item=str(one_item.read())
                            if one_item==None:
                                one_item=""	
                            _xy.append(one_item)
                        res_list.append(tuple(_xy))
                    if function.upper()=="GROUP":
                        a=[]
                        for ite in range(len(res_list[0])):
                            a.append(tuple([_y[ite] for _y in res_list]))
                        res_list=a
                    for one_row in res_list:
                        col_start=col
                        for one_item in one_row:
                            try:
                                sheet.put_cell(row_start, col_start, 1, one_item, xf_c)
                            except:
                                sheet.put_cell(row_start, col_start, 1, one_item, xf)
                            col_start=col_start+1
                            if col_start>254:
                                break
                        row_start=row_start+1
                        if row_start>60000:
                            break
                    if len(res_list)==0:
                        try:
                            sheet.put_cell(row, col, 1, "", xf_c)
                        except:
                            sheet.put_cell(row, col, 1, "", xf)
            wb=copy(self.rb)
            wb.save(new_xls_file_name)


class xls4tep():
    def __init__(self,xls_file_name=""):
        self.rb=open_workbook(xls_file_name,formatting_info=True,encoding_override="utf-8")
        self.sheet_list=[]

    def write_xls_by_tmp(self,cur=None,new_xls_file_name=""):
        for sheet in self.rb.sheets():
            self.sheet_list.append((sheet.number,sheet.name))
            col_cnt,row_cnt=sheet.ncols,sheet.nrows
            sheet_par=[]
            sql_list_str,function="",""
            for row in range(row_cnt):
                for col in range(col_cnt):
                    value=sheet.cell(row, col).value
                    ctype=sheet.cell_type(row, col)
                    if value[0:4]=="sql:":			
                        try:
                            sql_list_str=value[4:]
                            print sql_list_str
                        except BaseException,e:
                            print str(e)
                        xf=sheet.cell_xf_index(row, col)
                        sql_list=sql_list_str.split(";")
                        try:
                            sql_list.remove("")
                        except:
                            pass
                        sheet_par.append((row,col,sql_list))
            for one_par in sheet_par:
                (row,col,sql_list)=one_par
                row_start=row
                for sql in sql_list:
                    #print sql
                    if sql.upper().strip()[0:6]!="SELECT":
                        continue
                    try:
                        cur.execute(sql)
                    except:
                        continue
                    title=tuple([one[0] for one in cur.description])
                    res_list=[]
                    while True:
                        one_row=cur.fetchone()
                        if one_row==None:
                            break
                        _xy=[]
                        for one_item in one_row:
                            if one_item==None:
                                one_item=""	                            
                            if type(one_item) in [int,long,datetime.datetime,float]:
                                one_item=u""+str(one_item)
                            elif type(one_item) in [cx_Oracle.LOB]:
                                one_item=u""+one_item.read()
                            if one_item==None:
                                one_item=""	
                            try:
                                one_item=u""+one_item
                            except:
                                one_item=""
                            _xy.append(one_item)
                        res_list.append(tuple(_xy))
                    if function.upper()=="GROUP":
                        a=[]
                        for ite in range(len(res_list[0])):
                            a.append(tuple([_y[ite] for _y in res_list]))
                        res_list=a
                    for one_row in res_list:
                        col_start=col
                        for one_item in one_row:
                            try:
                                sheet.put_cell(row_start, col_start, 1, one_item, xf_c)
                            except:
                                sheet.put_cell(row_start, col_start, 1, one_item, xf)
                            col_start=col_start+1
                            if col_start>254:
                                break
                        row_start=row_start+1
                        if row_start>60000:
                            break
                    if len(res_list)==0:
                        try:
                            sheet.put_cell(row, col, 1, "", xf_c)
                        except:
                            sheet.put_cell(row, col, 1, "", xf)
            wb=copy(self.rb)
            wb.save(new_xls_file_name)



class MYFTP:
    def __init__(self, hostaddr, username, password, remotedir, port=21):
        self.hostaddr = hostaddr
        self.username = username
        self.password = password
        self.remotedir  = remotedir
        self.port     = port
        self.ftp      = FTP()
        self.file_list = []
        # self.ftp.set_debuglevel(2)
    def __del__(self):
        self.ftp.close()
        # self.ftp.set_debuglevel(0)
    def login(self):
        try:
            ftp = self.ftp
            timeout = 60
            socket.setdefaulttimeout(timeout)
            ftp.set_pasv(True)
            ftp.connect(self.hostaddr, self.port)
            ftp.login(self.username, self.password)
            self.debug_print(ftp.getwelcome())
        except BaseException,e:
            return ('NO',u'connect to ftp server:'+self.hostaddr+' failed.'+str(e.message))
        try:
            ftp.cwd(self.remotedir)
        except BaseException,e:
            return ('NO',u'No dir :'+self.remotedir+' on ftp server.'+str(e.message)) 
        return ('OK',u'connect to ftp server:'+self.hostaddr+' sucess.')

    def is_same_size(self, localfile, remotefile):
        try:
            remotefile_size = self.ftp.size(remotefile)
        except:
            remotefile_size = -1
        try:
            localfile_size = os.path.getsize(localfile)
        except:
            localfile_size = -1
        self.debug_print(u'lo:%d  re:%d' %(localfile_size, remotefile_size),)
        if remotefile_size == localfile_size:
            return 1
        else:
            return 0
    def remotefile_exists(self, remotefile):
        try:
            remotefile_size = self.ftp.size(remotefile)
            return True
        except:
            return False
    def download_file(self, localfile, remotefile):
        if self.remotefile_exists(remotefile)==False:
            return ('NO',u'%s File is not exists in ftp server.' % remotefile)

        try:
            file_handler = open(localfile, 'wb')
            self.ftp.retrbinary(u'RETR %s'%(remotefile), file_handler.write)
            file_handler.close()
            return ('OK',u'%s File down finish. size: %s.' % (remotefile,self.ftp.size(remotefile)))		
        except BaseException,e:
            return ('NO',u'%s File down failed.%s' % (remotefile,str(e.message)))

    def download_files(self, localdir='./', remotedir='./'):
        try:
            self.ftp.cwd(remotedir)
        except:
            return
        if not os.path.isdir(localdir):
            os.makedirs(localdir)
        self.file_list = []
        self.ftp.dir(self.get_file_list)
        remotenames = self.file_list
        #print(remotenames)
        #return
        for item in remotenames:
            filetype = item[0]
            filename = item[1]
            local = os.path.join(localdir, filename)
            if filetype == 'd':
                self.download_files(local, filename)
            elif filetype == '-':
                self.download_file(local, filename)
        self.ftp.cwd('..')
    def upload_file(self, localfile, remotefile):
        if not os.path.isfile(localfile):
            return ('NO',u'%s File put failed.file not exists' % localfile)
        try:
            file_handler = open(localfile, 'rb')
            self.ftp.storbinary(u'STOR %s' %remotefile, file_handler)
            file_handler.close()
            return ('OK',u'%s File put finish,file size %s.' % (localfile,self.ftp.size(remotefile)))
        except BaseException,e:
            return ('NO',u'%s File put failed, %s.' % (localfile,str(e.message)))

    def upload_files(self, localdir='./', remotedir = './'):
        if not os.path.isdir(localdir):
            return
        localnames = os.listdir(localdir)
        self.ftp.cwd(remotedir)
        for item in localnames:
            src = os.path.join(localdir, item)
            if os.path.isdir(src):
                try:
                    self.ftp.mkd(item)
                except:
                    self.debug_print('dir exists %s' %item)
                self.upload_files(src, item)
            else:
                self.upload_file(src, item)
        self.ftp.cwd('..')

    def get_file_list(self, line):
        ret_arr = []
        file_arr = self.get_filename(line)
        if file_arr[1] not in ['.', '..']:
            self.file_list.append(file_arr)

    def get_filename(self, line):
        pos = line.rfind(':')
        while(line[pos] != ' '):
            pos += 1
        while(line[pos] == ' '):
            pos += 1
        file_arr = [line[0], line[pos:]]
        return file_arr

    def debug_print(self,s):
        print (s)

    def deal_error(self,e):
        timenow  = time.localtime()
        datenow  = time.strftime(u'%Y-%m-%d', timenow)
        logstr = '%s error: %s' %(datenow, e)
        self.debug_print(logstr)
        file.write(logstr)
        sys.exit()

class MYSFTP:
    def __init__(self, hostaddr, username, password, remotedir, port=22):
        self.hostaddr = hostaddr
        self.username = username
        self.password = password
        self.remotedir  = remotedir
        self.port     = port
        self.transport = paramiko.Transport((self.hostaddr, self.port))
        self.file_list = []
        # self.ftp.set_debuglevel(2)
    def __del__(self):
        self.sftp.close()
        # self.ftp.set_debuglevel(0)
    def login(self):
        try:
            self.transport.connect(username = self.username, password = self.password)
            self.sftp = paramiko.SFTPClient.from_transport(self.transport)
        except BaseException,e:
            return ('NO',u'connect to ftp server:'+self.hostaddr+' failed.'+str(e.message))
        try:
            self.sftp.chdir(self.remotedir)
        except BaseException,e:
            return ('NO',u'No dir :'+self.remotedir+' on sftp server.'+str(e.message)) 
        return ('OK',u'connect to sftp server:'+self.hostaddr+' sucess.')

    def download_file(self, localfile, remotefile):
        try:
            self.sftp.get(remotefile,localfile)
            size = os.path.getsize(localfile)
            return ('OK',u'%s File down finish. size: %s.' % (remotefile,str(size)))		
        except BaseException,e:
            return ('NO',u'%s File down failed.%s' % (remotefile,str(e.message)))

    def upload_file(self, localfile, remotefile):
        try:
            self.sftp.put(localfile,remotefile)
            s=self.sftp.stat(remotefile)
            return ('OK',u'%s File put finish. size: %s.' % (remotefile,str(s.st_size)))		
        except BaseException,e:
            return ('NO',u'%s File put failed.%s' % (remotefile,str(e.message)))

    def close(self):
        self.sftp.close()

class sql_def(object):
    def __init__(self):
        self.get_db_con_sql='''select project_id,
                connection_id,
                connection_name,
                db_type,
                user_name,
                pwd,
                dsn,
                comments
           from mgf_connection
          where project_id = '%s' '''
        self.get_db_con_sql_full='''select project_id,
                        connection_id,
                        connection_name,
                        db_type,
                        user_name,
                        pwd,
                        dsn,
                        comments
                   from mgf_connection'''     
        self.get_db_con_sql_byid='''select a.project_id,b.project_name,
                        connection_id,
                        connection_name,
                        db_type,
                        user_name,
                        pwd,
                        dsn,
                        a.comments
                   from mgf_connection a,mgf_project b
                  where a.project_id=b.project_id
                  and b.state='A'
                  and connection_id = '%s' '''          
        self.add_db_con_sql=''' insert into mgf_connection(project_id,connection_id,connection_name,db_type,user_name,pwd,dsn,comments)
        values(:0,:1,:2,:3,:4,:5,:6,:7)'''
        self.get_mig_project_sql='''select project_id,project_name,comments,state  from mgf_project where state='A' order by project_id'''
        self.get_project_byid='''select project_id,project_name,comments,state from mgf_project where state='A' and project_id=%s'''
        self.get_db_con_detail_sql='''select a.project_id,
                        b.project_name,
                        connection_id,
                        connection_name,
                        db_type,
                        user_name,
                        pwd,
                        dsn,
                        a.comments
                   from mgf_connection a,mgf_project b
                  where a.project_id=b.project_id
                  and b.state='A'
                  and a.project_id = '%s'
                  and connection_name = '%s' '''
        self.get_db_con_detail_bypro='''select a.project_id,
                        b.project_name,
                        connection_id,
                        connection_name,
                        db_type,
                        user_name,
                        pwd,
                        dsn,
                        a.comments
                   from mgf_connection a,mgf_project b
                  where a.project_id=b.project_id
                  and b.state='A'
                  and a.project_id = '%s' '''
        self.del_db_con_sql='''delete from mgf_connection where connection_id='%s' '''
        self.update_db_con_sql='''update mgf_connection
            set project_id='%s',
                connection_name='%s',
                db_type='%s',
                user_name='%s',
                pwd='%s',
                dsn='%s',
                comments='%s'
          where connection_id = '%s'  '''
        self.new_project_sql='''insert into mgf_project(project_id,project_name,comments,state) values(:0,:1,:2,:3)'''
        self.get_max_projectid='''select max(project_id) from mgf_project'''
        self.get_max_con_id='''select max(connection_id) from mgf_connection '''
        self.add_con_sql='''insert into mgf_connection(project_id,connection_id,connection_name,db_type,user_name,pwd,dsn,comments) values(:0,:1,:2,:3,:4,:5,:6,:7)'''
        self.del_mig_project_sql='''update mgf_project set state='X' where project_id=%s'''
        self.get_table_sql='''select a.table_id,a.table_name,a.table_type,a.comments,a.project_id,b.project_name
        from mgf_all_table a,mgf_project b where a.table_type='%s' and a.project_id=%s and a.project_id=b.project_id order by table_id'''
        self.get_all_table_sql='''select a.table_id,a.table_name,a.table_type,a.comments,a.project_id,b.project_name,a.table_space
        from mgf_all_table a,mgf_project b where a.project_id=b.project_id and b.state='A' order by table_id'''        
        self.get_col_by_table_id='''select table_id,column_id,col_seq,col_name,col_type,null_able,comments from mgf_table_col 
        where table_id=%s order by col_seq'''
        self.del_col_by_table_id='''delete from mgf_table_col where table_id=%s'''
        self.add_tab_col='''insert into mgf_table_col(table_id,column_id,col_seq,col_name,col_type,null_able,comments) values
        (:0,:1,:2,:3,:4,:5,:6)'''
        self.get_max_col_id='''select max(column_id) from mgf_table_col'''
        self.get_tab_info_byid='''select a.table_id,a.table_name,a.table_type,a.comments,a.project_id,b.project_name,a.table_space,a.connection_id,c.connection_name
        from mgf_all_table a,mgf_project b,mgf_connection c where a.project_id=b.project_id and b.state='A' and a.table_id=%s
        and a.connection_id=c.connection_id'''
        self.new_one_tab_sql='''insert into mgf_all_table(table_id,table_name,table_type,comments,project_id,table_space,connection_id) values(:0,:1,:2,:3,:4,:5,:6)'''
        self.update_one_tab='''update mgf_all_table set table_type='%s',comments='%s',project_id=%s,table_space='%s',connection_id=%s
        where table_id=%s'''
        self.del_one_tab='''delete from mgf_all_table where table_id=%s'''        
        self.get_max_tab_id='''select max(table_id) from mgf_all_table'''
        self.get_mig_flow_catg_byproid='''select project_id,step_catg_id,step_catg_name,exec_order_id from mgf_mig_flow_catg where project_id=%s order by exec_order_id'''
        self.add_mig_catg_sql='''insert into mgf_mig_flow_catg(project_id,step_catg_id,step_catg_name,exec_order_id,comments,state) values(:0,:1,:2,:3,:4,:5)'''
        self.get_max_catg_id='''select nvl(max(step_catg_id),0) from mgf_mig_flow_catg'''
        self.get_max_exec_order_id='''select max(exec_order_id) from mgf_mig_flow_catg where project_id='%s' '''
        self.get_show_mig_step_sql='''
        select s.exec_order_id,
               s.step_name,
               s.function_code,
               s.func_par_list,
               s.state,
               to_char(h.start_time, 'YYYY-MM-DD HH24:MI:SS') start_time,
               to_char(h.end_time, 'YYYY-MM-DD HH24:MI:SS') end_time,
               (h.end_time - h.start_time)*86400 duration,
               h.rec_num,
               h.last_rec_num,
               s.step_id
          from mgf_mig_flow_step s, mgf_mig_detail_log h, mgf_mig_flow_catg g
         where s.step_catg_id = g.step_catg_id
           and g.project_id = :0
           and g.exec_order_id = :1
           and s.step_id = h.step_id(+)
         order by s.exec_order_id
        '''
        self.start_mig_step_sql='''insert into mgf_mig_detail_log(log_id,step_id,state) values(?,?,?)'''
        self.set_mig_step_state_all='''update mgf_mig_flow_step set state='%s' where step_catg_id=%s'''
        self.set_mig_step_state_all_by_exec_id='''update mgf_mig_flow_step set state='%s' where step_catg_id in 
        (select step_catg_id from mgf_mig_flow_catg where project_id=%s and exec_order_id=%s)'''
        self.set_mig_step_state_one='''update mgf_mig_flow_step set state='%s' where step_id=%s'''
        self.set_mig_catg_state_one='''update mgf_mig_flow_catg set state='%s' where project_id=%s and exec_order_id=%s'''
        self.set_mig_catg_state_one_by_catg='''update mgf_mig_flow_catg set state='%s' where step_catg_id=%s'''
        self.set_mig_step_state_after='''update mgf_mig_flow_step set state='%s' where exec_order_id>=%s and step_catg_id=%s'''
        self.get_max_log_id='''select max(log_id) from mgf_mig_detail_log'''
        self.get_max_step_id='''select max(step_id) from mgf_mig_flow_step'''
        self.get_max_step_order_id='''select max(exec_order_id) from mgf_mig_flow_step where step_catg_id=%s'''

        ###############Step Define
        self.get_tab_by_projectid='''select * from mgf_all_table where project_id=%s'''

        self.new_one_step='''insert into mgf_mig_flow_step(step_catg_id,step_id,step_name,function_code,func_par_list,exec_order_id) 
        values(:0,:1,:2,:3,:4,:5)'''

        self.update_after_step_order_id='''update mgf_mig_flow_step set exec_order_id=exec_order_id+1 where step_catg_id=:0 and exec_order_id>=:1'''

        ##############step run
        self.get_one_wait_step='''
        select *
          from (select a.step_id,
                       a.function_code,
                       a.func_par_list,
                       c.exec_order_id as exec_id1,
                       a.exec_order_id as exec_id2,
                       a.background,
	               nvl(a.flow,'f0') flow,
	               a.depend_flow
                  from mgf_mig_flow_step a, mgf_mig_flow_catg c
                 where a.step_catg_id = c.step_catg_id
                   and c.project_id = %s
                   and a.state = 'WAIT'
                 order by c.exec_order_id, a.exec_order_id)
         where rownum = 1
         and 'A' = (select state from mgf_project where project_id = %s)
        '''
        self.get_pause_step_exec='''
        select c.exec_order_id as exec_id1,a.exec_order_id as exec_id2
        from mgf_mig_flow_step a,
        mgf_mig_flow_catg c
        where a.step_catg_id = c.step_catg_id
        and c.project_id = %s
        and a.state = 'PAUSE'
        '''
        self.set_step_run_state='''update mgf_mig_flow_step a set a.state='RUN',start_time=sysdate where step_id=%s'''
        self.set_step_back_state='''update mgf_mig_flow_step a set a.state='BACK',start_time=sysdate where step_id=%s'''
        self.get_con_bytab='''select connection_id from mgf_all_table where table_id=%s'''
        self.get_mig_step_err='''
        select count(1) from mgf_mig_flow_step where state in ('ERROR','RUN') and step_catg_id in 
        (select step_catg_id from mgf_mig_flow_catg where project_id = %s)        
        '''
        self.del_remove_mig_log='''
        delete from mgf_mig_detail_log where step_id=%s
        '''
        self.get_remove_mig_log='''
        select (select max(his_id) from mgf_mig_detail_log_his) his_id,
        a.step_id,
        a.start_time,
        a.end_time,
        b.state,
        log_detail,
        last_rec_num,
        a.rec_num,
	a.atth1,
	a.atth1_name
        from mgf_mig_detail_log a,mgf_mig_flow_step b
        where a.step_id = %s
        and a.step_id=b.step_id
        '''
        self.insert_mig_log='''insert into mgf_mig_detail_log(step_id,start_time,end_time,log_detail,rec_num,atth1,atth1_name) values(:0,:1,:2,:3,:4,:5,:6)'''
        self.insert_mig_log_his='''insert into mgf_mig_detail_log_his(his_id,step_id,start_time,end_time,state,log_detail,rec_num,atth1,atth1_name) values(:0,:1,:2,:3,:4,:5,:6,:7,:8)'''
        self.get_mig_log_by_step='''select log_detail
        from mgf_mig_detail_log a
       where a.step_id = %s'''
        self.del_mig_step_byid='''delete from mgf_mig_flow_step where step_id = %s'''
        self.update_bef_step_order_id='''update mgf_mig_flow_step set exec_order_id=exec_order_id-1 where step_catg_id=:0 and exec_order_id>:1'''
        self.get_tab_by_con='''select table_id,table_name from mgf_all_table where connection_id=%s'''

        self.get_tab_col_byid='''select table_id,column_id,col_seq,col_name,col_type,null_able,comments from mgf_table_col 
        where table_id=%s order by col_seq asc'''
        self.get_table_info_byid='''select table_id,table_name,table_type,comments,project_id,table_space,connection_id,data from mgf_all_table where
        table_id=%s'''
        self.get_con_info_byid='''select project_id,connection_id,connection_name,db_type,user_name,pwd,dsn,comments from mgf_connection 
        where connection_id=%s'''
        self.mig_state='''select exec_order_id,state from mgf_mig_flow_catg a where project_id=%s order by exec_order_id asc'''
        self.get_mig_catg_id='''select a.step_catg_id,step_catg_name from mgf_mig_flow_catg a where a.project_id=%s and a.exec_order_id=%s'''
        self.get_mig_catg_by_stepid='''select a.step_catg_id from mgf_mig_flow_step a where a.step_id=%s'''
        self.check_catg_is_finish='''select count(*) from mgf_mig_flow_step a where a.step_catg_id=%s and a.state in ('WAIT','RUN','PAUSE')'''







class clear_table_data(object):
    def __init__(self,con=None,cur=None,table_name='',db_type=''):
        self.db_conn=con
        self.cur=cur
        self.table_name=table_name
        self.db_type=db_type

    def clear_table(self):
        try:
            if self.db_type == 'Oracle':
                self.cur.execute('truncate table '+self.table_name)
            elif self.db_type == 'TimesTen':
                self.cur.execute('delete from '+self.table_name)
                self.db_conn.commit()
            return ('OK','truncate table '+self.table_name+',Finish.')
        except BaseException,e:
            return ('NO',self.table_name+','+str(e.message))

    def drop_table(self):
        if self.db_type == 'Oracle':
            try:
                sql='''drop table %s purge''' % self.table_name.upper()
                self.cur.execute(sql)
                return ('OK','drop table '+self.table_name+',Finish.')
            except cx_Oracle.DatabaseError,e:
                error,=e.args
                if str(error.code)=='942':
                    return ('WARN','drop table '+self.table_name+',Finish. '+str(e.message))
                else:
                    try:
                        sql='''drop table "%s"''' % self.table_name.upper()
                        self.cur.execute(sql)
                        return ('OK','drop table '+self.table_name+',Finish.')
                    except cx_Oracle.DatabaseError,e:
                        error,=e.args
                        if str(error.code)=='942':
                            return ('WARN','drop table '+self.table_name+',Finish. '+str(e.message))
                        else:
                            return ('NO','drop table '+self.table_name+',Error. '+str(e.message))
        else:
            return ('NO','This Connection is not Oracle Connection.')


    def enable_contraints(self):
        try:
            self.cur.execute('''
	        select ur.CONSTRAINT_NAME,ur.TABLE_NAME
	    from user_constraints ur
	    where ur.TABLE_NAME = '%s'
	    and ur.CONSTRAINT_TYPE = 'R'
            and ur.status = 'DISABLED'
	    ''' % (self.table_name.upper(),))

            _contraints = self.cur.fetchall()

            for one_contraints in _contraints:
                self.cur.execute("alter table "+one_contraints[1]+" enable constraint "+ one_contraints[0])


            _sql = '''
	    select ur.CONSTRAINT_NAME,ur.TABLE_NAME
	    from user_constraints ur
	    where ur.CONSTRAINT_TYPE = 'R'
            and ur.status = 'DISABLED'
	    and ur.R_CONSTRAINT_NAME in
	     (select ur.CONSTRAINT_NAME
	      from user_constraints ur
	     where ur.CONSTRAINT_TYPE in ('U', 'P')
	       and ur.TABLE_NAME = '%s')
	    '''

            self.cur.execute(_sql % (self.table_name.upper(),))
            _contraints = self.cur.fetchall()

            for one_contraints in _contraints:
                self.cur.execute("alter table "+one_contraints[1]+" enable constraint "+ one_contraints[0])

            return ('OK',self.table_name+" enable FK sucess")
        except BaseException,e:
            return ('NO',self.table_name+" enable FK faild,"+str(e.message))

    def disable_pk(self):
        try:
            self.cur.execute('''select a.constraint_name from user_constraints a where a.table_name='%s' and a.constraint_type in ('P','U') ''' % (self.table_name.upper(),))

            _contraints = self.cur.fetchall()

            for one_contraints in _contraints:
                #print "alter constraint "+ one_contraints[0] + ' disable '
                self.cur.execute("alter table "+ self.table_name+ "  disable constraint "+ one_contraints[0] )
                try:
                    self.cur.execute("drop index %s" % one_contraints[0])
                except cx_Oracle.DatabaseError,e:
                    print "try drop pk index %s finish." % one_contraints[0],str(e.message)
            return ('OK',self.table_name+" disable PK Succes")
        except BaseException,e:            
            return ('NO',self.table_name+" disable PK faild,"+str(e.message))

    def enable_pk(self,index_tablespace=""):	    
        try:
            self.cur.execute('''select a.constraint_name from user_constraints a where a.table_name='%s' and a.constraint_type in ('P','U') ''' % (self.table_name.upper(),))

            _contraints = self.cur.fetchall()

            for one_contraints in _contraints:
                self.cur.execute("alter table "+ self.table_name+ "  enable constraint "+ one_contraints[0]+ " using index tablespace "+ index_tablespace)
            return ('OK',self.table_name+" enable PK Succes")
        except BaseException,e:            
            return ('NO',self.table_name+" enable PK faild,"+str(e.message))    

    def disable_contraints(self):
        try:
            self.cur.execute('''
               select ur.CONSTRAINT_NAME
           from user_constraints ur
           where ur.TABLE_NAME = '%s'
           and ur.CONSTRAINT_TYPE = 'R'
           and ur.status = 'ENABLED'
           ''' % (self.table_name.upper(),))

            _contraints = self.cur.fetchall()

            for one_contraints in _contraints:
                #print "alter constraint "+ one_contraints[0] + ' disable '
                self.cur.execute("alter table "+ self.table_name+ "  disable constraint "+ one_contraints[0] )



            _sql = '''
           select ur.CONSTRAINT_NAME,table_name
           from user_constraints ur
           where ur.CONSTRAINT_TYPE = 'R'
           and ur.status = 'ENABLED'
           and ur.R_CONSTRAINT_NAME in
            (select ur.CONSTRAINT_NAME
             from user_constraints ur
            where ur.CONSTRAINT_TYPE in ('U', 'P')
              and ur.TABLE_NAME = '%s')
           '''

            self.cur.execute(_sql % (self.table_name.upper(),))
            _contraints = self.cur.fetchall()
            #print _contraints

            for one_contraints in _contraints:
                self.cur.execute("alter table "+one_contraints[1]+ " disable constraint "+one_contraints[0])
            return ('OK',self.table_name+" disable FK Succes")
        except BaseException,e:            
            return ('NO',self.table_name+" disable FK faild,"+str(e.message))
    def nologging_table(self):
        log=[]
        try:
            self.cur.execute('alter table %s nologging' % self.table_name)
            log.append("table %s nologging finish." % self.table_name)
        except cx_Oracle.DatabaseError,e:
            log.append("table %s nologging failed.%s" % (self.table_name,e.message.message))
            return ('NO','\n'.join(log))
        try:
            sql="select index_name from user_indexes a where a.table_name='%s' and a.index_type<>'LOB' " % self.table_name.upper()
            re=self.cur.execute(sql).fetchall()
            for one in re:
                index_name=one[0]
                self.cur.execute("alter index %s nologging" % index_name)
                log.append("table %s index %s nologging finish." % (self.table_name,index_name))
        except cx_Oracle.DatabaseError,e:
            log.append("table %s index %s nologging failed.%s" % (self.table_name,index_name,e.message.message))
            return ('NO','\n'.join(log))
        return ('OK','\n'.join(log))
    def logging_table(self):
        log=[]
        try:
            self.cur.execute('alter table %s logging' % self.table_name)
            log.append("table %s logging finish." % self.table_name)
        except cx_Oracle.DatabaseError,e:
            log.append("table %s logging failed.%s" % (self.table_name,e.message.message))
            return ('NO','\n'.join(log))
        try:
            sql="select index_name from user_indexes a where a.table_name='%s' and a.index_type<>'LOB' " % self.table_name.upper()
            re=self.cur.execute(sql).fetchall()
            for one in re:
                index_name=one[0]
                self.cur.execute("alter index %s logging" % index_name)
                log.append("table %s index %s logging finish." % (self.table_name,index_name))
        except cx_Oracle.DatabaseError,e:
            log.append("table %s index %s logging failed.%s" % (self.table_name,index_name,e.message.message))
            return ('NO','\n'.join(log))
        return ('OK','\n'.join(log))

class run_mig_step(object):
    def __init__(self,con=None,project_id=-1):
        # 创建数据库连接到文件"mydb"
        self.con=con
        # 获取游标对象
        self.cur=self.con.cursor()

        self.project_id=project_id

        self.sql_def=sql_def()
        self.get_next_step_sql=self.sql_def.get_one_wait_step

        self.db_con_map={}
        self.log=writelog()
        #初始化连接
        '''re=self.cur.execute(self.sql_def.get_db_con_sql %(self.project_id)).fetchall()
        for one_db in re:
            (project_id,connection_id,connection_name,db_type,user_name,pwd,dsn,comments)=one_db 
            con_dist={}
            con_dist['project_id']=project_id
            con_dist['db_type']=db_type
            con_dist['user_name']=user_name
            con_dist['pwd']=pwd
            con_dist['dsn']=dsn
            if db_type=='Oracle':
                try:		    
                    con=cx_Oracle.connect(user_name+'/'+pwd+'@'+dsn)
                    cur=con.cursor()
                except BaseException,e:
                    con,cur=None,None
                    print "error:568"
                    print user_name+'/'+pwd+'@'+dsn , str(e)
            elif db_type=='TimesTen':
                try:
                    con=odbc.odbc("uid='%s';pwd='%s';dsn='%s'" % (user_name,pwd,dsn))
                except BaseException,e:
                    print "error:573"
                    print str(e)
            con_dist['con']=con
            con_dist['cur']=cur
            self.db_con_map[connection_id]=con_dist
		'''
        #self.par_map={}
        #self.init_paramter()

        file_conf=open('config/tool_db_connection.conf','r')
        con_dist=yaml.load(file_conf.read())
        file_conf.close()
        self.user=con_dist['user']
        self.pwd=con_dist['pwd']
        self.dsn=con_dist['dsn']    

    def init_paramter(self,tool_con=None,db_con_map=None):
        #初始化参数
        par_map={}
        if tool_con==None:
            return par_map
        tool_cur=tool_con.cursor()
        sql='''select project_id,par_code,get_type,con_id,sql,constant,current_value from mgf_parameter a where project_id=%s''' % self.project_id
        re=tool_cur.execute(sql).fetchall()
        for one in re:
            (project_id,par_code,get_type,con_id,sql,constant,current_value)=one
            if get_type=='SQL':
                try:
                    cur=db_con_map[con_id]['cur']
                    value=cur.execute(sql).fetchone()[0]
                except:
                    value=None
                par_map[par_code]=str(value)
                if str(value) != str(current_value):
                    try:
                        tool_cur.execute('''update mgf_parameter set current_value='%s' where project_id=%s and par_code='%s' ''' \
                                         % (str(value),str(self.project_id),par_code))
                        tool_con.commit()
                    except:
                        pass
            elif get_type=='CONSTANT':
                par_map[par_code]=constant
        return par_map
        #self.con.commit()
        #print self.par_map,self.project_id
    def get_paramter(self,tool_con=None,db_con_map=None,par_map={},code="",par_ver_code=""):
        #初始化参数
        if tool_con==None or code=="":
            return par_map
        tool_cur=tool_con.cursor()
        sql='''select project_id,par_code,get_type,con_id,sql,constant,current_value 
	    from mgf_parameter a where project_id=%s and par_code='%s' and par_ver_code='%s' ''' % (self.project_id,code,par_ver_code)
        tool_cur.execute(sql)
        res=tool_cur.fetchall()
        if not res:
            sql='''select project_id,par_code,get_type,con_id,sql,constant,current_value 
            from mgf_parameter a where project_id=%s and par_code='%s' and par_ver_code='Global' ''' % (self.project_id,code)
            tool_cur.execute(sql)
            res=tool_cur.fetchall()
        for one in res:
            (project_id,par_code,get_type,con_id,sql,constant,current_value)=one
            if get_type=='SQL':
                try:
                    cur=db_con_map[con_id]['cur']
                    value=cur.execute(sql).fetchone()[0]
                except:
                    value=None
                par_map[par_code]=str(value)
                if str(value) != str(current_value):
                    try:
                        tool_cur.execute('''update mgf_parameter set current_value='%s' where project_id=%s and par_code='%s' ''' \
                                         % (str(value),str(self.project_id),par_code))
                        tool_con.commit()
                    except:
                        pass
            elif get_type=='CONSTANT':
                par_map[par_code]=constant
            elif get_type=='COMMAND':
                par_dist=yaml.load(sql)
                ip=par_dist['ip']
                port=par_dist['port']
                user=par_dist['user']
                pwd=par_dist['pwd']
                symbol=par_dist['symbol']
                cmd=par_dist['cmd']
                is_ssh=par_dist['is_ssh']
                if is_ssh=='Y':
                    value=sshdoRes(str(ip),int(port),str(user),str(pwd),str(cmd),str(symbol))
                else:
                    value=telnetCallResult(str(ip),int(port),str(user),str(pwd),str(cmd),str(symbol))
                value=value.strip()
                par_map[par_code]=value
                if str(value) != str(current_value):
                    try:
                        tool_cur.execute('''update mgf_parameter set current_value='%s' where project_id=%s and par_code='%s' ''' \
                                         % (str(value),str(self.project_id),par_code))
                        tool_con.commit()
                    except:
                        pass		    
        return par_map
    def sequence_fix_pro(self,cur,one_seq,table_name_list,col_name_list):
        try:
            log=''
            sql='''select last_number,increment_by,cache_size from user_sequences where sequence_name='%s' ''' % one_seq.upper()
            cur.execute(sql)
            re=cur.fetchall()
            if len(re)==1:
                i,next_number,(last_number,increment_by,cache_size)=0,None,re[0]                
                for table_name in table_name_list:
                    sql='select nvl(max(%s),0) from %s' % (col_name_list[i],table_name)
                    (k,)=cur.execute(sql).fetchone()
                    if next_number==None or k>next_number:
                        next_number=k
                    i=i+1
                #if next_number==None or next_number==0:
                    #return ('OK',one_seq+' not need change.')
                next_number=next_number+increment_by+increment_by
                sql_a='''select %s.Nextval from dual''' % one_seq.upper()
                cur.execute(sql_a)
                sql_a='''select %s.Currval from dual''' % one_seq.upper()
                cur_number=cur.execute(sql_a).fetchone()[0]

                increment_tmp=next_number-cur_number-increment_by
                sql1='''alter sequence %s increment by %s nocache''' % (one_seq.upper(),str(increment_tmp))
                sql2='''select %s.nextval from dual''' % one_seq.upper()
                if cache_size==0:
                    sql3='''alter sequence %s increment by %s nocache''' % (one_seq.upper(),str(increment_by))
                else:
                    sql3='''alter sequence %s increment by %s cache %s''' % (one_seq.upper(),str(increment_by),str(cache_size))                     
                try:
                    if increment_tmp != 0:
                        cur.execute(sql1)
                        cur.execute(sql2)
                    (one_flag,one_log)=('OK',one_seq+' reset next number %s finish.' % str(next_number))
                except cx_Oracle.DatabaseError,e:
                    (one_flag,one_log)=('NO',one_seq+' reset next number error.'+str(e.message))
                finally:
                    cur.execute(sql3)
            else:
                (one_flag,one_log)=('NO',one_seq+' not fund on database.')
            return (one_flag,one_log)
        except BaseException,e:
            return ("NO",str(e.message))

    def do(self,step_id,function_code,function_key_str,execid1,execid2,background):
        start_time=datetime.datetime.now()
        sql_def_m=sql_def()
        try:
            tool_con=cx_Oracle.connect(self.user+'/'+self.pwd+'@'+self.dsn)
        except cx_Oracle.DatabaseError,e:
            print self.user+'/'+self.pwd+'@'+self.dsn,'Connect to Oracle DB failed,xxx',e
        # 获取游标对象
        tool_cur=tool_con.cursor()

        #print par_map
        #新增参数化支持
        sql="select par_ver_code from mgf_project where project_id=%s" % self.project_id
        tool_cur.execute(sql)
        par_ver_code,=tool_cur.fetchone()
        full_par_dist={}
        global_par_dist={}
        sql="select par_code,constant from mgf_parameter a where a.project_id=%s and a.get_type='CONSTANT' and par_ver_code='Global' order by a.par_code" % self.project_id
        tool_cur.execute(sql)        
        for one in tool_cur.fetchall():
            global_par_dist[one[0]]=one[1]        
        #获取local version code
        local_par_dist={}
        sql="select par_code,constant from mgf_parameter a where a.project_id=%s and a.get_type='CONSTANT' and par_ver_code='%s' order by a.par_code" % (self.project_id,par_ver_code)
        tool_cur.execute(sql)        
        for one in tool_cur.fetchall():
            local_par_dist[one[0]]=one[1]
        #合并参数
        keys_list=set(local_par_dist.keys()) | set(global_par_dist.keys())
        for one in keys_list:
            if one in local_par_dist:
                full_par_dist[one]=local_par_dist[one]
            else:
                full_par_dist[one]=global_par_dist[one]
        ###############################
        #初始化连接
        tool_cur.execute(self.sql_def.get_db_con_sql %(self.project_id))
        re=tool_cur.fetchall()
        db_con_map={}
        for one_db in re:
            (project_id,connection_id,connection_name,db_type,user_name,pwd,dsn,comments)=one_db 
            for one in full_par_dist:
                user_name=user_name.replace("${%s}" % one,full_par_dist[one])
                pwd=pwd.replace("${%s}" % one,full_par_dist[one])
                dsn=dsn.replace("${%s}" % one,full_par_dist[one])
            con_dist={}
            con_dist['project_id']=project_id
            con_dist['db_type']=db_type
            con_dist['user_name']=user_name
            con_dist['pwd']=pwd
            con_dist['dsn']=dsn
            if db_type=='Oracle':
                try:		    
                    con=cx_Oracle.connect("%s/%s@%s" % (user_name,pwd,dsn))
                    cur=con.cursor()
                except BaseException,e:
                    con,cur=None,None
                    print "error:629","%s/%s@%s" % (user_name,pwd,dsn),str(e)
            elif db_type=='TimesTen':
                try:
                    con=odbc.odbc('''uid='%s';pwd='%s';dsn='%s' ''' % (user_name,pwd,dsn))
                except BaseException,e:
                    print "error:634",str(e)
            con_dist['con']=con
            con_dist['cur']=cur
            db_con_map[connection_id]=con_dist

        #获取参数列表
        par_re=re_p.compile("\$\{([^\}]*)\}")
        par_code_list=par_re.findall(function_key_str)
        par_map={}
        #参数填充
        for one in par_code_list:
            par_map=self.get_paramter(tool_con,db_con_map,par_map,one,par_ver_code)
        atth1,atth1_name="",""
        if function_code=='RecreateDBTable':
            log_info=[]
            try:
                par_key_dist=yaml.load(function_key_str)
                #table_id=par_key_dist['table_id']
                #table_name=par_key_dist['table_name']
                #table_type=par_key_dist['table_type']
                table_list=par_key_dist['table_list']
                for _z in table_list:
                    table_id,table_name,table_type=_z.split('|')
                    #获取连接ID
                    sql=self.sql_def.get_con_bytab % (table_id)
                    for one in tool_cur.execute(sql).fetchall():
                        con_id=one[0]
                    sql=self.sql_def.get_con_info_byid % con_id
                    db_type=''
                    for one in tool_cur.execute(sql).fetchall():
                        db_type=one[3]

                    con=db_con_map[con_id]['con']
                    cur=db_con_map[con_id]['cur']

                    if db_type=='Oracle':
                        sql='drop table %s purge' % (table_name,)
                        try:
                            cur.execute(sql)
                        except:
                            pass
                        sql='drop table %s' % (table_name,)
                        try:
                            cur.execute(sql)
                        except:
                            pass
                    elif db_type=='TimesTen':
                        sql='drop table %s' % (table_name,)
                        try:
                            cur.execute(sql)
                        except:
                            pass

                    #组装建表语句：
                    sql=self.sql_def.get_tab_col_byid % table_id
                    re=tool_cur.execute(sql).fetchall()
                    if len(re)==0:
                        #end_time=datetime.datetime.now()
                        log_info.append(table_name+' col not define')
                        #(flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,0,'\n'.join(log_info))
                        raise BaseException,(table_name+" col not define",'')

                    #c_sql=',\n'.join([one[3]+' '+one[4] for one in re])
                    tmp_list=[]
                    for one in re:
                        _colstr=one[3]+' '+one[4]
                        if one[5]=='NO':
                            _colstr=_colstr+' not null'
                        tmp_list.append(_colstr)
                    c_sql=',\n'.join(tmp_list)
                    c_sql='create table '+table_name+' \n(\n'+c_sql+' \n)\n'

                    sql=self.sql_def.get_table_info_byid % table_id
                    re=tool_cur.execute(sql).fetchall()
                    table_space=''
                    src_data=''
                    for one in re:
                        table_space=one[5]
                        try:
                            src_data=one[7].read()
                            print src_data
                        except:
                            pass
                    if table_space<>None:
                        c_sql=c_sql+'tablespace '+table_space
                    print c_sql
                    #log=c_sql+' finish.'
                    cur.execute(c_sql)
                    log_info.append(c_sql+' finish.')
                    data=[]
                    try:
                        data=cPickle.loads(src_data)
                    except:
                        pass
                    for one_data in data:
                        sql_insert='insert into '+table_name+' values('+','.join([':'+str(a) for a in range(len(one_data))])+')'
                        cur.execute(sql_insert,one_data)
                    con.commit()
                    log_info.append('insert source data: '+str(len(data))+' rows.')
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,'-1','\n'.join(log_info))
            except BaseException,e:
                end_time=datetime.datetime.now()
                log_info.append(str(e.message))
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,0,'\n'.join(log_info))
        elif function_code=='RecreateDBResources':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                res_list=par_key_dist['res_list']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log=''
                for res_name in res_list:
                    sql='''
		    select sources_id,sources_type,sources_name,created_sql from mgf_db_sources a where a.project_id=%s and sources_name='%s'
		    ''' % (str(self.project_id),res_name)

                    tool_cur.execute(sql)
                    res_info_list=tool_cur.fetchall()
                    for res_info in res_info_list:
                        (sources_id,sources_type,sources_name,created_sql)=res_info
                        try:
                            created_sql=created_sql.read()
                            #获取参数列表
                            par_code_list=par_re.findall(created_sql)
                            #参数填充
                            for one in par_code_list:
                                par_map=self.get_paramter(tool_con,db_con_map,par_map,one,par_ver_code)

                            for one_par in par_map:
                                created_sql=created_sql.replace(r'${'+one_par+r'}',par_map[one_par])	

                            cur.execute(created_sql)
                            (flag,info)=('OK','''recreate %s: %s finish.''' % (sources_type,sources_name))
                        except cx_Oracle.DatabaseError,e:
                            error,=e.args
                            (flag,info)=('NO','''recreate %s: %s failed.%s\n''' % (sources_type,sources_name,e.message))
                        log=log+'\n'+info
                    if flag=='NO':
                        break		    
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))
        elif function_code=='ExecuteSql':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                sql=par_key_dist['sql']
                ignore_error=False
                try:
                    ignore_error=par_key_dist['ignore_error']
                except:
                    pass

                for one_par in par_map:
                    sql=sql.replace(r'${'+one_par+r'}',par_map[one_par])

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']
                try:
                    cur.execute("alter session enable parallel dml")
                except BaseException,e:
                    print str(e)

                cur.execute(sql)
                con.commit()
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,sql+' finish.')
            except cx_Oracle.DatabaseError,e:
                end_time=datetime.datetime.now()
                if ignore_error==True:
                    (flag,step_id,start,end,duration,rows,log)=('WARN',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,0,sql+' '+str(e.message.message))
                else:
                    (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,0,sql+' '+str(e.message.message))
        elif function_code=='ClearTableData':
            try:
                par_key_dist=yaml.load(function_key_str)
                inp_type=par_key_dist['inp_type']
                if inp_type=='By Configuration':
                    table_id=par_key_dist['table_id']
                    table_name=par_key_dist['table_name'].split(',')
                    table_type=par_key_dist['table_type']

                    #获取连接ID
                    sql=self.sql_def.get_con_bytab % (table_id)
                    for one in tool_cur.execute(sql).fetchall():
                        con_id=one[0]			
                elif inp_type == 'Manual Input':
                    con_id=int(par_key_dist['con_id'])
                    clear_table_list=par_key_dist['table_name_list']
                    for one_par in par_map:
                        clear_table_list=clear_table_list.replace(r'${'+one_par+r'}',par_map[one_par])			
                    table_name=clear_table_list.split(',')
                con=db_con_map[con_id]['con']
                cur=db_con_map[con_id]['cur']
                db_type=db_con_map[con_id]['db_type']
                log=''
                for one_table in table_name:
                    clear_db_table=clear_table_data(con,cur,one_table,'Oracle')
                    #clear_db_table.disable_contraints()
                    (one_flag,one_log)=clear_db_table.clear_table()
                    log=log+'\n'+one_log
                    flag=one_flag
                    if flag=='NO':
                        break
                    #clear_db_table.enable_contraints()
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='DisableTrigger':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                trigger_list=['ALTER TRIGGER '+one_tri+' DISABLE' for one_tri in par_key_dist['trigger_list']]

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                for sql in trigger_list:
                    cur.execute(sql)

                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,'Trigger:'+','.join(par_key_dist['trigger_list'])+' disable finish.')
            except cx_Oracle.DatabaseError,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,0,sql+' '+e.message.message)
        elif function_code=='EnableTrigger':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                trigger_list=['ALTER TRIGGER '+one_tri+' ENABLE' for one_tri in par_key_dist['trigger_list']]

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                for sql in trigger_list:
                    cur.execute(sql)

                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,'Trigger:'+','.join(par_key_dist['trigger_list'])+' enable finish.')
            except cx_Oracle.DatabaseError,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,0,sql+' '+e.message.message)	    
        elif function_code=='DisableOraTabFKey':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log_list,flag_list=[],[]
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])					
                    clear_db_table=clear_table_data(con,cur,table_name,db_type)
                    (flag,info)=clear_db_table.disable_contraints()
                    log_list.append(info)
                    if flag=='NO':
                        break	
                log='\n'.join(log_list)
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='EnableOraTabFKey':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log_list,flag_list=[],[]
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])				
                    clear_db_table=clear_table_data(con,cur,table_name,db_type)
                    (flag,info)=clear_db_table.enable_contraints()
                    log_list.append(info)
                    flag_list.append(flag)
                log='\n'.join(log_list)
                if 'NO' in flag_list:
                    flag='NO'
                else:
                    flag='OK'
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='CallOraProcedure':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                ora_proc_name=par_key_dist['ora_proc_name']
                par_list=par_key_dist['par_list']
                par_count=len(par_list[0].split(','))
                if par_list==[u'']:
                    par_count=0

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                bool_map={'yes':True,'no':False}

                if par_count==5:
                    [p1,p2,p3,p4,p5]=[par_list[0].split(',')[0],par_list[0].split(',')[1],par_list[0].split(',')[2],par_list[0].split(',')[3],par_list[0].split(',')[4]]
                    [v1,v2,v3,v4,v5] =[par_list[1].split(',')[0],par_list[1].split(',')[1],par_list[1].split(',')[2],par_list[1].split(',')[3],par_list[1].split(',')[4]]
                    try:
                        v1=bool_map[v1]
                    except:
                        pass
                    try:
                        v2=bool_map[v2]
                    except:
                        pass
                    try:
                        v3=bool_map[v3]
                    except:
                        pass
                    try:
                        v4=bool_map[v4]
                    except:
                        pass
                    try:
                        v5=bool_map[v5]
                    except:
                        pass
                    cur.callproc(ora_proc_name,(),{p1:v1,p2:v2,p3:v3,p4:v4,p5:v5})
                elif par_count==2:
                    [p1,p2]=[par_list[0].split(',')[0],par_list[0].split(',')[1]]
                    [v1,v2] =[par_list[1].split(',')[0],par_list[1].split(',')[1]]
                    try:
                        v1=bool_map[v1]
                    except:
                        pass
                    try:
                        v2=bool_map[v2]
                    except:
                        pass
                    cur.callproc(ora_proc_name,(),{p1:v1,p2:v2})
                elif par_count==3:
                    [p1,p2,p3]=[par_list[0].split(',')[0],par_list[0].split(',')[1],par_list[0].split(',')[2]]
                    [v1,v2,v3] =[par_list[1].split(',')[0],par_list[1].split(',')[1],par_list[1].split(',')[2]]
                    try:
                        v1=bool_map[v1]
                    except:
                        pass
                    try:
                        v2=bool_map[v2]
                    except:
                        pass
                    try:
                        v3=bool_map[v3]
                    except:
                        pass
                    cur.callproc(ora_proc_name,(),{p1:v1,p2:v2,p3:v3})
                elif par_count==4:
                    [p1,p2,p3,p4]=[par_list[0].split(',')[0],par_list[0].split(',')[1],par_list[0].split(',')[2],par_list[0].split(',')[3]]
                    [v1,v2,v3,v4] =[par_list[1].split(',')[0],par_list[1].split(',')[1],par_list[1].split(',')[2],par_list[1].split(',')[3]]
                    try:
                        v1=bool_map[v1]
                    except:
                        pass
                    try:
                        v2=bool_map[v2]
                    except:
                        pass
                    try:
                        v3=bool_map[v3]
                    except:
                        pass
                    try:
                        v4=bool_map[v4]
                    except:
                        pass
                    cur.callproc(ora_proc_name,(),{p1:v1,p2:v2,p3:v3,p4:v4})
                elif par_count==1:
                    [p1]=[par_list[0].split(',')[0]]
                    [v1] =[par_list[1].split(',')[0]]
                    try:
                        v1=bool_map[v1]
                    except:
                        pass
                    cur.callproc(ora_proc_name,(),{p1:v1})
                elif par_count==0:
                    cur.callproc(ora_proc_name,(),{})
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,'Call '+ora_proc_name+' Finish.')
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,'Call '+ora_proc_name+' '+str(e.message))
        elif function_code=='FtpFile':
            try:
                par_key_dist=yaml.load(function_key_str)
                ftp_type=par_key_dist['ftp_type']
                is_sftp=par_key_dist['is_sftp']
                if ftp_type=='GET':
                    info=""
                    r_ip=par_key_dist['r_ip']
                    r_port=par_key_dist['r_port']
                    r_user=par_key_dist['r_user']
                    r_pwd=par_key_dist['r_pwd']
                    r_file_path=par_key_dist['r_file_path']
                    r_file_name_str=par_key_dist['r_file_name']
                    r_file_name_list=r_file_name_str.split(",")
                    l_file_path=par_key_dist['l_file_path']
                    #hostaddr, username, password, remotedir, port=21
                    print r_ip,r_user,r_pwd,r_file_path,int(r_port)

                    for one_par in par_map:
                        r_ip=r_ip.replace(r'${'+one_par+r'}',par_map[one_par])
                        r_user=r_user.replace(r'${'+one_par+r'}',par_map[one_par])
                        r_pwd=r_pwd.replace(r'${'+one_par+r'}',par_map[one_par])
                        r_port=r_port.replace(r'${'+one_par+r'}',par_map[one_par])
                    if is_sftp == 'Y':
                        ftp=MYSFTP(r_ip,r_user,r_pwd,r_file_path,int(r_port))
                    else:
                        ftp=MYFTP(r_ip,r_user,r_pwd,r_file_path,int(r_port))
                    (flag,info_one)=ftp.login()
                    info=info_one
                    if flag=='NO':
                        end_time=datetime.datetime.now()
                        (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,1,info)			    
                    for r_file_name in r_file_name_list:
                        if r_file_name=="":
                            continue
                        (flag,info_one)=ftp.download_file(l_file_path+'/'+r_file_name,r_file_path+'/'+r_file_name)
                        info=info+"\n"+info_one
                        if flag=="NO":
                            break
                    end_time=datetime.datetime.now()
                    (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,1,info)
                if ftp_type=='PUT':
                    r_ip=par_key_dist['r_ip']
                    r_port=par_key_dist['r_port']
                    r_user=par_key_dist['r_user']
                    r_pwd=par_key_dist['r_pwd']

                    for one_par in par_map:
                        r_ip=r_ip.replace(r'${'+one_par+r'}',par_map[one_par])
                        r_user=r_user.replace(r'${'+one_par+r'}',par_map[one_par])
                        r_pwd=r_pwd.replace(r'${'+one_par+r'}',par_map[one_par])
                        r_port=r_port.replace(r'${'+one_par+r'}',par_map[one_par])

                    r_file_path=par_key_dist['r_file_path']		    
                    l_file_path=par_key_dist['l_file_path']
                    l_file_name_str=par_key_dist['l_file_name']
                    l_file_name_list=l_file_name_str.split(",")

                    if is_sftp == 'Y':
                        ftp=MYSFTP(r_ip,r_user,r_pwd,r_file_path,int(r_port))
                    else:
                        ftp=MYFTP(r_ip,r_user,r_pwd,r_file_path,int(r_port))
                    (flag,info_one)=ftp.login()
                    info=info_one
                    if flag=='NO':
                        end_time=datetime.datetime.now()
                        (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,1,info)			    
                    for l_file_name in l_file_name_list:
                        if l_file_name=="":
                            continue			
                        (flag,info_one)=ftp.upload_file(os.path.join(l_file_path,l_file_name),r_file_path+'/'+l_file_name)
                        info=info+"\n"+info_one
                        if flag=="NO":
                            break
                    end_time=datetime.datetime.now()
                    (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,1,info)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))		    
        elif function_code=='ImportDBData':
            par_key_dist=yaml.load(function_key_str)
            load_type=par_key_dist['load_type']
            table_id=par_key_dist['table_id']
            table_name=par_key_dist['table_name']
            table_type=par_key_dist['table_type']
            sql=self.sql_def.get_tab_col_byid % table_id
            re=tool_cur.execute(sql).fetchall()
            sqlldr_par=[]
            for _ab in re:
                (table_id,column_id,col_seq,col_name,col_type,null_able,comments)=_ab
                if col_type=='DATE':
                    _a=col_name+''' "TO_DATE(substr(:%s,1,19),'YYYY-MM-DD HH24:MI:SS')" ''' % col_name
                    if par_key_dist.has_key('date_format'):
                        if par_key_dist['date_format']=='YYYYMMDDHH24MISS':
                            _a=col_name+''' "TO_DATE(substr(:%s,1,14),'YYYYMMDDHH24MISS')" ''' % col_name
                    sqlldr_par.append(_a)
                else:
                    sqlldr_par.append(col_name)
            col_list_str=',\n'.join([one for one in sqlldr_par])
            check_list_count=len(re)
            insert_sql='insert into '+table_name+'('+','.join([one[3] for one in re])+') values('+','.join([':'+str(_one) for _one in range(len(re))])+')'
            #获取连接ID
            sql=self.sql_def.get_con_bytab % (table_id)
            for one in tool_cur.execute(sql).fetchall():
                con_id=one[0]
            try:
                con=db_con_map[con_id]['con']
                cur=db_con_map[con_id]['cur']
                user=db_con_map[con_id]['user_name']
                pwd=db_con_map[con_id]['pwd']
                dsn=db_con_map[con_id]['dsn']
                db_type=db_con_map[con_id]['db_type']
            except:
                pass

            cur.execute('select * from %s where 1=2' % table_name)
            _desc = cur.description
            _item_li = []
            _a={}
            i=0
            for one_item in _desc:
                (col_name,col_type,_x,_y,_z,_xy,_yz) = one_item
                _a[col_name]=i
                i+=1
                if col_type == cx_Oracle.BLOB or col_type == cx_Oracle.CLOB or col_type == cx_Oracle.LONG_STRING or col_type == cx_Oracle.LONG_BINARY or col_type==cx_Oracle.UNICODE or col_type == cx_Oracle.TIMESTAMP or col_type==cx_Oracle.STRING:
                    _item_li.append((col_name,col_type))
                        
            if load_type=='From Excel File':
                info=""
                xls_folder=par_key_dist['xls_folder']
                xls_name=par_key_dist['xls_name']

                xls_reader=read_xls(os.path.join(xls_folder,xls_name),check_list_count)
                rec_list=xls_reader.read_excel()
                flag='NO'
                try:
                    cur.execute("alter session set nls_date_format = 'yyyy-mm-dd hh24:mi:ss' ")
                except:
                    pass
                try:
                    xx=0
                    '''for one_row in rec_list:
                        try:
                            cur.execute(insert_sql,one_row)
                            xx+=1
                        except BaseException,e:
                            xx+=1
                            info=info[-1000:]+"rows:%d:%s" % (xx,str(e))
                    con.commit()'''
                    for one_list in chunks(rec_list,1000):
                        array_len=len(one_list)
                        cur.bindarraysize = array_len
                        one_blob_dist={}
                        for one_blob in _item_li:
                            (col_name,col_type) = one_blob
                            if col_type==cx_Oracle.UNICODE:
                                col_type=cx_Oracle.STRING
                            one_blob_dist[col_name]=cur.var(col_type,array_len)
                        _d=[]
                        for row in enumerate(one_list):
                            idx,one_re=row
                            _blob_list=list(one_re)
                            for one_blob in _item_li:
                                (col_name,col_type) = one_blob
                                line_num=_a[col_name]
                                one_blob_dist[col_name].setvalue(idx,_blob_list[line_num])
                                _blob_list.__setitem__(line_num,one_blob_dist[col_name])
                            _d.append(tuple(_blob_list))
                        cur.executemany(insert_sql,_d)
                        con.commit()
                        xx=xx+array_len
                    if len(rec_list)==xx:
                        flag='OK'
                        info=info+os.path.join(xls_folder,xls_name)+' import to table:'+table_name+' finish.'			
                        con.commit()			
                        end_time=datetime.datetime.now()
                        (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,xx,info)
                    else:
                        info=info+os.path.join(xls_folder,xls_name)+' import to table:'+table_name+' failed.'
                        info=info+"\nsrc rows:%d, load rows:%d" % (len(rec_list),xx)
                        end_time=datetime.datetime.now()
                        (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,len(rec_list),info)
                except BaseException,e:
                    end_time=datetime.datetime.now()
                    (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e))                    
            elif load_type=='From txt File':
                try:
                    txt_folder=par_key_dist['txt_folder']
                    txt_name=par_key_dist['txt_name']
                    split_str=par_key_dist['split_str']

                    txt_file=open(os.path.join(txt_folder,txt_name),'rb')
                    i,j,res_list=0,0,[]
                    for one_line in txt_file:
                        line_list=one_line.strip().split(split_str)
                        res_list.append(tuple(line_list))
                        if i == 100000:
                            try:
                                cur.executemany(insert_sql,res_list)
                            except:
                                for one_rec in res_list:
                                    cur.execute(insert_sql,one_rec)
                                    #print one_rec
                            con.commit()
                            i,res_list=0,[]
                        i=i+1
                        j=j+1
                    if len(res_list)>0:
                        try:
                            cur.executemany(insert_sql,res_list)
                        except:
                            for one_rec in res_list:
                                cur.execute(insert_sql,one_rec)
                                #print one_rec
                        con.commit()
                    txt_file.close()
                    end_time=datetime.datetime.now()
                    info=os.path.join(txt_folder,txt_name)+' import to table:'+table_name+' finish.'
                    (flag,step_id,start,end,duration,rows,log)=('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,j,info)
                except BaseException,e:
                    end_time=datetime.datetime.now()
                    info=os.path.join(txt_folder,txt_name)+' import to table:'+table_name+' failed.'
                    (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,info+str(e.message))
            elif load_type=='By Ora. sqlldr':
                if db_type.upper() != 'ORACLE':
                    end_time=datetime.datetime.now()
                    info='It is not Oracle Database.'
                    (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,info)			
                try:
                    sqlldr_file=par_key_dist['sqlldr_file']
                    sqlldr_file_split=par_key_dist['sqlldr_file_split']
                    #ctl file
                    ctl_str='''load data
CHARACTERSET AL32UTF8
append into table %s
fields terminated by '%s'
TRAILING NULLCOLS
(
%s         
)''' % (table_name,sqlldr_file_split,col_list_str)
                    ctl_file_name,sqldr_log_file_name=table_name+'_sqlldr.ctl',table_name+'_sqlldr.log'
                    ctl_file=open(ctl_file_name,'wb')
                    try:
                        ctl_def=par_key_dist['ctl_def']
                    except:
                        ctl_def=""
                    if ctl_def:
                        ctl_file.write(ctl_def)
                    else:
                        ctl_file.write(ctl_str)
                    ctl_file.close()
                    try:
                        skip=par_key_dist['skip']
                    except:
                        skip='0'
                    sqlldr_cmd='''sqlldr %s/%s@%s control=%s data=%s log=%s direct=y parallel=true skip=%s''' % (user,pwd,dsn,ctl_file_name,sqlldr_file,sqldr_log_file_name,skip)
                    run_cmd=subprocess.Popen(sqlldr_cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True)
                    (log,err)=run_cmd.communicate()
                    retcode=run_cmd.wait()
                    try:
                        log=open(sqldr_log_file_name,'rb').read()
                    except:
                        pass
                    log='\n'.join([str(log),str(err)])
                    log=log[-4000:]
                    if retcode != 0:
                        (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,start_time,(start_time-start_time).days*24*3600+(start_time-start_time).seconds,-1,str(sqlldr_cmd)+' exec failed'+str(log))

                    sql='select count(*) from '+table_name
                    total_count=cur.execute(sql).fetchone()[0]

                    end_time=datetime.datetime.now()
                    info=sqlldr_file+' import to table:'+table_name+' finish.'
                    (flag,step_id,start,end,duration,rows,log)=('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,total_count,info+str(log))
                except BaseException,e:
                    end_time=datetime.datetime.now()
                    info=sqlldr_file+' import to table:'+table_name+' failed.'
                    (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,info+str(e.message))
            elif load_type=='By TT. ttBulkCp':
                tmp_a=''
                db_type=par_key_dist['db_type']
                if db_type.upper() != 'TIMESTEN':
                    end_time=datetime.datetime.now()
                    info='It is not TimesTen Database.'
                    (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,info)			
                try:
                    ttbulkcp_file=par_key_dist['ttbulkcp_file']
                    ttbulkcp_table_name=par_key_dist['tt_table_name']
                    db_con_str=par_key_dist['db_con_str']
                    ttBulkCp_cmd='''ttBulkCp -i -notblLock -connStr "%s" %s %s''' % (db_con_str,ttbulkcp_table_name,ttbulkcp_file)
                    #print ttBulkCp_cmd
                    run_cmd=subprocess.Popen(ttBulkCp_cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,shell=True)
                    (a,err)=run_cmd.communicate()
                    retcode=run_cmd.wait()
                    tmp_a='\n'.join([str(a),str(err)])
                    end_time=datetime.datetime.now()
                    if retcode != 0:
                        (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(ttBulkCp_cmd)+' exec failed,'+str(tmp_a))
                    else:			
                        info=ttbulkcp_file+' import to table:'+ttbulkcp_table_name+' finish.'
                        (flag,step_id,start,end,duration,rows,log)=('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,info+str(tmp_a))
                except BaseException,e:
                    end_time=datetime.datetime.now()
                    info=ttbulkcp_file+' import to table:'+ttbulkcp_table_name+' failed.'
                    (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,info+str(e.message)+str(tmp_a))
            elif load_type=='From DB Select':
                try:
                    load_sql=par_key_dist['sql']
                    load_con_id=int(par_key_dist['db_con_id'])

                    load_con=db_con_map[load_con_id]['con']
                    load_cur=db_con_map[load_con_id]['cur']

                    re=load_cur.execute(load_sql)

                    i,j,res_list=0,0,[]
                    for one in re:
                        res_list.append(one)
                        if i == 100000:
                            try:
                                cur.executemany(insert_sql,res_list)
                            except:
                                for one_rec in res_list:
                                    cur.execute(insert_sql,one_rec)
                                    print one_rec
                            con.commit()
                            i,res_list=0,[]
                        i=i+1
                        j=j+1
                    if len(res_list)>0:
                        try:
                            cur.executemany(insert_sql,res_list)
                        except:
                            for one_rec in res_list:
                                cur.execute(insert_sql,one_rec)
                                print one_rec
                        con.commit()
                    end_time=datetime.datetime.now()
                    info=load_sql+' import to table:'+table_name+' finish.'
                    (flag,step_id,start,end,duration,rows,log)= ('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,j,info)
                except BaseException,e:
                    end_time=datetime.datetime.now()
                    info=load_sql+' import to table:'+table_name+' failed.'
                    (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,info+str(e.message))		

        elif function_code=='ExportDBData':
            par_key_dist=yaml.load(function_key_str)
            exp_type=par_key_dist['exp_type']
            db_con_id=int(par_key_dist['db_con_id'])
            load_sql=par_key_dist['load_sql']
            if not par_key_dist.has_key("tformat"):
                par_key_dist['tformat']="YYYY-MM-DD HH24:MI:SS"
            for one_par in par_map:
                load_sql=load_sql.replace(r'${'+one_par+r'}',par_map[one_par])			

            con=db_con_map[db_con_id]['con']
            cur=db_con_map[db_con_id]['cur']

            if exp_type=='To Excel':
                xls_name=par_key_dist['xls_name']		    
                #打开游标
                try:
                    re=cur.execute(load_sql)
                    xls_write=write_xls(xls_name,'Sheet1')
                    i,j,write_list=0,0,[]
                    title=tuple([one[0] for one in cur.description])
                    xls_write.write_excel([title])
                    for one in re:
                        write_list.append(one)
                        i=i+1
                        if i==10000:
                            xls_write.write_excel(write_list)
                            i=0
                            write_list=[]
                        j=j+1
                    if len(write_list)>0:
                        xls_write.write_excel(write_list)
                    xls_write.excel_save()
                    end_time=datetime.datetime.now()
                    log=load_sql+'\n'+'Export to excel: '+xls_name+' finish.'
                    (flag,step_id,start,end,duration,rows,log)= ('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,j,log)
                except BaseException,e:
                    end_time=datetime.datetime.now()
                    log=load_sql+'\n'+'Export to excel: '+xls_name+' Failed.'
                    (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,log+str(e.message))
            elif exp_type=='To TxtFile,SplitStr=\',\'' or exp_type=='To TxtFile,SplitStr=\'|\'':
                txt_name=par_key_dist['txt_name']	
                split_str=par_key_dist['split_str']
                tformat=par_key_dist['tformat']	
                #打开游标
                try:
                    re=cur.execute(load_sql)
                    w_file=open(txt_name,'wb')
                    i,j,write_list=0,0,[]
                    _apd1=write_list.append
                    #title=tuple([one[0] for one in cur.description])
                    tmp_list=[]
                    _apd2=tmp_list.append
                    for one in re:
                        j+=1
                        tmp_list.__init__()
                        for one_col in one:
                            if one_col==None:
                                one_col='NULL'
                            elif type(one_col)==datetime.datetime:
                                if tformat=="YYYY-MM-DD HH24:MI:SS":
                                    one_col=str(one_col)[:19]#.strftime("%Y-%m-%d %H:%M:%S")
                                elif tformat=="YYYYMMDDHH24MISS":
                                    one_col=str(one_col)[:19].replace("-","").replace(":","").replace(" ","")
                                    #one_col=one_col.strftime("%Y%m%d%H%M%S")
                            else:
                                one_col=str(one_col)
                            _apd2(one_col)
                        line=split_str.join(tmp_list)
                        w_file.write("%s\n" % line)
                    w_file.close()
                    end_time=datetime.datetime.now()
                    log=load_sql+'\n'+'Export to file: '+txt_name+' finish.'
                    (flag,step_id,start,end,duration,rows,log)= ('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,j,log)
                except BaseException,e:
                    end_time=datetime.datetime.now()
                    log=load_sql+'\n'+'Export to file: '+txt_name+' Failed.'
                    (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,log+str(e.message))

            elif exp_type=='To DB Table':
                to_db_con_id=int(par_key_dist['to_db_con_id'])
                to_table_id=par_key_dist['to_table_id']
                to_table_name=par_key_dist['to_table_name']

                to_db_con=db_con_map[to_db_con_id]['con']
                to_db_cur=db_con_map[to_db_con_id]['cur']

                sql=self.sql_def.get_tab_col_byid % to_table_id
                re=tool_cur.execute(sql).fetchall()
                col_list_str=',\n'.join([one[3] for one in re])
                check_list_count=len(re)
                insert_sql='insert into '+to_table_name+'('+','.join([one[3] for one in re])+') values('+','.join([':'+str(_one) for _one in range(len(re))])+')'

                #打开游标
                try:
                    re=cur.execute(load_sql)
                    i,j,res_list=0,0,[]
                    for one in re:
                        res_list.append(one)
                        if i == 100000:
                            try:
                                to_db_cur.executemany(insert_sql,res_list)
                            except:
                                for one_rec in res_list:
                                    to_db_cur.execute(insert_sql,one_rec)
                                    print one_rec
                            to_db_con.commit()
                            i,res_list=0,[]
                        i=i+1
                        j=j+1
                    if len(res_list)>0:
                        try:
                            to_db_cur.executemany(insert_sql,res_list)
                        except:
                            for one_rec in res_list:
                                to_db_cur.execute(insert_sql,one_rec)
                                print one_rec
                        to_db_con.commit()
                    end_time=datetime.datetime.now()
                    info=load_sql+' export to table:'+to_table_name+' finish.'
                    (flag,step_id,start,end,duration,rows,log)= ('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,j,info)
                except BaseException,e:
                    end_time=datetime.datetime.now()
                    info=load_sql+' export to table:'+to_table_name+' failed.'
                    (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,info+str(e.message))	
            elif exp_type=='To Sql File':
                txt_name=par_key_dist['txt_name']	
                sql_file_tablename=par_key_dist['sql_file_tablename']	
                w_file=open(txt_name,'wb')

                #打开游标
                try:
                    re=cur.execute(load_sql)
                    col_list=[one[0] for one in cur.description]
                    col_list_str=','.join(col_list)
                    export_sql='''insert into %s(%s) values (%s);\n'''
                    i,j,write_list=0,0,[]
                    #title=tuple([one[0] for one in cur.description])
                    for one in re:
                        tmp_list=[]
                        for one_col in one:
                            if one_col==None:
                                one_col='NULL'
                            elif type(one_col) in [cx_Oracle.BLOB,cx_Oracle.CLOB,cx_Oracle.LONG_STRING,cx_Oracle.LONG_BINARY]:
                                one_col=one_col.read()
                                one_col=one_col.replace("'","''")
                                one_col="'%s'" % one_col
                            elif type(one_col)==datetime.datetime:			
                                one_col="to_date('%s','yyyy-mm-dd hh24:mi:ss')" % one_col.strftime("%Y-%m-%d %H:%M:%S")
                            elif type(one_col) in [int,long,float]:
                                one_col=str(one_col)
                            else:
                                one_col=str(one_col)
                                one_col=one_col.replace("'","''")
                                one_col="'%s'" % one_col

                            tmp_list.append("%s"% str(one_col))
                        #line=",".join(tmp_list)
                        line=export_sql % (sql_file_tablename,col_list_str,",".join(tmp_list))
                        #line=','.join([str(one_col) for one_col in one]).replace('\n',' ')+'\n'
                        write_list.append(line)

                        i=i+1
                        if i==10000:
                            w_file.writelines(write_list)
                            i=0
                            write_list=[]
                        j=j+1
                    if len(write_list)>0:
                        w_file.writelines(write_list)
                    w_file.close()
                    end_time=datetime.datetime.now()
                    log=load_sql+'\n'+'Export to file: '+txt_name+' finish.'
                    (flag,step_id,start,end,duration,rows,log)= ('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,j,log)
                except BaseException,e:
                    end_time=datetime.datetime.now()
                    log=load_sql+'\n'+'Export to file: '+txt_name+' Failed.'
                    (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,log+str(e.message))
        elif function_code=='SqlReturnNbrAudit':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                sql=par_key_dist['sql']
                count_min=par_key_dist['count_min']
                count_max=par_key_dist['count_max']
                for one_par in par_map:
                    sql=sql.replace(r'${'+one_par+r'}',par_map[one_par])			    
                    count_min=count_min.replace(r'${'+one_par+r'}',par_map[one_par])
                    count_max=count_max.replace(r'${'+one_par+r'}',par_map[one_par])

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                cur.execute(sql)
                count=cur.fetchone()[0]
                end_time=datetime.datetime.now()
                if str(count).isdigit()==True:
                    if count>=long(count_min) and count<=long(count_max):
                        info=sql+', return count is '+str(count)+', in range ['+str(count_min)+','+str(count_max)+']'
                        (flag,step_id,start,end,duration,rows,log)= ('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,count,info)
                    else:
                        info=sql+', return count is '+str(count)+', not in range ['+str(count_min)+','+str(count_max)+']'
                        (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,count,info)
                    con.commit()
                else:
                    (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,'sql return value is not number type,'+str(count))
            except cx_Oracle.DatabaseError,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,0,sql+' '+str(e.message.message))
        elif function_code=='CallSystemCMD':		
            try:
                par_key_dist=yaml.load(function_key_str)
                cmd=par_key_dist['cmd']
                for one_par in par_map:
                    cmd=cmd.replace(r'${'+one_par+r'}',par_map[one_par])			    
                run_cmd=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,shell=True)
                (log,err)=run_cmd.communicate()
                retcode=run_cmd.wait()
                end_time=datetime.datetime.now()
                if retcode != 0:
                    (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,'\n'.join([str(cmd),str(log)]))
                else:
                    (flag,step_id,start,end,duration,rows,log)= ('OK',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,'\n'.join([str(cmd),str(log)]))
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,cmd+','+str(e.message))
        elif function_code=='TryDropOraTable':		
            try:
                par_key_dist=yaml.load(function_key_str)		
                con_id=int(par_key_dist['con_id'])
                table_name=par_key_dist['table_name_list'].split(',')
                con=db_con_map[con_id]['con']
                cur=db_con_map[con_id]['cur']
                db_type=db_con_map[con_id]['db_type']
                log=''
                for one_table in table_name:
                    for one_par in par_map:
                        one_table=one_table.replace(r'${'+one_par+r'}',par_map[one_par])				
                    clear_db_table=clear_table_data(con,cur,one_table,'Oracle')
                    (one_flag,one_log)=clear_db_table.drop_table()
                    log=log+'\n'+one_log
                    flag=one_flag
                    if flag=='NO':
                        break
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= (flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))
        elif function_code=='AnalyseOraTable':		
            try:
                par_key_dist=yaml.load(function_key_str)		
                con_id=int(par_key_dist['con_id'])
                table_name=par_key_dist['table_name_list'].split(',')
                try:
                    parallel_val=par_key_dist['parallel_val']
                except:
                    parallel_val='4'
                con=db_con_map[con_id]['con']
                cur=db_con_map[con_id]['cur']
                owner=db_con_map[con_id]['user_name']
                db_type=db_con_map[con_id]['db_type']
             
                log=''
                for one_table in table_name:
                    for one_par in par_map:
                        one_table=one_table.replace(r'${'+one_par+r'}',par_map[one_par])			
                    try:
                        cur.callproc("DBMS_STATS.GATHER_TABLE_STATS",(),{'ownname':owner,'tabname':one_table,'estimate_percent':10,'DEGREE':parallel_val,'CASCADE':True,'no_invalidate':False})
                        (one_flag,one_log)=('OK',one_table+' table analyse finish.')
                    except cx_Oracle.DatabaseError,e:
                        (one_flag,one_log)=('NO',one_table+' table analyse error.'+str(e.message))
                    log=log+'\n'+one_log
                    flag=one_flag
                    if flag=='NO':
                        break
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= (flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='CallRemoteCMD':		
            try:
                par_key_dist=yaml.load(function_key_str)		
                host=par_key_dist['ip']
                port=par_key_dist['port']
                user=par_key_dist['user']
                pwd=par_key_dist['pwd']
                cmd=par_key_dist['cmd']
                finish=par_key_dist['finish']
                is_ssh=par_key_dist['is_ssh']

                for one_par in par_map:
                    host=host.replace(r'${'+one_par+r'}',par_map[one_par])
                    user=user.replace(r'${'+one_par+r'}',par_map[one_par])
                    pwd=pwd.replace(r'${'+one_par+r'}',par_map[one_par])
                    port=port.replace(r'${'+one_par+r'}',par_map[one_par])
                    cmd=cmd.replace(r'${'+one_par+r'}',par_map[one_par])
                    finish=finish.replace(r'${'+one_par+r'}',par_map[one_par])

                if is_ssh=="Y":
                    print str(host),str(port),str(user),str(pwd),str(cmd),str(finish)
                    (flag,log)=sshdo(str(host),str(port),str(user),str(pwd),str(cmd),str(finish))
                else:
                    (flag,log)=telnetdo(str(host),str(port),str(user),str(pwd),str(cmd),str(finish))
                log=log[-4000:]
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= (flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='ResetSeqNextVal':		
            try:
                par_key_dist=yaml.load(function_key_str)
                if par_key_dist.has_key('fix_type'):
                    fix_type=par_key_dist['fix_type']
                else:
                    fix_type='Fixed Value'
                if fix_type=='Fixed Value':
                    con_id=int(par_key_dist['con_id'])
                    seq_list=par_key_dist['seq_list'].split(',')
                    next_number=par_key_dist['next_number']
                    for one_par in par_map:
                        next_number=next_number.replace(r'${'+one_par+r'}',par_map[one_par])		
                    next_number=long(next_number)
                    con=db_con_map[con_id]['con']
                    cur=db_con_map[con_id]['cur']
                    owner=db_con_map[con_id]['user_name']
                    db_type=db_con_map[con_id]['db_type']
                    log=''
                    for one_seq in seq_list:
                        sql='''select last_number,increment_by,cache_size from user_sequences where sequence_name='%s' ''' % one_seq.upper()
                        cur.execute(sql)
                        re=cur.fetchall()
                        if len(re)==1:
                            sql_a='''select %s.Nextval from dual''' % one_seq.upper()
                            cur.execute(sql_a)
                            sql_a='''select %s.Currval from dual''' % one_seq.upper()
                            cur_number=cur.execute(sql_a).fetchone()[0]
                            (last_number,increment_by,cache_size)=re[0]
                            increment_tmp=next_number-cur_number-increment_by
                            sql1='''alter sequence %s increment by %s nocache''' % (one_seq.upper(),str(increment_tmp))
                            sql2='''select %s.nextval from dual''' % one_seq.upper()
                            if cache_size==0:
                                sql3='''alter sequence %s increment by %s nocache''' % (one_seq.upper(),str(increment_by))
                            else:
                                sql3='''alter sequence %s increment by %s cache %s''' % (one_seq.upper(),str(increment_by),str(cache_size))			    
                            try:
                                if increment_tmp != 0:
                                    cur.execute(sql1)
                                    cur.execute(sql2)
                                (one_flag,one_log)=('OK',one_seq+' reset next number %s finish.' % str(next_number))
                            except cx_Oracle.DatabaseError,e:
                                (one_flag,one_log)=('NO',one_seq+' reset next number error.'+str(e.message))
                            finally:
                                cur.execute(sql3)
                        else:
                            (one_flag,one_log)=('NO',one_seq+' not fund on database.')
                        log=log+'\n'+one_log
                        flag=one_flag
                        if flag=='NO':
                            break
                elif fix_type=='Reference Value':
                    con_id=int(par_key_dist['con_id'])
                    ref_seq_list=par_key_dist['ref_seq_list']
                    ref_seq_list=yaml.load(ref_seq_list)
                    con=db_con_map[con_id]['con']
                    cur=db_con_map[con_id]['cur']
                    owner=db_con_map[con_id]['user_name']
                    db_type=db_con_map[con_id]['db_type']
                    log_list=[]
                    for ab in ref_seq_list:
                        one_seq=ab.keys()[0]
                        seq_table_list=ab[one_seq]['table']
                        seq_col_list=ab[one_seq]['column']
                        flag,log_tmp=self.sequence_fix_pro(cur,one_seq,seq_table_list,seq_col_list)
                        log_list.append(log_tmp)
                        if flag=='NO':
                            break
                    log='\n'.join(log_list)
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= (flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))
        elif function_code=='DropOraTabKey':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log,key_list='',[]
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])
                    sql='''
		    select a.table_name, a.key_name, a.create_sql
		    from mgf_table_key a, mgf_all_table b
		    where a.table_id = b.table_id
		    and b.table_name = '%s'
		    and b.connection_id = %s
		    and a.key_type in('_R')
		    order by a.key_seq
		    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    for _xy in tool_cur.fetchall():
                        if _xy not in key_list:
                            key_list.append(_xy)

                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])
                    sql='''
		    select a.table_name, a.key_name, a.create_sql
		    from mgf_table_key a, mgf_all_table b
		    where a.table_id = b.table_id
		    and b.table_name = '%s'
		    and b.connection_id = %s
		    and a.key_type in('R')
		    order by a.key_seq
		    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    for _xy in tool_cur.fetchall():
                        if _xy not in key_list:
                            key_list.append(_xy)

                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])
                    sql='''
		    select a.table_name, a.key_name, a.create_sql
		    from mgf_table_key a, mgf_all_table b
		    where a.table_id = b.table_id
		    and b.table_name = '%s'
		    and b.connection_id = %s
		    and a.key_type in('P','U','C')
		    order by a.key_seq
		    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    for _xy in tool_cur.fetchall():
                        if _xy not in key_list:
                            key_list.append(_xy)


                for key in key_list:
                    (table_name,key_name,create_sql)=key
                    try:
                        cur.execute('''ALTER TABLE %s drop CONSTRAINT %s cascade''' % (table_name,key_name))
                        (flag,info)=('OK','''drop table %s key %s finish.''' % (table_name,key_name))
                    except cx_Oracle.DatabaseError,e:
                        error,=e.args
                        print table_name,key_name,str(e.message)
                        if str(error.code)=='2443':
                            (flag,info)=('OK','''drop table %s key %s finish.''' % (table_name,key_name)+str(e.message))				
                        else:
                            (flag,info)=('NO','''drop table %s key %s failed.''' % (table_name,key_name)+str(e.message))
                            log=log+'\n'+info
                            break
                    try:
                        cur.execute('''drop index %s''' % key_name)
                    except cx_Oracle.DatabaseError,e:
                        pass
                    log=log+'\n'+info
                    if flag=='NO':
                        break		    
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))			    
        elif function_code=='CreateOraTabKey':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log,key_list='',[]
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])
                    sql='''
		    select a.table_name, a.key_name, a.create_sql
		    from mgf_table_key a, mgf_all_table b
		    where a.table_id = b.table_id
		    and b.table_name = '%s'
		    and b.connection_id = %s
		    and a.key_type in('P','U','C')
		    order by a.key_seq
		    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    for _xy in tool_cur.fetchall():
                        if _xy not in key_list:
                            key_list.append(_xy)

                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])
                    sql='''
		    select a.table_name, a.key_name, a.create_sql
		    from mgf_table_key a, mgf_all_table b
		    where a.table_id = b.table_id
		    and b.table_name = '%s'
		    and b.connection_id = %s
		    and a.key_type in('_R')
		    order by a.key_seq
		    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    for _xy in tool_cur.fetchall():
                        if _xy not in key_list:
                            key_list.append(_xy)

                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])
                    sql='''
		    select a.table_name, a.key_name, a.create_sql
		    from mgf_table_key a, mgf_all_table b
		    where a.table_id = b.table_id
		    and b.table_name = '%s'
		    and b.connection_id = %s
		    and a.key_type in('R')
		    order by a.key_seq
		    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    for _xy in tool_cur.fetchall():
                        if _xy not in key_list:
                            key_list.append(_xy)

                for key in key_list:
                    (table_name,key_name,create_sql)=key
                    try:
                        cur.execute(create_sql)
                        (flag,info)=('OK','''create table %s key %s finish.''' % (table_name,key_name))
                    except cx_Oracle.DatabaseError,e:
                        error,=e.args
                        print str(e.message)
                        if str(error.code) in ['2261','2275','2260']:
                            (flag,info)=('OK','''create table %s key %s finish.''' % (table_name,key_name)+str(e.message))
                        else:
                            (flag,info)=('NO','''create table %s key %s failed.''' % (table_name,key_name)+str(e.message))
                            log=log+'\n'+info
                            break	
                    log=log+'\n'+info
                    if flag=='NO':
                        break		    
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))			    
        elif function_code=='DisableOraTabPKey':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log_list=[]
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])					
                    clear_db_table=clear_table_data(con,cur,table_name,db_type)
                    (flag,info)=clear_db_table.disable_pk()		    
                    log_list.append(info)
                    if flag=='NO':
                        break		
                log='\n'.join(log_list)
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	

        elif function_code=='EnableOraTabPKey':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']
                index_tablespace=par_key_dist['index_tablespace']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log_list=[]
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])					
                    clear_db_table=clear_table_data(con,cur,table_name,db_type)
                    (flag,info)=clear_db_table.enable_pk(index_tablespace)		    
                    log_list.append(info)
                    if flag=='NO':
                        break		
                log='\n'.join(log_list)
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='DropOraTabIdx':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log=''
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])
                    sql='''
		    select b.table_name,a.idx_name,a.create_sql from mgf_table_idx a,mgf_all_table b 
		    where a.table_id=b.table_id and b.table_name='%s' and b.connection_id=%s order by a.idx_seq
		    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    idx_list=tool_cur.fetchall()
                    for idx in idx_list:
                        (table_name,idx_name,create_sql)=idx
                        try:
                            cur.execute('''drop index %s''' % idx_name)
                            (flag,info)=('OK','''drop table %s index %s finish.''' % (table_name,idx_name))
                        except cx_Oracle.DatabaseError,e:
                            error,=e.args
                            if str(error.code)=='1418':
                                (flag,info)=('OK','''drop table %s index %s finish.''' % (table_name,idx_name)+str(e.message))
                            else:
                                (flag,info)=('NO','''drop table %s index %s failed.''' % (table_name,idx_name)+str(e.message))
                                break
                        log=log+'\n'+info
                    if flag=='NO':
                        break		    
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='CreateOraTabIdx':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']
                try:
                    parallel_val=par_key_dist['parallel_val']
                except:
                    parallel_val='0'

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log=''
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])
                    sql='''
                    select b.table_name,a.idx_name,a.create_sql from mgf_table_idx a,mgf_all_table b 
                    where a.table_id=b.table_id and b.table_name='%s' and b.connection_id=%s order by a.idx_seq
                    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    idx_list=tool_cur.fetchall()
                    for idx in idx_list:
                        (table_name,idx_name,create_sql)=idx
                        try:
                            if parallel_val<>'0':
                                cur.execute(create_sql+" parallel %s" % parallel_val)
                                cur.execute("alter index %s noparallel" % idx_name)
                            else:
                                cur.execute(create_sql)
                            (flag,info)=('OK','''create table %s index %s finish.\nsql:%s''' % (table_name,idx_name,create_sql))
                            log=log+'\n'+info
                        except cx_Oracle.DatabaseError,e:
                            error,=e.args
                            if str(error.code)=='955':
                                (flag,info)=('OK','''create table %s index %s finish.\nsql:%s\n%s''' % (table_name,idx_name,create_sql,str(e)))
                                log=log+'\n'+info
                            else:
                                (flag,info)=('NO','''create table %s index %s failed.\nsql:%s\n%s''' % (table_name,idx_name,create_sql,str(e)))
                                log=log+'\n'+info
                                break
                    if flag=='NO':
                        break
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='NologgingOraTable':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log_list=[]
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])					
                    clear_db_table=clear_table_data(con,cur,table_name,db_type)
                    (flag,info)=clear_db_table.nologging_table()		    
                    log_list.append(info)
                    if flag=='NO':
                        break		
                log='\n'.join(log_list)
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	

        elif function_code=='LoggingOraTable':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                log_list=[]
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])					
                    clear_db_table=clear_table_data(con,cur,table_name,db_type)
                    (flag,info)=clear_db_table.logging_table()		    
                    log_list.append(info)
                    if flag=='NO':
                        break		
                log='\n'.join(log_list)
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='CheckOraSeq':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                map_str=par_key_dist['map_str']
                try:
                    fix_flag=par_key_dist['fix_flag']
                except:
                    fix_flag="N"
                seq_map={}
                seq_map=yaml.load(map_str)


                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']		    
                log_failed_list,log_sucess_list,flag_list,log=[],[],[],''
                for seq in seq_map:
                    seq_name=seq.keys()[0]
                    r_table_name=seq[seq_name]['table']
                    r_table_col=seq[seq_name]['column']

                    try:
                        sql='''select nvl(max(%s),0) from %s''' % (r_table_col,r_table_name)
                        max_col=cur.execute(sql).fetchone()[0]
                    except cx_Oracle.DatabaseError,e:
                        log_failed_list.append(sql+':'+str(e.message))
                        max_col=0
                    try:
                        sql='''select nvl(max(a.last_number),0) from user_sequences a where a.sequence_name='%s' ''' % (seq_name,)
                        res=cur.execute(sql).fetchone()
                        seq_last_value=res[0]
                    except cx_Oracle.DatabaseError,e:
                        seq_last_value=0
                        log_failed_list.append(sql+':'+str(e.message))
                    if max_col>=seq_last_value:
                        if fix_flag=="Y":
                            (one_seq,table_name,col_name)=seq_name,r_table_name,r_table_col
                            try:
                                log=''
                                sql='''select last_number,increment_by,cache_size from user_sequences where sequence_name='%s' ''' % one_seq.upper()
                                cur.execute(sql)
                                re=cur.fetchall()
                                if len(re)==1:
                                    (last_number,increment_by,cache_size)=re[0]
                                    sql='select nvl(max(%s),0) from %s' % (col_name,table_name)
                                    (next_number,)=cur.execute(sql).fetchone()
                                    next_number=next_number+increment_by
                                    sql_a='''select %s.Nextval from dual''' % one_seq.upper()
                                    cur.execute(sql_a)
                                    sql_a='''select %s.Currval from dual''' % one_seq.upper()
                                    cur_number=cur.execute(sql_a).fetchone()[0]

                                    increment_tmp=next_number-cur_number-increment_by
                                    sql1='''alter sequence %s increment by %s nocache''' % (one_seq.upper(),str(increment_tmp))
                                    sql2='''select %s.nextval from dual''' % one_seq.upper()
                                    if cache_size==0:
                                        sql3='''alter sequence %s increment by %s nocache''' % (one_seq.upper(),str(increment_by))
                                    else:
                                        sql3='''alter sequence %s increment by %s cache %s''' % (one_seq.upper(),str(increment_by),str(cache_size))			    
                                    try:
                                        if increment_tmp != 0:
                                            cur.execute(sql1)
                                            cur.execute(sql2)
                                        (one_flag,one_log)=('OK',(u'%s: Next Val:%s,Max ID %s.%s:%s;' %(seq_name,str(seq_last_value),r_table_name,r_table_col,str(max_col)))+(one_seq+' reset next number %s finish.' % str(next_number)))
                                    except cx_Oracle.DatabaseError,e:
                                        (one_flag,one_log)=('NO',(u'%s: Next Val:%s,Max ID %s.%s:%s;' %(seq_name,str(seq_last_value),r_table_name,r_table_col,str(max_col)))+(one_seq+' reset next number error.'+str(e.message)))
                                    finally:
                                        cur.execute(sql3)
                                else:
                                    (one_flag,one_log)=('NO',one_seq+' not fund on database.')
                            except BaseException,e:
                                (one_flag,one_log)=("NO",one_seq+" error: "+str(e.message))			
                        else:
                            one_flag="NO"			    
                            one_log=u'%s: Next Val:%s,Max ID %s.%s:%s' %(seq_name,str(seq_last_value),r_table_name,r_table_col,str(max_col))			    
                    else:
                        one_flag="OK"
                        one_log=u'%s: Next Val:%s,Max ID %s.%s:%s' %(seq_name,str(seq_last_value),r_table_name,r_table_col,str(max_col))
                    flag_list.append(one_flag)
                    if one_flag=="OK":
                        log_sucess_list.append(one_log)
                    else:
                        log_failed_list.append(one_log)

                log_ok='\n'.join(log_sucess_list)
                log_no='\n'.join(log_failed_list)
                log='*********************FAILED LIST*********************\n'+log_no+'\n\n*********************SUCESS LIST*********************\n'+log_ok
                if 'NO' in flag_list:
                    flag='NO'
                else:
                    flag='OK'
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='CompileOraInvalidObj':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                log_failed_list,log_sucess_list,flag_list,log=[],[],[],''
                sql_get_invalid='''select a.OBJECT_NAME,a.OBJECT_TYPE from user_objects a where a.status='INVALID' '''
                re=cur.execute(sql_get_invalid).fetchall()
                for one_obj in re:
                    (object_name,object_type)=one_obj
                    if object_type in ['VIEW','SYNONYM','PROCEDURE','PACKAGE','FUNCTION','TRIGGER']:
                        compile_sql='''alter %s %s compile''' %(object_type,object_name)
                    elif object_type in ['PACKAGE BODY']:
                        compile_sql='''alter PACKAGE %s compile BODY''' % object_name
                    elif object_type in ['UNDEFINED']:
                        compile_sql='''alter materizlized view %s compile''' % object_name	
                    elif object_type in ['JAVA CLASS']:
                        compile_sql='''alter java class %s resolve''' % object_name
                    elif object_type in ['TYPE BODY']:
                        compile_sql='''alter type %s compile body''' % object_name
                    else:
                        continue

                    try:
                        cur.execute(compile_sql)			
                        sql_check='''select count(*) from user_objects where object_name='%s' and object_type='%s' and status='VALID' ''' % (object_name,object_type)
                        if cur.execute(sql_check).fetchone()[0]==0:
                            flag_list.append('NO')
                            log_failed_list.append('object name: %s,obecjt type: %s conmpile failed,please check.' % (object_name,object_type)) 
                        else:
                            flag_list.append('WARN')
                            log_sucess_list.append('object name: %s,obecjt type: %s conmpile sucess.' % (object_name,object_type)) 
                    except cx_Oracle.DatabaseError,e:
                        flag_list.append('WARN')
                        log_failed_list.append('object name: %s,obecjt type: %s conmpile failed,' % (object_name,object_type)+str(e.message))

                log_ok='\n'.join(log_sucess_list)
                log_no='\n'.join(log_failed_list)
                log='*********************FAILED LIST*********************\n'+log_no+'\n\n*********************SUCESS LIST*********************\n'+log_ok
                if 'WARN' in flag_list:
                    flag='WARN'
                if 'NO' in flag_list:
                    flag='NO'
                if len(log_failed_list)==0 and len(log_sucess_list)==0:
                    flag='OK'
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='GenKPIFromDB':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                xls_template_data=par_key_dist['xls_template_data']
                kpi_file_name=par_key_dist['kpi_file_name']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']

                log_list=[]
                #生成临时template文件
                xls_template=open("xls_template.xls","wb")
                xls_template.write(xls_template_data)
                xls_template.close()

                gen_kpi=xls4tep("xls_template.xls")
                gen_kpi.write_xls_by_tmp(cur,kpi_file_name)
                atth1_name=os.path.split(kpi_file_name)[1]
                atth1=tool_cur.var(cx_Oracle.BLOB)
                _fp=open(kpi_file_name,"rb")
                tmp=_fp.read()
                _fp.close()
                atth1.setvalue(0,tmp)
                flag='OK'
                log_list.append("Generate kpi %s Finish." % kpi_file_name)
                log='\n'.join(log_list)
                con.close()
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,cur.rowcount,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e))
            finally:
                try:
                    os.remove("xls_template.xls")
                except:
                    pass
        elif function_code=='Separatrix':
            flag="OK"
            try:
                par_key_dist=yaml.load(function_key_str)
                need_man_review=par_key_dist['need_man_review']
                if need_man_review:
                    flag="REVIEW"
            except:
                pass
            end_time=datetime.datetime.now()
            (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,'Nothing,this is separatrix.')	
        elif function_code=='TxtFileFilter':
            try:
                par_key_dist=yaml.load(function_key_str)

                con_id=par_key_dist['db_con_id']
                filter_from=par_key_dist['filter_from']
                filter_to=par_key_dist['filter_to']
                filter_split=par_key_dist['filter_split']
                filter_index=par_key_dist['filter_index']
                sql_content_filter=par_key_dist['sql_content_filter']
                trim=par_key_dist['trim']
                filter_index=int(filter_index)
                parallel=par_key_dist['paral']
                file_lines=par_key_dist['file_lines']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']

                db_type=db_con_map[int(con_id)]['db_type']
                res=cur.execute(sql_content_filter).fetchall()
                filter_in={}
                for one in res:
                    filter_in[str(one[0])]=1

                if parallel and os.name<>"nt":
                    _path,_spfile=os.path.split(filter_from)
                    #清理拆分临时文件
                    for one in os.listdir(_path):
                        if _spfile+".sp." in one and one[-5:]<>".dest":
                            try:
                                os.remove(os.path.join(_path,one))
                            except BaseException,e:
                                print "remove file %s error" % one,str(e.message)
                                pass
                    #fsplitcmd="split -l 10000000 /bill/zqs/mig_tool/v62.bal /bill/zqs/mig_tool/spfile. "
                    fsplitcmd="split -l %s %s %s " % (file_lines,filter_from,filter_from+".sp.")
                    run_cmd=subprocess.Popen(fsplitcmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,shell=True)
                    (log,err)=run_cmd.communicate()
                    retcode=run_cmd.wait()
                    #print "retcode",retcode
                    if retcode == 0:
                        th={}
                        for one in os.listdir(_path):
                            if _spfile+".sp." in one and one[-5:]<>".dest":
                                if os.name=="nt":
                                    th[one]=threading.Thread(target=file_filter,kwargs={'filter_from':_path+'/'+one,'key_index':filter_index,'split_str':filter_split,'map_dist':filter_in,'filter_to':_path+'/'+one+".dest",'trim':trim})
                                    th[one].setDaemon(True)
                                else:
                                    th[one]=multiprocessing.Process(target=file_filter,kwargs={'filter_from':_path+'/'+one,'key_index':filter_index,'split_str':filter_split,'map_dist':filter_in,'filter_to':_path+'/'+one+".dest",'trim':trim})				

                        for one in th:
                            th[one].start()
                        while True:
                            live=0
                            for one in th:
                                #print th[one]
                                if th[one].is_alive():
                                    live+=1
                            if live==0:
                                #print "break,while"
                                break
                            time.sleep(1)
                        #清理拆分临时文件
                        for one in os.listdir(_path):
                            if _spfile+".sp." in one and one[-5:]<>".dest":
                                try:
                                    os.remove(os.path.join(_path,one))
                                except BaseException,e:
                                    print "remove file %s error" % one,str(e.message)
                                    pass
                        try:
                            os.remove(filter_to)
                        except BaseException,e:
                            print "remove dest file %s failed,if not exists please skip this." % filter_to,e.message
                            pass
                        #合并结果
                        for one in os.listdir(_path):
                            if _spfile+".sp." in one and one[-5:]==".dest":
                                cmd="cat %s >> %s;rm %s" % (_path+"/"+one,filter_to,_path+"/"+one)
                                run_cmd=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,shell=True)
                                (log,err)=run_cmd.communicate()
                                retcode=run_cmd.wait()
                                if retcode==0:
                                    flag,log,count=("OK","%s merge sucess." % filter_to,-1)
                                else:
                                    flag,log,count=("NO","%s merge failed." % filter_to,-1)
                                    break
                        #查行数
                        if flag=="OK":
                            cmd="wc -l %s" % filter_to
                            run_cmd=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,shell=True)
                            (res_count,err)=run_cmd.communicate()
                            retcode=run_cmd.wait()
                            if retcode==0:
                                count=res_count.split()[0].strip()
                            else:
                                count=-1
                    else:
                        flag,log,count=("NO","%s split failed." % filter_from,-1)
                else:
                    flag,log,count=file_filter(filter_from,filter_index,filter_split,filter_in,filter_to,trim)

                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,count,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='ExecPython':		
            try:
                par_key_dist=yaml.load(function_key_str)		
                pyscr=par_key_dist['python_script']
                log=pyscr+"\n"
                try:
                    # create file-like string to capture output
                    codeOut = StringIO.StringIO()
                    codeErr = StringIO.StringIO()
                    try:
                        # capture output and errors
                        sys.stdout = codeOut
                        sys.stderr = codeErr
                        exec pyscr
                    except BaseException,e:
                        raise e
                    finally:
                        # restore stdout and stderr
                        sys.stdout = sys.__stdout__
                        sys.stderr = sys.__stderr__        
                    log = log+codeOut.getvalue()
                    log = log+codeErr.getvalue()
                    codeOut.close()
                    codeErr.close()
                    flag="OK"
                except BaseException,e:
                    flag="NO"
                    log=log+str(e.args)+"\n"+str(e.message)

                log=log[-4000:]
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= (flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        elif function_code=='HugeTxtSplit':		
            try:
                log,flag="","OK"
                par_key_dist=yaml.load(function_key_str)		
                input_split_file_name=par_key_dist['input_split_file_name']
                input_split_count=par_key_dist['input_split_count']

                #查行数
                cmd="wc -l %s" % input_split_file_name
                run_cmd=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,shell=True)
                (res_count,err)=run_cmd.communicate()
                retcode=run_cmd.wait()
                if retcode==0:
                    fileRows=int(res_count.split()[0].strip())
                    #print "fileRows,",fileRows
                else:
                    fileRows=-1
                log=log+'\n'+str(res_count)

                if fileRows > 0:
                    tmp=os.system("rm %s.*" % input_split_file_name)
                    if tmp==0:
                        log=log+"\nrm %s.* exec sucess" % input_split_file_name
                    each_rows=(fileRows+int(input_split_count)-1)/int(input_split_count)
                    fsplitcmd="split -l %s %s %s " % (each_rows,input_split_file_name,input_split_file_name+".")
                    run_cmd=subprocess.Popen(fsplitcmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,shell=True)
                    (tmp,err)=run_cmd.communicate()
                    log=log+str(tmp)
                    retcode=run_cmd.wait()
                    #print "retcode",retcode
                    if retcode == 0:
                        _path=os.path.split(input_split_file_name)[0]
                        _file=os.path.split(input_split_file_name)[1]
                        i = 0
                        for one in os.listdir(_path):
                            if _file+"." in one and one[-3]==".":
                                to=one[:-2]+str(i)
                                os.rename(os.path.join(_path,one),os.path.join(_path,to))
                                log=log+"\n%s Split Finish" % to
                                i+=1
                    else:
                        flag='NO'
                else:
                    flag='NO'
                log=log[-4000:]
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= (flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)= ('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))
        elif function_code=='TableConsistentCheck':
            try:
                par_key_dist=yaml.load(function_key_str)
                con_id=par_key_dist['db_con_id']
                table_list=par_key_dist['table_list']

                #获取连接ID
                con=db_con_map[int(con_id)]['con']
                cur=db_con_map[int(con_id)]['cur']
                user_name=db_con_map[int(con_id)]['user_name']

                db_type=db_con_map[int(con_id)]['db_type']
                log,flag='',"OK"
                log_info_list=[]
                for table_name in table_list:
                    for one_par in par_map:
                        table_name=table_name.replace(r'${'+one_par+r'}',par_map[one_par])
                    #表字段信息一致性检查
                    sql='''
                    select a.col_name,a.col_type,a.null_able
                    from mgf_table_col a, mgf_all_table b
                    where a.table_id = b.table_id
                    and b.table_name = '%s'
                    and b.connection_id = %s
                    order by a.col_seq
                    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    col_list=tool_cur.fetchall()
                    if col_list:
                        db_sql='''select a.COLUMN_NAME,
                        a.DATA_TYPE,
                        a.DATA_LENGTH,
                        a.DATA_PRECISION,
                        a.DATA_SCALE,
                        a.NULLABLE,
                        b.comments
                        from user_tab_columns a, user_col_comments b
                        where a.TABLE_NAME = '%s'
                        and a.COLUMN_NAME = b.column_name(+)
                        and a.TABLE_NAME = b.table_name(+)
                        order by a.COLUMN_ID''' % (table_name.upper().strip())
                        cur.execute(db_sql)
                        db_col_res=cur.fetchall()
                        db_col_list=[]
                        for one_col in db_col_res:
                            (COLUMN_NAME,DATA_TYPE,DATA_LENGTH,DATA_PRECISION,DATA_SCALE,NULLABLE,COMMENTS)=one_col
                            if DATA_TYPE=='NUMBER':
                                if DATA_SCALE !=0 and DATA_SCALE != None:
                                    col_type=DATA_TYPE+'('+str(DATA_PRECISION)+','+str(DATA_SCALE)+')'
                                elif DATA_PRECISION:
                                    col_type=DATA_TYPE+'('+str(DATA_PRECISION)+')'
                                else:
                                    col_type=DATA_TYPE
                            elif DATA_TYPE=='VARCHAR2' or DATA_TYPE=='CHAR' or DATA_TYPE=='RAW':
                                col_type=DATA_TYPE+'('+str(DATA_LENGTH)+')'
                            else:
                                col_type=DATA_TYPE
                            if NULLABLE=='Y':
                                NULLABLE='YES'
                            else:
                                NULLABLE='NO'
                            if COMMENTS==None:
                                COMMENTS=''
                            COMMENTS=unicode(COMMENTS)
                            db_col_list.append((COLUMN_NAME,col_type,NULLABLE))
                        if db_col_list <> col_list:
                            flag="NO"
                            log_info_list.append("-------------TABLE %s COLUMN INFO IS DIFFERENT-----------" % table_name.upper().strip())
                            log_info_list.append("TOOL SAVED:  "+str(set(col_list)-set(db_col_list)))
                            log_info_list.append("DATABASE IS: "+str(set(db_col_list)-set(col_list)))
                        else:
                            #flag="OK"
                            log_info_list.append("-------------TABLE %s COLUMN INFO IS SAME-----------" % table_name.upper().strip())                       
                    #表索引一致性检查
                    sql='''
                    select a.idx_name,a.create_sql
                    from mgf_table_idx a, mgf_all_table b
                    where a.table_id = b.table_id
                    and b.table_name = '%s'
                    and b.connection_id = %s
                    order by a.idx_seq
                    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    idx_list=tool_cur.fetchall()
                    db_idx_list=[]
                    if idx_list:
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'CONSTRAINTS_AS_ALTER','value':False})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'REF_CONSTRAINTS','value':False})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'SQLTERMINATOR','value':False})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'STORAGE','value':False})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'CONSTRAINTS','value':True})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'SEGMENT_ATTRIBUTES','value':True})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'TABLESPACE','value':True})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'PRETTY','value':True})                        
                        db_sql='''
                        select a.index_name,
                        dbms_metadata.get_ddl(UPPER('INDEX'), a.index_name) sql
                        from user_indexes a
                        where a.table_name = '%s'
                        and a.index_type<>'LOB' 
                        and a.index_name not in
                        (select b.constraint_name
                        from user_constraints b
                        where b.table_name = '%s'
                        and b.constraint_type in ('P','U'))
                        ''' % (table_name.upper().strip(),table_name.upper().strip())
                        cur.execute(db_sql)
                        re_idx=cur.fetchall()
                        space=re_p.compile(' +')
                        for one in re_idx:
                            (IDX_NAME,SQL)=(one[0],one[1].read())		    
                            SQL=SQL.replace('"'+user_name.upper()+'".','')
                            SQL=SQL.replace('"','')
                            SQL=SQL.replace('\n',' ')
                            SQL=SQL.replace('  ',' ')
                            SQL=space.sub(' ',SQL)
                            SQL=SQL.strip()
                            db_idx_list.append((IDX_NAME,SQL))                        
                        if set(db_idx_list) <> set(idx_list):
                            flag="NO"
                            log_info_list.append("-------------TABLE %s INDEX INFO IS DIFFERENT-----------" % table_name.upper().strip())
                            log_info_list.append("TOOL SAVED:  "+str(set(idx_list)-set(db_idx_list)))
                            log_info_list.append("DATABASE IS: "+str(set(db_idx_list)-set(idx_list)))
                        else:
                            #flag="OK"
                            log_info_list.append("-------------TABLE %s INDEX INFO IS SAME-----------" % table_name.upper().strip())                             
                    #表主外键一致性检查
                    sql='''
                    select a.table_name,a.key_name,a.key_type,a.create_sql
                    from mgf_table_key a, mgf_all_table b
                    where a.table_id = b.table_id
                    and b.table_name = '%s'
                    and b.connection_id = %s
                    order by a.key_seq
                    ''' % (table_name.upper(),con_id)
                    tool_cur.execute(sql)
                    key_list=tool_cur.fetchall()
                    db_key_list=[]
                    if key_list:
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'CONSTRAINTS_AS_ALTER','value':False})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'REF_CONSTRAINTS','value':False})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'SQLTERMINATOR','value':False})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'STORAGE','value':False})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'CONSTRAINTS','value':True})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'SEGMENT_ATTRIBUTES','value':True})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'TABLESPACE','value':True})
                        cur.callproc("dbms_metadata.set_transform_param",(),{'transform_handle':-1,'name':'PRETTY','value':True})                        
                        sql='''
                        select a.constraint_name,(dbms_metadata.get_ddl('CONSTRAINT', a.constraint_name)),a.constraint_type,a.table_name
                        from user_constraints a
                        where a.constraint_type in ('P','U','C')
                        and table_name = '%s'
                        and generated = 'USER NAME'
                        order by constraint_type desc
                        ''' % (table_name.upper().strip(),)
                        cur.execute(sql)
                        re_key=[]
                        for one in cur.fetchall():
                            re_key.append((one[0],one[1].read(),one[2],one[3]))
        
                        sql='''
                        select ur.constraint_name,
                        (dbms_metadata.get_ddl('REF_CONSTRAINT', ur.constraint_name)),
                        'R' constraint_type,
                        ur.table_name
                        from user_constraints ur
                        where ur.CONSTRAINT_TYPE = 'R'
                        and TABLE_NAME = '%s'
                        and ur.r_constraint_name not in
                        (select a.constraint_name
                        from user_constraints a
                        where a.constraint_type in ('P', 'U', 'C')
                        and a.generated = 'USER NAME'
                        and table_name = '%s')
                        ''' % (table_name.upper().strip(),table_name.upper().strip())
                        cur.execute(sql)
                        for one in cur.fetchall():
                            re_key.append((one[0],one[1].read(),one[2],one[3]))		
        
                        sql='''
                        select ur.constraint_name,
                        (dbms_metadata.get_ddl('REF_CONSTRAINT', ur.constraint_name)),
                        '_R' constraint_type,
                        ur.table_name
                        from user_constraints ur
                        where ur.CONSTRAINT_TYPE = 'R'
                        and ur.R_CONSTRAINT_NAME in
                        (select ur.CONSTRAINT_NAME
                        from user_constraints ur
                        where ur.CONSTRAINT_TYPE in ('U', 'P')
                        and ur.TABLE_NAME = '%s')
                        order by constraint_name
                        ''' % (table_name.upper().strip(),)
                        cur.execute(sql)
                        for one in cur.fetchall():
                            re_key.append((one[0],one[1].read(),one[2],one[3]))
        
                        space=re_p.compile(' +')
                        for one in re_key:
                            (TABLE_NAME,PK_NAME,SQL,TYPE)=(one[3],one[0],one[1],one[2])		    
                            SQL=SQL.replace('"'+user_name.upper()+'".','')
                            SQL=SQL.replace('"','')
                            SQL=SQL.replace('\n',' ')
                            SQL=SQL.replace('\t',' ')
                            SQL=SQL.replace('  ',' ')
                            SQL=space.sub(' ',SQL)
                            SQL=SQL.strip()
                            SQL=SQL.replace('DISABLE','ENABLE')
                            db_key_list.append((TABLE_NAME,PK_NAME,TYPE,SQL))                                                
                        if set(db_key_list) <> set(key_list):
                            flag="NO"
                            log_info_list.append("-------------TABLE %s P/FKEY INFO IS DIFFERENT-----------" % table_name.upper().strip())
                            log_info_list.append("TOOL SAVED:  "+str(set(key_list)-set(db_key_list)))
                            log_info_list.append("DATABASE IS: "+str(set(db_key_list)-set(key_list)))
                        else:
                            #flag="OK"
                            log_info_list.append("-------------TABLE %s P/FKEY INFO IS SAME-----------" % table_name.upper().strip())                               
                log="\n".join(log_info_list)
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=(flag,step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,log)
            except BaseException,e:
                end_time=datetime.datetime.now()
                (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,str(e.message))	
        
        else:
            end_time=datetime.datetime.now()
            (flag,step_id,start,end,duration,rows,log)=('NO',step_id,start_time,end_time,(end_time-start_time).days*24*3600+(end_time-start_time).seconds,-1,'This function [%s] can not support,Please check.' % function_code)
        #更新目录状态
        if step_id>0:
            try:
                sql=sql_def_m.get_mig_catg_by_stepid % step_id
                mig_catg_id=tool_cur.execute(sql).fetchone()[0]
            except:
                pass

        if flag=='OK' or flag=='WARN' or flag=="REVIEW":
            sql=sql_def_m.get_remove_mig_log % step_id
            for one in tool_cur.execute(sql).fetchall():
                log_detail=tool_cur.var(cx_Oracle.CLOB)
                tmp=''
                try:
                    tmp=one[5].read()
                except:
                    pass
                log_detail.setvalue(0,tmp)
                atth1_detail=tool_cur.var(cx_Oracle.BLOB)
                tmp=''
                try:
                    tmp=one[8].read()
                except:
                    pass
                atth1_detail.setvalue(0,tmp)		
                tool_cur.execute(sql_def_m.insert_mig_log_his,(int_decode(one[0])+1,one[1],one[2],one[3],one[4],log_detail,one[7],atth1_detail,one[9]))
                tool_cur.execute(sql_def_m.del_remove_mig_log % step_id)
            insert_sql=sql_def_m.insert_mig_log#step_id,start_time,end_time,log_detail,rec_num
            tool_cur.execute(insert_sql,(step_id,start,end,log,rows,atth1,atth1_name))
            if flag=='OK':
                state='FINISH'
            elif flag=='WARN':
                state='WARN'
            elif flag=='REVIEW':
                state='REVIEW'
            update_sql=sql_def_m.set_mig_step_state_one % (state,step_id)
            tool_cur.execute(update_sql)
            tool_con.commit()

            #更新run成
            sql=sql_def_m.set_mig_catg_state_one_by_catg % ('RUN',str(mig_catg_id))
            tool_cur.execute(sql)
            tool_con.commit()

            #更新FINISH成
            sql=sql_def_m.check_catg_is_finish % (str(mig_catg_id),)
            check_finish_row=tool_cur.execute(sql).fetchone()[0]
            if check_finish_row==0:		
                sql=sql_def_m.set_mig_catg_state_one_by_catg % ('FINISH',str(mig_catg_id))
                tool_cur.execute(sql)
                tool_con.commit()

        elif flag=='NO':
            sql=sql_def_m.get_remove_mig_log % step_id
            for one in tool_cur.execute(sql).fetchall():
                log_detail=tool_cur.var(cx_Oracle.CLOB)
                try:
                    val=one[5].read()
                except:
                    val=""
                log_detail.setvalue(0,val)		
                atth1_detail=tool_cur.var(cx_Oracle.BLOB)
                tmp=''
                try:
                    tmp=one[8].read()
                except:
                    pass
                atth1_detail.setvalue(0,tmp)			
                tool_cur.execute(sql_def_m.insert_mig_log_his,(int_decode(one[0])+1,one[1],one[2],one[3],one[4],log_detail,one[7],atth1_detail,one[9]))
                tool_cur.execute(sql_def_m.del_remove_mig_log % step_id)
            insert_sql=sql_def_m.insert_mig_log#step_id,start_time,end_time,log_detail,rec_num
            tool_cur.execute(insert_sql,(step_id,start,end,log,rows,atth1,atth1_name))
            update_sql=sql_def_m.set_mig_step_state_one % ('ERROR',step_id)
            tool_cur.execute(update_sql)
            tool_con.commit()
            #更新ERROR成
            sql=sql_def_m.set_mig_catg_state_one_by_catg % ('ERROR',str(mig_catg_id))
            tool_cur.execute(sql)
            tool_con.commit()
        log_w.write('debug',log)

        #数据库连接退出
        try:
            tool_con.close()
        except BaseException,e:
            print str(e)
        for _xx in db_con_map:
            try:
                db_con_map[_xx]['con'].close()
            except BaseException,e:
                print str(e)

    def run_next_step(self):
        for one_flow in ['f0','f1','f2']:
            start_time=datetime.datetime.now()
            #print one_flow
            try:
                #self.init_paramter()
                check_stop_cnt=0
                step_id=-1
                start_time=datetime.datetime.now()
                time.sleep(0.1)
                sql='''
		select count(1) from mgf_mig_flow_step where state in ('ERROR') and step_catg_id in 
		(select step_catg_id from mgf_mig_flow_catg where project_id = %s)
		''' % (self.project_id)
                try:
                    check_stop_cnt=int_decode(self.cur.execute(sql).fetchone()[0])
                except:
                    pass
                if check_stop_cnt>0:
                    continue
                    print ('ERROR',-1,start_time,start_time,0,0,'ERROR :: Has running or error step.')

                #self.init_paramter()
                sql='''
		select *
		from (select a.step_id,
		a.function_code,
		a.func_par_list,
		c.exec_order_id as exec_id1,
		a.exec_order_id as exec_id2,
		a.background,
		nvl(a.flow,'f0') flow,
		a.depend_flow
		from mgf_mig_flow_step a, mgf_mig_flow_catg c
		where a.step_catg_id = c.step_catg_id
		and c.project_id = %s
		and a.state = 'WAIT'
		and nvl(a.flow,'f0')='%s'
		order by c.exec_order_id, a.exec_order_id)
		where rownum = 1
		and 'A' = (select state from mgf_project where project_id = %s)
		''' % (self.project_id,one_flow,self.project_id)
                #获取下个割接节点
                try:
                    re=self.cur.execute(sql).fetchall()
                except BaseException,e:
                    re=[]
                    #print 'get next step failed,',str(e.message)
                    self.log.write("debug",'get next step failed,'+str(e.message))
                    pass

                for one in re:
                    (step_id,function_code,function_key_str,execid1,execid2,background,flow,depend_flow) = one	    
                    function_key_str=function_key_str.read()
                    sql='''
		    select count(1) from mgf_mig_flow_step where state in ('RUN') and step_catg_id in 
		    (select step_catg_id from mgf_mig_flow_catg where project_id = %s and nvl(flow,'f0')='%s')
		    ''' % (self.project_id,one_flow)
                    try:
                        check_stop_cnt=int_decode(self.cur.execute(sql).fetchone()[0])
                    except:
                        pass
                    if check_stop_cnt>0:
                        continue
                        print ('ERROR',-1,start_time,start_time,0,0,'ERROR :: Has running or error step.')
                    #子进程处理时需要判断之前的主进程步骤是否已经结束
                    if one_flow <>"f0":
                        sql='''
			select count(*)
			  from mgf_mig_flow_step a, mgf_mig_flow_catg b
			 where a.step_catg_id = b.step_catg_id
			   and b.project_id = %s
			   and nvl(flow, 'f0') = 'f0'
			   and a.state in ('WAIT', 'RUN', 'BACK')
			   and (b.exec_order_id < %s or
			       (b.exec_order_id = %s and a.exec_order_id < %s))
			''' % (self.project_id,str(execid1),str(execid1),str(execid2))
                        try:
                            check_stop_cnt5=int_decode(self.cur.execute(sql).fetchone()[0])
                        except:
                            pass
                        if check_stop_cnt5>0:
                            continue
                            print ('ERROR',-1,start_time,start_time,0,0,'ERROR :: Has running or error step.')			
                    sql1='''select count(*)
		    from mgf_mig_flow_step a, mgf_mig_flow_catg b
		    where a.step_catg_id = b.step_catg_id
		    and b.project_id = %s
		    and a.state in ('PAUSE','REVIEW')
		    and (b.exec_order_id < %s or
		    (b.exec_order_id = %s and a.exec_order_id < %s))
		    ''' % (self.project_id,str(execid1),str(execid1),str(execid2))
                    try:
                        check_stop_cnt6=int_decode(self.cur.execute(sql1).fetchone()[0])
                    except BaseException,e:
                        pause_rec=[]
                        check_stop_cnt6=0
                        print str(e)
                        pass
                    if check_stop_cnt6>0:
                        print ('ERROR',-1,start_time,start_time,0,0,'ERROR :: Has pause or review step.')
                        continue

                    #判断依赖流程是否停止
                    if depend_flow<>None:
                        sql='''
			select count(*)
			from mgf_mig_flow_step a, mgf_mig_flow_catg b
			where a.state in ('WAIT', 'RUN', 'BACK')
			and a.step_catg_id in
			(select step_catg_id from mgf_mig_flow_catg where project_id = %s)
			and nvl(flow, 'f0') in (%s)
			and a.step_catg_id = b.step_catg_id
			and (b.exec_order_id < %s or
			(b.exec_order_id = %s and a.exec_order_id < %s))
			''' % (self.project_id,",".join(["'"+xz+"'" for xz in depend_flow.split(',')]),str(execid1),str(execid1),str(execid2))
                        try:
                            check_stop_cnt=int_decode(self.cur.execute(sql).fetchone()[0])
                        except:
                            pass
                        if check_stop_cnt>0:
                            print ('ERROR',-1,start_time,start_time,0,0,'ERROR :: depend flow has running step.')
                            continue

                    if background=='Y':
                        sql=self.sql_def.set_step_back_state % step_id
                    else:
                        chk_sql='''select count(1) from mgf_mig_flow_step where state in ('BACK') and step_catg_id in 
	    (select step_catg_id from mgf_mig_flow_catg where project_id = %s and nvl(flow,'f0')='%s')''' % (self.project_id,one_flow)
                        check_back_cnt=int_decode(self.cur.execute(chk_sql).fetchone()[0])
                        if check_back_cnt>0:
                            print ('ERROR',-1,start_time,start_time,0,0,'ERROR :: This flow has on back step.')
                            continue
                        sql=self.sql_def.set_step_run_state % step_id
                    self.cur.execute(sql)	
                    self.con.commit()
                    th_map={}
                    if os.name=="nt":
                        th_map[step_id]=threading.Thread(target=self.do,kwargs={'step_id':step_id,'function_code':function_code,'function_key_str':function_key_str,'execid1':execid1,'execid2':execid2,'background':background})
                        th_map[step_id].setDaemon(True)
                    else:
                        th_map[step_id]=multiprocessing.Process(target=self.do,kwargs={'step_id':step_id,'function_code':function_code,'function_key_str':function_key_str,'execid1':execid1,'execid2':execid2,'background':background})

                    th_map[step_id].start()		    
                    if background=='Y' :
                        print ('BACK',step_id,start_time,start_time,0,0,'')
                    else:
                        pass
                        #print self.do(step_id,function_code,function_key_str,execid1,execid2,background)

                print ('None',-1,start_time,start_time,0,0,'')
            except BaseException,e:
                print "error:2179",str(e.message)
                print ('NO',step_id,start_time,start_time,0,0,str(e.message))


def int_decode(value=0):
    if value==None:
        return 0
    else:
        return value

def telnetdo(HOST='', PORT=23, USER='', PASS='', COMMAND='',FINISH='$'): 
    ret,tmp='',''
    try:
        msg = ['Debug messages:\n'] 
        try:
            tn = telnetlib.Telnet(HOST,int(PORT),timeout=8) 
            tn.set_debuglevel(2)
        except:
            return ('NO',"Cannot open host")
        tmp=tmp+tn.read_until("login:",timeout=5)
        tn.write(USER + '\n')
        tmp=tmp+tn.read_until("Password:",timeout=5)
        tn.write(PASS + '\n')

        while True:
            s=tn.read_some()
            tmp=tmp+s
            if len(s)==0:
                break
            ret =ret+s
            ret=ret.replace('\r','')
            ret=ret.replace('\n','')
            if ret.find(FINISH)>-1:
                break
            if ret.find("Terminal type?")>-1:        #介于我这边都需要输入vt00所以为了良好的移植性我这边加了判断，若是不需要输入直接跳过输入vt100
                tn.write("vt100\n")
                tmp=tmp+tn.read_until(FINISH,timeout=5)
                break

        tn.write(COMMAND + '\n')
        tmp=tmp+tn.read_until(FINISH)

        tn.write("exit\n")
        tn.close()
        del tn
        return ('OK',tmp)
    except BaseException,e:
        return ('NO',tmp+e.message)
def sshdo(HOST='', PORT=22, USER='', PASS='', COMMAND='',FINISH='$'): 
    PORT=int(PORT)
    ret,tmp='',''
    try:
        msg = ['Debug messages:\n'] 
        try:
            tn = paramiko.SSHClient()
            tn.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            tn.connect(hostname = HOST,port=PORT,username=USER, password=PASS, timeout=5)
            tmp=tmp+"login %s sucess\n" % HOST
        except BaseException,e:
            return ('NO',"Cannot open host: %s" % str(e))

        stdin,stdout,stderr=tn.exec_command(COMMAND)
        err=stderr.read()
        out=stdout.read()
        tmp=tmp+"%s\n" % COMMAND
        if err:
            tmp=tmp+err
            raise tmp
        else:
            tmp=tmp+out
        return ('OK',tmp)
    except BaseException,e:
        return ('NO',tmp+e.message)
    finally:
        tn.close()
        del tn	
def telnetCallResult(HOST='', PORT=23, USER='', PASS='', COMMAND='',FINISH='$'): 
    ret,tmp='',''
    try:
        msg = ['Debug messages:\n'] 
        try:
            tn = telnetlib.Telnet(HOST,int(PORT),timeout=8) 
            tn.set_debuglevel(0)
        except BaseException,e:
            return ('NO',"Cannot open host: %s" % str(e))
        tmp=tn.read_until("login:",timeout=5)
        tn.write(USER + '\n')
        tmp=tn.read_until("Password:",timeout=5)
        tn.write(PASS + '\n')

        while True:
            s=tn.read_some()
            tmp=tmp+s
            if len(s)==0:
                break
            ret =ret+s
            ret=ret.replace('\r','')
            ret=ret.replace('\n','')
            if ret.find(FINISH)>-1:
                break
            if ret.find("Terminal type?")>-1:       
                tn.write("vt100\n")
                tmp=tmp+tn.read_until(FINISH,5)
                break
        tn.write(COMMAND + '\n')
        res=tn.read_until(FINISH)
        res=res.replace("\r\n","\n")
        res_list=res.split("\n")
        res=res_list[-2]
        tn.write("exit\n")
        tn.close()
        del tn
        return res
    except BaseException,e:
        print e.message
        return ""
def sshdoRes(HOST='', PORT=22, USER='', PASS='', COMMAND='',FINISH='$'): 
    ret,tmp='',''
    try:
        msg = ['Debug messages:\n'] 
        try:
            tn = paramiko.SSHClient()
            tn.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            tn.connect(hostname = HOST,port=PORT,username=USER, password=PASS, timeout=5)
            #tmp=tmp+"login %s sucess\n" % HOST
        except BaseException,e:
            return ('NO',"Cannot open host: %s" % str(e))

        stdin,stdout,stderr=tn.exec_command(COMMAND)
        #tn.invoke_shell
        err=stderr.read()
        out=stdout.read()
        tn.close()
        del tn	
        if err:
            tmp=tmp+err
            raise tmp
        else:
            tmp=tmp+out
        res=tmp.replace("\r\n","\n")
        res_list=tmp.split("\n")
        res=res_list[-2]	
        return res
    except BaseException,e:
        return ""
def run_inst(project_id=-1):
    con_dist=yaml.load(open('config/tool_db_connection.conf').read())
    user=con_dist['user']
    pwd=con_dist['pwd']
    dsn=con_dist['dsn']
    #int(con_dist['run_project_id'])
    run_project_id=project_id
    # 创建数据库连接到文件"mydb"
    #print con_dist
    try:
        tool_con=cx_Oracle.connect(user+'/'+pwd+'@'+dsn)
        log_w.write('debug','Connect to oracle ok, %s' % user+'/'+pwd+'@'+dsn)
        print 'Connect to oracle ok.'
    except cx_Oracle.DatabaseError,e:
        log_w.write('error','Connect to oracle DB failed,%s' % user+'/'+pwd+'@'+dsn+str(e.message))
        print 'Connect to oracle DB failed,'+str(e.message)
        return
    # 获取游标对象
    tool_cur=tool_con.cursor()
    #判断割接项目状态是否正常？
    sql="select count(1) from mgf_project where project_id=%s and state='A'" % str(run_project_id)
    try:
        tool_cur.execute(sql)
        state_cnt,=tool_cur.fetchone()
        if state_cnt==0:
            print "Project ID:%s is not available." % str(run_project_id)
            log_w.write('error',"Project ID:%s is not available." % str(run_project_id))
            return
    except cx_Oracle.DatabaseError,e:
        log_w.write('error',"Get project info error,%s" % str(e.message))
        print str(e.message)
        return

    sql_def_m=sql_def()
    run_inst_a=run_mig_step(tool_con,run_project_id)

    while True:
        flag='None'
        mig_catg_id=-1
        try:	    
            run_inst_a.run_next_step()
            #_a=str(threading.activeCount())
            #log_w.write('debug',"heartbeat,active count:%s" % _a)
            #print 'xian cheng:',threading.activeCount()
        except BaseException,e:
            log_w.write('error',"error:2430,%s" % str(e.message))
            print '2430',str(e.message)
        finally:
            time.sleep(0.1)

def last_day(year, month):
    days = [datetime.datetime(int(year), int(month), 1),]
    oneday = timedelta(days=1)
    month_list=[]
    for x in xrange(32):
        d = days[0]+x*oneday
        if d.month == days[0].month:
            month_list.append(d)
    return month_list[-1]	
def file_filter(filter_from=None,key_index=None,split_str=",",map_dist={},filter_to=None,trim=False):
    try:
        if filter_from and key_index and split_str and filter_to:
            #map_dist={}
            _z=open(filter_to,"wb")
            #for one in filter_in:
                #map_dist[str(one)]=1
            #filter_in=None
            _x=open(filter_from,"rb")
            _w,i=[],0
            _apd=_w.append
            for one in _x:
                _y=one.split(split_str,key_index)
                if trim:
                    _tmp=_y[key_index-1].strip()
                else:
                    _tmp=_y[key_index-1]
                if map_dist.has_key(_tmp):
                    _apd(one)
                    i+=1
                    if i%10000==0:
                        _z.writelines(_w)
                        _w.__init__()
            _x.close()
            _z.writelines(_w)
            _z.close()
            #map_dist=None
            print ("OK","Filter file %s finish" % filter_to,i)
            return ("OK","Filter file %s finish" % filter_to,i)
        else:
            print "Error paramters"
            return ("NO","Error paramters","-1")
    except BaseException,e:
        print ("NO",str(e.message),"-1")
        return ("NO",str(e.message),"-1")


def check_lic():
    lic_check_pass,msg=True,""
    try:
        lic_str=licen.encode_lic('config/license.lic')
        lic=yaml.load(lic_str)
        exp_date_str=lic['Expiration Time']
        exp_date=datetime.datetime.strptime(exp_date_str,"%Y-%m-%d")
        now = datetime.datetime.now()
        d = exp_date-now
        dif=d.days       
        if dif < 0:
            lic_check_pass,msg=False,'license expired,expired date is %s,Please check.' % exp_date_str
        elif dif > 180:
            lic_check_pass,msg=False,'license expired date error,Please check.'
    except BaseException,e:
        lic_check_pass,msg=False,str(e.message)

    return lic_check_pass,msg
def do_job_moni():
    jobserver=job_server()
    jobserver.job_moni()

class job_server(object):
    def __init__(self):
        con_dist=yaml.load(open('config/tool_db_connection.conf').read())
        user=con_dist['user']
        pwd=con_dist['pwd']
        dsn=con_dist['dsn']
        self.tool_con=False
        try:
            self.tool_con=cx_Oracle.connect(user+'/'+pwd+'@'+dsn)
            self.tool_cur=self.tool_con.cursor()    
        except cx_Oracle.DatabaseError,e:
            print 'Connect to Oracle DB failed,',e
    def job_moni(self):
        if not self.tool_con:
            return	
        while True:
            try:
                sql='''select job_id,job_name,month,day,hour,minute,project_id,trigger_type,status,last_trigger_time,next_trigger_time from mgf_jobs where status='A' '''
                re=self.tool_cur.execute(sql).fetchall()
                for one_job in re:
                    (job_id,job_name,month,day,hour,minute,project_id,trigger_type,status,last_trigger_time,next_trigger_time)=one_job
                    now=datetime.datetime.now()
                    if day=='EndDay':
                        last_day_month=month
                        if month=='*':
                            last_day_month=now.month
                        day=last_day(now.year,last_day_month).day

                    tri_falg=False	    
                    (now_mo,now_d,now_h,now_mi,now_s)=(now.month,now.day,now.hour,now.minute,now.second)
                    if (month=='*' or int(month)==int(now_mo)) and (day=='*' or int(day)==int(now_d)) and (hour=='*' or int(hour)==int(now_h))\
                       and (minute=='*' or int(minute)==int(now_mi)):
                        tri_falg=True
                    if last_trigger_time and tri_falg==True:
                        if (last_trigger_time.year,last_trigger_time.month,last_trigger_time.day,last_trigger_time.hour,last_trigger_time.minute)==\
                           (now.year,now.month,now.day,now.hour,now.minute):
                            tri_falg=False
                    if tri_falg==True:
                        if trigger_type=='START':
                            sql1='''select step_id
			    from mgf_mig_flow_step a
			    where exists (select 1
			    from mgf_mig_flow_catg b
			    where a.step_catg_id = b.step_catg_id
			    and b.project_id = %s)
			    and a.state in ('WAIT','BACK','RUN','ERROR') ''' % str(project_id)
                            self.tool_cur.execute(sql1)
                            has_wait=self.tool_cur.fetchall()
                            if len(has_wait)>0:
                                #log_w.write('error',"job start failed,has not finish step.")
                                raise BaseException,'has not finish or error step.'

                            self.tool_cur.execute('''update mgf_mig_flow_step a set a.state='WAIT' where exists (select 1 from mgf_mig_flow_catg b where b.step_catg_name<>'rollback' and a.step_catg_id=b.step_catg_id and b.project_id=%s) and nvl(a.state,'FINISH') != 'SKIP' ''' % str(project_id))
                            log_detail='Project:'+str(project_id)+' Start Finish.'
                            update_sql='''update mgf_jobs set last_trigger_time=:0,log_detail=:1 where job_id=%s''' % job_id
                            self.tool_cur.execute(update_sql,[now,log_detail])
                            self.tool_con.commit()
                            log_w.write('debug',log_detail)
                            print log_detail
            except BaseException,e:
                log_w.write('error',"project %s job failed,%s" % (str(project_id),str(e.message)))
            finally:
                time.sleep(10)

class tool_database(object):
    def __init__(self):
        con_dist=yaml.load(open('config/tool_db_connection.conf').read())
        user=con_dist['user']
        pwd=con_dist['pwd']
        dsn=con_dist['dsn']
        self.con=False
        try:
            self.con=cx_Oracle.connect(user+'/'+pwd+'@'+dsn)
            self.cur=self.con.cursor()    
        except cx_Oracle.DatabaseError,e:
            print 'Connect to Oracle DB failed,',e

    def judge_pdm(self):
        table_list=['MGF_PROJECT','MGF_CONNECTION','MGF_ALL_TABLE','MGF_TABLE_COL','MGF_TABLE_IDX','MGF_TABLE_KEY','MGF_MIG_FLOW_CATG','MGF_MIG_FLOW_STEP','MGF_MIG_FLOW_STEP_HIS','MGF_MIG_DETAIL_LOG','MGF_MIG_DETAIL_LOG_HIS','MGF_PARAMETER','MGF_PARAMETER_VER','MGF_JOBS','MGF_PROJECT_ATTACHMENT','MGF_DB_SOURCES']
        table_list_str="'"+"','".join(table_list)+"'"
        if not self.con:
            return ['NO',['Connect to Oracle DB failed.']]
        re=self.cur.execute('select table_name from user_tables where table_name in (%s)' % table_list_str)
        db_table_set=set([one[0] for one in re])
        table_set=set(table_list)
        dif_table_list=[]
        if table_set<>db_table_set:
            dif_table_list.append('table not exists:')
            dif_table_list=dif_table_list+list(table_set-db_table_set)
            return ['NO',dif_table_list]

        src_pdm_map={}
        src_pdm_map['MGF_PROJECT']= set(['STATE|VARCHAR2(2)', 'PROJECT_NAME|VARCHAR2(20)', 'PAR_VER_CODE|VARCHAR2(100)', 'PROJECT_ID|NUMBER(10)', 'COMMENTS|VARCHAR2(4000)'])
        src_pdm_map['MGF_CONNECTION']= set(['USER_NAME|VARCHAR2(100)', 'CONNECTION_NAME|VARCHAR2(100)', 'PROJECT_ID|NUMBER(10)', 'DSN|VARCHAR2(500)', 'PWD|VARCHAR2(100)', 'CONNECTION_ID|NUMBER(10)', 'COMMENTS|VARCHAR2(255)', 'DB_TYPE|VARCHAR2(100)'])
        src_pdm_map['MGF_ALL_TABLE']= set(['PROJECT_ID|NUMBER(10)', 'TABLE_SPACE|VARCHAR2(20)', 'TABLE_NAME|VARCHAR2(100)', 'TABLE_TYPE|VARCHAR2(10)', 'COMMENTS|VARCHAR2(255)', 'CONNECTION_ID|NUMBER(10)', 'TABLE_ID|NUMBER(10)', 'DATA|CLOB'])
        src_pdm_map['MGF_TABLE_COL']= set(['COL_NAME|VARCHAR2(30)', 'COL_TYPE|VARCHAR2(100)', 'COL_SEQ|NUMBER(4)', 'COMMENTS|VARCHAR2(3000)', 'COLUMN_ID|NUMBER(10)', 'TABLE_ID|NUMBER(10)', 'NULL_ABLE|VARCHAR2(10)'])
        src_pdm_map['MGF_TABLE_IDX']= set(['TABLE_ID|NUMBER(10)', 'IDX_SEQ|NUMBER(4)', 'CREATE_SQL|VARCHAR2(4000)', 'IDX_NAME|VARCHAR2(50)', 'IDX_ID|NUMBER(10)'])
        src_pdm_map['MGF_TABLE_KEY']= set(['CREATE_SQL|VARCHAR2(4000)', 'TABLE_NAME|VARCHAR2(100)', 'KEY_SEQ|NUMBER(4)', 'KEY_NAME|VARCHAR2(30)', 'TABLE_ID|NUMBER(10)', 'KEY_TYPE|VARCHAR2(10)', 'KEY_ID|NUMBER(10)'])
        src_pdm_map['MGF_MIG_FLOW_CATG']= set(['PROJECT_ID|NUMBER(10)', 'COMMENTS|VARCHAR2(100)', 'STEP_CATG_NAME|VARCHAR2(300)', 'STEP_CATG_ID|NUMBER(10)', 'STATE|VARCHAR2(10)', 'EXEC_ORDER_ID|NUMBER(10)'])
        src_pdm_map['MGF_MIG_FLOW_STEP']= set(['FUNC_PAR_LIST|CLOB', 'STEP_ID|NUMBER(10)', 'STEP_CATG_ID|NUMBER(10)', 'FUNCTION_CODE|VARCHAR2(30)', 'STATE|VARCHAR2(10)', 'BACKGROUND|VARCHAR2(2)', 'EXEC_ORDER_ID|NUMBER(10)', 'STEP_NAME|VARCHAR2(300)', 'START_TIME|DATE', 'FLOW|VARCHAR2(4)', 'DEPEND_FLOW|VARCHAR2(40)','COMMENTS|VARCHAR2(4000)'])
        src_pdm_map['MGF_MIG_FLOW_STEP_HIS']= set(['FUNC_PAR_LIST|CLOB', 'STEP_ID|NUMBER(10)', 'STEP_CATG_ID|NUMBER(10)', 'FUNCTION_CODE|VARCHAR2(30)', 'STATE|VARCHAR2(10)', 'UPDATE_DATE|DATE', 'BACKGROUND|VARCHAR2(2)', 'EXEC_ORDER_ID|NUMBER(10)', 'STEP_NAME|VARCHAR2(300)', 'START_TIME|DATE','FLOW|VARCHAR2(4)', 'DEPEND_FLOW|VARCHAR2(40)','COMMENTS|VARCHAR2(4000)'])
        src_pdm_map['MGF_MIG_DETAIL_LOG']= set(['ATTH1_NAME|VARCHAR2(250)','LOG_DETAIL|CLOB', 'LAST_REC_NUM|NUMBER(10)', 'STEP_ID|NUMBER(10)', 'END_TIME|DATE', 'REC_NUM|NUMBER(10)', 'START_TIME|DATE','ATTH1|BLOB'])
        src_pdm_map['MGF_MIG_DETAIL_LOG_HIS']= set(['ATTH1_NAME|VARCHAR2(250)','LOG_DETAIL|CLOB', 'LAST_REC_NUM|NUMBER(10)', 'STEP_ID|NUMBER(10)', 'END_TIME|DATE', 'STATE|VARCHAR2(10)', 'HIS_ID|NUMBER(10)', 'REC_NUM|NUMBER(10)', 'START_TIME|DATE','ATTH1|BLOB'])
        src_pdm_map['MGF_PARAMETER']= set(['COMMENTS|VARCHAR2(400)','PAR_VER_CODE|VARCHAR2(100)','SQL|VARCHAR2(4000)', 'CURRENT_VALUE|VARCHAR2(100)', 'PROJECT_ID|NUMBER(6)', 'CONSTANT|VARCHAR2(400)', 'CON_ID|NUMBER(6)', 'GET_TYPE|VARCHAR2(10)', 'PAR_CODE|VARCHAR2(100)'])
        src_pdm_map['MGF_JOBS']= set(['JOB_NAME|VARCHAR2(200)', 'LAST_TRIGGER_TIME|DATE', 'PROJECT_ID|NUMBER(6)', 'NEXT_TRIGGER_TIME|DATE', 'TRIGGER_TYPE|VARCHAR2(100)', 'MONTH|VARCHAR2(10)', 'JOB_ID|NUMBER(6)', 'HOUR|VARCHAR2(10)', 'STATUS|VARCHAR2(1)', 'MINUTE|VARCHAR2(10)', 'DAY|VARCHAR2(10)', 'LOG_DETAIL|VARCHAR2(4000)'])
        src_pdm_map['MGF_PROJECT_ATTACHMENT']= set(['ATTACH_ID|NUMBER(6)', 'ATTACHMENT|BLOB', 'ATTACH_NAME|VARCHAR2(100)', 'PROJECT_ID|NUMBER(6)', 'COMMENTS|VARCHAR2(4000)'])
        src_pdm_map['MGF_DB_SOURCES']= set(['CREATED_SQL|CLOB', 'SOURCES_NAME|VARCHAR2(100)', 'SOURCES_ID|NUMBER(10)', 'SOURCES_INIT|VARCHAR2(100)', 'COMMENTS|VARCHAR2(500)', 'SOURCES_TYPE|VARCHAR2(100)', 'PROJECT_ID|NUMBER(10)'])
        src_pdm_map['MGF_PARAMETER_VER']= set(['COMMENTS|VARCHAR2(500)', 'PAR_VER_CODE|VARCHAR2(100)', 'PAR_VER_ID|NUMBER(10)', 'PAR_VER_TYPE|VARCHAR2(10)', 'PROJECT_ID|NUMBER(10)'])

        db_pdm_map={}
        sql='''select a.COLUMN_NAME,
        a.DATA_TYPE,
        a.DATA_LENGTH,
        a.DATA_PRECISION,
        a.DATA_SCALE,
        a.NULLABLE,
        b.comments
        from user_tab_columns a,user_col_comments b
        where a.TABLE_NAME = '%s'
        and a.COLUMN_NAME=b.column_name(+)
        and a.TABLE_NAME=b.table_name(+)
        order by a.COLUMN_ID'''

        for one_table in table_list:
            res=self.cur.execute(sql % one_table)
            _x=set()
            for one in res:
                (COLUMN_NAME,DATA_TYPE,DATA_LENGTH,DATA_PRECISION,DATA_SCALE,NULLABLE,COMMENTS)=one
                if DATA_TYPE=='NUMBER':
                    if DATA_SCALE !=0 and DATA_SCALE != None:
                        col_type=DATA_TYPE+'('+str(DATA_PRECISION)+','+str(DATA_SCALE)+')'
                    else:
                        col_type=DATA_TYPE+'('+str(DATA_PRECISION)+')'
                elif DATA_TYPE=='VARCHAR2' or DATA_TYPE=='CHAR':
                    col_type=DATA_TYPE+'('+str(DATA_LENGTH)+')'
                else:
                    col_type=DATA_TYPE
                if NULLABLE=='Y':
                    NULLABLE='YES'
                else:
                    NULLABLE='NO'
                if COMMENTS==None:
                    COMMENTS=''
                _x.add(COLUMN_NAME+'|'+col_type)

            #print "src_pdm_map['%s']=" % one_table,str(_x)
            db_pdm_map[one_table]=_x
        #print db_pdm_map
        return_list=[]
        if src_pdm_map <> db_pdm_map:
            return_list.append("The database table version is inconsistent.")
            for one in src_pdm_map:
                if src_pdm_map[one]<>db_pdm_map[one]:
                    _x=list(src_pdm_map[one])
                    _y=list(db_pdm_map[one])
                    _x.sort()
                    _y.sort()
                    return_list.append("request["+one+"]:"+str(_x))
                    return_list.append("actual ["+one+"]:"+str(_y))
        if len(return_list)>0:
            return ['NO',return_list]
        else:
            return ['OK',return_list]

if __name__ == '__main__':
    lic_flag,msg=check_lic()
    if lic_flag:
        file_conf=open('config/tool_db_connection.conf')
        con_dist=yaml.load(file_conf.read())
        file_conf.close()
        project_list=str(con_dist['run_project_id']).split('|')

        check_pdm=tool_database()
        flag,info_list=check_pdm.judge_pdm()
        if flag=='NO':
            log_w.write('error',"start failed.")
            log_w.write('error',"\n".join(info_list))
        else:
            run_th={}
            if os.name=="nt":
                job_th=threading.Thread(target=do_job_moni)
                job_th.setDaemon(True)
            else:
                job_th=multiprocessing.Process(target=do_job_moni)

            job_th.start()
            time.sleep(2)

            for one_project_id in project_list:
                if os.name=="nt":
                    run_th[one_project_id]=threading.Thread(target=run_inst,kwargs={'project_id':one_project_id})
                    run_th[one_project_id].setDaemon(True)
                else:
                    run_th[one_project_id]=multiprocessing.Process(target=run_inst,kwargs={'project_id':one_project_id})

            for th in run_th:
                run_th[th].start()
                time.sleep(2)
            while 1:
                log_w.write('debug',"heartbeat")
                time.sleep(60)
                flag_0,a=check_lic()
                if not flag_0:
                    log_w.write('error',"license check error, %s" % a)
                    break
    else:
        log_w.write('error',"license check error, %s" % msg)
        print "license check error, %s" % msg