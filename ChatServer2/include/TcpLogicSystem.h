#pragma once
#include "const.h"
#include "MsgNode.h"
#include "CSession.h"

typedef std::function<void(std::shared_ptr<MsgNode>)> callback;

class TcpLogicSystem : public Singleton<TcpLogicSystem>{

   public:
      TcpLogicSystem();
      ~TcpLogicSystem();

      void PushTask(std::shared_ptr<MsgNode>);

   private:

      void HandLogin(std::shared_ptr<MsgNode>);


      void Close();
      void DealMsg();
      void initHandler();
      std::shared_ptr<MsgNode> GetTask();

      std::map<MSG_IDS, callback> _handlers;
      std::queue<std::shared_ptr<MsgNode>> _Msg_queue;
      std::mutex _mutex;
      std::condition_variable _cv;
      bool _b_stop;
};