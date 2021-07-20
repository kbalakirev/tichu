#pragma once

#include <server/lib/types/types.h>
#include <server/lib/gameplay/state/defs.h>

#include <string>

namespace NTichu::NServer::NTable {

using TTableId = ui64;

struct TTableOptions {
    std::string Name;
};

struct TTablePlace {
    TTableId Table;
    NGameplay::NState::EPosition Position;
};

struct TTableListItem {
    TTableId Table;
    TTableOptions Options;
    size_t Joined{0};
};

} // namespace NTichu::NServer::NTable
