#pragma once

#include "defs.h"
#include "events.h"

namespace NTichu::NGameplay::NState {

struct TSnapshot {
    EState State;

    NCards::TCards Cards;
    TPM<ui8> CardsCount;

    EPosition ActivePlayer;
    EPosition TrickPlayer;

    TTM<NCards::TScore> Score;

    TPM<ECall> Calls;

    TPM<TPM<NCards::TCard>> Exchange;

    NCards::TCombination LastComb;

    TEvent LastEvent;
};

} // namespace NTichu::NGameplay::NState