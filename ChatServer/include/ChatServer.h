#pragma once
#include "const.h"
#include "CSession.h"

class CSession;

class ChatServer : public std::enable_shared_from_this<ChatServer>{
   
   public:
      ChatServer(boost::asio::io_context &ioc, unsigned short port);
      ~ChatServer();
      void ClearSession(std::string);
      void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code &er);

   private:
      void StartAccept();
      net::io_context &_ioc;
      tcp::acceptor _acceptor;
      std::map<std::string, std::shared_ptr<CSession>> _tcpconnects;
      std::mutex _mutex;
      unsigned short _port;
}; 