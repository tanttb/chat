
#pragma once
#include "const.h"
#include "ChatServer.h"

class CSession;
class MsgNode{
   public:

      MsgNode(short len){
         _body = new char[len + 1];
         _body[len] = '\0';
      }
      
      ~MsgNode(){
         if(_body != nullptr){
            delete _body;
         }
      }

      void clear(){
         memset(_body, '\0', sizeof(_body));
      }
      
   short _len;
   MSG_IDS _id;
   char *_body;
   std::shared_ptr<CSession> _session;
};

class SendNode{
   public:

      SendNode(std::string data, short id):_id(id){
         _len = data.size();
         _body = new char[_len + 5];
         _body[_len + 5] = '\0';
         short host = boost::asio::detail::socket_ops::host_to_network_short(id);
         memcpy(_body, &host, 2);
         host = boost::asio::detail::socket_ops::host_to_network_short(_len);
         memcpy(_body + 2, &host, 2);
         memcpy(_body + 4, data.c_str(), _len);
      }
      
      ~SendNode(){
         if(_body != nullptr){
            delete _body;
         }
      }

      void clear(){
         memset(_body, '\0', sizeof(_body));
      }
      
   short _len;
   short _id;
   char *_body;
};