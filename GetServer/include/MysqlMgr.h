#pragma once
#include "MysqlPool.h"

class MysqlMgr : public Singleton<MysqlMgr>{
   friend class Singleton<MysqlMgr>;
   public:
      ~MysqlMgr(){};
      int RegUser(const std::string& name, const std::string& email, const std::string& passwd);
      bool CheckEmail(const std::string &name, const std::string &email);
      bool UpdatePwd(const std::string &name, const std::string &email);

   private:
      MysqlMgr(){};
      

      MysqlDao _Dao;
};