# coding: utf-8

__author__ = 'li.rongqing@nj'
__date__ = '$__DATE__$'

import sys, os, platform, re, string, shutil

SYSTIP = 'BILLING'
print '%s : begin...' % (SYSTIP, )
#
#帮助信息,根据需要调整
#

Help('''
=======================================================================================================
usage: scons [bits=64|32] [compile_mode=[debug [debug_new=yes] ]| release] [use_db=oracle] [use_qmdb=yes] [use_odbc=yes] [install] [help] 

Options:
  [-c]                 Clean Objects and DLL
  [bits=BITS]          Controls Object bits: 32, 64. Default value is 64.
  [compile_mode=MODE]  Controls Compile mode: debug, release. Default value is debug.
  [use_qmdb=yes]       Use QMDB
  [use_qmdb=no]        Use TT
  [install]            
  [help]               Print defined help message, or this one. 

Example: scons -c  
Example: scons
Example: scons install
Example: scons compile_mode=release install
Example: scons compile_mode=release use_qmdb=yes  install
Example: scons compile_mode=release use_qmdb=no  install
Example: scons compile_mode=debug install
Example: scons compile_mode=debug use_qmdb=yes  install
Example: scons compile_mode=debug use_qmdb=no  install
Example: scons compile_mode=debug dubug_new=yes install

========================================================================================================
''')

# 目录结构
# $VERSION_HOME
#      |
#      + src
#      |   |
#      |   + $(PROJECT)
#      |   |   |
#      |       + SConstruct
#      |       + SConstruct.py
#      |       + com_if.py
#      |       + ProjectSetting.py (自定义)
#      |       |
#      |       + <module_dir>
#      |            |
#      |            + SConscript.py (自定义)
#      |
#      + lib
#      |
#      + bin
#      |
#      + SConstruct (未实现，将来会打包)
#
#print '%s : ', locals()
#vVars = Variables('ccb_custom.py')
#vVars.Update('PROJECT_HOME')
#print 'sys.argv', dir(ARGUMENTS),ARGUMENTS.keys()

#上下文环境
env = Environment(
    #variables = vVars,
    ENV = {'PATH' : os.environ['PATH']}
    )

#项目的配置
import ProjectSetting

#上下文
cont = ProjectSetting.CompileOptions()
cont.ctrl._parse_param(ARGUMENTS)
#OS平台相关
cont.platform()
#引入需要的环境变量
cont.vars._env_vars()
#项目定制
cont.customize()
#保持上下文
env['context'] = cont
env['HOME'] = cont.ctrl.build_home #'#../..'
#print env.Dump()

#####################################################################
#自定义的构建器

#空的构建器,用于强依赖检查
env['BUILDERS']['EmptyBuild'] = Builder(
    action = ' '.join([
        'echo ',
        'to $TARGET',
        'from $SOURCE']),
    cmdstr="^|^")

#复制shell脚本的，主要是为了防止没有设置可执行权限位x，和unix文件格式
def cpsh_function(target = None, source = None, env = None):
    import stat
    src_file   = str(source[0])
    if not os.path.isfile(src_file):
        return '[%s] not a file' % src_file
    dst_file   = str(target[0])
    if os.path.isdir(dst_file):
        dst_file = os.path.join(dst_file,os.path.basename(dst_file))
    #
    lines = [l.rstrip()+os.linesep for l in file(src_file).readlines()]
    file(dst_file, 'w').write(''.join(lines))
    os.chmod(dst_file, stat.S_IRUSR+stat.S_IWUSR+stat.S_IXUSR
        +stat.S_IXGRP
        +stat.S_IXOTH)
    return None

def disp_cpsh(target = None, source = None, env = None):
    #print '111',repr(target[0].rel_path(os.path.abspath('')))
    return '%s : cp %s => %s' % (SYSTIP, source[0], target[0])

env['BUILDERS']['CpSh'] = Builder(action = Action(cpsh_function, disp_cpsh))

# python的头文件包装
#swig -python -c++ -module RatePywrap -DLONG64 -D_LINUX -o RatePywrap.h TRatableEvent.h
#swig_h2py()
env['BUILDERS']['Swig_h2py'] = Builder(
    action = ' '.join([
        '$SWIG',
        '-python -c++',
        '-module ${TARGET.filebase}',
        env['context'].ctrl.swig_option,
        '-outdir ${TARGET.dir}',
        '-o ${TARGET.filebase}.h '
        '$SOURCE']),
    cmdstr=">>$SOURCE => $TARGET<<")

