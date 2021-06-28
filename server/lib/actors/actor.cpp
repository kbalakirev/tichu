#include "actor.h"

namespace NActors {

TActorId TEventBase::Sender() const {
    return Sender_;
}

IActor::IActor(TActorConfig& config)
    : TBase(config)
    , Holder_(new THolder{caf::actor_cast<caf::actor>(address())})
{
}

TActorId IActor::Self() const {
    return address();
}

void IActor::Quit() {
    delete Holder_;
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

void IActor::Send(NActors::TActorId to, NActors::TEventPtr ev) {
    if (ev) {
        ev->Sender_ = Self();
    }

    send(caf::actor_cast<caf::actor>(to.Actor_), std::move(ev));
}

void IActor::Schedule(NActors::TActorId to, NActors::TEventPtr ev, TDuration delay) {
    if (ev) {
        ev->Sender_ = Self();
    }

    delayed_send(caf::actor_cast<caf::actor>(to.Actor_), delay, std::move(ev));
}

TActorSystem::TActorSystem(caf::actor_system& sys)
    : Sys_(sys)
{
}

void TActorSystem::Send(TActorId from, TActorId to, TEventPtr ev) {
    if (ev) {
        ev->Sender_ = from;
    }
    caf::send_as(caf::actor_cast<caf::actor>(from.Actor_), caf::actor_cast<caf::actor>(to.Actor_), std::move(ev));
}

} // namespace NActors

