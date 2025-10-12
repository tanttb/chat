#include "MysqlPool.h"

void MysqlPool::CheckConnection(){
   std::lock_guard<std::mutex> _lock(_mutex);
   int poolsize = _pool.size();
   auto curtime = std::chrono::system_clock::now().time_since_epoch();
   long long timesp = std::chrono::duration_cast<std::chrono::seconds>(curtime).count();

   for(int i = 0; i < poolsize; i++){
      auto con = std::move(_pool.front());
      _pool.pop();

      if(timesp - con->_last_time < 5){
         continue;
      }
      
      try{
         std::unique_ptr<sql::Statement> stmt = std::unique_ptr<sql::Statement>(con->_con->createStatement());
         stmt->executeQuery("select 1");
         con->_last_time = timesp;
         // LOG_INFO("Execute Timer Alive Query");

      }catch(sql::SQLException &e){
         LOG_ERROR("Error Keeping Connection Alive: " << e.what());
         sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
         auto *newcon = driver->connect(_url, _user, _pass);
         newcon->setSchema(_schema);
         con->_con.reset(newcon);
         con->_last_time = timesp;
      }

      _pool.push(std::move(con));
   }
   
}

MysqlPool::MysqlPool(std::string url, int poolsize, std::string passwd, int port, std::string schema, std::string user)
: _pool_size(poolsize), _user(user), _schema(schema), _pass(passwd), _b_stop(false), _url(url)
{
   try{

      for(int i = 0; i < _pool_size; i++){
         sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
         auto *con = driver->connect(_url, _user, _pass);
         con->setSchema(_schema);
         // std::unique_ptr<sql::Statement> stmt = std::unique_ptr<sql::Statement>(con->createStatement());
         
         auto curtime = std::chrono::system_clock::now().time_since_epoch();
         long long timesp = std::chrono::duration_cast<std::chrono::seconds>(curtime).count();
         _pool.push(std::make_unique<SqlConnection>(con, timesp));
      }

      _check = std::thread([this](){
         while(!_b_stop){
            CheckConnection();
            std::this_thread::sleep_for(std::chrono::seconds(60));
         }
      });
      _check.detach();

   }catch(sql::SQLException &e){
      LOG_ERROR("MYSQL_ERROR" << e.what());
   }
}

std::unique_ptr<SqlConnection> MysqlPool::getConnection(){
   std::unique_lock<std::mutex> lock(_mutex);
   _cv.wait(lock, [this]{
      if(_b_stop) return true;
      return !_pool.empty();
   });

   if(_b_stop) return nullptr;

   std::unique_ptr<SqlConnection> con = std::move(_pool.front());
   _pool.pop();
   return con;
}

void MysqlPool::returnConnection(std::unique_ptr<SqlConnection> a)
{
   if(a == nullptr) return;
   std::unique_lock<std::mutex> lock(_mutex);
   if(_b_stop) return;

   _pool.push(std::move(a));
   _cv.notify_one();
}

void MysqlPool::close()
{
   std::unique_lock<std::mutex> lock(_mutex);
   _b_stop = true;
   lock.unlock();
   _cv.notify_all();
}

MysqlPool::~MysqlPool()
{
   std::unique_lock<std::mutex> lock(_mutex);
   while(!_pool.empty()){
      _pool.pop();
   }
}


MysqlDao::MysqlDao()
{
   auto &cfg = ConfigMgr::Instance();
   std::string host = cfg["Mysql"]["Host"];
   std::string port = cfg["Mysql"]["Port"];
   std::string passwd = cfg["Mysql"]["Passwd"];
   std::string schema = cfg["Mysql"]["Schema"];
   std::string user = cfg["Mysql"]["User"];
   _mysqlpool.reset(new MysqlPool(host + ":" + port, 5, passwd, atoi(port.c_str()), schema, user));
}

MysqlDao::~MysqlDao()
{
   _mysqlpool->close();
}

int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& passwd)
{
   auto con = std::move(_mysqlpool->getConnection());

   try{
      if(con == nullptr) return false;

      std::unique_ptr<sql::PreparedStatement> stmt(con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
      stmt->setString(1, name);
      stmt->setString(2, email);
      stmt->setString(3, passwd);

      stmt->execute();
      
      std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
      std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("select @result AS result"));
      
      if(res->next()){
         int result = res->getInt("result");
         LOG_INFO("Result: " << result);
         _mysqlpool->returnConnection(std::move(con));
         return result;
      }
      
      _mysqlpool->returnConnection(std::move(con));
      return -1;
   }catch(sql::SQLException &e){
      _mysqlpool->returnConnection(std::move(con));
      LOG_ERROR("SQLException: " << e.what());
      return -1;
   }
}

bool MysqlDao::CheckEmail(const std::string &name, const std::string &email){
   auto con = std::move(_mysqlpool->getConnection());
   try{

      if(con == nullptr) return false;
      std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("select email from user where name = ?"));
      pstmt->setString(1, name);
      std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

      while(res->next()){
         LOG_INFO("Check Email: " << res->getString("email"));
         if(email != res->getString("email")){
            _mysqlpool->returnConnection(std::move(con));
            return false;
         }
         _mysqlpool->returnConnection(std::move(con));
         return true;
      }
      return true;

   }catch(sql::SQLException &e){
      _mysqlpool->returnConnection(std::move(con));
      LOG_ERROR("SQLException: " << e.what());
      return false;
   }
}

bool MysqlDao::UpdatePwd(const std::string &name, const std::string &pwd){
   auto con = std::move(_mysqlpool->getConnection());
   try{

      if(con == nullptr) return false;
      std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("update user set passwd= ? where name = ?"));
      pstmt->setString(1, pwd);
      pstmt->setString(2, name);

      std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
      int updateCount = pstmt->executeUpdate();

      LOG_INFO("Update Row: " << updateCount);
      _mysqlpool->returnConnection(std::move(con));
      return true;

   }catch(sql::SQLException &e){
      _mysqlpool->returnConnection(std::move(con));
      LOG_ERROR("SQLException: " << e.what());
      return false;
   }
}

bool MysqlDao::CheckPwd(const std::string &email, const std::string pwd, UserInfo& a){
   auto con = std::move(_mysqlpool->getConnection());
   if(con == nullptr) return false;

   try{

      std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("select * from user where email = ?"));
      pstmt->setString(1, email);
      std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
      std::string p = "";
      if(res->next()){
         LOG_INFO("Check Login Passwd a = " << pwd << " b = " << res->getString("passwd"));
         p = res->getString("passwd");
      }

      if(p != pwd){
         _mysqlpool->returnConnection(std::move(con));
         return false;
      }

      a.pwd = p;
      a.name = res->getString("name");
      a.uid = res->getInt("uid");
      a.email = email;

      _mysqlpool->returnConnection(std::move(con));
      return true;
   }catch(sql::SQLException &e){
      _mysqlpool->returnConnection(std::move(con));
      LOG_ERROR("SQLException: " << e.what());
      return false;
   }
}