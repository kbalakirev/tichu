#pragma once

#include "def.h"

#include <server/lib/user/user.h>
#include <server/lib/error_or/error_or.h>
#include <server/lib/actors/actor.h>

#include <vector>

namespace NTichu::NServer::NTable {

using NTichu::NServer::TUserId;

NEW_EVENT(TEvJoinAnyRequest) {
    explicit TEvJoinAnyRequest(TUserId user)
        : User(user)
    {
    }

    TUserId User;
};

NEW_EVENT(TEvJoinRequest) {
    explicit TEvJoinRequest(TUserId user, TTableId table)
        : User(user)
        , Table(table)
    {
    }

    TUserId User;
    TTableId Table;
};

NEW_EVENT(TEvCreateRequest) {
    explicit TEvCreateRequest(TUserId user, TTableOptions options)
        : User(user)
        , Options(std::move(options))
    {
    }

    TUserId User;
    TTableOptions Options;
};

NEW_EVENT(TEvLeaveRequest) {
    explicit TEvLeaveRequest(TUserId user, TTableId table)
        : User(user)
        , Table(table)
    {
    }

    TUserId User;
    TTableId Table;
};

NEW_EVENT(TEvSubscribeTableList) {
};

NEW_EVENT(TEvUnSubscribeTableList) {
};

NEW_EVENT(TEvJoinResponse) {
    explicit TEvJoinResponse(TErrorOr<TTablePosition, std::string> response)
        : Response(std::move(response))
    {
    }

    TErrorOr<TTablePosition, std::string> Response;
};

NEW_EVENT(TEvCreateResponse) {
    explicit TEvCreateResponse(TTablePosition response)
        : Response(response)
    {
    }

    explicit TEvCreateResponse(const std::string& response)
        : Response(response)
    {
    }

    TErrorOr<TTablePosition, std::string> Response;
};

NEW_EVENT(TEvLeaveResponse) {
    explicit TEvLeaveResponse(const std::string& response)
        : Response(response)
    {
    }

    TEvLeaveResponse() = default;

    TErrorOr<void, std::string> Response;
};

NEW_EVENT(TEvTableList) {
    explicit TEvTableList(std::vector<TTableListItem> list)
        : List(std::move(list))
    {
    }

    std::vector<TTableListItem> List;
};

} // namespace NTichu::NServer::NTable
