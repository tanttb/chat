#pragma once

#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;

class StatusConPool{
   public:
      StatusConPool(size_t poo_size, std::string host, std::string port);
      ~StatusConPool();
      std::unique_ptr<StatusService::Stub> getConnection();
      void returnConnection(std::unique_ptr<StatusService::Stub> a);
      void Close();

   private:
      std::atomic<bool> _b_stop;
      size_t _poolsize;
      std::string _host;
      std::string _port;
      std::queue<std::unique_ptr<StatusService::Stub>> _connections;
      std::mutex _mutex;
      std::condition_variable _cv;
};

class StatusGrpcClient : public Singleton<StatusGrpcClient>{
   friend class Singleton<StatusGrpcClient>;
   public:
      // ~StatusGrpcClient();
      GetChatServerRsp GetChatServer(int uid);

   private:
      StatusGrpcClient();
      std::unique_ptr<StatusConPool> _pool;
};