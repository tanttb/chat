#include "ChatServer.h"
#include "CSession.h"
#include "AsioIOServicePool.h"

ChatServer::ChatServer(boost::asio::io_context &ioc, unsigned short port): _ioc(ioc), _port(port), _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
   StartAccept();
};

ChatServer::~ChatServer()
{
   
}


void ChatServer::StartAccept()
{

   auto &ic = AsioIOServicePool::GetInstance()->GetIOService();
   std::shared_ptr<CSession>  new_session = std::make_shared<CSession>(ic, this);
   _acceptor.async_accept(new_session->getSocket(), std::bind(&ChatServer::HandleAccept, this, new_session, std::placeholders::_1));
}


void ChatServer::ClearSession(std::string id)
{
   std::lock_guard<std::mutex> lock(_mutex);
   if(_tcpconnects.find(id) != _tcpconnects.end()){
      _tcpconnects.erase(id);
   }
}

void ChatServer::HandleAccept(std::shared_ptr<CSession> sess, const boost::system::error_code &er)
{  
   if(!er){
      sess->Start();
      std::lock_guard<std::mutex> lock(_mutex);
      _tcpconnects.insert({sess->getUid(), sess});
      
   }else{
      LOG_ERROR("Tcp Session Connect Failed: " << er.what());
   }
   
   StartAccept();
}
