#pragma once

#include "def.h"

#include <server/lib/actors/actor.h>
#include <server/lib/user/user.h>

#include <string>

namespace NTichu::NServer::NAuthorizer {

struct TRegisterError {
    enum EType {
        UNKNOWN
    };
    
    EType Type;
    std::string Message;
};

NEW_EVENT(TEvRegisterRequest) {
};

NEW_EVENT(TEvRegisterResponse) {
    explicit TEvRegisterResponse(TAuthToken token)
        : Response(std::move(token))
    {
    }

    explicit TEvRegisterResponse(TRegisterError error)
        : Response(std::move(error))
    {
    }

    TErrorOr<TAuthToken, TRegisterError> Response;
};

NEW_EVENT(TEvAuthRequest) {
    explicit TEvAuthRequest(TAuthToken token)
        : Token(std::move(token))
    {
    }

    TAuthToken Token;
};

NEW_EVENT(TEvAuthResponse) {
    TEvAuthResponse(TUserId userId)
        : UserId(userId)
    {
    }

    TUserId UserId;
};

} // namespace NTichu::NServer::NAuthorizer
