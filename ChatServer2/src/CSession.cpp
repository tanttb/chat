#include "CSession.h"
#include "TcpLogicSystem.h"


CSession::CSession(boost::asio::io_context& ioc, ChatServer* server):\
_socket(ioc), _b_stop(false), _parse_head(false), _server(server)
{
   // Constructor body can be left empty or add initialization code here if needed
   boost::uuids::uuid id = boost::uuids::random_generator()();
   _session_uid = boost::uuids::to_string(id);
   _data = std::make_shared<MsgNode>(MAX_DATA_LENGTH);

}

tcp::socket& CSession::getSocket()
{
   return _socket;
}

std::string CSession::getSessionUid()
{
   return _session_uid;
}

void CSession::SetUserId(int id)
{
   _user_id = id;
}

int CSession::GetUserId()
{
   return _user_id;
}

void CSession::Start()
{
   AsyncReadHead();
}

void CSession::AsyncRead(int len, std::function<void(boost::system::error_code ec, std::size_t read_len)> handler)
{
   auto self = shared_from_this();
   net::async_read(_socket, net::buffer(_data->_body, len), handler);
}

void CSession::AsyncReadHead()
{
   auto self = shared_from_this();
   _data->clear();
   _data->_session = shared_from_this();
   auto handler = [self](boost::system::error_code ec, std::size_t read_len){
      try{
         if(ec){
            LOG_ERROR("Read Head Error: " << ec.what());
            return;
         }

         short msgid = 0;
         memcpy(&msgid, self->_data->_body, 2);
         msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);
         self->_data->_id = static_cast<MSG_IDS>(msgid);
         LOG_INFO("Read Head Msgid: " << msgid);

         memcpy(&msgid, self->_data->_body + 2, 2);
         msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);
         self->_data->_len = msgid;
         LOG_INFO("Read Head Msglen: " << msgid);

         if(msgid > MAX_DATA_LENGTH){
            LOG_ERROR("Read Head Msglen > MAX_DATA_LEN ");
         }

         self->AsyncReadBody();
      }catch(std::exception &e){
         LOG_ERROR("Exception: e " << e.what());
         return;
      }
   };

   AsyncRead(4, handler);
   
}

void CSession::AsyncReadBody()
{
   auto self = shared_from_this();
   _data->clear();

   auto handler = [self](boost::system::error_code ec, std::size_t read_len){
      if(ec){
         if (ec == boost::asio::error::connection_reset) LOG_ERROR("连接被对端重置，正在重试...")
         else LOG_ERROR("Read Body Error: " << ec.what())
         return;
      }
      
      LOG_INFO("Read Body: " << self->_data->_body);
      TcpLogicSystem::GetInstance()->PushTask(std::make_shared<MsgNode>(*(self->_data)));
      self->AsyncReadHead();
   };

   AsyncRead(_data->_len, handler);
}

void CSession::AsyncWrite(std::shared_ptr<SendNode> msg)
{
   auto self = shared_from_this();
   auto handler = [self](boost::system::error_code ec, std::size_t send_len){
      try{
         if(ec){
            LOG_ERROR("Send Body Error: " << ec.what());
            return;
         }
         std::lock_guard<std::mutex> lock(self->_send_mutex);
         if(!self->_send_queue.empty()){
            auto ptr = self->_send_queue.front();
            self->_send_queue.pop();
            self->AsyncWrite(ptr);
         }

      }catch(std::exception &e){
         LOG_ERROR("Exception: i " << e.what());
         return;
      }
   };
   net::async_write(_socket, net::buffer(msg->_body, msg->_len + 4), handler);
}

void CSession::Send(std::string msg, short msgid)
{
   std::lock_guard<std::mutex> lock(_send_mutex);
   if(_send_queue.size() > MAX_MSG_LENGTH){
      LOG_INFO("Send Body Size Fill: ");
      return;
   }
   
   if(_send_queue.size() == 0) AsyncWrite(std::make_shared<SendNode>(msg, msgid));
   else _send_queue.push(std::make_shared<SendNode>(msg, msgid));
   
}