#复制文件，如 cp zxinos/os/zxos.h ocs.net/os/zxos.h
def copy_file(target, source, env):
    src = str(source[0])
    dst = str(target[0])
    dot3 = lambda f:os.sep.join(f.split(os.sep)[-3:])
    #print 'copy %s => %s' % (dot3(src), dot3(dst))

    this_file = sys._getframe().f_code.co_filename
    this_path = this_file.split(os.sep)[:-2]

    src = os.sep.join(this_path+src.split(os.sep)[-3:])
    #print '**'*8,'copy %(src)s => %(dst)s' % locals()
    dst_path = os.path.dirname(dst)
    os.path.exists(dst_path) or os.makedirs(dst_path)
    ls = file(src,'rb').read()
    #print '**'*8,len(ls)
    file(dst,'wb').write(ls)
    return

def show_copy_file(target, source, env):
    src = str(source[0])
    dst = str(target[0])
    dot3 = lambda f:os.sep.join(f.split(os.sep)[-3:])
    print 'copy %s => %s' % (dot3(src), dot3(dst))
    #print 'copy %(src)s => %(dst)s' % locals()
    return

env['BUILDERS']['CopyFile'] = Builder(
    action = Action(copy_file, strfunction=show_copy_file),#cmdstr= ''
    src_suffix = '.h',)


#####################################################################
#辅助函数

#增加版本信息
def append_string(target, source, env):
    fname = str(target[0])
    verstr = env['context'].ctrl.version_str
    print 'Version Info [%(verstr)s] => [%(fname)s]' % locals()
    if not os.path.exists(fname):
        print 'not exists',fname
        return
    #fs = file(fname,'rwba')
    #fs.seek(0,2)
    #fs.write('\n'+verstr)
    #fs.close()
    return

def show_append_string(target, source, env):
    fname = str(target[0])
    version_str = env['context'].ctrl.version_str
    return

def append_version_info(env, target):
    cont = env['context']
    target_name = os.path.join(os.getcwd(), str(target[0]))
    version = cont.ctrl.version_str
    if not version:
        return target
    act = env.Action('echo "%(version)s" >> %(target_name)s' % locals())
    #print 'echo %(version)s >> %(target_name)s' % locals()
    #act = env.Action(append_string, strfunction=show_append_string)
    env.AddPostAction(target, act)
    return target

env.AddMethod(append_version_info, "AppendVersionInfo")

def expand(self,LINE):
    cont = env['context']
    return cont.vars.expand(LINE)

env.AddMethod(expand, "Expand")

def select_opt(self,LINE,opt):
    cont = env['context']
    LINE = cont.vars.expand(LINE)
    return cont.vars.select_opt(LINE,[opt],True)

env.AddMethod(select_opt, "SelectOpt")

def parse_opt(self,LINE):
    cont = env['context']
    LINE = cont.vars.expand(LINE)
    return cont.vars.select_opt(LINE,['-I','-D','-l','-L'],False)

env.AddMethod(parse_opt, "ParseOpt")



