#include <gtest/gtest.h>

#include <server/lib/gameplay/cards/cards.h>

#include <unordered_set>

using namespace NTichu::NGameplay::NCards;

TEST(Card, Defined) {
    TCard def;
    TCard special = TCard(EValue::PHOENIX);
    TCard basic = TCard(EValue::TWO, ESuit::STAR);

    ASSERT_FALSE(def.Defined());
    ASSERT_FALSE((bool) def);

    ASSERT_TRUE(special.Defined());
    ASSERT_TRUE((bool) special);
    ASSERT_TRUE(basic.Defined());
    ASSERT_TRUE((bool) basic);

    ASSERT_TRUE(TCard::Phoenix().Defined());
    ASSERT_TRUE(TCard::MahJong().Defined());
    ASSERT_TRUE(TCard::Dragon().Defined());
    ASSERT_TRUE(TCard::Dog().Defined());
}

TEST(Card, Basic) {
    for (auto value: AllValues) {
        if (value == EValue::DRAGON ||
            value == EValue::PHOENIX ||
            value == EValue::DOG ||
            value == EValue::MAH_JONG)
        {
            ASSERT_FALSE(TCard(value).Basic());
            continue;
        }

        for (ESuit suit: AllSuits) {
            ASSERT_TRUE(TCard(value, suit).Basic());
        }
    }
}

TEST(Card, ValueAndSuit) {
    ASSERT_EQ(TCard::Dog().Value(), EValue::DOG);
    ASSERT_EQ(TCard::MahJong().Value(), EValue::MAH_JONG);
    ASSERT_EQ(TCard::Phoenix().Value(), EValue::PHOENIX);
    ASSERT_EQ(TCard::Dragon().Value(), EValue::DRAGON);

    for (EValue value: AllValues) {
        if (!TCard(value).Basic()) {
            continue;
        }

        for (ESuit suit: AllSuits) {
            ASSERT_EQ(TCard(value, suit).Value(), value);
            ASSERT_EQ(TCard(value, suit).Suit(), suit);
        }
    }
}

TEST(Card, Comp) {
    ASSERT_EQ(TCard::Dog(), TCard(EValue::DOG));
    ASSERT_EQ(TCard::MahJong(), TCard(EValue::MAH_JONG));
    ASSERT_EQ(TCard::Phoenix(), TCard(EValue::PHOENIX));
    ASSERT_EQ(TCard::Dragon(), TCard(EValue::DRAGON));

    size_t i = 0;
    size_t  j = 0;
    for (auto card: TCards::Deck()) {
        for (auto other: TCards::Deck()) {
            ASSERT_EQ(card == other, i == j) << AsString(card) << ", " << AsString(other) << ", " << i << ", " << j;
            ++j;
        }

        j = 0;
        ++i;
    }
}

TEST(Cards, Num) {
    TCards cards;
    ASSERT_TRUE(cards.Empty());
    ASSERT_EQ(cards.Num(), 0);

    cards = TCards(TCard::Phoenix());
    ASSERT_FALSE(cards.Empty());
    ASSERT_EQ(cards.Num(), 1);
}

TEST(Cards, Join) {
    TCards cards = TCards::Un(
                TCard::Phoenix(),
                TCard(EValue::TWO, ESuit::STAR),
                TCards::Un(TCard(EValue::KING, ESuit::JACK), TCard(EValue::KING, ESuit::PAGODA))
            );

    cards.Join(TCard::Dragon(), TCard::Dog());

    ASSERT_EQ(cards.Num(), 6);
    ASSERT_TRUE(cards.Has(TCard::Phoenix()));
    ASSERT_TRUE(cards.Has(TCard::Dragon()));
    ASSERT_TRUE(cards.Has(TCard::Dog()));
    ASSERT_TRUE(cards.Has(TCard(EValue::TWO, ESuit::STAR)));
    ASSERT_TRUE(cards.Has(TCard(EValue::KING, ESuit::JACK)));
    ASSERT_TRUE(cards.Has(TCard(EValue::KING, ESuit::PAGODA)));
}

TEST(Cards, Rm) {
    TCards cards = TCards::Un(
            TCard::Phoenix(),
            TCard(EValue::TWO, ESuit::STAR),
            TCards::Un(TCard(EValue::KING, ESuit::JACK), TCard(EValue::KING, ESuit::PAGODA))
    );

    cards.Join(TCard::Dragon(), TCard::Dog());

    cards.Rm(TCard::Dog(), TCards::Un(TCard(EValue::KING, ESuit::JACK), TCard(EValue::KING, ESuit::PAGODA)));

    ASSERT_EQ(cards.Num(), 3);
    ASSERT_TRUE(cards.Has(TCard::Phoenix()));
    ASSERT_TRUE(cards.Has(TCard::Dragon()));
    ASSERT_FALSE(cards.Has(TCard::Dog()));
    ASSERT_TRUE(cards.Has(TCard(EValue::TWO, ESuit::STAR)));
    ASSERT_FALSE(cards.Has(TCard(EValue::KING, ESuit::JACK)));
    ASSERT_FALSE(cards.Has(TCard(EValue::KING, ESuit::PAGODA)));
}

