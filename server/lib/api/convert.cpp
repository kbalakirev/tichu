#include "convert.h"

namespace NTichu::NServer::NApi::NConvert {

using namespace NActors;

NGameplay::NState::EPosition Convert(tichu::EPosition position) {
    return (NGameplay::NState::EPosition) position;
}
tichu::EPosition Convert(NGameplay::NState::EPosition position) {
    return (tichu::EPosition) position;
}

NTable::TTableOptions Convert(const tichu::TTableOptions& value) {
    NTable::TTableOptions options;
    options.Name = value.name();

    return options;
}
tichu::TTableOptions Convert(const NTable::TTableOptions& value) {
    tichu::TTableOptions options;
    options.set_name(value.Name);

    return options;
}

NTable::TTableListItem Convert(const tichu::TTableListItem& value) {
    NTable::TTableListItem item;
    item.Table = value.table_id();
    item.Joined = value.joined();
    item.Options = Convert(value.options());
    return item;
}
tichu::TTableListItem Convert(const NTable::TTableListItem& value) {
    tichu::TTableListItem item;
    item.set_table_id(value.Table);
    item.set_joined(value.Joined);
    *(item.mutable_options()) = Convert(value.Options);
    return item;
}

tichu::TRegisterResponse ProtoFromActorMessage(NAuthorizer::TEvRegisterResponse* ev) {
    tichu::TRegisterResponse resp;
    if (ev->Response.Fail()) {
        resp.set_error(ev->Response.Error().Message);
    } else {
        resp.set_token(ev->Response.Value());
    }

    return resp;
}
tichu::TJoinTableResponse ProtoFromActorMessage(NTable::TEvJoinResponse* ev) {
    tichu::TJoinTableResponse resp;
    if (ev->Response.Fail()) {
        resp.set_error(ev->Response.Error());
    } else {
        resp.set_table_id(ev->Response.Value().Table);
        resp.set_position(Convert(ev->Response.Value().Position));
    }

    return resp;
}
tichu::TCreateTableResponse ProtoFromActorMessage(NTable::TEvCreateResponse* ev) {
    tichu::TCreateTableResponse resp;
    if (ev->Response.Fail()) {
        resp.set_error(ev->Response.Error());
    } else {
        resp.set_table_id(ev->Response.Value().Table);
        resp.set_position(Convert(ev->Response.Value().Position));
    }

    return resp;
}
tichu::TLeaveTableResponse ProtoFromActorMessage(NTable::TEvLeaveResponse* ev) {
    tichu::TLeaveTableResponse resp;
    if (ev->Response.Fail()) {
        resp.set_error(ev->Response.Error());
    }
    return resp;
}
tichu::TTableList ProtoFromActorMessage(NTable::TEvTableList* ev) {
    tichu::TTableList resp;
    for (const auto& item: ev->List) {
        *(resp.add_list()) = Convert(item);
    }

    return resp;
}

NAuthorizer::TEvRegisterRequest::TPtr ActorMessageFromProto(const tichu::TRegisterRequest&) {
    return std::make_shared<NAuthorizer::TEvRegisterRequest>();
}
NTable::TEvJoinAnyRequest::TPtr ActorMessageFromProto(const tichu::TJoinAnyTableRequest&, TUserId user) {
    return std::make_shared<NTable::TEvJoinAnyRequest>(user);
}
NTable::TEvJoinRequest::TPtr ActorMessageFromProto(const tichu::TJoinTableRequest& proto, TUserId user) {
    return std::make_shared<NTable::TEvJoinRequest>(user, proto.table_id());
}
NTable::TEvCreateRequest::TPtr ActorMessageFromProto(const tichu::TCreateTableRequest& proto, TUserId user) {
    return std::make_shared<NTable::TEvCreateRequest>(user, Convert(proto.options()));
}
NTable::TEvLeaveRequest::TPtr ActorMessageFromProto(const tichu::TLeaveTableRequest& proto, TUserId user) {
    return std::make_shared<NTable::TEvLeaveRequest>(user, proto.table_id());
}
NTable::TEvSubscribeTableList::TPtr ActorMessageFromProto(const tichu::TSubscribeTableList&) {
    return std::make_shared<NTable::TEvSubscribeTableList>();
}

} // NTichu::NServer::NApi::NConvert
