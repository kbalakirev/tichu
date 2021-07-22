#pragma once

#include <tichu_grpc_service.grpc.pb.h>

#include <server/lib/grpc/server.h>

namespace NTichu::NServer::NApi {

REGISTER_RPC_HANDLER(tichu::TTichuGrpcService, JoinAnyTable);
REGISTER_RPC_HANDLER(tichu::TTichuGrpcService, JoinTable);
REGISTER_RPC_HANDLER(tichu::TTichuGrpcService, CreateTable);
REGISTER_RPC_HANDLER(tichu::TTichuGrpcService, LeaveTable);

NGrpc::TRpcActorFactory CreateJoinAnyTableActorFactory(NActors::TActorId authorizer, NActors::TActorId tableActor);
NGrpc::TRpcActorFactory CreateJoinTableActorFactory(NActors::TActorId authorizer, NActors::TActorId tableActor);
NGrpc::TRpcActorFactory CreateCreateTableActorFactory(NActors::TActorId authorizer, NActors::TActorId tableActor);
NGrpc::TRpcActorFactory CreateLeaveTableActorFactory(NActors::TActorId authorizer, NActors::TActorId tableActor);

} // namespace NTichu::NServer::NApi
