#include "cards.h"

#ifdef ENABLE_INLINE_CARDS
    #define CARDS_CONSTEXPR CARDS_CONSTEXPR
#else
    #define CARDS_CONSTEXPR
#endif

#include <functional>
#include <bit>

namespace NTichu::NGameplay::NCards {

class TBase {
// constants
public:
    static constexpr TCardsMask MK_1 = 1;
    static constexpr TCardsMask MK_2 = 3;
    static constexpr TCardsMask MK_4 = 15;

    static constexpr TCardsMask MK_DOG = MK_1 << (TCardId) EValue::DOG;
    static constexpr TCardsMask MK_MJ = MK_1 << (TCardId) EValue::MAH_JONG;
    static constexpr TCardsMask MK_PHX = MK_1 << (TCardId) EValue::PHOENIX;
    static constexpr TCardsMask MK_DRAGON = MK_1 << (TCardId) EValue::DRAGON;

    static constexpr TCardsMask MK_TWO = MK_1 << (TCardId) EValue::TWO;
    static constexpr TCardsMask MK_THREE = MK_1 << (TCardId) EValue::THREE;
    static constexpr TCardsMask MK_FOUR = MK_1 << (TCardId) EValue::FOUR;
    static constexpr TCardsMask MK_FIVE = MK_1 << (TCardId) EValue::FIVE;
    static constexpr TCardsMask MK_SIX = MK_1 << (TCardId) EValue::SIX;
    static constexpr TCardsMask MK_SEVEN = MK_1 << (TCardId) EValue::SEVEN;
    static constexpr TCardsMask MK_EIGHT = MK_1 << (TCardId) EValue::EIGHT;
    static constexpr TCardsMask MK_NINE = MK_1 << (TCardId) EValue::NINE;
    static constexpr TCardsMask MK_TEN = MK_1 << (TCardId) EValue::TEN;
    static constexpr TCardsMask MK_JACK = MK_1 << (TCardId) EValue::JACK;
    static constexpr TCardsMask MK_QUEEN = MK_1 << (TCardId) EValue::QUEEN;
    static constexpr TCardsMask MK_KING = MK_1 << (TCardId) EValue::KING;
    static constexpr TCardsMask MK_ACE = MK_1 << (TCardId) EValue::ACE;

    static constexpr TCardsMask MK_ALL_TWO = MK_4 << (TCardId) EValue::TWO;
    static constexpr TCardsMask MK_ALL_THREE = MK_4 << (TCardId) EValue::THREE;
    static constexpr TCardsMask MK_ALL_FOUR = MK_4 << (TCardId) EValue::FOUR;
    static constexpr TCardsMask MK_ALL_FIVE = MK_4 << (TCardId) EValue::FIVE;
    static constexpr TCardsMask MK_ALL_SIX = MK_4 << (TCardId) EValue::SIX;
    static constexpr TCardsMask MK_ALL_SEVEN = MK_4 << (TCardId) EValue::SEVEN;
    static constexpr TCardsMask MK_ALL_EIGHT = MK_4 << (TCardId) EValue::EIGHT;
    static constexpr TCardsMask MK_ALL_NINE = MK_4 << (TCardId) EValue::NINE;
    static constexpr TCardsMask MK_ALL_TEN = MK_4 << (TCardId) EValue::TEN;
    static constexpr TCardsMask MK_ALL_JACK = MK_4 << (TCardId) EValue::JACK;
    static constexpr TCardsMask MK_ALL_QUEEN = MK_4 << (TCardId) EValue::QUEEN;
    static constexpr TCardsMask MK_ALL_KING = MK_4 << (TCardId) EValue::KING;
    static constexpr TCardsMask MK_ALL_ACE = MK_4 << (TCardId) EValue::ACE;