TEST(Cards, Has) {
    TCards cards = TCards::Un(
                TCard(EValue::TWO, ESuit::STAR),
                TCard(EValue::ACE, ESuit::PAGODA),
                TCard(EValue::QUEEN, ESuit::JACK),
                TCard::Phoenix(),
                TCard::Dog()
            );

    TCards other = TCards::Un(
                TCard::Phoenix(),
                TCard::Dog()
            );

    ASSERT_TRUE(cards.Has(TCard(EValue::TWO, ESuit::STAR)));
    ASSERT_TRUE(cards.Has(TCard(EValue::ACE, ESuit::PAGODA)));
    ASSERT_TRUE(cards.Has(TCard(EValue::QUEEN, ESuit::JACK)));
    ASSERT_TRUE(cards.Has(TCard::Phoenix()));
    ASSERT_TRUE(cards.Has(TCard::Dog()));
    ASSERT_TRUE(cards.Has(other));
    ASSERT_FALSE(cards.Has(TCard::MahJong()));
    ASSERT_FALSE(other.Has(cards));
    ASSERT_TRUE(other.HasAny(cards));
    ASSERT_TRUE(cards.HasAny(other));

    other = TCards::Un(TCard::Phoenix(), TCard::MahJong());
    ASSERT_TRUE(other.HasAny(cards));
    ASSERT_TRUE(cards.HasAny(other));

    ASSERT_FALSE(other.Has(cards));
    ASSERT_FALSE(cards.Has(other));
}

TEST(Cards, Drop) {
    TCards cards = TCards::Un(
            TCard(EValue::TWO, ESuit::STAR),
            TCard(EValue::ACE, ESuit::PAGODA),
            TCard(EValue::QUEEN, ESuit::JACK),
            TCard::Phoenix(),
            TCard::Dog()
    );

    ASSERT_FALSE(cards.Empty());
    ASSERT_EQ(cards.Num(), 5);

    cards.Drop();
    ASSERT_TRUE(cards.Empty());
    ASSERT_EQ(cards.Num(), 0);
}

TEST(Cards, Score) {
    TCards cards = TCards::Un(
            TCard(EValue::TWO, ESuit::STAR),
            TCard(EValue::ACE, ESuit::PAGODA),
            TCard(EValue::QUEEN, ESuit::JACK),
            TCard::Phoenix(),
            TCard::Dog()
    );

    ASSERT_EQ(cards.Score(), -25);
    cards.Join(TCard(EValue::FIVE));
    ASSERT_EQ(cards.Score(), -20);
    cards.Join(TCard(EValue::TEN));
    ASSERT_EQ(cards.Score(), -10);
    cards.Join(TCard(EValue::KING));
    ASSERT_EQ(cards.Score(), 0);
    cards.Join(TCard::Dragon());
    ASSERT_EQ(cards.Score(), 25);

    ASSERT_EQ(TCards::Deck().Score(), 100);
}

TEST(Cards, Iter) {
    std::unordered_set<TCard> cards;
    for (auto card: TCards::Deck()) {
        cards.insert(card);
    }

    for (auto value: AllValues) {
        if (TCard(value).Basic()) {
            for (auto suit: AllSuits) {
                ASSERT_TRUE(cards.count(TCard(value, suit)));
            }
        } else {
            ASSERT_TRUE(cards.count(TCard(value)));
        }
    }
}

TEST(Combination, Simple) {
    TCards emptyCards;
    TCards singleCards = TCard(EValue::TWO);
    TCards pairCards = TCards::Un(
            TCard(EValue::THREE, ESuit::JACK),
            TCard(EValue::THREE, ESuit::STAR)
            );
    TCards tripleCards = TCards::Un(pairCards, TCard(EValue::THREE, ESuit::PAGODA));
    TCards quadCards = TCards::Un(tripleCards, TCard(EValue::THREE, ESuit::SWORD));

    TCards fullHouseCards = TCards::Un(
            tripleCards,
            TCard(EValue::FOUR, ESuit::JACK),
            TCard(EValue::FOUR, ESuit::PAGODA)
        );

    TCards adjPairsCards = TCards::Un(
            pairCards,
            TCard(EValue::FOUR, ESuit::JACK),
            TCard(EValue::FOUR, ESuit::PAGODA)
        );

    TCards straightMjCards = TCards::Un(
            TCard::MahJong(),
            TCard(EValue::TWO),
            TCard(EValue::THREE),
            TCard(EValue::FOUR),
            TCard(EValue::FIVE)
        );

    TCards straightCards = straightMjCards;
    TCards strFlushCards = straightMjCards;
    straightCards.Rm(TCard::MahJong());
    straightCards.Join(TCard(EValue::SIX, (ESuit) 1));
    strFlushCards.Rm(TCard::MahJong());
    strFlushCards.Join(TCard(EValue::SIX));

    ASSERT_EQ(TCombination(emptyCards).Ty(), ECombination::EMPTY);
    ASSERT_EQ(TCombination(singleCards).Ty(), ECombination::SINGLE);
    ASSERT_EQ(TCombination(pairCards).Ty(), ECombination::PAIR);
    ASSERT_EQ(TCombination(tripleCards).Ty(), ECombination::TRIPLE);
    ASSERT_EQ(TCombination(quadCards).Ty(), ECombination::QUAD);
    ASSERT_EQ(TCombination(fullHouseCards).Ty(), ECombination::FULL_HOUSE);
    ASSERT_EQ(TCombination(adjPairsCards).Ty(), ECombination::ADJ_PAIRS);
    ASSERT_EQ(TCombination(straightMjCards).Ty(), ECombination::STRAIGHT);
    ASSERT_EQ(TCombination(straightCards).Ty(), ECombination::STRAIGHT);
    ASSERT_EQ(TCombination(strFlushCards).Ty(), ECombination::STRAIGHT_FLUSH);
    ASSERT_EQ(TCombination(TCard::Dog()).Ty(), ECombination::DOG);
}
