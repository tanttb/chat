
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOServicePool.h"
#include "ChatServer.h"
#include "ConfigMgr.h"
#include "ChatServiceImpl.h"
#include "TcpLogicSystem.h"

using namespace std;
bool _b_stop = false;
std::condition_variable _cv;
std::mutex _mutex;


int main(){
   try{
      boost::asio::io_context ioc;
      boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);


      std::string rpc_address(ConfigMgr::Instance()["ChatServer2"]["Host"] + ":" + ConfigMgr::Instance()["RPCPort"]["Port"]);
      ChatServiceImpl service;
      grpc::ServerBuilder builder;

      builder.AddListeningPort(rpc_address, grpc::InsecureServerCredentials());
      builder.RegisterService(&service);
      std::shared_ptr<grpc::Server> server(builder.BuildAndStart());
      LOG_INFO("Grpc Listening Port: " << ConfigMgr::Instance()["RPCPort"]["Port"]);
      std::thread grpcserver([server](){
         server->Wait();
      });

      string port = ConfigMgr::Instance()["ChatServer2"]["Port"];
      LOG_INFO("Port: " << port);
      ChatServer s(ioc, atoi(port.c_str())); // Pass port as std::string if CServer expects (io_context, std::string)
      thread TcpLogicDeal([&ioc](){
         TcpLogicSystem::GetInstance()->DealMsg();
      });
      TcpLogicDeal.detach();

      signals.async_wait([&ioc, server](auto, int signal_number){
         LOG_INFO("Server ShutDown Signal: " << signal_number);
         ioc.stop();
         server->Shutdown();
         TcpLogicSystem::GetInstance()->Close();
      });

      ioc.run();

   }catch(std::exception &e){
      LOG_INFO("Exception: main " << e.what());
   }
}