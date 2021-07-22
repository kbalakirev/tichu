#include "table_move.h"
#include "convert.h"

#include <cassert>

using namespace NActors;

namespace NTichu::NServer::NApi {
namespace {
    
template <class TRpc, class TTableEvent>
class TRpcActor: public IActor {
public:
    explicit TRpcActor(TActorConfig& config, NGrpc::TRpcContext<TRpc>* ctx, TActorId authorizer, TActorId tableActor)
        : IActor(config)
        , Ctx_(ctx)
        , Authorizer_(std::move(authorizer))
        , TableActor_(std::move(tableActor))
    {
    }

    static NGrpc::TRpcActorFactory Factory(TActorId authorizer, TActorId tableActor) {
        return [auth = std::move(authorizer), table = std::move(tableActor)](NGrpc::IGrpcContext* ctx) {
            auto* casted = dynamic_cast<NGrpc::TRpcContext<TRpc>*>(ctx);
            assert(casted);
            return TActorSystem::Instance().Spawn<TRpcActor>(casted, std::move(auth), std::move(table));
        };
    }

private:
    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(NGrpc::TEvLostConnection, OnLostConnection);
        CREATE_HANDLER(NAuthorizer::TEvAuthResponse, OnAuthResponse);
        CREATE_HANDLER(TTableEvent, OnTableResponse);
    }

    void OnLostConnection(NGrpc::TEvLostConnection::TPtr) {
        Ctx_->Destroy();
        Quit();
    }

    void OnAuthResponse(NAuthorizer::TEvAuthResponse::TPtr ev) {
        if (ev->UserId == INVALID_USER_ID) {
            typename TRpc::TResponse response;
            response.set_error("unknown auth token");
            Ctx_->Finish(response, grpc::Status::OK);
            Quit();
        } else {
            Send(TableActor_, NConvert::ActorMessageFromProto(Ctx_->GetRequest(), ev->UserId));
        }
    }

    void OnTableResponse(TTableEvent::TPtr ev) {
        Ctx_->Finish(NConvert::ProtoFromActorMessage(ev.get()), grpc::Status::OK);
        Quit();
    }

    THandler Bootstrap() override {
        Send(Authorizer_, MakeEvent<NAuthorizer::TEvAuthRequest>(Ctx_->GetRequest().token()));

        return AS_STATE_FUNC(StateWork);
    }

private:
    NGrpc::TRpcContext<TRpc>* Ctx_;
    TActorId Authorizer_;
    TActorId TableActor_;
};

}

NGrpc::TRpcActorFactory CreateJoinAnyTableActorFactory(NActors::TActorId authorizer, NActors::TActorId tableActor) {
    return TRpcActor<TJoinAnyTableHandler, NTable::TEvJoinResponse>::Factory(std::move(authorizer), std::move(tableActor));
}
NGrpc::TRpcActorFactory CreateJoinTableActorFactory(NActors::TActorId authorizer, NActors::TActorId tableActor) {
    return TRpcActor<TJoinTableHandler, NTable::TEvJoinResponse>::Factory(std::move(authorizer), std::move(tableActor));
}
NGrpc::TRpcActorFactory CreateCreateTableActorFactory(NActors::TActorId authorizer, NActors::TActorId tableActor) {
    return TRpcActor<TCreateTableHandler, NTable::TEvCreateResponse>::Factory(std::move(authorizer), std::move(tableActor));
}
NGrpc::TRpcActorFactory CreateLeaveTableActorFactory(NActors::TActorId authorizer, NActors::TActorId tableActor) {
    return TRpcActor<TLeaveTableHandler, NTable::TEvLeaveResponse>::Factory(std::move(authorizer), std::move(tableActor));
}

} // namespace NTichu::NServer::NApi
