#pragma once

#include "cards.h"
#include <functional>

namespace NTichu::NGameplay::NCards {

constexpr TCard::TCard(TCardId cardId) noexcept
        : Id_(cardId) {
}

constexpr TCard::TCard(EValue value) noexcept
    : Id_(TBase::Card(value))
{
}

constexpr TCard::TCard(EValue value, ESuit suit) noexcept
    : Id_(TBase::Card(value, suit))
{
}

constexpr bool TCard::Defined() const noexcept {
    return TBase::ValidateCard(Id_);
}

constexpr EValue TCard::Value() const noexcept {
    return TBase::Value(Id_);
}

constexpr ESuit TCard::Suit() const noexcept {
    return TBase::Suit(Id_);
}

constexpr TCardId TCard::Id() const noexcept {
    return Id_;
}

constexpr bool TCard::Basic() const noexcept {
    return Mask(Id_) & MK_ALL_BASE;
}

constexpr TCards TCard::AsCards() const noexcept {
    return TCards(Mask(Id_));
}

constexpr TCard TCard::MahJong() noexcept {
    return TCard(Card(EValue::MAH_JONG));
}

constexpr TCard TCard::Dog() noexcept {
    return TCard(Card(EValue::DOG));
}

constexpr TCard TCard::Phoenix() noexcept {
    return TCard(Card(EValue::PHOENIX));
}

constexpr TCard TCard::Dragon() noexcept {
    return TCard(Card(EValue::DRAGON));
}

constexpr bool TCard::operator==(TCard other) const noexcept {
    return other.Id_ == Id_;
}

constexpr bool TCard::operator!=(TCard other) const noexcept {
    return other.Id_ != Id_;
}

constexpr TCard::operator bool() noexcept {
    return TCard::Defined();
}

constexpr TCards::TCards(TCard card) noexcept
    : Mask_(TBase::Mask(card.Id()))
{
}

constexpr TCards::TCards(TCardsMask mask) noexcept
    : Mask_(mask)
{
}

constexpr TCards::TCards(NTichu::NGameplay::NCards::TCombination comb) noexcept
    : Mask_(comb.AsCards().Mask())
{
}

constexpr void TCards::Join(TCards cards) noexcept {
    Mask_ |= cards.Mask_;
}

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

constexpr void TCards::Rm(TCards cards) noexcept {
    Mask_ &= ~cards.Mask_;
}

template <class... TArgs>
constexpr void TCards::Rm(TCards cards, TArgs... args) noexcept {
    Rm(cards);
    Rm(args...);
}

constexpr void TCards::Drop() noexcept {
    Mask_ = 0;
}

constexpr bool TCards::Empty() const noexcept {
    return !Mask_;
}

constexpr ui8 TCards::Num() const noexcept { return TBase::NumCards(Mask_); }
constexpr TCardsMask TCards::Mask() const noexcept { return Mask_; }

constexpr bool TCards::Has(TCards cards) const noexcept {
    return (Mask_ | cards.Mask_) == Mask_;
}

constexpr bool TCards::HasAny(TCards cards) const noexcept {
    return (Mask_ & cards.Mask_);
}

constexpr TCard TCards::Last() const noexcept {
    return TCard(TBase::LastCard(Mask_));
}

constexpr TScore TCards::Score() const noexcept {
    return TBase::Score(Mask_);
}

constexpr TCards TCards::Deck() noexcept {
    return TCards(MK_ALL);
}

constexpr bool TCards::operator==(TCards other) const noexcept {
    return Mask_ == other.Mask_;
}

constexpr bool TCards::operator!=(TCards other) const noexcept {
    return Mask_ != other.Mask_;
}

class TCards::TIterator {
public:
    friend class TCards;

    constexpr TCard operator*() const noexcept {
        return TCard(TBase::LastCard(Mask_));
    }

    constexpr bool operator==(TIterator other) const noexcept {
        return Mask_ == other.Mask_;
    }

    constexpr bool operator!=(TIterator other) const noexcept {
        return Mask_ != other.Mask_;
    }

