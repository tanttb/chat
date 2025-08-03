#include "CServer.h"

int main(){
   try{
      unsigned short port= 9987;
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