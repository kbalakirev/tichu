#pragma once

#include <server/lib/types/types.h>
#include <server/lib/actors/actor.h>
#include <server/lib/table/table.h>

#include <server/lib/gameplay/state/events.h>

#include <functional>
#include <string>

namespace NTichu::NServer {

using TUserId = ui64;

constexpr TUserId INVALID_USER_ID = 0;

namespace NEvUserManager {
    struct TProxy: NActors::TEvent<TProxy> {
        TProxy(TUserId userId, NActors::TEventPtr event)
                : UserId(userId)
                , Event(std::move(event))
        {
        }

        TUserId UserId;
        NActors::TEventPtr Event;
    };

    struct TTableStreamReq: public NActors::TEvent<TTableStreamReq> {
    };

    struct TTableStreamError: public NActors::TEvent<TTableStreamError> {
    };
}

struct TUserManagerOptions {
    NActors::TActorId TableManager;
};

NActors::TActorId CreateUserManager(TUserManagerOptions options);

} // namespace NTichu::NServer
