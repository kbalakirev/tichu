#pragma once

#include "events.h"

#include <server/lib/actors/actor.h>

#include <string>

namespace NTichu::NServer::NAuthorizer {

NActors::TActorId CreateAuthorizer();

} // namespace NTichu::NServer::NAuthorizer
