#pragma once

#include "defs.h"

#include <server/lib/gameplay/cards/cards.h>

#include <variant>
#include <cassert>

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
        std::monostate,
        TEvGTChoice,
        TEvExchange,
        TEvCall,
        TEvMove,
        TEvPass,
        TEvGiveDragon>;

inline EPosition& Sender(TEvent& ev) {
    switch (ev.index()) {
        case 1: {
            return std::get<TEvGTChoice>(ev).Who;
        }
        case 2: {
            return std::get<TEvExchange>(ev).Who;
        }
        case 3: {
            return std::get<TEvCall>(ev).Who;
        }
        case 4: {
            return std::get<TEvMove>(ev).Who;
        }
        case 5: {
            return std::get<TEvPass>(ev).Who;
        }
        case 6: {
            return std::get<TEvGiveDragon>(ev).From;
        }
        default: {
            assert(false);
        }
    }
}

class IEventConsumer {
public:
    virtual TStateErrorOr GTChoice(TEvGTChoice) = 0;
    virtual TStateErrorOr Exchange(TEvExchange) = 0;
    virtual TStateErrorOr Move(TEvMove) = 0;
    virtual TStateErrorOr Pass(TEvPass) = 0;
    virtual TStateErrorOr GiveDragon(TEvGiveDragon) = 0;
    virtual TStateErrorOr Call(TEvCall) = 0;

    TStateErrorOr OnAction(TEvent ev) {
        switch (ev.index()) {
            case 0: {
                return TStateError{TStateError::UNKNOWN, "uninitialized action"};
            }
            case 1: {
                return GTChoice(std::get<TEvGTChoice>(ev));
            }
            case 2: {
                return Exchange(std::get<TEvExchange>(ev));
            }
            case 3: {
                return Call(std::get<TEvCall>(ev));
            }
            case 4: {
                return Move(std::get<TEvMove>(ev));
            }
            case 5: {
                return Pass(std::get<TEvPass>(ev));
            }
            case 6: {
                return GiveDragon(std::get<TEvGiveDragon>(ev));
            }
            default: {
                return TStateError{TStateError::UNKNOWN, "unsupported action"};
            }
        }
    }
};

} // namespace NTichu::NGameplay::NState
