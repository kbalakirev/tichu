#pragma once

#include <tichu_grpc_service.grpc.pb.h>

#include <server/lib/grpc/server.h>

namespace NTichu::NServer::NApi {

REGISTER_RPC_HANDLER(tichu::TTichuGrpcService, Register);

NGrpc::TRpcActorFactory CreateRegisterActorFactory(NActors::TActorId authorizer);

} // namespace NTichu::NServer::NApi
