#pragma once

#include <tichu_grpc_service.pb.h>

#include <server/lib/table/events.h>
#include <server/lib/table/def.h>
#include <server/lib/authorizer/events.h>
#include <server/lib/authorizer/def.h>

namespace NTichu::NServer::NApi::NConvert {

NGameplay::NState::EPosition Convert(tichu::EPosition position);
tichu::EPosition Convert(NGameplay::NState::EPosition position);

NTable::TTableOptions Convert(const tichu::TTableOptions&);
tichu::TTableOptions Convert(const NTable::TTableOptions&);

NTable::TTableListItem Convert(const tichu::TTableListItem&);
tichu::TTableListItem Convert(const NTable::TTableListItem&);

tichu::TRegisterResponse ProtoFromActorMessage(NAuthorizer::TEvRegisterResponse*);
tichu::TJoinTableResponse ProtoFromActorMessage(NTable::TEvJoinResponse*);
tichu::TCreateTableResponse ProtoFromActorMessage(NTable::TEvCreateResponse*);
tichu::TLeaveTableResponse ProtoFromActorMessage(NTable::TEvLeaveResponse*);
tichu::TTableList ProtoFromActorMessage(NTable::TEvTableList*);

NAuthorizer::TEvRegisterRequest::TPtr ActorMessageFromProto(const tichu::TRegisterRequest&);
NTable::TEvJoinAnyRequest::TPtr ActorMessageFromProto(const tichu::TJoinAnyTableRequest&, TUserId);
NTable::TEvJoinRequest::TPtr ActorMessageFromProto(const tichu::TJoinTableRequest&, TUserId);
NTable::TEvCreateRequest::TPtr ActorMessageFromProto(const tichu::TCreateTableRequest&, TUserId);
NTable::TEvLeaveRequest::TPtr ActorMessageFromProto(const tichu::TLeaveTableRequest&, TUserId);
NTable::TEvSubscribeTableList::TPtr ActorMessageFromProto(const tichu::TSubscribeTableList&);

} // NTichu::NServer::NApi::NConvert