    static constexpr TCardsMask MK_ALL_SPECIAL = MK_DOG | MK_MJ | MK_PHX | MK_DRAGON;
    static constexpr TCardsMask MK_ALL_BASE = MK_ALL_TWO | MK_ALL_THREE | MK_ALL_FOUR |
                                              MK_ALL_FIVE | MK_ALL_SIX | MK_ALL_SEVEN |
                                              MK_ALL_EIGHT | MK_ALL_NINE | MK_ALL_TEN |
                                              MK_ALL_JACK | MK_ALL_QUEEN | MK_ALL_KING | MK_ALL_ACE;

    static constexpr TCardsMask MK_SUIT = MK_TWO | MK_THREE | MK_FOUR | MK_FIVE |
                                          MK_SIX | MK_SEVEN | MK_EIGHT | MK_NINE |
                                          MK_TEN | MK_JACK | MK_QUEEN | MK_KING | MK_ACE;

    static constexpr TCardsMask MK_ALL = MK_ALL_BASE | MK_ALL_SPECIAL;

    static constexpr TCardsMask MK_SUIT_JACK = MK_SUIT << (ui8) ESuit::JACK;
    static constexpr TCardsMask MK_SUIT_PAGODA = MK_SUIT << (ui8) ESuit::PAGODA;
    static constexpr TCardsMask MK_SUIT_STAR = MK_SUIT << (ui8) ESuit::STAR;
    static constexpr TCardsMask MK_SUIT_SWORD = MK_SUIT << (ui8) ESuit::SWORD;

    static constexpr TCardsMask MK_SCORE_5 = MK_ALL_FIVE;
    static constexpr TCardsMask MK_SCORE_10 = MK_ALL_TEN | MK_ALL_KING;

    static constexpr TCardId INVALID_CARD_ID = std::numeric_limits<ui8>::max();

// helpers
public:
    static constexpr ui8 NumCards(TCardsMask cards) noexcept {
        return std::__popcount(cards);
    }

    static constexpr TCardId LastCard(TCardsMask cards) noexcept {
        return std::__countr_zero(cards);
    }

    static constexpr TCardId LastSkipedCard(TCardsMask cards) noexcept {
        return LastCard(~cards);
    }

    static constexpr TCardsMask Mask(TCardId card) noexcept {
        return MK_1 << card;
    }

    static constexpr bool ValidateCard(TCardId card) noexcept {
        return (Mask(card) & MK_ALL);
    }

    static constexpr bool ValidateCards(TCardsMask cards) noexcept {
        return !(cards & ~MK_ALL);
    }

    static constexpr EValue Value(TCardId card) noexcept {
        if (Mask(card) & MK_ALL_BASE) {
            return (EValue) (card & ~MK_2);
        }

        return (EValue) card;
    }

    static constexpr ESuit Suit(TCardId card) noexcept {
        if (Mask(card) & MK_ALL_BASE) {
            return (ESuit) (card & MK_2);
        }

        return ESuit::INVALID;
    }

    static constexpr TCardId Card(EValue value) noexcept {
        return (TCardId) value;
    }

    static constexpr TCardId Card(EValue value, ESuit suit) noexcept {
        return (TCardId) value + (TCardId) suit;
    }

    static constexpr bool ValidateValue(EValue value) noexcept {
        return ValidateCard(Card(value));
    }

    static constexpr bool ValidateSuit(ESuit suit) noexcept {
        return (ui8) suit < NUM_SUITS;
    }

    static constexpr TScore Score(TCardsMask mask) noexcept {
        return 5 * (TScore) NumCards(mask & MK_SCORE_5) +
               10 * (TScore) NumCards(mask & MK_SCORE_10) +
               25 * (TScore) NumCards(mask & MK_DRAGON) +
               -25 * (TScore) NumCards(mask & MK_PHX);
    }

    static constexpr bool IsBomb(ECombination comb) noexcept {
        return comb == ECombination::STRAIGHT_FLUSH ||
               comb == ECombination::QUAD;
    }

