#include "table.h"
#include "def.h"
#include "events.h"

#include <server/lib/gameplay/state/defs.h>

#include <array>
#include <unordered_map>
#include <unordered_set>

using namespace NActors;

namespace NTichu::NServer::NTable {
namespace {

using namespace NTichu::NGameplay::NState;

struct TMember {
    TUserId User{INVALID_USER_ID};
};

class TMembership {
public:
    size_t Count() const noexcept {
        size_t count = 0;
        for (const auto& member: Membership_) {
            if (member.User != INVALID_USER_ID) {
                ++count;
            }
        }

        return count;
    }

    bool Complete() const noexcept {
        return Count() == (size_t) EPosition::NUM;
    }

    EPosition UserPosition(TUserId user) const noexcept {
        for (size_t i = 0; i < (size_t) EPosition::NUM; ++i) {
            if (Membership_[i].User == user) {
                return (EPosition) i;
            }
        }

        return EPosition::INVALID;
    }

    TErrorOr<EPosition, std::string> Join(TUserId user) {
        if (Complete()) {
            return std::string("table is full");
        }

        if (ValidatePosition(UserPosition(user))) {
            return UserPosition(user);
        }

        for (size_t i = 0; i < (size_t) EPosition::NUM; ++i) {
            if (Membership_[i].User == INVALID_USER_ID) {
                Membership_[i].User = user;

                return (EPosition) i;
            }
        }

        assert(false && "internal error");
    }

    bool Leave(TUserId user) {
        EPosition pos = UserPosition(user);
        if (!ValidatePosition(pos)) {
            return false;
        }

        Membership_[(size_t) pos].User = INVALID_USER_ID;
        return true;
    }

private:
    std::array<TMember, (size_t) EPosition::NUM> Membership_;
};

class TTableManager {
public:
    TTablePosition JoinAny(TUserId user) {
        TTableMap::iterator it;

        if (Pending_.empty()) {
            ++Tag_;

            TValue value;
            value.Options.Name = "default";

            std::tie(it, std::ignore) = Pending_.emplace(std::make_pair(Tag_, std::move(value)));
        } else {
            it = Pending_.begin();
        }

        return Join(it, user);
    }

    TErrorOr<TTablePosition, std::string> Join(TTableId table, TUserId user) {
        auto it = Pending_.find(table);

        if (it == Pending_.end()) {
            if (Complete_.find(table) != Complete_.end()) {
                return std::string("table is full");
            } else {
                return std::string("table not found");
            }
        }

        return Join(it, user);
    }

    TErrorOr<bool, std::string> Leave(TTableId table, TUserId user) {
        auto it = Pending_.find(table);
        if (it != Pending_.end()) {
            bool left = it->second.Membership.Leave(user);
            if (it->second.Membership.Count() == 0) {
                Pending_.erase(it);
            }

            return left;
        }

        it = Complete_.find(table);
        if (it != Complete_.end()) {
            bool left = it->second.Membership.Leave(user);
            Pending_.emplace(std::make_pair(it->first, std::move(it->second)));
            Complete_.erase(it);

            return left;
        }

        return std::string("table not found");
    }

    TTablePosition Create(TUserId user, TTableOptions options) {
        ++Tag_;

        TValue value;
        value.Options = std::move(options);
        auto [it, _] = Pending_.emplace(std::make_pair(Tag_, std::move(value)));

        return Join(it, user);
    }

    std::vector<TTableListItem> List() const {
        std::vector<TTableListItem> response;
        response.reserve(Pending_.size());

        for (const auto& [key, value]: Pending_) {
            response.emplace_back(TTableListItem{
                .Table = key,
                .Options = value.Options,
                .Joined = value.Membership.Count()
            });
        }

        return response;
    }

private:
    struct TValue {
        TMembership Membership;
        TTableOptions Options;
    };

    using TTableMap = std::unordered_map<TTableId, TValue>;

    ui64 Tag_{0};
    TTableMap Pending_;
    TTableMap Complete_;

private:
    TTablePosition Join(TTableMap::iterator it, TUserId user) {
        auto& value = it->second;
        auto key = it->first;

        auto r = value.Membership.Join(user);

        assert(r.Succeed() && "internal error");

        TTablePosition response{
            .Table = key,
            .Position = r.Value()
        };

        if (value.Membership.Complete()) {
            Complete_.emplace(std::make_pair(key, std::move(value)));
            Pending_.erase(it);
        }

        return response;
    }
};

class TTableManagerActor: public IActor {
public:
    TTableManagerActor(TActorConfig& config)
        : IActor(config)
    {
    }

private:
    void OnJoinAny(TEvJoinAnyRequest::TPtr ev) {
        auto place = Manager_.JoinAny(ev->User);
        ev->Answer(ev->Sender(), MakeEvent<TEvJoinResponse>(place));
        
        OnTableListUpdate();
    }

    void OnJoin(TEvJoinRequest::TPtr ev) {
        auto response = Manager_.Join(ev->Table, ev->User);
        ev->Answer(ev->Sender(), MakeEvent<TEvJoinResponse>(std::move(response)));
        
        OnTableListUpdate();
    }

    void OnLeave(TEvLeaveRequest::TPtr ev) {
        auto response = Manager_.Leave(ev->Table, ev->User);
        if (response.Fail()) {
            ev->Answer(ev->Sender(), MakeEvent<TEvLeaveResponse>(response.Error()));
        } else {
            ev->Answer(ev->Sender(), MakeEvent<TEvLeaveResponse>());
        }

        if (response.Value()) {
            OnTableListUpdate();
        }
    }

    void OnCreate(TEvCreateRequest::TPtr ev) {
        auto response = Manager_.Create(ev->User, ev->Options);
        ev->Answer(ev->Sender(), MakeEvent<TEvCreateResponse>(response));

        OnTableListUpdate();
    }

    void OnSubscribeTableList(TEvSubscribeTableList::TPtr ev) {
        TableListSubs.emplace(ev->Sender());
        Send(ev->Sender(), MakeEvent<TEvTableList>(Manager_.List()));
    }

    void OnUnSubscribeTableList(TEvUnSubscribeTableList::TPtr ev) {
        TableListSubs.erase(ev->Sender());
    }

    void OnTableListUpdate() {
        auto tables = Manager_.List();
        for (const auto& actor: TableListSubs) {
            Send(actor, MakeEvent<TEvTableList>(tables));
        }
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TEvJoinAnyRequest, OnJoinAny);
        CREATE_HANDLER(TEvJoinRequest, OnJoin);
        CREATE_HANDLER(TEvCreateRequest, OnCreate);
        CREATE_HANDLER(TEvLeaveRequest, OnLeave);
        CREATE_HANDLER(TEvSubscribeTableList, OnSubscribeTableList);
        CREATE_HANDLER(TEvUnSubscribeTableList, OnUnSubscribeTableList);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    TTableManager Manager_;
    std::unordered_set<TActorId> TableListSubs;
};

}

NActors::TActorId CreateTableManager() {
    return TActorSystem::Instance().Spawn<TTableManagerActor>();
}

} // namespace NTichu::NServer
