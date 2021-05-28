#pragma once

#include "defs.h"
#include "events.h"
#include "snapshot.h"

#include <server/lib/gameplay/state/generator.h>

#include <memory>

namespace NTichu::NGameplay::NState {

class IState: public IEventConsumer {
public:
    virtual void Start() = 0;

    virtual TSnapshot Snapshot(EPosition pos) const = 0;
    virtual EState State() const = 0;

    virtual ~IState() = default;
};

std::unique_ptr<IState> CreateState(IGeneratorPtr generator);

} // namespace NTichu::NGameplay::NState
