# coding: utf-8
__author__ = 'li.rongqing@nj'
__date__ = '$__DATE__$'

import sys, os, re, platform
Import("env")
env = env.Clone()
ALL_SRC='T$(ODBC_FILETAG)DBQuery.cpp T$(ODBC_FILETAG)DBCSQuery.cpp TOraDBQuery.cpp TDBFactory.cpp TQmdbInterfaceAdapter.cpp' 
ALL_SRC=env.Expand(ALL_SRC)

cmd = '$(AR) $(ORACLELIBS)'
cmd = env.Expand(cmd)
env.SetOptionObj2So(cmd)
env.BuildShlib('dbc',ALL_SRC.split(),'')

def pycom_function(target = None, source = None, env = None):
    import stat,os
    src_file   = os.path.abspath(str(source[0]))
    if not os.path.isfile(src_file):
        return '[%s] not a file' % src_file
    dst_file   = str(target[0])
    import   py_compile
    oldp = os.getcwd()
    os.chdir(os.path.dirname(src_file))
    py_compile.compile(os.path.basename(src_file))
    os.chdir(oldp)
    ls = file(src_file+'c','rb').read()
    file(dst_file,'wb').write(ls)

    return 

def disp_pycom(target = None, source = None, env = None):
    #print '111',repr(target[0].rel_path(os.path.abspath('')))
    return 'XX : compile %s => %s' % (source[0], target[0])

env['BUILDERS']['Pycom'] = Builder(action = Action(pycom_function, disp_pycom))

pyc = env.Pycom(env['HOME']+'/bin/GetDBPassword.pyc','../GetDBPassword.py')

env.Depends('.',pyc)
env.Depends('..',pyc)
env.Alias('install', '..')


