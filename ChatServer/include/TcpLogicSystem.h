#pragma once
#include "const.h"
#include "MsgNode.h"
#include "CSession.h"
#include "userdata.h"

typedef std::function<void(std::shared_ptr<MsgNode>)> callback;

class TcpLogicSystem : public Singleton<TcpLogicSystem>{

   public:
      TcpLogicSystem();
      ~TcpLogicSystem();

      void PushTask(std::shared_ptr<MsgNode>);
      void DealMsg();
      void Close();
      
   private:

      void HandLogin(std::shared_ptr<MsgNode>);

      bool GetBaseInfo(std::string, int, std::shared_ptr<UserInfo>);
      
      
      void initHandler();
      std::shared_ptr<MsgNode> GetTask();

      std::map<MSG_IDS, callback> _handlers;
      std::queue<std::shared_ptr<MsgNode>> _Msg_queue;
      std::mutex _mutex;
      std::condition_variable _cv;
      bool _b_stop;
};