    static constexpr ECombination CombTy(TCardsMask cards, EValue phRep) noexcept {
        if (ValidateValue(phRep)) {
            return CombTyRep(cards, phRep);
        }

        return CombTyNoRep(cards);
    }

    static constexpr bool CombinableSingle(TCardsMask old, TCardsMask ne) noexcept {
        EValue oldVal = Value(LastCard(old));
        EValue neVal = Value(LastCard(ne));

        if (neVal == EValue::DRAGON) {
            return oldVal != EValue::DRAGON;
        }

        if (neVal == EValue::PHOENIX) {
            return oldVal != EValue::DRAGON && oldVal != EValue::PHOENIX;
        }

        return (ui8) neVal > (ui8) oldVal;
    }

    static constexpr bool CombinableSingleValue(TCardsMask old, TCardsMask ne) noexcept {
        EValue oldVal = Value(LastCard(old));
        EValue neVal = Value(LastCard(ne));

        return neVal > oldVal;
    }

    static constexpr bool CombinableFullHouse(TCardsMask old, TCardsMask ne) noexcept {
        ui64 oldStat = CombStat(old, 3);
        ui64 neStat = CombStat(ne, 3);

        return neStat > oldStat;
    }

    static constexpr bool CombinableStraightType(TCardsMask old, TCardsMask ne) noexcept {
        EValue oldValue = Value(LastCard(old));
        EValue neValue = Value(LastCard(ne));

        return oldValue < neValue && NumCards(old) == NumCards(ne);
    }

    static constexpr bool CombinableStraightFlush(TCardsMask old, TCardsMask ne) noexcept {
        EValue oldValue = Value(LastCard(old));
        EValue neValue = Value(LastCard(ne));

        return NumCards(ne) > NumCards(old) || (NumCards(ne) == NumCards(old) && neValue > oldValue);
    }

    static constexpr TCardsMask RepPh(TCardsMask cards, EValue phRep) noexcept {
        if (Mask(Card(phRep)) & MK_ALL_SPECIAL) {
            return 0;
        }

        if (!(cards & MK_PHX)) {
            return 0;
        }

        ui8 shift = LastSkipedCard((cards >> Card(phRep)));
        if (shift >= NUM_SUITS) {
            return 0;
        }

        return (cards | Card(phRep, (ESuit) shift)) & ~MK_PHX;
    }

private:
    static constexpr ECombination CombTyNoRep(TCardsMask cards) noexcept {
        if (IsEmpty(cards)) {
            return ECombination::EMPTY;
        }

        if (IsDog(cards)) {
            return ECombination::DOG;
        }

        if (IsSingle(cards)) {
            return ECombination::SINGLE;
        }

        if (IsPair(cards)) {
            return ECombination::PAIR;
        }

        if (IsTriple(cards)) {
            return ECombination::TRIPLE;
        }

        if (IsQuad(cards)) {
            return ECombination::QUAD;
        }

        if (IsFullHouse(cards)) {
            return ECombination::FULL_HOUSE;
        }

        if (IsAdjPairs(cards)) {
            return ECombination::ADJ_PAIRS;
        }

        if (IsStraight(cards)) {
            return ECombination::STRAIGHT;
        }

        if (IsStraightFlush(cards)) {
            return ECombination::STRAIGHT_FLUSH;
        }

        return ECombination::INVALID;
    }

    static constexpr ECombination CombTyRep(TCardsMask cards, EValue phRep) noexcept {
        cards = RepPh(cards, phRep);
        if (!cards) {
            return ECombination::INVALID;
        }

        ECombination ty = CombTyNoRep(cards);
        if (ty == ECombination::STRAIGHT_FLUSH) {
            return ECombination::STRAIGHT;
        }

        if (ty == ECombination::QUAD) {
            return ECombination::INVALID;
        }

        return ty;
    }

    static constexpr ui64 LocalCombStat(TCardsMask comb, ui8 num, TCardsMask valueMask, ui8 shift) noexcept {
        return (ui64) (NumCards(comb & valueMask) == num) << shift;
    }

