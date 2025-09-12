#pragma once

#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RPConPool{
   public:
      RPConPool(std::size_t pool_size, std::string host, std::string port):
      _poolsize(pool_size), _host(host), _port(port), b_stop_(false){

         for(size_t i = 0; i < pool_size; i++){

            std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port, grpc::InsecureChannelCredentials());
            _connections.push(std::move(VarifyService::NewStub(channel)));

         }
         
      }

      ~RPConPool(){
         std::lock_guard<std::mutex> lock(_mutex);
         Close();
         while(!_connections.empty()){
            _connections.pop();
         }
      }

      void Close(){
         b_stop_ = true;
         _cv.notify_all();
      }

      std::unique_ptr<VarifyService::Stub> getConnection(){
         std::unique_lock<std::mutex> lock(_mutex);
         _cv.wait(lock, [this]{
            if(b_stop_) return true;
            return !_connections.empty();
         });

         if(b_stop_){
            return nullptr;
         }

         auto context = std::move(_connections.front());
         _connections.pop();
         return context;
      }

      void returnConnection(std::unique_ptr<VarifyService::Stub> stub){
         std::lock_guard<std::mutex> lock(_mutex);
         if(b_stop_){
            return;
         }

         _connections.push(std::move(stub));
         _cv.notify_one();
      }  

   private:
      std::atomic<bool> b_stop_;
      size_t _poolsize;
      std::string _host;
      std::string _port;
      std::queue<std::unique_ptr<VarifyService::Stub>> _connections;
      std::mutex _mutex;
      std::condition_variable _cv;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>{
   friend class Singleton<VerifyGrpcClient>;
   
   public:
      GetVarifyRsp GetVarifyCode(std::string email){
         ClientContext context;
         GetVarifyRsp reply;
         GetVarifyReq request;
         request.set_email(email);
         auto _stub = _RPCPool->getConnection();
         Status status = _stub->GetVarifyCode(&context, request, &reply);
         _RPCPool->returnConnection(std::move(_stub));
         if(status.ok()){
            return reply;
         }else{
            reply.set_error(ErrorCodes::RPCFailed);
            return reply;
         }
      }
   
   private:
      VerifyGrpcClient(){
         std::string host = ConfigMgr::Instance()["VarifyServer"]["Host"];
         std::string port = ConfigMgr::Instance()["VarifyServer"]["Port"];
         _RPCPool.reset(new RPConPool(3, host, port));
      }

      std::unique_ptr<RPConPool> _RPCPool;
};

