#pragma once
#include "Singleton.h"
#include <unordered_map>
#include <memory>
#include <mutex>

class CSession;

class UserMgr : public Singleton<UserMgr>{
   friend class Singleton<UserMgr>;

   public:

      std::shared_ptr<CSession> GetSession(int uid);
      void SetUserSession(int, std::shared_ptr<CSession>);
      void RemoveUserSession(int);

   private:
      UserMgr();
      std::mutex _mutex;
      std::unordered_map<int, std::shared_ptr<CSession>> _uid_to_session;
};