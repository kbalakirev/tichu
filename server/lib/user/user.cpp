#include "user.h"

#include <unordered_map>
#include <optional>

using namespace NActors;
using namespace NTichu::NGameplay::NState;

namespace NTichu::NServer {

using namespace NEvUserManager;

class TUserActor: public IActor {
public:
    explicit TUserActor(TActorConfig& config, TUserId userId, TActorId tableManager)
        : IActor(config)
        , SelfId_(userId)
        , TableManager_(std::move(tableManager))
    {
    }

private:
    struct TTable {
        TTableId Table;
        EPosition Position;
        NGameplay::NState::TSnapshot LastSnapshot;
        NActors::TActorId Consumer;
    };

    void OnJoinReq(NEvTableManager::TJoinAnyReq::TPtr ev) {
        if (ActiveTable_) {
            Send(
                    ev->Sender(),
                    MakeEvent<NEvTableManager::TJoinAnyResp>("unable to join: player has an active table")
                );

            return;
        }

        Send(TableManager_, std::move(ev));
    }

    void OnJoinResp(NEvTableManager::TJoinAnyResp::TPtr ev) {
        if (!ActiveTable_) {
            return;
        }

        ActiveTable_->Table = ev->TaId;
        ActiveTable_->Position = ev->Position;

        Send(ActiveTable_->Consumer, std::move(ev));
    }

    void OnAction(NEvTableManager::TActionReq::TPtr ev) {
        if (!ActiveTable_) {
            Send(
                    ev->Sender(),
                    MakeEvent<NEvTableManager::TActionResp>(TStateError(TStateError::UNKNOWN, "player has no active table"))
                );

            return;
        }

        Sender(ev->Action) = ActiveTable_->Position;

        TActorSystem::Send(ev->Sender(), TableManager_, std::move(ev));
    }

    void OnSnapshot(NEvTableManager::TSnapshot::TPtr ev) {
        if (!ActiveTable_) {
            return;
        }

        ActiveTable_->LastSnapshot = ev->Snapshot;

        Send(ActiveTable_->Consumer, std::move(ev));
    }

    void OnTableStream(TTableStreamReq::TPtr ev) {
        if (!ActiveTable_) {
            Send(ev->Sender(), MakeEvent<TTableStreamError>());
            return;
        }

        ActiveTable_->Consumer = ev->Sender();
        Send(ev->Sender(), MakeEvent<NEvTableManager::TSnapshot>(ActiveTable_->LastSnapshot));
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(NEvTableManager::TJoinAnyReq, OnJoinReq);
        CREATE_HANDLER(NEvTableManager::TJoinAnyResp, OnJoinResp);
        CREATE_HANDLER(NEvTableManager::TActionReq, OnAction);
        CREATE_HANDLER(NEvTableManager::TSnapshot, OnSnapshot);
        CREATE_HANDLER(TTableStreamReq, OnTableStream);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    TUserId SelfId_;
    NActors::TActorId TableManager_;

    std::optional<TTable> ActiveTable_;
};

class TUserManager: public IActor {
public:
    explicit TUserManager(TActorConfig& config, TUserManagerOptions options)
        : IActor(config)
        , TableManager_(std::move(options.TableManager))
    {
    }

private:
    void OnEvent(TProxy::TPtr ev) {
        auto ptr = Table_.find(ev->UserId);

        if (ptr == Table_.end()) {
            auto user = System().Spawn<TUserActor>(ev->UserId, TableManager_);
            ptr = Table_.emplace(std::make_pair(ev->UserId, std::move(user))).first;
        }

        TActorSystem::Send(ev->Sender(), ptr->second, std::move(ev->Event));
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TProxy, OnEvent);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    NActors::TActorId TableManager_;
    std::unordered_map<TUserId, NActors::TActorId> Table_;
};

NActors::TActorId CreateUserManager(TUserManagerOptions options) {
    return TActorSystem::Instance().Spawn<TUserManager>(std::move(options));
}

} // namespace NTichu::NServer
