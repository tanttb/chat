#include "TcpLogicSystem.h"
#include "redisMgr.h"
#include "MysqlMgr.h"
#include "StatusGrpcClient.h"
#include "UserMgr.h"

TcpLogicSystem::TcpLogicSystem():_b_stop(false)
{
   initHandler();
}

TcpLogicSystem::~TcpLogicSystem()
{
   Close();
}

void TcpLogicSystem::PushTask(std::shared_ptr<MsgNode> msg)
{  
   std::unique_lock<std::mutex> lock(_mutex);
   if(_b_stop) return;
   _Msg_queue.push(msg);
   // LOG_INFO("q->body: " <<msg->_body);
   // LOG_INFO("q->body: " <<_Msg_queue.front()->_body);

   _cv.notify_one();
}



std::shared_ptr<MsgNode> TcpLogicSystem::GetTask()
{
   std::unique_lock<std::mutex> lock(_mutex);
   _cv.wait(lock, [&]{
      if(_b_stop) return true;
      return !_Msg_queue.empty();
   });

   if(_b_stop) return nullptr;
   auto ret = _Msg_queue.front();
   
   // LOG_INFO("ret->body: " <<ret->_body);
   _Msg_queue.pop();
   return ret;
}


void TcpLogicSystem::DealMsg()
{
   while(!_b_stop){
      auto msg = GetTask();
      if(msg == nullptr) return;

      if(_handlers.find(msg->_id) != _handlers.end()){
         _handlers[msg->_id](msg);
      }else{
         LOG_INFO("msg id [" << msg->_id << "] handler not found")
      }
   }
}

void TcpLogicSystem::initHandler()
{
   _handlers.emplace(MSG_IDS::MSG_CHAT_LOGIN, std::bind(&TcpLogicSystem::HandLogin, this, std::placeholders::_1));
}

void TcpLogicSystem::Close()
{
   std::lock_guard<std::mutex> lock(_mutex);
   _b_stop = true;
   while(!_Msg_queue.empty()){
      _Msg_queue.pop();
   }
}

void TcpLogicSystem::HandLogin(std::shared_ptr<MsgNode> msg)
{
   // LOG_INFO("msg->body: " <<msg->_body);
   std::string data(msg->_body);
   Json::Reader reader;
   Json::Value root;
   reader.parse(data, root);
   auto uid = root["uid"].asInt();
   auto token = root["token"].asString();
   LOG_INFO("User Id: " << uid << " token: " << token);

   Json::Value rtval;

   // auto reply = StatusGrpcClient::GetInstance()->Login(uid, token);
   // if(reply.error() == ErrorCodes::UidValied){
	// 	rtval["error"] = ErrorCodes::UidValied;
	// }
   // else if(reply.error() == ErrorCodes::TokenInvalid) {
	// 	rtval["error"] = ErrorCodes::TokenInvalid;
	// }
   // else if(reply.error() == ErrorCodes::RPCFailed){
   //    rtval["error"] = ErrorCodes::RPCFailed;
   // }
   // else rtval["error"] = ErrorCodes::Success;

   std::string token_key = USERTOKENPREFIX + std::to_string(uid);
   std::string token_val = "";
   bool success = RedisMgr::GetInstance()->Get(token_key, token_val);
   if(!success){
      rtval["error"] = ErrorCodes::RPCFailed;
   }else if(token_val != token){
      rtval["error"] = ErrorCodes::TokenInvalid;
   }else{
      rtval["error"] = ErrorCodes::Success;
   }

   std::string base_key = USER_BASE_INFO + std::to_string(uid);
	auto user_info = std::make_shared<UserInfo>();
	bool b_base = GetBaseInfo(base_key, uid, user_info);
	if (!b_base) {
		rtval["error"] = ErrorCodes::UidValied;
		return;
	}
	rtval["uid"] = uid;
	rtval["pwd"] = user_info->pwd;
	rtval["name"] = user_info->name;
	rtval["email"] = user_info->email;
	// rtval["nick"] = user_info->nick;
	// rtval["desc"] = user_info->desc;
	// rtval["sex"] = user_info->sex;
	// rtval["icon"] = user_info->icon;

   //获取申请列表

   //获取好友列表

   msg->_session->SetUserId(uid);

   std::string ipkey = USERIPPREFIX + std::to_string(uid);
   RedisMgr::GetInstance()->Set(ipkey, ConfigMgr::Instance()["ChatServer1"]["Name"]);

   UserMgr::GetInstance()->SetUserSession(uid, msg->_session);


   std::string jsonstr = rtval.toStyledString();
   LOG_INFO(jsonstr);
   msg->_session->Send(jsonstr, MSG_IDS::MSG_CHAT_LOGIN_RSP);
}

bool TcpLogicSystem::GetBaseInfo(std::string key, int uid, std::shared_ptr<UserInfo> userinfo)
{
   std::string info_str = "";
   bool success = RedisMgr::GetInstance()->Get(key, info_str);
   if (success) {
      Json::Reader reader;
      Json::Value root;
      reader.parse(info_str, root);
      userinfo->uid = root["uid"].asInt();
      userinfo->name = root["name"].asString();
      userinfo->pwd = root["pwd"].asString();
      userinfo->email = root["email"].asString();
      // userinfo->nick = root["nick"].asString();
      // userinfo->desc = root["desc"].asString();
      // userinfo->sex = root["sex"].asInt();
      // userinfo->icon = root["icon"].asString();
      // LOG_INFO("user login uid is  " << userinfo->uid << " name  is "
      //    << userinfo->name << " pwd is " << userinfo->pwd << " email is " << userinfo->email);
	}
	else {
		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}
      
		userinfo = user_info;
		Json::Value redis_root;
		redis_root["uid"] = uid;
		redis_root["pwd"] = userinfo->pwd;
		redis_root["name"] = userinfo->name;
		redis_root["email"] = userinfo->email;
		// redis_root["nick"] = userinfo->nick;
		// redis_root["desc"] = userinfo->desc;
		// redis_root["sex"] = userinfo->sex;
		// redis_root["icon"] = userinfo->icon;
		RedisMgr::GetInstance()->Set(key, redis_root.toStyledString());
	}

	return true;
}