#include "CServer.h"
#include "HttpConnection.h"

CServer::CServer(boost::asio::io_context &ioc, unsigned short port): _ioc(ioc)\
, _port(port), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)), _socket(ioc)
{
   
};

void CServer::Start()
{
   auto self = shared_from_this();
   _acceptor.async_accept(_socket, [self](beast::error_code ec){
      try{
         if(ec){
            self->Start();
            return;
         }
         //创建一个连接处理socket
         std::make_shared<HttpConnection>(std::move(self->_socket))->Start();

         //继续监听
         self->Start();
      }catch(std::exception &e){
         std::cerr <<"[error message]: " << e.what() << std::endl;
         return;
      }
   });
}
