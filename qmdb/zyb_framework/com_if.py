# coding: utf-8
'''
    公共接口,用于翻译原来的makeall方式，到scons
    原来make方式的实质是定义一系列的编译器变量，根据编译器变量
    组装构建每个make目标的命令

    本部分主要是辅助如何把原来编译器变量翻译到python来
'''
__author__ = 'li.rongqing@nj'
__date__ = '$__DATE__$'

import sys, os, platform, re, string


#编译器变量
class CompileVars:
    def __init__(self):
        #所有的变量名都由大写字母和下划线组成
        return

    def _env_vars(self):
        if not hasattr(self, '_env_params'):
            return
        for pn,pv in self._env_params:
            pv = os.environ.get(pn, pv)
            if hasattr(self, pn):
                #print 'repeat pn =',pn,' pv =',pv
                pass
            setattr(self, pn, pv)
            print 'env:', pn, pv
            continue
        return

    #没有设置才设置
    def SetDefault(self,key,value):
        setattr(self,key,getattr(self,key,value))
        return
    #批量更新变量
    def SetBulk(self, raw_text):
        for line in raw_text.split('\n'):
            line = line.strip()
            if not line or line[0]=='#':
                continue
            if line.count('=') == 0:
                continue
            if line.count('=') != 1:
                #print 'line =',line
                pass
            vn,vv = line.split('=',1)
            #不覆盖上面特别处理的
            self.SetDefault(vn.strip(),vv.strip())
            continue
        return
    def dump(self):
        # 变量 $(大写字母或者下划线)
        var_pattern = '(?P<vname>[A-Z_]*)'
        for aname in self.__dict__.keys():
            if not re.match(var_pattern, aname):
                continue
            daname = aname+' '*32
            print daname[:32], '=', getattr(self,aname)
            continue
        return
    #实现一个展开变量的接口（任意层）
    def expand(self, LINE):
        # 变量 $(大写字母或者下划线),有时有-号
        var_pattern = re.escape('$(')+'(?P<vname>[A-Z_0-9\-]*)'+re.escape(')')
        #展开
        sLine = LINE
        var_match = re.search(var_pattern, sLine)
        while var_match:
            vParts = []
            #变量之前
            vParts.append(sLine[:var_match.start()])
            #变量
            vname = var_match.groupdict()['vname']
            if not hasattr(self,vname):
                #报错
                print 'not has attr [%s]' %vname
                #防止再次报错
                setattr(self,vname,'')
                pass
            vParts.append(getattr(self,vname,''))
            #变量之后
            vParts.append(sLine[var_match.end():])
            #完成了一次最内层的替换
            sLine = ''.join(vParts)
            #更新状态
            var_match = re.search(var_pattern, sLine)
            continue
        #有序排重
        vPart = []
        for part in sLine.split():
            if vPart.count(part) != 0:
                continue
            vPart.append(part)
            continue
        sLine = ' '.join(vPart)
        return sLine
    #
    def select_opt(self, LINE, prefix_list, need):
        RETs = []
        #挑选需要的前缀，（同时剥离）
        if need:
            for item in LINE.split():
                for pre in prefix_list:
                    if item.startswith(pre):
                        RETs.append(item[len(pre):])
                        break
                    continue
                continue
            pass
        else:
            #排除不需要的前缀，（不能剥离）
            for item in LINE.split():
                for pre in prefix_list:
                    if item.startswith(pre):
                        break
                    continue
                else:
                    RETs.append(item)
                    continue
                continue
            pass
        return RETs
    #单元测试
    def test(self):
        sLine = '-D$(AV$(OS)) +AA$(AS)${AS}'
        self.OS = 'HP'
        self.AVHP = 'avhp'
        self.AVIBM = 'avibm'
        self.AVSUN = 'avsun'
        self.AS = '123456'
        self.OS = 'HP'
        assert(self.expand(sLine) == '-Davhp +AA123456${AS}')
        self.OS = 'IBM'
        assert(self.expand(sLine) == '-Davibm +AA123456${AS}')
        sLine = '-DA -DB -LC -lm -ln +AA -AA -m64'
        assert(self.select_opt(sLine,[],True) == [])
        assert(self.select_opt(sLine,['-D'],True) == ['A','B',])
        assert(self.select_opt(sLine,['-D','-l'],True) == ['A','B','m','n'])
        assert(self.select_opt(sLine,['-D','-L','-l'],False) == ['+AA','-AA','-m64'])
    pass

#控制变量,主要用来解析参数
class ControlVars:
    def __init__(self):
        self.project = ''
        self.version = ''
        self.mode = 'debug' #debug, release
        self.bits = '64'    #64,32
        self.plat = ''
        self.build_home = '#../..'
        self.swig_option=''
        self.version_str = 'ZSmart'
        self.cc = 'cc'
        self.cxx = ''
        self.link = ''
        self.shlink=''
        self.include_dirs = ''
        self.defines = ''
        self.cc_flags = ''
        self.lib_dirs = ''
        self.libs = ''
        self.link_flags = ''
        self.shlink_flags = ''
        self.other_flags = ''
        self._params = None # [[变量名 参数名 缺省值],]
        self.set_plat()
        return
    def set_plat(self):
        plat_name = ''
        #平台名称去掉非字母数字
        for ch in platform.system().lower():
            if ch in string.letters or ch in string.digits:
                plat_name += ch
                continue
            continue
        #
        self.plat = plat_name
        return
    def _parse_param(self,ARGUMENTS):
        #ARGUMENTS
        if not self._params:
            return
        for vn,pn,pv in self._params:
            vv = ARGUMENTS.get(pn, pv)
            if hasattr(self, vn):
                print 'repeat vn =',vn,' vv =',vv
                pass
            setattr(self, vn, vv)
            continue
        return


# 使用脚本设置变量
class ShellSetVar:
    def __init__(self, ctrl_obj, vars_obj):
        self.ctrl = ctrl_obj
        self.vars = vars_obj
        return
    def platform(self):
        plat_name = 'do_'+self.ctrl.plat
        func = getattr(self, plat_name, None)
        if func:
            func()
            #self.do_aix()
            #self.do_sunos()
            #self.do_linux()
            #self.do_hpux()
            pass
        return
    def customize(self):
        if not hasattr(self,'_customize'):
            return
        self._customize()
        return
    def default(self,env):
        if not hasattr(self,'_default'):
            return
        self._default(env)
        return
    pass
