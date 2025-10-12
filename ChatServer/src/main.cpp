
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOServicePool.h"
#include "ChatServer.h"
#include "ConfigMgr.h"
#include "TcpLogicSystem.h"

using namespace std;
bool _b_stop = false;
std::condition_variable _cv;
std::mutex _mutex;


int main(){
   try{
      boost::asio::io_context ioc;
      boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
      signals.async_wait([&ioc](auto, int signal_number){
         LOG_INFO("Server ShutDown Signal: " << signal_number);
         TcpLogicSystem::GetInstance()->Close();
         ioc.stop();
      });

      string port = ConfigMgr::Instance()["ChatServer1"]["Port"];
      LOG_INFO("Port: " << port);
      ChatServer s(ioc, atoi(port.c_str())); // Pass port as std::string if CServer expects (io_context, std::string)
      std::thread LogicDeal([](){
         TcpLogicSystem::GetInstance()->DealMsg();
      });
      LogicDeal.detach();
      s.StartAccept();
      ioc.run();

   }catch(std::exception &e){
      LOG_INFO("Exception: main " << e.what());
   }
}