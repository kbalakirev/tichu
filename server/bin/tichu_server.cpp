#include <server/lib/api/api.h>

#include <server/lib/authorizer/authorizer.h>
#include <server/lib/table/table.h>

using namespace NActors;
using namespace NTichu::NServer;

void ActorsMain() {
    NGrpc::TGrpcServerOptions options{
        .Address = std::string("10.129.0.6:4760")
    };

    auto authorizer = NAuthorizer::CreateAuthorizer();
    auto tableManager = NTable::CreateTableManager();

    NGrpc::TGrpcServer<tichu::TTichuGrpcService> server(std::move(options));

    server.Initialize();
    server.RegisterRpc<NApi::TRegisterHandler>(NApi::CreateRegisterActorFactory(authorizer));

    server.RegisterRpc<NApi::TJoinAnyTableHandler>(NApi::CreateJoinAnyTableActorFactory(authorizer, tableManager));

    server.RegisterRpc<NApi::TJoinTableHandler>(NApi::CreateJoinTableActorFactory(authorizer, tableManager));

    server.RegisterRpc<NApi::TCreateTableHandler>(NApi::CreateJoinTableActorFactory(authorizer, tableManager));

    server.RegisterRpc<NApi::TJoinTableHandler>(NApi::CreateCreateTableActorFactory(authorizer, tableManager));

    server.RegisterRpc<NApi::TSubscribeTableListHandler>(NApi::CreateSubscribeTableListActorFactory(tableManager));

    server.Run();
}

ACTORS_MAIN();