#目标说明##########################################################
'''
脚本翻译说明
可执行文件：
编译c文件
$CC -o $TARGET -c $CFLAGS $CCFLAGS $_CCCOMCOM $SOURCES
  'CCFLAGS': [],
  'CFLAGS': [],

编译cpp文件
$CXX -o $TARGET -c $CXXFLAGS $CCFLAGS $_CCCOMCOM $SOURCES
  'CXXFLAGS': [],
连接
$LINK -o $TARGET $LINKFLAGS $__RPATH $SOURCES $_LIBDIRFLAGS $_LIBFLAGS
  'LINKFLAGS': ['$SMARTLINKFLAGS'],

动态库文件：
编译c文件
$SHCC -o $TARGET -c $SHCFLAGS $SHCCFLAGS $_CCCOMCOM $SOURCES
  'SHCCFLAGS': ['$CCFLAGS'],
  'SHCFLAGS': ['$CFLAGS'],
编译cpp文件
$SHCXX -o $TARGET -c $SHCXXFLAGS $SHCCFLAGS $_CCCOMCOM $SOURCES
  'SHCXXFLAGS': ['$CXXFLAGS'],
  'SHCCFLAGS': ['$CCFLAGS'],
连接
$SHLINK -o $TARGET $SHLINKFLAGS $__RPATH $SOURCES $_LIBDIRFLAGS $_LIBFLAGS
  'SHLINKFLAGS': ['$LINKFLAGS', '-qmkshrobj', '-qsuppress=1501-218'],

从上面可以看出来scons编译需要指定
    CCFLAGS  编译c,cpp都用到
    CFLAGS   编译c用到
    CXXFLAGS 编译cpp用到
    LINKFLAGS 连接用到
另外，宏定义用
    CPPDEFINES
inclue目录用
    CPPPATH
指定libpath用
    LIBPATH
指定lib用
    LIBS

翻译的过程就是要组装上面几个变量：
env.Append(CCFLAGS = ...,
    LINKFLAGS=...,
    CPPDEFINES=...,
    CPPPATH=...)

'''
#设置编译c文件的选项#
def opt_src2obj(env, LINE):
    '''
    对于外面传入参数。
    可能含有变量园括号()包围的，环境变量要求是花括号{}包围
    第一个可能是编译命令（如果指定了编译命令）
    里面可能带有-c -o 要去掉
    不能带有 *.o *.c *.cpp等文件名
    '''
    #先展开
    sLine = env.Expand(LINE)
    if not sLine:
        return
    vField = sLine.split()
    #第一个是否为编译命令
    if vField[0][0] not in ('-','+'):
        env.Replace(CC=vField[0])
        sLine = ' '.join(vField[1:])
        pass
    CPPPATH    = env.SelectOpt(sLine, '-I'),
    CPPDEFINES = env.SelectOpt(sLine, '-D')
    CCFLAGS    = env.ParseOpt(sLine)
    #排除 -c -o
    while CCFLAGS.count('-c') != 0:
        CCFLAGS.remove('-c')
    while CCFLAGS.count('-o') != 0:
        CCFLAGS.remove('-o')
    env.Replace(
        CCFLAGS=CCFLAGS,
        CPPPATH=CPPPATH,
        CPPDEFINES=CPPDEFINES)
    return

env.AddMethod(opt_src2obj, "SetOptionC2Obj")

#设置编译cpp文件的选项#
def opt_srcpp2obj(env, LINE):
    '''
    对于外面传入参数。
    可能含有变量园括号()包围的，环境变量要求是花括号{}包围
    第一个可能是编译命令（如果指定了编译命令）
    里面可能带有-c -o 要去掉
    不能带有 *.o *.c *.cpp等文件名
    '''
    #先展开
    sLine = env.Expand(LINE)
    if not sLine:
        return
    vField = sLine.split()
    #第一个是否为编译命令
    if vField[0][0] not in ('-','+'):
        env.Replace(CXX=vField[0])
        sLine = ' '.join(vField[1:])
        pass
    CPPPATH    = env.SelectOpt(sLine, '-I'),
    CPPDEFINES = env.SelectOpt(sLine, '-D')
    CCFLAGS    = env.ParseOpt(sLine)
    #排除 -c -o
    while CCFLAGS.count('-c') != 0:
        CCFLAGS.remove('-c')
    while CCFLAGS.count('-o') != 0:
        CCFLAGS.remove('-o')
    env.Replace(
        CCFLAGS=CCFLAGS,
        CPPPATH=CPPPATH,
        CPPDEFINES=CPPDEFINES)
    return

env.AddMethod(opt_srcpp2obj, "SetOptionCpp2Obj")


#连接exe的选项设置#
def opt_obj2exe(env, LINE):
    sLine = env.Expand(LINE)
    if not sLine:
        return
    vField = sLine.split()
    #第一个是否为编译命令
    if vField[0][0] not in ('-','+'):
        env.Replace(LINK=vField[0])
        sLine = ' '.join(vField[1:])
        pass
    LINKFLAGS = env.ParseOpt(sLine)
    #排除 -o
    while LINKFLAGS.count('-o') != 0:
        LINKFLAGS.remove('-o')
        continue

    LIBS    = env.SelectOpt(sLine, '-l')
    LIBPATH = env.SelectOpt(sLine, '-L')
    LIBPATH.append(env['HOME']+'/lib')
    env.Replace(
        LINKFLAGS=LINKFLAGS,
        LIBPATH=LIBPATH,
        LIBS=LIBS)
    return

env.AddMethod(opt_obj2exe, "SetOptionObj2Exe")

