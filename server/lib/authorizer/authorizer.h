#pragma once

#include <server/lib/types/types.h>
#include <server/lib/actors/actor.h>
#include <server/lib/user/user.h>

#include <string>

namespace NTichu::NServer {

using TAuthToken = std::string;

struct TAuthorizerEvent {
};

namespace NEvAuthorizer {
    struct TRegisterReq: public NActors::TEvent<TRegisterReq> {
    };

    struct TRegisterResp: public NActors::TEvent<TRegisterResp> {
        explicit TRegisterResp(TAuthToken token)
                : Token(token)
        {
        }

        TAuthToken Token;
    };

    struct TProxyReq: public NActors::TEvent<TProxyReq> {
        TProxyReq(TAuthToken token, NActors::TActorId to, NActors::TEventPtr event)
                : Token(std::move(token))
                , To(std::move(to))
                , Event(std::move(event))
        {
        }

        TAuthToken Token;
        NActors::TActorId To;
        NActors::TEventPtr Event;
    };

    struct TProxyError: public NActors::TEvent<TProxyError> {
        explicit TProxyError(std::string message)
                : Message(std::move(message))
        {
        }

        std::string Message;
    };
}

NActors::TActorId CreateAuthorizer();

} // namespace NTichu::NServer
