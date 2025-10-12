#include "ChatGrpcClient.h"


ChatConPool::ChatConPool(size_t poolsize, std::string host, std::string port):\
_b_stop(false), _poolsize(poolsize), _host(host), _port(port)
{
   //channel表示一个到服务器的连接,可以被多个 Stub 共享
   //Channel 不知道具体的服务接口，只负责传输数据
   //高并发场景：推荐复用一个 Channel 给多个 Stub
   //特定隔离需求：如果每个 Stub 需要完全独立的连接（比如每个 Stub 有独特的配置或安全策略），才需要单独 Channel
   
   std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
   for(int i = 0; i < _poolsize; i++){
      _queue.push(ChatService::NewStub(channel)); 
   }
}

ChatConPool::~ChatConPool()
{
   std::lock_guard<std::mutex> lock(_mutex);
   Close();
   while(!_queue.empty()){
      _queue.pop();
   }
}

void ChatConPool::Close()
{
   _b_stop = true;
   _cv.notify_all();
}

std::unique_ptr<ChatService::Stub> ChatConPool::GetChatServerStub()
{
   std::unique_lock<std::mutex> lock(_mutex);
   _cv.wait(lock, [this](){

      if(_b_stop) return true;
      return !_queue.empty();
   });

   if(_b_stop) return nullptr;

   auto ret = std::move(_queue.front());
   _queue.pop();
   return ret;
}

void ChatConPool::ReturnChatServerStub(std::unique_ptr<ChatService::Stub> ret)
{
   std::lock_guard<std::mutex> lock(_mutex);
   
   if(_b_stop) return;

   _queue.push(std::move(ret));
   _cv.notify_one();
   
}

ChatGrpcClient::ChatGrpcClient()
{
   auto &cfg = ConfigMgr::Instance();
   _pool[cfg["ChatServer1"]["Name"]] = std::make_shared<ChatConPool>(4, cfg["ChatServer1"]["Host"], cfg["ChatServer1"]["Post"]);
   _pool[cfg["ChatServer2"]["Name"]] = std::make_shared<ChatConPool>(4, cfg["ChatServer2"]["Host"], cfg["ChatServer2"]["Post"]);
}
