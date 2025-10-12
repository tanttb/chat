#pragma once
#include <thread>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>
#include "userdata.h"
#include "const.h"

class SqlConnection{

   public:
      SqlConnection(sql::Connection *con, long long lasttime) : _con(std::move(con)), _last_time(lasttime){};

      std::unique_ptr<sql::Connection> _con;
      long long _last_time;
};

class MysqlPool{
   public:
      MysqlPool(std::string url, int poolsize, std::string passwd, int port, std::string schema, std::string user);
      ~MysqlPool();
      void CheckConnection();
      std::unique_ptr<SqlConnection> getConnection();
      void returnConnection(std::unique_ptr<SqlConnection> a);
      void close();

   private:
      
      
      std::string _url;
      std::string _pass;
      std::string _schema;
      std::string _user;
      int _pool_size;
      std::queue<std::unique_ptr<SqlConnection>> _pool;
      std::mutex _mutex;
      std::condition_variable _cv;
      std::atomic<bool> _b_stop;
      std::thread _check;
};


class MysqlDao{
   public:

   MysqlDao();
   ~MysqlDao();

   int RegUser(const std::string& name, const std::string& email, const std::string& passwd);
   bool CheckEmail(const std::string &name, const std::string &email);
   bool UpdatePwd(const std::string &name, const std::string &pwd);
   bool CheckPwd(const std::string &name, const std::string pwd, UserInfo&);
   std::shared_ptr<UserInfo> GetUser(int uid);

   private:
      
      std::unique_ptr<MysqlPool> _mysqlpool;
      
};