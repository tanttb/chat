#pragma once
#include "const.h"

class redisConPool;

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
      ~RedisMgr();
   private:

      RedisMgr();
      
      std::unique_ptr<redisConPool> _con_pool = nullptr;
};

class redisConPool{
   public:
      redisConPool(size_t size, const char * host, int port, const char* pwd);
      ~redisConPool();
      redisContext* getConnection();
      void returnConnection(redisContext*);
      void close();


   private:
      std::atomic<bool> _b_stop;
      size_t _poolsize;
      const char* _host;
      int _port;
      std::queue<redisContext*> _connections;
      std::mutex _mutex;
      std::condition_variable _cv;

};