#pragma once

#include <server/lib/gameplay/state/defs.h>
#include <server/lib/types/types.h>

#include <server/lib/actors/actor.h>
#include <server/lib/gameplay/state/events.h>
#include <server/lib/gameplay/gameplay.h>

#include <functional>
#include <string>

namespace NTichu::NServer {

using TTableId = ui64;

namespace NEvTableManager {
    struct TActionReq: public NActors::TEvent<TActionReq> {
        TActionReq(TTableId taId, NGameplay::NState::TEvent action)
                : TaId(taId)
                , Action(std::move(action))
        {
        }

        TTableId TaId;
        NGameplay::NState::TEvent Action;
    };

    struct TActionResp: public NActors::TEvent<TActionResp> {
        TActionResp(NGameplay::NState::TStateErrorOr errorOr)
                : ErrorOr(std::move(errorOr))
        {
        }

        NGameplay::NState::TStateErrorOr ErrorOr;
    };

    struct TSnapshot: public NActors::TEvent<TSnapshot> {
        TSnapshot(NGameplay::NState::TSnapshot snapshot)
                : Snapshot(std::move(snapshot))
        {
        }

        NGameplay::NState::TSnapshot Snapshot;
    };

    struct TJoinAnyReq: public NActors::TEvent<TJoinAnyReq> {
    };

    struct TJoinAnyResp: public NActors::TEvent<TJoinAnyResp> {
        TJoinAnyResp(TTableId taId, NGameplay::NState::EPosition pos)
                : TaId(taId)
                , Position(pos)
        {
        }

        TJoinAnyResp(std::string mes)
                : TaId{}
                , Position(NGameplay::NState::EPosition::INVALID)
                , Status(ERROR)
                , Message(std::move(mes))
        {
        }

        enum EStatus {
            OK,
            ERROR
        };

        TTableId TaId;
        NGameplay::NState::EPosition Position;

        EStatus Status = OK;
        std::string Message;
    };
}

NActors::TActorId CreateTableManager();

} // namespace NTichu::NServer
