#pragma once

#include "const.h"
#include "Singleton.h"
#include "ConfigMgr.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "redisMgr.h"
#include "MysqlMgr.h"
#include "UserMgr.h"
#include <queue>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::ReplyFriendReq;
using message::ReplyFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;


using message::ChatService;

class ChatServiceImpl final: public ChatService::Service{

   public:
      ChatServiceImpl();
      Status NotifyAddFriend(ServerContext* context, const AddFriendReq*, AddFriendRsp*) override;
      Status NotifyAuthFriend(ServerContext* context, const AuthFriendReq*, AuthFriendRsp*) override;
      Status NotifyTextChatMsg(ServerContext* context, const TextChatMsgReq*, TextChatMsgRsp*) override;
      // bool GetBaseInfo(std::string ket,)
};
