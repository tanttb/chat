
#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include "redisMgr.h"
#include "MysqlMgr.h"
#include "StatusGrpcClient.h"

void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
   _get_handlers.insert({url, handler});
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
   _post_handlers.insert({url, handler});
}



LogicSystem::LogicSystem()
{
   RegGet("/get_test",[](std::shared_ptr<HttpConnection> connection){
      beast::ostream(connection->_response.body()) << "receive get_test" << std::endl;
      
      int i = 0;
      for(auto &elem : connection->_get_params){
         i++;
         beast::ostream(connection->_response.body()) << "param " << i << " key: " \
         << elem.first << "  value: " << elem.second << std::endl;
      }
   });

   RegPost("/get_varifycode",[](std::shared_ptr<HttpConnection> connection){
      auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
      std::cout << "receive body is: " << body_str << std::endl;
      connection->_response.set(http::field::content_type, "text/json");

      Json::Value root;
      Json::Reader reader;
      Json::Value src_root;

      bool parse_success = reader.parse(body_str, src_root);
      if(!parse_success){
         std::cout << "Failed to parse Json data" << std::endl;
         root["error"] = ErrorCodes::Error_Json;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }

      if(!src_root.isMember("email")){
         std::cout << "Failed to parse Json data" << std::endl;
         root["error"] = ErrorCodes::Error_Json;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }

      auto email = src_root["email"].asString();
      GetVarifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
                     
      std::cout << "receive email is: " << email << std::endl;

      root["error"] = rsp.error();
      root["email"] = src_root["email"];
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->_response.body()) << jsonstr;
      return true;
   });

   RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection){
      auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
      LOG_INFO("receive body: " << body_str);
      connection->_response.set(http::field::content_type, "text/json");
      Json::Value root;
      Json::Reader reader;
      Json::Value src_root;

      bool parse_json = reader.parse(body_str, src_root);
      if(!parse_json){
         LOG_INFO("Json Parse Failed");
         root["error"] = ErrorCodes::Error_Json;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true; 
      }

      auto email = src_root["email"].asString();
      auto user = src_root["user"].asString();
      auto passwd = src_root["passwd"].asString();
      auto confirm = src_root["confirm"].asString();
      auto varifycode = src_root["varifycode"].asString();

      std::string verify_code;
      std::string key = CODEPREFIX + email;
      
      bool b_get_varify = RedisMgr::GetInstance()->Get(key, verify_code);
      if(!b_get_varify){
         LOG_INFO("Get Verify Code Expired");
         root["error"] = ErrorCodes::VarifyExpired;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }

      if(verify_code != src_root["varifycode"].asString()){
         LOG_INFO("Verify Code Error");
         root["error"] = ErrorCodes::VarifyCodeErr;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }

      bool b_user_exist = RedisMgr::GetInstance()->ExistsKey(src_root["user"].asString());
      if(b_user_exist){
         LOG_INFO("User Have Register");
         root["error"] = ErrorCodes::UserExist;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }

      int uid = MysqlMgr::GetInstance()->RegUser(user, email, passwd);
      if(uid == -2 || uid == -1 || uid == -3){
         LOG_INFO("Register Username or Email Exist");
         root["error"] = ErrorCodes::UserExist;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }

      root["error"] = ErrorCodes::Success;
      root["uid"] = uid;
      root["email"] = src_root["email"].asString();
      root["user"] = src_root["user"].asString();
      root["passwd"] = src_root["passwd"].asString();
      root["confirm"] = src_root["confirm"].asString();
      root["varifycode"] = src_root["varifycode"].asString();
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->_response.body()) << jsonstr;
      return true;
   });

   RegPost("/reset_pwd", [](std::shared_ptr<HttpConnection> connection){
      auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
      LOG_INFO("receive body: " << body_str);
      connection->_response.set(http::field::content_type, "text/json");
      Json::Value root;
      Json::Reader reader;
      Json::Value src_root;

      bool parse_json = reader.parse(body_str, src_root);
      if(!parse_json){
         LOG_INFO("Json Parse Failed");
         root["error"] = ErrorCodes::Error_Json;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true; 
      }

      auto email = src_root["email"].asString();
      auto user = src_root["user"].asString();
      auto passwd = src_root["passwd"].asString();
      auto verify_code = src_root["varifycode"].asString();

      std::string verifycode;
      bool b_get_verify = RedisMgr::GetInstance()->Get(CODEPREFIX + src_root["email"].asString(), verifycode);
      if(!b_get_verify){
         LOG_INFO("Get Verify Code Expired");
         root["error"] = ErrorCodes::VarifyExpired;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }
      if(verify_code != verifycode){
         LOG_INFO("Reset Passwd Verify Code Error");
         root["error"] = ErrorCodes::VarifyCodeErr;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }

      bool email_valid = MysqlMgr::GetInstance()->CheckEmail(user, email);
      if(!email_valid){
         LOG_INFO("Confirm User According With Email");
         root["error"] = ErrorCodes::VerifyUserEmail;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }

      bool b_update_passwd = MysqlMgr::GetInstance()->UpdatePwd(user, passwd);

      LOG_INFO("Success Reset Passwd");
      root["error"] = 0;
      root["email"] = email;
      root["user"] = user;
      root["passwd"] = passwd;
      root["verifycode"] = verify_code;
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->_response.body()) << jsonstr;
      return true;
   });

   RegPost("/user_login", [](std::shared_ptr<HttpConnection> connection){
      auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
      LOG_INFO("receive body: " << body_str);
      connection->_response.set(http::field::content_type, "text/json");
      Json::Value root;
      Json::Reader reader;
      Json::Value src_root;

      bool parse_json = reader.parse(body_str, src_root);
      if(!parse_json){
         LOG_INFO("Json Parse Failed");
         root["error"] = ErrorCodes::Error_Json;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true; 
      }

      auto email = src_root["user"].asString();
      auto passwd = src_root["passwd"].asString();
      UserInfo userin;
      bool check = MysqlMgr::GetInstance()->CheckPwd(email, passwd, userin);
      if(!check){
         LOG_INFO("User Pwd Not Match");
         root["error"] = ErrorCodes::PasswdInvalid;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }

      auto reply = StatusGrpcClient::GetInstance()->GetChatServer(userin.uid);
      if(reply.error()){
         LOG_INFO("Grpc Get Chatserver Failed: " << reply.error());
         root["error"] = ErrorCodes::RPCFailed;
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
      }
      LOG_INFO("Success To Load Userinfo Uid: " << userin.uid);
      root["error"] = 0;
      root["email"] = email;
      root["host"] = reply.host();
      root["port"] = reply.port();
      root["uid"] = userin.uid;
      root["token"] = reply.token();
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->_response.body()) << jsonstr;
      return true;
   });

}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
   if(_get_handlers.find(path) == _get_handlers.end()){
      return false;
   }

   _get_handlers[path](con);
   return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
   if(_post_handlers.find(path) == _post_handlers.end()){
      return false;
   }

   _post_handlers[path](con);
   return true;

}
