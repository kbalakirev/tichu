#include <gtest/gtest.h>

#include <server/lib/gameplay/state/state.h>
#include <server/lib/gameplay/state/state.cpp>

#include <iostream>

using namespace NTichu::NGameplay::NState;
using namespace NTichu::NGameplay::NCards;


class TFakeGenerator: public IGenerator {
public:
    explicit TFakeGenerator(std::array<TCard, NUM_CARDS> cards) {
        Cards_ = cards;
    }

    void Reset() override {
        Given_ = 0;
    }

    TCards Generate(ui32 num) override {
        TCards cards;
        for (size_t i = 0; i < num; ++i) {
            cards.Join(Cards_.at(Given_ + i));
        }

        Given_ += num;
        return cards;
    }

private:
    std::array<TCard, NUM_CARDS> Cards_;
    size_t Given_ = 0;
};


TEST(State, Example) {
    std::array<TCard, NUM_CARDS> generator {
        TCard(EValue::TWO, ESuit::JACK),
        TCard(EValue::FOUR, ESuit::PAGODA),
        TCard(EValue::EIGHT, ESuit::STAR),
        TCard(EValue::JACK, ESuit::SWORD),
        TCard(EValue::QUEEN, ESuit::PAGODA),
        TCard(EValue::PHOENIX),
        TCard(EValue::ACE, ESuit::STAR),
        TCard(EValue::ACE, ESuit::JACK),

        TCard(EValue::SEVEN, ESuit::SWORD),
        TCard(EValue::SEVEN, ESuit::PAGODA),
        TCard(EValue::TEN, ESuit::JACK),
        TCard(EValue::TEN, ESuit::SWORD),
        TCard(EValue::TEN, ESuit::STAR),
        TCard(EValue::QUEEN, ESuit::JACK),
        TCard(EValue::QUEEN, ESuit::STAR),
        TCard(EValue::KING, ESuit::STAR),

        TCard(EValue::TWO, ESuit::SWORD),
        TCard(EValue::SIX, ESuit::STAR),
        TCard(EValue::TEN, ESuit::PAGODA),
        TCard(EValue::JACK, ESuit::PAGODA),
        TCard(EValue::JACK, ESuit::STAR),
        TCard(EValue::KING, ESuit::SWORD),
        TCard(EValue::ACE, ESuit::PAGODA),
        TCard::Dragon(),

        TCard::Dog(),
        TCard(EValue::THREE, ESuit::JACK),
        TCard(EValue::THREE, ESuit::PAGODA),
        TCard(EValue::FIVE, ESuit::STAR),
        TCard(EValue::FIVE, ESuit::SWORD),
        TCard(EValue::FIVE, ESuit::JACK),
        TCard(EValue::EIGHT, ESuit::JACK),
        TCard(EValue::NINE, ESuit::STAR),

        TCard(EValue::TWO, ESuit::STAR),
        TCard(EValue::FOUR, ESuit::SWORD),
        TCard(EValue::SIX, ESuit::SWORD),
        TCard(EValue::SIX, ESuit::JACK),
        TCard(EValue::SEVEN, ESuit::STAR),
        TCard(EValue::QUEEN, ESuit::SWORD),

        TCard(EValue::THREE, ESuit::STAR),
        TCard(EValue::EIGHT, ESuit::SWORD),
        TCard(EValue::NINE, ESuit::JACK),
        TCard(EValue::JACK, ESuit::JACK),
        TCard(EValue::KING, ESuit::JACK),
        TCard(EValue::ACE, ESuit::SWORD),

        TCard(EValue::THREE, ESuit::SWORD),
        TCard(EValue::FOUR, ESuit::STAR),
        TCard(EValue::FOUR, ESuit::JACK),
        TCard(EValue::FIVE, ESuit::PAGODA),
        TCard(EValue::SEVEN, ESuit::JACK),
        TCard(EValue::NINE, ESuit::SWORD),

        TCard::MahJong(),
        TCard(EValue::TWO, ESuit::PAGODA),
        TCard(EValue::SIX, ESuit::PAGODA),
        TCard(EValue::EIGHT, ESuit::PAGODA),
        TCard(EValue::NINE, ESuit::PAGODA),
        TCard(EValue::KING, ESuit::PAGODA),
    };

    {
        TCards cards;
        for (auto card: generator) {
            cards.Join(card);
        }

        ASSERT_EQ(cards, TCards::Deck()) << AsString(cards);
    }

    auto state = CreateState(std::make_unique<TFakeGenerator>(generator));

    ASSERT_EQ(state->State(), EState::NOT_INITIALIZED);

    state->Start();
    ASSERT_EQ(state->State(), EState::NEW_ROUND);

    ASSERT_TRUE(state->GTChoice({EPosition::POS_0, true}).Succeed());
    ASSERT_EQ(state->State(), EState::NEW_ROUND);

    ASSERT_TRUE(state->GTChoice({EPosition::POS_1, false}).Succeed());
    ASSERT_EQ(state->State(), EState::NEW_ROUND);

    ASSERT_TRUE(state->GTChoice({EPosition::POS_2, false}).Succeed());
    ASSERT_EQ(state->State(), EState::NEW_ROUND);

    ASSERT_TRUE(state->GTChoice({EPosition::POS_3, false}).Succeed());
    ASSERT_EQ(state->State(), EState::EXCHANGING);

    TStateErrorOr r;
    r = state->Exchange(
            {EPosition::POS_0,
             {
                TCard(),
                TCard(EValue::SEVEN, ESuit::STAR),
                TCard(EValue::JACK, ESuit::SWORD),
                TCard(EValue::EIGHT, ESuit::STAR)
             }
            }
        );

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::EXCHANGING);

    r = state->Exchange(
            {EPosition::POS_1,
             {
                     TCard(EValue::THREE, ESuit::STAR),
                     TCard(),
                     TCard(EValue::NINE, ESuit::JACK),
                     TCard(EValue::EIGHT, ESuit::SWORD)
             }
            }
    );

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::EXCHANGING);

    r = state->Exchange(
            {EPosition::POS_2,
             {
                     TCard::Dragon(),
                     TCard(EValue::FOUR, ESuit::STAR),
                     TCard(),
                     TCard(EValue::TEN, ESuit::PAGODA)
             }
            }
    );

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::EXCHANGING);

    r = state->Exchange(
            {EPosition::POS_3,
             {
                     TCard::Dog(),
                     TCard(EValue::KING, ESuit::PAGODA),
                     TCard(EValue::TWO, ESuit::PAGODA),
                     TCard()
             }
            }
    );

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({EPosition::POS_3, TCard::MahJong().AsCards()});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Move({EPosition::POS_0, TCard(EValue::ACE, ESuit::JACK).AsCards()});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_1});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
        EPosition::POS_0,
        TCombination(TCards::Un(
                  TCard(EValue::TWO, ESuit::STAR),
                  TCard(EValue::TWO, ESuit::JACK),
                  TCard(EValue::THREE, ESuit::STAR),
                  TCard::Phoenix(),
                  TCard(EValue::FOUR, ESuit::SWORD),
                  TCard(EValue::FOUR, ESuit::PAGODA)
                ),
            EValue::THREE
            )
        });

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_1});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_0,
            TCards::Un(
                TCard(EValue::SIX, ESuit::SWORD),
                TCard(EValue::SIX, ESuit::JACK)
            )
        });

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Move({
            EPosition::POS_1,
            TCards::Un(
                TCard(EValue::QUEEN, ESuit::JACK),
                TCard(EValue::QUEEN, ESuit::STAR)
            )
        });

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_0});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_1,
            TCards::Un(
                TCard(EValue::SEVEN, ESuit::STAR),
                TCard(EValue::SEVEN, ESuit::SWORD),
                TCard(EValue::SEVEN, ESuit::PAGODA)
            )
        });

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Move({
            EPosition::POS_2,
            TCards::Un(
                TCard(EValue::JACK, ESuit::STAR),
                TCard(EValue::JACK, ESuit::SWORD),
                TCard(EValue::JACK, ESuit::PAGODA)
            )
        });

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_0});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Move({
            EPosition::POS_1,
            TCards::Un(
                TCard(EValue::KING, ESuit::STAR),
                TCard(EValue::KING, ESuit::JACK),
                TCard(EValue::KING, ESuit::PAGODA)
            )
        });

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);


    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_0});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_1,
            TCards::Un(
                TCard(EValue::TEN, ESuit::STAR),
                TCard(EValue::TEN, ESuit::JACK),
                TCard(EValue::TEN, ESuit::SWORD)
            )
        });

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);


    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_0});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);


    r = state->Move({
            EPosition::POS_1,
            TCard(EValue::FOUR, ESuit::STAR).AsCards()
        });

    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Move({
            EPosition::POS_3,
            TCard(EValue::SIX, ESuit::PAGODA).AsCards()
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Move({
            EPosition::POS_0,
            TCard(EValue::ACE, ESuit::STAR).AsCards()
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_1});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Move({
            EPosition::POS_3,
            TCards::Un(
                    TCard(EValue::EIGHT, ESuit::PAGODA),
                    TCard(EValue::EIGHT, ESuit::SWORD),
                    TCard(EValue::EIGHT, ESuit::JACK),
                    TCard(EValue::EIGHT, ESuit::STAR)
                )
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_0});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_1});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_3,
            TCards::Un(
                TCard(EValue::THREE, ESuit::JACK),
                TCard(EValue::THREE, ESuit::PAGODA),
                TCard(EValue::FIVE, ESuit::STAR),
                TCard(EValue::FIVE, ESuit::SWORD),
                TCard(EValue::FIVE, ESuit::JACK)
            )
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_0});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_1});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_3,
            TCards::Un(
                TCard(EValue::NINE, ESuit::STAR),
                TCard(EValue::NINE, ESuit::PAGODA)
            )
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Move({
            EPosition::POS_0,
            TCards::Un(
                TCard(EValue::QUEEN, ESuit::SWORD),
                TCard(EValue::QUEEN, ESuit::PAGODA)
            )
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_1});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_0,
            TCard::Dragon().AsCards()
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_1});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::GIVE_DRAGON);

    r = state->GiveDragon({EPosition::POS_0, EPosition::POS_3});
    ASSERT_EQ(state->State(), EState::NEW_TURN);


    r = state->Move({
            EPosition::POS_0,
            TCard::Dog().AsCards()
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_2,
            TCards::Un(
                TCard(EValue::TWO, ESuit::SWORD),
                TCard(EValue::THREE, ESuit::SWORD),
                TCard(EValue::FOUR, ESuit::JACK),
                TCard(EValue::FIVE, ESuit::PAGODA),
                TCard(EValue::SIX, ESuit::STAR),
                TCard(EValue::SEVEN, ESuit::JACK)
            )
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_1});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_2,
            TCards::Un(
                TCard(EValue::NINE, ESuit::JACK),
                TCard(EValue::NINE, ESuit::SWORD)
            )
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_1});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_2,
            TCard(EValue::KING, ESuit::SWORD).AsCards()
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Move({
            EPosition::POS_1,
            TCard(EValue::ACE, ESuit::SWORD).AsCards()
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_2});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_1,
            TCard(EValue::JACK, ESuit::JACK).AsCards()
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Move({
            EPosition::POS_2,
            TCard(EValue::ACE, ESuit::PAGODA).AsCards()
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::TURN);

    r = state->Pass({EPosition::POS_3});
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_TURN);

    r = state->Move({
            EPosition::POS_2,
            TCard(EValue::TWO, ESuit::PAGODA).AsCards()
        });
    ASSERT_TRUE(r.Succeed());
    ASSERT_EQ(state->State(), EState::NEW_ROUND);

    /*
     * team 0 score: 200 + 30
     * team 1 score: 70
     */
}
