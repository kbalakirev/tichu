#pragma once

#include <server/lib/types/types.h>
#include <server/lib/actors/actor.h>
#include <server/lib/user/user.h>

#include <string>

namespace NTichu::NServer {

struct TAuthToken {
    ui64 Identity{0};
    ui64 Tag{0};
};

inline bool operator==(TAuthToken lhs, TAuthToken rhs) {
    return lhs.Identity == rhs.Identity && lhs.Tag == rhs.Tag;
}

struct TAuthEv {
    struct TRegisterReq: public NActors::TBufEvent<TRegisterReq> {
    };

    struct TRegisterResp: public NActors::TBufEvent<TRegisterResp> {
        explicit TRegisterResp(TAuthToken token)
            : Token(token)
        {
        }

        TAuthToken Token;
    };

    struct TProxyReq: public NActors::TBufEvent<TProxyReq> {
        TProxyReq(TAuthToken token, NActors::TEventPtr event)
            : Token(std::move(token))
            , Event(std::move(event))
        {
        }

        TAuthToken Token;
        NActors::TEventPtr Event;
    };

    struct TProxyError: public NActors::TBufEvent<TProxyError> {
        explicit TProxyError(std::string message)
            : Message(std::move(message))
        {
        }

        std::string Message;
    };
};

NActors::TWeakActor CreateAuthorizer(NActors::TActorSystem sys, NActors::TWeakActor);

} // namespace NTichu::NServer

template <>
struct std::hash<NTichu::NServer::TAuthToken> {
    ui64 operator()(NTichu::NServer::TAuthToken token) const {
        return std::hash<ui64>()(token.Identity) ^ std::hash<ui64>()(token.Tag);
    }
};
