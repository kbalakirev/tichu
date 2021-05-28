#pragma once

#include <server/lib/types/types.h>
#include <server/lib/error_or/error_or.h>

#include <array>
#include <string>

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

enum class ECall {
    UNDEFINED = 0,
    GRAND,
    SMALL,
    NONE,
};

enum class EPosition {
    POS_0 = 0,
    POS_1 = 1,
    POS_2 = 2,
    POS_3 = 3,

    NUM,
    INVALID
};

enum class ETeam {
    TEAM_0 = 0,
    TEAM_1 = 1,

    NUM,
    INVALID
};

inline bool ValidatePosition(EPosition pos) {
    return (ui32) pos >= 0 && pos < EPosition::NUM;
}

inline bool ValidateTeam(ETeam team) {
    return ETeam::TEAM_1 == team || ETeam::TEAM_0 == team;
}

inline ETeam Team(EPosition position) {
    if (!ValidatePosition(position)) {
        return ETeam::INVALID;
    }

    return (ETeam) ((ui32) position & 1u);
}

inline EPosition Teammate(EPosition position) {
    if (!ValidatePosition(position)) {
        return EPosition::INVALID;
    }
    return (EPosition) ((ui32) position ^ 2u);
}

inline ETeam OppositeTeam(ETeam team) {
    if (!ValidateTeam(team)) {
        return ETeam::INVALID;
    }
    return (ETeam) ((ui32) team ^ 1u);
}

inline EPosition NextPosition(EPosition pos) {
    if (!ValidatePosition(pos)) {
        return EPosition::INVALID;
    }

    return (EPosition) (((ui32) pos + 1) % (ui32) EPosition::NUM);
}

static constexpr std::array<EPosition, (ui32) EPosition::NUM> AllPositions {
    EPosition::POS_0,
    EPosition::POS_1,
    EPosition::POS_2,
    EPosition::POS_3
};

static constexpr std::array<ETeam, (ui32) ETeam::NUM> AllTeams {
    ETeam::TEAM_0,
    ETeam::TEAM_1
};

template <class TValue>
using TPM = std::array<TValue, (ui32) EPosition::NUM>;

template <class TValue>
using TTM = std::array<TValue, (ui32) ETeam::NUM>;

} // namespace NTichu::NGameplay::NState
