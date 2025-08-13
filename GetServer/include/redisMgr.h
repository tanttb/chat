#pragma once
#include "const.h"

class RedisMgr : public Singleton<RedisMgr>{
   friend class Singleton<RedisMgr>;
   public:
      
      bool Connect(const std::string &host, const int port);
      bool Get(const std::string &key, std::string &val);
      bool Set(const std::string &key, const std::string &val);
      bool Auth(const std::string &passwd);
      bool LPush(const std::string &key, const std::string &val);
      bool LPop(const std::string &key, std::string& value);
      bool RPush(const std::string& key, const std::string& value);
      bool RPop(const std::string& key, std::string& value);
      bool HSet(const std::string &key, const std::string &hkey, const std::string &value);
      bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
      bool Del(const std::string &key);
      bool ExistsKey(const std::string &key);
      std::string HGet(const std::string &key, const std::string &hkey);
      void Close();

   private:

      RedisMgr();
      redisContext *_connect;
      redisReply *_reply;
};