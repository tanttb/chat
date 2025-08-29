#pragma once
#include "MysqlPool.h"

class MysqlMgr : public Singleton<MysqlMgr>{
   friend class Singleton<MysqlMgr>;
   public:
      ~MysqlMgr(){};
      int RegUser(const std::string& name, const std::string& email, const std::string& passwd);

   private:
      MysqlMgr(){};
      

      MysqlDao _Dao;
};