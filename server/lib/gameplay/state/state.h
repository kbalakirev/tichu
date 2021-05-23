#pragma once

#include "events.h"
#include "snapshot.h"

#include <server/lib/error_or/error_or.h>
#include <server/lib/gameplay/state/generator.h>

#include <string>
#include <memory>

namespace NTichu::NGameplay::NState {

struct TStateError {
    enum EType {
        UNKNOWN
    };

    EType Type;
    std::string Message = "";
};

using TStateErrorOr = TErrorOr<void, TStateError>;

enum class EState {
    NOT_INITIALIZED = 0,
    NEW_ROUND,
    EXCHANGING,
    GAME_OVER,
    NEW_TURN,
    TURN,
    GIVE_DRAGON,

    NUM,
    INVALID
};

class IState {
public:
    virtual void Start() = 0;

    virtual TStateErrorOr GTChoice(TEvGTChoice) = 0;
    virtual TStateErrorOr Exchange(TEvExchange) = 0;
    virtual TStateErrorOr Move(TEvMove) = 0;
    virtual TStateErrorOr Pass(TEvPass) = 0;
    virtual TStateErrorOr GiveDragon(TEvGiveDragon) = 0;

    virtual TSnapshot Snapshot(EPosition pos) const = 0;
    virtual EState State() const = 0;

    virtual ~IState() = default;
};

std::unique_ptr<IState> CreateState(IGeneratorPtr generator);

} // namespace NTichu::NGameplay::NState
