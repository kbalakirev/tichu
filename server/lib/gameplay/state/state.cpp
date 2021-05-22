#include "state.h"

#include "cassert"

namespace NTichu::NGameplay::NState {

using namespace NCards;

struct TEvNone {
};

enum class EInnerState {
    NOT_INITIALIZED = (ui32) EState::NOT_INITIALIZED,
    NEW_ROUND = (ui32) EState::NEW_ROUND,
    EXCHANGING = (ui32) EState::EXCHANGING,
    GAME_OVER = (ui32) EState::GAME_OVER,
    NEW_TURN = (ui32) EState::NEW_TURN,
    TURN = (ui32) EState::TURN,
    GIVE_DRAGON = (ui32) EState::GIVE_DRAGON,

    INVALID = (ui32) EState::INVALID,
    TRICK,
    ROUND_OVER
};

struct TInnerState {
    TInnerState() {

    }
    void ResetRound() {

    }
    void ResetState() {

    }

    IGeneratorPtr Generator;
    TPM<NCards::TCards> Cards;
    TPM<NCards::TCards> Trick;

    TPM<ECall> Call;
    TPM<TPM<NCards::TCard>> Exchange;

    NCards::TCombination LastComb;
    NCards::TCards ActiveCards;
    EPosition ActivePlayer;
    EPosition TrickPlayer;
    EPosition DonePlayer;

    TTM<NCards::TScore> Score;
};

class TState: public IState {
public:
    TState(IGeneratorPtr generator) {

    }

public:
    // TODO: use max score from config
    static constexpr ui32 MAX_SCORE = 1000;

    static constexpr ui32 CARDS_INIT = 8;
    static constexpr ui32 CARDS_TOTAL = 14;

private:
    template <class TEv>
    TStateErrorOr Advance(TEv ev);

    void Start() override {
        auto _ = Advance(TEvNone());
    }

    TStateErrorOr Move(TEvMove move) override {
        return Advance(move);
    }

    TStateErrorOr Pass(TEvPass pass) override {
        return Advance(pass);
    }

    TStateErrorOr GTChoice(TEvGTChoice choice) override {
        return Advance(choice);
    }

    TStateErrorOr GiveDragon(TEvGiveDragon give) override {
        return Advance(give);
    }

    TStateErrorOr Exchange(TEvExchange exchange) override {
        return Advance(exchange);
    }

