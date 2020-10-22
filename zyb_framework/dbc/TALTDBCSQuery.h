#ifndef _TALTDBCSQUERY_H_
#define _TALTDBCSQUERY_H_

#include "TALTDBQuery.h"

#define _TTDBCS_CONN_STRING_              "ALTDBCS"

class TOdbcDBCSField : public TOdbcDBField
{

};

class TOdbcDBCSParam : public TOdbcDBParam
{

};

class TOdbcDBCSDatabase: public TOdbcDBDatabase
{
public:
	bool Connect(bool bIsAutoCommit=false) throw (TOdbcDBException); 
};

class TOdbcDBCSQuery: public TOdbcDBQuery
{
public:
	TOdbcDBCSQuery(TOdbcDBCSDatabase *pTOdbcDBDatabase);
	virtual ~TOdbcDBCSQuery();
};

#ifdef HP_UNIX
//  #pragma pack 4
#endif

#endif