#连接动态库的选项设置#
def opt_obj2so(env, LINE):
    sLine  = env.Expand(LINE)
    if not sLine:
        return
    vField = sLine.split()
    #第一个是否为编译命令
    if vField[0][0] not in ('-','+'):
        env.Replace(SHLINK=vField[0])
        sLine = ' '.join(vField[1:])
        pass
    LINKFLAGS = env.ParseOpt(sLine)
    #排除 -o
    while LINKFLAGS.count('-o') != 0:
        LINKFLAGS.remove('-o')
    LIBS    = env.SelectOpt(sLine, '-l')
    LIBPATH = env.SelectOpt(sLine, '-L')
    LIBPATH.append(env['HOME']+'/lib')
    env.Replace(
        SHLINKFLAGS=LINKFLAGS,
        LIBPATH=LIBPATH,
        LIBS=LIBS)
    return

env.AddMethod(opt_obj2so, "SetOptionObj2So")

#连接静态库的选项设置#
def opt_obj2lib(env, LINE):
    sLine  = env.Expand(LINE)
    if not sLine:
        return
    AR = sLine.split()[0]
    sLine  = ' '.join(sLine.split()[1:])
    LINKFLAGS = env.ParseOpt(sLine)
    LIBS    = env.SelectOpt(sLine, '-l')
    LIBPATH = env.SelectOpt(sLine, '-L')
    LIBPATH.append(env['HOME']+'/lib')
    env.Replace(AR=AR,
        ARFLAGS=LINKFLAGS,
        LIBPATH=LIBPATH,
        LIBS=LIBS)
    return

env.AddMethod(opt_obj2lib, "SetOptionObj2Lib")

#产生exe文件，并且建立安装别名
def build_bin(env,biname,srcs,libinfo):
    env = env.Clone()
    sLine  = env.Expand(libinfo)
    vLibs    = env.SelectOpt(sLine, '-l')
    vLibPath = env.SelectOpt(sLine, '-L')
    if vLibs and vLibPath:
        target = env.Program(biname,srcs,LIBPATH=vLibPath,LIBS=vLibs)
        pass
    else:
        target = env.Program(biname,srcs)
        pass
    env.Install(env['HOME']+'/bin', target)
    env.Alias('install', env['HOME']+'/bin')
    env.AppendVersionInfo(target)
    return target

env.AddMethod(build_bin, "BuildBin")

#产生动态库文件，并且建立安装别名
def build_shlib(env,biname,srcs,libinfo):
    env = env.Clone()
    sLine  = env.Expand(libinfo)
    vLibs    = env.SelectOpt(sLine, '-l')
    vLibPath = env.SelectOpt(sLine, '-L')
    target = env.SharedLibrary(biname,srcs,LIBPATH=vLibPath,LIBS=vLibs)
    env.Install(env['HOME']+'/lib', target)
    env.Alias('install', env['HOME']+'/lib')
    env.AppendVersionInfo(target)
    return target

env.AddMethod(build_shlib, "BuildShlib")

#产生静态库文件，并且建立安装别名
def build_lib(env,biname,srcs,libinfo):
    env = env.Clone()
    sLine  = env.Expand(libinfo)
    vLibs    = env.SelectOpt(sLine, '-l')
    vLibPath = env.SelectOpt(sLine, '-L')
    target = env.StaticLibrary(biname,srcs,LIBPATH=vLibPath,LIBS=vLibs)
    env.Install(env['HOME']+'/lib', target)
    env.Alias('install', env['HOME']+'/lib')
    return target

env.AddMethod(build_lib, "BuildLib")

#####################################################################
#设置顶层env的缺省属性 #
env.SetOptionC2Obj  (' '.join([
    cont.ctrl.cc,
    cont.ctrl.include_dirs,
    cont.ctrl.defines,
    cont.ctrl.cc_flags,
    cont.ctrl.other_flags]))

env.SetOptionCpp2Obj  (' '.join([
    cont.ctrl.cxx,
    cont.ctrl.include_dirs,
    cont.ctrl.defines,
    cont.ctrl.cc_flags,
    cont.ctrl.other_flags]))

env.SetOptionObj2Exe(' '.join([
    cont.ctrl.link,
    cont.ctrl.lib_dirs,
    cont.ctrl.libs,
    cont.ctrl.link_flags,
    cont.ctrl.other_flags]))

env.SetOptionObj2So (' '.join([
    cont.ctrl.shlink,
    cont.ctrl.lib_dirs,
    cont.ctrl.libs,
    cont.ctrl.shlink_flags,
    cont.ctrl.other_flags]))

#项目定制设置顶层env的缺省属性 #
cont.default(env)

#引入工程
SConscript(Glob('*/SConscript.py'), exports='env')
shutil.copyfile(os.getcwd()+'/.FM.ver.info', os.environ['HOME']+'/bin/.FM.ver.info')
print '%s : end.' % (SYSTIP, )
