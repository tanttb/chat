#include "redisMgr.h"

RedisMgr::RedisMgr()
{
   
}

bool RedisMgr::Connect(const std::string &host, int port)
{
   _connect = redisConnect(host.c_str(), port);

   if(_connect != NULL && _connect->err){
      LOG_ERROR("Redis Connect Failed");
      return false;
   }
   LOG_INFO("Redis Connect Success");
   return true;
}

bool RedisMgr::Get(const std::string &key, std::string &val)
{
   _reply = (redisReply *)redisCommand(_connect, "GET %s",key.c_str());
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
   _reply = (redisReply *)redisCommand(_connect, "SET %s %s", key.c_str(), val.c_str());

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
   _reply = (redisReply *)redisCommand(_connect, "AUTH %s", passwd.c_str());

   if (this->_reply->type == REDIS_REPLY_ERROR) {
      LOG_ERROR("AUTH Failed");
      //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
      freeReplyObject(this->_reply);
      return false;
   }
   else {
      //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
      freeReplyObject(this->_reply);
      LOG_INFO("AUTH Success");
      return true;
    }
}

bool RedisMgr::LPush(const std::string &key, const std::string &value)
{
   this->_reply = (redisReply*)redisCommand(this->_connect, "LPUSH %s %s", key.c_str(), value.c_str());
   if (NULL == this->_reply)
   {
      LOG_ERROR("Execut Command LPush " << key << " " << value << " Failed");
      freeReplyObject(this->_reply);
      return false;
   }

   if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0) {
      LOG_ERROR("Execut Command LPush " << key << " " << value << " Integer Failed");
      freeReplyObject(this->_reply);
      return false;
   }

   LOG_INFO("Execut Command LPush " << key << " " << value << " Success");
   freeReplyObject(this->_reply);
   return true;
}

bool RedisMgr::LPop(const std::string &key, std::string& value)
{
   this->_reply = (redisReply*)redisCommand(this->_connect, "LPOP %s ", key.c_str());
   if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
      LOG_ERROR("Execut Command LPop " << key << " " << value << " Failed");
      freeReplyObject(this->_reply);
      return false;
   }
    
   value = _reply->str;
   LOG_INFO("Execut Command LPop " << key << " " << value << " Success");
   freeReplyObject(this->_reply);
   return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value)
{
   this->_reply = (redisReply*)redisCommand(this->_connect, "RPUSH %s %s", key.c_str(), value.c_str());
   if (NULL == this->_reply)
   {
      LOG_ERROR("Execut Command RPush " << key << " " << value << " Failed");
      freeReplyObject(this->_reply);
      return false;
   }

   if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0) {
      LOG_ERROR("Execut Command RPush " << key << " " << value << " Integer Failed");
      freeReplyObject(this->_reply);
      return false;
   }

   LOG_INFO("Execut Command RPush " << key << " " << value << " Success");
    freeReplyObject(this->_reply);
    return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value)
{
   this->_reply = (redisReply*)redisCommand(this->_connect, "RPOP %s ", key.c_str());
   if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
      LOG_ERROR("Execut Command RPop " << key << " " << value << " Failed");
      freeReplyObject(this->_reply);
      return false;
    }
   value = _reply->str;
   LOG_INFO("Execut Command RPop " << key << " " << value << " Success");
   freeReplyObject(this->_reply);
   return true;
}

bool RedisMgr::HSet(const std::string &key, const std::string &hkey, const std::string &value) {

   this->_reply = (redisReply*)redisCommand(this->_connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
   if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER ) {
      std::cout << "Execut command [ HSet " << key << "  " << hkey <<"  " << value << " ] failure ! " << std::endl;
      LOG_ERROR("Execut Command HSet " << key << "  " << hkey <<"  " << value << " Failed");
      freeReplyObject(this->_reply);
      return false;
   }
   LOG_INFO("Execut Command HSet " << key << "  " << hkey << "  " << value << " Success");
   freeReplyObject(this->_reply);
   return true;
}

bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
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
   this->_reply = (redisReply*)redisCommandArgv(this->_connect, 4, argv, argvlen);
   if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
      LOG_ERROR("Execut Command HSet " << key << "  " << hkey << "  " << hvalue << " Failed");
      freeReplyObject(this->_reply);
      return false;
   }
   
   LOG_INFO("Execut Command HSet " << key << "  " << hkey << "  " << hvalue << " Success");
   freeReplyObject(this->_reply);
   return true;
}

bool RedisMgr::Del(const std::string &key)
{
   this->_reply = (redisReply*)redisCommand(this->_connect, "DEL %s", key.c_str());
   if (this->_reply == nullptr || this->_reply->type != REDIS_REPLY_INTEGER) {
      LOG_ERROR("Execut Command Del " << key << " Failed");
      freeReplyObject(this->_reply);
      return false;
   }
   LOG_INFO("Execut Command Del " << key << " Success");
   freeReplyObject(this->_reply);
   return true;
}

bool RedisMgr::ExistsKey(const std::string &key)
{
   this->_reply = (redisReply*)redisCommand(this->_connect, "exists %s", key.c_str());
   if (this->_reply == nullptr || this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer == 0) {
      LOG_ERROR("Not Found Key " << key);
      freeReplyObject(this->_reply);
      return false;
   }
   LOG_INFO("Found Key " << key);
   freeReplyObject(this->_reply);
   return true;
}

std::string RedisMgr::HGet(const std::string &key, const std::string &hkey)
{
   const char* argv[3];
   size_t argvlen[3];
   argv[0] = "HGET";
   argvlen[0] = 4;
   argv[1] = key.c_str();
   argvlen[1] = key.length();
   argv[2] = hkey.c_str();
   argvlen[2] = hkey.length();
   this->_reply = (redisReply*)redisCommandArgv(this->_connect, 3, argv, argvlen);
   if (this->_reply == nullptr || this->_reply->type == REDIS_REPLY_NIL) {
      freeReplyObject(this->_reply);
      LOG_ERROR("Execut Command HGet " << key << " "<< hkey << " Failed");
      return "";
   }
   std::string value = this->_reply->str;
   freeReplyObject(this->_reply);
   LOG_INFO("Execut Command HGet " << key << " " << hkey << " Success");
   return value;
}

void RedisMgr::Close()
{
   redisFree(_connect);
}


