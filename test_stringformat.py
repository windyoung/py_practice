#-*- coding: utf-8 -*-


'''
1、普通的{} 2、带有关键字的{a} {b} 3、带有数字的{0} {1} 4、关键字和数字混用的方式 5、列表来替换字符串中的值 6、对象来替换字符串中的值
'''
s="test,string{},format{},result{}"
print(s.format(' 1 ',' 2 ',' 3 '))



s="test2,string{a},format{b},result{c}"
print(s.format(a='-1 ',b='-2 ',c='-3 '))
print(s.format(a='-1 ',c='-3 ',b='-2 '))
