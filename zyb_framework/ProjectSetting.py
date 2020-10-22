# coding: utf-8
"""
  项目级的自己定义设置
  Framework 1.2
  参见 #TODO 部分
"""
__author__ = 'li.rongqing@nj'
__date__ = '$__DATE__$'

import sys, os, platform, re, string, time

from com_if import *

#TODO: 定义项目的make变量
class Framework_1_2_Vars(CompileVars):
    def __init__(self):
        CompileVars.__init__(self)
        #TODO: 增加要用到的环境变量，写死，或取系统设置的
        self._env_params = [
            'ORACLE_HOME=/oracle/product/102'.split('='),
            'TIMESTEN_HOME=/tt/TimesTen/tt70'.split('='),
            'TOOLS_PATH=/home/v70/tools'.split('='),
            #'PROJECT_DIR=#../..'.split('='),
            'ZXOS_HOME=#../..'.split('='),
            ['QMDB_HOME',os.environ.get('QuickMDB_HOME','#../QuickMDB')],
            'FRAMEWORK_HOME=#../..'.split('='),
            ]
        return

#TODO: 定义项目的控制变量
class Framework_1_2_Ctrl(ControlVars):
    def __init__(self):
        ControlVars.__init__(self)
        #TODO: 增加要用到的控制变量，变量 参数名 缺省值
        # 变量名(python用)
        # 参数名(scons命令行用)
        # 缺省值(不解释)
        self._params =[
        'mode     compile_mode release  '.split(),
        'bits     object_mode  64     '.split(),
        'db       use_db       oracle '.split(), #oracle
        'odbc     use_odbc     yes    '.split(), #yes,no
        'ver      ver          v1.0   '.split(), #
#        'astf     assertparam  yes    '.split(), #yes,no
#        'opdt     optimezedate yes    '.split(), #yes,no
        'qmdb     use_qmdb     yes    '.split(), #yes,no
        'pyver    python_ver   2.3    '.split(), #2.3 2.7 or....
        'debugflag dubug_new   no     '.split(),
        ]
        return


#TODO: 定义项目的上下文
class CompileOptions(ShellSetVar):
    def __init__(self):
        #TODO: 由前面定义的变量类实例化
        ShellSetVar.__init__(self,Framework_1_2_Ctrl(),Framework_1_2_Vars())
        return
    def do_aix(self):
        #TODO: 设置一些平台相关的make变量
        self.vars.PLATFORM = 'IBM'
        return
    def do_sunos(self):
        self.vars.PLATFORM = 'SUN'
        return
    def do_linux(self):
        self.vars.PLATFORM = 'LINUX'
        self.vars.OBJECT_MODE='64'
        return
    def do_hpux(self):
        self.vars.PLATFORM = 'HP'
        return
    def _customize(self):
        #TODO: 加入项目个性化的参数
        self.make_incl_platform()
        self.make_incl()
        self.default_target_rule()
        self.ctrl.build_home = self.vars.FRAMEWORK_HOME
        return
    def _default(self,env):
        suffix = env.Expand('.$(TRUESUFFIX)')
        if suffix:
            env.Replace(SHLIBSUFFIX=suffix)
            pass
        return
    def make_incl_platform(self):
        '''
        这里翻译 makeall里面产生 变量到 Makefile.incl.platform 的逻辑
        '''
        self.vars.SRCHOME = '#'
        #目前只支持oracle
        if self.ctrl.db == 'oracle':
            self.vars.DATABASE = self.ctrl.db.upper()
        else:
            self.vars.DATABASE = ''
        self.vars.SetDefault('OBJECT_MODE',self.ctrl.bits)
        self.vars.SetDefault('COMPILE_MODE',self.ctrl.mode.upper())
        #这个变量好像没有意义
        if self.ctrl.odbc == 'yes':
            self.vars.ODBC_FLAG = ''
        else:
            self.vars.ODBC_FLAG = '-D_NO_ODBC'

        ver = self.ctrl.ver
        date = time.asctime()
        self.vars.VERID = 'ZSmart %(ver)-32s Build: %(date)s' % locals()

#        if self.ctrl.astf == 'yes':
#            self.vars.ASSERT_FLAG = '-D_ASSERT_PARAM'
#        else:
#            self.vars.ASSERT_FLAG = ''

        #
