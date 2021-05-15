#pragma once

#include <server/lib/types/types.h>

#include <string>
#include <array>

#include <limits>

#ifdef ENABLE_INLINE_CARDS
    #define CARDS_CONSTEXPR constexpr
#else
    #define CARDS_CONSTEXPR
#endif

namespace NTichu::NGameplay::NCards {

using TCardId = ui8;
using TCardsMask = ui64;
using TScore = i16;

enum class ESuit: ui8 {
    JACK = 0,
    SWORD,
    PAGODA,
    STAR,

    INVALID = std::numeric_limits<ui8>::max()
};

enum class EValue: ui8 {
    DOG = 0,
    MAH_JONG = 1,
    PHOENIX = 2,
    TWO = 4,
    THREE = 8,
    FOUR = 12,
    FIVE = 16,
    SIX = 20,
    SEVEN = 24,
    EIGHT = 28,
    NINE = 32,
    TEN = 36,
    JACK = 40,
    QUEEN = 44,
    KING = 48,
    ACE = 52,
    DRAGON = 56,

    INVALID = std::numeric_limits<ui8>::max(),
};

static constexpr ui8 NUM_SUITS = 4;
static constexpr ui8 NUM_VALUES = 17;
static constexpr ui8 NUM_CARDS = 56;

enum class ECombination: ui8 {
    SINGLE = 1,
    PAIR = 2,
    TRIPLE = 3,
    QUAD = 4,
    FULL_HOUSE = 5,
    ADJ_PAIRS = 6,
    STRAIGHT = 7,
    STRAIGHT_FLUSH = 8,
    DOG = 9,
    EMPTY = 10,

    INVALID = std::numeric_limits<ui8>::max(),
};

static constexpr std::array<ESuit, NUM_SUITS> AllSuits {
    ESuit::JACK,
    ESuit::SWORD,
    ESuit::PAGODA,
    ESuit::STAR,
};

static constexpr std::array<EValue, NUM_VALUES> AllValues {
    EValue::DOG,
    EValue::MAH_JONG,
    EValue::PHOENIX,
    EValue::TWO,
    EValue::THREE,
    EValue::FOUR,
    EValue::FIVE,
    EValue::SIX,
    EValue::SEVEN,
    EValue::EIGHT,
    EValue::NINE,
    EValue::TEN,
    EValue::JACK,
    EValue::QUEEN,
    EValue::KING,
    EValue::ACE,
    EValue::DRAGON
};

class TCard;
class TCards;
class TCombination;

std::string AsString(ESuit suit);
std::string AsString(EValue value);
std::string AsString(ECombination combTy);
std::string AsString(TCard card);
std::string AsString(TCards cards);
std::string AsString(TCombination comb);

class TCard {
public:
    CARDS_CONSTEXPR TCard() noexcept;
    CARDS_CONSTEXPR explicit TCard(TCardId) noexcept;
    CARDS_CONSTEXPR explicit TCard(EValue) noexcept;
    CARDS_CONSTEXPR explicit TCard(EValue value, ESuit suit) noexcept;

    [[nodiscard]] CARDS_CONSTEXPR bool Defined() const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR EValue Value() const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR ESuit Suit() const noexcept;

    [[nodiscard]] CARDS_CONSTEXPR TCardId Id() const noexcept;

    [[nodiscard]] CARDS_CONSTEXPR bool Basic() const noexcept;

    [[nodiscard]] CARDS_CONSTEXPR TCards AsCards() const noexcept;

    static CARDS_CONSTEXPR TCard MahJong() noexcept;
    static CARDS_CONSTEXPR TCard Dog() noexcept;
    static CARDS_CONSTEXPR TCard Phoenix() noexcept;
    static CARDS_CONSTEXPR TCard Dragon() noexcept;

public:
    CARDS_CONSTEXPR bool operator==(TCard other) const noexcept;
    CARDS_CONSTEXPR bool operator!=(TCard other) const noexcept;
    CARDS_CONSTEXPR operator bool() noexcept;

private:
    TCardId Id_;
};

class TCards {
public:
    TCards() noexcept = default;
    CARDS_CONSTEXPR TCards(TCard card) noexcept;
    CARDS_CONSTEXPR explicit TCards(TCardsMask mask) noexcept;
    CARDS_CONSTEXPR TCards(TCombination comb) noexcept;

