#include "table.h"

#include <server/lib/gameplay/state/state.h>
#include <server/lib/flags/flags.h>

#include <unordered_map>
#include <array>
#include <memory>
#include <cassert>

using namespace NActors;
using namespace NTichu::NGameplay::NState;

namespace NTichu::NServer {

template <class TPlayer>
class TTableMembership {
public:
    bool Complete() const {
        return ValidatePosition(FindPosition());
    }

    EPosition Join(TPlayer player) {
        assert(!Complete());

        EPosition pos = FindPosition();

        PosMap_.Store((ui8) pos);
        Membership[(ui8) pos] = std::move(player);

        return pos;
    }

    TPlayer Member(EPosition pos) const {
        assert(PosMap_.Stored((ui8) pos));

        return Membership[(ui8) pos];
    }

private:
    EPosition FindPosition() const {
        for (auto pos: AllPositions) {
            if (!PosMap_.Stored((ui8) pos)) {
                return pos;
            }
        }

        return EPosition::INVALID;
    }

private:
    TFlags PosMap_;
    std::array<TPlayer, (ui32) EPosition::NUM> Membership;
};

class TTableManager: public IActor {
public:
    explicit TTableManager(TActorConfig& config)
        : IActor(config)
    {
        auto now = std::chrono::system_clock::now();
        Tag = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    }

private:
    void OnAction(TTaEv::TActionReq::TPtr action) {
        auto ptr = Complete_.find(action->TaId);

        if (ptr == Complete_.end()) {
            Send(
                    action->Sender(),
                    MakeEvent<TTaEv::TActionResp>(TStateError{TStateError::UNKNOWN, "table not found"})
                );

            return;
        }

        auto& state = ptr->second;

        auto res = state.State->OnAction(action->Action);
        Send(action->Sender(), MakeEvent<TTaEv::TActionResp>(std::move(res)));

        if (!res.Succeed()) {
            return;
        }

        for (auto pos: AllPositions) {
            auto snapshot = state.State->Snapshot(pos);
            snapshot.LastEvent = action->Action;
            Send(state.Membership.Member(pos), MakeEvent<TTaEv::TSnapshot>(std::move(snapshot)));
        }
    }

    void OnJoinAny(TTaEv::TJoinAnyReq::TPtr req) {
        if (Pending_.empty()) {
            Pending_.emplace(
                    std::make_pair(GenerateTableId(), TTable{{}, CreateState(CreateRandomGenerator())})
                );
        }

        auto& table = Pending_.begin()->second;

        auto taPos = table.Membership.Join(req->Sender());

        TTaEv::TJoinAnyResp resp(Pending_.begin()->first, taPos);

        Send(req->Sender(), MakeEvent<TTaEv::TJoinAnyResp>(std::move(resp)));

        if (table.Membership.Complete()) {
            table.State->Start();

            for (auto pos: AllPositions) {
                Send(table.Membership.Member(pos), MakeEvent<TTaEv::TSnapshot>(table.State->Snapshot(pos)));
            }

            auto node = Pending_.extract(Pending_.begin());
            Complete_.emplace(std::make_pair(node.key(), std::move(node.mapped())));
        }
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TTaEv::TActionReq, OnAction);
        CREATE_HANDLER(TTaEv::TJoinAnyReq, OnJoinAny);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    TTableId GenerateTableId() {
        ++Initializer;
        return TTableId{Initializer, Tag};
    }

private:
    struct TTable {
        TTableMembership<TWeakActor> Membership;
        std::unique_ptr<IState> State;
    };

    std::unordered_map<TTableId, TTable> Complete_;
    std::unordered_map<TTableId, TTable> Pending_;

    ui64 Initializer{0};
    ui64 Tag;
};

NActors::TWeakActor CreateTableManager(NActors::TActorSystem sys) {
    return sys.Spawn<TTableManager>();
}

} // namespace NTichu::NServer