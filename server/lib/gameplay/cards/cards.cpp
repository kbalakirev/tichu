#include "impl.h"

#include <sstream>

namespace NTichu::NGameplay::NCards {

std::string AsString(ESuit suit) {
    switch (suit) {
        case ESuit::JACK: return "JA";
        case ESuit::SWORD: return "SW";
        case ESuit::STAR: return "ST";
        case ESuit::PAGODA: return "PA";
        default: return "IN";
    }
}

std::string AsString(EValue value) {
    switch (value) {
        case EValue::TWO: return "TW";
        case EValue::THREE: return "TH";
        case EValue::FOUR: return "FO";
        case EValue::FIVE: return "FI";
        case EValue::SIX: return "SI";
        case EValue::SEVEN: return "SE";
        case EValue::EIGHT: return "EI";
        case EValue::NINE: return "NI";
        case EValue::TEN: return "TE";
        case EValue::JACK: return "JA";
        case EValue::QUEEN: return "QU";
        case EValue::KING: return "KI";
        case EValue::ACE: return "AC";
        case EValue::MAH_JONG: return "MJ";
        case EValue::DOG: return "DO";
        case EValue::PHOENIX: return "PH";
        case EValue::DRAGON: return "DR";
        default: return "IN";
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