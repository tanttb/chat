#include "redisMgr.h"

RedisMgr::RedisMgr()
{
   auto &cfg = ConfigMgr::Instance();
   auto host = cfg["Redis"]["Host"];
   auto port = atoi(cfg["Redis"]["Port"].c_str());
   auto pwd = cfg["Redis"]["Passwd"];
   _con_pool.reset(new redisConPool(5, host.c_str(), port, pwd.c_str()));
}

RedisMgr::~RedisMgr()
{
   Close();
}

redisConPool::redisConPool(size_t size, const char * host, int port, const char* pwd):
_poolsize(size), _host(host), _port(port), _b_stop(false)
{
   for(size_t i = 0; i < size; i++){
      auto *redisCon = redisConnect(host, port);
      if(redisCon == nullptr || redisCon->err != 0){
         redisFree(redisCon);
         continue;
      }

      auto reply = (redisReply*)redisCommand(redisCon, "AUTH %s", pwd);
      if(reply->type == REDIS_REPLY_ERROR){
         LOG_ERROR("AUTH 认证失败");
         freeReplyObject(reply);
         continue;
      }

      freeReplyObject(reply);
      _connections.push(redisCon);
   }
   LOG_INFO("建立redis pool Success");
}

redisConPool::~redisConPool()
{
   std::lock_guard<std::mutex> lock(_mutex);
   while(!_connections.empty()){
      auto *i = _connections.front();
      _connections.pop();
      redisFree(i);
   }
   LOG_INFO("RedisPool Destruct");
}

redisContext* redisConPool::getConnection()
{
   std::unique_lock<std::mutex> lock(_mutex);
   _cv.wait(lock, [this]{
      if(_b_stop){
         return true;
      }
      return !_connections.empty();
   });

   if(_b_stop) return nullptr;

   auto* con = _connections.front();
   _connections.pop();
   return con;
}

void redisConPool::returnConnection(redisContext* ret)
{
   std::unique_lock<std::mutex> lock(_mutex);
   if(_b_stop) return;
   _connections.push(ret);
   _cv.notify_one();
}

void redisConPool::close()
{
   std::lock_guard<std::mutex> lock(_mutex);
   if(_b_stop) return;
   _b_stop = true;
   _cv.notify_all();
}


bool RedisMgr::Connect(const std::string &host, int port)
{
   auto* _connect = _con_pool->getConnection();

   if(_connect != NULL && _connect->err){
      LOG_ERROR("Redis Connect Failed");
      return false;
   }
   LOG_INFO("Redis Connect Success");
   return true;
}

bool RedisMgr::Get(const std::string &key, std::string &val)
{
   auto* _connect = _con_pool->getConnection();
   auto _reply = (redisReply *)redisCommand(_connect, "GET %s",key.c_str());
   if(_reply == NULL){
      LOG_ERROR("GET "<< key << " Failed");
      freeReplyObject(_reply);
      return false;
   }

   if(_reply->type != REDIS_REPLY_STRING){
      LOG_ERROR("GET "<< key << " Type Not String");
      freeReplyObject(_reply);
      return false;
   }

   val = _reply->str;
   freeReplyObject(_reply);
   LOG_INFO("GET "<< key << " Success");
   return true;
}

bool RedisMgr::Set(const std::string &key, const std::string &val)
{
   auto* _connect = _con_pool->getConnection();
   auto _reply = (redisReply *)redisCommand(_connect, "SET %s %s", key.c_str(), val.c_str());

   if(_reply == NULL){
      LOG_ERROR("Execut Command SET " << key << " " << val << "Failed");
      freeReplyObject(_reply);
      return false;
   }

   if(!(_reply->type == REDIS_REPLY_STATUS && (strcmp(_reply->str, "OK") == 0 || strcmp(_reply->str, "ok") == 0))){
      LOG_ERROR("Execut Command SET " << key << " " << val << " Status Failed");
      freeReplyObject(_reply);
      return false;
   }

   LOG_INFO("Execut Command SET " << key << " " << val << " Success");
   freeReplyObject(_reply);
   return true;
}

bool RedisMgr::Auth(const std::string &passwd)
{
   auto* _connect = _con_pool->getConnection();
   auto _reply = (redisReply *)redisCommand(_connect, "AUTH %s", passwd.c_str());

   if (_reply->type == REDIS_REPLY_ERROR) {
      LOG_ERROR("AUTH Failed");
      //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
      freeReplyObject(_reply);
      return false;
   }
   else {
      //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
      freeReplyObject(_reply);
      LOG_INFO("AUTH Success");
      return true;
    }
}

bool RedisMgr::LPush(const std::string &key, const std::string &value)
{
   auto* _connect = _con_pool->getConnection();
   auto _reply = (redisReply*)redisCommand(_connect, "LPUSH %s %s", key.c_str(), value.c_str());
   if (NULL == _reply)
   {
      LOG_ERROR("Execut Command LPush " << key << " " << value << " Failed");
      freeReplyObject(_reply);
      return false;
   }

   if (_reply->type != REDIS_REPLY_INTEGER || _reply->integer <= 0) {
      LOG_ERROR("Execut Command LPush " << key << " " << value << " Integer Failed");
      freeReplyObject(_reply);
      return false;
   }

   LOG_INFO("Execut Command LPush " << key << " " << value << " Success");
   freeReplyObject(_reply);
   return true;
}

