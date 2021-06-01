#include "actor.h"

namespace NActors {

TWeakActor TEventBase::Sender() const {
    return Sender_;
}

IActor::IActor(TActorConfig& config)
    : TBase(config)
{
}

TWeakActor IActor::Self() const {
    return address();
}

void IActor::Quit() {
    quit();
}

void IActor::Become(NActors::IActor::THandler handler) {
    become(handler);
}

caf::behavior IActor::make_behavior() {
    return Bootstrap();
}

TActorSystem IActor::System() {
    return TActorSystem(home_system());
}

void IActor::Send(NActors::TWeakActor to, NActors::TEventPtr ev) {
    if (ev) {
        ev->Sender_ = Self();
    }

    send(caf::actor_cast<caf::actor>(to.Actor_), std::move(ev));
}

void IActor::Schedule(NActors::TWeakActor to, NActors::TEventPtr ev, TDuration delay) {
    if (ev) {
        ev->Sender_ = Self();
    }

    delayed_send(caf::actor_cast<caf::actor>(to.Actor_), delay, std::move(ev));
}

TActorSystem::TActorSystem(caf::actor_system& sys)
    : Sys_(sys)
{
}

void TActorSystem::Send(TWeakActor from, TWeakActor to, TEventPtr ev) {
    if (ev) {
        ev->Sender_ = from;
    }
    caf::send_as(caf::actor_cast<caf::actor>(from.Actor_), caf::actor_cast<caf::actor>(to.Actor_), std::move(ev));
}

void TBufEventBase::Pop() {
    if (!CallStack_.empty()) {
        CallStack_.pop_back();
    }
}

TWeakActor TBufEventBase::Top() const {
    if (!CallStack_.empty()) {
        return CallStack_.back();
    }

    return TWeakActor{};
}

void TBufEventBase::Push(TWeakActor sender) {
    CallStack_.emplace_back(std::move(sender));
}

} // namespace NActors

