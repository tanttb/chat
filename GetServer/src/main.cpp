#include "CServer.h"
#include "redisMgr.h"

int main(){
   std::string gate_p = ConfigMgr::Instance()["GateServer"]["Port"];
   std::cout << gate_p.c_str() << std::endl;
   unsigned short port = atoi(gate_p.c_str());
   try{
      net::io_context ioc;
      boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
      signals.async_wait([&ioc](const boost::system::error_code &ec, int signal_number){
         if(ec){
            return;
         }
         ioc.stop();
      });

      std::make_shared<CServer>(ioc, port)->Start();
      ioc.run();

   }catch(std::exception &e){
      std::cerr << "[error message]: " << e.what() << std::endl;
      return EXIT_FAILURE;
   }
}

// #define a RedisMgr::GetInstance()

// int main() {
//     assert(a->Set("blogwebsite","llfc.club"));
//     std::string value="";
//     assert(a->Get("blogwebsite", value) );
//     assert(a->Get("nonekey", value) == false);
//     assert(a->HSet("bloginfo","blogwebsite", "llfc.club"));
//     assert(a->HGet("bloginfo","blogwebsite") != "");
//     assert(a->ExistsKey("bloginfo"));
//     assert(a->Del("bloginfo"));
//     assert(a->Del("bloginfo"));
//     assert(a->ExistsKey("bloginfo") == false);
//     assert(a->LPush("lpushkey1", "lpushvalue1"));
//     assert(a->LPush("lpushkey1", "lpushvalue2"));
//     assert(a->LPush("lpushkey1", "lpushvalue3"));
//     assert(a->RPop("lpushkey1", value));
//     assert(a->RPop("lpushkey1", value));
//     assert(a->LPop("lpushkey1", value));
//     assert(a->LPop("lpushkey2", value)==false);
//     a->Close();
// }