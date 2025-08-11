#include "AsioIOServicePool.h"
#include <iostream>
using namespace std;

AsioIOServicePool::AsioIOServicePool(std::size_t size) : _ioServices(size),
_works(size), _nextIOService(0)
{
   for(size_t i = 0; i < size; i++){
      _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i].get_executor()));
   }

   for(size_t i = 0; i < _ioServices.size(); i++){
      _thread.emplace_back([this, i](){
         _ioServices[i].run();
      });
   }
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
   
   auto &service = _ioServices[_nextIOService++];
   _nextIOService %= _ioServices.size();
   return service;
}

void AsioIOServicePool::Stop()
{
   for(auto &w : _works){
      w.reset();
   }

   for(auto & t : _thread){
      t.join();
   }
}

AsioIOServicePool::~AsioIOServicePool()
{
   Stop();
   cout << "AsioIOService destruct" << endl;
}

