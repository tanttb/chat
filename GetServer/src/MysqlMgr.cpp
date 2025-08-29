#include "MysqlMgr.h"

int MysqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& passwd)
{
   return _Dao.RegUser(name, email, passwd);
}