    CARDS_CONSTEXPR void Join(TCards cards) noexcept;
    template <class... TArgs>
    constexpr void Join(TCards cards, TArgs... args) noexcept;

    template <class... TArgs>
    static constexpr TCards Un(TCards cards, TArgs... args) noexcept;

    CARDS_CONSTEXPR void Rm(TCards cards) noexcept;
    template <class... TArgs>
    constexpr void Rm(TCards cards, TArgs... args) noexcept;

    CARDS_CONSTEXPR void Drop() noexcept;

    [[nodiscard]] CARDS_CONSTEXPR bool Empty() const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR ui8 Num() const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR TCardsMask Mask() const noexcept;

    [[nodiscard]] CARDS_CONSTEXPR bool Has(TCards cards) const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR bool HasAny(TCards cards) const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR TCard Last() const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR TScore Score() const noexcept;

    static CARDS_CONSTEXPR TCards Deck() noexcept;

public:
    CARDS_CONSTEXPR bool operator==(TCards other) const noexcept;
    CARDS_CONSTEXPR bool operator!=(TCards other) const noexcept;

public:
    class TIterator {
    public:
        friend class TCards;

        CARDS_CONSTEXPR TCard operator*() const noexcept;

        CARDS_CONSTEXPR bool operator==(TIterator other) const noexcept;

        CARDS_CONSTEXPR bool operator!=(TIterator other) const noexcept;

        CARDS_CONSTEXPR TIterator& operator++() noexcept;

    private:
        explicit CARDS_CONSTEXPR TIterator(TCards cards) noexcept;

    private:
        TCardsMask Mask_{0};
    };

    [[nodiscard]] CARDS_CONSTEXPR TIterator begin() const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR TIterator end() const noexcept;

private:
    TCardsMask Mask_{0};
};

class TCombination {
public:
    TCombination() noexcept = default;
    CARDS_CONSTEXPR TCombination(TCards cards, EValue phRep = EValue::INVALID) noexcept;

    [[nodiscard]] CARDS_CONSTEXPR ECombination Ty() const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR EValue PhRep() const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR TCards AsCards() const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR bool Defined() const noexcept;
    [[nodiscard]] CARDS_CONSTEXPR bool IsBomb() const noexcept;

    [[nodiscard]] CARDS_CONSTEXPR TCombination Combine(TCombination comb) const noexcept;

    CARDS_CONSTEXPR void Drop() noexcept;
    static CARDS_CONSTEXPR TCombination Invalid() noexcept;

public:
    CARDS_CONSTEXPR bool operator==(TCombination other) const noexcept;
    CARDS_CONSTEXPR bool operator!=(TCombination other) const noexcept;
    CARDS_CONSTEXPR operator bool() const noexcept;

private:
    [[nodiscard]] CARDS_CONSTEXPR TCardsMask CombMask() const noexcept;

private:
    EValue PhRep_{EValue::INVALID};
    ECombination Type_{ECombination::EMPTY};
    TCardsMask Mask_{0};
};


template <class... TArgs>
constexpr void TCards::Join(TCards cards, TArgs... args) noexcept {
    Join(cards);
    Join(args...);
}

template <class... TArgs>
constexpr TCards TCards::Un(TCards cards, TArgs... args) noexcept {
    cards.Join(args...);
    return cards;
}

template <class... TArgs>
constexpr void TCards::Rm(TCards cards, TArgs... args) noexcept {
    Rm(cards);
    Rm(args...);
}

} // namespace NTichu::NGameplay::NCards

#undef CARDS_CONSTEXPR

template <>
struct std::hash<NTichu::NGameplay::NCards::TCard> {
    bool operator()(NTichu::NGameplay::NCards::TCard card) const noexcept {
        ui64 hash = std::hash<ui64>()((ui64) card.Value());
        if (card.Basic()) {
            hash ^= std::hash<ui64>()((ui64) card.Suit());
        }

        return hash;
    }
};
