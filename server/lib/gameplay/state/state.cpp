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
    TInnerState(IGeneratorPtr generator)
        : Generator(std::move(generator))
    {
        ResetState();
    }

    void ResetRound() {
        Generator->Reset();

        for (auto& cards: Cards) {
            cards = TCards();
        }

        for (auto& trick: Trick) {
            trick = TCards();
        }

        for (auto& call: Call) {
            call = ECall::UNDEFINED;
        }

        for (auto& ex: Exchange) {
            for (auto& card: ex) {
                card = TCard();
            }
        }

        LastComb = TCombination();
        ActiveCards = TCards();

        ActivePlayer = EPosition::INVALID;
        TrickPlayer = EPosition::INVALID;
        DonePlayer = EPosition::INVALID;
    }

    void ResetState() {
        for (auto& score: Score) {
            score = 0;
        }

        ResetRound();
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
    TState(IGeneratorPtr generator)
        : State_(std::move(generator))
        , Type_(EInnerState::NOT_INITIALIZED)
    {
    }

public:
    // TODO: use max score from config
    static constexpr ui32 MAX_SCORE = 1000;

    static constexpr ui32 CARDS_INIT = 8;
    static constexpr ui32 CARDS_TOTAL = 14;

    static constexpr TScore GRAND_TICHU_SCORE = 200;
    static constexpr TScore SMALL_TICHU_SCORE = 100;
    static constexpr TScore TEAM_DONE_SCORE = 200;

private:
    template <class TEv>
    TStateErrorOr Advance(TEv ev);

    void Start() override {
        assert(Advance(TEvNone()).Succeed());
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

    EState State() const override {
        return (EState) Type_;
    }

public:
    TInnerState State_;
    EInnerState Type_;
};

std::unique_ptr<IState> CreateState(IGeneratorPtr generator) {
    return std::make_unique<TState>(std::move(generator));
}

EPosition NextActive(const TInnerState& state, EPosition from) {
    for (size_t i = 0; i < (ui32) EPosition::NUM; ++i) {
        if (from == state.TrickPlayer || !state.Cards[(ui32) from].Empty()) {
            return from;
        }

        from = NextPosition(from);
    }

    return EPosition::INVALID;
}

void DoneAction(TInnerState& state, EPosition pos) {
    if (!state.Cards[(ui32) pos].Empty()) {
        return;
    }

    if (!ValidatePosition(state.DonePlayer)) {
        state.DonePlayer = pos;
    }
}

void MoveAction(TInnerState& state, TEvMove ev) {
    state.Cards[(ui32) ev.Who].Rm(ev.Comb.AsCards());

    state.ActiveCards.Join(ev.Comb.AsCards());
    state.LastComb = state.LastComb.Combine(ev.Comb);

    state.TrickPlayer = ev.Who;
    state.ActivePlayer = NextActive(state, NextPosition(ev.Who));

    DoneAction(state, ev.Who);
}

void TrickAction(TInnerState& state, EPosition takes) {
    state.Trick[(ui32) takes].Join(state.ActiveCards);

    state.ActiveCards.Drop();
    state.LastComb.Drop();
    state.TrickPlayer = EPosition::INVALID;

    state.ActivePlayer = NextActive(state, state.ActivePlayer);
}

bool TrickCheck(const TInnerState& state) {
    return NextActive(state, NextPosition(state.ActivePlayer)) == state.TrickPlayer;
}

bool LastDragon(const TInnerState& state) {
    return state.LastComb.AsCards() == TCard::Dragon();
}

EPosition GiveDragonTo(const TInnerState& state) {
    auto op = OppositeTeam(Team(state.TrickPlayer));

    EPosition last = EPosition::INVALID;

    for (auto pos: AllPositions) {
        if (Team(pos) == op) {
            if (ValidatePosition(last)) {
                return EPosition::INVALID;
            }

            last = pos;
        }
    }

    assert(ValidatePosition(last));
    return last;
}

TTM<TScore> UpdatedScore(const TInnerState& state) {
    TTM<TScore> score = state.Score;

    for (size_t i = 0; i < (ui32) EPosition::NUM; ++i) {
        if (state.Call[i] == ECall::GRAND) {
            if (i == (ui32) state.DonePlayer) {
                score[(ui32) Team(state.DonePlayer)] += TState::GRAND_TICHU_SCORE;
            } else {
                score[(ui32) Team(state.DonePlayer)] -= TState::GRAND_TICHU_SCORE;
            }
        } else if (state.Call[i] == ECall::SMALL) {
            if (i == (ui32) state.DonePlayer) {
                score[(ui32) Team(state.DonePlayer)] += TState::SMALL_TICHU_SCORE;
            } else {
                score[(ui32) Team(state.DonePlayer)] -= TState::SMALL_TICHU_SCORE;
            }
        }
    }

    TTM<ui32> active{0};
    EPosition last = EPosition::INVALID;
    for (auto pos: AllPositions) {
        if (!state.Cards[(ui32) pos].Empty()) {
            ++active[(ui32) Team(pos)];
            last = pos;
        }
    }

    if (active[(ui32) ETeam::TEAM_0] == 2u) {
        score[(ui32) ETeam::TEAM_1] += TState::TEAM_DONE_SCORE;
        return score;
    } else if (active[(ui32) ETeam::TEAM_1] == 2u) {
        score[(ui32) ETeam::TEAM_0] += TState::TEAM_DONE_SCORE;
        return score;
    }

    assert(ValidatePosition(last));

    score[(ui32) OppositeTeam(Team(last))] += state.ActiveCards.Score();
    score[(ui32) OppositeTeam(Team(last))] += state.Cards[(ui32) last].Score();
    score[(ui32) Team(state.DonePlayer)] += state.Trick[(ui32) last].Score();

    for (auto pos: AllPositions) {
        if (pos != last) {
            score[(ui32) Team(pos)] += state.Trick[(ui32) pos].Score();
        }
    }

    return score;
}

void UpdateScore(TInnerState& state) {
    state.Score = UpdatedScore(state);
}

bool GameOver(const TInnerState& state) {
    auto score = UpdatedScore(state);

    auto a = score[(ui32) ETeam::TEAM_0];
    auto b = score[(ui32) ETeam::TEAM_1];

    return (a >= TState::MAX_SCORE && a > b) || (b >= TState::MAX_SCORE && b > a);
}

bool RoundOver(const TInnerState& state, TEvMove ev) {
    TTM<ui32> done{0};
    for (auto pos: AllPositions) {
        if (pos == ev.Who) {
            done[(ui32) Team(pos)] += (state.Cards[(ui32) pos] == ev.Comb.AsCards());
        } else {
            done[(ui32) Team(pos)] += (state.Cards[(ui32) pos].Empty());
        }
    }

    auto a = done[(ui32) ETeam::TEAM_0];
    auto b = done[(ui32) ETeam::TEAM_1];

    return a > 1 || b > 1 || a + b > 2;
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

    ui32 calls = 0;
    for (auto pos: AllPositions) {
        if (state.Call[(ui32) pos] != ECall::UNDEFINED) {
            ++calls;
        }
    }

    return calls + 1u < (ui32) EPosition::NUM;
}

SWITCH_CHECK_IMPL(NEW_ROUND, EXCHANGING, TEvGTChoice) {
    assert(VALIDATE_STATE(NEW_ROUND)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    ui32 calls = 0;
    for (auto pos: AllPositions) {
        if (state.Call[(ui32) pos] != ECall::UNDEFINED) {
            ++calls;
        }
    }

    return calls + 1u == (ui32) EPosition::NUM;
}

SWITCH_CHECK_IMPL(EXCHANGING, EXCHANGING, TEvExchange) {
    assert(VALIDATE_STATE(EXCHANGING)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    ui32 exchanges = 0;
    for (auto to: AllPositions) {
        for (auto from: AllPositions) {
            if (state.Exchange[(ui32) from][(ui32) to].Defined()) {
                ++exchanges;
                break;
            }
        }
    }

    return exchanges + 1u < (ui32) EPosition::NUM;
}

SWITCH_CHECK_IMPL(EXCHANGING, NEW_TURN, TEvExchange) {
    assert(VALIDATE_STATE(EXCHANGING)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    ui32 exchanges = 0;
    for (auto to: AllPositions) {
        for (auto from: AllPositions) {
            if (state.Exchange[(ui32) from][(ui32) to].Defined()) {
                ++exchanges;
                break;
            }
        }
    }

    return exchanges + 1u == (ui32) EPosition::NUM;
}

SWITCH_CHECK_IMPL(NEW_TURN, NEW_TURN, TEvMove) {
    assert(VALIDATE_STATE(NEW_TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    return !RoundOver(state, ev) && ev.Comb.Ty() == ECombination::DOG;
}

SWITCH_CHECK_IMPL(NEW_TURN, TURN, TEvMove) {
    assert(VALIDATE_STATE(NEW_TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    return !RoundOver(state, ev) && ev.Comb.Ty() != ECombination::DOG;
}

SWITCH_CHECK_IMPL(NEW_TURN, ROUND_OVER, TEvMove) {
    assert(VALIDATE_STATE(NEW_TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    return RoundOver(state, ev);
}

SWITCH_CHECK_IMPL(TURN, TURN, TEvMove) {
    assert(VALIDATE_STATE(TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    return !RoundOver(state, ev);
}

SWITCH_CHECK_IMPL(TURN, ROUND_OVER, TEvMove) {
    assert(VALIDATE_STATE(TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    return RoundOver(state, ev);
}

SWITCH_CHECK_IMPL(TURN, TURN, TEvPass) {
    assert(VALIDATE_STATE(TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    return !TrickCheck(state);
}

SWITCH_CHECK_IMPL(TURN, TRICK, TEvPass) {
    assert(VALIDATE_STATE(TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    return TrickCheck(state) && (!LastDragon(state) || ValidatePosition(GiveDragonTo(state)));
}

SWITCH_CHECK_IMPL(TURN, GIVE_DRAGON, TEvPass) {
    assert(VALIDATE_STATE(TURN)(state));
    if (!VALIDATE_EVENT(ev, state)) {
        return false;
    }

    return TrickCheck(state) && LastDragon(state) && !ValidatePosition(GiveDragonTo(state));
}

SWITCH_CHECK_IMPL(ROUND_OVER, NEW_ROUND, TEvNone) {
    assert(VALIDATE_STATE(ROUND_OVER)(state));

    return !GameOver(state);
}

SWITCH_CHECK_IMPL(ROUND_OVER, GAME_OVER, TEvNone) {
    assert(VALIDATE_STATE(ROUND_OVER)(state));
    return GameOver(state);
}

SWITCH_CHECK_IMPL(TRICK, NEW_TURN, TEvNone) {
    assert(VALIDATE_STATE(TRICK)(state));
    return true;
}

SWITCH_CHECK_IMPL(GIVE_DRAGON, NEW_TURN, TEvGiveDragon) {
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

    for (auto& cards: state.Cards) {
        cards = state.Generator->Generate(TState::CARDS_INIT);
    }

    assert(VALIDATE_STATE(NEW_ROUND)(state));
}

SWITCH_IMPL(NEW_ROUND, NEW_ROUND, TEvGTChoice) {
    assert(SWITCH_CHECK(NEW_ROUND, NEW_ROUND)(ev, state));

    if (ev.Choice) {
        state.Call[(ui32) ev.Who] = ECall::GRAND;
    } else {
        state.Call[(ui32) ev.Who] = ECall::NONE;
    }

    assert(VALIDATE_STATE(NEW_ROUND)(state));
}

SWITCH_IMPL(NEW_ROUND, EXCHANGING, TEvGTChoice) {
    assert(SWITCH_CHECK(NEW_ROUND, EXCHANGING)(ev, state));

    if (ev.Choice) {
        state.Call[(ui32) ev.Who] = ECall::GRAND;
    } else {
        state.Call[(ui32) ev.Who] = ECall::NONE;
    }

    for (auto& cards: state.Cards) {
        cards.Join(state.Generator->Generate(TState::CARDS_TOTAL - TState::CARDS_INIT));
    }

    assert(VALIDATE_STATE(EXCHANGING)(state));
}

SWITCH_IMPL(EXCHANGING, EXCHANGING, TEvExchange) {
    assert(SWITCH_CHECK(EXCHANGING, EXCHANGING)(ev, state));

    for (size_t i = 0; i < (ui32) EPosition::NUM; ++i) {
        state.Exchange[i][(ui32) ev.Who] = ev.Exchange[i];
    }

    assert(VALIDATE_STATE(EXCHANGING));
}

SWITCH_IMPL(EXCHANGING, NEW_TURN, TEvExchange) {
    assert(SWITCH_CHECK(EXCHANGING, NEW_TURN)(ev, state));

    for (size_t i = 0; i < (ui32) EPosition::NUM; ++i) {
        state.Exchange[i][(ui32) ev.Who] = ev.Exchange[i];
    }

    for (size_t to = 0; to < (ui32) EPosition::NUM; ++to) {
        for (size_t from = 0; from < (ui32) EPosition::NUM; ++from) {
            if (from != to) {
                state.Cards[from].Rm(state.Exchange[to][from]);
                state.Cards[to].Join(state.Exchange[to][from]);
            }
        }
    }

    for (size_t i = 0; i < (ui32) EPosition::NUM; ++i) {
        if (state.Cards[i].Has(TCard::MahJong())) {
            state.ActivePlayer = (EPosition) i;
            break;
        }
    }

    assert(VALIDATE_STATE(NEW_TURN)(state));
}

SWITCH_IMPL(NEW_TURN, NEW_TURN, TEvMove) {
    assert(SWITCH_CHECK(NEW_TURN, NEW_TURN)(ev, state));

    state.Cards[(ui32) ev.Who].Rm(ev.Comb.AsCards());
    state.Trick[(ui32) ev.Who].Join(ev.Comb.AsCards());

    DoneAction(state, ev.Who);

    state.ActivePlayer = NextActive(state, Teammate(ev.Who));

    assert(VALIDATE_STATE(NEW_TURN)(state));
}

SWITCH_IMPL(NEW_TURN, TURN, TEvMove) {
    assert(SWITCH_CHECK(NEW_TURN, TURN)(ev, state));

    MoveAction(state, ev);

    assert(VALIDATE_STATE(TURN)(state));
}

SWITCH_IMPL(NEW_TURN, ROUND_OVER, TEvMove) {
    assert(SWITCH_CHECK(NEW_TURN, ROUND_OVER)(ev, state));

    MoveAction(state, ev);

    assert(VALIDATE_STATE(ROUND_OVER)(state));
}

SWITCH_IMPL(TURN, TURN, TEvMove) {
    assert(SWITCH_CHECK(TURN, TURN)(ev, state));

    MoveAction(state, ev);

    assert(VALIDATE_STATE(TURN)(state));
}

SWITCH_IMPL(TURN, ROUND_OVER, TEvMove) {
    assert(SWITCH_CHECK(TURN, ROUND_OVER)(ev, state));

    MoveAction(state, ev);

    assert(VALIDATE_STATE(ROUND_OVER)(state));
}

SWITCH_IMPL(TURN, TURN, TEvPass) {
    assert(SWITCH_CHECK(TURN, TURN)(ev, state));

    state.ActivePlayer = NextActive(state, NextPosition(ev.Who));

    assert(VALIDATE_STATE(TURN)(state));
}

SWITCH_IMPL(TURN, TRICK, TEvPass) {
    assert(SWITCH_CHECK(TURN, TRICK)(ev, state));

    state.ActivePlayer = NextActive(state, NextPosition(ev.Who));

    assert(VALIDATE_STATE(TRICK)(state));
}

SWITCH_IMPL(TURN, GIVE_DRAGON, TEvPass) {
    assert(SWITCH_CHECK(TURN, GIVE_DRAGON)(ev, state));

    state.ActivePlayer = NextActive(state, NextPosition(ev.Who));

    assert(VALIDATE_STATE(GIVE_DRAGON)(state));
}

SWITCH_IMPL(ROUND_OVER, NEW_ROUND, TEvNone) {
    assert(SWITCH_CHECK(ROUND_OVER, NEW_ROUND)(ev, state));

    UpdateScore(state);
    state.ResetRound();

    for (auto pos: AllPositions) {
        state.Cards[(ui32) pos] = state.Generator->Generate(TState::CARDS_INIT);
    }

    assert(VALIDATE_STATE(NEW_ROUND)(state));
}

SWITCH_IMPL(ROUND_OVER, GAME_OVER, TEvNone) {
    assert(SWITCH_CHECK(ROUND_OVER, GAME_OVER)(ev, state));

    UpdateScore(state);
    state.ResetRound();

    assert(VALIDATE_STATE(GAME_OVER)(state));
}

SWITCH_IMPL(TRICK, NEW_TURN, TEvNone) {
    assert(SWITCH_CHECK(TRICK, NEW_TURN)(ev, state));

    TrickAction(state, LastDragon(state) ? GiveDragonTo(state): state.ActivePlayer);

    assert(VALIDATE_STATE(NEW_TURN)(state));
}

SWITCH_IMPL(GIVE_DRAGON, NEW_TURN, TEvGiveDragon) {
    assert(SWITCH_CHECK(GIVE_DRAGON, NEW_TURN)(ev, state));

    TrickAction(state, ev.To);

    assert(VALIDATE_STATE(NEW_TURN)(state));
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

    return TStateError{TStateError::UNKNOWN};
}

} // namespace NTichu::NGameplay::NState
