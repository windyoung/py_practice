//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/dbc/
#ifndef _TDBFACTORY_H
#define _TDBFACTORY_H

#include "TDBInterface.h"
#include "TOraDBQuery.h"
#ifdef _TT
#include "TTTDBCSQuery.h"
#include "TTTDBQuery.h"
#elif defined _ALT
#include "TALTDBQuery.h"
#include "TALTDBCSQuery.h"
#elif defined _SOLID
#include "TSolDBQuery.h"
#elif defined _QM
#include "TQMDBQuery.h"
#include "TQMDBCSQuery.h"
#endif

class TDBFactory
{ 
//本部分分配的内存由外部自已释放
public:
    static TDBInterface* CreateDB(const char *sDbType);
};

#endif

