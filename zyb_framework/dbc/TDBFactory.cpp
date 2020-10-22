#include "TDBFactory.h"
#include "TMySql.h"
#include "debug_new.h"


TDBInterface* TDBFactory::CreateDB(const char *sDbType)
{ 
    //TDBInterface *pDB = NULL;
    if (strcmp(sDbType,_ORADB_CONN_STRING_) == 0)
    { 
        return new TDBDatabase;
    }
#ifdef _SOLID        
    else if (strcmp(sDbType,_TSOLDB_CONN_STRING_) == 0)
    {
        return new TSolDBDatabase;
      }  
#else
    else if (strcmp(sDbType,_TTDB_CONN_STRING_) == 0)
    {
        return new TOdbcDBDatabase;
    }
    else if (strcmp(sDbType,_TTDBCS_CONN_STRING_) == 0)
    {
        return new TOdbcDBCSDatabase;
    }
    #ifdef _MYSQL
        else if (strcmp(sDbType,_MYSQL_CONN_STRING_) == 0)
        { 
            return new TMySqlDatabase;
        }    
    #endif
#endif        
    else
    {
        throw TException("TDBFactory::CreateDB, Invalid Database Type");
    }
}