    static constexpr ui64 CombStat(TCardsMask comb, ui8 num) noexcept {
        return LocalCombStat(comb, num, MK_ALL_TWO, 0) |
               LocalCombStat(comb, num, MK_ALL_THREE, 1) |
               LocalCombStat(comb, num, MK_ALL_FOUR, 2) |
               LocalCombStat(comb, num, MK_ALL_FIVE, 3) |
               LocalCombStat(comb, num, MK_ALL_SIX, 4) |
               LocalCombStat(comb, num, MK_ALL_SEVEN, 5) |
               LocalCombStat(comb, num, MK_ALL_EIGHT, 6) |
               LocalCombStat(comb, num, MK_ALL_NINE, 7) |
               LocalCombStat(comb, num, MK_ALL_TEN, 8) |
               LocalCombStat(comb, num, MK_ALL_JACK, 9) |
               LocalCombStat(comb, num, MK_ALL_QUEEN, 10) |
               LocalCombStat(comb, num, MK_ALL_KING, 11) |
               LocalCombStat(comb, num, MK_ALL_ACE, 12);
    }

    static constexpr bool IsSeq(ui64 combStat) noexcept {
        return LastSkipedCard(combStat >> (LastCard(combStat)));
    }

    static constexpr bool IsFlush(TCardsMask comb) noexcept {
        return NumCards(comb & MK_SUIT_JACK) == NumCards(comb) ||
               NumCards(comb & MK_SUIT_STAR) == NumCards(comb) ||
               NumCards(comb & MK_SUIT_SWORD) == NumCards(comb) ||
               NumCards(comb & MK_SUIT_PAGODA) == NumCards(comb);
    }

    static constexpr bool IsSingleValueComb(TCardsMask comb, ui8 num) noexcept {
        return NumCards(comb) == num && NumCards(CombStat(comb, num)) == 1;
    }

    static constexpr bool IsEmpty(TCardsMask comb) noexcept {
        return comb == 0;
    }

    static constexpr bool IsDog(TCardsMask comb) noexcept {
        return comb == MK_DOG;
    }

    static constexpr bool IsSingle(TCardsMask comb) noexcept {
        return NumCards(comb) == 1 && comb != MK_DOG;
    }

    static constexpr bool IsPair(TCardsMask comb) noexcept {
        return IsSingleValueComb(comb, 2);
    }

    static constexpr bool IsTriple(TCardsMask comb) noexcept {
        return IsSingleValueComb(comb, 3);
    }

    static constexpr bool IsQuad(TCardsMask comb) noexcept {
        return IsSingleValueComb(comb, 4);
    }

    static constexpr bool IsFullHouse(TCardsMask comb) noexcept {
        return NumCards(comb) == 5 && NumCards(CombStat(comb, 2)) == 1 && NumCards(CombStat(comb, 3)) == 1;
    }

    static constexpr bool IsAdjPairs(TCardsMask comb) noexcept {
        ui64 stat = CombStat(comb, 2);
        ui8 num = NumCards(comb);
        return num > 2 && NumCards(stat) * 2 == num && IsSeq(stat);
    }

    static constexpr bool IsStraight(TCardsMask comb) noexcept {
        bool mj = comb & MK_MJ;
        comb &= ~MK_MJ;
        ui64 stat = CombStat(comb, 1);

        return (!IsFlush(comb) || mj) && // it's not straight flush
               IsSeq(stat) && // cards is a sequential
               NumCards(comb) == NumCards(stat) && // values are unique
               NumCards(comb) >= 5 - mj; // common length >= 5
    }

