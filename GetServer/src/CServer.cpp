#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(boost::asio::io_context &ioc, unsigned short port): _ioc(ioc)\
, _port(port), _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
   
};

void CServer::Start()
{
   auto self = shared_from_this();
   auto &io_context = AsioIOServicePool::GetInstance()->GetIOService();
   std::shared_ptr<HttpConnection> new_conn = std::make_shared<HttpConnection>(io_context);
   _acceptor.async_accept(new_conn->GetSocket(), [self, new_conn](beast::error_code ec){
      try{
         if(ec){
            self->Start();
            return;
         }
         //创建一个连接处理socket
         //继续监听
         new_conn->Start();
         self->Start();
      }catch(std::exception &e){
         std::cerr <<"[error message]: " << e.what() << std::endl;
         self->Start();
      }
   });
}
