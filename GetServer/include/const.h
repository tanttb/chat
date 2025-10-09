#pragma once

#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
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
   MSG_CHAT_LOGIN_RSP = 1009,
   TokenInvalid = 1010
};

struct UserInfo{
    std::string name;
    std::string email;
    int uid;
    std::string pwd;
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
