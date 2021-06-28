#include "table.h"

#include <server/lib/gameplay/state/state.h>
#include <server/lib/flags/flags.h>

#include <unordered_map>
#include <array>
#include <memory>
#include <cassert>

using namespace NActors;

namespace NTichu::NServer {

using namespace NEvTableManager;
using namespace NGameplay;

template <class TPlayer>
class TTableMembership {
public:
    bool Complete() const {
        return ValidatePosition(FindPosition());
    }

    NState::EPosition Join(TPlayer player) {
        assert(!Complete());

        NState::EPosition pos = FindPosition();

        PosMap_.Store((ui8) pos);
        Membership[(ui8) pos] = std::move(player);

        return pos;
    }

    TPlayer Member(NState::EPosition pos) const {
        assert(PosMap_.Stored((ui8) pos));

        return Membership[(ui8) pos];
    }

private:
    NState::EPosition FindPosition() const {
        for (auto pos: NState::AllPositions) {
            if (!PosMap_.Stored((ui8) pos)) {
                return pos;
            }
        }

        return NState::EPosition::INVALID;
    }

private:
    TFlags PosMap_;
    std::array<TPlayer, (ui32) NState::EPosition::NUM> Membership;
};

class TTableManager: public IActor {
public:
    explicit TTableManager(TActorConfig& config)
        : IActor(config)
    {
    }

private:
    void OnAction(TActionReq::TPtr ev) {
        auto ptr = Complete_.find(ev->TaId);

        if (ptr == Complete_.end()) {
            Send(
                    ev->Sender(),
                    MakeEvent<TActionResp>(NState::TStateError{NState::TStateError::UNKNOWN, "table not found"})
                );

            return;
        }

        auto& state = ptr->second;

        auto res = state.State->OnAction(ev->Action);
        Send(ev->Sender(), MakeEvent<TActionResp>(std::move(res)));

        if (!res.Succeed()) {
            return;
        }

        for (auto pos: NState::AllPositions) {
            auto snapshot = state.State->Snapshot(pos);
            snapshot.LastEvent = ev->Action;
            Send(state.Membership.Member(pos), MakeEvent<TSnapshot>(std::move(snapshot)));
        }
    }

    void OnJoinAny(TJoinAnyReq::TPtr ev) {
        if (Pending_.empty()) {
            Pending_.emplace(
                    std::make_pair(GenerateTableId(), TTable{{}, CreateState(NState::CreateRandomGenerator())})
                );
        }

        auto& table = Pending_.begin()->second;

        auto taPos = table.Membership.Join(ev->Sender());

        TJoinAnyResp resp(Pending_.begin()->first, taPos);

        Send(ev->Sender(), MakeEvent<TJoinAnyResp>(std::move(resp)));

        if (table.Membership.Complete()) {
            table.State->Start();

            for (auto pos: NState::AllPositions) {
                Send(table.Membership.Member(pos), MakeEvent<TSnapshot>(table.State->Snapshot(pos)));
            }

            auto node = Pending_.extract(Pending_.begin());
            Complete_.emplace(std::make_pair(node.key(), std::move(node.mapped())));
        }
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TActionReq, OnAction);
        CREATE_HANDLER(TJoinAnyReq, OnJoinAny);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    TTableId GenerateTableId() {
        ++Initializer;
        return Initializer;
    }

private:
    struct TTable {
        TTableMembership<NActors::TActorId> Membership;
        std::unique_ptr<NState::IState> State;
    };

    std::unordered_map<TTableId, TTable> Complete_;
    std::unordered_map<TTableId, TTable> Pending_;

    ui64 Initializer{0};
};

NActors::TActorId CreateTableManager() {
    return TActorSystem::Instance().Spawn<TTableManager>();
}

} // namespace NTichu::NServer