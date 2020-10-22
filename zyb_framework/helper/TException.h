//CHECKED_VERSION=FR3|FILE_PATH=BASEDIR/helper/
#ifndef _T_EXCEPTION_H_
#define _T_EXCEPTION_H_

#include <string>
using namespace std;

class TException
{
public:
    TException();
    TException(const char * fmt, ...);
    TException(const string strErrMsg);
    TException(int iLine, const char *sFileName, const char * fmt, ...);
    virtual ~TException();

    virtual char * GetErrMsg() const;
    virtual char * ToString() const;
private:
    string  m_strErrMsg;
};


#endif //_T_EXCEPTION_H_
