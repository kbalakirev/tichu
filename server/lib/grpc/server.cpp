#include "server.h"

using namespace NActors;

namespace NGrpc {

void IGrpcContext::Destroy() {
    delete Tag_;
}

IGrpcContext::IGrpcContext(TCallData* tag)
    : Tag_(tag)
{
}

void IGrpcContext::Write(const TGrpcMessage& message) {
    Tag_->OnWrite();
    WriteImpl(message);
}

void IGrpcContext::Finish(const TGrpcMessage& message, grpc::Status status) {
    Tag_->OnFinish();
    FinishImpl(message, status);
}

void TCallData::OnWrite() {
    ChangeStatus(WRITING);
}

void TCallData::OnFinish() {
    ChangeStatus(FINISH);
}

TCallData::TCallData(TGrpcContextFactory ctxFactory, TRpcActorFactory actorFactory)
    : CtxFactory_(std::move(ctxFactory))
    , ActorFactory_(std::move(actorFactory))
{
    ChangeStatus(CREATE);
    Proceed(true);
}

void TCallData::Proceed(bool ok) {
    if (!ok) {
        if (Actor_) {
            TActorSystem::Send(TActorId(), Actor_, MakeEvent<TEvLostConnection>());
        }

        if (GetStatus() != CREATE) {
            return;
        }

        Proceed(true);

        return;
    }

    switch (GetStatus()) {
        case REQUEST: {
            ChangeStatus(PROCESSING);
            Actor_ = ActorFactory_(Ctx_.get());
            new TCallData(std::move(CtxFactory_), std::move(ActorFactory_));

            break;
        }
        case WRITING: {
            ChangeStatus(PROCESSING);

            break;
        }
        case FINISH: {
            delete this;

            break;
        }
        case CREATE: {
            Ctx_ = CtxFactory_(this);
            ChangeStatus(REQUEST);
            Ctx_->Listen();

            break;
        }
    }
}

TCallData::EStatus TCallData::GetStatus() const {
    return Status_.load(std::memory_order_relaxed);
}

void TCallData::ChangeStatus(TCallData::EStatus status) {
    Status_.store(status, std::memory_order_relaxed);
}

} // namespace NGrpc