#pragma once

#include <tichu_grpc_service.grpc.pb.h>

#include <server/lib/grpc/server.h>

namespace NTichu::NServer::NApi {

REGISTER_RPC_HANDLER(tichu::TTichuGrpcService, SubscribeTableList);

NGrpc::TRpcActorFactory CreateSubscribeTableListActorFactory(NActors::TActorId tableActor);

} // namespace NTichu::NServer::NApi
