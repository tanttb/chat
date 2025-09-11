#include "MysqlMgr.h"

int MysqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& passwd)
{
   return _Dao.RegUser(name, email, passwd);
}

bool MysqlMgr::CheckEmail(const std::string &name, const std::string &email)
{
   return _Dao.CheckEmail(name, email);
}

bool MysqlMgr::UpdatePwd(const std::string &name, const std::string &pwd)
{
   return _Dao.UpdatePwd(name, pwd);
}
