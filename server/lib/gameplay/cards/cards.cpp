#include "cards.h"

#include <sstream>

namespace NTichu::NGameplay::NCards {

std::string AsString(ESuit suit) {
    switch (suit) {
        case ESuit::JACK: return "JACK";
        case ESuit::SWORD: return "SWORD";
        case ESuit::STAR: return "STAR";
        case ESuit::PAGODA: return "PAGODA";
        default: return "INVALID";
    }
}

std::string AsString(EValue value) {
    switch (value) {
        case EValue::TWO: return "2";
        case EValue::THREE: return "3";
        case EValue::FOUR: return "4";
        case EValue::FIVE: return "5";
        case EValue::SIX: return "6";
        case EValue::SEVEN: return "7";
        case EValue::EIGHT: return "8";
        case EValue::NINE: return "9";
        case EValue::TEN: return "10";
        case EValue::JACK: return "JACK";
        case EValue::QUEEN: return "QUEEN";
        case EValue::KING: return "KING";
        case EValue::ACE: return "ACE";
        case EValue::MAH_JONG: return "1";
        case EValue::DOG: return "DOG";
        case EValue::PHOENIX: return "PHX";
        case EValue::DRAGON: return "DRAGON";
        default: return "INVALID";
    }
}

std::string AsString(ECombination combTy) {
    switch (combTy) {
        case ECombination::EMPTY: return "EMPTY";
        case ECombination::DOG: return "DOG";
        case ECombination::SINGLE: return "SINGLE";
        case ECombination::PAIR: return "PAIR";
        case ECombination::TRIPLE: return "TRIPLE";
        case ECombination::QUAD: return "QUAD";
        case ECombination::FULL_HOUSE: return "FULL_HOUSE";
        case ECombination::ADJ_PAIRS: return "ADJ_PAIRS";
        case ECombination::STRAIGHT: return "STRAIGHT";
        case ECombination::STRAIGHT_FLUSH: return "STRAIGHT_FLUSH";
        default: return "INVALID";
    }
}

std::string AsString(TCard card) {
    std::stringstream ss;

    if (card.Basic()) {
        ss << "(" << AsString(card.Value()) << ", " << AsString(card.Suit()) << ")";
    } else {
        ss << AsString(card.Value());
    }

    return ss.str();
}

std::string AsString(TCards cards) {
    std::stringstream ss;
    ss << "{";

    size_t i = 0;
    for (auto card: cards) {
        ss << AsString(card);
        ++i;
        if (i != cards.Num()) {
            ss << ", ";
        }
    }

    ss << "}";

    return ss.str();
}

std::string AsString(TCombination comb) {
    std::stringstream ss;
    ss << "[" << AsString(comb.Ty()) << ": " << AsString(comb.AsCards());
    if (comb.PhRep() != EValue::INVALID) {
        ss << " " << AsString(EValue::PHOENIX) << " -> " << AsString(comb.PhRep());
    }
    ss << "]";

    return ss.str();
}

} // NTichu::NGameplay::NCards