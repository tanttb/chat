#pragma once
#include "const.h"
#include "MsgNode.h"


class ChatServer;
class SendNode;
class MsgNode;

class CSession:public std::enable_shared_from_this<CSession>{

   public:

      CSession(boost::asio::io_context&, ChatServer*);

      tcp::socket& getSocket();
      std::string getSessionUid();

      void SetUserId(int);
      int GetUserId();
      void Start();
      void Send(std::string, short msgid);

   private:
      void AsyncRead(int, std::function<void(boost::system::error_code ec, std::size_t read_len)>);
      void AsyncWrite(std::shared_ptr<SendNode> msg);
      void AsyncReadHead();
      void AsyncReadBody();

      ChatServer* _server;
      tcp::socket _socket;
      bool _b_stop;
      bool _parse_head;
      int _user_id;
      std::string _session_uid;
      std::shared_ptr<MsgNode> _data;
      std::queue<std::shared_ptr<SendNode>> _send_queue;
      std::mutex _recv_mutex;
      std::mutex _send_mutex;
};