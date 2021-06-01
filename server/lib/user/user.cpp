#include "user.h"

#include <unordered_map>
#include <optional>

using namespace NActors;
using namespace NTichu::NGameplay::NState;

namespace NTichu::NServer {

class TUserActor: public IActor {
public:
    explicit TUserActor(TActorConfig& config, TUserId userId, TWeakActor tableManager)
        : IActor(config)
        , SelfId_(userId)
        , TableManager_(std::move(tableManager))
    {
    }

private:
    struct TTable {
        TTableId Table;
        EPosition Position;
        TWeakActor Consumer;
    };

    void OnJoinReq(TTaEv::TJoinAnyReq::TPtr req) {
        if (ActiveTable_) {
            Send(
                    req->Sender(),
                    MakeEvent<TTaEv::TJoinAnyResp>("unable to join: player connected to some table")
                );

            return;
        }

        ActiveTable_ = TTable {
            .Consumer = req->Sender()
        };

        Send(TableManager_, std::move(req));
    }

    void OnJoinResp(TTaEv::TJoinAnyResp::TPtr resp) {
        if (!ActiveTable_) {
            return;
        }

        ActiveTable_->Table = resp->TaId;
        ActiveTable_->Position = resp->Position;

        Send(ActiveTable_->Consumer, std::move(resp));
    }

    void OnAction(TTaEv::TActionReq::TPtr action) {
        if (!ActiveTable_) {
            Send(
                    action->Sender(),
                    MakeEvent<TTaEv::TActionResp>(TStateError(TStateError::UNKNOWN, "player has no active table"))
                );

            return;
        }

        TActorSystem::Send(action->Sender(), TableManager_, std::move(action));
    }

    void OnSnapshot(TTaEv::TSnapshot::TPtr ev) {
        if (!ActiveTable_) {
            return;
        }

        Send(ActiveTable_->Consumer, std::move(ev));
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TTaEv::TJoinAnyReq, OnJoinReq);
        CREATE_HANDLER(TTaEv::TJoinAnyResp, OnJoinResp);
        CREATE_HANDLER(TTaEv::TActionReq, OnAction);
        CREATE_HANDLER(TTaEv::TSnapshot, OnSnapshot);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    TUserId SelfId_;
    TWeakActor TableManager_;

    std::optional<TTable> ActiveTable_;
};

class TUserManager: public IActor {
public:
    explicit TUserManager(TActorConfig& config, TWeakActor tableManager)
        : IActor(config)
        , TableManager_(std::move(tableManager))
    {
    }

private:
    void OnEvent(TUserEv::TProxy::TPtr event) {
        auto ptr = Table_.find(event->UserId);

        if (ptr == Table_.end()) {
            auto user = System().Spawn<TUserActor>(event->UserId, TableManager_);
            ptr = Table_.emplace(std::make_pair(event->UserId, std::move(user))).first;
        }

        TActorSystem::Send(event->Sender(), ptr->second, std::move(event->Event));
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TUserEv::TProxy, OnEvent);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    TWeakActor TableManager_;
    std::unordered_map<TUserId, TWeakActor> Table_;
};

NActors::TWeakActor CreateUserManager(NActors::TActorSystem sys, NActors::TWeakActor tableManager) {
    return sys.Spawn<TUserManager>(std::move(tableManager));
}

} // namespace NTichu::NServer
