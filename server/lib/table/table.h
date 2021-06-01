#pragma once

#include <server/lib/gameplay/state/defs.h>
#include <server/lib/types/types.h>

#include <server/lib/actors/actor.h>
#include <server/lib/gameplay/state/events.h>
#include <server/lib/gameplay/gameplay.h>

#include <functional>
#include <string>

namespace NTichu::NServer {

struct TTableId {
    ui64 Identity;
    ui64 Tag;
};

inline bool operator==(TTableId lhs, TTableId rhs) {
    return lhs.Identity == rhs.Identity && lhs.Tag == rhs.Tag;
}

struct TTaEv {
    struct TActionReq: public NActors::TBufEvent<TActionReq> {
        TActionReq(TTableId taId, NGameplay::NState::TEvent action)
            : TaId(taId)
            , Action(std::move(action))
        {
        }

        TTableId TaId;
        NGameplay::NState::TEvent Action;
    };

    struct TActionResp: public NActors::TBufEvent<TActionResp> {
        TActionResp(NGameplay::NState::TStateErrorOr errorOr)
            : ErrorOr(std::move(errorOr))
        {
        }

        NGameplay::NState::TStateErrorOr ErrorOr;
    };

    struct TSnapshot: public NActors::TBufEvent<TSnapshot> {
        TSnapshot(NGameplay::NState::TSnapshot snapshot)
            : Snapshot(std::move(snapshot))
        {
        }

        NGameplay::NState::TSnapshot Snapshot;
    };

    struct TJoinAnyReq: public NActors::TBufEvent<TJoinAnyReq> {
    };

    struct TJoinAnyResp: public NActors::TBufEvent<TJoinAnyResp> {
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
};

NActors::TWeakActor CreateTableManager(NActors::TActorSystem sys);

} // namespace NTichu::NServer

template <>
struct std::hash<NTichu::NServer::TTableId> {
    ui64 operator()(NTichu::NServer::TTableId tableId) const {
        return std::hash<ui64>()(tableId.Identity) ^ std::hash<ui64>()(tableId.Tag);
    }
};
