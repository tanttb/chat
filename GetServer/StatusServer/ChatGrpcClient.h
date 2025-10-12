#pragma once

#include "const.h"
#include "Singleton.h"
#include "ConfigMgr.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "redisMgr.h"
#include "MysqlMgr.h"
#include "UserMgr.h"
#include <queue>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::ReplyFriendReq;
using message::ReplyFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::ChatService;


class ChatConPool{
   public:

      ChatConPool(size_t poolsize, std::string host, std::string port);
      ~ChatConPool();

      void Close();
      std::unique_ptr<ChatService::Stub> GetChatServerStub();
      void ReturnChatServerStub(std::unique_ptr<ChatService::Stub>);

   private:

      std::atomic<bool> _b_stop;
      std::string _host;
      std::string _port;
      std::condition_variable _cv;
      std::mutex _mutex;
      int _poolsize;
      std::queue<std::unique_ptr<ChatService::Stub>> _queue;
};

class ChatGrpcClient : public Singleton<ChatGrpcClient>{
   friend class Singleton<ChatGrpcClient>;

   public:

      ~ChatGrpcClient();

   private:

      ChatGrpcClient();

   private:
      std::unordered_map<std::string, std::shared_ptr<ChatConPool>> _pool;  

};