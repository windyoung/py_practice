#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cx_Oracle
import Scripts.TableCmpMgr  as CmpMgr
import Scripts.TableDataMgr as DataMgr

connstr1   = "mig1/smart@localhost/orcl"
connstr2   = "mig_tool/smart@localhost/orcl"
table_name = 'advice_type'

def main():
    data_mgr2 = DataMgr.TableDataMgr(connstr2, table_name)
    data_mgr1 = DataMgr.TableDataMgr(connstr1, table_name)
    cmp_mgr   = CmpMgr.TableDataCmpMgr(data_mgr2)
    data_mgr1.get_data_ref(cmp_mgr.compare)
    data_mgr2.close()
    data_mgr1.close()
    cmp_mgr.print_report()

if __name__ == '__main__':
    main()
