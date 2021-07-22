#include "register.h"
#include "convert.h"

#include <cassert>

using namespace NActors;

namespace NTichu::NServer::NApi {
namespace {

class TRpcActor: public IActor {
public:
    using TRpc = TRegisterHandler;

    explicit TRpcActor(TActorConfig& config, NGrpc::TRpcContext<TRpc>* ctx, TActorId authorizer)
        : IActor(config)
        , Ctx_(ctx)
        , Authorizer_(std::move(authorizer))
    {
    }

private:
    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(NGrpc::TEvLostConnection, OnLostConnection);
        CREATE_HANDLER(NAuthorizer::TEvRegisterResponse, OnRegisterResponse);
    }
    
    void OnLostConnection(NGrpc::TEvLostConnection::TPtr) {
        Ctx_->Destroy();
        Quit();
    }

    void OnRegisterResponse(NAuthorizer::TEvRegisterResponse::TPtr ev) {
        Ctx_->Finish(NConvert::ProtoFromActorMessage(ev.get()), grpc::Status::OK);
        Quit();
    }

    THandler Bootstrap() override {
        Send(Authorizer_, NConvert::ActorMessageFromProto(Ctx_->GetRequest()));

        return AS_STATE_FUNC(StateWork);
    }

private:
    NGrpc::TRpcContext<TRpc>* Ctx_;
    TActorId Authorizer_;
};

}

NGrpc::TRpcActorFactory CreateRegisterActorFactory(NActors::TActorId authorizer) {
    return [auth = std::move(authorizer)](NGrpc::IGrpcContext* ctx) {
        auto* casted = dynamic_cast<NGrpc::TRpcContext<TRegisterHandler>*>(ctx);
        assert(casted);
        return TActorSystem::Instance().Spawn<TRpcActor>(casted, std::move(auth));
    };
}

} // namespace NTichu::NServer::NApi
