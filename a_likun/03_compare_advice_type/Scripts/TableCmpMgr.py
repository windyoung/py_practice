#!/usr/bin/env python
# -*- coding: utf-8 -*-

from enum import Enum
import cx_Oracle

class EnumCmpResult(Enum):
    AllEqual = 1
    PkEqual  = 2
    NotExist = 3

class TableDataCmpMgr(object):
    def __init__(self, data_mgr):
        super(TableDataCmpMgr, self).__init__()
        self.data_mgr    = data_mgr
        self.table_name  = data_mgr.table_name
        self.rows        = data_mgr.get_data_all()
        self.cursor      = data_mgr.get_cursor()
        self.description = data_mgr.get_description()
        self.rows_pk     = tuple(map(lambda x: self.get_pk_values(x), self.rows))
        self.pk_cols_cnt = len(list(filter(lambda x: x[1], self.description)))
        self.report_data = {
            'nop':    0,
            'update': 0,
            'insert': 0,
            'error':  0,
        }

    def get_pk_values(self, row):
        columns = map(lambda x: x[0], filter(lambda x: x[1], self.description))
        return tuple(map(lambda x: getattr(row, x), columns))

    def get_sql_update(self):
        columns_normal = map(lambda x: x[0], filter(lambda x: not x[1], self.description))
        columns_pk     = map(lambda x: x[0], filter(lambda x:     x[1], self.description))
        sql_tmpl       = 'update %s set %s where %s'
        return sql_tmpl % (
            self.table_name,
            ', '   .join(map(lambda x: '%s = :%s' % (x, x), columns_normal)),
            ' and '.join(map(lambda x: '%s = :%s' % (x, x), columns_pk)),
        )

    def get_sql_insert(self):
        columns  = list(map(lambda x: x[0], self.description))
        sql_tmpl = 'insert into %s (%s) values (%s)'
        return sql_tmpl % (
            self.table_name,
            ', '.join(map(lambda x: '%s'  % x, columns)),
            ', '.join(map(lambda x: ':%s' % x, columns)),
        )

    def compare(self, row):
        cmp_result = EnumCmpResult.NotExist
        if (row in self.rows):
            cmp_result = EnumCmpResult.AllEqual
        elif (self.pk_cols_cnt > 0 and self.get_pk_values(row) in self.rows_pk):
            cmp_result = EnumCmpResult.PkEqual

        sql = None
        if (cmp_result == EnumCmpResult.PkEqual):
            sql = self.get_sql_update()
        elif (cmp_result == EnumCmpResult.NotExist):
            sql = self.get_sql_insert()

        if (sql):
            try:
                self.cursor.execute(sql, row._asdict())
                self.data_mgr.commit()
                if (cmp_result == EnumCmpResult.PkEqual):
                    self.report_data['update'] = self.report_data['update'] + 1
                elif (cmp_result == EnumCmpResult.NotExist):
                    self.report_data['insert'] = self.report_data['insert'] + 1
            except cx_Oracle.IntegrityError as e:
                errorObj, = e.args
                print('cmp_result = %s, row = %s' % (cmp_result, row))
                print("Error Code:",    errorObj.code)
                print("Error Message:", errorObj.message)
                self.report_data['error'] = self.report_data['error'] + 1
        else:
            self.report_data['nop'] = self.report_data['nop'] + 1

    def print_report(self):
        for key, value in self.report_data.items():
            print('%7s: %d' % (key, value))

if __name__ == '__main__':
    pass
