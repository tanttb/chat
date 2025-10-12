#include "UserMgr.h"
#include "CSession.h"
#include "redisMgr.h"


std::shared_ptr<CSession> UserMgr::GetSession(int uid)
{
   std::lock_guard<std::mutex> lock(_mutex);
   if(_uid_to_session.find(uid) != _uid_to_session.end()){
      return _uid_to_session[uid];
   }
   return nullptr;
}

void UserMgr::SetUserSession(int uid, std::shared_ptr<CSession> a)
{
   std::lock_guard<std::mutex> lock(_mutex);
   _uid_to_session[uid] = a;
}

void UserMgr::RemoveUserSession(int uid)
{
   std::lock_guard<std::mutex> lock(_mutex);
   if(_uid_to_session.find(uid) != _uid_to_session.end()) _uid_to_session.erase(uid);
   
}

UserMgr::UserMgr()
{
   _uid_to_session.clear();
}
