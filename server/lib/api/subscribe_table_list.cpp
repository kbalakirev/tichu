#include "subscribe_table_list.h"
#include "convert.h"

#include <cassert>

using namespace NActors;

namespace NTichu::NServer::NApi {
namespace {
    
class TRpcActor: public IActor {
public:
    using TRpc = TSubscribeTableListHandler;

    explicit TRpcActor(TActorConfig& config, NGrpc::TRpcContext<TRpc>* ctx, TActorId tableActor)
        : IActor(config)
        , Ctx_(ctx)
        , TableActor_(std::move(tableActor))
    {
    }

private:
    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(NGrpc::TEvLostConnection, OnLostConnection);
        CREATE_HANDLER(NTable::TEvTableList, OnTableList);
    }

    void OnLostConnection(NGrpc::TEvLostConnection::TPtr) {
        Ctx_->Destroy();
        SelfDestroy();
    }

    void OnTableList(NTable::TEvTableList::TPtr ev) {
        Ctx_->Write(NConvert::ProtoFromActorMessage(ev.get()));
    }

    THandler Bootstrap() override {
        Send(TableActor_, NConvert::ActorMessageFromProto(Ctx_->GetRequest()));

        return AS_STATE_FUNC(StateWork);
    }

    void SelfDestroy() {
        Send(TableActor_, MakeEvent<NTable::TEvUnSubscribeTableList>());
        Quit();
    }

private:
    NGrpc::TRpcContext<TRpc>* Ctx_;
    TActorId TableActor_;
};

}

NGrpc::TRpcActorFactory CreateSubscribeTableListActorFactory(NActors::TActorId tableActor) {
    return [table = std::move(tableActor)](NGrpc::IGrpcContext* ctx) {
        auto* casted = dynamic_cast<NGrpc::TRpcContext<typename TRpcActor::TRpc>*>(ctx);
        assert(casted);
        return TActorSystem::Instance().Spawn<TRpcActor>(casted, std::move(table));
    };
}

} // namespace NTichu::NServer::NApi
