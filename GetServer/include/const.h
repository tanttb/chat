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

#include "utils.h"
#include "Singleton.h"
#include "ConfigMgr.h"

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using     tcp   = boost::asio::ip::tcp;

enum ErrorCodes{
   Success = 0,
   Error_Json = 1001,
   RPCFailed = 1002
};
