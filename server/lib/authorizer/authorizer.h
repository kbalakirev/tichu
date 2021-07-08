#pragma once

#include <server/lib/types/types.h>
#include <server/lib/actors/actor.h>
#include <server/lib/user/user.h>
#include <server/lib/error_or/error_or.h>

#include <string>

namespace NTichu::NServer {

using TAuthToken = std::string;

namespace NAuthorizerEvent {
    struct TRegisterError {
        enum EType {
            UNKNOWN
        };
        
        EType Type;
        std::string Message;
    };

    NEW_EVENT(TRegisterRequest) {
    };

    NEW_EVENT(TRegisterResponse) {
        explicit TRegisterResponse(TAuthToken token)
            : Response(std::move(token))
        {
        }

        explicit TRegisterResponse(TRegisterError error)
            : Response(std::move(error))
        {
        }

        TErrorOr<TAuthToken, TRegisterError> Response;
    };

    NEW_EVENT(TAuthRequest) {
        explicit TAuthRequest(TAuthToken token)
            : Token(std::move(token))
        {
        }

        TAuthToken Token;
    };

    NEW_EVENT(TAuthResponse) {
        TAuthResponse(TUserId userId)
            : UserId(userId)
        {
        }

        TUserId UserId;
    };
} // namespace NAuthorizerEvent

NActors::TActorId CreateAuthorizer();

} // namespace NTichu::NServer
