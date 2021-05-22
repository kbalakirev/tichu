#pragma once

#include <server/lib/gameplay/cards/cards.h>

#include <random>
#include <array>
#include <memory>

namespace NTichu::NGameplay::NState {

class IGenerator {
public:
    virtual void Reset() = 0;
    virtual NCards::TCards Generate(ui32 num) = 0;

    virtual ~IGenerator() = default;
};

using IGeneratorPtr = std::shared_ptr<IGenerator>;

IGeneratorPtr CreateGenerator(unsigned int seed);
IGeneratorPtr CreateRandomGenerator();
IGeneratorPtr CreateTestGenerator();

} // namespace NTichu::NGameplay::NState
