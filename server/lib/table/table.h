#pragma once

#include <server/lib/gameplay/state/defs.h>
#include <server/lib/types/types.h>

#include <server/lib/actors/actor.h>
#include <server/lib/user/user.h>
#include <server/lib/gameplay/state/events.h>
#include <server/lib/gameplay/state/state.h>
#include <server/lib/gameplay/gameplay.h>

#include <functional>
#include <string>

namespace NTichu::NServer {

using TTableId = ui64;

struct TTableOptions {
    std::string Name;
};

struct TTableListItem {
    TTableId TableId;
    TTableOptions Options;
    size_t Count{0};
};

namespace NTableEvent {
    struct TTable {
        TTableId TableId;
        NTichu::NGameplay::NState::EPosition Position;
    };

    NEW_EVENT(TJoinAnyRequest) {
    };

    NEW_EVENT(TGameActionRequest) {
        explicit TGameActionRequest(NTichu::NGameplay::NState::TEvent event)
            : Event(std::move(event))
        {
        }

        NTichu::NGameplay::NState::TEvent Event;
    };

    NEW_EVENT(TGameActionResponse) {
        explicit TGameActionResponse(NTichu::NGameplay::NState::TStateErrorOr response)
            : Response(std::move(response))
        {
        }

        NTichu::NGameplay::NState::TStateErrorOr Response;
    };

    NEW_EVENT(TCreateTableRequest) {
        explicit TCreateTableRequest(TTableOptions options)
            : Options(std::move(options))
        {
        }

        TTableOptions Options;
    };

    NEW_EVENT(TCreateTableResponse) {
        explicit TCreateTableResponse(TTable table)
            : Table(table)
        {
        }

        TTable Table;
    };

    NEW_EVENT(TJoinTableRequest) {
        explicit TJoinTableRequest(TTableId table)
            : Table(table)
        {
        }

        TTableId Table;
    };

    struct TFindTableError {
        enum EType {
            UNKNOWN,
            TABLE_IS_FULL,
            TABLE_NOT_EXISTS
        };

        EType Type{UNKNOWN};
        std::string Message;
    };

    NEW_EVENT(TJoinTableResponse) {
        explicit TJoinTableResponse(TFindTableError error)
            : Response(std::move(error))
        {
        }

        explicit TJoinTableResponse(TTable table)
            : Response(table)
        {
        }

        TErrorOr<TTable, TFindTableError> Response;
    };

    NEW_EVENT(TSubscribeRequest) {
    };

    NEW_EVENT(TSubscribeEvent) {
        explicit TSubscribeEvent(std::vector<TTableListItem> tables)
            : Tables(std::move(tables))
        {
        }
        
        std::vector<TTableListItem> Tables;
    };

    NEW_EVENT(TUnSubscribeRequest) {
    };

    NEW_EVENT(TLeaveTableRequest) {
        explicit TLeaveTableRequest(TTable table)
            : Table(table)
        {
        }

        TTable Table;
    };

    NEW_EVENT(TLeaveTableResponse) {
        explicit TLeaveTableResponse(TFindTableError error)
            : Response(std::move(error))
        {
        }

        explicit TLeaveTableResponse(TTable table)
            : Response(table)
        {
        }

        TErrorOr<TTable, TFindTableError> Response;
    };

} // namespace NTableEvent

NActors::TActorId CreateTableManager();

} // namespace NTichu::NServer
