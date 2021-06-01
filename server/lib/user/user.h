#pragma once

#include <server/lib/types/types.h>
#include <server/lib/actors/actor.h>
#include <server/lib/table/table.h>

#include <server/lib/gameplay/state/events.h>

#include <functional>
#include <string>

namespace NTichu::NServer {

struct TUserId {
    ui64 Identity;
    ui64 Tag;
};

struct TUserEv {
    struct TProxy: NActors::TBufEvent<TProxy> {
        TProxy(TUserId userId, NActors::TEventPtr event)
            : UserId(userId)
            , Event(std::move(event))
        {
        }

        TUserId UserId;
        NActors::TEventPtr Event;
    };
};

inline bool operator==(TUserId lhs, TUserId rhs) {
    return lhs.Identity == rhs.Identity && lhs.Tag == rhs.Tag;
}

NActors::TWeakActor CreateUserManager(NActors::TActorSystem sys, NActors::TWeakActor tableManager);

} // namespace NTichu::NServer

template <>
struct std::hash<NTichu::NServer::TUserId> {
    ui64 operator()(NTichu::NServer::TUserId userId) const {
        return std::hash<ui64>()(userId.Identity) ^ std::hash<ui64>()(userId.Tag);
    }
};
