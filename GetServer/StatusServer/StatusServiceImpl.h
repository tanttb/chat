#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "const.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;
using message::LoginRsp;
using message::LoginReq;

struct ChatServer {
   std::string host;
   std::string port;
   std::string name;
   int con_count;

   bool operator <(const ChatServer &a) const{
      return con_count < a.con_count;
   }
};


class StatusServiceImpl final : public StatusService::Service
{
public:
   StatusServiceImpl();
   Status GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply) override;
   Status Login(ServerContext* context, const LoginReq* request, LoginRsp* reply) override;

private:

   ChatServer getChatServer();
   void insertToken(int uid, std::string token);
   std::priority_queue<ChatServer> _servers;
   std::unordered_map<int, std::string> _tokens;
   std::mutex _ser_mutex;
   std::mutex _token_mutex;
};


std::string generate_unique_string() {
    // 创建UUID对象
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    // 将UUID转换为字符串
    std::string unique_string = to_string(uuid);
    return unique_string;
}
Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
   std::string prefix("llfc status server has received :  ");
   const auto& server = getChatServer();
   reply->set_host(server.host);
   reply->set_port(server.port);
   reply->set_error(ErrorCodes::Success);
   reply->set_token(generate_unique_string());
   insertToken(request->uid(), reply->token());
   return Status::OK;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
   auto uid = request->uid();
   auto token = request->token();

   if(_tokens.find(uid) == _tokens.end()){
      reply->set_error(ErrorCodes::UidValied);
      return Status::OK;
   }

   if(_tokens[uid] != token){
      reply->set_error(ErrorCodes::TokenInvalid);
      return Status::OK;
   }

   reply->set_error(ErrorCodes::Success);
   reply->set_uid(uid);
   reply->set_token(token);
   return Status::OK;
}

ChatServer StatusServiceImpl::getChatServer()
{
   std::lock_guard<std::mutex> lock(_ser_mutex);
   auto minServer = _servers.top();
   _servers.pop();
   minServer.con_count++;
   _servers.push(minServer);
   return minServer;
}

void StatusServiceImpl::insertToken(int uid, std::string token)
{
   std::lock_guard<std::mutex> lock(_token_mutex);
   _tokens[uid] = token;
}
StatusServiceImpl::StatusServiceImpl()
{
   ChatServer server;
   server.port = ConfigMgr::Instance()["ChatServer1"]["Port"];
   server.host = ConfigMgr::Instance()["ChatServer1"]["Host"];
   server.con_count = 0;
   server.name = ConfigMgr::Instance()["ChatServer1"]["Name"];
   _servers.push(server);

   server.port = ConfigMgr::Instance()["ChatServer2"]["Port"];
   server.host = ConfigMgr::Instance()["ChatServer2"]["Host"];
   server.con_count = 0;
   server.name = ConfigMgr::Instance()["ChatServer2"]["Name"];
   _servers.push(server);
}

