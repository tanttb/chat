#include "ChatServer.h"
#include "CSession.h"
#include "AsioIOServicePool.h"
#include "UserMgr.h"

ChatServer::ChatServer(boost::asio::io_context &ioc, unsigned short port): _ioc(ioc), _port(port), _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
   // StartAccept();
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


void ChatServer::ClearSession(std::string sessionid)
{

   std::lock_guard<std::mutex> lock(_mutex);
   if(_tcpconnects.find(sessionid) != _tcpconnects.end()){
      UserMgr::GetInstance()->RemoveUserSession(_tcpconnects[sessionid]->GetUserId());
      _tcpconnects.erase(sessionid);
   }
}

void ChatServer::HandleAccept(std::shared_ptr<CSession> sess, const boost::system::error_code &er)
{  
   LOG_ERROR("Tcp Session connect");
   if(!er){
      sess->Start();
      std::lock_guard<std::mutex> lock(_mutex);
      _tcpconnects.insert({sess->getSessionUid(), sess});
      
   }else{
      LOG_ERROR("Tcp Session Connect Failed: " << er.what());
   }
   
   StartAccept();
}
