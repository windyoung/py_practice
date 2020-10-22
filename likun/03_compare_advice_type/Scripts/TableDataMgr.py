#!/usr/bin/env python
# -*- coding: utf-8 -*-

import collections
import cx_Oracle

sql_tmpl_select = 'select t.* from %s t'
sql_tmpl_pk     = '''
    select lower(b.column_name) as column_name
      from user_constraints a
      join user_cons_columns b
        on b.table_name = a.table_name
       and b.constraint_name = a.constraint_name
       and a.constraint_type = 'P'
       and a.table_name = upper('%s')
'''

def OutputTypeHandler(cursor, name, defaultType, size, precision, scale):
    if defaultType == cx_Oracle.CLOB:
        return cursor.var(cx_Oracle.LONG_STRING, arraysize=cursor.arraysize)

def makeNamedTupleFactory(cursor):
    columnNames = [d[0].lower() for d in cursor.description]
    Row = collections.namedtuple('Row', columnNames)
    return Row

class TableDataMgr(object):
    def __init__(self, connstr, table_name):
        super(TableDataMgr, self).__init__()
        self.connstr      = connstr
        self.table_name   = table_name
        self._connection  = cx_Oracle.connect(connstr)
        self._cursor      = self._connection.cursor()
        self._description = None

    def get_data_all(self):
        self._cursor.outputtypehandler = OutputTypeHandler
        self._cursor.execute(sql_tmpl_select % self.table_name)
        self._description = self._cursor.description
        self._cursor.rowfactory = makeNamedTupleFactory(self._cursor)
        return self._cursor.fetchall()

    def get_data_ref(self, handler):
        self._cursor.outputtypehandler = OutputTypeHandler
        self._cursor.execute(sql_tmpl_select % self.table_name)
        self._description = self._cursor.description
        self._cursor.rowfactory = makeNamedTupleFactory(self._cursor)
        for row in self._cursor:
            handler(row)

    def get_cursor(self):
        return self._cursor

    def commit(self):
        self._connection.commit()

    def get_description(self):
        pk_columns = self._get_pk_columns()
        desc = list(map(lambda x: [x[0].lower(), False, x[1]], self._description))
        for d in desc:
            if d[0] in pk_columns:
                d[1] = True
        return desc

    def close(self):
        self._cursor.close()
        self._connection.close()

    def _get_pk_columns(self):
        self._cursor.execute(sql_tmpl_pk % self.table_name)
        return list(map(lambda x: x[0], self._cursor))

if __name__ == '__main__':
    pass