    static constexpr bool IsStraightFlush(TCardsMask comb) noexcept {
        ui64 stat = CombStat(comb, 1);
        return NumCards(stat) == NumCards(comb) &&
               IsSeq(stat) &&
               IsFlush(comb) &&
               NumCards(comb) >= 5;
    }
};

CARDS_CONSTEXPR TCard::TCard() noexcept
    : Id_{TBase::INVALID_CARD_ID}
{
}

CARDS_CONSTEXPR TCard::TCard(TCardId cardId) noexcept
        : Id_(cardId) {
}

CARDS_CONSTEXPR TCard::TCard(EValue value) noexcept
    : Id_(TBase::Card(value))
{
}

CARDS_CONSTEXPR TCard::TCard(EValue value, ESuit suit) noexcept
    : Id_(TBase::Card(value, suit))
{
}

CARDS_CONSTEXPR bool TCard::Defined() const noexcept {
    return TBase::ValidateCard(Id_);
}

CARDS_CONSTEXPR EValue TCard::Value() const noexcept {
    return TBase::Value(Id_);
}

CARDS_CONSTEXPR ESuit TCard::Suit() const noexcept {
    return TBase::Suit(Id_);
}

CARDS_CONSTEXPR TCardId TCard::Id() const noexcept {
    return Id_;
}

CARDS_CONSTEXPR bool TCard::Basic() const noexcept {
    return TBase::Mask(Id_) & TBase::MK_ALL_BASE;
}

CARDS_CONSTEXPR TCards TCard::AsCards() const noexcept {
    return TCards(TBase::Mask(Id_));
}

CARDS_CONSTEXPR TCard TCard::MahJong() noexcept {
    return TCard(TBase::Card(EValue::MAH_JONG));
}

CARDS_CONSTEXPR TCard TCard::Dog() noexcept {
    return TCard(TBase::Card(EValue::DOG));
}

CARDS_CONSTEXPR TCard TCard::Phoenix() noexcept {
    return TCard(TBase::Card(EValue::PHOENIX));
}

CARDS_CONSTEXPR TCard TCard::Dragon() noexcept {
    return TCard(TBase::Card(EValue::DRAGON));
}

CARDS_CONSTEXPR bool TCard::operator==(TCard other) const noexcept {
    return other.Id_ == Id_;
}

CARDS_CONSTEXPR bool TCard::operator!=(TCard other) const noexcept {
    return other.Id_ != Id_;
}

CARDS_CONSTEXPR TCard::operator bool() noexcept {
    return TCard::Defined();
}

CARDS_CONSTEXPR TCards::TCards(TCard card) noexcept
    : Mask_(TBase::Mask(card.Id()))
{
}

CARDS_CONSTEXPR TCards::TCards(TCardsMask mask) noexcept
    : Mask_(mask)
{
}

CARDS_CONSTEXPR TCards::TCards(NTichu::NGameplay::NCards::TCombination comb) noexcept
    : Mask_(comb.AsCards().Mask())
{
}

CARDS_CONSTEXPR void TCards::Join(TCards cards) noexcept {
    Mask_ |= cards.Mask_;
}

CARDS_CONSTEXPR void TCards::Rm(TCards cards) noexcept {
    Mask_ &= ~cards.Mask_;
}

CARDS_CONSTEXPR void TCards::Drop() noexcept {
    Mask_ = 0;
}

CARDS_CONSTEXPR bool TCards::Empty() const noexcept {
    return !Mask_;
}

CARDS_CONSTEXPR ui8 TCards::Num() const noexcept { return TBase::NumCards(Mask_); }
CARDS_CONSTEXPR TCardsMask TCards::Mask() const noexcept { return Mask_; }

CARDS_CONSTEXPR bool TCards::Has(TCards cards) const noexcept {
    return (Mask_ | cards.Mask_) == Mask_;
}

CARDS_CONSTEXPR bool TCards::HasAny(TCards cards) const noexcept {
    return (Mask_ & cards.Mask_);
}

CARDS_CONSTEXPR TCard TCards::Last() const noexcept {
    return TCard(TBase::LastCard(Mask_));
}

CARDS_CONSTEXPR TScore TCards::Score() const noexcept {
    return TBase::Score(Mask_);
}

CARDS_CONSTEXPR TCards TCards::Deck() noexcept {
    return TCards(TBase::MK_ALL);
}

CARDS_CONSTEXPR bool TCards::operator==(TCards other) const noexcept {
    return Mask_ == other.Mask_;
}

CARDS_CONSTEXPR bool TCards::operator!=(TCards other) const noexcept {
    return Mask_ != other.Mask_;
}

CARDS_CONSTEXPR TCard TCards::TIterator::operator*() const noexcept {
    return TCard(TBase::LastCard(Mask_));
}

CARDS_CONSTEXPR bool TCards::TIterator::operator==(TCards::TIterator other) const noexcept {
    return Mask_ == other.Mask_;
}

CARDS_CONSTEXPR bool TCards::TIterator::operator!=(TCards::TIterator other) const noexcept {
    return Mask_ != other.Mask_;
}

CARDS_CONSTEXPR TCards::TIterator& TCards::TIterator::operator++() noexcept {
    Mask_ ^= TBase::Mask(TBase::LastCard(Mask_));
    return *this;
}

CARDS_CONSTEXPR TCards::TIterator::TIterator(TCards cards) noexcept: Mask_(cards.Mask()) {}

CARDS_CONSTEXPR TCards::TIterator TCards::begin() const noexcept {
    return TIterator(*this);
}

CARDS_CONSTEXPR TCards::TIterator TCards::end() const noexcept {
    return TIterator(TCards(0));
}

CARDS_CONSTEXPR TCombination::TCombination(TCards cards, EValue phRep) noexcept
    : PhRep_(phRep), Type_(TBase::CombTy(cards.Mask(), phRep)), Mask_(cards.Mask())
{
}

CARDS_CONSTEXPR ECombination TCombination::Ty() const noexcept {
    return Type_;
}

CARDS_CONSTEXPR EValue TCombination::PhRep() const noexcept {
    return PhRep_;
}

CARDS_CONSTEXPR TCards TCombination::AsCards() const noexcept {
    return TCards(Mask_);
}

CARDS_CONSTEXPR bool TCombination::Defined() const noexcept {
    return Type_ != ECombination::INVALID;
}

CARDS_CONSTEXPR bool TCombination::IsBomb() const noexcept {
    return TBase::IsBomb(Type_);
}

CARDS_CONSTEXPR void TCombination::Drop() noexcept {
    Mask_ = 0;
    PhRep_ = EValue::INVALID;
    Type_ = ECombination::EMPTY;
}

CARDS_CONSTEXPR bool TCombination::operator==(TCombination other) const noexcept {
    return Mask_ == other.Mask_ && PhRep_ == other.PhRep_;
}

CARDS_CONSTEXPR bool TCombination::operator!=(TCombination other) const noexcept {
    return !(*this == other);
}

CARDS_CONSTEXPR TCombination::operator bool() const noexcept {
    return Defined();
}

CARDS_CONSTEXPR TCombination TCombination::Invalid() noexcept {
    TCombination comb;
    comb.Type_ = ECombination::INVALID;
    return comb;
}

CARDS_CONSTEXPR TCardsMask TCombination::CombMask() const noexcept {
    if (!TBase::ValidateValue(PhRep_)) {
        return Mask_;
    }

    return TBase::RepPh(Mask_, PhRep_);
}

CARDS_CONSTEXPR TCombination TCombination::Combine(TCombination comb) const noexcept {
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

            if (TBase::CombinableSingleValue(CombMask(), comb.CombMask())) {
                return comb;
            }

            return Invalid();
        }

        case ECombination::FULL_HOUSE: {
            if (comb.IsBomb()) {
                return comb;
            }

            if (TBase::CombinableFullHouse(CombMask(), comb.CombMask())) {
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

#undef CARDS_CONSTEXPR