#        if self.ctrl.opdt == 'yes':
#            self.vars.OPTIMIZE_DATE = '-D_IMP_DATE'
#        else:
#            self.vars.OPTIMIZE_DATE = ''

        #ODBC_TYPE 原文件里面始终为_TT
        if True:
            self.vars.ODBC_FILETAG = 'TT'
            if self.ctrl.qmdb == 'yes':
                self.vars.ODBC_FILETAG = 'QM'
                self.vars.COMPILE_ODBC_FLAG = '_QM'
                #self.vars.ODBC_INCLUDE = '-I$(QMDB_HOME)'
                #self.vars.ODBC_LIB = '-L$(QMDB_HOME)/lib -lmdbHelper -lmdbInterface -lmdbControl -lmdbCommon -lmdbAgent'
                pass
            else:
                self.vars.COMPILE_ODBC_FLAG = '_TT'
                self.vars.ODBC_INCLUDE = '-I$(TIMESTEN_HOME)/include'
                self.vars.ODBC_LIB = '-L$(TIMESTEN_HOME)/lib -ltten'
                sTTver = os.popen('ttversion').read().split()[2]
                sTTver = ''.join(sTTver.split('.')[:3])
                if int(sTTver) >= 1122:
                    self.vars.TIMESTEN_VER_FLAG = 'TT_1122'
                    pass
                else:
                    self.vars.TIMESTEN_VER_FLAG = 'TT_'+sTTver
                    pass
                pass
            pass
        else:
            self.vars.TIMESTEN_VER_FLAG = 'TT_NOUSE'
            pass
        
        if self.ctrl.debugflag == 'yes':
            self.vars.DEBUG_NEW_FLAG='debug_new.cpp'
            self.vars.DEBUG_NEW='_BORLAND_C_'
        else:
            self.vars.DEBUG_NEW_FLAG = ''
            self.vars.DEBUG_NEW='NO_DEBUG_NEW'

        if self.ctrl.mode == 'release':
            self.vars.DEBUG_NEW_FLAG = ''
            self.vars.DEBUG_NEW='NO_DEBUG_NEW'
        return

    def make_incl(self):
        '''
        这里翻译 Makefile.incl 里面定义变量的逻辑
        原来多行的要特殊处理
        AR 后面不能有 -o
        '''
        raw_text = r'''
#############################
# Application Dirs and Libs #
#############################
HELPERDIR         = $(SRCHOME)/helper
DBDIR             = $(SRCHOME)/dbc
JOSONCPPDIR       = $(SRCHOME)/jsoncpp

APP_INC = -I$(HELPERDIR)  -I$(DBDIR) -I$(QMDB_HOME) -I$(JOSONCPPDIR)


APP_LIB = -L$(HELPERDIR) -lhelper -L$(DBDIR) -ldbc -L$(QMDB_HOME)/lib -lmdbHelper -lmdbInterface -lmdbControl -lmdbCommon -lmdbAgent -L$(JOSONCPPDIR) -ljsoncpp

##############################
# 32bit or 64bit object mode #
##############################
PY_OBJ_MODE32=py32/
PY_OBJ_MODE64=
ORACLE_LIB32=lib32
ORACLE_LIB64=lib

#########################
# Debug or Release mode #
#########################
COMPILE_DEBUG     = -g -D$(COMPILE_ODBC_FLAG)
COMPILE_RELEASE   = -g -O -D$(COMPILE_ODBC_FLAG)
BASEFLAGS_DEBUG   =  -D_DEBUG -D_PERF $(TRACK_FLAG) $(POST_PAID_FLAG)
BASEFLAGS_RELEASE =  $(TRACK_FLAG)

####################
# PLATFORM SETTING #
####################

IBM_COMPILER = GNUC
SUN_COMPILER = GNUC
HP_COMPILER  = aCC
LINUX_COMPILER= GNUC
CYGWIN_COMPILER= GNUC

IBM_DEBUG_SPECAIL_OPTION = -qformat=all
IBM_RELEASE_SPECAIL_OPTION =
SUN_DEBUG_SPECAIL_OPTION =
SUN_RELEASE_SPECAIL_OPTION =
HP_DEBUG_SPECAIL_OPTION =
HP_RELEASE_SPECAIL_OPTION =
LINUX_DEBUG_SPECAIL_OPTION =
LINUX_RELEASE_SPECAIL_OPTION =
CYGWIN_DEBUG_SPECAIL_OPTION =
CYGWIN_RELEASE_SPECAIL_OPTION =

#1053314
LINUX_DEBUG_STANDARD_COMPILE_OPTION = -Wall -Wformat=2 -Wconversion -Wsign-conversion -Wshadow
LINUX_RELEASE_STANDARD_COMPILE_OPTION =
IBM_DEBUG_STANDARD_COMPILE_OPTION =-qinfo=dcl:pro:ret:trd -qformat=all
IBM_RELEASE_STANDARD_COMPILE_OPTION =
STANDARD_COMPILE = $($(PLATFORM)_$(COMPILE_MODE)_STANDARD_COMPILE_OPTION)

IBM_CC	  = xlC
SUN_CC	  = CC
HP_CC     = aCC
LINUX_CC  = g++
CYGWIN_CC = g++

SUN_AR   = CC -G -KPIC -xarch=generic64 -misalign -DSUN_CC_HAS_PVFC_BUG  -o
IBM_AR   = xlC -q$(OBJECT_MODE) -G -qmkshrobj -qpic  -o
HP_AR    = aCC -b +DD$(OBJECT_MODE) -o
LINUX_AR = g++ -shared -o
CYGWIN_AR= ar -rv

IBM_LINKFLAGS   =  -q$(OBJECT_MODE) -brtl -qpic
SUN_LINKFLAGS   = -DSYB_LP64 -KPIC -xarch=generic64 -misalign -DSUN_CC_HAS_PVFC_BUG
HP_LINKFLAGS    = +DD$(OBJECT_MODE) -AA -mt -Wl,+as,mpas
LINUX_LINKFLAGS =
CYGWIN_LINKFLAGS=

IBM_FLAGS   = -q$(OBJECT_MODE) -qpic  -qrtti
SUN_FLAGS   = -DSYB_LP64 -KPIC -xarch=generic64 -misalign -DSUN_CC_HAS_PVFC_BUG  -D_REENTRANT -w
HP_FLAGS    = +Z +DD$(OBJECT_MODE) -AA -mt +u1 -DHP_UNIX -D_64_BIT_ -w -DOS_HP -D_BIG_ENDIAN_ -D_REENTRANT
LINUX_FLAGS = -fPIC
CYGWIN_FLAGS=

SUN_LIBS   = -ldl -lrt
IBM_LIBS   = -lpthread  -lm
HP_LIBS	   = -lpthread
LINUX_LIBS = -lutil
CYGWIN_LIBS=

SUN_TRUESUFFIX   = so
IBM_TRUESUFFIX   = so
HP_TRUESUFFIX	 = sl
LINUX_TRUESUFFIX = so
CYGWIN_TRUESUFFIX= dll

####################################
# 3rd Party Software INCs and LIBs #
####################################
SUN_TIMESTENINC = $(ODBC_INCLUDE)
SUN_INFORMIXINC	= -I$(INFORMIXDIR)/incl/esql
SUN_SYBASEINC   = -I$(SYBASEDIR)/include
SUN_ORACLEINC   = -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/rdbms/public
SUN_PYTHONINC   = -I$(TOOLS_PATH)/runtime/include/python$(PYVER)

SUN_INFORMIXLIBS = -L$(INFORMIXDIR)/lib -L$(INFORMIXDIR)/lib/esql -lthsql
SUN_SYBASELIBS   = -L$(SYBASEDIR)/lib -ltcl -lcomn -lintl -lblk -lct -lcs
SUN_ORACLELIBS   = -L$(ORACLE_HOME)/lib -lclntsh
SUN_PYTHONLIBPATH= $(TOOLS_PATH)/runtime/lib/python$(PYVER)/config
SUN_PYTHONLIB    = -L$(SUN_PYTHONLIBPATH) -lpython$(PYVER)
SUN_TIMESTENLIB  = $(ODBC_LIB)

#===============================================
IBM_TIMESTENINC = $(ODBC_INCLUDE)
IBM_INFORMIXINC	= -I$(INFORMIXDIR)/incl/esql
IBM_SYBASEINC	  = -I$(SYBASEDIR)/include
IBM_ORACLEINC   = -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/rdbms/public
IBM_PYTHONINC   = -I$(TOOLS_PATH)/runtime/$(PY_OBJ_MODE$(OBJECT_MODE))include/python$(PYVER)

IBM_INFORMIXLIBS = -L$(INFORMIXDIR)/lib -L$(INFORMIXDIR)/lib/esql -lthsql
IBM_SYBASELIBS   = -L$(SYBASEDIR)/lib -ltcl -lcomn -lintl -lblk -lct -lcs
IBM_ORACLELIBS   = -L$(ORACLE_HOME)/$(ORACLE_LIB$(OBJECT_MODE)) -lclntsh
IBM_PYTHONLIBPATH= $(TOOLS_PATH)/runtime/$(PY_OBJ_MODE$(OBJECT_MODE))lib/python$(PYVER)/config
IBM_PYTHONLIB    = -L$(IBM_PYTHONLIBPATH) -lpython$(PYVER)
IBM_TIMESTENLIB  = $(ODBC_LIB)

#===============================================
HP_TIMESTENINC = $(ODBC_INCLUDE)
HP_INFORMIXINC = -I$(INFORMIXDIR)/incl/esql
HP_SYBASEINC	 = -I$(SYBASEDIR)/include
HP_ORACLEINC   = -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/rdbms/public
HP_PYTHONINC   = -I$(TOOLS_PATH)/runtime/$(PY_OBJ_MODE$(OBJECT_MODE))include/python$(PYVER)

HP_INFORMIXLIBS = -L$(INFORMIXDIR)/lib -L$(INFORMIXDIR)/lib/esql -lthsql
HP_SYBASELIBS   = -L$(SYBASEDIR)/lib -ltcl -lcomn -lintl -lblk -lct -lcs
HP_ORACLELIBS   = -L$(ORACLE_HOME)/$(ORACLE_LIB$(OBJECT_MODE)) -lclntsh
HP_PYTHONLIBPATH= $(TOOLS_PATH)/runtime/$(PY_OBJ_MODE$(OBJECT_MODE))lib/python$(PYVER)/config
HP_PYTHONLIB    = -L$(HP_PYTHONLIBPATH) -lpython$(PYVER)
HP_TIMESTENLIB  = $(ODBC_LIB)

#===============================================
LINUX_TIMESTENINC = $(ODBC_INCLUDE)
LINUX_INFORMIXINC	= -I$(INFORMIXDIR)/incl/esql
LINUX_SYBASEINC	  = -I$(SYBASEDIR)/include
LINUX_ORACLEINC   = -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/rdbms/public
LINUX_PYTHONINC   = -I$(TOOLS_PATH)/runtime/include/python$(PYVER)

LINUX_INFORMIXLIBS = -L$(INFORMIXDIR)/lib -L$(INFORMIXDIR)/lib/esql -lthsql
LINUX_SYBASELIBS   = -L$(SYBASEDIR)/lib -ltcl -lcomn -lintl -lblk -lct -lcs
LINUX_ORACLELIBS   = -L$(ORACLE_HOME)/lib -lclntsh
LINUX_PYTHONLIBPATH= $(TOOLS_PATH)/runtime/lib/python$(PYVER)/config
LINUX_PYTHONLIB    = -L$(LINUX_PYTHONLIBPATH) -lpython$(PYVER)
LINUX_TIMESTENLIB  = $(ODBC_LIB)

#===============================================
CYGWIN_TIMESTENINC = $(ODBC_INCLUDE)
CYGWIN_INFORMIXINC = -I$(INFORMIXDIR)/incl/esql
CYGWIN_SYBASEINC	 = -I$(SYBASEDIR)/include
CYGWIN_ORACLEINC   = -I/usr/include/oracle
CYGWIN_PYTHONINC   = -I/usr/include/python$(PYVER)

CYGWIN_INFORMIXLIBS = -L$(INFORMIXDIR)/lib -L$(INFORMIXDIR)/lib/esql -lthsql
CYGWIN_SYBASELIBS   = -L$(SYBASEDIR)/lib -ltcl -lcomn -lintl -lblk -lct -lcs
CYGWIN_ORACLELIBS   = -L/lib/oracle -loci
CYGWIN_PYTHONLIBPATH= /lib/python$(PYVER)/config
CYGWIN_PYTHONLIB    = -L$(CYGWIN_PYTHONLIBPATH) -lpython$(PYVER)
CYGWIN_TIMESTENLIB  = $(ODBC_LIB)

#===============================================

#########################
# Choose Platform param #
#########################
CCC	     = $($(PLATFORM)_CC)
LLDFLAGS = $($(PLATFORM)_LINKFLAGS)
AR       = $($(PLATFORM)_AR)
COMPILER = $($(PLATFORM)_COMPILER)

TIMESTENINC   = $($(PLATFORM)_TIMESTENINC)
INFORMIXINC   = $($(PLATFORM)_INFORMIXINC)
SYBASEINC     = $($(PLATFORM)_SYBASEINC)
ORACLEINC     = $($(PLATFORM)_ORACLEINC)
PYTHONINC     = $($(PLATFORM)_PYTHONINC)

INFORMIXLIBS  = $($(PLATFORM)_INFORMIXLIBS)
SYBASELIBS    = $($(PLATFORM)_SYBASELIBS)
ORACLELIBS    = $($(PLATFORM)_ORACLELIBS)
PYTHONLIBPATH = $($(PLATFORM)_PYTHONLIBPATH)
PYTHONLIB     = $($(PLATFORM)_PYTHONLIB)
TIMESTENLIB   = $($(PLATFORM)_TIMESTENLIB)
TRUESUFFIX    = $($(PLATFORM)_TRUESUFFIX)

LONGSIZE = LONG$(OBJECT_MODE)
CCFLAGS	 = $(COMPILE_$(COMPILE_MODE)) $(ODBC_FLAG)  -D$(COMPILER) -D$(LONGSIZE) -D$(DEBUG_NEW) $(BASEFLAGS_$(COMPILE_MODE)) -DOS_$(PLATFORM) -D_$(PLATFORM) -D_$(DATABASE) $($(PLATFORM)_FLAGS) -DDB_$(DATABASE) $($(PLATFORM)_$(COMPILE_MODE)_SPECAIL_OPTION) -D$(TIMESTEN_VER_FLAG) $(STANDARD_COMPILE)

LOCALINC = $(PYTHONINC) $($(DATABASE)INC) $($(ODBC_FLAG)TIMESTENINC) $(APP_INC)
LOCALLIB = $(APP_LIB) $($(DATABASE)LIBS) $(PYTHONLIB)  $($(PLATFORM)_LIBS) $($(ODBC_FLAG)TIMESTENLIB)
        '''
        self.vars.PYVER = self.ctrl.pyver

        self.vars.SetBulk(raw_text)
        return

    def default_target_rule(self):
        '''
        这里翻译 Makefile.incl 里面定义默认规则的逻辑
####################
# Complie and Link #
####################
.SUFFIXES: .cpp .o .exe

.cpp.o:
	$(CCC) -c $(CCFLAGS) -c $< -o $*.o $(LOCALINC)

.o.exe: python.exp
	$(CCC)  $(LLDFLAGS) -o $* $< $(LOCALLIB)
	echo $(VERID) >> $*

.cpp.exe:
	$(CCC) -c $(CCFLAGS) -c $< -o $*.o $(LOCALINC)
	$(CCC)  $(LLDFLAGS) -o $* $*.o  $(LOCALLIB)
	echo $(VERID) >> $*

python.exp:
	nm -Bex -X64 $(PYTHONLIBPATH)/$(PYTHONLIBNAME) \
	| sed -e '/ [^BDT] /d' -e '/\./d' -e 's/.* [BDT] //' -e '/::/d'\
	| sort | uniq >> python.exp
        '''
        # self.vars.dump()
        sLine = '$(CCC) -c $(CCFLAGS) -c $(LOCALINC)'
        self.ctrl.cxx = self.vars.expand(sLine)
        sLine = '$(CCC) $(LLDFLAGS) $(LOCALLIB)'
        self.ctrl.link = self.vars.expand(sLine)
        sLine = '$(AR)'
        self.ctrl.shlink = self.vars.expand(sLine)
        self.ctrl.version_str = self.vars.VERID
        #print 'CXX   : ',self.ctrl.cxx
        #print 'LINK  : ',self.ctrl.link
        #print 'SHLINK: ',self.ctrl.shlink
        return
