#pragma once

#include "base.h"

#include <string>
#include <array>

namespace NTichu::NGameplay::NCards {

static constexpr std::array<ESuit, TBase::NUM_SUITS> ArraySuits {
    ESuit::JACK,
    ESuit::SWORD,
    ESuit::PAGODA,
    ESuit::STAR,
};

static constexpr std::array<EValue, TBase::NUM_VALUES> ArrayValues {
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

class TCard: public TBase {
public:
    TCard() noexcept = default;
    constexpr explicit TCard(TCardId) noexcept;
    constexpr explicit TCard(EValue) noexcept;
    constexpr explicit TCard(EValue value, ESuit suit) noexcept;

    [[nodiscard]] constexpr bool Defined() const noexcept;
    [[nodiscard]] constexpr EValue Value() const noexcept;
    [[nodiscard]] constexpr ESuit Suit() const noexcept;

    [[nodiscard]] constexpr TCardId Id() const noexcept;

    [[nodiscard]] constexpr bool Basic() const noexcept;

    [[nodiscard]] constexpr TCards AsCards() const noexcept;

    static constexpr TCard MahJong() noexcept;
    static constexpr TCard Dog() noexcept;
    static constexpr TCard Phoenix() noexcept;
    static constexpr TCard Dragon() noexcept;

public:
    constexpr bool operator==(TCard other) const noexcept;
    constexpr bool operator!=(TCard other) const noexcept;
    constexpr operator bool() noexcept;

private:
    TCardId Id_{INVALID_CARD_ID};
};

class TCards: public TBase {
public:
    TCards() noexcept = default;
    constexpr TCards(TCard card) noexcept;
    constexpr explicit TCards(TCardsMask mask) noexcept;
    constexpr TCards(TCombination comb) noexcept;

    constexpr void Join(TCards cards) noexcept;
    template <class... TArgs>
    constexpr void Join(TCards cards, TArgs... args) noexcept;

    template <class... TArgs>
    static constexpr TCards Un(TCards cards, TArgs... args) noexcept;

    constexpr void Rm(TCards cards) noexcept;
    template <class... TArgs>
    constexpr void Rm(TCards cards, TArgs... args) noexcept;

    constexpr void Drop() noexcept;

    [[nodiscard]] constexpr bool Empty() const noexcept;
    [[nodiscard]] constexpr ui8 Num() const noexcept;
    [[nodiscard]] constexpr TCardsMask Mask() const noexcept;

    [[nodiscard]] constexpr bool Has(TCards cards) const noexcept;
    [[nodiscard]] constexpr bool HasAny(TCards cards) const noexcept;
    [[nodiscard]] constexpr TCard Last() const noexcept;
    [[nodiscard]] constexpr TScore Score() const noexcept;

    static constexpr TCards Deck() noexcept;

public:
    constexpr bool operator==(TCards other) const noexcept;
    constexpr bool operator!=(TCards other) const noexcept;

public:
    class TIterator;

    [[nodiscard]] constexpr TIterator begin() const noexcept;
    [[nodiscard]] constexpr TIterator end() const noexcept;

private:
    TCardsMask Mask_{0};
};

class TCombination: public TBase {
public:
    TCombination() noexcept = default;
    constexpr TCombination(TCards cards, EValue phRep = EValue::INVALID) noexcept;

    [[nodiscard]] constexpr ECombination Ty() const noexcept;
    [[nodiscard]] constexpr EValue PhRep() const noexcept;
    [[nodiscard]] constexpr TCards AsCards() const noexcept;
    [[nodiscard]] constexpr bool Defined() const noexcept;
    [[nodiscard]] constexpr bool IsBomb() const noexcept;

    [[nodiscard]] constexpr TCombination Combine(TCombination comb) const noexcept;

    constexpr void Drop() noexcept;
    static constexpr TCombination Invalid() noexcept;

public:
    constexpr bool operator==(TCombination other) const noexcept;
    constexpr bool operator!=(TCombination other) const noexcept;
    constexpr operator bool() const noexcept;

private:
    [[nodiscard]] constexpr TCardsMask CombMask() const noexcept;

private:
    EValue PhRep_{EValue::INVALID};
    ECombination Type_{ECombination::EMPTY};
    TCardsMask Mask_{0};
};

} // namespace NTichu::NGameplay::NCards
