#pragma once

#include "defs.h"

#include <server/lib/gameplay/cards/cards.h>

#include <variant>

namespace NTichu::NGameplay::NState {

struct TEvGTChoice {
    EPosition Who{EPosition::INVALID};
    bool Choice;
};

struct TEvExchange {
    EPosition Who{EPosition::INVALID};
    TPM<NCards::TCard> Exchange;
};

struct TEvCall {
    EPosition Who{EPosition::INVALID};
};

struct TEvMove {
    EPosition Who{EPosition::INVALID};
    NCards::TCombination Comb;
};

struct TEvPass {
    EPosition Who;
};

struct TEvGiveDragon {
    EPosition From;
    EPosition To;
};

using TEvent = std::variant<
        TEvGTChoice,
        TEvExchange,
        TEvCall,
        TEvMove,
        TEvPass,
        TEvGiveDragon>;

} // namespace NTichu::NGameplay::NState