bool RedisMgr::LPop(const std::string &key, std::string& value)
{
   auto* _connect = _con_pool->getConnection();
   auto _reply = (redisReply*)redisCommand(_connect, "LPOP %s ", key.c_str());
   if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
      LOG_ERROR("Execut Command LPop " << key << " " << value << " Failed");
      freeReplyObject(_reply);
      return false;
   }
    
   value = _reply->str;
   LOG_INFO("Execut Command LPop " << key << " " << value << " Success");
   freeReplyObject(_reply);
   return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value)
{
   auto* _connect = _con_pool->getConnection();
   auto _reply = (redisReply*)redisCommand(_connect, "RPUSH %s %s", key.c_str(), value.c_str());
   if (NULL == _reply)
   {
      LOG_ERROR("Execut Command RPush " << key << " " << value << " Failed");
      freeReplyObject(_reply);
      return false;
   }

   if (_reply->type != REDIS_REPLY_INTEGER || _reply->integer <= 0) {
      LOG_ERROR("Execut Command RPush " << key << " " << value << " Integer Failed");
      freeReplyObject(_reply);
      return false;
   }

   LOG_INFO("Execut Command RPush " << key << " " << value << " Success");
    freeReplyObject(_reply);
    return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value)
{
   auto* _connect = _con_pool->getConnection();
   auto _reply = (redisReply*)redisCommand(_connect, "RPOP %s ", key.c_str());
   if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
      LOG_ERROR("Execut Command RPop " << key << " " << value << " Failed");
      freeReplyObject(_reply);
      return false;
    }
   value = _reply->str;
   LOG_INFO("Execut Command RPop " << key << " " << value << " Success");
   freeReplyObject(_reply);
   return true;
}

bool RedisMgr::HSet(const std::string &key, const std::string &hkey, const std::string &value) {
   auto* _connect = _con_pool->getConnection();
   auto _reply = (redisReply*)redisCommand(_connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
   if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER ) {
      std::cout << "Execut command [ HSet " << key << "  " << hkey <<"  " << value << " ] failure ! " << std::endl;
      LOG_ERROR("Execut Command HSet " << key << "  " << hkey <<"  " << value << " Failed");
      freeReplyObject(_reply);
      return false;
   }
   LOG_INFO("Execut Command HSet " << key << "  " << hkey << "  " << value << " Success");
   freeReplyObject(_reply);
   return true;
}

bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
   auto* _connect = _con_pool->getConnection();
   const char* argv[4];
   size_t argvlen[4];
   argv[0] = "HSET";
   argvlen[0] = 4;
   argv[1] = key;
   argvlen[1] = strlen(key);
   argv[2] = hkey;
   argvlen[2] = strlen(hkey);
   argv[3] = hvalue;
   argvlen[3] = hvaluelen;
   auto _reply = (redisReply*)redisCommandArgv(_connect, 4, argv, argvlen);
   if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
      LOG_ERROR("Execut Command HSet " << key << "  " << hkey << "  " << hvalue << " Failed");
      freeReplyObject(_reply);
      return false;
   }
   
   LOG_INFO("Execut Command HSet " << key << "  " << hkey << "  " << hvalue << " Success");
   freeReplyObject(_reply);
   return true;
}

bool RedisMgr::Del(const std::string &key)
{
   auto* _connect = _con_pool->getConnection();
   auto _reply = (redisReply*)redisCommand(_connect, "DEL %s", key.c_str());
   if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
      LOG_ERROR("Execut Command Del " << key << " Failed");
      freeReplyObject(_reply);
      return false;
   }
   LOG_INFO("Execut Command Del " << key << " Success");
   freeReplyObject(_reply);
   return true;
}

bool RedisMgr::ExistsKey(const std::string &key)
{
   auto* _connect = _con_pool->getConnection();
   auto _reply = (redisReply*)redisCommand(_connect, "exists %s", key.c_str());
   if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER || _reply->integer == 0) {
      LOG_ERROR("Not Found Key " << key);
      freeReplyObject(_reply);
      return false;
   }
   LOG_INFO("Found Key " << key);
   freeReplyObject(_reply);
   return true;
}

std::string RedisMgr::HGet(const std::string &key, const std::string &hkey)
{
   auto* _connect = _con_pool->getConnection();
   const char* argv[3];
   size_t argvlen[3];
   argv[0] = "HGET";
   argvlen[0] = 4;
   argv[1] = key.c_str();
   argvlen[1] = key.length();
   argv[2] = hkey.c_str();
   argvlen[2] = hkey.length();
   auto _reply = (redisReply*)redisCommandArgv(_connect, 3, argv, argvlen);
   if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
      freeReplyObject(_reply);
      LOG_ERROR("Execut Command HGet " << key << " "<< hkey << " Failed");
      return "";
   }
   std::string value = _reply->str;
   freeReplyObject(_reply);
   LOG_INFO("Execut Command HGet " << key << " " << hkey << " Success");
   return value;
}

void RedisMgr::Close()
{
   _con_pool->close();
}


