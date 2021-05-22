#include "generator.h"

#include <server/lib/gameplay/cards/cards.h>

#include <cassert>
#include <algorithm>

namespace NTichu::NGameplay::NState {

class TGenerator: public IGenerator {
public:
    TGenerator(unsigned int seed);

    void Reset() override;
    NCards::TCards Generate(ui32 num) override;

    static TGenerator Random();
    static TGenerator Test();

private:
    std::array<NCards::TCard, NCards::NUM_CARDS> Cards_;
    std::mt19937 Generator_;
    ui32 Given_{0};
};

TGenerator::TGenerator(unsigned int seed)
    : Generator_(seed)
{
    Reset();
}

void TGenerator::Reset() {
    Given_ = 0;
    size_t index = 0;
    for (auto card: NCards::TCards::Deck()) {
        Cards_[index++] = card;
    }

    assert(index == NCards::NUM_CARDS);
    std::shuffle(Cards_.begin(), Cards_.end(), Generator_);
}

NCards::TCards TGenerator::Generate(ui32 num) {
    assert(Given_ + num <= NCards::NUM_CARDS);
    NCards::TCards cards;
    for (size_t i = 0; i < num; ++i) {
        cards.Join(Cards_[Given_++]);
    }

    return cards;
}

TGenerator TGenerator::Random() {
    std::random_device d;
    std::uniform_int_distribution<unsigned int> dist;
    return TGenerator(dist(d));
}

TGenerator TGenerator::Test() {
    return TGenerator(0);
}

IGeneratorPtr CreateGenerator(unsigned int seed) {
    return std::make_shared<TGenerator>(seed);
}

IGeneratorPtr CreateRandomGenerator() {
    return std::make_shared<TGenerator>(TGenerator::Random());
}

IGeneratorPtr CreateTestGenerator() {
    return std::make_shared<TGenerator>(TGenerator::Test());
}

} // namespace NTichu::NGameplay::NState
