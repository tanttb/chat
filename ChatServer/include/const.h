#pragma once

#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <memory>
#include <iostream>
#include <map>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <hiredis/hiredis.h>

#include "utils.h"
#include "Singleton.h"
#include "ConfigMgr.h"

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using     tcp   = boost::asio::ip::tcp;

const std::string CODEPREFIX = "code_";
const std::string USERIPPREFIX = "uip_";
const std::string USERTOKENPREFIX = "utoken_";
const std::string IPCOUNTPREFIX = "ipcount_";
const std::string USER_BASE_INFO = "ubaseinfo_";
const std::string LOGIN_COUNT = "logincount_";

constexpr const int HEADLEN = 4;
constexpr const int MAX_DATA_LENGTH = 65563;
constexpr const int MAX_MSG_LENGTH = 1000;

enum ErrorCodes{
   Success = 0,
   Error_Json = 1001,
   RPCFailed = 1002,
   VarifyExpired = 1003,  
   VarifyCodeErr = 1004,  //验证码错误
   VerifyUserEmail = 1005,
   UserExist = 1006,
   PasswdInvalid = 1007,
   UidValied = 1008,
   TokenInvalid = 1010
};

enum MSG_IDS : short{
	MSG_CHAT_LOGIN = 1005,
	MSG_CHAT_LOGIN_RSP = 1006,
	ID_SEARCH_USER_REQ = 1007, 
	ID_SEARCH_USER_RSP = 1008, 
	ID_ADD_FRIEND_REQ = 1009, 
	ID_ADD_FRIEND_RSP  = 1010,
	ID_NOTIFY_ADD_FRIEND_REQ = 1011,  
	ID_AUTH_FRIEND_REQ = 1013,  
	ID_AUTH_FRIEND_RSP = 1014,  
	ID_NOTIFY_AUTH_FRIEND_REQ = 1015,
	ID_TEXT_CHAT_MSG_REQ = 1017, 
	ID_TEXT_CHAT_MSG_RSP = 1018, 
	ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, 
	ID_NOTIFY_OFF_LINE_REQ = 1021, 
	ID_HEART_BEAT_REQ = 1023,    
	ID_HEARTBEAT_RSP = 1024, 
};

namespace Color {
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string RESET = "\033[0m";
}

#define LOG_INFO(info)  {std::cout << Color::GREEN << "[INFO ]: " << info << Color::RESET <<std::endl;}
#define LOG_ERROR(info) {std::cout << Color::RED <<"[ERROR]: " << info << Color::RESET <<std::endl;}
