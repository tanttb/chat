#include "HttpConnection.h"

//socket没有拷贝构造
HttpConnection::HttpConnection(boost::asio::io_context &ioc): _socket(ioc)
{
   
}

void HttpConnection::Start()
{
   auto self = shared_from_this();
   async_read(_socket, _buffer, _request, [self](beast::error_code ec, std::size_t len){
      try{
         if(ec){
            std::cout << "[error message]: " << ec.what() << std::endl;
            return;
         }

         boost::ignore_unused(len);
         self->HandleReq();
         self->CheckDeadline(); //启动定时器

      }catch(std::exception &e){
         std::cerr << "[error message]: " << e.what() << std::endl;
      }
   });
}

void HttpConnection::CheckDeadline()
{
   auto self = shared_from_this();
   _deadline.async_wait([self](beast::error_code ec){
      if(!ec){
         self->_socket.close(ec);
      }
   });
}

void HttpConnection::WriteResponse()
{
   auto self = shared_from_this();
   _response.content_length(_response.body().size());
   http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t len){
      self->_socket.shutdown(tcp::socket::shutdown_send, ec);
      self->_deadline.cancel();
   });
}

void HttpConnection::HandleReq()
{
   _response.version(_request.version());
   _response.keep_alive(false);

   if(_request.method() == http::verb::get){
      PreParseGetParam();
      bool success = LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());
      if(!success){
         _response.result(http::status::not_found);
         _response.set(http::field::content_type, "text/plain");
         beast::ostream(_response.body()) << "url not found\r\n";
         WriteResponse();
         return;
      }

      _response.result(http::status::ok);
      _response.set(http::field::server, "GateServer");
      WriteResponse();
      return;

   }

   if(_request.method() == http::verb::post){
      bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());
      if(!success){
         _response.result(http::status::not_found);
         _response.set(http::field::content_type, "text/plain");
         beast::ostream(_response.body()) << "url not found\r\n";
         WriteResponse();
         return;
      }

      _response.result(http::status::ok);
      _response.set(http::field::server, "GateServer");
      WriteResponse();
      return;

   }

}

void HttpConnection::PreParseGetParam()
{
   std::string uri = _request.target();
   size_t query_pos = uri.find('?');
   
   if(query_pos == std::string::npos){ 
      _get_url = uri;
      return;
   }

   _get_url = uri.substr(0, query_pos);
   uri = uri.erase(0, query_pos + 1);

   std::string key;
   std::string value;
   size_t pos = 0;
   while((pos = uri.find('&')) != std::string::npos){
      size_t tmp = uri.find('=');
      key = UrlDecode(uri.substr(0, tmp));
      value = UrlDecode(uri.substr(tmp + 1, pos - tmp - 1));
      uri = uri.erase(0, pos + 1);
      pos = 0;
      _get_params.insert({key,value});

   }

   size_t tmp = uri.find('=');
   key = UrlDecode(uri.substr(0, tmp));
   value = UrlDecode(uri.substr(tmp + 1));
   _get_params.insert({key,value});
}