    TSnapshot Snapshot(EPosition pos) const override {
        return TSnapshot();
    }

private:
    TInnerState State_;
    EInnerState Type_;
};

std::unique_ptr<IState> CreateState(IGeneratorPtr generator) {
    return std::make_unique<TState>(std::move(generator));
}

template <EInnerState From, EInnerState To, class TEv>
bool SwitchCheck(TEv ev, const TInnerState& state) {
    return false;
}

template <EInnerState From, EInnerState To, class TEv>
void Switch(TEv ev, TInnerState& state) {
    assert(false);
}

template <EInnerState State>
bool ValidateState(const TInnerState& state) {
    return false;
}

template <class TEv>
bool ValidateEvent(TEv ev, const TInnerState& state) {
    return false;
}

#define STATE_VALIDATOR_IMPL(State) \
    template <> \
    bool ValidateState<EInnerState::State>(const TInnerState& state)

#define SWITCH_CHECK_IMPL(From, To, TEv) \
    template <> \
    bool SwitchCheck<EInnerState::From, EInnerState::To, TEv>(TEv ev, const TInnerState& state)

#define SWITCH_IMPL(From, To, TEv) \
    template <> \
    void Switch<EInnerState::From, EInnerState::To, TEv>(TEv ev, TInnerState& state)

#define EVENT_VALIDATOR_IMPL(TEv) \
    template <> \
    bool ValidateEvent<TEv>(TEv ev, const TInnerState& state)

#define SWITCH_CHECK(From, To) \
    SwitchCheck<EInnerState::From, EInnerState::To>

#define VALIDATE_STATE(State) \
    ValidateState<EInnerState::State>

#define SWITCH(From, To) \
    Switch<EInnerState::From, EInnerState::To>

#define VALIDATE_EVENT \
    ValidateEvent

/*
 * state validators block begin
 */

STATE_VALIDATOR_IMPL(NOT_INITIALIZED) {
    return true;
}

STATE_VALIDATOR_IMPL(NEW_ROUND) {
    return true;
}

STATE_VALIDATOR_IMPL(EXCHANGING) {
    return true;
}

STATE_VALIDATOR_IMPL(NEW_TURN) {
    return true;
}

STATE_VALIDATOR_IMPL(TURN) {
    return true;
}

STATE_VALIDATOR_IMPL(TRICK) {
    return true;
}

STATE_VALIDATOR_IMPL(ROUND_OVER) {
    return true;
}

STATE_VALIDATOR_IMPL(GIVE_DRAGON) {
    return true;
}

STATE_VALIDATOR_IMPL(GAME_OVER) {
    return true;
}

/*
 * state validators block end
 */

/*
 * event validators block begin
 */

EVENT_VALIDATOR_IMPL(TEvGTChoice) {
    return true;
}

EVENT_VALIDATOR_IMPL(TEvExchange) {
    return true;
}

EVENT_VALIDATOR_IMPL(TEvMove) {
    return true;
}

EVENT_VALIDATOR_IMPL(TEvPass) {
    return true;
}

EVENT_VALIDATOR_IMPL(TEvGiveDragon) {
    return true;
}

/*
 * event validators block end
 */

/*
 * switch checks block begin
 */

SWITCH_CHECK_IMPL(NOT_INITIALIZED, NEW_ROUND, TEvNone) {
    assert(VALIDATE_STATE(NOT_INITIALIZED)(state));
    return true;
}

SWITCH_CHECK_IMPL(NEW_ROUND, NEW_ROUND, TEvGTChoice) {
    assert(VALIDATE_STATE(NEW_ROUND)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    return true;
}

SWITCH_CHECK_IMPL(NEW_ROUND, EXCHANGING, TEvGTChoice) {
    assert(VALIDATE_STATE(NEW_ROUND)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(EXCHANGING, EXCHANGING, TEvExchange) {
    assert(VALIDATE_STATE(EXCHANGING)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(EXCHANGING, NEW_TURN, TEvExchange) {
    assert(VALIDATE_STATE(EXCHANGING)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(NEW_TURN, NEW_TURN, TEvMove) {
    assert(VALIDATE_STATE(NEW_TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(NEW_TURN, TURN, TEvMove) {
    assert(VALIDATE_STATE(NEW_TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(NEW_TURN, ROUND_OVER, TEvMove) {
    assert(VALIDATE_STATE(NEW_TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(TURN, TURN, TEvMove) {
    assert(VALIDATE_STATE(TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(TURN, ROUND_OVER, TEvMove) {
    assert(VALIDATE_STATE(TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(TURN, TURN, TEvPass) {
    assert(VALIDATE_STATE(TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(TURN, TRICK, TEvPass) {
    assert(VALIDATE_STATE(TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(TURN, GIVE_DRAGON, TEvPass) {
    assert(VALIDATE_STATE(TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

SWITCH_CHECK_IMPL(ROUND_OVER, NEW_ROUND, TEvNone) {
    assert(VALIDATE_STATE(ROUND_OVER)(state));

    return true;
}

SWITCH_CHECK_IMPL(ROUND_OVER, GAME_OVER, TEvNone) {
    assert(VALIDATE_STATE(ROUND_OVER)(state));
    return true;
}

SWITCH_CHECK_IMPL(TRICK, NEW_ROUND, TEvNone) {
    assert(VALIDATE_STATE(TRICK)(state));
    return true;
}

SWITCH_CHECK_IMPL(GIVE_DRAGON, NEW_ROUND, TEvGiveDragon) {
    assert(VALIDATE_STATE(GIVE_DRAGON)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }
    return true;
}

/*
 * switch checks block end
 */

/*
 * switch block begin
 */

SWITCH_IMPL(NOT_INITIALIZED, NEW_ROUND, TEvNone) {
    assert(SWITCH_CHECK(NOT_INITIALIZED, NEW_ROUND)(ev, state));

    assert(VALIDATE_STATE(NEW_ROUND));
}

SWITCH_IMPL(NEW_ROUND, NEW_ROUND, TEvGTChoice) {
    assert(SWITCH_CHECK(NEW_ROUND, NEW_ROUND)(ev, state));

    assert(VALIDATE_STATE(NEW_ROUND));
}

SWITCH_IMPL(NEW_ROUND, EXCHANGING, TEvGTChoice) {
    assert(SWITCH_CHECK(NEW_ROUND, EXCHANGING)(ev, state));

    assert(VALIDATE_STATE(EXCHANGING));
}

SWITCH_IMPL(EXCHANGING, EXCHANGING, TEvExchange) {
    assert(SWITCH_CHECK(EXCHANGING, EXCHANGING)(ev, state));

    assert(VALIDATE_STATE(EXCHANGING));
}

SWITCH_IMPL(EXCHANGING, NEW_TURN, TEvExchange) {
    assert(SWITCH_CHECK(EXCHANGING, NEW_TURN)(ev, state));

    assert(VALIDATE_STATE(NEW_TURN));
}

SWITCH_IMPL(NEW_TURN, NEW_TURN, TEvMove) {
    assert(SWITCH_CHECK(NEW_TURN, NEW_TURN)(ev, state));

    assert(VALIDATE_STATE(NEW_TURN));
}

SWITCH_IMPL(NEW_TURN, TURN, TEvMove) {
    assert(SWITCH_CHECK(NEW_TURN, TURN)(ev, state));

    assert(VALIDATE_STATE(TURN));
}

SWITCH_IMPL(NEW_TURN, ROUND_OVER, TEvMove) {
    assert(SWITCH_CHECK(NEW_TURN, ROUND_OVER)(ev, state));

    assert(VALIDATE_STATE(ROUND_OVER));
}

SWITCH_IMPL(TURN, TURN, TEvMove) {
    assert(SWITCH_CHECK(TURN, TURN)(ev, state));

    assert(VALIDATE_STATE(TURN));
}

SWITCH_IMPL(TURN, ROUND_OVER, TEvMove) {
    assert(SWITCH_CHECK(TURN, ROUND_OVER)(ev, state));

    assert(VALIDATE_STATE(ROUND_OVER));
}

SWITCH_IMPL(TURN, TURN, TEvPass) {
    assert(SWITCH_CHECK(TURN, TURN)(ev, state));

    assert(VALIDATE_STATE(TURN));
}

SWITCH_IMPL(TURN, TRICK, TEvPass) {
    assert(SWITCH_CHECK(TURN, TRICK)(ev, state));

    assert(VALIDATE_STATE(TRICK));
}

SWITCH_IMPL(TURN, GIVE_DRAGON, TEvPass) {
    assert(SWITCH_CHECK(TURN, GIVE_DRAGON)(ev, state));

    assert(VALIDATE_STATE(GIVE_DRAGON));
}

SWITCH_IMPL(ROUND_OVER, NEW_ROUND, TEvNone) {
    assert(SWITCH_CHECK(ROUND_OVER, NEW_ROUND)(ev, state));

    assert(VALIDATE_STATE(NEW_ROUND));
}

SWITCH_IMPL(ROUND_OVER, GAME_OVER, TEvNone) {
    assert(SWITCH_CHECK(ROUND_OVER, GAME_OVER)(ev, state));

    assert(VALIDATE_STATE(GAME_OVER));
}

SWITCH_IMPL(TRICK, NEW_ROUND, TEvNone) {
    assert(SWITCH_CHECK(TRICK, NEW_ROUND)(ev, state));

    assert(VALIDATE_STATE(NEW_ROUND));
}

SWITCH_IMPL(GIVE_DRAGON, NEW_ROUND, TEvGiveDragon) {
    assert(SWITCH_CHECK(GIVE_DRAGON, NEW_ROUND)(ev, state));

    assert(VALIDATE_STATE(NEW_ROUND));
}

/*
 * switch block end
 */

template <class TEv>
TStateErrorOr TState::Advance(TEv ev) {
    switch (Type_) {
        case EInnerState::NOT_INITIALIZED: {
            SWITCH(NOT_INITIALIZED, NEW_ROUND)(ev, State_);
            Type_ = EInnerState::NEW_ROUND;

            return TStateErrorOr();
        }
        case EInnerState::NEW_ROUND: {
            if (SWITCH_CHECK(NEW_ROUND, NEW_ROUND)(ev, State_)) {
                SWITCH(NEW_ROUND, NEW_ROUND)(ev, State_);
                Type_ = EInnerState::NEW_ROUND;

                return TStateErrorOr();
            }

            if (SWITCH_CHECK(NEW_ROUND, EXCHANGING)(ev, State_)) {
                SWITCH(NEW_ROUND, EXCHANGING)(ev, State_);
                Type_ = EInnerState::EXCHANGING;

                return TStateErrorOr();
            }

            return TStateError{TStateError::UNKNOWN};
        }
        case EInnerState::EXCHANGING: {
            if (SWITCH_CHECK(EXCHANGING, EXCHANGING)(ev, State_)) {
                SWITCH(EXCHANGING, EXCHANGING)(ev, State_);
                Type_ = EInnerState::EXCHANGING;
                return TStateErrorOr();
            }

            if (SWITCH_CHECK(EXCHANGING, NEW_TURN)(ev, State_)) {
                SWITCH(EXCHANGING, NEW_TURN)(ev, State_);
                Type_ = EInnerState::NEW_TURN;

                return TStateErrorOr();
            }

            return TStateError{TStateError::UNKNOWN};
        }
        case EInnerState::NEW_TURN: {
            if (SWITCH_CHECK(NEW_TURN, ROUND_OVER)(ev, State_)) {
                SWITCH(NEW_TURN, ROUND_OVER)(ev, State_);
                Type_ = EInnerState::ROUND_OVER;

                return Advance(TEvNone());
            }

            if (SWITCH_CHECK(NEW_TURN, NEW_TURN)(ev, State_)) {
                SWITCH(NEW_TURN, NEW_TURN)(ev, State_);
                Type_ = EInnerState::NEW_TURN;

                return TStateErrorOr();
            }

            if (SWITCH_CHECK(NEW_TURN, TURN)(ev, State_)) {
                SWITCH(NEW_TURN, TURN)(ev, State_);
                Type_ = EInnerState::TURN;

                return TStateErrorOr();
            }

            return TStateError{TStateError::UNKNOWN};
        }
        case EInnerState::TURN: {
            if (SWITCH_CHECK(TURN, ROUND_OVER)(ev, State_)) {
                SWITCH(TURN, ROUND_OVER)(ev, State_);
                Type_ = EInnerState::ROUND_OVER;

                return Advance(TEvNone());
            }

            if (SWITCH_CHECK(TURN, TURN)(ev, State_)) {
                SWITCH(TURN, TURN)(ev, State_);
                Type_ = EInnerState::TURN;

                return TStateErrorOr();
            }

            if (SWITCH_CHECK(TURN, TRICK)(ev, State_)) {
                SWITCH(TURN, TRICK)(ev, State_);
                Type_ = EInnerState::TRICK;

                return Advance(TEvNone());
            }

            if (SWITCH_CHECK(TURN, GIVE_DRAGON)(ev, State_)) {
                SWITCH(TURN, GIVE_DRAGON)(ev, State_);
                Type_ = EInnerState::GIVE_DRAGON;

                return TStateErrorOr();
            }

            if (SWITCH_CHECK(TURN, NEW_TURN)(ev, State_)) {
                SWITCH(TURN, NEW_TURN)(ev, State_);
                Type_ = EInnerState::NEW_TURN;

                return TStateErrorOr();
            }

            return TStateError{TStateError::UNKNOWN};
        }
        case EInnerState::TRICK: {
            if (SWITCH_CHECK(TRICK, NEW_TURN)(ev, State_)) {
                SWITCH(TRICK, NEW_TURN)(ev, State_);
                Type_ = EInnerState::NEW_TURN;

                return TStateErrorOr();
            }

            return TStateError{TStateError::UNKNOWN};
        }
        case EInnerState::GIVE_DRAGON: {
            if (SWITCH_CHECK(GIVE_DRAGON, NEW_TURN)(ev, State_)) {
                SWITCH(GIVE_DRAGON, NEW_TURN)(ev, State_);
                Type_ = EInnerState::NEW_TURN;
                return TStateErrorOr();
            }

            return TStateError{TStateError::UNKNOWN};
        }
        case EInnerState::ROUND_OVER: {
            if (SWITCH_CHECK(ROUND_OVER, NEW_ROUND)(ev, State_)) {
                SWITCH(ROUND_OVER, NEW_ROUND)(ev, State_);
                Type_ = EInnerState::NEW_ROUND;

                return TStateErrorOr();
            }

            if (SWITCH_CHECK(ROUND_OVER, GAME_OVER)(ev, State_)) {
                SWITCH(ROUND_OVER, GAME_OVER)(ev, State_);
                Type_ = EInnerState::GAME_OVER;

                return TStateErrorOr();
            }

            return TStateError{TStateError::UNKNOWN};
        }

        default: {
            assert(false);
        }
    }
}

} // namespace NTichu::NGameplay::NState
