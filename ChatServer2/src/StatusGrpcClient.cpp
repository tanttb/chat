#include "./../include/StatusGrpcClient.h"

StatusConPool::StatusConPool(size_t pool_size, std::string host, std::string port):
_poolsize(pool_size), _host(host), _port(port), _b_stop(false)
{
   for(size_t i = 0; i < pool_size; i++){
      std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port, grpc::InsecureChannelCredentials());
      _connections.push(StatusService::NewStub(channel));
   }
}

StatusConPool::~StatusConPool()
{
   std::lock_guard<std::mutex> lock(_mutex);
   Close();
   while(!_connections.empty()){
      _connections.pop();
   }
}

std::unique_ptr<StatusService::Stub> StatusConPool::getConnection()
{
   std::unique_lock<std::mutex> lock(_mutex);
   _cv.wait(lock, [this]{
      if(_b_stop) return true;

      return !_connections.empty();
   });

   if(_b_stop) return nullptr;
   auto con = std::move(_connections.front());
   _connections.pop();
   return con;
}

void StatusConPool::returnConnection(std::unique_ptr<StatusService::Stub> a)
{
   if(_b_stop || a == nullptr) return;
   std::lock_guard<std::mutex> lock(_mutex);
   _connections.push(std::move(a));
   _cv.notify_one();
}

void StatusConPool::Close()
{
   _b_stop = true;
   _cv.notify_all();
}

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
{
   ClientContext content;
   GetChatServerReq request;
   GetChatServerRsp response;

   request.set_uid(uid);
   auto stub = std::move(_pool->getConnection());
   Status status = stub->GetChatServer(&content, request, &response);
   _pool->returnConnection(std::move(stub));
   if(status.ok()){
      return response;
   }
   else{
      response.set_error(ErrorCodes::RPCFailed);
      return response;
   }
}

LoginRsp StatusGrpcClient::Login(int uid, std::string token)
{
   ClientContext content;
   LoginReq request;
   LoginRsp response;

   request.set_uid(uid);
   request.set_token(token);

   auto stub = std::move(_pool->getConnection());
   Status stats = stub->Login(&content, request, &response);
   _pool->returnConnection(std::move(stub));
   if(stats.ok()){
      return response;
   }
   else{
      response.set_error(ErrorCodes::RPCFailed);
      return response;
   }
}

StatusGrpcClient::StatusGrpcClient()
{
   std::string host = ConfigMgr::Instance()["StutsServer"]["Host"];
   std::string port = ConfigMgr::Instance()["StutsServer"]["Port"];
   _pool.reset(new StatusConPool(3, host, port));
}
