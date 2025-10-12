#include "../include/ChatServiceImpl.h"


ChatServiceImpl::ChatServiceImpl()
{
   
}

Status ChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq*, AddFriendRsp*)
{
   return Status::OK;
}

Status ChatServiceImpl::NotifyAuthFriend(ServerContext* context, const AuthFriendReq*, AuthFriendRsp*)
{
   return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(ServerContext* context, const TextChatMsgReq*, TextChatMsgRsp*)
{
   return Status::OK;
}