    constexpr TIterator& operator++() noexcept {
        Mask_ ^= TBase::Mask(TBase::LastCard(Mask_));
        return *this;
    }

private:
    explicit constexpr TIterator(TCards cards) noexcept: Mask_(cards.Mask()) {}

private:
    TCardsMask Mask_{0};
};

constexpr TCards::TIterator TCards::begin() const noexcept {
    return TIterator(*this);
}

constexpr TCards::TIterator TCards::end() const noexcept {
    return TIterator(TCards(0));
}

constexpr TCombination::TCombination(TCards cards, EValue phRep) noexcept
    : PhRep_(phRep), Type_(TBase::CombTy(cards.Mask(), phRep)), Mask_(cards.Mask())
{
}

constexpr ECombination TCombination::Ty() const noexcept {
    return Type_;
}

constexpr EValue TCombination::PhRep() const noexcept {
    return PhRep_;
}

constexpr TCards TCombination::AsCards() const noexcept {
    return TCards(Mask_);
}

constexpr bool TCombination::Defined() const noexcept {
    return Type_ != ECombination::INVALID;
}

constexpr bool TCombination::IsBomb() const noexcept {
    return TBase::IsBomb(Type_);
}

constexpr void TCombination::Drop() noexcept {
    Mask_ = 0;
    PhRep_ = EValue::INVALID;
    Type_ = ECombination::EMPTY;
}

constexpr bool TCombination::operator==(TCombination other) const noexcept {
    return Mask_ == other.Mask_ && PhRep_ == other.PhRep_;
}

constexpr bool TCombination::operator!=(TCombination other) const noexcept {
    return !(*this == other);
}

constexpr TCombination::operator bool() const noexcept {
    return Defined();
}

constexpr TCombination TCombination::Invalid() noexcept {
    TCombination comb;
    comb.Type_ = ECombination::INVALID;
    return comb;
}

constexpr TCardsMask TCombination::CombMask() const noexcept {
    if (!ValidateValue(PhRep_)) {
        return Mask_;
    }

    return TBase::RepPh(Mask_, PhRep_);
}

constexpr TCombination TCombination::Combine(TCombination comb) const noexcept {
    if (!comb.Defined() || !Defined()) {
        return Invalid();
    }

    switch (Type_) {
        case ECombination::EMPTY: {
            return comb;
        }
        case ECombination::DOG: {
            return Invalid();
        }
        case ECombination::SINGLE: {
            if (comb.IsBomb()) {
                return comb;
            } else if (comb.Ty() != ECombination::SINGLE) {
                return Invalid();
            }

            if (TBase::CombinableSingle(CombMask(), comb.CombMask())) {
                if (comb.AsCards().Has(TCard::Phoenix())) {
                    return TCombination(comb.AsCards(), comb.AsCards().Last().Value());
                }

                return comb;
            }

            return Invalid();
        }
        case ECombination::PAIR:
        case ECombination::TRIPLE: {
            if (comb.IsBomb()) {
                return comb;
            }

            if (CombinableSingleValue(CombMask(), comb.CombMask())) {
                return comb;
            }

            return Invalid();
        }

        case ECombination::FULL_HOUSE: {
            if (comb.IsBomb()) {
                return comb;
            }

            if (CombinableFullHouse(CombMask(), comb.CombMask())) {
                return comb;
            }

            return Invalid();
        }

        case ECombination::STRAIGHT:
        case ECombination::ADJ_PAIRS: {
            if (comb.IsBomb()) {
                return comb;
            }

            if (TBase::CombinableStraightType(CombMask(), comb.CombMask())) {
                return comb;
            }

            return Invalid();
        }
        case ECombination::QUAD: {
            if (comb.Ty() == ECombination::STRAIGHT_FLUSH) {
                return comb;
            }

            if (comb.Ty() != ECombination::QUAD) {
                return Invalid();
            }

            if (TBase::CombinableSingleValue(CombMask(), comb.CombMask())) {
                return comb;
            }

            return Invalid();
        }
        case ECombination::STRAIGHT_FLUSH: {
            if (comb.Ty() != ECombination::STRAIGHT_FLUSH) {
                return Invalid();
            }

            if (TBase::CombinableStraightFlush(CombMask(), comb.CombMask())) {
                return comb;
            }

            return Invalid();
        }

        default: {
            return Invalid();
        }
    }
}

} // NTichu::NGameplay::NCards

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
