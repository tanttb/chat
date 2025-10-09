
#pragma once
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"

class AsioIOServicePool : public Singleton<AsioIOServicePool>{
   friend class Singleton<AsioIOServicePool>;
   public:
      using IOService = boost::asio::io_context;
      using Work =boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
      using WorkPtr = std::unique_ptr<Work>;
      ~AsioIOServicePool();
      AsioIOServicePool(const AsioIOServicePool&) = delete;
      AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
      boost::asio::io_context &GetIOService();
      void Stop();

   private:
      AsioIOServicePool(std::size_t size = 2);
      std::vector<IOService> _ioServices;
      std::vector<WorkPtr> _works;
      std::vector<std::thread> _thread;
      std::size_t _nextIOService;

};