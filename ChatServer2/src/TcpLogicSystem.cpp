#include "TcpLogicSystem.h"
#include "redisMgr.h"
#include "StatusGrpcClient.h"

TcpLogicSystem::TcpLogicSystem():_b_stop(false)
{
   
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
   std::string data(msg->_body);
   Json::Reader reader;
   Json::Value root;
   reader.parse(data, root);
   auto uid = root["uid"].asInt();
   auto token = root["token"].asString();
   LOG_INFO("User Id: " << uid << " token: " << token);

   Json::Value rtval;

   auto reply = StatusGrpcClient::GetInstance()->Login(uid, token);
   if(reply.error() == ErrorCodes::UidValied){
		rtval["error"] = ErrorCodes::UidValied;
	}
   else if(reply.error() == ErrorCodes::TokenInvalid) {
		rtval["error"] = ErrorCodes::TokenInvalid;
	}
   else if(reply.error() == ErrorCodes::RPCFailed){
      rtval["error"] = ErrorCodes::RPCFailed;
   }
   else rtval["error"] = ErrorCodes::Success;
   
   std::string jsonstr = rtval.toStyledString();
   msg->_session->Send(jsonstr, MSG_IDS::MSG_CHAT_LOGIN_RSP);
}